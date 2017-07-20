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
#define YYLAST   18431

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  206
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  301
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1078
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1990

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
    1165,  1162,  1178,  1175,  1192,  1191,  1200,  1198,  1210,  1209,
    1228,  1226,  1245,  1244,  1253,  1251,  1262,  1262,  1269,  1268,
    1280,  1278,  1291,  1292,  1296,  1299,  1302,  1303,  1304,  1307,
    1308,  1311,  1313,  1316,  1317,  1320,  1321,  1324,  1325,  1329,
    1330,  1335,  1336,  1339,  1340,  1341,  1345,  1346,  1350,  1351,
    1355,  1356,  1360,  1361,  1366,  1367,  1373,  1374,  1375,  1376,
    1379,  1382,  1384,  1387,  1388,  1392,  1394,  1397,  1400,  1403,
    1404,  1407,  1408,  1412,  1418,  1424,  1431,  1433,  1438,  1443,
    1449,  1453,  1457,  1461,  1466,  1471,  1476,  1481,  1487,  1496,
    1501,  1506,  1512,  1514,  1518,  1522,  1527,  1531,  1534,  1537,
    1541,  1545,  1549,  1553,  1558,  1566,  1568,  1571,  1572,  1573,
    1574,  1576,  1578,  1583,  1584,  1587,  1588,  1589,  1593,  1594,
    1596,  1597,  1601,  1603,  1606,  1610,  1616,  1618,  1621,  1621,
    1625,  1624,  1628,  1630,  1633,  1636,  1634,  1651,  1647,  1662,
    1664,  1666,  1668,  1670,  1672,  1674,  1678,  1679,  1680,  1683,
    1689,  1693,  1699,  1702,  1707,  1709,  1714,  1719,  1723,  1724,
    1728,  1729,  1731,  1733,  1739,  1740,  1742,  1746,  1747,  1752,
    1756,  1757,  1761,  1762,  1766,  1768,  1774,  1779,  1780,  1782,
    1786,  1787,  1788,  1789,  1793,  1794,  1795,  1796,  1797,  1798,
    1800,  1805,  1808,  1809,  1813,  1814,  1818,  1819,  1822,  1823,
    1826,  1827,  1830,  1831,  1835,  1836,  1837,  1838,  1839,  1840,
    1841,  1845,  1846,  1849,  1850,  1851,  1854,  1856,  1858,  1859,
    1862,  1864,  1868,  1870,  1874,  1878,  1882,  1887,  1888,  1890,
    1891,  1892,  1893,  1896,  1900,  1901,  1905,  1906,  1910,  1911,
    1912,  1913,  1917,  1921,  1926,  1930,  1934,  1938,  1942,  1947,
    1948,  1949,  1950,  1951,  1955,  1957,  1958,  1959,  1962,  1963,
    1964,  1965,  1966,  1967,  1968,  1969,  1970,  1971,  1972,  1973,
    1974,  1975,  1976,  1977,  1978,  1979,  1980,  1981,  1982,  1983,
    1984,  1985,  1986,  1987,  1988,  1989,  1990,  1991,  1992,  1993,
    1994,  1995,  1996,  1997,  1998,  1999,  2000,  2001,  2002,  2003,
    2004,  2005,  2007,  2008,  2010,  2011,  2013,  2014,  2015,  2016,
    2017,  2018,  2019,  2020,  2021,  2022,  2023,  2024,  2025,  2026,
    2027,  2028,  2029,  2030,  2031,  2032,  2033,  2034,  2035,  2036,
    2037,  2041,  2045,  2050,  2049,  2064,  2062,  2080,  2079,  2098,
    2097,  2116,  2115,  2133,  2133,  2148,  2148,  2166,  2167,  2168,
    2173,  2175,  2179,  2183,  2189,  2193,  2199,  2201,  2205,  2207,
    2211,  2215,  2216,  2220,  2222,  2226,  2228,  2232,  2234,  2238,
    2241,  2246,  2248,  2252,  2255,  2260,  2264,  2268,  2272,  2276,
    2280,  2284,  2288,  2292,  2296,  2300,  2304,  2308,  2312,  2316,
    2320,  2322,  2326,  2328,  2332,  2334,  2338,  2345,  2352,  2354,
    2359,  2360,  2361,  2362,  2363,  2364,  2365,  2366,  2367,  2369,
    2370,  2374,  2375,  2376,  2377,  2381,  2387,  2396,  2409,  2410,
    2413,  2416,  2419,  2420,  2423,  2427,  2430,  2433,  2440,  2441,
    2445,  2446,  2448,  2453,  2454,  2455,  2456,  2457,  2458,  2459,
    2460,  2461,  2462,  2463,  2464,  2465,  2466,  2467,  2468,  2469,
    2470,  2471,  2472,  2473,  2474,  2475,  2476,  2477,  2478,  2479,
    2480,  2481,  2482,  2483,  2484,  2485,  2486,  2487,  2488,  2489,
    2490,  2491,  2492,  2493,  2494,  2495,  2496,  2497,  2498,  2499,
    2500,  2501,  2502,  2503,  2504,  2505,  2506,  2507,  2508,  2509,
    2510,  2511,  2512,  2513,  2514,  2515,  2516,  2517,  2518,  2519,
    2520,  2521,  2522,  2523,  2524,  2525,  2526,  2527,  2528,  2529,
    2530,  2531,  2532,  2533,  2537,  2542,  2543,  2547,  2548,  2549,
    2550,  2552,  2556,  2557,  2568,  2569,  2571,  2573,  2585,  2586,
    2587,  2591,  2592,  2593,  2597,  2598,  2599,  2602,  2604,  2608,
    2609,  2610,  2611,  2613,  2614,  2615,  2616,  2617,  2618,  2619,
    2620,  2621,  2622,  2625,  2630,  2631,  2632,  2634,  2635,  2637,
    2638,  2639,  2640,  2641,  2642,  2643,  2644,  2645,  2647,  2649,
    2651,  2653,  2655,  2656,  2657,  2658,  2659,  2660,  2661,  2662,
    2663,  2664,  2665,  2666,  2667,  2668,  2669,  2670,  2671,  2673,
    2675,  2677,  2679,  2680,  2683,  2684,  2688,  2692,  2694,  2698,
    2699,  2703,  2709,  2712,  2716,  2717,  2718,  2719,  2720,  2721,
    2722,  2727,  2729,  2733,  2734,  2737,  2738,  2742,  2745,  2747,
    2749,  2753,  2754,  2755,  2756,  2759,  2763,  2764,  2765,  2766,
    2770,  2772,  2779,  2780,  2781,  2782,  2787,  2788,  2789,  2790,
    2792,  2793,  2795,  2796,  2797,  2798,  2799,  2803,  2805,  2809,
    2811,  2814,  2817,  2819,  2821,  2824,  2826,  2830,  2832,  2835,
    2838,  2844,  2846,  2849,  2850,  2855,  2858,  2862,  2862,  2867,
    2870,  2871,  2875,  2876,  2880,  2881,  2882,  2886,  2891,  2896,
    2897,  2901,  2906,  2911,  2912,  2916,  2917,  2922,  2924,  2929,
    2940,  2954,  2966,  2981,  2982,  2983,  2984,  2985,  2986,  2987,
    2997,  3006,  3008,  3010,  3014,  3015,  3016,  3017,  3018,  3034,
    3035,  3037,  3039,  3046,  3047,  3048,  3049,  3050,  3051,  3052,
    3053,  3055,  3060,  3064,  3065,  3069,  3072,  3079,  3083,  3092,
    3099,  3107,  3109,  3110,  3114,  3115,  3116,  3118,  3123,  3124,
    3135,  3136,  3137,  3138,  3149,  3152,  3155,  3156,  3157,  3158,
    3169,  3173,  3174,  3175,  3177,  3178,  3179,  3183,  3185,  3188,
    3190,  3191,  3192,  3193,  3196,  3198,  3199,  3203,  3205,  3208,
    3210,  3211,  3212,  3216,  3218,  3221,  3224,  3226,  3228,  3232,
    3233,  3235,  3236,  3242,  3243,  3245,  3255,  3257,  3259,  3262,
    3263,  3264,  3268,  3269,  3270,  3271,  3272,  3273,  3274,  3275,
    3276,  3277,  3278,  3282,  3283,  3287,  3289,  3297,  3299,  3303,
    3307,  3312,  3316,  3324,  3325,  3329,  3330,  3336,  3337,  3346,
    3347,  3355,  3358,  3362,  3365,  3370,  3375,  3377,  3378,  3379,
    3382,  3384,  3390,  3391,  3395,  3396,  3400,  3401,  3405,  3406,
    3409,  3414,  3415,  3419,  3422,  3424,  3428,  3434,  3435,  3436,
    3440,  3444,  3452,  3457,  3469,  3471,  3475,  3478,  3480,  3485,
    3490,  3496,  3499,  3504,  3509,  3511,  3518,  3520,  3523,  3524,
    3527,  3530,  3531,  3536,  3538,  3542,  3548,  3558,  3559
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

#define YYTABLE_NINF -1062

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1062)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1605,   142, -1605, -1605,  5697, 13614, 13614,   -41, 13614, 13614,
   13614, 13614, 11381, 13614, -1605, 13614, 13614, 13614, 13614, 16869,
   16869, 13614, 13614, 13614, 13614, 13614, 13614, 13614, 13614, 11584,
   17534, 13614,   -15,     9, -1605, -1605, -1605,   141, -1605,   352,
   -1605, -1605, -1605,   184, 13614, -1605,     9,   215,   239,   249,
   -1605,     9, 11787, 15068, 11990, -1605, 14644, 10366,   203, 13614,
   17775,   153,   316,   257,   271, -1605, -1605, -1605,   282,   292,
     333,   347, -1605, 15068,   376,   422,   551,   561,   566,   570,
     585, -1605, -1605, -1605, -1605, -1605, 13614,   515,  2403, -1605,
   -1605, 15068, -1605, -1605, -1605, -1605, 15068, -1605, 15068, -1605,
     372,   475, 15068, 15068, -1605,   291, -1605, -1605, 12193, -1605,
   -1605,   354,   510,   618,   618, -1605,   631,   519,   396,   492,
   -1605,    96, -1605,   667, -1605, -1605, -1605, -1605, 14186,   576,
   -1605, -1605,   524,   531,   546,   556,   557,   564,   573,   590,
   11365, -1605, -1605, -1605, -1605,   159,   670,   720,   721,   723,
     730, -1605,   731,   732, -1605,   143,   543, -1605,   643,     4,
   -1605,  1221,    57, -1605, -1605,  2417,   139,   611,   171, -1605,
     148,   144,   612,   205, -1605, -1605,   741, -1605,   655, -1605,
   -1605,   620,   650, -1605, 13614, -1605,   667,   576, 18024,  3587,
   18024, 13614, 18024, 18024, 18288, 18288,   619, 16388, 18024,   769,
   15068,   751,   751,   504,   751, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605,    64, 13614,   641, -1605, -1605,   663,
     629,   307,   630,   307,   751,   751,   751,   751,   751,   751,
     751,   751, 16869, 17038,   621,   821,   655, -1605, 13614,   641,
   -1605,   678, -1605,   679,   646, -1605,   150, -1605, -1605, -1605,
     307,   139, -1605, 12396, -1605, -1605, 13614,  9148,   832,   100,
   18024, 10163, -1605, 13614, 13614, 15068, -1605, -1605, 11771,   647,
   -1605, 12380, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, 15971, -1605, 15971, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605,    81,    90,   650, -1605, -1605, -1605, -1605,   648,
    4309,    92, -1605, -1605,   688,   835, -1605,   691, 15363, -1605,
     658,   659, 13598, -1605,    -2, 15836, 14715, 14715, 14715, 15068,
   14715,   653,   844,   660, -1605,    68, -1605, 16457,   101, -1605,
     845,   102,   733, -1605,   735, -1605, 16869, 13614, 13614,   664,
     681, -1605, -1605, 16560, 11584, 13614, 13614, 13614, 13614, 13614,
     114,   129,   467, -1605, 13817, 16869,   637, -1605, 15068, -1605,
     -11,   519, -1605, -1605, -1605, -1605, 17634,   853,   766, -1605,
   -1605, -1605,    80, 13614,   673,   674, 18024,   676,  2362,   685,
    5900, 13614, -1605,   469,   689,   640,   469,   480,   484, -1605,
   15068, 15971,   687, 10569, 14644, -1605, -1605,  5141, -1605, -1605,
   -1605, -1605, -1605,   667, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, 13614, 13614, 13614, 13614, 12599, 13614, 13614,
   13614, 13614, 13614, 13614, 13614, 13614, 13614, 13614, 13614, 13614,
   13614, 13614, 13614, 13614, 13614, 13614, 13614, 13614, 13614, 13614,
   13614, 17734, 13614, -1605, 13614, 13614, 13614, 13988, 15068, 15068,
   15068, 15068, 15068, 14186,   758,   713,  5071, 13614, 13614, 13614,
   13614, 13614, 13614, 13614, 13614, 13614, 13614, 13614, 13614, -1605,
   -1605, -1605, -1605,  1770, 13614, 13614, -1605, 10569, 10569, 13614,
   13614, 16560,   690,   667, 12802, 15884, -1605, 13614, -1605,   694,
     866,   736,   695,   697, 14140,   307, 13005, -1605, 13208, -1605,
     646,   698,   701,  2692, -1605,   112, 10569, -1605,  4290, -1605,
   -1605, 15932, -1605, -1605, 10772, -1605, 13614, -1605,   793,  9351,
     892,   705, 13801,   893,   111,    62, -1605, -1605, -1605,   734,
   -1605, -1605, -1605, 15971, -1605,   738,   715,   904, 16313, 15068,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,   724,
   -1605, -1605,   718,   725,   722,   727,   729,   739,   246,   737,
     742, 17810, 14891, -1605, -1605, 15068, 15068, 13614,   307,   153,
   -1605, 16313,   847, -1605, -1605, -1605,   307,   130,   131,   744,
     745,  3296,   105,   747,   748,   497,   800,   743,   307,   132,
     752, 17086,   749,   925,   936,   756,   759,   761,   762, -1605,
   14539, 15068, -1605, -1605,   880,  3069,    16, -1605, -1605, -1605,
     519, -1605, -1605, -1605,   919,   833,   787,   379,   823, 13614,
     851,   972,   792, -1605,   830, -1605,   158, -1605, 15971, 15971,
     978,   832,    80, -1605,   803,   984, -1605, 15971,    32, -1605,
     456,    82, -1605, -1605, -1605, -1605, -1605, -1605, -1605,  1134,
    3151, -1605, -1605, -1605, -1605,   993,   826, -1605, 16869, 13614,
     806,   996, 18024,   995, -1605, -1605,   878, 14034, 11975, 18207,
   18288, 18325, 13614, 17976, 18362, 11563, 12983, 13185,  4085,  2632,
    3975,  3975,  3975,  3975,  1507,  1507,  1507,  1507,  1507,   965,
     965,   596,   596,   596,   504,   504,   504, -1605,   751, 18024,
     809,   810, 17142,   814,  1012,     1, 13614,    56,   641,   145,
   -1605, -1605, -1605,  1011,   766, -1605,   667, 16663, -1605, -1605,
   -1605, 18288, 18288, 18288, 18288, 18288, 18288, 18288, 18288, 18288,
   18288, 18288, 18288, 18288, -1605, 13614,   219, -1605,   160, -1605,
     641,   250,   825,  3353,   829,   839,   827,  3531,   134,   841,
   -1605, 18024, 16416, -1605, 15068, -1605,    32,   411, 16869, 18024,
   16869, 17190,   878,    32,   307,   164, -1605,   158,   879,   850,
   13614, -1605,   204, -1605, -1605, -1605,  8945,   535, -1605, -1605,
   18024, 18024,     9, -1605, -1605, -1605, 13614,   940, 16189, 16313,
   15068,  9554,   858,   859, -1605,  1040, 14363,   926, -1605,   898,
   -1605,  1054,   868,  1754, 15971, 16313, 16313, 16313, 16313, 16313,
     871,   998,  1000,  1001,  1003,  1006,   881, 16313,     3, -1605,
   -1605, -1605, -1605, -1605, -1605,     8, -1605, 18118, -1605, -1605,
     406, -1605,  6103,  4704,   883, 14891, -1605, 14891, -1605, 14891,
   -1605, 15068, 15068, 14891, -1605, 14891, 14891, 15068, -1605,  1069,
     884, -1605,   283, -1605, -1605,  4027, -1605, 18118,  1065, 16869,
     882, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
     905,  1080, 15068,  4704,   891, 16560, 16766,  1077, -1605, 13614,
   -1605, 13614, -1605, 13614, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605,   890, -1605, 13614, -1605, -1605,  5291, -1605, 15971,
    4704,   895, -1605, -1605, -1605, -1605,  1082,   901, 13614, 17634,
   -1605, -1605, 13988,   902, -1605, 15971, -1605,   910,  6306,  1072,
      50, -1605, -1605,    94,  1770, -1605,  4290, -1605, 15971, -1605,
   -1605,   307, 18024, -1605, 10975, -1605, 16313,    49,   912,  4704,
     833, -1605, -1605, 18362, 13614, -1605, -1605, 13614, -1605, 13614,
   -1605,  4135,   913, 10569,   800,  1078,   833, 15971,  1097,   878,
   15068, 17734,   307,  4202,   916, -1605, -1605,   157,   917, -1605,
   -1605,  1100,  3584,  3584, 16416, -1605, -1605, -1605,  1063,   920,
    1047,  1052,  1053,  1055,  1058,    67,   933,   404, -1605, -1605,
   -1605, -1605, -1605,   970, -1605, -1605, -1605, -1605,  1122,   935,
     694,   307,   307, 13411,   833,  4290, -1605, -1605,  4784,   539,
       9, 10163, -1605,  6509,   938,  6712,   939, 16189, 16869,   942,
    1004,   307, 18118,  1120, -1605, -1605, -1605, -1605,   644, -1605,
      55, 15971,   959,  1007,   982, 15971, 15068,  2814, -1605, -1605,
   -1605,  1135, -1605,   950,   993,   684,   684,  1079,  1079, 16152,
     947,  1142, 16313, 16313, 16313, 16313, 16313, 16313, 17634,  2574,
   15515, 16313, 16313, 16313, 16313, 16070, 16313, 16313, 16313, 16313,
   16313, 16313, 16313, 16313, 16313, 16313, 16313, 16313, 16313, 16313,
   16313, 16313, 16313, 16313, 16313, 16313, 16313, 16313, 16313, 15068,
   -1605, -1605,  1070, -1605, -1605,   952,   956,   957, -1605,   958,
   -1605, -1605,   341, 17810, -1605,   961, -1605, 16313,   307, -1605,
   -1605,   140, -1605,   542,  1151, -1605, -1605,   136,   966,   307,
   11178, 18024, 17246, -1605,  3008, -1605,  5494,   766,  1151, -1605,
      -1,    27, -1605, 18024,  1028,   969, -1605,   968,  1072, -1605,
   15971,   832, 15971,    61,  1153,  1085,   206, -1605,   641,   207,
   -1605, -1605, 16869, 13614, 18024, 18118,   974,    49, -1605,   971,
      49,   976, 18362, 18024, 17294,   977, 10569,   979,   975, 15971,
     985,   987, 15971,   833, -1605,   646,   384, 10569, 13614, -1605,
   -1605, -1605, -1605, -1605, -1605,  1041,   983,  1181,  1102, 16416,
   16416, 16416, 16416, 16416, 16416,  1034, -1605, 17634,    93, 16416,
   -1605, -1605, -1605, 16869, 18024,   997, -1605,     9,  1163,  1121,
   10163, -1605, -1605, -1605,  1005, 13614,  1004,   307, 16560, 16189,
    1009, 16313,  6915,   654,  1010, 13614,    59,   287, -1605,  1020,
   -1605, 15971, 15068, -1605,  1071, -1605, -1605, 15923,  1177,  1013,
   16313, -1605, 16313, -1605,  1014,  1015,  1206, 17349,  1016, 18118,
    1207,  1018,  1023,  1024,  1081,  1214,  1031, -1605, -1605, -1605,
   17397,  1030,  1222, 18163, 18251, 10549, 16313, 18072, 12781,  3240,
   13388, 12575,  4902, 13984, 13984, 13984, 13984,  3406,  3406,  3406,
    3406,  3406,   922,   922,   684,   684,   684,  1079,  1079,  1079,
    1079, -1605,  1036, -1605,  1037,  1039,  1042,  1045, -1605, -1605,
   18118, 15068, 15971, 15971, -1605,   542,  4704,  1412, -1605, 16560,
   -1605, -1605, 18288, 13614,  1038, -1605,  1048,  1987, -1605,    95,
   13614, -1605, -1605, -1605, 13614, -1605, 13614, -1605,   832, -1605,
   -1605,   118,  1225,  1161, 13614, -1605,  1050,   307, 18024,  1072,
    1051, -1605,  1062,    49, 13614, 10569,  1064, -1605, -1605,   766,
   -1605, -1605,  1068,  1076,  1056, -1605,  1066, 16416, -1605, 16416,
   -1605, -1605,  1073,  1046,  1242,  1126,  1074, -1605,  1258,  1084,
    1086,  1087, -1605,  1132,  1083,  1262, -1605, -1605,   307, -1605,
    1247, -1605,  1090, -1605, -1605,  1094,  1096,   137, -1605, -1605,
   18118,  1098,  1099, -1605,  3891, -1605, -1605, -1605, -1605, -1605,
   -1605,  1147, 15971, -1605, 15971, -1605, 18118, 17452, -1605, -1605,
   16313, -1605, 16313, -1605, 16313, -1605, -1605, -1605, -1605, 16313,
   17634, -1605, -1605, 16313, -1605, 16313, -1605, 10955, 16313,  1095,
    7118, -1605, -1605, -1605, -1605,   542, -1605, -1605, -1605, -1605,
     547, 14820,  4704,  1183, -1605,  4565,  1130,  3656, -1605, -1605,
   -1605,   758,  3647,   116,   117,  1111,   766,   713,   138, 18024,
   -1605, -1605, -1605,  1145,  4871,  4969, 18024, -1605,    76,  1286,
    1231, 13614, -1605, 18024, 10569,  1200,  1072,  2039,  1072,  1123,
   18024,  1124, -1605,  2121,  1119,  2136, -1605, -1605,    49, -1605,
   -1605,  1186, -1605, -1605, 16416, -1605, 16416, -1605, 16416, -1605,
   -1605, -1605, -1605, 16416, -1605, 17634, -1605,  2228, -1605,  8945,
   -1605, -1605, -1605, -1605,  9757, -1605, -1605, -1605,  8945, 15971,
   -1605,  1127, 16313, 17500, 18118, 18118, 18118,  1187, 18118, 17555,
   10955, -1605, -1605,   542,  4704,  4704, 15068, -1605,  1305, 15667,
      84, -1605, 14820,   766, 15257, -1605,  1146, -1605,   120,  1129,
     121, -1605, 15173, -1605, -1605, -1605,   122, -1605, -1605,  4891,
   -1605,  1136, -1605,  1253,   667, -1605, 14997, -1605, 14997, -1605,
   -1605,  1325,   758, -1605, 14292, -1605, -1605, -1605, -1605,  1332,
    1266, 13614, -1605, 18024,  1156,  1154,  1072,   493, -1605,  1200,
    1072, -1605, -1605, -1605, -1605,  2287,  1157, 16416,  1220, -1605,
   -1605, -1605,  1224, -1605,  8945,  9960,  9757, -1605, -1605, -1605,
    8945, -1605, -1605, 18118, 16313, 16313, 16313,  7321,  1159,  1160,
   -1605, 16313, -1605,  4704, -1605, -1605, -1605, -1605, -1605, 15971,
    2685,  4565, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605,   363, -1605,  1130, -1605, -1605, -1605,
   -1605, -1605,    99,   359, -1605,  1348,   125, 15363,  1253,  1349,
   -1605, 15971,   667, -1605, -1605,  1166,  1351, 13614, -1605, 18024,
   -1605,   313,  1167, -1605, -1605, -1605,  1072,   493, 14468, -1605,
    1072, -1605, 16416, 16416, -1605, -1605, -1605, -1605,  7524, 18118,
   18118, 18118, -1605, -1605, -1605, 18118, -1605,  4246,  1358,  1359,
    1173, -1605, -1605, 16313, 15173, 15173,  1306, -1605,  4891,  4891,
     544, -1605, -1605, -1605, 16313,  1293, -1605,  1196,  1182,   127,
   16313, -1605, 15068, -1605, 16313, 18024,  1303, -1605,  1378, -1605,
    7727,  1189, -1605, -1605,   493, -1605, -1605,  7930,  1191,  1275,
   -1605,  1289,  1233, -1605, -1605,  1294, 15971,  1213,  2685, -1605,
   -1605, 18118, -1605, -1605,  1232, -1605,  1361, -1605, -1605, -1605,
   -1605, 18118,  1391,   497, -1605, -1605, 18118,  1210, 18118, -1605,
     338,  1216,  8133, -1605, -1605, -1605,  1212, -1605,  1215,  1238,
   15068,   713,  1235, -1605, -1605, -1605, 16313,  1240,    69, -1605,
    1337, -1605, -1605, -1605,  8336, -1605,  4704,   883, -1605,  1248,
   15068,   591, -1605, 18118, -1605,  1227,  1416,   656,    69, -1605,
   -1605,  1344, -1605,  4704,  1234, -1605,  1072,    73, -1605, -1605,
   -1605, -1605, 15971, -1605,  1236,  1237,   128, -1605,  1241,   656,
     156,  1072,  1230, -1605, 15971,   527, 15971,   224,  1421,  1347,
    1241, -1605,  1428, -1605,   383, -1605, -1605, -1605,   166,  1424,
    1356, 13614, -1605,   527,  8539, 15971, -1605, 15971, -1605,  8742,
     253,  1426,  1362, 13614, -1605, 18024, -1605, -1605, -1605, -1605,
   -1605,  1434,  1367, 13614, -1605, 18024, 13614, -1605, 18024, 18024
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
       0,     0,   289,   299,     0,     0,  1036,     0,   209,   532,
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
     926,     0,     0,   394,   133,   199,  1036,     0,     0,   211,
    1036,   851,     0,     0,   133,   246,   146,   167,     0,   569,
     554,   975,   190,   342,   343,   421,   240,     0,   814,   814,
       0,   367,   355,     0,     0,     0,   373,   375,     0,     0,
     380,   387,   388,   386,     0,     0,   329,  1017,     0,     0,
       0,   425,     0,   324,     0,   303,     0,   613,   816,   133,
       0,     0,   201,   207,     0,   573,   859,     0,     0,   169,
     345,   123,     0,   346,   347,     0,   813,     0,   813,   369,
     365,   370,   631,   632,     0,   356,   389,   390,   382,   383,
     381,   419,   416,  1049,   335,   331,   420,     0,   325,   614,
     815,     0,     0,   395,   133,   203,     0,   249,     0,   194,
       0,   401,     0,   361,   368,   372,     0,     0,   871,   337,
       0,   611,   531,   534,     0,   247,     0,     0,   170,   352,
       0,   400,   362,   417,  1018,     0,   816,   412,   871,   612,
     536,     0,   193,     0,     0,   351,  1036,   871,   276,   415,
     414,   413,  1078,   411,     0,     0,     0,   350,  1030,   412,
       0,  1036,     0,   349,     0,     0,  1078,     0,   281,   279,
    1030,   133,   816,  1032,     0,   396,   133,   336,     0,   282,
       0,     0,   277,     0,     0,   815,  1031,     0,  1035,     0,
       0,   285,   275,     0,   278,   284,   338,   189,  1033,  1034,
     397,   286,     0,     0,   273,   283,     0,   274,   288,   287
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1605, -1605, -1605,  -568, -1605, -1605, -1605,   169,     2,   -42,
     408, -1605,  -242,  -524, -1605, -1605,   299,   973,  1691, -1605,
    1889, -1605,  -502, -1605,    23, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605,  -456, -1605, -1605,  -152,
     181,    31, -1605, -1605, -1605, -1605, -1605, -1605,    33, -1605,
   -1605, -1605, -1605, -1605, -1605,    34, -1605, -1605,   960,   980,
     967,  -110,  -714,  -896,   464,   521,  -460,   208,  -972, -1605,
    -180, -1605, -1605, -1605, -1605,  -758,    40, -1605, -1605, -1605,
   -1605,  -451, -1605,  -640, -1605,  -454, -1605, -1605,   861, -1605,
    -160, -1605, -1605, -1095, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605,  -196, -1605,  -108, -1605, -1605, -1605,
   -1605, -1605,  -279, -1605,    -9, -1087, -1605, -1604,  -484, -1605,
    -159,   108,  -120,  -458, -1605,  -286, -1605, -1605, -1605,     0,
     -26,   -18,    36,  -739,   -72, -1605, -1605,    22, -1605,   -14,
   -1605, -1605,    -5,   -46,  -144, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605,  -612,  -879, -1605, -1605, -1605, -1605,
   -1605,  1673,  1105, -1605,   390, -1605,   254, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605,  -181,  -574,  -516, -1605, -1605, -1605, -1605,
   -1605,   318, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1080,  -353,  2861,    30, -1605,  1395,  -413, -1605, -1605,  -496,
    3510,  3435, -1605,    72, -1605, -1605,   398,   194,  -644, -1605,
   -1605,   481,   269,   262, -1605,   270, -1605, -1605, -1605, -1605,
   -1605,   462, -1605, -1605, -1605,   115,  -920,  -174,  -441,  -424,
   -1605,   540,  -118, -1605, -1605,    39,    43,   628, -1605, -1605,
     326,   -12, -1605,  -341,    26,  -365,   133,    98, -1605, -1605,
    -475,  1128, -1605, -1605, -1605, -1605, -1605,   672,   530, -1605,
   -1605, -1605,  -340,  -683, -1605,  1088, -1074, -1605,   -73,  -194,
     -97,   662, -1605,  -443, -1605,  -457,  -940, -1284,  -362,    38,
   -1605,   364,   437, -1605, -1605, -1605, -1605,   387, -1605,   104,
   -1140
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   115,   938,   651,   185,  1577,   748,
     361,   362,   363,   364,   889,   890,   891,   117,   118,   119,
     120,   121,   420,   684,   685,   559,   261,  1645,   565,  1554,
    1646,  1889,   874,   354,   588,  1849,  1134,  1333,  1908,   437,
     186,   686,   978,  1201,  1394,   125,   654,   995,   687,   706,
     999,   626,   994,   240,   540,   688,   655,   996,   439,   381,
     403,   128,   980,   941,   914,  1154,  1580,  1260,  1060,  1796,
    1649,   825,  1066,   564,   834,  1068,  1437,   817,  1049,  1052,
    1249,  1915,  1916,   674,   675,   700,   701,   368,   369,   371,
    1614,  1774,  1775,  1347,  1489,  1603,  1768,  1898,  1918,  1807,
    1853,  1854,  1855,  1590,  1591,  1592,  1593,  1809,  1810,  1816,
    1865,  1596,  1597,  1601,  1761,  1762,  1763,  1785,  1957,  1490,
    1491,   187,   130,  1932,  1933,  1766,  1493,  1494,  1495,  1496,
     131,   254,   560,   561,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,  1626,   142,   977,  1200,   143,   671,
     672,   673,   258,   412,   555,   660,   661,  1295,   662,  1296,
     144,   145,   632,   633,  1285,  1286,  1403,  1404,   146,   859,
    1028,   147,   860,  1029,   148,   861,  1030,   149,   862,  1031,
     150,   863,  1032,   635,  1288,  1406,   151,   864,   152,   153,
    1838,   154,   656,  1616,   657,  1170,   946,  1365,  1362,  1754,
    1755,   155,   156,   157,   243,   158,   244,   255,   424,   547,
     159,  1289,  1290,   868,   869,   160,  1090,   969,   603,  1091,
    1035,  1223,  1036,  1407,  1408,  1226,  1227,  1038,  1414,  1415,
    1039,   793,   530,   199,   200,   689,   677,   513,  1186,  1187,
     779,   780,   965,   162,   246,   163,   164,   189,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   740,   250,   251,
     629,   234,   235,   743,   744,  1301,  1302,   396,   397,   932,
     175,   617,   176,   670,   177,   345,  1776,  1828,   382,   432,
     695,   696,  1083,  1945,  1952,  1953,  1181,  1344,   910,  1345,
     911,   912,   840,   841,   842,   346,   347,   871,   574,  1579,
     963
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     188,   190,   494,   192,   193,   194,   195,   197,   198,   444,
     201,   202,   203,   204,   343,   257,   224,   225,   226,   227,
     228,   229,   230,   231,   233,   404,   252,   122,   262,   407,
     408,   961,   957,   266,   522,   124,   415,   126,   127,   260,
    1182,   666,  1371,   351,   802,   544,   788,   268,   516,   271,
     816,   493,   352,   663,   355,   440,   249,   975,   342,   956,
     242,  1478,   417,   784,   785,   665,   667,   444,   737,   247,
    1174,   998,   777,   248,  1357,  1070,   548,   888,   893,   350,
     259,   260,   414,   937,  1199,  1256,   872,  1056,  1044,   778,
     -78,  1435,   809,  1663,   832,   -78,   419,    14,    14,   -43,
    1210,   -42,   514,   416,   -43,   434,   -42,   812,  1818,   556,
     609,   612,   129,   589,   593,   595,   597,    14,   600,   161,
     830,    14,   390,   556,   813,  1606,  1608,  -907,    14,  -357,
    1671,  1756,  1183,   549,  1825,  1819,  1825,  1663,   417,   899,
     556,   916,     3,   916,  1372,   916,   916,   916,  1245,  1503,
     511,   512,  -911,   430,   531,   191,  1508,  1235,   414,  1619,
     348,  1265,  1266, -1022,  -718,  1358,   511,   512,  1100,   533,
     605,   958,   419,   116,  1416,  -607,   590,  1184,  1359,   416,
     807,   253,  -107,  1842,   431,   123,   525,   652,   653,   532,
     908,   909,   542,  1363,  1947,   206,    40,  -107,  1360,   640,
    -917,  1509, -1022,   419,  1970,   256,   636,  1101,   638,   514,
     416,  -610,   541,  -904,  -906,  -912,   936,   641,  -905,  1294,
    -947,   393,   269,  1298,  1236,   341,  1364,  -910,   519,  -600,
    -908,   515,   606,   416,  -950,  1373,   366,  -106,  1368,  1948,
    1885,  -607,   380,  -815,   422,  1144,  -296,  -815,   551,  1971,
    1620,   551,  -106,  -914,  -608,  1268,  -907,  1436,   260,   562,
     405,   833,   573,   511,   512,   402,  -280,   380,  1185,   495,
    -815,   380,   380,  -296,  -949,   519,  -891,  -892,  -918,   -78,
     553,  -911,  1664,  1665,   558,  1213,  1517,  1428,   -43,  1478,
     -42,   707,  1510,  1523,   435,  1525,  1820,   380,   557,   610,
     613,   518,   905,  -726,  -725,  -727,   584,  1959,   831,  1263,
     443,  1267,   639,   111,  1607,  1609,  1053,  1393,  -357,  1672,
    1757,  1055,  -813,  1826,  1547,  1875,  1943,   900,   901,   917,
    1949,  1011,   620,  1348,  1553,  1613,  1981,  1196,   515,  -915,
    1972,   993,  -904,  -906,  -912,   222,   222,  -905,   881,  -947,
     342,  1836,   619,  -921,   367,  -916,  -910,   520,  1413,  -908,
    -543,  1140,  1141,  -950,   705,  -720,   623,   518,   523,   529,
    1166,   789,   260,   416,   409,   444,  1900,  1813,   372,   233,
     631,   260,   260,   631,   260,   605,   571,   373,   572,   645,
    1821,   374,   343,  1265,  1266,  1814,  1837,  1967,  1960,   370,
    -879,   375,   353,  -949,   520,  -891,  -892,  1578,   197,  1822,
     882,   263,  1823,   618,  1815,  -879,   690,   404,   753,   754,
     440,  1901,   634,   634,   758,   634,   116,  1982,   702,   539,
     116,  -882,  1635,   943,   563,   264,   342,   206,    40,  1515,
    1157,   511,   512,   881,   577,   265,  -882,   430,   708,   709,
     710,   711,   713,   714,   715,   716,   717,   718,   719,   720,
     721,   722,   723,   724,   725,   726,   727,   728,   729,   730,
     731,   732,   733,   734,   735,   736,  -719,   738,   376,   739,
     739,   742,   760,   410,  1356,  1342,  1343,  1438,   377,   747,
     411,   761,   762,   763,   764,   765,   766,   767,   768,   769,
     770,   771,   772,   773,   964,   430,   966,   249,  1666,   739,
     783,   242,   702,   702,   739,   787,  1425,   583,   759,   761,
     247,  1291,   791,  1293,   248,  1189,  -609,   676,   129,   378,
     494,   799,  1769,   801,  1770,   697,   342,   405,   348,  1207,
     819,   702,  1190,   379,   944,   511,   512,   908,   909,   820,
     391,   821,   391,   421,   992,   111, -1022,  1380,   222,   945,
    1382,   481,  1262,   391,  -123,  -880,  1129,   391,  -123,  1370,
     647,   806,   383,   482,   647,  1868,  1627,   431,  1629,   493,
    -880,  1567,   824,   741,   666,  -123,  1004,   431,  1215,   116,
     511,   512, -1022,   391,  1869, -1022,   663,  1870,   391,   694,
     423,   123,   895,   341,  1000,   392,   380,  -919,   665,   667,
    1050,  1051,   782,   947,  1247,  1248,  -721,   786,   384,   888,
     749,   385,   964,   966,   642,   394,   395,   394,   395,  1045,
     966,   386,   165,  1135,   982,  1136,   387,  1137,   394,   395,
     388,  1139,   394,   395,  1342,  1343,   781,   221,   223,   478,
     479,   480,  -919,   481,   416,   389,   583,   380,   751,   380,
     380,   380,   380,   429,   544,   482,  1642,   749,   394,   395,
    1968,   406,   393,   394,   395,   756,  1130,   835,   808,  1574,
    1575,   814,   776,   430,   692,  1046,  1782,    55,   433,   693,
    1787,  1783,  1784,   222,   972,   441,   179,   180,    65,    66,
      67,   391,   222,   583,   622,   436,  1524,   983,   426,   222,
     441,   179,   180,    65,    66,    67,  1409,   811,  1411,  1395,
     391,   222,   445,   391,   795,  1955,  1956,   647,   116,   446,
     647,  1866,  1867,   666,  1862,  1863,   418,  1125,  1126,  1127,
    -601,   991,   486,  1519,   447,   663,  1081,  1084,   870,  1264,
    1265,  1266,  1386,  1128,   448,   449,  1507,   665,   667,  1432,
    1265,  1266,   450,  1396,   592,   594,   596,   442,   599,   836,
    1003,   451,   953,   954,   894,   694,   394,   395,  1929,  1930,
    1931,   962,   442,  1427,   425,   427,   428,   676,   452,  1925,
    -602,  -603,  1940,  -604,   648,   394,   395,   495,   394,   395,
    -605,   484,   485,   487,  1054,  1048,  1958,   517,  -913,   931,
     933,  -606,   418,  1611,   398,  -719,   521,   526,   528,   205,
     482,   260,   431,   534,   537,  -917,   518,   922,   924,   837,
     538,  1470,   441,   179,   180,    65,    66,    67,  -717,   545,
     554,    50,   546,   418,   575,   567,  1841,   222, -1061,   578,
    1844,   579,   601,   602,  1065,   950,   585,   586,   604,   611,
     535,   624,   625,   614,  1034,   615,   543,   668,   669,    55,
     678,   679,   666,   680,  1498,   794,   380,   209,   210,   211,
     212,   213,   682,  -128,   663,   165,   704,   822,  1636,   165,
     792,   691,   796,   642,   797,   803,   665,   667,   804,   182,
    1667,   556,    91,   826,   442,    93,    94,   829,    95,   183,
      97,   843,   838,   844,  1161,   573,  1162,   875,   821,  1549,
     873,   877,   643,   876,   129,   878,   649,  1214,   879,  1164,
     898,  1521,   913,   107,   921,  1558,   883,   880,   990,   915,
     884,   902,   903,  1173,   906,   923,   907,   697,   697,   918,
     934,   939,   920,   643,   747,   649,   643,   649,   649,   925,
     122,  1027,   926,  1040,   927,   928,   940,   942,   124,  1194,
     126,   127,  1122,  1123,  1124,  1125,  1126,  1127,  1917,  1202,
     129,   949,  1203,  -742,  1204,   116,  1938,   948,   702,   951,
     952,  1128,   955,   960,  1376,   608,   959,   123,  1917,  1063,
     116,  1950,   968,   973,   616,   974,   621,  1939,   970,   976,
     979,   628,   985,   986,   988,   475,   476,   477,   478,   479,
     480,   989,   481,   646,   222,   997,  1007,   249,  1005,  1072,
    1009,   242,  1250,   365,   482,  1078,  1008,   981,  1244,  -723,
     247,   116,  1057,  1167,   248,   129,  1644,  1047,   165,  1071,
    1138,   694,   161,   123,  1037,  1650,  1067,  1069,  1076,  1177,
    1624,   400,  1075,  1077,   401,  1079,   129,  1092,  1093,  1657,
    1094,  1095,  1191,  1096,  1251,   676,  1097,  1098,  1143,  1147,
    1149,  1153,  1133,   222,  1145,  1175,  1034,  1150,  1350,  1151,
    1156,  1160,   676,  1163,  1169,   666,  1171,   781,  1172,   814,
    1176,  1211,  1299,  1178,  1180,  1152,   116,   663,  1197,  1206,
    1209,  1212,  1217,  -920,  1218,  1228,  1229,  1230,   123,   665,
     667,   583,  1231,  1232,   222,  1233,   222,   116,  1234,  1237,
    1238,  1239,  1241,   776,  1261,   811,  1253,  1255,  1258,   123,
    1259,  1270,  1272,  1271,  1277,  1798,  1351,  1278,  1128,   628,
    1281,  1282,  1334,  1332,   222,  1352,  1335,  1336,  1337,  1339,
    1346,   129,  1349,   129,  1366,   993,  1367,  1374,  1375,   380,
    1381,  1379,  1383,  1385,  1388,  1269,  1387,  1397,   814,  1273,
     666,  1222,  1222,  1027,  1390,  1391,  1398,   165,  1378,   122,
    1399,  1412,   663,  1018,  1881,  1421,  1419,   124,  1422,   126,
     127,   702,  1439,  1424,   665,   667,  1429,  1442,  1433,  1444,
    1445,  1448,   702,  1352,   811,  1450,  1454,  1459,  1449,  1453,
     116,  1456,   116,  1460,   116,   222,  1457,  1458,  1462,  1420,
    1464,  1465,  1240,  1469,   123,  1471,   123,  1472,  1500,  1511,
    1473,   222,   222,  1474,  1512,  1274,  1514,  1501,  1516,  1533,
     260,  1534,  1528,   441,    63,    64,    65,    66,    67,  1518,
    1434,  1522,  1536,  1529,    72,   488,  1526,  1538,  1543,   583,
    1532,  1545,  1928,  1423,   129,  1527,  1225,  1537,  1279,  1548,
    1544,   161,  1840,  1559,  1369,  1283,   962,  1540,  1550,  1541,
    1542,  1551,  1847,  1552,  1571,  1582,  1555,  1556,   870,  1595,
    1621,  1034,  1034,  1034,  1034,  1034,  1034,   490,  1966,  1610,
    1615,  1034,   676,  1389,  1622,   676,  1392,  1625,  1633,  1661,
    1630,  1631,  1637,  1655,  1652,   442,   971,  1669,  1670,   365,
     365,   365,   598,   365,  1764,   116,  1765,  1882,  1612,  1771,
     441,    63,    64,    65,    66,    67,  1777,   123,  1499,  1778,
    1781,    72,   488,  1780,  1790,  1504,  1792,  1803,  1804,  1505,
    1793,  1506,  1824,  1830,  1833,  1834,  1839,  1856,  1858,  1513,
     129,   650,   444,  1860,  1864,  1440,  1872,  1873,  1874,  1520,
     702,  1191,  1904,   222,   222,  1002,  1879,  1880,  1884,  1887,
    1888,  -353,   489,  1890,   490,  1893,  1891,  1819,  1027,  1027,
    1027,  1027,  1027,  1027,  1895,  1896,  1899,   491,  1027,   492,
    1905,  1906,   442,  1902,   219,   219,  1907,  1912,  1480,   116,
    1919,  1400,  1914,  1923,  1926,  1927,  1041,  1935,  1042,  1951,
    1962,   116,  1937,  1941,  1942,  1961,  1944,  1965,  1973,  1974,
    1983,  1441,  1338,   123,   165,  1984,  1476,  1477,  1986,  1964,
    1987,  1922,  1767,   755,  1969,  1492,  1061,   752,  1208,   165,
      14,  1168,  1497,  1936,  1426,  1492,  1797,  1934,   750,  1034,
     896,  1034,  1497,  1788,  1557,  1812,  1668,  1817,  1602,  1976,
    1451,  1946,  1829,  1583,  1455,  1786,  1292,  1410,  1361,  1461,
    1284,  1225,  1405,   637,  1224,  1405,  1466,  1401,   676,  1402,
     165,  1417,  1242,  1660,  1188,  1082,  1623,  1963,  1978,   702,
    1475,  1897,   630,  1573,  1276,  1341,  1331,     0,   222,     0,
       0,     0,   703,     0,  1481,     0,     0,  1148,     0,  1482,
       0,   441,  1483,   180,    65,    66,    67,  1484,     0,     0,
       0,     0,     0,   628,  1159,     0,  1560,     0,  1561,     0,
   -1062, -1062, -1062, -1062, -1062,   473,   474,   475,   476,   477,
     478,   479,   480,     0,   481,   165,  1027,     0,  1027,   222,
       0,     0,     0,     0,   892,   892,   482,  1648,   129,  1485,
    1486,  1662,  1487,     0,   222,   222,   165,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1605,     0,  1535,     0,
       0,     0,  1539,   442,     0,   495,  1034,     0,  1034,  1546,
    1034,     0,  1488,     0,     0,  1034,  1779,     0,     0,     0,
    1832,     0,     0,     0,     0,  1492,     0,   219,     0,     0,
       0,  1492,  1497,  1492,     0,     0,     0,     0,  1497,   116,
    1497,     0,     0,   676,     0,     0,     0,     0,     0,     0,
     341,   123,     0,     0,     0,  1492,  1600,   129,     0,  1530,
       0,  1531,  1497,  1651,     0,     0,   129,     0,  1795,  1648,
       0,     0,  1604,     0,     0,   222,     0,     0,     0,   165,
       0,   165,     0,   165,     0,  1061,  1257,     0,     0,     0,
       0,     0,   217,   217,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1027,     0,  1027,     0,  1027,     0,  1034,
     216,   216,  1027,     0,     0,     0,     0,     0,   116,     0,
       0,   239,     0,   116,     0,  1827,     0,   116,     0,     0,
     123,     0,  1910,     0,     0,     0,     0,     0,     0,   123,
       0,     0,     0,  1492,     0,   380,     0,   239,   583,     0,
    1497,   341,   129,     0,     0,     0,     0,     0,   129,  1877,
       0,  1753,   219,     0,     0,   129,     0,     0,  1760,   342,
       0,   219,  1835,  1808,     0,   341,     0,   341,   219,     0,
       0,     0,     0,   341,     0,   282,     0,     0,     0,     0,
     219,   444,     0,  1772,   165,     0,  1638,     0,  1639,     0,
    1640,   664,     0,     0,     0,  1641,  1027,     0,     0,     0,
       0,     0,     0,   116,   116,   116,     0,     0,     0,   116,
    1377,     0,   284,     0,     0,   123,   116,     0,     0,     0,
       0,   123,     0,     0,     0,   205,     0,     0,   123,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   892,     0,
     892,   205,   892,   206,    40,     0,   892,    50,   892,   892,
    1142,     0,     0,     0,  1034,  1034,     0,     0,     0,     0,
       0,  1418,     0,    50,     0,  1831,     0,     0,   165,     0,
       0,     0,     0,     0,     0,     0,   628,  1061,     0,     0,
     165,     0,   569,   209,   210,   211,   212,   213,   570,  1791,
       0,     0,     0,     0,     0,   217,   129,     0,     0,   209,
     210,   211,   212,   213,     0,   182,   219,     0,    91,   335,
       0,    93,    94,   216,    95,   183,    97,     0,  1080,     0,
       0,     0,     0,     0,     0,   774,   583,    93,    94,   339,
      95,   183,    97,     0,     0,   344,     0,     0,   129,   107,
     340,     0,     0,     0,     0,   129,  1975,   341,     0,     0,
    1892,  1027,  1027,     0,     0,   107,     0,   116,  1985,   775,
       0,   111,     0,   239,     0,   239,  1851,   628,  1988,   123,
       0,  1989,     0,  1753,  1753,     0,     0,  1760,  1760,     0,
     129,     0,     0,  1480,     0,     0,     0,     0,     0,  1911,
       0,   380,  1857,  1859,     0,     0,     0,     0,     0,   116,
       0,     0,   129,   676,     0,     0,   116,     0,     0,     0,
       0,   123,     0,     0,     0,     0,     0,     0,   123,     0,
       0,   239,     0,   676,     0,    14,   962,     0,     0,     0,
     217,     0,   676,     0,     0,  1480,     0,     0,  1954,   217,
     962,   116,     0,     0,  1845,  1846,   217,     0,   216,  1909,
       0,     0,     0,   123,     0,     0,     0,   216,   217,  1954,
       0,  1979,   129,   116,   216,     0,     0,   129,     0,  1924,
       0,     0,     0,     0,     0,   123,   216,    14,     0,     0,
       0,     0,     0,   219,     0,     0,     0,   216,   165,  1481,
       0,     0,     0,     0,  1482,     0,   441,  1483,   180,    65,
      66,    67,  1484,     0,     0,     0,   892,     0,     0,     0,
       0,     0,   239,     0,     0,   239,     0,  1480,     0,     0,
       0,     0,     0,   116,     0,     0,     0,     0,   116,     0,
       0,     0,  1480,     0,     0,   123,     0,     0,     0,     0,
     123,  1481,   219,     0,  1485,  1486,  1482,  1487,   441,  1483,
     180,    65,    66,    67,  1484,     0,     0,     0,     0,    14,
       0,   344,   239,   344,     0,     0,     0,   165,   442,     0,
       0,     0,   165,     0,    14,     0,   165,  1502,     0,     0,
       0,     0,     0,   219,   217,   219,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1485,  1486,     0,  1487,
       0,     0,   216,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   219,     0,     0,     0,     0,     0,   344,
     442,     0,     0,  1481,  1480,     0,     0,     0,  1482,  1628,
     441,  1483,   180,    65,    66,    67,  1484,     0,  1481,     0,
       0,     0,     0,  1482,     0,   441,  1483,   180,    65,    66,
      67,  1484,     0,     0,   239,     0,   239,     0,     0,   858,
       0,     0,   165,   165,   165,     0,    14,     0,   165,     0,
       0,     0,     0,     0,     0,   165,     0,     0,  1485,  1486,
       0,  1487,     0,  1480,   219,     0,     0,     0,     0,     0,
       0,     0,   858,  1485,  1486,     0,  1487,     0,     0,     0,
     219,   219,   442,     0,     0,     0,     0,     0,     0,     0,
     344,  1632,     0,   344,     0,     0,     0,   442,     0,     0,
       0,     0,     0,     0,     0,    14,  1634,     0,     0,     0,
    1481,     0,     0,     0,   664,  1482,     0,   441,  1483,   180,
      65,    66,    67,  1484,     0,     0,     0,     0,     0,   239,
     239,     0,     0,     0,     0,     0,     0,     0,   239,     0,
       0,   217,     0,     0,     0,     0,   524,   497,   498,   499,
     500,   501,   502,   503,   504,   505,   506,   507,   508,   216,
       0,     0,     0,     0,     0,  1485,  1486,     0,  1487,  1481,
       0,     0,     0,     0,  1482,     0,   441,  1483,   180,    65,
      66,    67,  1484,     0,     0,     0,     0,     0,     0,   442,
       0,   509,   510,     0,     0,     0,   165,     0,  1643,     0,
     217,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,     0,     0,     0,     0,   216,     0,
       0,     0,   219,   219,  1485,  1486,     0,  1487,     0,     0,
       0,     0,   344,     0,   839,     0,     0,     0,   165,     0,
       0,   217,     0,   217,     0,   165,   509,   510,   442,     0,
       0,     0,     0,   239,   205,     0,     0,  1789,     0,   216,
       0,   216,     0,   664,     0,     0,   511,   512,     0,     0,
       0,   217,     0,     0,     0,     0,    50,     0,     0,     0,
     165,     0,     0,     0,     0,     0,     0,     0,     0,   216,
     858,     0,     0,     0,     0,     0,     0,   239,     0,     0,
       0,     0,   165,     0,   239,   239,   858,   858,   858,   858,
     858,     0,   209,   210,   211,   212,   213,     0,   858,     0,
       0,   511,   512,     0,     0,     0,     0,   344,   344,   681,
       0,     0,     0,     0,   239,     0,   344,   398,     0,     0,
      93,    94,   217,    95,   183,    97,     0,     0,     0,     0,
       0,     0,     0,     0,  1102,  1103,  1104,   219,   217,   217,
     216,     0,   165,     0,     0,     0,     0,   165,   107,     0,
       0,     0,   399,     0,   239,  1105,   216,   216,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,
     239,   239,   664,     0,     0,     0,     0,     0,   219,     0,
     216,     0,     0,  1128,     0,     0,   239,     0,     0,     0,
       0,     0,     0,   219,   219,     0,     0,     0,     0,   239,
       0,     0,     0,     0,     0,     0,     0,   858,     0,     0,
     239,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,   239,   481,
       0,     0,   239,     0,     0,     0,     0,     0,     0,     0,
       0,   482,     0,     0,     0,   239,   524,   497,   498,   499,
     500,   501,   502,   503,   504,   505,   506,   507,   508,     0,
       0,     0,     0,     0,     0,  1074,     0,     0,     0,     0,
     217,   217,   344,   344,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   219,     0,     0,     0,   216,   216,
       0,   509,   510,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   239,    34,    35,    36,   239,     0,   239,     0,
       0,  1297,     0,     0,     0,     0,   207,     0,     0,     0,
       0,     0,     0,   858,   858,   858,   858,   858,   858,   216,
       0,     0,   858,   858,   858,   858,   858,   858,   858,   858,
     858,   858,   858,   858,   858,   858,   858,   858,   858,   858,
     858,   858,   858,   858,   858,   858,   858,   858,   858,   858,
       0,     0,     0,     0,     0,     0,   511,   512,   344,     0,
      81,    82,    83,    84,    85,     0,     0,     0,   858,     0,
       0,   214,     0,     0,   344,   836,     0,    89,    90,     0,
       0,     0,     0,     0,     0,   664,     0,   344,     0,     0,
       0,    99,     0,     0,     0,   217,     0,     0,     0,     0,
       0,   239,     0,   239,     0,   104,     0,     0,     0,     0,
     218,   218,     0,   216,     0,     0,   344,     0,     0,   805,
       0,   241,     0,     0,     0,   205,     0,     0,     0,     0,
     239,     0,     0,   239,     0,   837,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   217,    50,     0,     0,
     239,   239,   239,   239,   239,   239,     0,     0,   216,     0,
     239,   217,   217,     0,   216,     0,     0,     0,     0,     0,
     664,     0,     0,     0,     0,     0,     0,     0,     0,   216,
     216,     0,   858,   209,   210,   211,   212,   213,     0,     0,
     344,     0,   239,     0,   344,     0,   839,     0,   239,     0,
       0,   858,     0,   858,     0,   182,     0,     0,    91,     0,
       0,    93,    94,     0,    95,   183,    97,     0,  1275,     0,
       0,     0,     0,     0,     0,     0,     0,   858,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   107,
       0,     0,     0,     0,     0,     0,     0,     0,   453,   454,
     455,     0,   217,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   239,   239,     0,     0,   239,   456,   457,
     216,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,     0,     0,     0,   344,
       0,   344,     0,     0,     0,     0,     0,   482,     0,   453,
     454,   455,     0,     0,     0,     0,     0,     0,   239,     0,
     239,     0,     0,   218,     0,     0,     0,     0,   344,   456,
     457,   344,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,     0,     0,     0,
       0,     0,     0,   239,     0,   239,     0,     0,   482,     0,
       0,   858,     0,   858,     0,   858,     0,     0,     0,     0,
     858,   216,     0,     0,   858,     0,   858,     0,     0,   858,
     344,   453,   454,   455,     0,     0,   344,     0,     0,     0,
       0,     0,   239,   239,     0,     0,   239,     0,     0,     0,
       0,   456,   457,   239,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,     0,
       0,  1354,     0,     0,     0,     0,     0,     0,     0,     0,
     482,     0,     0,     0,     0,   239,     0,   239,   218,   239,
       0,   344,   344,     0,   239,     0,   216,   218,     0,     0,
       0,     0,     0,     0,   218,     0,     0,     0,     0,     0,
     239,     0,     0,   858,     0,     0,   218,     0,     0,     0,
       0,     0,     0,     0,     0,   239,   239,   218,     0,   935,
       0,     0,     0,   239,     0,   239,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,  1125,  1126,  1127,     0,   239,     0,   239,
       0,     0,     0,     0,     0,   239,     0,     0,     0,  1128,
     524,   497,   498,   499,   500,   501,   502,   503,   504,   505,
     506,   507,   508,     0,     0,     0,     0,     0,   239,     0,
       0,   344,     0,   344,     0,     0,     0,     0,     0,     0,
       0,     0,   241,     0,     0,   858,   858,   858,     0,     0,
       0,   967,   858,     0,   239,   509,   510,     0,     0,     0,
     239,     0,   239,   453,   454,   455,     0,     0,     0,     0,
     344,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   344,   218,   456,   457,     0,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   482,     0,     0,     0,     0,     0,     0,     0,
     511,   512,     0,     0,     0,     0,     0,     0,     0,   865,
       0,     0,     0,     0,     0,     0,     0,     0,   344, -1062,
   -1062, -1062, -1062, -1062,  1120,  1121,  1122,  1123,  1124,  1125,
    1126,  1127,   239,     0,     0,     0,     0,     0,     0,     0,
       0,   344,   865,     0,     0,  1128,     0,     0,     0,   239,
       0,     0,     0,   239,   239,     0,     0,     0,     0,     0,
       0,     0,     0,   904,     0,   344,     0,   344,   239,     0,
       0,     0,     0,   344,   858,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   858,     0,     0,     0,     0,
       0,   858,     0,     0,     0,   858,     0,     0,     0,   220,
     220,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     245,   453,   454,   455,     0,     0,     0,   239,     0,     0,
       0,     0,     0,  1006,     0,     0,     0,     0,   344,   218,
       0,   456,   457,     0,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   858,   481,     0,
       0,     0,     0,     0,     0,     0,     0,   239,     0,     0,
     482,   524,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   239,     0,     0,     0,   218,     0,
       0,     0,     0,   239,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   239,     0,   239,     0,     0,
       0,     0,     0,     0,     0,     0,   509,   510,     0,     0,
       0,     0,     0,  1033,     0,     0,   239,     0,   239,   218,
     344,   218,  1219,  1220,  1221,   205,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   344,   282,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,   218,
     865,     0,     0,     0,     0,     0,  1852,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   865,   865,   865,   865,
     865,     0,     0,     0,     0,   284,     0,     0,   865,     0,
       0,   511,   512,   209,   210,   211,   212,   213,   205,     0,
       0,  1010,     0,     0,  1132,     0,     0,   205,     0,     0,
       0,     0,   220,     0,     0,   344,     0,     0,     0,     0,
      50,    93,    94,     0,    95,   183,    97,     0,  -400,    50,
     218,     0,     0,     0,     0,     0,   441,   179,   180,    65,
      66,    67,     0,     0,  1155,     0,   218,   218,     0,   107,
       0,     0,     0,     0,  1598,   569,   209,   210,   211,   212,
     213,   570,     0,     0,     0,   209,   210,   211,   212,   213,
       0,  1155,     0,     0,     0,     0,     0,     0,   182,     0,
     218,    91,   335,     0,    93,    94,     0,    95,   183,    97,
       0,   344,     0,    93,    94,     0,    95,   183,    97,     0,
       0,     0,   339,   344,     0,   344,     0,   865,   442,     0,
    1198,     0,   107,   340,     0,     0,     0,     0,     0,     0,
       0,   107,  1599,     0,   344,     0,   344,     0,     0,     0,
       0,     0,   241,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1033,     0,   220,     0,     0,
       0,     0,     0,     0,     0,     0,   220,     0,     0,     0,
       0,     0,     0,   220,     0,     0,     0,     0,     0,     0,
       0,   453,   454,   455,     0,   220,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   245,     0,   218,   218,
       0,   456,   457,  1435,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,     0,
       0,     0,     0,   865,   865,   865,   865,   865,   865,   218,
     482,     0,   865,   865,   865,   865,   865,   865,   865,   865,
     865,   865,   865,   865,   865,   865,   865,   865,   865,   865,
     865,   865,   865,   865,   865,   865,   865,   865,   865,   865,
       0,   245,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   865,     0,
       0,     0,     0,   867, -1062, -1062, -1062, -1062,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,   220,   481,     0,     0,     0,     0,   453,   454,   455,
       0,     0,     0,     0,   482,     0,   897,     0,     0,     0,
       0,     0,     0,   218,     0,     0,     0,   456,   457,     0,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,     0,   481,     0,     0,     0,   866,  1436,
    1033,  1033,  1033,  1033,  1033,  1033,   482,     0,   218,     0,
    1033,     0,     0,     0,   218,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   218,
     218,   866,   865,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,   865,   481,   865,     0,   453,   454,   455,     0,     0,
       0,     0,     0,     0,   482,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   456,   457,   865,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   482,     0,     0,  1479,   220,     0,
     218,     0,   453,   454,   455,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1146,     0,     0,
       0,     0,   456,   457,     0,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,  1033,   481,
    1033,     0,     0,     0,  1062,     0,     0,   220,     0,     0,
       0,   482,     0,     0,     0,     0,     0,     0,     0,     0,
    1085,  1086,  1087,  1088,  1089,     0,     0,     0,     0,     0,
       0,     0,  1099,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   220,     0,
     220,   865,     0,   865,     0,   865,     0,     0,     0,     0,
     865,   218,     0,     0,   865,     0,   865,   205,     0,   865,
       0,     0,     0,     0,     0,  1205,     0,     0,   220,   866,
     282,     0,     0,  1581,     0,     0,  1594,     0,     0,    50,
       0,     0,     0,     0,     0,   866,   866,   866,   866,   866,
       0,     0,     0,     0,     0,     0,     0,   866,     0,     0,
       0,   205,     0,   206,    40,     0,     0,   284,     0,     0,
       0,     0,     0,     0,     0,   209,   210,   211,   212,   213,
     205,     0,     0,    50,     0,  1033,     0,  1033,     0,  1033,
       0,     0,  1216,     0,  1033,     0,   218,   182,     0,   220,
      91,  1195,    50,    93,    94,     0,    95,   183,    97,     0,
     576,     0,     0,   865,     0,   220,   220,     0,     0,   209,
     210,   211,   212,   213,     0,  1658,  1659,     0,     0,     0,
       0,   107,     0,     0,     0,  1594,  1850,   569,   209,   210,
     211,   212,   213,   570,     0,   774,     0,    93,    94,   245,
      95,   183,    97,     0,     0,     0,     0,     0,     0,     0,
     182,     0,     0,    91,   335,     0,    93,    94,     0,    95,
     183,    97,     0,     0,     0,   107,   866,     0,     0,   810,
       0,   111,     0,     0,   339,     0,     0,     0,  1033,     0,
       0,     0,     0,     0,   107,   340,     0,     0,     0,     0,
       0,   245,     0,     0,     0,   865,   865,   865,     0,     0,
       0,     0,   865,     0,  1806,     0,     0,  1089,  1287,     0,
       0,  1287,  1594,     0,     0,     0,  1300,  1303,  1304,  1305,
    1307,  1308,  1309,  1310,  1311,  1312,  1313,  1314,  1315,  1316,
    1317,  1318,  1319,  1320,  1321,  1322,  1323,  1324,  1325,  1326,
    1327,  1328,  1329,  1330,     0,     0,     0,   220,   220,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1340,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1584,     0,     0,     0,
       0,     0,   866,   866,   866,   866,   866,   866,   245,     0,
       0,   866,   866,   866,   866,   866,   866,   866,   866,   866,
     866,   866,   866,   866,   866,   866,   866,   866,   866,   866,
     866,   866,   866,   866,   866,   866,   866,   866,   866,     0,
       0,     0,     0,     0,     0,     0,   205,     0,     0,     0,
       0,     0,     0,  1033,  1033,     0,     0,   866,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,   865,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   865,     0,     0,     0,     0,
    1585,   865,     0,     0,     0,   865,  1430,     0,     0,     0,
       0,     0,   220,  1586,   209,   210,   211,   212,   213,  1587,
       0,     0,     0,     0,     0,  1446,     0,  1447,     0,     0,
       0,     0,     0,     0,     0,     0,   182,     0,     0,    91,
    1588,     0,    93,    94,     0,    95,  1589,    97,     0,     0,
       0,  1467,     0,     0,     0,     0,     0,   245,     0,     0,
       0,     0,     0,   220,     0,     0,     0,   865,     0,     0,
     107,     0,     0,     0,     0,     0,     0,  1921,   220,   220,
       0,   866,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1581,   205,     0,     0,     0,     0,
     866,     0,   866,     0,   453,   454,   455,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,   456,   457,   866,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,     0,   209,   210,   211,   212,   213,     0,     0,
       0,     0,     0,   482,     0,     0,     0,     0,     0,   220,
       0,     0,     0,     0,     0,   182,     0,     0,    91,    92,
       0,    93,    94,     0,    95,   183,    97,     0,     0,     0,
       0,   453,   454,   455,     0,  1563,     0,  1564,     0,  1565,
       0,     0,     0,     0,  1566,     0,     0,     0,  1568,   107,
    1569,   456,   457,  1570,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     482,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,     0,     0,
     866,     0,   866,     0,   866,     0,     0,     0,     0,   866,
     245,  1128,   205,   866,     0,   866,     0,     0,   866,   453,
     454,   455,     0,     0,  1246,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,  1653,     0,   456,
     457,     0,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,     0,     0,     0,
     209,   210,   211,   212,   213,     0,     0,     0,   482,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   245,  1758,     0,    93,    94,
    1759,    95,   183,    97,     0,     0,     0,     0,     0,     0,
       0,  1617,   866,     0,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,   107,  1599,     0,  1799,
    1800,  1801,     0,     0,     0,     0,  1805,     0,    11,   413,
      13,     0,     0,     0,     0,     0,     0,     0,     0,   757,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,   866,   866,   866,     0,     0,  1618,
       0,   866,     0,     0,    50,     0,     0,     0,     0,     0,
    1811,     0,    55,     0,     0,     0,     0,     0,     0,     0,
     178,   179,   180,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   181,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,   205,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   182,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   183,    97,    50,     0,     0,    99,  1861,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,  1871,
       0,   104,   105,   106,     0,  1876,   107,   108,     0,  1878,
       0,     0,   111,   112,     0,   113,   114,     0,     0,     0,
     209,   210,   211,   212,   213,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,    93,    94,
       0,    95,   183,    97,     0,     0,     0,     0,    11,    12,
      13,     0,     0,   866,     0,     0,     0,     0,     0,     0,
       0,  1913,     0,     0,   866,     0,   107,   704,     0,    14,
     866,    15,    16,     0,   866,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,  1894,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,    56,    57,    58,   866,    59,    60,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,    88,    89,    90,    91,    92,     0,    93,    94,
       0,    95,    96,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,   103,
       0,   104,   105,   106,     0,     0,   107,   108,     0,   109,
     110,  1165,   111,   112,     0,   113,   114,     5,     6,     7,
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
       0,     0,    52,    53,    54,    55,    56,    57,    58,     0,
      59,    60,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,    88,    89,    90,    91,    92,
       0,    93,    94,     0,    95,    96,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,   103,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,  1355,   111,   112,     0,   113,   114,
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
     683,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
       0,   109,   110,  1131,   111,   112,     0,   113,   114,     5,
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
       0,   107,   108,     0,   109,   110,  1179,   111,   112,     0,
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
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1252,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,  1254,    47,     0,
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
       0,    47,     0,    48,     0,    49,  1431,     0,    50,    51,
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
     106,     0,     0,   107,   108,     0,   109,   110,  1572,   111,
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
     110,  1802,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,  1848,    49,     0,     0,    50,    51,     0,
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
       0,     0,   107,   108,     0,   109,   110,  1883,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,  1886,    48,     0,
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
       0,   109,   110,  1903,   111,   112,     0,   113,   114,     5,
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
       0,   107,   108,     0,   109,   110,  1920,   111,   112,     0,
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
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1977,
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
     109,   110,  1980,   111,   112,     0,   113,   114,     5,     6,
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
     107,   108,     0,   109,   110,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
     552,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
      13,     0,     0,   823,     0,     0,     0,     0,     0,     0,
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
       0,    11,    12,    13,     0,     0,  1064,     0,     0,     0,
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
       0,     0,     0,     0,    11,    12,    13,     0,     0,  1647,
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
       0,     0,  1794,     0,     0,     0,     0,     0,     0,     0,
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
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,    11,     0,    13,     0,     0,     0,     0,
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
       0,   107,   184,     0,   349,     0,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
    1105,     0,    10,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,  1124,  1125,  1126,  1127,     0,     0,   698,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1128,    15,
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
     183,    97,     0,   699,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   184,     0,     0,     0,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,   818,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,
    1127,     0,     0,  1192,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1128,    15,    16,     0,     0,     0,
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
      92,     0,    93,    94,     0,    95,   183,    97,     0,  1193,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   184,     0,     0,     0,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,   413,     0,     0,     0,
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
       0,     0,     0,     0,    50,     0,     0,     0,     0,   196,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
     178,   179,   180,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   181,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   182,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   183,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,   483,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   184,     0,     0,
       0,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,   232,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   482,     0,    15,    16,     0,     0,     0,     0,
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
       0,     0,     0,    99,     0,     0,   100,     0,     0,   566,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   184,     0,   267,   454,   455,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,   456,   457,     0,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,   482,     0,    17,     0,    18,    19,
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
     104,   105,   106,     0,     0,   107,   184,     0,   270,     0,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   413,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,   178,   179,   180,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   181,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   182,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   183,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,   568,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   184,   550,     0,     0,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,
    1127,   712,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1128,     0,     0,     0,     0,    15,
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
     105,   106,     0,     0,   107,   184,     0,     0,     0,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,
    1121,  1122,  1123,  1124,  1125,  1126,  1127,     0,     0,     0,
     757,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1128,     0,    15,    16,     0,     0,     0,     0,    17,     0,
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
       7,     8,     9,     0,     0,     0,     0,     0,    10,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,     0,   798,     0,     0,     0,     0,     0,     0,
       0,     0,   482,     0,     0,    15,    16,     0,     0,     0,
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
       0,    10,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,     0,     0,     0,   800,     0,     0,     0,
       0,     0,     0,     0,   482,     0,     0,     0,    15,    16,
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
       0,     0,     0,     0,    10,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1124,  1125,  1126,  1127,     0,     0,     0,     0,     0,  1243,
       0,     0,     0,     0,     0,     0,     0,  1128,     0,     0,
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
       0,   104,   105,   106,     0,     0,   107,   184,   453,   454,
     455,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   456,   457,
       0,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,   482,     0,     0,
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
      99,     0,     0,   100,     0,     0,   587,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     184,   453,   454,   455,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,   827,     0,
      10,   456,   457,     0,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
     482,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,   644,
      39,    40,     0,   828,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,   178,   179,   180,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   181,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   182,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   183,    97,
       0,   272,   273,    99,   274,   275,   100,     0,   276,   277,
     278,   279,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   184,     0,     0,   280,   281,   111,   112,
       0,   113,   114, -1062, -1062, -1062, -1062,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,
       0,     0,     0,     0,     0,   283,     0,     0,     0,     0,
       0,     0,     0,  1128,     0,     0,     0,     0,     0,   285,
     286,   287,   288,   289,   290,   291,     0,     0,     0,   205,
       0,   206,    40,     0,     0,     0,     0,     0,     0,     0,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,    50,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   205,   326,     0,   745,   328,
     329,   330,     0,     0,     0,   331,   580,   209,   210,   211,
     212,   213,   581,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,   272,   273,     0,   274,   275,     0,   582,
     276,   277,   278,   279,     0,    93,    94,     0,    95,   183,
      97,   336,     0,   337,     0,     0,   338,     0,   280,   281,
       0,     0,     0,   209,   210,   211,   212,   213,     0,     0,
       0,     0,     0,   107,     0,     0,     0,   746,     0,   111,
       0,     0,     0,     0,     0,     0,     0,   283,     0,     0,
       0,    93,    94,     0,    95,   183,    97,     0,     0,     0,
       0,   285,   286,   287,   288,   289,   290,   291,     0,     0,
       0,   205,     0,   206,    40,     0,     0,     0,     0,   107,
     981,     0,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,    50,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   205,   326,     0,
     327,   328,   329,   330,     0,     0,     0,   331,   580,   209,
     210,   211,   212,   213,   581,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,   272,   273,     0,   274,   275,
       0,   582,   276,   277,   278,   279,     0,    93,    94,     0,
      95,   183,    97,   336,     0,   337,     0,     0,   338,     0,
     280,   281,     0,   282,     0,   209,   210,   211,   212,   213,
       0,     0,     0,     0,     0,   107,     0,     0,     0,   746,
       0,   111,     0,     0,     0,     0,     0,     0,     0,   283,
       0,   438,     0,    93,    94,     0,    95,   183,    97,     0,
     284,     0,     0,   285,   286,   287,   288,   289,   290,   291,
       0,     0,     0,   205,     0,     0,     0,     0,     0,     0,
       0,   107,     0,     0,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,    50,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,     0,
     326,     0,     0,   328,   329,   330,     0,     0,     0,   331,
     332,   209,   210,   211,   212,   213,   333,     0,     0,     0,
       0,     0,     0,     0,   205,     0,     0,     0,     0,     0,
       0,     0,     0,   334,  1073,     0,    91,   335,     0,    93,
      94,     0,    95,   183,    97,   336,    50,   337,     0,     0,
     338,   272,   273,     0,   274,   275,     0,   339,   276,   277,
     278,   279,     0,     0,     0,     0,     0,   107,   340,     0,
       0,     0,  1773,     0,     0,     0,   280,   281,     0,   282,
       0,     0,   209,   210,   211,   212,   213,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   182,   283,     0,    91,     0,     0,
      93,    94,     0,    95,   183,    97,   284,     0,     0,   285,
     286,   287,   288,   289,   290,   291,     0,     0,     0,   205,
       0,     0,     0,     0,     0,     0,     0,     0,   107,     0,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,    50,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,     0,   326,     0,     0,   328,
     329,   330,     0,     0,     0,   331,   332,   209,   210,   211,
     212,   213,   333,     0,     0,     0,     0,     0,     0,     0,
     205,     0,   929,     0,   930,     0,     0,     0,     0,   334,
       0,     0,    91,   335,     0,    93,    94,     0,    95,   183,
      97,   336,    50,   337,     0,     0,   338,   272,   273,     0,
     274,   275,     0,   339,   276,   277,   278,   279,     0,     0,
       0,     0,     0,   107,   340,     0,     0,     0,  1843,     0,
       0,     0,   280,   281,     0,   282,     0,     0,   209,   210,
     211,   212,   213,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   283,     0,     0,     0,     0,    93,    94,     0,    95,
     183,    97,   284,     0,     0,   285,   286,   287,   288,   289,
     290,   291,     0,     0,     0,   205,     0,     0,     0,     0,
       0,     0,     0,     0,   107,     0,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,    50,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,     0,   326,     0,   327,   328,   329,   330,     0,     0,
       0,   331,   332,   209,   210,   211,   212,   213,   333,     0,
       0,     0,     0,     0,     0,     0,   205,     0,     0,     0,
       0,     0,     0,     0,     0,   334,     0,     0,    91,   335,
       0,    93,    94,     0,    95,   183,    97,   336,    50,   337,
       0,     0,   338,   272,   273,     0,   274,   275,     0,   339,
     276,   277,   278,   279,     0,     0,     0,     0,     0,   107,
     340,     0,     0,     0,     0,     0,     0,     0,   280,   281,
       0,   282,     0,     0,   209,   210,   211,   212,   213,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   283,     0,   359,
       0,     0,    93,    94,     0,    95,   183,    97,   284,     0,
       0,   285,   286,   287,   288,   289,   290,   291,     0,     0,
       0,   205,     0,     0,     0,     0,     0,     0,     0,     0,
     107,     0,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,    50,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,     0,   326,     0,
       0,   328,   329,   330,     0,     0,     0,   331,   332,   209,
     210,   211,   212,   213,   333,     0,     0,     0,     0,     0,
       0,     0,   205,     0,     0,     0,     0,     0,     0,     0,
       0,   334,     0,     0,    91,   335,     0,    93,    94,     0,
      95,   183,    97,   336,    50,   337,     0,     0,   338,     0,
     272,   273,     0,   274,   275,   339,  1576,   276,   277,   278,
     279,     0,     0,     0,     0,   107,   340,     0,     0,     0,
       0,     0,     0,     0,     0,   280,   281,     0,   282,     0,
     209,   210,   211,   212,   213,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   283,   887,     0,     0,    93,    94,
       0,    95,   183,    97,     0,   284,     0,     0,   285,   286,
     287,   288,   289,   290,   291,     0,     0,     0,   205,     0,
       0,     0,     0,     0,     0,     0,   107,     0,     0,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
      50,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,     0,   326,     0,     0,   328,   329,
     330,     0,     0,     0,   331,   332,   209,   210,   211,   212,
     213,   333,     0,     0,     0,     0,     0,     0,     0,   205,
       0,     0,     0,     0,     0,     0,     0,     0,   334,     0,
       0,    91,   335,     0,    93,    94,     0,    95,   183,    97,
     336,    50,   337,     0,     0,   338,  1673,  1674,  1675,  1676,
    1677,     0,   339,  1678,  1679,  1680,  1681,     0,     0,     0,
       0,     0,   107,   340,     0,     0,     0,     0,     0,     0,
    1682,  1683,  1684,     0,     0,     0,     0,   209,   210,   211,
     212,   213,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1685,     0,     0,     0,     0,    93,    94,     0,    95,   183,
      97,     0,     0,     0,  1686,  1687,  1688,  1689,  1690,  1691,
    1692,     0,     0,     0,   205,     0,     0,     0,     0,     0,
       0,     0,     0,   107,     0,  1693,  1694,  1695,  1696,  1697,
    1698,  1699,  1700,  1701,  1702,  1703,    50,  1704,  1705,  1706,
    1707,  1708,  1709,  1710,  1711,  1712,  1713,  1714,  1715,  1716,
    1717,  1718,  1719,  1720,  1721,  1722,  1723,  1724,  1725,  1726,
    1727,  1728,  1729,  1730,  1731,  1732,  1733,     0,     0,     0,
    1734,  1735,   209,   210,   211,   212,   213,     0,  1736,  1737,
    1738,  1739,  1740,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1741,  1742,  1743,     0,   205,     0,
      93,    94,     0,    95,   183,    97,  1744,     0,  1745,  1746,
       0,  1747,     0,     0,     0,     0,     0,     0,  1748,  1749,
      50,  1750,     0,  1751,  1752,     0,   272,   273,   107,   274,
     275,     0,     0,   276,   277,   278,   279,     0,     0,     0,
       0,     0,  1585,     0,     0,     0,     0,     0,     0,     0,
       0,   280,   281,     0,     0,  1586,   209,   210,   211,   212,
     213,  1587,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   182,     0,
     283,    91,    92,     0,    93,    94,     0,    95,  1589,    97,
       0,     0,     0,     0,   285,   286,   287,   288,   289,   290,
     291,     0,     0,     0,   205,     0,     0,     0,     0,     0,
       0,     0,   107,     0,     0,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,    50,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
       0,   326,     0,   327,   328,   329,   330,     0,     0,     0,
     331,   580,   209,   210,   211,   212,   213,   581,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   272,   273,
       0,   274,   275,     0,   582,   276,   277,   278,   279,     0,
      93,    94,     0,    95,   183,    97,   336,     0,   337,     0,
       0,   338,     0,   280,   281,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   107,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   283,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   285,   286,   287,   288,
     289,   290,   291,     0,     0,     0,   205,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,    50,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,     0,   326,     0,  1298,   328,   329,   330,     0,
       0,     0,   331,   580,   209,   210,   211,   212,   213,   581,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     272,   273,     0,   274,   275,     0,   582,   276,   277,   278,
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
     322,   323,   324,   325,     0,   326,     0,     0,   328,   329,
     330,     0,     0,     0,   331,   580,   209,   210,   211,   212,
     213,   581,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   582,     0,
       0,     0,     0,     0,    93,    94,     0,    95,   183,    97,
     336,     0,   337,     0,     0,   338,   453,   454,   455,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   107,     0,     0,     0,   456,   457,     0,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,   453,   454,   455,     0,     0,     0,
       0,     0,     0,     0,     0,   482,     0,     0,     0,     0,
       0,     0,     0,     0,   456,   457,     0,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,   453,   454,   455,     0,     0,     0,     0,     0,
       0,     0,     0,   482,   282,     0,     0,     0,     0,     0,
       0,     0,   456,   457,     0,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
       0,   284,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   482,   282,     0,   205,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,   591,     0,     0,     0,     0,   284,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   205,     0,     0,     0,     0,     0,     0,     0,
       0,   569,   209,   210,   211,   212,   213,   570,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,   790,     0,     0,   182,     0,     0,    91,   335,     0,
      93,    94,     0,    95,   183,    97,     0,  1443,     0,     0,
       0,     0,  1306,     0,     0,     0,     0,     0,   339,   569,
     209,   210,   211,   212,   213,   570,     0,     0,   107,   340,
     845,   846,     0,     0,     0,     0,   847,     0,   848,   815,
       0,     0,   182,     0,     0,    91,   335,     0,    93,    94,
     849,    95,   183,    97,     0,     0,     0,     0,    34,    35,
      36,   205,     0,     0,     0,     0,   339,     0,     0,     0,
       0,   207,  1102,  1103,  1104,     0,   107,   340,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1105,     0,     0,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,   850,   851,
     852,   853,   854,   855,     0,    81,    82,    83,    84,    85,
       0,  1128,     0,     0,     0,     0,   214,  1058,     0,     0,
       0,   182,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   183,    97,     0,     0,     0,    99,     0,     0,     0,
       0,     0,     0,     0,     0,   856,     0,     0,     0,    29,
     104,     0,     0,     0,     0,   107,   857,    34,    35,    36,
     205,     0,   206,    40,     0,     0,     0,     0,     0,     0,
     207,     0,     0,     0,     0,     0,     0,     0,  1280,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   208,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1059,    75,   209,   210,
     211,   212,   213,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
     182,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     183,    97,     0,   845,   846,    99,     0,     0,     0,   847,
       0,   848,     0,     0,     0,     0,     0,     0,     0,   104,
       0,     0,     0,   849,   107,   215,     0,     0,     0,     0,
     111,    34,    35,    36,   205,     0,     0,     0,   453,   454,
     455,     0,     0,     0,   207,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,   456,   457,
       0,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,     0,     0,     0,     0,
       0,   850,   851,   852,   853,   854,   855,   482,    81,    82,
      83,    84,    85,     0,     0,     0,  1012,  1013,     0,   214,
       0,     0,     0,     0,   182,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   183,    97,  1014,     0,     0,    99,
       0,     0,     0,     0,  1015,  1016,  1017,   205,   856,     0,
       0,     0,     0,   104,     0,     0,     0,  1018,   107,   857,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,   527,     0,     0,    29,     0,     0,
       0,     0,     0,     0,     0,    34,    35,    36,   205,     0,
     206,    40,     0,     0,     0,     0,     0,     0,   207,     0,
       0,     0,     0,     0,  1019,  1020,  1021,  1022,  1023,  1024,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1025,     0,     0,     0,   208,   182,     0,     0,
      91,    92,     0,    93,    94,     0,    95,   183,    97,     0,
       0,     0,     0,     0,     0,    75,   209,   210,   211,   212,
     213,  1026,    81,    82,    83,    84,    85,     0,     0,     0,
       0,   107,     0,   214,     0,     0,     0,     0,   182,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   183,    97,
      29,     0,     0,    99,     0,     0,     0,     0,    34,    35,
      36,   205,     0,   206,    40,     0,     0,   104,     0,     0,
       0,   207,   107,   215,     0,     0,   607,     0,   111,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   208,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   627,    75,   209,
     210,   211,   212,   213,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   214,     0,     0,     0,
       0,   182,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   183,    97,    29,  1001,     0,    99,     0,     0,     0,
       0,    34,    35,    36,   205,     0,   206,    40,     0,     0,
     104,     0,     0,     0,   207,   107,   215,     0,     0,     0,
       0,   111,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   208,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,   209,   210,   211,   212,   213,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,   214,
       0,     0,     0,     0,   182,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   183,    97,    29,     0,     0,    99,
       0,     0,     0,     0,    34,    35,    36,   205,     0,   206,
      40,     0,     0,   104,     0,     0,     0,   207,   107,   215,
       0,     0,     0,     0,   111,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   208,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1158,    75,   209,   210,   211,   212,   213,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,   214,     0,     0,     0,     0,   182,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   183,    97,    29,
       0,     0,    99,     0,     0,     0,     0,    34,    35,    36,
     205,     0,   206,    40,     0,     0,   104,     0,     0,     0,
     207,   107,   215,     0,     0,     0,     0,   111,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   208,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,   209,   210,
     211,   212,   213,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
     182,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     183,    97,     0,     0,     0,    99,     0,     0,   453,   454,
     455,     0,     0,     0,     0,     0,     0,     0,     0,   104,
       0,     0,     0,     0,   107,   215,     0,     0,   456,   457,
     111,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,   453,   454,   455,     0,
       0,     0,     0,     0,     0,     0,     0,   482,     0,     0,
       0,     0,     0,     0,     0,     0,   456,   457,     0,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,     0,     0,     0,     0,     0,     0,
       0,     0,   453,   454,   455,   482,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   456,   457,   536,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
     453,   454,   455,     0,     0,     0,     0,     0,     0,     0,
       0,   482,     0,     0,     0,     0,     0,     0,     0,     0,
     456,   457,   919,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,     0,   481,     0,     0,
       0,     0,     0,     0,     0,     0,   453,   454,   455,   482,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   456,   457,   987,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,   453,   454,   455,     0,     0,     0,
       0,     0,     0,     0,     0,   482,     0,     0,     0,     0,
       0,     0,     0,     0,   456,   457,  1043,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,     0,     0,     0,     0,     0,     0,     0,  1102,
    1103,  1104,     0,   482,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1105,     0,  1353,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,  1124,  1125,  1126,  1127,     0,     0,  1102,  1103,  1104,
       0,     0,     0,     0,     0,     0,     0,     0,  1128,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1105,     0,
    1384,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,
    1125,  1126,  1127,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1102,  1103,  1104,     0,  1128,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1105,     0,  1452,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,     0,     0,
    1102,  1103,  1104,     0,     0,     0,     0,     0,     0,     0,
       0,  1128,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1105,     0,  1463,  1106,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,  1125,  1126,  1127,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1102,  1103,  1104,     0,  1128,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1105,     0,  1562,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,
    1127,     0,    34,    35,    36,   205,     0,   206,    40,     0,
       0,     0,     0,     0,  1128,   207,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1654,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   236,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   237,     0,     0,     0,     0,     0,
       0,     0,     0,   209,   210,   211,   212,   213,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
     214,  1656,     0,     0,     0,   182,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   183,    97,     0,     0,     0,
      99,     0,    34,    35,    36,   205,     0,   206,    40,     0,
       0,     0,     0,     0,   104,   658,     0,     0,     0,   107,
     238,     0,     0,     0,     0,   111,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   208,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   209,   210,   211,   212,   213,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
     214,     0,     0,     0,     0,   182,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   183,    97,     0,     0,     0,
      99,     0,    34,    35,    36,   205,     0,   206,    40,     0,
       0,     0,     0,     0,   104,   207,     0,     0,     0,   107,
     659,     0,     0,     0,     0,   111,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   236,     0,     0,   205,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   209,   210,   211,   212,   213,    50,    81,
      82,    83,    84,    85,     0,     0,   356,   357,     0,     0,
     214,   205,     0,     0,     0,   182,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   183,    97,     0,     0,     0,
      99,     0,     0,    50,   209,   210,   211,   212,   213,     0,
       0,   885,   886,     0,   104,     0,     0,     0,     0,   107,
     238,     0,     0,     0,     0,   111,   358,     0,     0,   359,
       0,     0,    93,    94,     0,    95,   183,    97,     0,   209,
     210,   211,   212,   213,     0,     0,     0,     0,     0,     0,
       0,   360,     0,     0,     0,     0,     0,     0,     0,     0,
     107,     0,     0,     0,   887,     0,     0,    93,    94,     0,
      95,   183,    97,     0,     0,     0,   453,   454,   455,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   107,   456,   457,   984,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,   453,   454,   455,     0,     0,     0,
       0,     0,     0,     0,     0,   482,     0,     0,     0,     0,
       0,     0,     0,     0,   456,   457,     0,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,  1102,  1103,  1104,     0,     0,     0,     0,     0,
       0,     0,     0,   482,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1105,  1468,     0,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,  1102,  1103,
    1104,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1128,     0,     0,     0,     0,     0,     0,     0,  1105,
       0,     0,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1124,  1125,  1126,  1127,  1103,  1104,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1128,     0,     0,
       0,     0,     0,     0,  1105,     0,     0,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,   455,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1128,     0,     0,     0,     0,   456,   457,     0,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,  1104,   481,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   482,     0,     0,     0,
       0,     0,  1105,     0,     0,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,
    1121,  1122,  1123,  1124,  1125,  1126,  1127,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   456,   457,
    1128,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   457,   482,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   482,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   482
};

static const yytype_int16 yycheck[] =
{
       5,     6,   161,     8,     9,    10,    11,    12,    13,   129,
      15,    16,    17,    18,    56,    33,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    98,    31,     4,    46,   102,
     103,   675,   672,    51,   186,     4,   108,     4,     4,    44,
     960,   406,  1182,    57,   540,   239,   521,    52,   166,    54,
     552,   161,    57,   406,    59,   128,    30,   701,    56,   671,
      30,  1345,   108,   517,   518,   406,   406,   187,   481,    30,
     949,   754,   513,    30,  1169,   833,   250,   601,   602,    57,
      44,    86,   108,   651,   980,  1057,   588,   826,   802,   513,
       9,    32,   546,     9,    32,    14,   108,    48,    48,     9,
     996,     9,    70,   108,    14,     9,    14,   548,     9,     9,
       9,     9,     4,   115,   356,   357,   358,    48,   360,     4,
       9,    48,    86,     9,   548,     9,     9,    70,    48,     9,
       9,     9,    38,   251,     9,    36,     9,     9,   184,     9,
       9,     9,     0,     9,    83,     9,     9,     9,  1044,    54,
     134,   135,    70,   164,    90,   196,    38,    90,   184,    83,
      56,   106,   107,   160,   160,   166,   134,   135,   160,   215,
     102,   673,   184,     4,    81,    70,   178,    83,   179,   184,
     545,   196,   181,  1787,   181,     4,   191,   198,   199,   215,
      50,    51,   238,   166,    38,    83,    84,   196,   199,    70,
     196,    83,   199,   215,    38,   196,   387,   199,   389,    70,
     215,    70,   238,    70,    70,    70,   200,   391,    70,  1098,
      70,   157,    53,   130,   157,    56,   199,    70,    70,    70,
      70,   199,   164,   238,    70,   174,    83,   181,  1178,    83,
    1844,    70,    73,   193,   111,   889,   197,   197,   253,    83,
     174,   256,   196,   196,    70,   200,   199,   198,   263,   264,
     165,   199,   181,   134,   135,    96,   197,    98,   174,   161,
     197,   102,   103,   193,    70,    70,    70,    70,   196,   198,
     257,   199,   198,   199,   261,   999,  1381,  1259,   198,  1573,
     198,   443,   174,  1388,   198,  1390,   197,   128,   198,   198,
     198,   196,   197,   160,   160,   160,   348,    83,   197,  1067,
     129,  1069,   198,   201,   198,   198,   818,  1213,   198,   198,
     198,   823,   182,   198,  1419,   198,   198,   197,   197,   197,
     174,   197,   378,   197,   197,   197,    83,   977,   199,   196,
     174,   196,   199,   199,   199,    19,    20,   199,   102,   199,
     348,    38,   378,   196,   201,   196,   199,   199,  1237,   199,
       8,   885,   886,   199,   437,   160,   378,   196,   187,   200,
     938,   523,   377,   378,    83,   495,    38,    14,   121,   384,
     385,   386,   387,   388,   389,   102,   282,   130,   284,   394,
      31,   120,   434,   106,   107,    32,    83,    14,   174,    83,
     181,   130,   199,   199,   199,   199,   199,  1481,   413,    50,
     164,   196,    53,   377,    51,   196,   421,   490,   491,   492,
     493,    83,   386,   387,   496,   389,   257,   174,   433,   235,
     261,   181,  1527,    54,   265,   196,   434,    83,    84,  1379,
     915,   134,   135,   102,   340,   196,   196,   164,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   160,   482,   196,   484,
     485,   486,   496,   192,  1167,   102,   103,   200,   196,   487,
     199,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   678,   164,   680,   481,  1582,   514,
     515,   481,   517,   518,   519,   520,  1255,   348,   496,   524,
     481,  1095,   527,  1097,   481,   966,    70,   412,   420,   196,
     689,   536,  1606,   538,  1608,   431,   534,   165,   434,   993,
     554,   546,   966,   196,   165,   134,   135,    50,    51,   554,
      83,   556,    83,   199,   748,   201,   160,  1197,   232,   180,
    1200,    57,  1064,    83,   160,   181,   160,    83,   164,  1181,
      90,   545,   196,    69,    90,    31,  1516,   181,  1518,   689,
     196,  1460,   559,   485,   949,   181,   780,   181,  1001,   420,
     134,   135,   196,    83,    50,   199,   949,    53,    83,   430,
      90,   420,   607,   434,   756,    90,   437,   196,   949,   949,
      75,    76,   514,   659,    75,    76,   160,   519,   196,  1143,
     487,    70,   796,   797,   157,   158,   159,   158,   159,   803,
     804,    70,     4,   875,   707,   877,    70,   879,   158,   159,
      70,   883,   158,   159,   102,   103,   513,    19,    20,    53,
      54,    55,   196,    57,   659,    70,   487,   488,   489,   490,
     491,   492,   493,    32,   858,    69,  1545,   534,   158,   159,
    1954,   196,   157,   158,   159,   494,   870,   573,   545,   132,
     133,   548,   513,   164,   204,   803,  1626,   111,   196,   205,
    1630,   198,   199,   367,   699,   119,   120,   121,   122,   123,
     124,    83,   376,   534,   378,    38,  1389,   712,    90,   383,
     119,   120,   121,   122,   123,   124,  1232,   548,  1234,  1215,
      83,   395,   198,    83,   530,   198,   199,    90,   559,   198,
      90,  1818,  1819,  1098,  1814,  1815,   108,    53,    54,    55,
      70,   746,   199,  1383,   198,  1098,   843,   844,   579,   105,
     106,   107,  1206,    69,   198,   198,  1368,  1098,  1098,   105,
     106,   107,   198,  1217,   356,   357,   358,   191,   360,    31,
     775,   198,   668,   669,   605,   606,   158,   159,   122,   123,
     124,   677,   191,  1258,   112,   113,   114,   672,   198,   198,
      70,    70,  1932,    70,   157,   158,   159,   689,   158,   159,
      70,    70,    70,   160,   822,   810,  1946,   196,   196,   640,
     641,    70,   184,  1496,   164,   160,   196,   198,    49,    81,
      69,   826,   181,   160,   203,   196,   196,   633,   634,    91,
       9,  1333,   119,   120,   121,   122,   123,   124,   160,   160,
       8,   103,   196,   215,   196,   198,  1786,   521,   160,    14,
    1790,   160,   199,     9,   831,   661,   198,   198,   198,    14,
     232,   197,   181,   130,   792,   130,   238,    14,   102,   111,
     197,   197,  1237,   197,  1349,     9,   707,   139,   140,   141,
     142,   143,   197,   196,  1237,   257,   196,    94,  1528,   261,
     196,   202,   197,   157,   197,   197,  1237,  1237,   197,   161,
    1583,     9,   164,   198,   191,   167,   168,    14,   170,   171,
     172,   196,   174,     9,   919,   181,   921,   199,   923,  1421,
     196,   199,   392,   198,   816,   198,   396,  1000,   199,   934,
      83,  1385,   132,   195,     9,  1437,   199,   198,   744,   196,
     198,   197,   197,   948,   197,     9,   198,   843,   844,   197,
      70,    32,   203,   423,   952,   425,   426,   427,   428,   203,
     937,   792,   203,   794,   203,   203,   133,   180,   937,   974,
     937,   937,    50,    51,    52,    53,    54,    55,  1898,   984,
     872,     9,   987,   160,   989,   816,  1926,   136,   993,   197,
     160,    69,    14,     9,  1188,   367,   193,   816,  1918,   830,
     831,  1941,     9,   197,   376,     9,   378,  1927,   182,    14,
     132,   383,   203,   203,   200,    50,    51,    52,    53,    54,
      55,     9,    57,   395,   698,    14,   197,  1001,   203,   835,
     203,  1001,  1050,    60,    69,   841,   197,   196,  1043,   160,
    1001,   872,   102,   939,  1001,   937,  1548,   197,   420,     9,
     881,   882,   937,   872,   792,  1557,   198,   198,   160,   955,
    1514,    88,   136,     9,    91,   197,   958,   196,    70,  1571,
      70,    70,   968,    70,  1051,   960,    70,   196,     9,    14,
     198,   912,   199,   757,   200,   952,  1014,   182,  1160,     9,
     199,    14,   977,   203,   199,  1460,    14,   964,   197,   966,
     198,   997,  1100,   193,    32,   911,   937,  1460,   196,   196,
      32,    14,   196,   196,    14,    52,   196,    70,   937,  1460,
    1460,   952,    70,    70,   798,    70,   800,   958,    70,   196,
     160,     9,   197,   964,    14,   966,   198,   198,   196,   958,
     136,   182,   160,   136,     9,  1647,  1160,   197,    69,   521,
     203,     9,   200,    83,   828,  1160,   200,   200,   200,   198,
       9,  1053,   196,  1055,   136,   196,   198,    14,    83,  1000,
     199,   197,   196,   196,   199,  1071,   197,   136,  1045,  1075,
    1545,  1012,  1013,  1014,   199,   198,   203,   559,  1193,  1166,
       9,   157,  1545,    91,  1838,    32,   199,  1166,    77,  1166,
    1166,  1206,   182,   198,  1545,  1545,   197,   136,   198,    32,
     197,   197,  1217,  1218,  1045,     9,     9,   136,   203,   203,
    1051,   203,  1053,     9,  1055,   899,   203,   203,   197,  1247,
     200,     9,  1038,   197,  1053,   198,  1055,   198,   200,    14,
     198,   915,   916,   198,    83,  1076,   196,   199,   197,   203,
    1255,     9,   196,   119,   120,   121,   122,   123,   124,   197,
    1265,   197,   136,   197,   130,   131,   198,     9,   136,  1100,
     197,     9,  1916,  1250,  1166,   199,  1014,   203,  1084,    32,
     197,  1166,  1784,   136,  1180,  1091,  1182,   203,   198,   203,
     203,   197,  1794,   197,   199,   112,   198,   198,  1129,   169,
      14,  1229,  1230,  1231,  1232,  1233,  1234,   173,  1952,   198,
     165,  1239,  1197,  1209,    83,  1200,  1212,   117,   199,    14,
     197,   197,   136,   136,   197,   191,   698,   181,   199,   356,
     357,   358,   359,   360,   198,  1166,    83,  1839,  1497,    14,
     119,   120,   121,   122,   123,   124,    14,  1166,  1353,    83,
     196,   130,   131,   197,   197,  1360,   136,   198,   198,  1364,
     136,  1366,    14,    14,   198,    14,   199,     9,     9,  1374,
    1262,   398,  1492,   200,    68,  1271,    83,   181,   196,  1384,
    1385,  1277,  1884,  1057,  1058,   757,    83,     9,   199,   198,
     115,   102,   171,   160,   173,   182,   102,    36,  1229,  1230,
    1231,  1232,  1233,  1234,   172,    14,   196,   186,  1239,   188,
     198,   196,   191,   197,    19,    20,   178,   182,     6,  1250,
      83,  1227,   182,   175,   197,     9,   798,    83,   800,   199,
      83,  1262,   198,   197,   197,    14,   195,     9,    14,    83,
      14,  1272,  1143,  1262,   816,    83,  1342,  1343,    14,  1951,
      83,  1907,  1604,   493,  1956,  1347,   828,   490,   994,   831,
      48,   940,  1347,  1923,  1256,  1357,  1646,  1918,   488,  1397,
     609,  1399,  1357,  1633,  1434,  1671,  1584,  1756,  1487,  1963,
    1286,  1939,  1768,  1483,  1290,  1629,  1096,  1233,  1170,  1295,
    1092,  1229,  1230,   388,  1013,  1233,  1302,  1228,  1383,  1229,
     872,  1239,  1040,  1576,   964,   843,  1511,  1950,  1965,  1514,
    1341,  1873,   384,  1475,  1077,  1151,  1129,    -1,  1192,    -1,
      -1,    -1,   434,    -1,   112,    -1,    -1,   899,    -1,   117,
      -1,   119,   120,   121,   122,   123,   124,   125,    -1,    -1,
      -1,    -1,    -1,   915,   916,    -1,  1442,    -1,  1444,    -1,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,   937,  1397,    -1,  1399,  1243,
      -1,    -1,    -1,    -1,   601,   602,    69,  1554,  1470,   167,
     168,  1579,   170,    -1,  1258,  1259,   958,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1492,    -1,  1404,    -1,
      -1,    -1,  1408,   191,    -1,  1497,  1534,    -1,  1536,  1415,
    1538,    -1,   200,    -1,    -1,  1543,  1621,    -1,    -1,    -1,
    1772,    -1,    -1,    -1,    -1,  1517,    -1,   232,    -1,    -1,
      -1,  1523,  1517,  1525,    -1,    -1,    -1,    -1,  1523,  1470,
    1525,    -1,    -1,  1528,    -1,    -1,    -1,    -1,    -1,    -1,
    1481,  1470,    -1,    -1,    -1,  1547,  1487,  1549,    -1,  1397,
      -1,  1399,  1547,  1559,    -1,    -1,  1558,    -1,  1645,  1646,
      -1,    -1,  1491,    -1,    -1,  1349,    -1,    -1,    -1,  1051,
      -1,  1053,    -1,  1055,    -1,  1057,  1058,    -1,    -1,    -1,
      -1,    -1,    19,    20,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1534,    -1,  1536,    -1,  1538,    -1,  1637,
      19,    20,  1543,    -1,    -1,    -1,    -1,    -1,  1549,    -1,
      -1,    30,    -1,  1554,    -1,  1767,    -1,  1558,    -1,    -1,
    1549,    -1,  1891,    -1,    -1,    -1,    -1,    -1,    -1,  1558,
      -1,    -1,    -1,  1635,    -1,  1576,    -1,    56,  1579,    -1,
    1635,  1582,  1644,    -1,    -1,    -1,    -1,    -1,  1650,  1832,
      -1,  1592,   367,    -1,    -1,  1657,    -1,    -1,  1599,  1767,
      -1,   376,  1777,  1669,    -1,  1606,    -1,  1608,   383,    -1,
      -1,    -1,    -1,  1614,    -1,    31,    -1,    -1,    -1,    -1,
     395,  1911,    -1,  1612,  1166,    -1,  1534,    -1,  1536,    -1,
    1538,   406,    -1,    -1,    -1,  1543,  1637,    -1,    -1,    -1,
      -1,    -1,    -1,  1644,  1645,  1646,    -1,    -1,    -1,  1650,
    1192,    -1,    68,    -1,    -1,  1644,  1657,    -1,    -1,    -1,
      -1,  1650,    -1,    -1,    -1,    81,    -1,    -1,  1657,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   875,    -1,
     877,    81,   879,    83,    84,    -1,   883,   103,   885,   886,
     887,    -1,    -1,    -1,  1792,  1793,    -1,    -1,    -1,    -1,
      -1,  1243,    -1,   103,    -1,  1771,    -1,    -1,  1250,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1258,  1259,    -1,    -1,
    1262,    -1,   138,   139,   140,   141,   142,   143,   144,  1637,
      -1,    -1,    -1,    -1,    -1,   232,  1798,    -1,    -1,   139,
     140,   141,   142,   143,    -1,   161,   521,    -1,   164,   165,
      -1,   167,   168,   232,   170,   171,   172,    -1,   174,    -1,
      -1,    -1,    -1,    -1,    -1,   165,  1767,   167,   168,   185,
     170,   171,   172,    -1,    -1,    56,    -1,    -1,  1840,   195,
     196,    -1,    -1,    -1,    -1,  1847,  1961,  1788,    -1,    -1,
    1856,  1792,  1793,    -1,    -1,   195,    -1,  1798,  1973,   199,
      -1,   201,    -1,   282,    -1,   284,  1807,  1349,  1983,  1798,
      -1,  1986,    -1,  1814,  1815,    -1,    -1,  1818,  1819,    -1,
    1882,    -1,    -1,     6,    -1,    -1,    -1,    -1,    -1,  1891,
      -1,  1832,  1808,  1809,    -1,    -1,    -1,    -1,    -1,  1840,
      -1,    -1,  1904,  1898,    -1,    -1,  1847,    -1,    -1,    -1,
      -1,  1840,    -1,    -1,    -1,    -1,    -1,    -1,  1847,    -1,
      -1,   340,    -1,  1918,    -1,    48,  1932,    -1,    -1,    -1,
     367,    -1,  1927,    -1,    -1,     6,    -1,    -1,  1944,   376,
    1946,  1882,    -1,    -1,  1792,  1793,   383,    -1,   367,  1890,
      -1,    -1,    -1,  1882,    -1,    -1,    -1,   376,   395,  1965,
      -1,  1967,  1964,  1904,   383,    -1,    -1,  1969,    -1,  1910,
      -1,    -1,    -1,    -1,    -1,  1904,   395,    48,    -1,    -1,
      -1,    -1,    -1,   698,    -1,    -1,    -1,   406,  1470,   112,
      -1,    -1,    -1,    -1,   117,    -1,   119,   120,   121,   122,
     123,   124,   125,    -1,    -1,    -1,  1143,    -1,    -1,    -1,
      -1,    -1,   431,    -1,    -1,   434,    -1,     6,    -1,    -1,
      -1,    -1,    -1,  1964,    -1,    -1,    -1,    -1,  1969,    -1,
      -1,    -1,     6,    -1,    -1,  1964,    -1,    -1,    -1,    -1,
    1969,   112,   757,    -1,   167,   168,   117,   170,   119,   120,
     121,   122,   123,   124,   125,    -1,    -1,    -1,    -1,    48,
      -1,   282,   481,   284,    -1,    -1,    -1,  1549,   191,    -1,
      -1,    -1,  1554,    -1,    48,    -1,  1558,   200,    -1,    -1,
      -1,    -1,    -1,   798,   521,   800,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   167,   168,    -1,   170,
      -1,    -1,   521,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   828,    -1,    -1,    -1,    -1,    -1,   340,
     191,    -1,    -1,   112,     6,    -1,    -1,    -1,   117,   200,
     119,   120,   121,   122,   123,   124,   125,    -1,   112,    -1,
      -1,    -1,    -1,   117,    -1,   119,   120,   121,   122,   123,
     124,   125,    -1,    -1,   573,    -1,   575,    -1,    -1,   578,
      -1,    -1,  1644,  1645,  1646,    -1,    48,    -1,  1650,    -1,
      -1,    -1,    -1,    -1,    -1,  1657,    -1,    -1,   167,   168,
      -1,   170,    -1,     6,   899,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   611,   167,   168,    -1,   170,    -1,    -1,    -1,
     915,   916,   191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     431,   200,    -1,   434,    -1,    -1,    -1,   191,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,   200,    -1,    -1,    -1,
     112,    -1,    -1,    -1,   949,   117,    -1,   119,   120,   121,
     122,   123,   124,   125,    -1,    -1,    -1,    -1,    -1,   668,
     669,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   677,    -1,
      -1,   698,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,   698,
      -1,    -1,    -1,    -1,    -1,   167,   168,    -1,   170,   112,
      -1,    -1,    -1,    -1,   117,    -1,   119,   120,   121,   122,
     123,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,   191,
      -1,    59,    60,    -1,    -1,    -1,  1798,    -1,   200,    -1,
     757,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    -1,   757,    -1,
      -1,    -1,  1057,  1058,   167,   168,    -1,   170,    -1,    -1,
      -1,    -1,   573,    -1,   575,    -1,    -1,    -1,  1840,    -1,
      -1,   798,    -1,   800,    -1,  1847,    59,    60,   191,    -1,
      -1,    -1,    -1,   792,    81,    -1,    -1,   200,    -1,   798,
      -1,   800,    -1,  1098,    -1,    -1,   134,   135,    -1,    -1,
      -1,   828,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
    1882,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   828,
     829,    -1,    -1,    -1,    -1,    -1,    -1,   836,    -1,    -1,
      -1,    -1,  1904,    -1,   843,   844,   845,   846,   847,   848,
     849,    -1,   139,   140,   141,   142,   143,    -1,   857,    -1,
      -1,   134,   135,    -1,    -1,    -1,    -1,   668,   669,   197,
      -1,    -1,    -1,    -1,   873,    -1,   677,   164,    -1,    -1,
     167,   168,   899,   170,   171,   172,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,  1192,   915,   916,
     899,    -1,  1964,    -1,    -1,    -1,    -1,  1969,   195,    -1,
      -1,    -1,   199,    -1,   913,    31,   915,   916,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
     939,   940,  1237,    -1,    -1,    -1,    -1,    -1,  1243,    -1,
     949,    -1,    -1,    69,    -1,    -1,   955,    -1,    -1,    -1,
      -1,    -1,    -1,  1258,  1259,    -1,    -1,    -1,    -1,   968,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   976,    -1,    -1,
     979,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   997,    57,
      -1,    -1,  1001,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,  1014,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    -1,    -1,    -1,   836,    -1,    -1,    -1,    -1,
    1057,  1058,   843,   844,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1349,    -1,    -1,    -1,  1057,  1058,
      -1,    59,    60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1071,    78,    79,    80,  1075,    -1,  1077,    -1,
      -1,   197,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,  1092,  1093,  1094,  1095,  1096,  1097,  1098,
      -1,    -1,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,   939,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,  1147,    -1,
      -1,   156,    -1,    -1,   955,    31,    -1,   162,   163,    -1,
      -1,    -1,    -1,    -1,    -1,  1460,    -1,   968,    -1,    -1,
      -1,   176,    -1,    -1,    -1,  1192,    -1,    -1,    -1,    -1,
      -1,  1180,    -1,  1182,    -1,   190,    -1,    -1,    -1,    -1,
      19,    20,    -1,  1192,    -1,    -1,   997,    -1,    -1,   197,
      -1,    30,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
    1209,    -1,    -1,  1212,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1243,   103,    -1,    -1,
    1229,  1230,  1231,  1232,  1233,  1234,    -1,    -1,  1237,    -1,
    1239,  1258,  1259,    -1,  1243,    -1,    -1,    -1,    -1,    -1,
    1545,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1258,
    1259,    -1,  1261,   139,   140,   141,   142,   143,    -1,    -1,
    1071,    -1,  1271,    -1,  1075,    -1,  1077,    -1,  1277,    -1,
      -1,  1280,    -1,  1282,    -1,   161,    -1,    -1,   164,    -1,
      -1,   167,   168,    -1,   170,   171,   172,    -1,   174,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1306,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,  1349,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1342,  1343,    -1,    -1,  1346,    30,    31,
    1349,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,  1180,
      -1,  1182,    -1,    -1,    -1,    -1,    -1,    69,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,  1397,    -1,
    1399,    -1,    -1,   232,    -1,    -1,    -1,    -1,  1209,    30,
      31,  1212,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,  1442,    -1,  1444,    -1,    -1,    69,    -1,
      -1,  1450,    -1,  1452,    -1,  1454,    -1,    -1,    -1,    -1,
    1459,  1460,    -1,    -1,  1463,    -1,  1465,    -1,    -1,  1468,
    1271,    10,    11,    12,    -1,    -1,  1277,    -1,    -1,    -1,
      -1,    -1,  1481,  1482,    -1,    -1,  1485,    -1,    -1,    -1,
      -1,    30,    31,  1492,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,  1534,    -1,  1536,   367,  1538,
      -1,  1342,  1343,    -1,  1543,    -1,  1545,   376,    -1,    -1,
      -1,    -1,    -1,    -1,   383,    -1,    -1,    -1,    -1,    -1,
    1559,    -1,    -1,  1562,    -1,    -1,   395,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1574,  1575,   406,    -1,   200,
      -1,    -1,    -1,  1582,    -1,  1584,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,  1606,    -1,  1608,
      -1,    -1,    -1,    -1,    -1,  1614,    -1,    -1,    -1,    69,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,    -1,    -1,  1637,    -1,
      -1,  1442,    -1,  1444,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   481,    -1,    -1,  1654,  1655,  1656,    -1,    -1,
      -1,   200,  1661,    -1,  1663,    59,    60,    -1,    -1,    -1,
    1669,    -1,  1671,    10,    11,    12,    -1,    -1,    -1,    -1,
    1481,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1492,   521,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   578,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1559,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,  1771,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1582,   611,    -1,    -1,    69,    -1,    -1,    -1,  1788,
      -1,    -1,    -1,  1792,  1793,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   197,    -1,  1606,    -1,  1608,  1807,    -1,
      -1,    -1,    -1,  1614,  1813,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1824,    -1,    -1,    -1,    -1,
      -1,  1830,    -1,    -1,    -1,  1834,    -1,    -1,    -1,    19,
      20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    10,    11,    12,    -1,    -1,    -1,  1856,    -1,    -1,
      -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,  1669,   698,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,  1896,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1906,    -1,    -1,
      69,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,  1923,    -1,    -1,    -1,   757,    -1,
      -1,    -1,    -1,  1932,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1944,    -1,  1946,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    -1,    -1,
      -1,    -1,    -1,   792,    -1,    -1,  1965,    -1,  1967,   798,
    1771,   800,    78,    79,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1788,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,   828,
     829,    -1,    -1,    -1,    -1,    -1,  1807,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   845,   846,   847,   848,
     849,    -1,    -1,    -1,    -1,    68,    -1,    -1,   857,    -1,
      -1,   134,   135,   139,   140,   141,   142,   143,    81,    -1,
      -1,   200,    -1,    -1,   873,    -1,    -1,    81,    -1,    -1,
      -1,    -1,   232,    -1,    -1,  1856,    -1,    -1,    -1,    -1,
     103,   167,   168,    -1,   170,   171,   172,    -1,   111,   103,
     899,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,   913,    -1,   915,   916,    -1,   195,
      -1,    -1,    -1,    -1,   128,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,   940,    -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,
     949,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,  1932,    -1,   167,   168,    -1,   170,   171,   172,    -1,
      -1,    -1,   185,  1944,    -1,  1946,    -1,   976,   191,    -1,
     979,    -1,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   195,   196,    -1,  1965,    -1,  1967,    -1,    -1,    -1,
      -1,    -1,  1001,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1014,    -1,   367,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   376,    -1,    -1,    -1,
      -1,    -1,    -1,   383,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,   395,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   406,    -1,  1057,  1058,
      -1,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,  1092,  1093,  1094,  1095,  1096,  1097,  1098,
      69,    -1,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,
      -1,   481,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1147,    -1,
      -1,    -1,    -1,   578,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,   521,    57,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    69,    -1,   611,    -1,    -1,    -1,
      -1,    -1,    -1,  1192,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,   578,   198,
    1229,  1230,  1231,  1232,  1233,  1234,    69,    -1,  1237,    -1,
    1239,    -1,    -1,    -1,  1243,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1258,
    1259,   611,  1261,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,  1280,    57,  1282,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,  1306,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,  1346,   698,    -1,
    1349,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,  1397,    57,
    1399,    -1,    -1,    -1,   829,    -1,    -1,   757,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     845,   846,   847,   848,   849,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   857,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   798,    -1,
     800,  1450,    -1,  1452,    -1,  1454,    -1,    -1,    -1,    -1,
    1459,  1460,    -1,    -1,  1463,    -1,  1465,    81,    -1,  1468,
      -1,    -1,    -1,    -1,    -1,   200,    -1,    -1,   828,   829,
      31,    -1,    -1,  1482,    -1,    -1,  1485,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,   845,   846,   847,   848,   849,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   857,    -1,    -1,
      -1,    81,    -1,    83,    84,    -1,    -1,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      81,    -1,    -1,   103,    -1,  1534,    -1,  1536,    -1,  1538,
      -1,    -1,   200,    -1,  1543,    -1,  1545,   161,    -1,   899,
     164,   976,   103,   167,   168,    -1,   170,   171,   172,    -1,
     111,    -1,    -1,  1562,    -1,   915,   916,    -1,    -1,   139,
     140,   141,   142,   143,    -1,  1574,  1575,    -1,    -1,    -1,
      -1,   195,    -1,    -1,    -1,  1584,   200,   138,   139,   140,
     141,   142,   143,   144,    -1,   165,    -1,   167,   168,   949,
     170,   171,   172,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,   195,   976,    -1,    -1,   199,
      -1,   201,    -1,    -1,   185,    -1,    -1,    -1,  1637,    -1,
      -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,
      -1,  1001,    -1,    -1,    -1,  1654,  1655,  1656,    -1,    -1,
      -1,    -1,  1661,    -1,  1663,    -1,    -1,  1092,  1093,    -1,
      -1,  1096,  1671,    -1,    -1,    -1,  1101,  1102,  1103,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,
    1125,  1126,  1127,  1128,    -1,    -1,    -1,  1057,  1058,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1147,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      -1,    -1,  1092,  1093,  1094,  1095,  1096,  1097,  1098,    -1,
      -1,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,  1792,  1793,    -1,    -1,  1147,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,  1813,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1824,    -1,    -1,    -1,    -1,
     125,  1830,    -1,    -1,    -1,  1834,  1261,    -1,    -1,    -1,
      -1,    -1,  1192,   138,   139,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,    -1,  1280,    -1,  1282,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,  1306,    -1,    -1,    -1,    -1,    -1,  1237,    -1,    -1,
      -1,    -1,    -1,  1243,    -1,    -1,    -1,  1896,    -1,    -1,
     195,    -1,    -1,    -1,    -1,    -1,    -1,  1906,  1258,  1259,
      -1,  1261,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1923,    81,    -1,    -1,    -1,    -1,
    1280,    -1,  1282,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,  1306,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,  1349,
      -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,  1450,    -1,  1452,    -1,  1454,
      -1,    -1,    -1,    -1,  1459,    -1,    -1,    -1,  1463,   195,
    1465,    30,    31,  1468,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
    1450,    -1,  1452,    -1,  1454,    -1,    -1,    -1,    -1,  1459,
    1460,    69,    81,  1463,    -1,  1465,    -1,    -1,  1468,    10,
      11,    12,    -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,  1562,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1545,   165,    -1,   167,   168,
     169,   170,   171,   172,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   200,  1562,    -1,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,   195,   196,    -1,  1654,
    1655,  1656,    -1,    -1,    -1,    -1,  1661,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,  1654,  1655,  1656,    -1,    -1,   200,
      -1,  1661,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
    1670,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    81,    -1,    -1,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,   103,    -1,    -1,   176,  1813,    -1,
     179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,  1824,
      -1,   190,   191,   192,    -1,  1830,   195,   196,    -1,  1834,
      -1,    -1,   201,   202,    -1,   204,   205,    -1,    -1,    -1,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,  1813,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1896,    -1,    -1,  1824,    -1,   195,   196,    -1,    48,
    1830,    50,    51,    -1,  1834,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,  1858,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,    98,
      -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
     109,   110,   111,   112,   113,   114,  1896,   116,   117,   118,
     119,   120,   121,   122,   123,   124,    -1,   126,   127,   128,
     129,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,   153,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,   173,    -1,    -1,   176,    -1,    -1,
     179,    -1,    -1,    -1,    -1,    -1,   185,   186,    -1,   188,
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
      -1,    -1,   108,   109,   110,   111,   112,   113,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    -1,
     126,   127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    91,    92,    93,    94,    95,    96,    -1,
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
      -1,    96,    -1,    98,    -1,   100,   101,    -1,   103,   104,
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
      96,    -1,    98,    99,   100,    -1,    -1,   103,   104,    -1,
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
      -1,    91,    92,    93,    94,    -1,    96,    97,    98,    -1,
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
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    27,    -1,    29,    -1,    -1,    -1,    -1,
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
      -1,   195,   196,    -1,   198,    -1,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      31,    -1,    13,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    50,
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
     171,   172,    -1,   174,    -1,   176,    -1,    -1,   179,    -1,
      -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,
     191,   192,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
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
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
      -1,   199,    -1,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    34,
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
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,   108,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,
     179,    -1,    -1,   198,    -1,    -1,   185,    -1,    -1,    -1,
      -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,    -1,
      -1,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   195,   196,    -1,   198,    11,    12,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,   198,    -1,
      -1,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
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
      -1,    -1,   176,    -1,    -1,   179,    -1,    -1,   198,    -1,
      -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,
      -1,   195,   196,   197,    -1,    -1,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    50,
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
      -1,    -1,    -1,    -1,    -1,    13,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
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
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    50,    51,    -1,    -1,    -1,
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
      -1,    13,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    50,    51,
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
      -1,    -1,    -1,    -1,    13,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
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
     196,    10,    11,    12,    -1,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    27,    -1,
      13,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      69,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,   102,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,     3,     4,   176,     6,     7,   179,    -1,    10,    11,
      12,    13,   185,    -1,    -1,    -1,    -1,   190,   191,   192,
      -1,    -1,   195,   196,    -1,    -1,    28,    29,   201,   202,
      -1,   204,   205,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    81,   128,    -1,   130,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,   161,
      10,    11,    12,    13,    -1,   167,   168,    -1,   170,   171,
     172,   173,    -1,   175,    -1,    -1,   178,    -1,    28,    29,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,   195,    -1,    -1,    -1,   199,    -1,   201,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,   195,
     196,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    81,   128,    -1,
     130,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,   161,    10,    11,    12,    13,    -1,   167,   168,    -1,
     170,   171,   172,   173,    -1,   175,    -1,    -1,   178,    -1,
      28,    29,    -1,    31,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,   199,
      -1,   201,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      -1,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
      68,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   195,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   161,    91,    -1,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,   173,   103,   175,    -1,    -1,
     178,     3,     4,    -1,     6,     7,    -1,   185,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,   195,   196,    -1,
      -1,    -1,   200,    -1,    -1,    -1,    28,    29,    -1,    31,
      -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   161,    57,    -1,   164,    -1,    -1,
     167,   168,    -1,   170,   171,   172,    68,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,    -1,    -1,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    83,    -1,    85,    -1,    -1,    -1,    -1,   161,
      -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,   173,   103,   175,    -1,    -1,   178,     3,     4,    -1,
       6,     7,    -1,   185,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,   195,   196,    -1,    -1,    -1,   200,    -1,
      -1,    -1,    28,    29,    -1,    31,    -1,    -1,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,   167,   168,    -1,   170,
     171,   172,    68,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   195,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,    -1,   130,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,   173,   103,   175,
      -1,    -1,   178,     3,     4,    -1,     6,     7,    -1,   185,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   195,
     196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,
      -1,    31,    -1,    -1,   139,   140,   141,   142,   143,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,   164,
      -1,    -1,   167,   168,    -1,   170,   171,   172,    68,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     195,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,    -1,
      -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,   173,   103,   175,    -1,    -1,   178,    -1,
       3,     4,    -1,     6,     7,   185,   186,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    57,   164,    -1,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    68,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,    -1,    -1,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,
      -1,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
     173,   103,   175,    -1,    -1,   178,     3,     4,     5,     6,
       7,    -1,   185,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   195,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   161,   162,   163,    -1,    81,    -1,
     167,   168,    -1,   170,   171,   172,   173,    -1,   175,   176,
      -1,   178,    -1,    -1,    -1,    -1,    -1,    -1,   185,   186,
     103,   188,    -1,   190,   191,    -1,     3,     4,   195,     6,
       7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    29,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,
      57,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,    -1,   130,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,   161,    10,    11,    12,    13,    -1,
     167,   168,    -1,   170,   171,   172,   173,    -1,   175,    -1,
      -1,   178,    -1,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
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
     123,   124,   125,   126,    -1,   128,    -1,    -1,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,
      -1,    -1,    -1,    -1,   167,   168,    -1,   170,   171,   172,
     173,    -1,   175,    -1,    -1,   178,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    31,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,   197,    -1,    -1,   161,    -1,    -1,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,   174,    -1,    -1,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,   185,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,   195,   196,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,   197,
      -1,    -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,
      70,   170,   171,   172,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,    91,    10,    11,    12,    -1,   195,   196,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    69,    -1,    -1,    -1,    -1,   156,    38,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    70,
     190,    -1,    -1,    -1,    -1,   195,   196,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,
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
     147,   148,   149,    -1,    -1,    -1,    50,    51,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    70,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,   185,    -1,
      -1,    -1,    -1,   190,    -1,    -1,    -1,    91,   195,   196,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   156,    -1,    -1,    -1,   119,   161,    -1,    -1,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,   185,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,   195,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      70,    -1,    -1,   176,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,   190,    -1,    -1,
      -1,    91,   195,   196,    -1,    -1,   199,    -1,   201,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    70,    71,    -1,   176,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
     190,    -1,    -1,    -1,    91,   195,   196,    -1,    -1,    -1,
      -1,   201,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    70,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,   190,    -1,    -1,    -1,    91,   195,   196,
      -1,    -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    70,
      -1,    -1,   176,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,   190,    -1,    -1,    -1,
      91,   195,   196,    -1,    -1,    -1,    -1,   201,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,   176,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   190,
      -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    30,    31,
     201,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,   136,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,   136,    33,    34,    35,    36,    37,    38,    39,
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
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,   136,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
     136,    34,    35,    36,    37,    38,    39,    40,    41,    42,
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
      55,    -1,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,    -1,    -1,    -1,    69,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   136,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   130,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
     156,   136,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
     176,    -1,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,    -1,    -1,    -1,   190,    91,    -1,    -1,    -1,   195,
     196,    -1,    -1,    -1,    -1,   201,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
     176,    -1,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,    -1,    -1,    -1,   190,    91,    -1,    -1,    -1,   195,
     196,    -1,    -1,    -1,    -1,   201,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   103,   145,
     146,   147,   148,   149,    -1,    -1,   111,   112,    -1,    -1,
     156,    81,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
     176,    -1,    -1,   103,   139,   140,   141,   142,   143,    -1,
      -1,   111,   112,    -1,   190,    -1,    -1,    -1,    -1,   195,
     196,    -1,    -1,    -1,    -1,   201,   161,    -1,    -1,   164,
      -1,    -1,   167,   168,    -1,   170,   171,   172,    -1,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     195,    -1,    -1,    -1,   164,    -1,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   195,    30,    31,    32,    33,
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
      -1,    -1,    -1,    -1,    -1,    -1,    31,    69,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69
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
       0,     0,     4,     2,     0,     1,     0,     1,     0,     9,
       0,    10,     0,    11,     0,     9,     0,    10,     0,     8,
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
#line 6906 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 754 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 6914 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 761 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 6920 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 762 "hphp.y" /* yacc.c:1646  */
    { }
#line 6926 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 765 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 6932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 766 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6938 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 767 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6944 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 768 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6950 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 769 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6956 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 770 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 6962 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 771 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 6970 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 6977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 6983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 6995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 779 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 784 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7018 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 789 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7027 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 794 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7034 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 797 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7041 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 800 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 804 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7057 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 808 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7065 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 812 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 816 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7081 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 819 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7088 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 824 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7094 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 825 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7100 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 826 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7106 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 827 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7112 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7118 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 829 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7124 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 830 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7130 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7136 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7142 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7148 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7154 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7160 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7166 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 915 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7172 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 917 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7178 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 922 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7184 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 923 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7191 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 929 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7197 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 933 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7203 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 934 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7209 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 936 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7215 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 938 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7221 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 943 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7227 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 944 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7234 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 950 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7240 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 954 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 956 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7254 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 958 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 963 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7267 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 965 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7273 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 968 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7279 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 970 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7285 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 971 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7291 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 976 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7300 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 983 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7309 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 991 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7316 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 994 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7323 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 1000 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7329 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1001 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7335 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7341 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1005 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7347 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7353 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7359 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1010 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7365 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1014 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7371 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1019 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7377 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1020 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7384 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1022 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7392 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1026 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7399 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1029 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7407 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1033 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7414 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1035 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7422 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7429 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1040 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7437 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7443 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1044 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7449 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1045 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7455 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1046 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7461 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1047 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7467 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1048 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1049 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1050 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1051 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1052 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7497 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1053 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7503 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1054 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7509 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1056 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7521 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1057 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7528 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1061 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1063 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7543 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1068 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7550 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1070 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 7558 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1074 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 7566 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7572 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 7578 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1087 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7584 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1088 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 7590 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1089 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 7596 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1090 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 7605 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1094 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7611 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1095 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7617 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1096 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7623 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1097 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7629 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1098 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7635 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1099 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7641 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1100 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7647 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1101 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7653 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1102 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7659 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1103 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 7665 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1104 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7671 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1105 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 7681 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1113 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);}
#line 7687 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1114 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7693 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1123 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 7699 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1124 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7705 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1128 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7712 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1130 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7720 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1136 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7726 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1137 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7732 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1141 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 7738 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1142 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7744 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1146 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 7750 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7759 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1158 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7768 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1165 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7777 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1171 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7786 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1178 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1184 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7804 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1192 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7811 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1196 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 7817 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1200 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7824 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1204 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 7830 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 1210 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7837 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 1213 "hphp.y" /* yacc.c:1646  */
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
#line 7855 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 1228 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7862 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1231 "hphp.y" /* yacc.c:1646  */
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
#line 7880 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1245 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1248 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1253 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7902 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1256 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7910 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1262 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 7916 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1265 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 7922 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1269 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1272 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7940 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1280 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1283 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7958 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1291 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7964 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1292 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 7971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1296 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1299 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1302 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 7989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1303 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 7995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1304 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 8003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1307 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 8009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1308 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 8015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1312 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1313 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8027 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1316 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8033 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1317 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8039 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1320 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8045 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1321 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8051 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1324 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8057 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1326 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8063 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1329 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8069 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1331 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8075 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1335 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8081 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1336 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8087 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1339 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8093 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1340 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1341 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8105 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1345 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8111 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1347 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1350 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1352 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1355 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1360 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8147 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1362 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8153 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1366 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8159 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1368 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8166 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1373 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8172 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1374 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8178 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1375 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8184 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1376 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8190 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1381 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8196 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1383 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8202 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1384 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8208 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1387 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8214 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1388 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1393 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1394 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1399 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1400 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8244 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1403 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8250 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1404 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8256 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1407 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8262 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1408 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8268 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1416 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8275 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1422 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8282 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1428 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8290 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1432 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8296 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1436 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8303 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1441 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1446 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8318 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1449 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1455 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1459 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8338 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1464 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1469 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8352 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1474 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8359 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1479 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8366 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1485 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1491 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8380 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1499 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8387 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1504 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8394 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1509 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8402 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1513 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8408 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1516 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1520 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 8422 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1524 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 8430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1527 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1532 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8443 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1535 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8450 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1539 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8457 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1543 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8464 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1547 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8471 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1551 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8478 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1556 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1561 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8492 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1567 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8498 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1568 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8504 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1571 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,false);}
#line 8510 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1572 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),true,false);}
#line 8516 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1573 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,true);}
#line 8522 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1575 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),false, false);}
#line 8528 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1577 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),false,true);}
#line 8534 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1579 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),true, false);}
#line 8540 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1583 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8546 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1584 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 8552 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1587 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8558 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1588 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8564 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1589 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 8570 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1593 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 8576 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1595 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 8582 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1596 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 8588 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1597 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 8594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1602 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8600 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1603 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8606 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1606 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1611 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 8619 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1617 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8625 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1618 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8631 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1621 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 8637 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1622 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 8644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1625 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 8650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1626 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 8657 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1628 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8664 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1631 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 8671 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1633 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8677 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1636 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8685 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1643 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8694 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1651 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8702 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1658 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8711 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1663 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 8717 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1665 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8723 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1667 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8729 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1669 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 8735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1671 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 8741 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1672 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 8748 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1675 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 8754 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1678 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8760 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1679 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8766 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1680 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8772 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1686 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 8778 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1691 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 8785 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1694 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 8793 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1701 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 8799 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1702 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 8806 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1707 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 8813 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1710 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 8819 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1717 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 8826 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8832 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8838 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1728 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8844 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1730 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1732 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8856 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1733 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 8867 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1739 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 8873 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1741 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 8879 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1742 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 8885 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1746 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,0,(yyvsp[0]),0);}
#line 8891 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1748 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),0,(yyvsp[0]),0);}
#line 8897 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1753 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8903 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1756 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8909 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1757 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 8915 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1761 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 8921 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1762 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 8927 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1766 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 8934 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1769 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 8941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1774 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 8948 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1779 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 8954 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1780 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 8961 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1782 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 8967 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1786 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 8973 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1787 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 8979 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1788 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 8985 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1789 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 8991 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1793 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8997 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1794 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 9003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1795 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 9009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1796 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 9015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1797 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 9021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1799 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 9027 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1801 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 9033 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1805 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9041 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1808 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9047 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1809 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9053 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1813 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9059 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1814 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9065 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1818 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9071 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1819 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1822 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9083 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1823 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9089 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1826 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9095 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1827 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9101 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1830 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9107 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1832 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9113 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9119 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9125 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9131 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9137 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9143 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1840 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9155 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1845 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9161 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9167 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1849 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9173 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1850 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9179 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1851 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9185 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1855 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9191 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1857 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9197 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1858 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9203 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1859 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9209 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1863 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9215 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1865 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9221 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1869 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9227 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1871 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9233 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1875 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1879 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9248 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1883 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9254 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1887 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9260 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1889 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9266 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1890 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9272 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1891 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9278 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1892 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9284 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1893 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9290 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1896 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9296 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1900 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9302 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1901 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9308 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1905 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9314 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1906 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9320 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 1910 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9326 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 1911 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 1912 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9338 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 1913 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9344 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 1917 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9350 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 1922 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9356 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 1926 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 9362 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 1930 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9368 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 1934 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 9374 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 1938 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9380 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 1943 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9386 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 1947 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9392 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 1948 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9398 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 1949 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9404 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9410 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 1951 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9416 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 9422 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 9428 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 1958 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 9434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 1961 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 9440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 9446 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 9452 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 9458 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 9464 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 1966 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 9470 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 1967 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 9476 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 1968 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 9482 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 1969 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 9488 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 1970 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 9494 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 1971 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 9500 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 1972 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 9506 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 9512 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 1974 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 9518 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 1975 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 9524 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 9530 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 1977 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 9536 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 1978 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 9542 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 1979 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 9548 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 1980 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 9554 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 1981 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 9560 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 9566 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 1983 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 9572 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 1984 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 9578 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 1985 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 9584 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 9590 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 1987 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 9596 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 1988 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 9602 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 1989 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 9608 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 1990 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 9614 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 9620 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 1992 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 9626 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 1993 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 9632 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 9638 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 1995 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 9644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 1996 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 9650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 1997 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 9656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 1998 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 9662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 9668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 9674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2001 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 9680 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2002 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 9686 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 9692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 9698 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 9705 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 9711 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2008 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 9718 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2010 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 9724 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2012 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 9730 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9736 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 9742 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 9748 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2016 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 9754 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9760 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 9766 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2019 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 9772 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2020 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 9778 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2021 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 9784 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2022 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 9790 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2023 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 9796 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2024 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 9802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2025 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 9808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2026 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 9814 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9820 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2028 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9826 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2029 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9832 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2030 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9838 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2031 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9844 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2032 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2033 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9856 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2034 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9862 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2035 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 9868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2036 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 9874 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2037 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9880 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2044 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 9886 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2045 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2050 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9901 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2056 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9912 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2064 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9921 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2070 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2080 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 9945 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2088 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         (yyvsp[-3]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-3]), nullptr, (yyvsp[-3]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-3]),
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2098 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9969 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2106 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         (yyvsp[-6]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-6]), nullptr, (yyvsp[-6]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-6]),
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2116 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9993 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2122 "hphp.y" /* yacc.c:1646  */
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
#line 10009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2133 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 10022 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2141 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10034 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10044 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2156 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10056 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2166 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10062 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2167 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10068 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2169 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10074 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2173 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10081 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2175 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10087 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2182 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10093 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2185 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2192 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10105 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2195 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10111 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2200 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2201 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2206 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2207 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2211 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_ARRAY);}
#line 10141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2215 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10147 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2216 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10153 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2221 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10159 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2222 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10165 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2227 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10171 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2228 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10177 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2233 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10183 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2234 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10189 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2240 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10195 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2242 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10201 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2247 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10207 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2248 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10213 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2254 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10219 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2256 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10225 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2260 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10231 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2264 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10237 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2268 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10243 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2272 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10249 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2276 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2280 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2284 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10267 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2288 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10273 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2292 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10279 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2296 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10285 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2300 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10291 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2304 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10297 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2308 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10303 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2312 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10309 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2316 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10315 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2321 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10321 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2322 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10327 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2327 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10333 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2328 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2333 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2334 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10351 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2339 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10359 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2346 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10367 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2353 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2355 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10379 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2359 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10385 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2360 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10391 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2361 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10397 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2362 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10403 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2363 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10409 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2364 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2365 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10421 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2366 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10427 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2367 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2369 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2370 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10446 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2374 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10452 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2375 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 10458 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2376 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 10464 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2377 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 10470 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2384 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 10476 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2387 "hphp.y" /* yacc.c:1646  */
    { Token t1; _p->onArray(t1,(yyvsp[-1]));
                                         Token t2; _p->onArray(t2,(yyvsp[0]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[-1]),NULL,t1,0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-1]),t2,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),file,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),line,0,0);
                                         (yyval).setText("");}
#line 10490 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2398 "hphp.y" /* yacc.c:1646  */
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray((yyvsp[-2]),(yyvsp[-5]));
                                         _p->onArray((yyvsp[-1]),(yyvsp[-3]));
                                         _p->onCallParam((yyvsp[-4]),NULL,(yyvsp[-2]),0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-4]),(yyvsp[-1]),0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),file,0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),line,0,0);
                                         (yyval).setText((yyvsp[0]).text());}
#line 10504 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2409 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 10510 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2410 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 10516 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2415 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10522 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2416 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10528 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2419 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),0,(yyvsp[0]),0);}
#line 10534 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2420 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10540 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2423 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10547 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2427 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10555 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2430 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10561 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2433 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 10573 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2440 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10579 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2441 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10585 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2445 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10591 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2447 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 10597 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 10603 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10609 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10615 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2455 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10621 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2456 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10627 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2457 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10633 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2458 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10639 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10645 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2460 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10651 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10657 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2462 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10663 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2463 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10669 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10675 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2465 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10681 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2466 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10687 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2467 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10693 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2468 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10699 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2469 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10705 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2470 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10711 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2471 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10717 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2472 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10723 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2473 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10729 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2474 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2475 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10741 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2476 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10747 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2477 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10753 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2478 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10759 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2479 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10765 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2480 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2481 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10777 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2482 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10783 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2484 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10801 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10807 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10813 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10819 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10831 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2491 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10837 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2492 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10843 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2493 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10849 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2494 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10855 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2495 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10861 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2496 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10867 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2497 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10873 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2498 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10879 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2499 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10885 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2500 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10891 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2501 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10897 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10903 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2503 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10909 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2504 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10915 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2505 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10921 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2506 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10927 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2507 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10933 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2508 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10939 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2509 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10945 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2510 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10951 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2511 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10957 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2512 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10963 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2513 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10969 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2514 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10975 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2515 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10981 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2516 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10987 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10993 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2518 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10999 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2519 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11005 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2520 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11011 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2521 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11017 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2522 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11023 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2523 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11029 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2524 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11035 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2525 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11041 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2526 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11047 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2527 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11053 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2528 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11059 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2529 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11065 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2530 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11071 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2531 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2532 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11083 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2533 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11089 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2538 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11095 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2542 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11101 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2543 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11107 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2547 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11113 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2548 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11119 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2549 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11125 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2550 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11132 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2552 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2556 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11145 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2565 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11151 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2568 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2569 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11164 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2571 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11171 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2581 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11177 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2585 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11183 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2586 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11189 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2587 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11195 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2591 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11201 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2592 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11207 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2593 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11213 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2597 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11219 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2598 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11225 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2599 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11231 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2603 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11237 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2604 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11243 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2608 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11249 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2609 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2610 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2611 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11268 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2613 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11274 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2614 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11280 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2615 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 11286 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2616 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 11292 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2617 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 11298 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2618 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 11304 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2619 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 11310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2620 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 11316 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2621 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 11322 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2624 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11328 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2626 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11334 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2630 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11340 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2631 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11346 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2633 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11352 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2634 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11358 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11364 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2637 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11370 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2638 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11376 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11382 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2640 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11388 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2641 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11394 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2643 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2644 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 11418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2648 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 11424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 11430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2652 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 11436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2654 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 11442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2655 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 11448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2656 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 11454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2657 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 11460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2658 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 11466 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 11472 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2660 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 11478 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2661 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 11484 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2662 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 11490 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 11496 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 11502 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2665 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 11508 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2666 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 11514 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2667 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 11520 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 11526 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2669 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11532 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11538 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2672 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 11544 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2674 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 11550 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2676 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 11556 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2678 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 11562 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2679 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 11568 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2681 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 11575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2683 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 11581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 11588 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2690 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 11594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2693 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 11600 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2694 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 11606 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2698 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11612 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2699 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11618 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2705 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11624 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2711 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11630 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11636 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2716 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11642 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2717 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11648 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11654 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11660 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 11666 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 11672 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2723 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 11679 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2728 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11685 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11691 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2733 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11697 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11703 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2737 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 11709 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11715 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2744 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11721 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2746 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11727 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2748 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2749 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2753 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11745 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2754 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11751 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2755 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11757 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2758 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11763 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2760 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11769 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2763 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 11775 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2764 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 11781 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2765 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 11787 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2766 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11793 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2770 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 11800 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2773 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 11808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2780 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11814 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2781 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11820 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2784 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 11828 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2787 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11834 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2788 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11840 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2789 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11846 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2791 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11852 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2792 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11858 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2794 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11864 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2795 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11870 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2796 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11876 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2797 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11882 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2798 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11888 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2799 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11894 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2804 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11900 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2805 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11906 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2810 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11912 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2811 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11918 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2816 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11924 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2818 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11930 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2820 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11936 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2821 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11942 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2825 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11948 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2826 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11954 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11960 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2832 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 11966 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2837 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11972 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2840 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11978 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2845 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11984 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2846 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11990 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11996 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2850 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 12003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2857 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 12009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2859 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 12015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2862 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 12021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2864 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12027 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12033 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2870 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12039 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2871 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12045 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2875 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12051 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2876 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12057 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2880 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12063 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2881 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12069 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2882 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12075 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2886 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12081 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2891 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12087 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2896 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12093 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2897 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2901 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12105 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2906 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12111 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 2911 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 2912 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 2916 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 2918 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 2923 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 2925 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12147 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 2931 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12161 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 2942 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 2957 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12189 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 2969 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12203 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 2981 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12209 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 2982 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12215 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 2983 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12221 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 2984 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12227 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 2985 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12233 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 2986 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12239 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 2988 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3005 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12259 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3007 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3009 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3010 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3014 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3015 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3016 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3025 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12315 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3034 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12321 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3036 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12327 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3037 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12333 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3041 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3046 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3047 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12351 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3048 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12357 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3049 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12363 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3050 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12369 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3051 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12375 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3052 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12381 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3054 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12387 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3056 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12393 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3060 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12399 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3064 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12405 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3065 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12411 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3071 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12417 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3075 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12423 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3082 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 12429 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3091 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 12435 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3095 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 12441 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3099 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12447 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3108 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12453 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3109 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12459 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3110 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12465 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3114 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12471 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3115 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 12477 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3116 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 12483 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3118 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]) = 1; _p->onIndirectRef((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12489 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3123 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12495 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3124 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12501 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3135 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12507 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3136 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12513 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3137 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12519 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3140 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12533 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3151 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3152 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12545 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3156 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3157 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12557 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3160 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12571 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3169 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12577 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3173 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 12583 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3174 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 12589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3176 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 12595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3177 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 12601 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3178 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 12607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3179 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 12613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3184 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12619 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3185 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12625 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3189 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12631 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3190 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12637 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3191 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12643 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3192 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12649 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3195 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12655 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3197 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 12661 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3198 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12667 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3199 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 12673 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3204 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12679 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3205 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12685 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3209 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12691 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3210 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12697 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3211 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12703 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3212 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12709 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3217 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12715 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3218 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12721 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3223 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12727 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3225 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3227 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3228 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12745 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3232 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 12751 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3234 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 12757 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3235 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 12763 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3237 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 12770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3242 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3244 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3246 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 12796 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3256 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 12802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3258 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 12808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3259 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12814 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3262 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 12820 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3263 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 12826 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3264 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 12832 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3268 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 12838 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3269 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 12844 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3270 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3271 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12856 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3272 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12862 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3273 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3274 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 12874 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3275 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 12880 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3276 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 12886 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3277 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 12892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3278 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 12898 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3282 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12904 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3283 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12910 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3288 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12916 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3290 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12922 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3304 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12930 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3309 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 12938 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3313 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12946 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3318 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 12954 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3324 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12960 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3325 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12966 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3329 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12972 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3330 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12978 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3336 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12984 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3340 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 12990 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3346 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12996 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3350 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 13003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3358 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3362 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 13023 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3365 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 13030 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3371 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13036 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3376 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]); }
#line 13042 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3377 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13048 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3378 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13054 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3379 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13060 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1036:
#line 3400 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13066 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3401 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13072 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3410 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13078 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3421 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13084 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3423 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13090 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3427 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13096 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3430 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13102 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3434 "hphp.y" /* yacc.c:1646  */
    {}
#line 13108 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3435 "hphp.y" /* yacc.c:1646  */
    {}
#line 13114 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3436 "hphp.y" /* yacc.c:1646  */
    {}
#line 13120 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3442 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3447 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13137 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3456 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13143 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3462 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13152 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3470 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13158 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3471 "hphp.y" /* yacc.c:1646  */
    { }
#line 13164 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3477 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13170 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3479 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13176 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3480 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13186 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3485 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3491 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("array"); }
#line 13200 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3496 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13206 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3501 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13214 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3505 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3510 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 13226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3512 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 13232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3518 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 13239 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3520 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 13247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3523 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3524 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3527 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13269 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3530 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13275 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3533 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 13283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3536 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13290 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3538 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 13299 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3544 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 13308 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3550 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 13318 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1077:
#line 3558 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1078:
#line 3559 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13330 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;


#line 13334 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 3562 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}
