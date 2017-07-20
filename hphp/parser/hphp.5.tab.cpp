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



int Compiler5parse (HPHP::HPHP_PARSER_NS::Parser *_p);

#endif /* !YY_YY_HPHP_Y_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 897 "hphp.5.tab.cpp" /* yacc.c:358  */

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
#define YYLAST   18392

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  206
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  302
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1079
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
    2870,  2871,  2875,  2876,  2880,  2881,  2882,  2886,  2888,  2896,
    2897,  2901,  2903,  2911,  2912,  2916,  2917,  2922,  2924,  2929,
    2940,  2954,  2966,  2981,  2982,  2983,  2984,  2985,  2986,  2987,
    2997,  3006,  3008,  3010,  3014,  3015,  3016,  3017,  3018,  3034,
    3035,  3037,  3046,  3047,  3048,  3049,  3050,  3051,  3052,  3053,
    3055,  3060,  3064,  3065,  3069,  3072,  3079,  3083,  3092,  3099,
    3101,  3107,  3109,  3110,  3114,  3115,  3116,  3123,  3124,  3129,
    3130,  3135,  3136,  3137,  3138,  3149,  3152,  3155,  3156,  3157,
    3158,  3169,  3173,  3174,  3175,  3177,  3178,  3179,  3183,  3185,
    3188,  3190,  3191,  3192,  3193,  3196,  3198,  3199,  3203,  3205,
    3208,  3210,  3211,  3212,  3216,  3218,  3221,  3224,  3226,  3228,
    3232,  3233,  3235,  3236,  3242,  3243,  3245,  3255,  3257,  3259,
    3262,  3263,  3264,  3268,  3269,  3270,  3271,  3272,  3273,  3274,
    3275,  3276,  3277,  3278,  3282,  3283,  3287,  3289,  3297,  3299,
    3303,  3307,  3312,  3316,  3324,  3325,  3329,  3330,  3336,  3337,
    3346,  3347,  3355,  3358,  3362,  3365,  3370,  3375,  3377,  3378,
    3379,  3382,  3384,  3390,  3391,  3395,  3396,  3400,  3401,  3405,
    3406,  3409,  3414,  3415,  3419,  3422,  3424,  3428,  3434,  3435,
    3436,  3440,  3444,  3452,  3457,  3469,  3471,  3475,  3478,  3480,
    3485,  3490,  3496,  3499,  3504,  3509,  3511,  3518,  3520,  3523,
    3524,  3527,  3530,  3531,  3536,  3538,  3542,  3548,  3558,  3559
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
  "simple_indirect_reference", "variable_no_calls",
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
     426,   427,   428,   429,   430,   431,    40,    41,    59,   123,
     125,    36,    96,    93,    34,    39
};
# endif

#define YYPACT_NINF -1625

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1625)))

#define YYTABLE_NINF -1063

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1063)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1625,   188, -1625, -1625,  5774, 13691, 13691,     6, 13691, 13691,
   13691, 13691, 11458, 13691, -1625, 13691, 13691, 13691, 13691, 16865,
   16865, 13691, 13691, 13691, 13691, 13691, 13691, 13691, 13691, 11661,
   17530, 13691,   203,   210, -1625, -1625, -1625,   123, -1625,   421,
   -1625, -1625, -1625,   126, 13691, -1625,   210,   323,   346,   386,
   -1625,   210, 11864, 15145, 12067, -1625, 14721, 10443,   326, 13691,
   17771,   159,    72,   524,   282, -1625, -1625, -1625,   414,   422,
     439,   442, -1625, 15145,   445,   455,   621,   635,   637,   647,
     649, -1625, -1625, -1625, -1625, -1625, 13691,   516,  1323, -1625,
   -1625, 15145, -1625, -1625, -1625, -1625, 15145, -1625, 15145, -1625,
     523,   539, 15145, 15145, -1625,   219, -1625, -1625, 12270, -1625,
   -1625,   538,   544,   553,   553, -1625,   709,   579,     2,   555,
   -1625,    97, -1625,   711, -1625, -1625, -1625, -1625, 14263,   694,
   -1625, -1625,   561,   564,   577,   585,   590,   591,   593,   594,
   11848, -1625, -1625, -1625, -1625,    60,   683,   729,   730,   731,
     732, -1625,   733,   734, -1625,   157,   609, -1625,   646,    13,
   -1625,  1350,   145, -1625, -1625,  3062,    89,   625,   124, -1625,
     149,   136,   626,   187, -1625,    67, -1625,   755, -1625,   667,
   -1625, -1625,   633,   666, -1625, 13691, -1625,   711,   694, 17985,
    3369, 17985, 13691, 17985, 17985, 18249, 18249,   634, 16182, 17985,
     782, 15145,   764,   764,   347,   764, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625, -1625,    48, 13691,   655, -1625, -1625,
     677,   642,   427,   643,   427,   764,   764,   764,   764,   764,
     764,   764,   764, 16865, 16425,   641,   831,   667, -1625, 13691,
     655, -1625,   686, -1625,   688,   653, -1625,   151, -1625, -1625,
   -1625,   427,    89, -1625, 12473, -1625, -1625, 13691,  9225,   842,
      99, 17985, 10240, -1625, 13691, 13691, 15145, -1625, -1625, 12457,
     654, -1625, 13675, -1625, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625,  3186, -1625,  3186, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625,    93,    91,   666, -1625, -1625, -1625, -1625,
     660,  3042,   100, -1625, -1625,   698,   849, -1625,   706, 15440,
   -1625,   669,   670, 15913, -1625,    38, 15961, 14792, 14792, 14792,
   15145, 14792,   678,   860,   680, -1625,   102, -1625, 16453,   101,
   -1625,   862,   103,   749, -1625,   751, -1625, 16865, 13691, 13691,
     685,   702, -1625, -1625, 16556, 11661, 13691, 13691, 13691, 13691,
   13691,   106,   461,   463, -1625, 13894, 16865,   522, -1625, 15145,
   -1625,   202,   579, -1625, -1625, -1625, -1625, 17630,   870,   786,
   -1625, -1625, -1625,    88, 13691,   692,   699, 17985,   700,  2102,
     703,  5977, 13691,   444,   697,   570,   444,   491,   467, -1625,
   15145,  3186,   705, 10646, 14721, -1625, -1625,  2972, -1625, -1625,
   -1625, -1625, -1625,   711, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, 13691, 13691, 13691, 13691, 12676, 13691, 13691,
   13691, 13691, 13691, 13691, 13691, 13691, 13691, 13691, 13691, 13691,
   13691, 13691, 13691, 13691, 13691, 13691, 13691, 13691, 13691, 13691,
   13691, 17730, 13691, -1625, 13691, 13691, 13691, 14065, 15145, 15145,
   15145, 15145, 15145, 14263,   791,   922,  5055, 13691, 13691, 13691,
   13691, 13691, 13691, 13691, 13691, 13691, 13691, 13691, 13691, -1625,
   -1625, -1625, -1625,  1670, 13691, 13691, -1625, 10646, 10646, 13691,
   13691,   538,   158, 16556,   707,   711, 12879, 16009, -1625, 13691,
   -1625,   708,   898,   756,   717,   718, 14217,   427, 13082, -1625,
   13285, -1625,   653,   720,   724,  2412, -1625,   161, 10646, -1625,
    4200, -1625, -1625, 16079, -1625, -1625, 10849, -1625, 13691, -1625,
     829,  9428,   915,   735, 13878,   913,   128,    52, -1625, -1625,
   -1625,   747, -1625, -1625, -1625,  3186, -1625,  2760,   736,   920,
   16350, 15145, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625,   740, -1625, -1625,   738,   742,   744,   743,   745,   750,
     274,   748,   753,  4184, 14968, -1625, -1625, 15145, 15145, 13691,
     427,   159, -1625, 16350,   847, -1625, -1625, -1625,   427,   130,
     132,   752,   760,  2463,   191,   762,   754,   175,   828,   767,
     427,   133,   768, 17034,   761,   959,   961,   774,   775,   776,
     779, -1625, 14616, 15145, -1625, -1625,   917,  3892,    23, -1625,
   -1625, -1625,   579, -1625, -1625, -1625,   951,   852,   808,   343,
     832, 13691,   538,   857,   985,   800, -1625,   838, -1625,   158,
   -1625,  3186,  3186,   989,   842,    88, -1625,   806,   995, -1625,
    3186,    55, -1625,   351,   165, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625,   945,  4004, -1625, -1625, -1625, -1625,   996,   824,
   -1625, 16865, 13691,   814,  1003, 17985,  1002, -1625, -1625,   886,
    3923, 12052, 18168, 18249, 18286, 13691, 17937, 18323, 11640, 13060,
   13465, 12652, 14898, 15251, 15251, 15251, 15251,  2825,  2825,  2825,
    2825,  2825,   713,   713,   741,   741,   741,   347,   347,   347,
   -1625,   764, 17985,   817,   818, 17082,   823,  1015,   -11, 13691,
       1,   655,   199,   158, -1625, -1625, -1625,  1011,   786, -1625,
     711, 16659, -1625, -1625, -1625, 18249, 18249, 18249, 18249, 18249,
   18249, 18249, 18249, 18249, 18249, 18249, 18249, 18249, -1625, 13691,
     360,   176, -1625, -1625,   655,   392,   826,  4097,   830,   833,
     844,  4185,   135,   836, -1625, 17985,  4685, -1625, 15145, -1625,
      55,    42, 16865, 17985, 16865, 17138,   886,    55,   427,   178,
     866,   837, 13691, -1625,   192, -1625, -1625, -1625,  9022,   515,
   -1625, -1625, 17985, 17985,   210, -1625, -1625, -1625, 13691,   933,
   16226, 16350, 15145,  9631,   854,   855, -1625,  1027, 14440,   919,
   -1625,   896, -1625,  1048,   874,  3519,  3186, 16350, 16350, 16350,
   16350, 16350,   863,  1004,  1007,  1008,  1009,  1010,   877, 16350,
      19, -1625, -1625, -1625, -1625, -1625, -1625,    14, -1625, 18079,
   -1625, -1625,   229, -1625,  6180, 14111,   888, 14968, -1625, 14968,
   -1625, 14968, -1625, 15145, 15145, 14968, -1625, 14968, 14968, 15145,
   -1625,  1076,   889, -1625,   374, -1625, -1625,  4458, -1625, 18079,
    1074, 16865,   895, -1625, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625,   914,  1089, 15145, 14111,   900, 16556, 16762,  1086,
   -1625, 13691, -1625, 13691, -1625, 13691, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625,   901, -1625, 13691, -1625, -1625,  5368,
   -1625,  3186, 14111,   906, -1625, -1625, -1625, -1625,  1092,   910,
   13691, 17630, -1625, -1625, 14065,   911, -1625,  3186, -1625,   921,
    6383,  1078,    46, -1625, -1625,    95,  1670, -1625,  4200, -1625,
    3186, -1625, -1625,   427, 17985, -1625, 11052, -1625, 16350,    79,
     925, 14111,   852, -1625, -1625, 18323, 13691, -1625, -1625, 13691,
   -1625, 13691, -1625,  4763,   926, 10646,   828,  1085,   852,  3186,
    1110,   886, 15145, 17730,   427,  4864,   930, -1625, -1625,   171,
     931, -1625, -1625,  1114,  3321,  3321,  4685, -1625, -1625, -1625,
    1080,   937,  1064,  1068,  1069,  1070,  1071,    53,   946,   255,
   -1625, -1625, -1625, -1625, -1625,   984, -1625, -1625, -1625, -1625,
    1137,   950,   708,   427,   427, 13488,   852,  4200, -1625, -1625,
    4948,   639,   210, 10240, -1625,  6586,   953,  6789,   954, 16226,
   16865,   957,  1013,   427, 18079,  1142, -1625, -1625, -1625, -1625,
     340, -1625,   233,  3186,   976,  1024,  1012,  3186, 15145,  4381,
   -1625, -1625, -1625,  1160, -1625,   973,   996,   575,   575,  1102,
    1102, 17290,   971,  1167, 16350, 16350, 16350, 16350, 16350, 16350,
   17630,  5315, 15592, 16350, 16350, 16350, 16350, 16107, 16350, 16350,
   16350, 16350, 16350, 16350, 16350, 16350, 16350, 16350, 16350, 16350,
   16350, 16350, 16350, 16350, 16350, 16350, 16350, 16350, 16350, 16350,
   16350, 15145, -1625, -1625,  1098, -1625, -1625,   982,   994,   997,
   -1625,   999, -1625, -1625,   381,  4184, -1625,   991, -1625, 16350,
     427, -1625, -1625,   118, -1625,   622,  1197, -1625, -1625,   137,
    1017,   427, 11255, 17985, 17186, -1625,  3401, -1625,  5571,   786,
    1197, -1625,   241,    41, -1625, 17985,  1072,  1020, -1625,  1021,
    1078, -1625,  3186,   842,  3186,    62,  1193,  1128,   193, -1625,
     655,   194, -1625, -1625, 16865, 13691, 17985, 18079,  1023,    79,
   -1625,  1019,    79,  1025, 18323, 17985, 17242,  1026, 10646,  1028,
    1029,  3186,  1030,  1033,  3186,   852, -1625,   653,   428, 10646,
   13691, -1625, -1625, -1625, -1625, -1625, -1625,  1087,  1032,  1217,
    1136,  4685,  4685,  4685,  4685,  4685,  4685,  1081, -1625, 17630,
      90,  4685, -1625, -1625, -1625, 16865, 17985,  1038, -1625,   210,
    1207,  1163, 10240, -1625, -1625, -1625,  1043, 13691,  1013,   427,
   16556, 16226,  1047, 16350,  6992,   556,  1049, 13691,    69,   307,
   -1625,  1063, -1625,  3186, 15145, -1625,  1112, -1625, -1625,  4281,
    1214,  1052, 16350, -1625, 16350, -1625,  1053,  1050,  1245, 17345,
    1054, 18079,  1247,  1055,  1056,  1058,  1127,  1255,  1073, -1625,
   -1625, -1625, 17393,  1065,  1257, 18124, 18212, 10626, 16350, 18033,
   12858, 13263, 14063, 14722, 15075, 16307, 16307, 16307, 16307,  1834,
    1834,  1834,  1834,  1834,  1133,  1133,   575,   575,   575,  1102,
    1102,  1102,  1102, -1625,  1075, -1625,  1077,  1099,  1101,  1103,
   -1625, -1625, 18079, 15145,  3186,  3186, -1625,   622, 14111,  1042,
   -1625, 16556, -1625, -1625, 18249, 13691,  1067, -1625,  1079,  1169,
   -1625,   325, 13691, -1625, -1625, -1625, 13691, -1625, 13691, -1625,
     842, -1625, -1625,   129,  1254,  1186, 13691, -1625,  1088,   427,
   17985,  1078,  1090, -1625,  1105,    79, 13691, 10646,  1106, -1625,
   -1625,   786, -1625, -1625,  1107,  1084,  1104, -1625,  1109,  4685,
   -1625,  4685, -1625, -1625,  1115,  1093,  1262,  1140,  1095, -1625,
    1264,  1113,  1118,  1119, -1625,  1168,  1120,  1298, -1625, -1625,
     427, -1625,  1277, -1625,  1117, -1625, -1625,  1126,  1130,   139,
   -1625, -1625, 18079,  1131,  1132, -1625,  3756, -1625, -1625, -1625,
   -1625, -1625, -1625,  1188,  3186, -1625,  3186, -1625, 18079, 17448,
   -1625, -1625, 16350, -1625, 16350, -1625, 16350, -1625, -1625, -1625,
   -1625, 16350, 17630, -1625, -1625, 16350, -1625, 16350, -1625, 11032,
   16350,  1129,  7195, -1625, -1625, -1625, -1625,   622, -1625, -1625,
   -1625, -1625,   607, 14897, 14111,  1213, -1625,  2865,  1162,  2445,
   -1625, -1625, -1625,   791,  2570,   108,   109,  1143,   786,   922,
     140, 17985, -1625, -1625, -1625,  1170,  5243, 11442, 17985, -1625,
      73,  1320,  1259, 13691, -1625, 17985, 10646,  1221,  1078,  1441,
    1078,  1146, 17985,  1147, -1625,  2086,  1149,  2168, -1625, -1625,
      79, -1625, -1625,  1210, -1625, -1625,  4685, -1625,  4685, -1625,
    4685, -1625, -1625, -1625, -1625,  4685, -1625, 17630, -1625,  2183,
   -1625,  9022, -1625, -1625, -1625, -1625,  9834, -1625, -1625, -1625,
    9022,  3186, -1625,  1152, 16350, 17496, 18079, 18079, 18079,  1215,
   18079, 17551, 11032, -1625, -1625,   622, 14111, 14111, 15145, -1625,
    1338, 15744,    84, -1625, 14897,   786, 15334, -1625,  1172, -1625,
     111,  1155,   112, -1625, 15250, -1625, -1625, -1625,   113, -1625,
   -1625,  1753, -1625,  1157, -1625,  1273,   711, -1625, 15074, -1625,
   15074, -1625, -1625,  1344,   791, -1625, 14369, -1625, -1625, -1625,
   -1625,  1345,  1279, 13691, -1625, 17985,  1171,  1174,  1078,   547,
   -1625,  1221,  1078, -1625, -1625, -1625, -1625,  2276,  1175,  4685,
    1228, -1625, -1625, -1625,  1229, -1625,  9022, 10037,  9834, -1625,
   -1625, -1625,  9022, -1625, -1625, 18079, 16350, 16350, 16350,  7398,
    1176,  1177, -1625, 16350, -1625, 14111, -1625, -1625, -1625, -1625,
   -1625,  3186,   763,  2865, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625, -1625, -1625,   152, -1625,  1162, -1625,
   -1625, -1625, -1625, -1625,   125,   602, -1625,  1352,   114, 15440,
    1273,  1362, -1625,  3186,   711, -1625, -1625,  1179,  1364, 13691,
   -1625, 17985, -1625,   116,  1180, -1625, -1625, -1625,  1078,   547,
   14545, -1625,  1078, -1625,  4685,  4685, -1625, -1625, -1625, -1625,
    7601, 18079, 18079, 18079, -1625, -1625, -1625, 18079, -1625,  3424,
    1371,  1374,  1184, -1625, -1625, 16350, 15250, 15250,  1318, -1625,
    1753,  1753,   656, -1625, -1625, -1625, 16350,  1304, -1625,  1208,
    1192,   115, 16350, -1625, 15145, -1625, 16350, 17985,  1307, -1625,
    1383, -1625,  7804,  1194, -1625, -1625,   547, -1625, -1625,  8007,
    1196,  1280, -1625,  1294,  1237, -1625, -1625,  1296,  3186,  1218,
     763, -1625, -1625, 18079, -1625, -1625,  1227, -1625,  1365, -1625,
   -1625, -1625, -1625, 18079,  1388,   175, -1625, -1625, 18079,  1219,
   18079, -1625,   170,  1206,  8210, -1625, -1625, -1625,  1222, -1625,
    1223,  1243, 15145,   922,  1224, -1625, -1625, -1625, 16350,  1240,
      80, -1625,  1340, -1625, -1625, -1625,  8413, -1625, 14111,   888,
   -1625,  1250, 15145,   771, -1625, 18079, -1625,  1234,  1426,   576,
      80, -1625, -1625,  1353, -1625, 14111,  1239, -1625,  1078,    87,
   -1625, -1625, -1625, -1625,  3186, -1625,  1241,  1246,   117, -1625,
    1249,   576,   134,  1078,  1251, -1625,  3186,   549,  3186,   263,
    1432,  1357,  1249, -1625,  1439, -1625,   490, -1625, -1625, -1625,
     154,  1435,  1370, 13691, -1625,   549,  8616,  3186, -1625,  3186,
   -1625,  8819,   344,  1440,  1372, 13691, -1625, 17985, -1625, -1625,
   -1625, -1625, -1625,  1442,  1375, 13691, -1625, 17985, 13691, -1625,
   17985, 17985
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,     0,   438,     0,   867,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   959,
     947,     0,   733,     0,   739,   740,   741,    29,   805,   934,
     935,   162,   163,   742,     0,   143,     0,     0,     0,     0,
      30,     0,     0,     0,     0,   197,     0,     0,     0,     0,
       0,     0,   407,   408,   409,   406,   405,   404,     0,     0,
       0,     0,   226,     0,     0,     0,    37,    38,    40,    41,
      39,   746,   748,   749,   743,   744,     0,     0,     0,   750,
     745,     0,   716,    32,    33,    34,    36,    35,     0,   747,
       0,     0,     0,     0,   751,   410,   545,    31,     0,   161,
     133,   939,   734,     0,     0,     4,   123,   125,   804,     0,
     715,     0,     6,   196,     7,     9,     8,    10,     0,     0,
     402,   451,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   449,   922,   923,   527,   521,   522,   523,   524,   525,
     526,   432,   530,     0,   431,   894,   717,   724,     0,   807,
     520,   401,   897,   898,   909,   450,     0,     0,   453,   452,
     895,   896,   893,   929,   933,     0,   510,   806,    11,   407,
     408,   409,     0,     0,    36,     0,   123,   196,     0,   999,
     450,  1000,     0,  1002,  1003,   529,   446,     0,   439,   444,
       0,     0,   492,   493,   494,   495,    29,   934,   742,   719,
      37,    38,    40,    41,    39,     0,     0,  1023,   915,   717,
       0,   718,   471,     0,   473,   511,   512,   513,   514,   515,
     516,   517,   519,     0,   963,     0,   814,   729,   216,     0,
    1023,   429,   728,   722,     0,   738,   718,   942,   943,   949,
     941,   730,     0,   430,     0,   732,   518,     0,     0,     0,
       0,   435,     0,   141,   437,     0,     0,   147,   149,     0,
       0,   151,     0,    75,    76,    81,    82,    67,    68,    59,
      79,    90,    91,     0,    62,     0,    66,    74,    72,    93,
      85,    84,    57,    80,   100,   101,    58,    96,    55,    97,
      56,    98,    54,   102,    89,    94,    99,    86,    87,    61,
      88,    92,    53,    83,    69,   103,    77,    70,    60,    47,
      48,    49,    50,    51,    52,    71,   105,   104,   107,    64,
      45,    46,    73,  1070,  1071,    65,  1075,    44,    63,    95,
       0,     0,   123,   106,  1014,  1069,     0,  1072,     0,     0,
     153,     0,     0,     0,   187,     0,     0,     0,     0,     0,
       0,     0,     0,   816,     0,   111,   113,   315,     0,     0,
     314,   320,     0,     0,   227,     0,   230,     0,     0,     0,
       0,  1020,   212,   224,   955,   959,   564,   591,   591,   564,
     591,     0,   984,     0,   753,     0,     0,     0,   982,     0,
      16,     0,   127,   204,   218,   225,   621,   557,     0,  1008,
     537,   539,   541,   871,   438,   451,     0,     0,   449,   450,
     452,     0,     0,   735,     0,   736,     0,     0,     0,   186,
       0,     0,   129,   306,     0,    28,   195,     0,   223,   208,
     222,   407,   410,   196,   403,   176,   177,   178,   179,   180,
     182,   183,   185,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   947,     0,   175,   938,   938,   969,     0,     0,     0,
       0,     0,     0,     0,     0,   400,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   470,
     472,   872,   873,     0,   938,     0,   885,   306,   306,   938,
       0,   940,   930,   955,     0,   196,     0,     0,   155,     0,
     869,   864,   814,     0,   451,   449,     0,   967,     0,   562,
     813,   958,   738,   451,   449,   450,   129,     0,   306,   428,
       0,   887,   731,     0,   133,   266,     0,   544,     0,   158,
       0,     0,   436,     0,     0,     0,     0,     0,   150,   174,
     152,  1070,  1071,  1067,  1068,     0,  1074,  1060,     0,     0,
       0,     0,    78,    43,    65,    42,  1015,   181,   184,   154,
     133,     0,   171,   173,     0,     0,     0,     0,     0,     0,
     114,     0,     0,     0,   815,   112,    18,     0,   108,     0,
     316,     0,   156,     0,     0,   157,   228,   229,  1004,     0,
       0,   451,   449,   450,   453,   452,     0,  1050,   236,     0,
     956,     0,     0,     0,     0,   814,   814,     0,     0,     0,
       0,   159,     0,     0,   752,   983,   805,     0,     0,   981,
     810,   980,   126,     5,    13,    14,     0,   234,     0,     0,
     550,     0,     0,     0,   814,     0,   726,     0,   725,   720,
     551,     0,     0,     0,     0,   871,   133,     0,   816,   870,
    1079,   427,   441,   506,   903,   921,   138,   132,   134,   135,
     136,   137,   401,     0,   528,   808,   809,   124,   814,     0,
    1024,     0,     0,     0,   816,   307,     0,   533,   198,   232,
       0,   476,   478,   477,   489,     0,     0,   509,   474,   475,
     479,   481,   480,   498,   499,   496,   497,   500,   501,   502,
     503,   504,   490,   491,   483,   484,   482,   485,   486,   488,
     505,   487,   937,     0,     0,   973,     0,   814,  1007,     0,
    1006,  1023,   900,   929,   214,   206,   220,     0,  1008,   210,
     196,     0,   442,   445,   447,   455,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   875,     0,
     874,   877,   899,   881,  1023,   878,     0,     0,     0,     0,
       0,     0,     0,     0,  1001,   440,   862,   866,   813,   868,
       0,   721,     0,   962,     0,   961,   232,     0,   721,   946,
     945,     0,     0,   874,   877,   944,   878,   433,   268,   270,
     133,   548,   547,   434,     0,   133,   250,   142,   437,     0,
       0,     0,     0,     0,   262,   262,   148,   814,     0,     0,
    1059,     0,  1056,   814,     0,  1030,     0,     0,     0,     0,
       0,   812,     0,    37,    38,    40,    41,    39,     0,     0,
     755,   759,   760,   761,   762,   763,   765,     0,   754,   131,
     803,   764,  1023,  1073,     0,     0,     0,     0,    21,     0,
      22,     0,    19,     0,   109,     0,    20,     0,     0,     0,
     120,   816,     0,   118,   113,   110,   115,     0,   313,   321,
     318,     0,     0,   993,   998,   995,   994,   997,   996,    12,
    1048,  1049,     0,   814,     0,     0,     0,   955,   952,     0,
     561,     0,   575,   813,   563,   813,   590,   578,   584,   587,
     581,   992,   991,   990,     0,   986,     0,   987,   989,     0,
       5,     0,     0,     0,   615,   616,   624,   623,     0,   449,
       0,   813,   556,   560,     0,     0,  1009,     0,   538,     0,
       0,  1037,   871,   292,  1078,     0,     0,   886,     0,   936,
     813,  1026,  1022,   308,   309,   714,   815,   305,     0,   871,
       0,     0,   234,   535,   200,   508,     0,   598,   599,     0,
     596,   813,   968,     0,     0,   306,   236,     0,   234,     0,
       0,   232,     0,   947,   456,     0,     0,   883,   884,   901,
     902,   931,   932,     0,     0,     0,   850,   821,   822,   823,
     830,     0,    37,    38,    40,    41,    39,     0,     0,   836,
     842,   843,   844,   845,   846,     0,   834,   832,   833,   856,
     814,     0,   864,   966,   965,     0,   234,     0,   888,   737,
       0,   272,     0,     0,   139,     0,     0,     0,     0,     0,
       0,     0,   242,   243,   254,     0,   133,   252,   168,   262,
       0,   262,     0,   813,     0,     0,     0,     0,     0,   813,
    1058,  1061,  1029,   814,  1028,     0,   814,   786,   787,   784,
     785,   820,     0,   814,   812,   568,   593,   593,   568,   593,
     559,     0,     0,   975,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1064,   188,     0,   191,   172,     0,     0,     0,
     116,     0,   121,   122,   114,   815,   119,     0,   317,     0,
    1005,   160,  1021,  1050,  1041,  1045,   235,   237,   327,     0,
       0,   953,     0,   566,     0,   985,     0,    17,     0,  1008,
     233,   327,     0,     0,   721,   553,     0,   727,  1010,     0,
    1037,   542,     0,     0,  1079,     0,   297,   295,   877,   889,
    1023,   877,   890,  1025,     0,     0,   310,   130,     0,   871,
     231,     0,   871,     0,   507,   972,   971,     0,   306,     0,
       0,     0,     0,     0,     0,   234,   202,   738,   876,   306,
       0,   826,   827,   828,   829,   837,   838,   854,     0,   814,
       0,   850,   572,   595,   595,   572,   595,     0,   825,   858,
       0,   813,   861,   863,   865,     0,   960,     0,   876,     0,
       0,     0,     0,   269,   549,   144,     0,   437,   242,   244,
     955,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     256,     0,  1065,     0,     0,  1051,     0,  1057,  1055,   813,
       0,     0,     0,   757,   813,   811,     0,     0,   814,     0,
       0,   800,   814,     0,     0,     0,     0,   814,     0,   766,
     801,   802,   979,     0,   814,   769,   771,   770,     0,     0,
     767,   768,   772,   774,   773,   790,   791,   788,   789,   792,
     793,   794,   795,   796,   781,   782,   776,   777,   775,   778,
     779,   780,   783,  1063,     0,   133,     0,     0,     0,     0,
     117,    23,   319,     0,     0,     0,  1042,  1047,     0,   401,
     957,   955,   443,   448,   454,     0,     0,    15,     0,   401,
     627,     0,     0,   629,   622,   625,     0,   620,     0,  1012,
       0,  1038,   546,     0,   298,     0,     0,   293,     0,   312,
     311,  1037,     0,   327,     0,   871,     0,   306,     0,   927,
     327,  1008,   327,  1011,     0,     0,     0,   457,     0,     0,
     840,   813,   849,   831,     0,     0,   814,     0,     0,   848,
     814,     0,     0,     0,   824,     0,     0,   814,   835,   855,
     964,   327,     0,   133,     0,   265,   251,     0,     0,     0,
     241,   164,   255,     0,     0,   258,     0,   263,   264,   133,
     257,  1066,  1052,     0,     0,  1027,     0,  1077,   819,   818,
     756,   576,   813,   567,     0,   579,   813,   592,   585,   588,
     582,     0,   813,   558,   758,     0,   597,   813,   974,   798,
       0,     0,     0,    24,    25,    26,    27,  1044,  1039,  1040,
    1043,   238,     0,     0,     0,   408,   399,     0,     0,     0,
     213,   326,   328,     0,   398,     0,     0,     0,  1008,   401,
       0,   565,   988,   323,   219,   618,     0,     0,   552,   540,
       0,   301,   291,     0,   294,   300,   306,   532,  1037,   401,
    1037,     0,   970,     0,   926,   401,     0,   401,  1013,   327,
     871,   924,   853,   852,   839,   577,   813,   571,     0,   580,
     813,   594,   586,   589,   583,     0,   841,   813,   857,   401,
     133,   271,   140,   145,   166,   245,     0,   253,   259,   133,
     261,     0,  1053,     0,     0,     0,   570,   799,   555,     0,
     978,   977,   797,   133,   192,  1046,     0,     0,     0,  1016,
       0,     0,     0,   239,     0,  1008,     0,   364,   360,   366,
     716,    36,     0,   354,     0,   359,   363,   376,     0,   374,
     379,     0,   378,     0,   377,     0,   196,   330,     0,   332,
       0,   333,   334,     0,     0,   954,     0,   619,   617,   628,
     626,   302,     0,     0,   289,   299,     0,     0,  1037,     0,
     209,   532,  1037,   928,   215,   323,   221,   401,     0,     0,
       0,   574,   847,   860,     0,   217,   267,     0,     0,   133,
     248,   165,   260,  1054,  1076,   817,     0,     0,     0,     0,
       0,     0,   426,     0,  1017,     0,   344,   348,   423,   424,
     358,     0,     0,     0,   339,   677,   678,   676,   679,   680,
     697,   699,   698,   668,   640,   638,   639,   658,   673,   674,
     634,   645,   646,   648,   647,   667,   651,   649,   650,   652,
     653,   654,   655,   656,   657,   659,   660,   661,   662,   663,
     664,   666,   665,   635,   636,   637,   641,   642,   644,   682,
     683,   687,   688,   689,   690,   691,   692,   675,   694,   684,
     685,   686,   669,   670,   671,   672,   695,   696,   700,   702,
     701,   703,   704,   681,   706,   705,   708,   710,   709,   643,
     713,   711,   712,   707,   693,   633,   371,   630,     0,   340,
     392,   393,   391,   384,     0,   385,   341,   418,     0,     0,
       0,     0,   422,     0,   196,   205,   322,     0,     0,     0,
     290,   304,   925,     0,     0,   394,   133,   199,  1037,     0,
       0,   211,  1037,   851,     0,     0,   133,   246,   146,   167,
       0,   569,   554,   976,   190,   342,   343,   421,   240,     0,
     814,   814,     0,   367,   355,     0,     0,     0,   373,   375,
       0,     0,   380,   387,   388,   386,     0,     0,   329,  1018,
       0,     0,     0,   425,     0,   324,     0,   303,     0,   613,
     816,   133,     0,     0,   201,   207,     0,   573,   859,     0,
       0,   169,   345,   123,     0,   346,   347,     0,   813,     0,
     813,   369,   365,   370,   631,   632,     0,   356,   389,   390,
     382,   383,   381,   419,   416,  1050,   335,   331,   420,     0,
     325,   614,   815,     0,     0,   395,   133,   203,     0,   249,
       0,   194,     0,   401,     0,   361,   368,   372,     0,     0,
     871,   337,     0,   611,   531,   534,     0,   247,     0,     0,
     170,   352,     0,   400,   362,   417,  1019,     0,   816,   412,
     871,   612,   536,     0,   193,     0,     0,   351,  1037,   871,
     276,   415,   414,   413,  1079,   411,     0,     0,     0,   350,
    1031,   412,     0,  1037,     0,   349,     0,     0,  1079,     0,
     281,   279,  1031,   133,   816,  1033,     0,   396,   133,   336,
       0,   282,     0,     0,   277,     0,     0,   815,  1032,     0,
    1036,     0,     0,   285,   275,     0,   278,   284,   338,   189,
    1034,  1035,   397,   286,     0,     0,   273,   283,     0,   274,
     288,   287
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1625, -1625, -1625,  -575, -1625, -1625, -1625,   177,     0,   -31,
     335, -1625,  -262,  -545, -1625, -1625,   314,  1492,  1704, -1625,
    1788, -1625,  -501, -1625,    28, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625, -1625, -1625,  -448, -1625, -1625,  -173,
     143,    27, -1625, -1625, -1625, -1625, -1625, -1625,    33, -1625,
   -1625, -1625, -1625, -1625, -1625,    37, -1625, -1625,   974,   987,
     986,   -91,  -714,  -894,   482,   537,  -443,   225,  -968, -1625,
    -163, -1625, -1625, -1625, -1625,  -755,    56, -1625, -1625, -1625,
   -1625,  -423, -1625,  -624, -1625,  -432, -1625, -1625,   887, -1625,
    -147, -1625, -1625, -1098, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625,  -174, -1625,   -86, -1625, -1625, -1625,
   -1625, -1625,  -257, -1625,    15, -1049, -1625, -1624,  -463, -1625,
    -152,    68,  -127,  -438, -1625,  -265, -1625, -1625, -1625,    21,
     -25,    11,    54,  -752,   -70, -1625, -1625,    10, -1625,   -12,
   -1625, -1625,    -5,   -53,  -122, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625,  -628,  -876, -1625, -1625, -1625, -1625,
   -1625,  1798,  1123, -1625,   416, -1625,   280, -1625, -1625, -1625,
   -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625,
   -1625, -1625, -1625,   142,  -520,  -551, -1625, -1625, -1625, -1625,
   -1625,   345, -1625, -1625, -1625, -1625, -1625, -1625, -1625, -1625,
   -1039,  -392,  2724,    -3, -1625,  1100,  -418, -1625, -1625,  -494,
    3487,  3585, -1625,  -665, -1625, -1625,   425,    22,  -636, -1625,
   -1625,   501,   294,  -683, -1625,   296, -1625, -1625, -1625, -1625,
   -1625,   483, -1625, -1625, -1625,    75,  -898,  -161,  -439,  -431,
   -1625,   562,  -123, -1625, -1625,    35,    36,   612, -1625, -1625,
    1158,   -21, -1625,  -379,    47,  -142, -1625,   163, -1625, -1625,
   -1625,  -487,  1148, -1625, -1625, -1625, -1625, -1625,   618,   329,
   -1625, -1625, -1625,  -367,  -687, -1625,  1097, -1212, -1625,   -68,
    -171,   -65,   687, -1625,  -413, -1625,  -433,  -842, -1297,  -332,
      70, -1625,   391,   466, -1625, -1625, -1625, -1625,   415, -1625,
    1575, -1155
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   115,   940,   653,   186,  1579,   751,
     362,   363,   364,   365,   891,   892,   893,   117,   118,   119,
     120,   121,   421,   687,   688,   561,   262,  1647,   567,  1556,
    1648,  1891,   876,   355,   590,  1851,  1136,  1335,  1910,   437,
     187,   689,   980,  1203,  1396,   125,   656,   997,   690,   709,
    1001,   628,   996,   241,   542,   691,   657,   998,   439,   382,
     404,   128,   982,   943,   916,  1156,  1582,  1262,  1062,  1798,
    1651,   827,  1068,   566,   836,  1070,  1439,   819,  1051,  1054,
    1251,  1917,  1918,   677,   678,   703,   704,   369,   370,   372,
    1616,  1776,  1777,  1349,  1491,  1605,  1770,  1900,  1920,  1809,
    1855,  1856,  1857,  1592,  1593,  1594,  1595,  1811,  1812,  1818,
    1867,  1598,  1599,  1603,  1763,  1764,  1765,  1787,  1959,  1492,
    1493,   188,   130,  1934,  1935,  1768,  1495,  1496,  1497,  1498,
     131,   255,   562,   563,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,  1628,   142,   979,  1202,   143,   674,
     675,   676,   259,   413,   557,   663,   664,  1297,   665,  1298,
     144,   145,   634,   635,  1287,  1288,  1405,  1406,   146,   861,
    1030,   147,   862,  1031,   148,   863,  1032,   149,   864,  1033,
     150,   865,  1034,   637,  1290,  1408,   151,   866,   152,   153,
    1840,   154,   658,  1618,   659,  1172,   948,  1367,  1364,  1756,
    1757,   155,   156,   157,   244,   158,   245,   256,   424,   549,
     159,  1291,  1292,   870,   871,   160,  1092,   971,   605,  1093,
    1037,  1225,  1038,  1409,  1410,  1228,  1229,  1040,  1416,  1417,
    1041,   797,   532,   200,   201,   692,   680,   513,  1188,  1189,
     783,   784,   967,   162,   247,   163,   164,   190,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   743,   175,   251,
     252,   631,   235,   236,   746,   747,  1303,  1304,   397,   398,
     934,   176,   619,   177,   673,   178,   346,  1778,  1830,   383,
     432,   698,   699,  1085,  1947,  1954,  1955,  1183,  1346,   912,
    1347,   913,   914,   842,   843,   844,   347,   348,   873,   576,
    1581,   965
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     189,   191,   444,   193,   194,   195,   196,   198,   199,   494,
     202,   203,   204,   205,   524,   666,   225,   226,   227,   228,
     229,   230,   231,   232,   234,   344,   253,   243,   668,  1373,
     405,   124,   122,   522,   408,   409,   792,   126,   416,   261,
     670,   127,   963,   516,   258,   352,   958,   269,   806,   272,
    1480,   959,   353,   818,   356,   418,   343,   263,   890,   895,
     440,   444,   267,   740,  1184,   248,   249,   351,   977,   546,
     493,  1000,   129,  1359,   781,  1176,  1058,   250,   939,   161,
    1072,   261,   782,   415,   834,   788,   789,   420,  1201,   874,
     550,  1258,  1046,  1665,    14,   595,   597,   599,   260,   602,
     -43,  1437,   -78,   417,  1212,   -43,   434,   -78,   558,   -42,
     611,   814,   614,  1039,   -42,   558,   811,  1608,  1610,   815,
    -357,  1673,  1758,  1827,  1827,   514,  1665,    14,    14,   551,
    -600,  1036,   418,  1185,  1820,    14,    14,   832,   533,   901,
     391,   558,   918,  1237,   918,  1374,   918,   123,   918,   918,
     207,    40,  1247,   591,  1838,   371,  1621,   511,   512,   514,
     415,  1821, -1023,   535,   420,  1844,  1815,  1510,   910,   911,
    -107,  1418,  1949,  -718,  1102,   960,   511,   512,  1186, -1023,
     417,   116,  -106,   431,  1816,  -107,   544,   527,     3,   511,
     512,   534,  1972,  -610,  -607,   420,  -608,  -106, -1023,  1839,
     431, -1023,   192,  1817,   607,   394,  -906,  1365,  1902,  -916,
    1238,   417,  1511,  1103,   543,  -907,   592,  1950, -1023,  -905,
    1300,  -948,  1887,   938,  1296,   910,   911,  -904,   519,   495,
     270,   643,  -719,   342,   417,  -911,  1375,  1973,  -918,  -815,
    1366,  -910,   367,  -815,   207,    40,  -908,  1622,  -951,   553,
     381,   835,   553,  1903,   515,  1146,  -915,   519,   541,   261,
     564,  -607,  -950,  -891,  -892,   669,   608,  1438,   521,  1187,
     710,  1580,   443,   403,   575,   381,  -296,  -280,  1480,   381,
     381,  -296,  1666,  1667,  -815,  1519,   555,  1215,   515,   -43,
     560,   -78,  1525,  1430,  1527,   435,  -725,   559,   -42,   612,
    -813,   615,   410,  1512,   641,   381,  1609,  1611,  1951,  -357,
    1674,  1759,  1828,  1877,  1265,  1945,  1269,  -726,   586,  1055,
     518,  1395,  1822,  1549,  1057,   833,   622,   902,  1974,   903,
     919,   525,  1013,  1227,  1350,  -906,  1555,  1615,  1370,  1267,
    1268,  -913,  1142,  1143,  -907,   753,  1961,  -720,  -905,   343,
    -948,  1036,   793,  -914,   621,  1198,  -904,   520,   625,  -727,
     368,  -917,   111,  1415,  -911,  1168,   430,  -920,   444,   708,
    -910,   753,  1668,   261,   417,  -908,   883,  -951,   531,  1505,
     234,   633,   261,   261,   633,   261,   520,   518,   907,  1131,
     647,  -950,  -891,  -892,   753,   995,  1771,   945,  1772,   254,
     654,   655,   375,   344,   481,   753,   257,  1360,   753,   198,
     431,   411,   376,  1267,  1268,  -123,   482,   693,   412,  -123,
    1361,  -609,   405,   757,   758,   440,   762,  1983,   705,  -543,
    1159,  1637,   620,  1270,   343,   116,  -123,  1962,   884,   116,
    1362,   636,   636,   565,   636,  1266,  1267,  1268,   711,   712,
     713,   714,   716,   717,   718,   719,   720,   721,   722,   723,
     724,   725,   726,   727,   728,   729,   730,   731,   732,   733,
     734,   735,   736,   737,   738,   739,   607,   741,   243,   742,
     742,   745,  1358,   883,   764,   511,   512,   750,   679,   129,
     406,   765,   766,   767,   768,   769,   770,   771,   772,   773,
     774,   775,   776,   777,  1969,  1427,   763,  1440,   946,   742,
     787,  -721,   705,   705,   742,   791,   248,   249,  1984,   264,
     966,   765,   968,   947,   795,   354,   585,   392,   250,  1191,
     638,   642,   640,   803,   752,   805,   343,  1192,   430,  1517,
     494,  -879,   265,   705,   821,   430,   392,  -918,  1227,  1407,
     392,   822,  1407,   823,   799,  1372,  -879,   649,  1419,   666,
     785,   511,   512,  1209,   123,  1264,  1036,  1036,  1036,  1036,
    1036,  1036,   668,  -882,   392,  1382,  1036,  1293,  1384,  1295,
     994,   649,   266,   752,   670,  1217,  1569,  1002,  -882,   826,
    1052,  1053,  1344,  1345,   810,   511,   512,   816,   116,   392,
     890,   493,   395,   396,   897,   392,   393,   697,   949,  -880,
     377,   342,   649,  1006,   381,  1137,   165,  1138,   378,  1139,
     644,   395,   396,  1141,  -880,   395,   396,   392,  1127,  1128,
    1129,   222,   224,  1823,   423,   379,   392,   760,   380,   966,
     968,   384,   984,   426,  1130,   373,  1047,   968,   744,   395,
     396,   385,  1824,   392,   374,  1825,   417,   924,   926,  1970,
     649,  1434,  1267,  1268,   585,   381,   755,   381,   381,   381,
     381,  1644,   696,   394,   395,   396,  1629,   786,  1631,   650,
     395,   396,   790,  1411,  1048,  1413,   952,  1870,   406,   546,
     780,   386,   594,   596,   598,   695,   601,   974,  1931,  1932,
    1933,  1132,   395,   396,  1526,   387,  1871,   388,   666,  1872,
     985,   395,   396,   585,  1249,  1250,  1532,   389,  1533,   390,
     419,   668,   645,  1397,  1344,  1345,   651,   813,   395,   396,
     425,   427,   428,   670,  1036,   407,  1036,   422,   116,  1576,
    1577,   429,  1509,   430,   993,  1785,  1786,  1957,  1958,   436,
     679,   433,   645,  -601,   651,   645,   651,   651,   872,   445,
     495,  1521,   446,   475,   476,   477,   478,   479,   480,   992,
     481,  1868,  1869,  1429,  1005,   447,  1388,  1864,  1865,  1942,
    1083,  1086,   482,   448,   896,   697,  1784,  1398,   449,   450,
    1789,   451,   452,  1960,   478,   479,   480,   419,   481,  -602,
    -603,  -604,  -605,   484,   485,    55,   487,  1050,   486,   669,
     482,  1613,   753,   441,   180,   181,    65,    66,    67,   933,
     935,   517,  -912,   261,   753,  -606,   753,  -719,   419,   523,
     399,   530,   528,   482,  1472,  1056,   431,   536,  -916,   518,
     540,    34,    35,    36,   539,   537,  -717,   666,   547,   548,
     556,   545,   569,  1640,   208,  1641,   577,  1642, -1062,  1074,
     668,  1067,  1643,   580,  1500,  1080,   581,   587,   588,   604,
     165,  1036,   670,  1036,   165,  1036,   613,   603,   606,   616,
    1036,   617,   626,   627,   671,   442,   129,   381,   672,   681,
     441,   180,   181,    65,    66,    67,   682,   683,  1669,   694,
     685,  -128,    55,   707,   796,   753,  1638,   798,    81,    82,
      83,    84,    85,   644,   800,   801,  1163,   807,  1164,   215,
     823,   808,  1551,   824,   558,    89,    90,   831,   575,   846,
     900,  1166,   845,   828,  1216,  1154,   875,   877,  1560,    99,
     878,   880,   129,   879,   881,  1175,  1843,   885,   882,   904,
    1846,   886,   909,   104,   750,  1523,  1793,   905,   669,   908,
     915,   123,   442,   917,   922,   920,   124,   122,   923,  1927,
     925,  1196,   126,  1029,  1036,  1042,   127,   927,   928,   929,
     610,  1204,   930,   941,  1205,   942,  1206,   936,   944,   618,
     705,   623,  -742,   950,   951,   116,   630,   953,   954,   961,
     243,  1177,  1919,   957,   962,   970,   972,   129,   648,  1065,
     116,   975,   976,   785,   161,   816,   978,   123,   981,  1378,
     987,   988,  1919,   990,   991,   999,  -723,  1009,   129,  1007,
    1010,  1941,   983,   165,  1049,  1059,  1073,   679,   248,   249,
    1246,   441,   180,   181,    65,    66,    67,  1011,  1482,  1646,
     250,   116,  1069,  1071,   679,  1077,  1078,  1079,  1652,  1094,
    1140,   697,  1242,  1252,   441,    63,    64,    65,    66,    67,
     666,  1081,  1659,  1100,  1095,    72,   488,  1096,  1097,  1098,
    1099,  1253,   123,   668,  1626,  1145,  1940,  1135,  1149,  1147,
      14,  1155,  1352,  1151,   816,   670,  1152,   669,  1153,  1158,
    1162,  1952,  1301,   123,  1165,  1171,  1173,  1174,  1281,  1178,
    1182,  1847,  1848,   442,  1180,  1285,   116,  1211,   490,   220,
     220,  1199,  1208,   129,  1214,   129,  1219,  -919,  1220,  1036,
    1036,   585,  1230,  1231,  1232,   630,   442,   116,  1233,  1234,
    1235,  1236,  1239,   780,  1240,   813,  1241,  1243,  1800,  1261,
    1353,  1255,  1257,  1260,  1483,   666,  1263,  1354,  1272,  1484,
    1273,   441,  1485,   181,    65,    66,    67,  1486,   668,  1279,
    1280,  1130,  1274,   165,  1283,  1482,  1284,   223,   223,   381,
     670,  1334,  1336,  1124,  1125,  1126,  1127,  1128,  1129,  1341,
    1380,  1224,  1224,  1029,  1337,   124,   122,  1338,   123,  1339,
     123,   126,  1130,   705,  1883,   127,  1348,  1376,  1368,  1487,
    1488,  1377,  1489,  1351,   705,  1354,   995,    14,  1383,  1369,
    1381,  1385,  1387,  1399,   813,  1389,  1401,  1020,  1390,  1392,
     116,  1393,   116,   442,   116,  1400,   129,  1421,  1414,  1423,
    1424,  1426,  1490,   161,  1431,  1441,  1446,  1435,  1444,  1447,
    1450,  1402,   261,  1451,  1452,  1276,  1456,  1455,  1458,  1459,
    1422,  1460,  1436,  1461,  1462,  1466,  1467,  1502,  1513,  1514,
    1464,  1536,  1471,  1540,   679,  1473,  1538,   679,  1503,   585,
    1425,  1483,  1930,  1529,  1516,  1842,  1484,  1518,   441,  1485,
     181,    65,    66,    67,  1486,  1849,  1535,  1474,  1539,  1475,
    1530,  1476,  1520,  1524,  1545,  1528,  1531,  1547,   872,  1550,
    1453,   123,  1534,   973,  1457,  1552,  1542,  1546,  1968,  1463,
     669,  1543,  1544,  1553,  1561,  1584,  1468,  1554,  1573,  1557,
    1558,  1597,   129,   220,  1623,  1617,  1487,  1488,  1627,  1489,
    1884,  1612,  1624,  1632,  1633,   116,  1639,  1614,  1635,  1654,
    1501,  1657,  1663,  1671,  1672,  1766,  1767,  1506,  1773,  1779,
     442,  1507,  1780,  1508,  1794,  1795,  1826,   444,  1782,  1504,
    1783,  1515,  1792,  1004,  1805,  1806,  1832,  1835,  1836,  1841,
    1858,  1522,   705,  1860,  1862,  1906,  1866,  1874,  1876,  1875,
    1881,   223,  1882,  1886,  1889,  1890,  -353,  1892,  1893,  1897,
    1895,  1821,  1898,  1904,   206,   669,  1914,   123,  1029,  1029,
    1029,  1029,  1029,  1029,  1043,  1901,  1044,  1494,  1029,  1908,
    1907,  1909,  1916,  1921,  1499,  1925,    50,  1494,  1537,   116,
     165,  1928,  1541,  1769,  1499,  1929,  1937,  1939,  1943,  1548,
    1964,   116,  1063,  1944,  1946,   165,  1963,  1482,  1967,  1975,
    1953,  1443,  1966,  1976,  1985,  1986,  1988,  1971,  1989,  1340,
     679,  1924,   210,   211,   212,   213,   214,   759,   220,   441,
      63,    64,    65,    66,    67,   754,   756,   220,  1210,  1170,
      72,   488,  1938,  1428,   220,  1799,   165,   399,  1790,    14,
      93,    94,  1559,    95,   184,    97,   220,  1936,   898,  1814,
    1670,  1819,  1978,  1948,  1604,  1831,  1585,   667,  1625,  1788,
    1662,   705,   639,  1150,  1294,  1412,  1226,  1363,   107,  1286,
    1477,   489,   400,   490,  1403,  1244,   223,  1404,  1190,   630,
    1161,   706,  1084,   632,  1980,   223,   491,   624,   492,  1965,
     129,   442,   223,  1899,  1343,  1278,  1333,  1575,     0,     0,
       0,   165,   366,  1483,   223,     0,     0,     0,  1484,     0,
     441,  1485,   181,    65,    66,    67,  1486,   495,     0,     0,
       0,     0,   165,     0,     0,     0,  1029,     0,  1029,     0,
     401,  1664,     0,   402,  1650,     0,     0,  1494,     0,     0,
       0,     0,     0,  1494,  1499,  1494,     0,     0,     0,     0,
    1499,  1834,  1499,     0,     0,   679,     0,     0,  1487,  1488,
       0,  1489,     0,     0,     0,   123,     0,  1494,  1781,   129,
       0,     0,     0,   220,  1499,     0,     0,     0,   129,     0,
       0,   349,   442,     0,     0,     0,  1606,     0,     0,     0,
       0,  1630,     0,     0,     0,     0,     0,     0,     0,   116,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     342,     0,     0,     0,     0,   165,  1602,   165,     0,   165,
       0,  1063,  1259,     0,     0,  1797,  1650,     0,     0,     0,
       0,   223,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   123,     0,     0,     0,     0,     0,
       0,     0,     0,   123,     0,  1494,     0,     0,     0,     0,
       0,     0,  1499,  1029,   129,  1029,     0,  1029,     0,     0,
     129,     0,  1029,   217,   217,     0,     0,   129,   116,     0,
       0,     0,     0,   116,   240,     0,     0,   116,  1829,     0,
       0,  1912,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   206,     0,   207,    40,   381,     0,  1774,   585,     0,
     240,   342,     0,     0,     0,     0,  1879,     0,     0,   343,
       0,  1755,     0,    50,  1837,     0,     0,     0,  1762,     0,
     165,     0,     0,     0,     0,   342,   444,   342,     0,   123,
       0,     0,     0,   342,     0,   123,     0,     0,     0,     0,
       0,   220,   123,     0,     0,     0,  1379,     0,     0,   210,
     211,   212,   213,   214,     0,     0,  1029,   218,   218,     0,
       0,     0,     0,   116,   116,   116,     0,     0,     0,   116,
       0,     0,  1859,  1861,   206,   778,   116,    93,    94,     0,
      95,   184,    97,     0,   345,     0,     0,     0,     0,   366,
     366,   366,   600,   366,     0,     0,    50,  1420,   573,   223,
     574,   220,     0,     0,   165,   107,     0,     0,   129,   779,
       0,   111,   630,  1063,     0,     0,   165, -1063, -1063, -1063,
   -1063, -1063,  1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,
       0,   652,   210,   211,   212,   213,   214,     0,     0,     0,
       0,     0,   220,  1130,   220,     0,     0,     0,     0,     0,
     129,     0,     0,     0,     0,     0,   579,   129,  1760,   223,
      93,    94,  1761,    95,   184,    97,     0,     0,     0,     0,
     220,     0,     0,     0,     0,     0,     0,   217,     0,     0,
       0,     0,     0,   123,     0,     0,   585,     0,   107,  1601,
       0,     0,   129,     0,     0,     0,     0,     0,  1977,     0,
     223,  1913,   223,   630,     0,     0,     0,   342,     0,     0,
    1987,  1029,  1029,     0,   129,   679,     0,   116,     0,     0,
    1990,     0,     0,  1991,     0,   123,  1853,   240,   223,   240,
       0,     0,   123,  1755,  1755,   679,     0,  1762,  1762,     0,
       0,   220,     0,     0,   679,     0,   700,     0,     0,   349,
       0,   381,     0,     0,     0,     0,     0,   220,   220,   116,
       0,     0,     0,     0,     0,     0,   116,   123,     0,     0,
       0,   218,     0,     0,   129,     0,     0,     0,     0,   129,
       0,     0,     0,     0,     0,   240,     0,     0,     0,   123,
       0,   667,     0,     0,     0,     0,     0,     0,     0,   223,
       0,   116,     0,     0,     0,     0,     0,     0,     0,  1911,
       0,   345,   217,   345,     0,   223,   223,     0,     0,     0,
       0,   217,     0,   116,   165,     0,     0,     0,   217,  1926,
       0,     0,  1482,     0,     0,   894,   894,     0,     0,     0,
     217,     0,     0,     0,     0,     0,     0,     0,     0,   123,
       0,   217,     0,     0,   123,     0,   526,   497,   498,   499,
     500,   501,   502,   503,   504,   505,   506,   507,   508,   345,
       0,     0,     0,     0,    14,   240,     0,     0,   240,     0,
       0,     0,     0,   116,     0,     0,     0,     0,   116,     0,
     837,     0,     0,     0,     0,     0,     0,     0,     0,   220,
     220,   509,   510,   165,     0,     0,   218,     0,   165,     0,
       0,     0,   165,     0,  1482,   218,     0,     0,     0,     0,
       0,     0,   218,     0,     0,   240,     0,     0,     0,  1482,
       0,     0,     0,     0,   218,     0,     0,     0,  1483,     0,
     667,     0,     0,  1484,     0,   441,  1485,   181,    65,    66,
      67,  1486,     0,     0,     0,     0,    14,   223,   223,   345,
       0,     0,   345,     0,     0,     0,     0,   217,     0,     0,
       0,    14,     0,     0,     0,     0,   511,   512,     0,     0,
       0,     0,     0,     0,     0,     0,   955,   956,     0,     0,
       0,     0,     0,  1487,  1488,   964,  1489,     0,   165,   165,
     165,     0,     0,     0,   165,     0,     0,     0,     0,     0,
       0,   165,     0,     0,     0,     0,     0,   442,     0,   240,
    1483,   240,  1482,     0,   860,  1484,  1634,   441,  1485,   181,
      65,    66,    67,  1486,   220,  1483,     0,     0,     0,   684,
    1484,     0,   441,  1485,   181,    65,    66,    67,  1486,     0,
       0,     0,     0,     0,     0,     0,     0,   860,     0,     0,
       0,   218,     0,     0,    14,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1487,  1488,     0,  1489,   667,
       0,     0,     0,     0,     0,   220,     0,     0,     0,     0,
    1487,  1488,   223,  1489,     0,     0,     0,     0,     0,   442,
     220,   220,     0,   345,     0,   841,     0,     0,  1636,   894,
       0,   894,     0,   894,   442,   240,   240,   894,     0,   894,
     894,  1144,     0,  1645,   240,     0,     0,     0,  1483,     0,
       0,     0,     0,  1484,     0,   441,  1485,   181,    65,    66,
      67,  1486,     0,   223,     0,   217,     0,     0,     0,     0,
       0,     0,   165,     0,     0,     0,     0,     0,   223,   223,
     700,   700,     0,     0,     0,     0,   526,   497,   498,   499,
     500,   501,   502,   503,   504,   505,   506,   507,   508,     0,
       0,     0,     0,  1487,  1488,     0,  1489,     0,     0,     0,
       0,   220,     0,     0,   165,     0,     0,     0,     0,   345,
     345,   165,     0,     0,     0,   217,     0,   442,   345,     0,
       0,   509,   510,     0,     0,     0,  1791,   526,   497,   498,
     499,   500,   501,   502,   503,   504,   505,   506,   507,   508,
       0,     0,     0,     0,     0,     0,   165,     0,     0,   218,
     240,     0,     0,     0,     0,     0,   217,     0,   217,   223,
       0,     0,     0,     0,     0,     0,  1169,     0,   165,     0,
       0,     0,   509,   510,     0,     0,   206,     0,     0,     0,
       0,     0,  1179,     0,   217,   860,     0,     0,     0,     0,
       0,     0,   240,     0,     0,  1193,   511,   512,    50,   240,
     240,   860,   860,   860,   860,   860,     0,     0,     0,   218,
       0,     0,   667,   860,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1600,  1213,     0,     0,     0,   165,   240,
       0,     0,     0,   165,   210,   211,   212,   213,   214,     0,
       0,     0,     0,     0,     0,     0,     0,   511,   512,     0,
     218,   283,   218,     0,     0,   217,     0,     0,     0,   809,
       0,     0,    93,    94,     0,    95,   184,    97,     0,   240,
       0,   217,   217,     0,     0,     0,  1076,     0,   218,     0,
       0,     0,     0,   345,   345,     0,     0,   894,   285,     0,
     107,  1601,     0,     0,     0,   240,   240,   667,  1271,     0,
       0,   206,  1275,     0,     0,   217,     0,     0,     0,     0,
     906,   240,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,   240,     0,     0,     0,     0,     0,
       0,  -400,   860,     0,     0,   240,     0,     0,     0,   441,
     180,   181,    65,    66,    67,     0,     0,     0,     0,   218,
       0,     0,     0,   240,     0,     0,     0,   240,   571,   210,
     211,   212,   213,   214,   572,   218,   218,     0,     0,     0,
     240,     0,     0,     0,     0,     0,     0,     0,     0,   345,
       0,   183,     0,     0,    91,   336,     0,    93,    94,     0,
      95,   184,    97,   219,   219,   345,     0,     0,     0,     0,
       0,     0,     0,     0,   242,   340,     0,  1371,   345,   964,
       0,   442,     0,   217,   217,   107,   341,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   240,     0,     0,
       0,   240,     0,   240,     0,     0,  1391,   345,     0,  1394,
       0,   838,     0,     0,     0,     0,     0,     0,   860,   860,
     860,   860,   860,   860,   217,     0,     0,   860,   860,   860,
     860,   860,   860,   860,   860,   860,   860,   860,   860,   860,
     860,   860,   860,   860,   860,   860,   860,   860,   860,   860,
     860,   860,   860,   860,   860,     0,     0,     0,     0,     0,
       0,   206,     0,     0,     0,     0,     0,     0,  1442,     0,
       0,   839,     0,   860,  1193,     0,     0,   218,   218,     0,
       0,   345,     0,    50,     0,   345,     0,   841, -1063, -1063,
   -1063, -1063, -1063,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,     0,     0,     0,   240,     0,   240,     0,
       0,     0,     0,     0,   482,     0,  1586,     0,   217,   210,
     211,   212,   213,   214,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   240,     0,     0,   240,  1478,
    1479,   183,     0,     0,    91,     0,     0,    93,    94,     0,
      95,   184,    97,     0,   840,   240,   240,   240,   240,   240,
     240,     0,     0,   217,     0,   240,   206,     0,     0,   217,
       0,     0,     0,     0,     0,   107,     0,   219,     0,     0,
       0,     0,     0,     0,   217,   217,     0,   860,    50,     0,
     345,     0,   345,     0,     0,     0,     0,   240,     0,     0,
       0,     0,     0,   240,     0,     0,   860,     0,   860,     0,
    1587,     0,   218,     0,     0,     0,     0,     0,     0,   345,
       0,     0,   345,  1588,   210,   211,   212,   213,   214,  1589,
       0,     0,   860,     0,     0,     0,     0,     0,     0,  1562,
       0,  1563,     0,     0,     0,     0,   183,     0,     0,    91,
    1590,     0,    93,    94,     0,    95,  1591,    97,     0,     0,
       0,     0,     0,   218,     0,     0,     0,     0,   240,   240,
       0,     0,   240,   206,     0,   217,     0,     0,   218,   218,
     107,   345,     0,     0,     0,     0,     0,   345,     0,  1607,
       0,     0,     0,   283,     0,    50,   496,   497,   498,   499,
     500,   501,   502,   503,   504,   505,   506,   507,   508,     0,
       0,     0,   219,     0,     0,     0,     0,     0,     0,     0,
       0,   219,     0,   240,     0,   240,     0,     0,   219,     0,
     285,   210,   211,   212,   213,   214,     0,     0,     0,     0,
     219,   509,   510,   206,     0,     0,     0,     0,     0,     0,
       0,   219,   345,   345,     0,     0,  1653,     0,     0,    93,
      94,     0,    95,   184,    97,    50,     0,     0,   240,   218,
     240,     0,     0,   578,     0,     0,   860,     0,   860,     0,
     860,     0,     0,     0,     0,   860,   217,   107,   707,   860,
       0,   860,     0,     0,   860,     0,     0,     0,     0,     0,
     571,   210,   211,   212,   213,   214,   572,   240,   240,     0,
       0,   240,     0,     0,     0,     0,   511,   512,   240,     0,
       0,     0,     0,   183,     0,   242,    91,   336,     0,    93,
      94,     0,    95,   184,    97,     0,     0,   283,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   340,     0,     0,
       0,     0,   345,     0,   345,     0,     0,   107,   341,     0,
     240,     0,   240,     0,   240,     0,  1810,   219,     0,   240,
       0,   217,     0,     0,   285,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   240,     0,   206,   860,     0,
       0,   345,     0,     0,     0,     0,     0,     0,     0,     0,
     240,   240,   345,     0,     0,     0,     0,     0,   240,    50,
     240,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   867,     0,     0,     0,     0,     0,
       0,     0,   240,     0,   240,     0,     0,     0,     0,     0,
     240,     0,     0,     0,   571,   210,   211,   212,   213,   214,
     572,     0,     0,     0,     0,     0,     0,   867,     0,     0,
       0,     0,     0,   240,     0,     0,     0,   183,  1833,   345,
      91,   336,     0,    93,    94,     0,    95,   184,    97,     0,
     860,   860,   860,     0,     0,     0,     0,   860,     0,   240,
       0,   340,   345,     0,     0,   240,     0,   240,     0,     0,
       0,   107,   341,   526,   497,   498,   499,   500,   501,   502,
     503,   504,   505,   506,   507,   508,   345,     0,   345,  1221,
    1222,  1223,   206,     0,   345,     0,     0,     0,     0,     0,
       0,   453,   454,   455,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,   219,     0,     0,   509,   510,
       0,   456,   457,  1894,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,   345,
     210,   211,   212,   213,   214,     0,     0,     0,     0,     0,
     482,     0,     0,     0,     0,     0,     0,   240,     0,     0,
       0,     0,     0,     0,     0,   219,     0,     0,    93,    94,
       0,    95,   184,    97,   240,     0,     0,     0,   240,   240,
       0,     0,     0,   511,   512,   206,   221,   221,     0,   964,
       0,     0,     0,   240,     0,     0,   107,   246,     0,   860,
    1035,  1956,     0,   964,     0,     0,   219,    50,   219,     0,
     860,     0,     0,     0,     0,     0,   860,     0,     0,     0,
     860,     0,  1956,     0,  1981,     0,     0,     0,     0,     0,
     283,     0,     0,     0,   219,   867,     0,     0,     0,     0,
       0,   345,   240,   210,   211,   212,   213,   214,     0,     0,
       0,   867,   867,   867,   867,   867,     0,     0,   345,     0,
       0,     0,     0,   867,     0,   183,     0,   285,    91,     0,
       0,    93,    94,     0,    95,   184,    97,  1854,     0,  1134,
     206,     0,   860,     0,  1356,     0,     0,     0,     0,     0,
       0,     0,   240,     0,     0,     0,     0,     0,     0,   107,
       0,     0,    50,     0,  1852,   219,     0,     0,     0,   240,
       0,     0,     0,     0,     0,     0,     0,     0,   240,  1157,
       0,   219,   219,     0,     0,     0,   345,     0,     0,     0,
     240,     0,   240,     0,     0,     0,     0,   571,   210,   211,
     212,   213,   214,   572,     0,     0,  1157,     0,     0,     0,
       0,   240,     0,   240,     0,   219,     0,     0,     0,     0,
     183,     0,     0,    91,   336,     0,    93,    94,     0,    95,
     184,    97,     0,  1082,     0,     0,     0,     0,     0,     0,
       0,     0,   867,     0,   340,  1200,     0,     0,     0,     0,
       0,     0,     0,     0,   107,   341,     0,     0,     0,     0,
     221,     0,   345,     0,     0,     0,     0,   242,     0,     0,
       0,     0,     0,     0,   345,     0,   345,     0,     0,     0,
    1035,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   345,     0,   345,     0,     0,
       0,     0,     0,     0,     0,     0,   453,   454,   455,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   219,   219,     0,   456,   457,  1437,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,     0,     0,     0,     0,   867,   867,
     867,   867,   867,   867,   219,   482,     0,   867,   867,   867,
     867,   867,   867,   867,   867,   867,   867,   867,   867,   867,
     867,   867,   867,   867,   867,   867,   867,   867,   867,   867,
     867,   867,   867,   867,   867,   221,     0,     0,     0,     0,
       0,     0,     0,     0,   221,     0,     0,     0,     0,     0,
       0,   221,     0,   867,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   221,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   246,     0,     0,     0,     0,     0,
       0,     0,   453,   454,   455,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   219,     0,
       0,     0,   456,   457,     0,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
       0,     0,     0,     0,  1438,  1035,  1035,  1035,  1035,  1035,
    1035,   482,     0,   219,     0,  1035,     0,     0,   246,   219,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   219,   219,     0,   867,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   206,     0,   867,     0,   867,     0,
     221,     0,     0,     0,   453,   454,   455,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,   867,     0,   456,   457,     0,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,   210,   211,   212,   213,   214,   868,     0,     0,
       0,     0,  1481,   482,     0,   219,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      93,    94,   937,    95,   184,    97,     0,     0,     0,     0,
     868,     0,     0,     0,     0,     0,     0,   453,   454,   455,
       0,     0,     0,     0,     0,     0,     0,     0,   107,   983,
       0,     0,     0,  1035,     0,  1035,     0,   456,   457,     0,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,     0,   481,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   869,   482,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   867,     0,   867,     0,
     867,     0,     0,     0,     0,   867,   219,     0,   221,   867,
       0,   867,     0,     0,   867,   453,   454,   455,   899,     0,
       0,     0,     0,     0,   969,     0,     0,     0,  1583,     0,
       0,  1596,     0,     0,     0,   456,   457,     0,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,     0,     0,     0,     0,     0,   221,     0,
       0,     0,     0,     0,   482,     0,     0,     0,     0,     0,
    1035,     0,  1035,     0,  1035,   206,     0,     0,     0,  1035,
       0,   219,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   206,     0,   207,    40,     0,     0,    50,   867,   221,
       0,   221,     0,     0,     0,   887,   888,  1008,     0,     0,
    1660,  1661,     0,    50,     0,     0,     0,     0,     0,     0,
    1596,     0,   283,     0,     0,     0,     0,   221,   868,     0,
       0,     0,     0,   210,   211,   212,   213,   214,     0,     0,
       0,     0,     0,     0,   868,   868,   868,   868,   868,   210,
     211,   212,   213,   214,     0,     0,   868,     0,   889,   285,
       0,    93,    94,     0,    95,   184,    97,     0,     0,     0,
       0,     0,   206,  1035,     0,   778,     0,    93,    94,     0,
      95,   184,    97,     0,     0,     0,     0,     0,     0,   107,
     867,   867,   867,     0,    50,  1012,     0,   867,   221,  1808,
       0,     0,     0,     0,     0,   107,     0,  1596,     0,   812,
       0,   111,     0,     0,   221,   221,     0,     0,     0,     0,
       0,     0,   838,     0,     0,     0,  1064,     0,     0,   571,
     210,   211,   212,   213,   214,   572,     0,     0,     0,     0,
       0,     0,  1087,  1088,  1089,  1090,  1091,     0,   246,     0,
       0,     0,   183,     0,  1101,    91,   336,     0,    93,    94,
       0,    95,   184,    97,     0,  1445,     0,     0,     0,     0,
       0,     0,   206,     0,     0,   868,   340,     0,   453,   454,
     455,     0,   839,     0,     0,     0,   107,   341,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,   456,   457,
     246,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,     0,     0,  1035,  1035,
     210,   211,   212,   213,   214,     0,     0,   482,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   867,
       0,     0,   183,     0,     0,    91,   221,   221,    93,    94,
     867,    95,   184,    97,     0,  1277,   867,     0,     0,     0,
     867,     0,     0,  1197,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   107,     0,     0,     0,
       0,   868,   868,   868,   868,   868,   868,   246,     0,     0,
     868,   868,   868,   868,   868,   868,   868,   868,   868,   868,
     868,   868,   868,   868,   868,   868,   868,   868,   868,   868,
     868,   868,   868,   868,   868,   868,   868,   868,     0,     0,
       0,     0,   867,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1923,     0,     0,     0,   868,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1583,
       0,     0,     0,     0,     0,     0,     0,     0,  1148,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1091,
    1289,   221,     0,  1289,     0,     0,     0,     0,  1302,  1305,
    1306,  1307,  1309,  1310,  1311,  1312,  1313,  1314,  1315,  1316,
    1317,  1318,  1319,  1320,  1321,  1322,  1323,  1324,  1325,  1326,
    1327,  1328,  1329,  1330,  1331,  1332,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   246,     0,     0,     0,
       0,     0,   221,     0,  1342,  1014,  1015,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   221,   221,     0,
     868,     0,     0,     0,     0,  1016,     0,     0,     0,     0,
       0,     0,     0,  1017,  1018,  1019,   206,     0,     0,   868,
       0,   868,     0,   453,   454,   455,  1020,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,   456,   457,   868,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,     0,  1021,  1022,  1023,  1024,  1025,  1026,     0,
       0,     0,   482,     0,     0,     0,     0,     0,   221,     0,
       0,  1027,     0,     0,     0,     0,   183,     0,  1432,    91,
      92,     0,    93,    94,     0,    95,   184,    97,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1448,     0,  1449,
    1028,     0,     0,     0,   453,   454,   455,     0,     0,     0,
     107,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1469,   456,   457,     0,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   482,     0,     0,     0,     0,     0,   868,
       0,   868,     0,   868,     0,     0,     0,     0,   868,   246,
       0,     0,   868,     0,   868,     0,     0,   868,   453,   454,
     455,     0,     0,  1207,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   456,   457,
       0,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   482,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   246,     0,     0,  1565,     0,  1566,
       0,  1567,     0,     0,     0,     0,  1568,     0,     0,     0,
    1570,   868,  1571,     0,     0,  1572,     0,     0,     5,     6,
       7,     8,     9,     0,  1218,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,   414,    13,     0,     0,     0,     0,     0,
       0,     0,     0,   761,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,   868,   868,   868,    43,     0,  1248,  1655,
     868,     0,     0,     0,     0,     0,     0,     0,    50,  1813,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,   179,   180,   181,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   182,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   183,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   184,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,  1801,  1802,  1803,     0,   104,   105,   106,  1807,     0,
     107,   108,     0,   453,   454,   455,   111,   112,     0,   113,
     114,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   456,   457,     0,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,   868,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   482,   868,     0,     0,     0,     0,     0,   868,
       0,     0,     0,   868,     0,  1104,  1105,  1106,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1107,  1896,     0,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,
    1129,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,  1130,   868,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
    1863,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1873,     0,     0,     0,     0,    14,  1878,    15,    16,
       0,  1880,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,  1619,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
      56,    57,    58,  1915,    59,    60,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,  1299,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,    88,
      89,    90,    91,    92,     0,    93,    94,     0,    95,    96,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,   103,     0,   104,   105,
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
      53,    54,    55,    56,    57,    58,     0,    59,    60,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,    88,    89,    90,    91,    92,     0,    93,    94,
       0,    95,    96,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,   103,
       0,   104,   105,   106,     0,     0,   107,   108,     0,   109,
     110,  1357,   111,   112,     0,   113,   114,     5,     6,     7,
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
      86,     0,     0,    87,     0,     0,     0,     0,   183,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   184,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,   686,   111,   112,
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
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    1133,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
       0,     0,     0,     0,   183,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   184,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   108,
       0,   109,   110,  1181,   111,   112,     0,   113,   114,     5,
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
       0,     0,    87,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,  1254,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,  1256,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,    98,     0,     0,    99,     0,     0,   100,     0,
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
      48,     0,    49,  1433,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,    98,     0,     0,    99,     0,
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
       0,    87,     0,     0,     0,     0,   183,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   184,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  1574,   111,   112,     0,   113,
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
       0,    86,     0,     0,    87,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   109,   110,  1804,   111,
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
    1850,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   183,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   184,    97,    98,     0,     0,    99,     0,     0,
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
      59,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   183,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   184,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,  1885,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,  1888,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   183,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   184,    97,
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
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    1905,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
       0,     0,     0,     0,   183,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   184,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   108,
       0,   109,   110,  1922,   111,   112,     0,   113,   114,     5,
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
       0,     0,    87,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,  1979,   111,   112,     0,
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
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1982,
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
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,   554,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,    61,    62,   180,   181,    65,    66,    67,
       0,    68,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   183,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   184,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
     825,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,   180,   181,
      65,    66,    67,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   109,   110,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,  1066,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,   180,   181,    65,    66,    67,     0,    68,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   183,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   184,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   108,     0,   109,
     110,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,  1649,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,    61,    62,   180,   181,    65,    66,    67,     0,
      68,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   183,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   184,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,  1796,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,   180,   181,    65,
      66,    67,     0,    68,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   183,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   184,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,    61,    62,
     180,   181,    65,    66,    67,     0,    68,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,     0,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,   179,   180,   181,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     182,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   183,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   184,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   185,
       0,   350,     0,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,  1107,     0,    10,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,
    1128,  1129,     0,     0,   701,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1130,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,   179,   180,   181,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   182,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,     0,
     702,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   185,     0,     0,     0,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,   179,   180,
     181,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   182,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   185,     0,     0,   820,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,     0,     0,
    1194,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1130,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,   179,   180,   181,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   182,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,     0,  1195,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   185,     0,
       0,     0,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,   414,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,   179,   180,   181,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   182,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   183,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   184,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,   453,   454,   455,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   456,   457,     0,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,   482,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,   197,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,   179,   180,   181,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   182,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,  1620,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   185,     0,     0,     0,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,     0,   481,     0,   233,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   482,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
     179,   180,   181,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   182,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   183,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   184,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   185,   453,   454,
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
       0,     0,     0,   179,   180,   181,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   182,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   183,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   184,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,   483,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     185,     0,   268,   454,   455,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,   456,   457,     0,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,   482,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,   179,   180,   181,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   182,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   183,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   184,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   185,     0,   271,     0,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   414,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,   179,
     180,   181,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   182,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,   453,   454,   455,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   456,   457,     0,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,     0,   481,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,   482,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,   179,   180,   181,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     182,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   183,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   184,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,   568,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   185,
     552,     0,     0,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,   715,   481,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   482,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,   179,   180,   181,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   182,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   185,     0,     0,     0,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,
    1126,  1127,  1128,  1129,     0,     0,     0,   761,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1130,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,   179,   180,
     181,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   182,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   185,     0,     0,     0,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,     0,   481,     0,     0,
     802,     0,     0,     0,     0,     0,     0,     0,     0,   482,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,   179,   180,   181,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   182,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   185,     0,
       0,     0,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,
    1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,     0,
       0,     0,     0,   804,     0,     0,     0,     0,     0,     0,
       0,     0,  1130,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,   179,   180,   181,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   182,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   183,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   184,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   185,     0,     0,     0,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,     0,     0,     0,  1245,     0,     0,     0,
       0,     0,     0,     0,   482,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,   179,   180,   181,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   182,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   185,   453,   454,   455,     0,   111,
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
     179,   180,   181,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   182,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   183,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   184,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,   570,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   185,   453,   454,
     455,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,   829,     0,    10,   456,   457,
       0,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,   482,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,   646,    39,    40,     0,
     830,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,   179,   180,   181,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   182,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   183,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   184,    97,     0,   273,   274,
      99,   275,   276,   100,     0,   277,   278,   279,   280,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     185,     0,     0,   281,   282,   111,   112,     0,   113,   114,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,
    1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,     0,
       0,     0,   284,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1130,     0,     0,     0,   286,   287,   288,   289,
     290,   291,   292,     0,     0,     0,   206,     0,   207,    40,
       0,     0,     0,     0,     0,     0,     0,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,    50,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   206,   327,     0,   748,   329,   330,   331,     0,
       0,     0,   332,   582,   210,   211,   212,   213,   214,   583,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
     273,   274,     0,   275,   276,     0,   584,   277,   278,   279,
     280,     0,    93,    94,     0,    95,   184,    97,   337,     0,
     338,     0,     0,   339,     0,   281,   282,     0,     0,     0,
     210,   211,   212,   213,   214,     0,     0,     0,     0,     0,
     107,     0,     0,     0,   749,     0,   111,     0,     0,     0,
       0,     0,   183,     0,   284,    91,    92,     0,    93,    94,
       0,    95,   184,    97,     0,     0,     0,     0,   286,   287,
     288,   289,   290,   291,   292,     0,     0,     0,   206,     0,
     207,    40,     0,     0,     0,     0,   107,     0,     0,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
      50,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   206,   327,     0,   328,   329,   330,
     331,     0,     0,     0,   332,   582,   210,   211,   212,   213,
     214,   583,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,   273,   274,     0,   275,   276,     0,   584,   277,
     278,   279,   280,     0,    93,    94,     0,    95,   184,    97,
     337,     0,   338,     0,     0,   339,     0,   281,   282,     0,
     283,     0,   210,   211,   212,   213,   214,     0,     0,     0,
       0,     0,   107,     0,     0,     0,   749,     0,   111,     0,
       0,     0,     0,     0,     0,     0,   284,     0,   438,     0,
      93,    94,     0,    95,   184,    97,     0,   285,     0,     0,
     286,   287,   288,   289,   290,   291,   292,     0,     0,     0,
     206,     0,     0,     0,     0,     0,     0,     0,   107,     0,
       0,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,    50,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,     0,   327,     0,     0,
     329,   330,   331,     0,     0,     0,   332,   333,   210,   211,
     212,   213,   214,   334,     0,     0,     0,     0,     0,     0,
       0,   206,     0,     0,     0,     0,     0,     0,     0,     0,
     335,  1075,     0,    91,   336,     0,    93,    94,     0,    95,
     184,    97,   337,    50,   338,     0,     0,   339,   273,   274,
       0,   275,   276,     0,   340,   277,   278,   279,   280,     0,
       0,     0,     0,     0,   107,   341,     0,     0,     0,  1775,
       0,     0,     0,   281,   282,     0,   283,     0,     0,   210,
     211,   212,   213,   214,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   183,   284,     0,    91,     0,     0,    93,    94,     0,
      95,   184,    97,   285,     0,     0,   286,   287,   288,   289,
     290,   291,   292,     0,     0,     0,   206,     0,     0,     0,
       0,     0,     0,     0,     0,   107,     0,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,    50,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,     0,   327,     0,     0,   329,   330,   331,     0,
       0,     0,   332,   333,   210,   211,   212,   213,   214,   334,
       0,     0,     0,     0,     0,     0,     0,   206,     0,   931,
       0,   932,     0,     0,     0,     0,   335,     0,     0,    91,
     336,     0,    93,    94,     0,    95,   184,    97,   337,    50,
     338,     0,     0,   339,   273,   274,     0,   275,   276,     0,
     340,   277,   278,   279,   280,     0,     0,     0,     0,     0,
     107,   341,     0,     0,     0,  1845,     0,     0,     0,   281,
     282,     0,   283,     0,     0,   210,   211,   212,   213,   214,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,   284,     0,
       0,     0,     0,    93,    94,     0,    95,   184,    97,   285,
       0,  1130,   286,   287,   288,   289,   290,   291,   292,     0,
       0,     0,   206,     0,     0,     0,     0,     0,     0,     0,
       0,   107,     0,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,    50,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,     0,   327,
       0,   328,   329,   330,   331,     0,     0,     0,   332,   333,
     210,   211,   212,   213,   214,   334,     0,     0,     0,     0,
       0,     0,     0,   206,     0,     0,     0,     0,     0,     0,
       0,     0,   335,     0,     0,    91,   336,     0,    93,    94,
       0,    95,   184,    97,   337,    50,   338,     0,     0,   339,
     273,   274,     0,   275,   276,     0,   340,   277,   278,   279,
     280,     0,     0,     0,     0,     0,   107,   341,     0,     0,
       0,     0,     0,     0,     0,   281,   282,     0,   283,     0,
       0,   210,   211,   212,   213,   214,     0,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,   284,   481,   360,     0,     0,    93,
      94,     0,    95,   184,    97,   285,     0,   482,   286,   287,
     288,   289,   290,   291,   292,     0,     0,     0,   206,     0,
       0,     0,     0,     0,     0,     0,     0,   107,     0,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
      50,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,     0,   327,     0,     0,   329,   330,
     331,     0,     0,     0,   332,   333,   210,   211,   212,   213,
     214,   334,     0,     0,     0,     0,     0,     0,     0,   206,
       0,     0,     0,     0,     0,     0,     0,     0,   335,     0,
       0,    91,   336,     0,    93,    94,     0,    95,   184,    97,
     337,    50,   338,     0,     0,   339,     0,   273,   274,     0,
     275,   276,   340,  1578,   277,   278,   279,   280,     0,     0,
       0,     0,   107,   341,     0,     0,     0,     0,     0,     0,
       0,     0,   281,   282,     0,   283,     0,   210,   211,   212,
     213,   214,     0,     0,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,
    1129,   284,   889,     0,     0,    93,    94,     0,    95,   184,
      97,     0,   285,     0,  1130,   286,   287,   288,   289,   290,
     291,   292,     0,     0,     0,   206,     0,     0,     0,     0,
       0,     0,     0,   107,     0,     0,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,    50,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,     0,   327,     0,     0,   329,   330,   331,     0,     0,
       0,   332,   333,   210,   211,   212,   213,   214,   334,     0,
       0,     0,     0,     0,     0,     0,   206,     0,     0,     0,
       0,     0,     0,     0,     0,   335,     0,     0,    91,   336,
       0,    93,    94,     0,    95,   184,    97,   337,    50,   338,
       0,     0,   339,  1675,  1676,  1677,  1678,  1679,     0,   340,
    1680,  1681,  1682,  1683,     0,     0,     0,     0,     0,   107,
     341,     0,     0,     0,     0,     0,     0,  1684,  1685,  1686,
       0,     0,     0,     0,   210,   211,   212,   213,   214,     0,
   -1063, -1063, -1063, -1063,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,  1687,   481,     0,
       0,     0,    93,    94,     0,    95,   184,    97,     0,     0,
     482,  1688,  1689,  1690,  1691,  1692,  1693,  1694,     0,     0,
       0,   206,     0,     0,     0,     0,     0,     0,     0,     0,
     107,     0,  1695,  1696,  1697,  1698,  1699,  1700,  1701,  1702,
    1703,  1704,  1705,    50,  1706,  1707,  1708,  1709,  1710,  1711,
    1712,  1713,  1714,  1715,  1716,  1717,  1718,  1719,  1720,  1721,
    1722,  1723,  1724,  1725,  1726,  1727,  1728,  1729,  1730,  1731,
    1732,  1733,  1734,  1735,     0,     0,     0,  1736,  1737,   210,
     211,   212,   213,   214,     0,  1738,  1739,  1740,  1741,  1742,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1743,  1744,  1745,     0,   206,     0,    93,    94,     0,
      95,   184,    97,  1746,     0,  1747,  1748,     0,  1749,     0,
       0,     0,     0,     0,     0,  1750,  1751,    50,  1752,     0,
    1753,  1754,     0,   273,   274,   107,   275,   276,     0,     0,
     277,   278,   279,   280,     0,     0,     0,     0,     0,  1587,
       0,     0,     0,     0,     0,     0,     0,     0,   281,   282,
       0,     0,  1588,   210,   211,   212,   213,   214,  1589,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   183,     0,   284,    91,    92,
       0,    93,    94,     0,    95,  1591,    97,     0,     0,     0,
       0,   286,   287,   288,   289,   290,   291,   292,     0,     0,
       0,   206,     0,     0,     0,     0,     0,     0,     0,   107,
       0,     0,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,    50,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,     0,   327,     0,
     328,   329,   330,   331,     0,     0,     0,   332,   582,   210,
     211,   212,   213,   214,   583,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   273,   274,     0,   275,   276,
       0,   584,   277,   278,   279,   280,     0,    93,    94,     0,
      95,   184,    97,   337,     0,   338,     0,     0,   339,     0,
     281,   282,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   107,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   284,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   286,   287,   288,   289,   290,   291,   292,
       0,     0,     0,   206,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,    50,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,     0,
     327,     0,  1300,   329,   330,   331,     0,     0,     0,   332,
     582,   210,   211,   212,   213,   214,   583,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   273,   274,     0,
     275,   276,     0,   584,   277,   278,   279,   280,     0,    93,
      94,     0,    95,   184,    97,   337,     0,   338,     0,     0,
     339,     0,   281,   282,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   107,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   284,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   286,   287,   288,   289,   290,
     291,   292,     0,     0,     0,   206,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,    50,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,     0,   327,     0,     0,   329,   330,   331,     0,     0,
       0,   332,   582,   210,   211,   212,   213,   214,   583,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   584,     0,     0,     0,     0,
       0,    93,    94,     0,    95,   184,    97,   337,     0,   338,
       0,     0,   339,   453,   454,   455,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   107,
       0,     0,     0,   456,   457,     0,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,   453,   454,   455,     0,     0,     0,     0,     0,     0,
       0,     0,   482,     0,     0,     0,     0,     0,     0,     0,
       0,   456,   457,     0,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,   453,
     454,   455,     0,     0,     0,     0,     0,     0,     0,     0,
     482,     0,     0,     0,     0,     0,     0,     0,     0,   456,
     457,     0,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   482,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   453,
     454,   455,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   456,
     457,   589,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,     0,     0,  1308,
       0,     0,     0,     0,     0,     0,     0,     0,   482,     0,
       0,     0,     0,     0,     0,     0,     0,   847,   848,   593,
       0,     0,     0,   849,     0,   850,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   851,     0,     0,
       0,     0,     0,     0,     0,    34,    35,    36,   206,     0,
       0,     0,   453,   454,   455,     0,     0,     0,   208,     0,
       0,     0,     0,     0,     0,     0,   794,     0,     0,     0,
      50,     0,   456,   457,     0,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
       0,     0,     0,     0,     0,   852,   853,   854,   855,   856,
     857,   482,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,   215,  1060,     0,     0,     0,   183,    89,
      90,    91,    92,     0,    93,    94,   817,    95,   184,    97,
       0,     0,     0,    99,     0,     0,     0,     0,     0,     0,
       0,     0,   858,     0,     0,     0,    29,   104,     0,     0,
       0,     0,   107,   859,    34,    35,    36,   206,     0,   207,
      40,     0,     0,     0,     0,     0,     0,   208,   529,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   209, -1063, -1063, -1063, -1063,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,
    1127,  1128,  1129,  1061,    75,   210,   211,   212,   213,   214,
       0,    81,    82,    83,    84,    85,  1130,     0,     0,     0,
       0,     0,   215,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,     0,
     847,   848,    99,     0,     0,     0,   849,     0,   850,     0,
       0,     0,     0,     0,     0,     0,   104,     0,     0,     0,
     851,   107,   216,     0,     0,     0,     0,   111,    34,    35,
      36,   206,     0,     0,     0,   453,   454,   455,     0,     0,
       0,   208,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,   456,   457,     0,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,     0,     0,     0,     0,     0,   852,   853,
     854,   855,   856,   857,   482,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   215,     0,     0,     0,
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,    29,     0,     0,    99,     0,     0,     0,
       0,    34,    35,    36,   206,   858,   207,    40,     0,     0,
     104,     0,     0,     0,   208,   107,   859,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,   538,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   209,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,   210,   211,   212,   213,   214,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,   215,
       0,     0,     0,     0,   183,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   184,    97,    29,     0,     0,    99,
       0,     0,     0,     0,    34,    35,    36,   206,     0,   207,
      40,     0,     0,   104,     0,     0,     0,   208,   107,   216,
       0,     0,   609,     0,   111,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   209,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   629,    75,   210,   211,   212,   213,   214,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,   215,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,    29,
    1003,     0,    99,     0,     0,     0,     0,    34,    35,    36,
     206,     0,   207,    40,     0,     0,   104,     0,     0,     0,
     208,   107,   216,     0,     0,     0,     0,   111,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   209,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,   210,   211,
     212,   213,   214,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   215,     0,     0,     0,     0,
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,    29,     0,     0,    99,     0,     0,     0,     0,
      34,    35,    36,   206,     0,   207,    40,     0,     0,   104,
       0,     0,     0,   208,   107,   216,     0,     0,     0,     0,
     111,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   209,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1160,
      75,   210,   211,   212,   213,   214,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   215,     0,
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,    29,     0,     0,    99,     0,
       0,     0,     0,    34,    35,    36,   206,     0,   207,    40,
       0,     0,   104,     0,     0,     0,   208,   107,   216,     0,
       0,     0,     0,   111,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   209,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,   210,   211,   212,   213,   214,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,   215,     0,     0,     0,     0,   183,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   184,    97,     0,     0,
       0,    99,     0,     0,   453,   454,   455,     0,     0,     0,
       0,     0,     0,     0,     0,   104,     0,     0,     0,     0,
     107,   216,     0,     0,   456,   457,   111,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,   453,   454,   455,     0,     0,     0,     0,     0,
       0,     0,     0,   482,     0,     0,     0,     0,     0,     0,
       0,     0,   456,   457,     0,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
       0,     0,     0,     0,     0,     0,     0,     0,   453,   454,
     455,   482,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   456,   457,
     921,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,   453,   454,   455,     0,
       0,     0,     0,     0,     0,     0,     0,   482,     0,     0,
       0,     0,     0,     0,     0,     0,   456,   457,   989,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,     0,     0,     0,     0,     0,     0,
       0,     0,   453,   454,   455,   482,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   456,   457,  1045,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
    1104,  1105,  1106,     0,     0,     0,     0,     0,     0,     0,
       0,   482,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1107,  1355,     0,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1124,  1125,  1126,  1127,  1128,  1129,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1104,  1105,  1106,     0,  1130,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1107,     0,  1386,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,
    1129,     0,     0,  1104,  1105,  1106,     0,     0,     0,     0,
       0,     0,     0,     0,  1130,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1107,     0,  1282,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,
    1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1104,  1105,
    1106,     0,  1130,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1107,
       0,  1454,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,
    1126,  1127,  1128,  1129,     0,     0,  1104,  1105,  1106,     0,
       0,     0,     0,     0,     0,     0,     0,  1130,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1107,     0,  1465,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,
    1128,  1129,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1104,  1105,  1106,     0,  1130,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1107,     0,  1564,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,  1124,  1125,  1126,  1127,  1128,  1129,     0,    34,    35,
      36,   206,     0,   207,    40,     0,     0,     0,     0,     0,
    1130,   208,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1656,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   237,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     238,     0,     0,     0,     0,     0,     0,     0,     0,   210,
     211,   212,   213,   214,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   215,  1658,     0,     0,
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,     0,     0,     0,    99,     0,    34,    35,
      36,   206,     0,   207,    40,     0,     0,     0,     0,     0,
     104,   660,     0,     0,     0,   107,   239,     0,     0,     0,
       0,   111,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   209,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   210,
     211,   212,   213,   214,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   215,     0,     0,     0,
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,     0,     0,     0,    99,     0,    34,    35,
      36,   206,     0,   207,    40,     0,     0,     0,     0,     0,
     104,   208,     0,     0,     0,   107,   661,     0,     0,     0,
       0,   662,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   237,
       0,     0,   206,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   210,
     211,   212,   213,   214,    50,    81,    82,    83,    84,    85,
       0,     0,   357,   358,     0,     0,   215,     0,     0,     0,
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,     0,     0,     0,    99,     0,     0,     0,
     210,   211,   212,   213,   214,     0,     0,     0,     0,     0,
     104,     0,     0,     0,     0,   107,   239,     0,     0,     0,
       0,   111,   359,     0,     0,   360,     0,     0,    93,    94,
       0,    95,   184,    97,     0,     0,     0,   453,   454,   455,
       0,     0,     0,     0,     0,     0,     0,   361,     0,     0,
       0,     0,     0,     0,     0,     0,   107,   456,   457,   986,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,     0,   481,   453,   454,   455,     0,     0,
       0,     0,     0,     0,     0,     0,   482,     0,     0,     0,
       0,     0,     0,     0,     0,   456,   457,     0,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,  1104,  1105,  1106,     0,     0,     0,     0,
       0,     0,     0,     0,   482,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1107,  1470,     0,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,
    1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,  1104,
    1105,  1106,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1130,     0,     0,     0,     0,     0,     0,     0,
    1107,     0,     0,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,
    1125,  1126,  1127,  1128,  1129,  1105,  1106,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1130,     0,
       0,     0,     0,     0,     0,  1107,     0,     0,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,
     455,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1130,     0,     0,     0,     0,   456,   457,
       0,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,  1106,   481,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   482,     0,     0,
       0,     0,     0,  1107,     0,     0,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   456,
     457,  1130,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   457,   482,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   482,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   482
};

static const yytype_int16 yycheck[] =
{
       5,     6,   129,     8,     9,    10,    11,    12,    13,   161,
      15,    16,    17,    18,   187,   407,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    56,    31,    30,   407,  1184,
      98,     4,     4,   175,   102,   103,   523,     4,   108,    44,
     407,     4,   678,   166,    33,    57,   674,    52,   542,    54,
    1347,   675,    57,   554,    59,   108,    56,    46,   603,   604,
     128,   188,    51,   481,   962,    30,    30,    57,   704,   240,
     161,   758,     4,  1171,   513,   951,   828,    30,   653,     4,
     835,    86,   513,   108,    32,   517,   518,   108,   982,   590,
     251,  1059,   806,     9,    48,   357,   358,   359,    44,   361,
       9,    32,     9,   108,   998,    14,     9,    14,     9,     9,
       9,   550,     9,   796,    14,     9,   548,     9,     9,   550,
       9,     9,     9,     9,     9,    70,     9,    48,    48,   252,
      70,   796,   185,    38,     9,    48,    48,     9,    90,     9,
      86,     9,     9,    90,     9,    83,     9,     4,     9,     9,
      83,    84,  1046,   115,    38,    83,    83,   134,   135,    70,
     185,    36,   160,   216,   185,  1789,    14,    38,    50,    51,
     181,    81,    38,   160,   160,   676,   134,   135,    83,   160,
     185,     4,   181,   181,    32,   196,   239,   192,     0,   134,
     135,   216,    38,    70,    70,   216,    70,   196,   196,    83,
     181,   199,   196,    51,   102,   157,    70,   166,    38,   196,
     157,   216,    83,   199,   239,    70,   178,    83,   199,    70,
     130,    70,  1846,   200,  1100,    50,    51,    70,    70,   161,
      53,   392,   160,    56,   239,    70,   174,    83,   196,   193,
     199,    70,    83,   197,    83,    84,    70,   174,    70,   254,
      73,   199,   257,    83,   199,   891,   196,    70,   236,   264,
     265,    70,    70,    70,    70,   407,   164,   198,   201,   174,
     443,  1483,   129,    96,   181,    98,   197,   197,  1575,   102,
     103,   193,   198,   199,   197,  1383,   258,  1001,   199,   198,
     262,   198,  1390,  1261,  1392,   198,   160,   198,   198,   198,
     182,   198,    83,   174,   198,   128,   198,   198,   174,   198,
     198,   198,   198,   198,  1069,   198,  1071,   160,   349,   820,
     196,  1215,   197,  1421,   825,   197,   379,   197,   174,   197,
     197,   188,   197,  1016,   197,   199,   197,   197,  1180,   106,
     107,   196,   887,   888,   199,   487,    83,   160,   199,   349,
     199,  1016,   525,   196,   379,   979,   199,   199,   379,   160,
     201,   196,   201,  1239,   199,   940,   164,   196,   495,   437,
     199,   513,  1584,   378,   379,   199,   102,   199,   201,    54,
     385,   386,   387,   388,   389,   390,   199,   196,   197,   160,
     395,   199,   199,   199,   536,   196,  1608,    54,  1610,   196,
     198,   199,   120,   434,    57,   547,   196,   166,   550,   414,
     181,   192,   130,   106,   107,   160,    69,   422,   199,   164,
     179,    70,   490,   491,   492,   493,   496,    83,   433,     8,
     917,  1529,   378,   200,   434,   258,   181,   174,   164,   262,
     199,   387,   388,   266,   390,   105,   106,   107,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   102,   482,   481,   484,
     485,   486,  1169,   102,   496,   134,   135,   487,   413,   421,
     165,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,    14,  1257,   496,   200,   165,   514,
     515,   160,   517,   518,   519,   520,   481,   481,   174,   196,
     681,   526,   683,   180,   529,   199,   349,    83,   481,   968,
     388,    70,   390,   538,   487,   540,   536,   968,   164,  1381,
     692,   181,   196,   548,   556,   164,    83,   196,  1231,  1232,
      83,   556,  1235,   558,   532,  1183,   196,    90,  1241,   951,
     513,   134,   135,   995,   421,  1066,  1231,  1232,  1233,  1234,
    1235,  1236,   951,   181,    83,  1199,  1241,  1097,  1202,  1099,
     751,    90,   196,   536,   951,  1003,  1462,   760,   196,   561,
      75,    76,   102,   103,   547,   134,   135,   550,   421,    83,
    1145,   692,   158,   159,   609,    83,    90,   430,   661,   181,
     196,   434,    90,   784,   437,   877,     4,   879,   196,   881,
     157,   158,   159,   885,   196,   158,   159,    83,    53,    54,
      55,    19,    20,    31,    90,   196,    83,   494,   196,   800,
     801,   196,   710,    90,    69,   121,   807,   808,   485,   158,
     159,   196,    50,    83,   130,    53,   661,   635,   636,  1956,
      90,   105,   106,   107,   487,   488,   489,   490,   491,   492,
     493,  1547,   205,   157,   158,   159,  1518,   514,  1520,   157,
     158,   159,   519,  1234,   807,  1236,   664,    31,   165,   860,
     513,    70,   357,   358,   359,   204,   361,   702,   122,   123,
     124,   872,   158,   159,  1391,    70,    50,    70,  1100,    53,
     715,   158,   159,   536,    75,    76,  1399,    70,  1401,    70,
     108,  1100,   393,  1217,   102,   103,   397,   550,   158,   159,
     112,   113,   114,  1100,  1399,   196,  1401,   199,   561,   132,
     133,    32,  1370,   164,   749,   198,   199,   198,   199,    38,
     675,   196,   423,    70,   425,   426,   427,   428,   581,   198,
     692,  1385,   198,    50,    51,    52,    53,    54,    55,   747,
      57,  1820,  1821,  1260,   779,   198,  1208,  1816,  1817,  1934,
     845,   846,    69,   198,   607,   608,  1628,  1219,   198,   198,
    1632,   198,   198,  1948,    53,    54,    55,   185,    57,    70,
      70,    70,    70,    70,    70,   111,   160,   812,   199,   951,
      69,  1498,   954,   119,   120,   121,   122,   123,   124,   642,
     643,   196,   196,   828,   966,    70,   968,   160,   216,   196,
     164,    49,   198,    69,  1335,   824,   181,   160,   196,   196,
       9,    78,    79,    80,   203,   233,   160,  1239,   160,   196,
       8,   239,   198,  1536,    91,  1538,   196,  1540,   160,   837,
    1239,   833,  1545,    14,  1351,   843,   160,   198,   198,     9,
     258,  1536,  1239,  1538,   262,  1540,    14,   199,   198,   130,
    1545,   130,   197,   181,    14,   191,   818,   710,   102,   197,
     119,   120,   121,   122,   123,   124,   197,   197,  1585,   202,
     197,   196,   111,   196,   196,  1047,  1530,     9,   145,   146,
     147,   148,   149,   157,   197,   197,   921,   197,   923,   156,
     925,   197,  1423,    94,     9,   162,   163,    14,   181,     9,
      83,   936,   196,   198,  1002,   913,   196,   199,  1439,   176,
     198,   198,   874,   199,   199,   950,  1788,   199,   198,   197,
    1792,   198,   198,   190,   954,  1387,  1639,   197,  1100,   197,
     132,   818,   191,   196,   203,   197,   939,   939,     9,   198,
       9,   976,   939,   796,  1639,   798,   939,   203,   203,   203,
     368,   986,   203,    32,   989,   133,   991,    70,   180,   377,
     995,   379,   160,   136,     9,   818,   384,   197,   160,   193,
    1003,   954,  1900,    14,     9,     9,   182,   939,   396,   832,
     833,   197,     9,   966,   939,   968,    14,   874,   132,  1190,
     203,   203,  1920,   200,     9,    14,   160,   197,   960,   203,
     197,  1929,   196,   421,   197,   102,     9,   962,  1003,  1003,
    1045,   119,   120,   121,   122,   123,   124,   203,     6,  1550,
    1003,   874,   198,   198,   979,   136,   160,     9,  1559,   196,
     883,   884,  1040,  1052,   119,   120,   121,   122,   123,   124,
    1462,   197,  1573,   196,    70,   130,   131,    70,    70,    70,
      70,  1053,   939,  1462,  1516,     9,  1928,   199,    14,   200,
      48,   914,  1162,   198,  1047,  1462,   182,  1239,     9,   199,
      14,  1943,  1102,   960,   203,   199,    14,   197,  1086,   198,
      32,  1794,  1795,   191,   193,  1093,   939,    32,   173,    19,
      20,   196,   196,  1055,    14,  1057,   196,   196,    14,  1794,
    1795,   954,    52,   196,    70,   523,   191,   960,    70,    70,
      70,    70,   196,   966,   160,   968,     9,   197,  1649,   136,
    1162,   198,   198,   196,   112,  1547,    14,  1162,   182,   117,
     136,   119,   120,   121,   122,   123,   124,   125,  1547,     9,
     197,    69,   160,   561,   203,     6,     9,    19,    20,  1002,
    1547,    83,   200,    50,    51,    52,    53,    54,    55,   198,
    1195,  1014,  1015,  1016,   200,  1168,  1168,   200,  1055,   200,
    1057,  1168,    69,  1208,  1840,  1168,     9,    14,   136,   167,
     168,    83,   170,   196,  1219,  1220,   196,    48,   199,   198,
     197,   196,   196,   136,  1047,   197,     9,    91,   199,   199,
    1053,   198,  1055,   191,  1057,   203,  1168,   199,   157,    32,
      77,   198,   200,  1168,   197,   182,    32,   198,   136,   197,
     197,  1229,  1257,   203,     9,  1078,     9,   203,   203,   203,
    1249,   203,  1267,   136,     9,   200,     9,   200,    14,    83,
     197,     9,   197,     9,  1199,   198,   136,  1202,   199,  1102,
    1252,   112,  1918,   199,   196,  1786,   117,   197,   119,   120,
     121,   122,   123,   124,   125,  1796,   203,   198,   203,   198,
     196,   198,   197,   197,   136,   198,   197,     9,  1131,    32,
    1288,  1168,   197,   701,  1292,   198,   203,   197,  1954,  1297,
    1462,   203,   203,   197,   136,   112,  1304,   197,   199,   198,
     198,   169,  1264,   233,    14,   165,   167,   168,   117,   170,
    1841,   198,    83,   197,   197,  1168,   136,  1499,   199,   197,
    1355,   136,    14,   181,   199,   198,    83,  1362,    14,    14,
     191,  1366,    83,  1368,   136,   136,    14,  1494,   197,   200,
     196,  1376,   197,   761,   198,   198,    14,   198,    14,   199,
       9,  1386,  1387,     9,   200,  1886,    68,    83,   196,   181,
      83,   233,     9,   199,   198,   115,   102,   160,   102,   172,
     182,    36,    14,   197,    81,  1547,   182,  1264,  1231,  1232,
    1233,  1234,  1235,  1236,   802,   196,   804,  1349,  1241,   196,
     198,   178,   182,    83,  1349,   175,   103,  1359,  1406,  1252,
     818,   197,  1410,  1606,  1359,     9,    83,   198,   197,  1417,
      83,  1264,   830,   197,   195,   833,    14,     6,     9,    14,
     199,  1274,  1953,    83,    14,    83,    14,  1958,    83,  1145,
    1385,  1909,   139,   140,   141,   142,   143,   493,   368,   119,
     120,   121,   122,   123,   124,   488,   490,   377,   996,   942,
     130,   131,  1925,  1258,   384,  1648,   874,   164,  1635,    48,
     167,   168,  1436,   170,   171,   172,   396,  1920,   611,  1673,
    1586,  1758,  1965,  1941,  1489,  1770,  1485,   407,  1513,  1631,
    1578,  1516,   389,   901,  1098,  1235,  1015,  1172,   195,  1094,
    1343,   171,   199,   173,  1230,  1042,   368,  1231,   966,   917,
     918,   434,   845,   385,  1967,   377,   186,   379,   188,  1952,
    1472,   191,   384,  1875,  1153,  1079,  1131,  1477,    -1,    -1,
      -1,   939,    60,   112,   396,    -1,    -1,    -1,   117,    -1,
     119,   120,   121,   122,   123,   124,   125,  1499,    -1,    -1,
      -1,    -1,   960,    -1,    -1,    -1,  1399,    -1,  1401,    -1,
      88,  1581,    -1,    91,  1556,    -1,    -1,  1519,    -1,    -1,
      -1,    -1,    -1,  1525,  1519,  1527,    -1,    -1,    -1,    -1,
    1525,  1774,  1527,    -1,    -1,  1530,    -1,    -1,   167,   168,
      -1,   170,    -1,    -1,    -1,  1472,    -1,  1549,  1623,  1551,
      -1,    -1,    -1,   523,  1549,    -1,    -1,    -1,  1560,    -1,
      -1,    56,   191,    -1,    -1,    -1,  1493,    -1,    -1,    -1,
      -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1472,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1483,    -1,    -1,    -1,    -1,  1053,  1489,  1055,    -1,  1057,
      -1,  1059,  1060,    -1,    -1,  1647,  1648,    -1,    -1,    -1,
      -1,   523,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1551,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1560,    -1,  1637,    -1,    -1,    -1,    -1,
      -1,    -1,  1637,  1536,  1646,  1538,    -1,  1540,    -1,    -1,
    1652,    -1,  1545,    19,    20,    -1,    -1,  1659,  1551,    -1,
      -1,    -1,    -1,  1556,    30,    -1,    -1,  1560,  1769,    -1,
      -1,  1893,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    83,    84,  1578,    -1,  1614,  1581,    -1,
      56,  1584,    -1,    -1,    -1,    -1,  1834,    -1,    -1,  1769,
      -1,  1594,    -1,   103,  1779,    -1,    -1,    -1,  1601,    -1,
    1168,    -1,    -1,    -1,    -1,  1608,  1913,  1610,    -1,  1646,
      -1,    -1,    -1,  1616,    -1,  1652,    -1,    -1,    -1,    -1,
      -1,   701,  1659,    -1,    -1,    -1,  1194,    -1,    -1,   139,
     140,   141,   142,   143,    -1,    -1,  1639,    19,    20,    -1,
      -1,    -1,    -1,  1646,  1647,  1648,    -1,    -1,    -1,  1652,
      -1,    -1,  1810,  1811,    81,   165,  1659,   167,   168,    -1,
     170,   171,   172,    -1,    56,    -1,    -1,    -1,    -1,   357,
     358,   359,   360,   361,    -1,    -1,   103,  1245,   283,   701,
     285,   761,    -1,    -1,  1252,   195,    -1,    -1,  1800,   199,
      -1,   201,  1260,  1261,    -1,    -1,  1264,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,   399,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      -1,    -1,   802,    69,   804,    -1,    -1,    -1,    -1,    -1,
    1842,    -1,    -1,    -1,    -1,    -1,   341,  1849,   165,   761,
     167,   168,   169,   170,   171,   172,    -1,    -1,    -1,    -1,
     830,    -1,    -1,    -1,    -1,    -1,    -1,   233,    -1,    -1,
      -1,    -1,    -1,  1800,    -1,    -1,  1769,    -1,   195,   196,
      -1,    -1,  1884,    -1,    -1,    -1,    -1,    -1,  1963,    -1,
     802,  1893,   804,  1351,    -1,    -1,    -1,  1790,    -1,    -1,
    1975,  1794,  1795,    -1,  1906,  1900,    -1,  1800,    -1,    -1,
    1985,    -1,    -1,  1988,    -1,  1842,  1809,   283,   830,   285,
      -1,    -1,  1849,  1816,  1817,  1920,    -1,  1820,  1821,    -1,
      -1,   901,    -1,    -1,  1929,    -1,   431,    -1,    -1,   434,
      -1,  1834,    -1,    -1,    -1,    -1,    -1,   917,   918,  1842,
      -1,    -1,    -1,    -1,    -1,    -1,  1849,  1884,    -1,    -1,
      -1,   233,    -1,    -1,  1966,    -1,    -1,    -1,    -1,  1971,
      -1,    -1,    -1,    -1,    -1,   341,    -1,    -1,    -1,  1906,
      -1,   951,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   901,
      -1,  1884,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1892,
      -1,   283,   368,   285,    -1,   917,   918,    -1,    -1,    -1,
      -1,   377,    -1,  1906,  1472,    -1,    -1,    -1,   384,  1912,
      -1,    -1,     6,    -1,    -1,   603,   604,    -1,    -1,    -1,
     396,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1966,
      -1,   407,    -1,    -1,  1971,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,   341,
      -1,    -1,    -1,    -1,    48,   431,    -1,    -1,   434,    -1,
      -1,    -1,    -1,  1966,    -1,    -1,    -1,    -1,  1971,    -1,
     575,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1059,
    1060,    59,    60,  1551,    -1,    -1,   368,    -1,  1556,    -1,
      -1,    -1,  1560,    -1,     6,   377,    -1,    -1,    -1,    -1,
      -1,    -1,   384,    -1,    -1,   481,    -1,    -1,    -1,     6,
      -1,    -1,    -1,    -1,   396,    -1,    -1,    -1,   112,    -1,
    1100,    -1,    -1,   117,    -1,   119,   120,   121,   122,   123,
     124,   125,    -1,    -1,    -1,    -1,    48,  1059,  1060,   431,
      -1,    -1,   434,    -1,    -1,    -1,    -1,   523,    -1,    -1,
      -1,    48,    -1,    -1,    -1,    -1,   134,   135,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   671,   672,    -1,    -1,
      -1,    -1,    -1,   167,   168,   680,   170,    -1,  1646,  1647,
    1648,    -1,    -1,    -1,  1652,    -1,    -1,    -1,    -1,    -1,
      -1,  1659,    -1,    -1,    -1,    -1,    -1,   191,    -1,   575,
     112,   577,     6,    -1,   580,   117,   200,   119,   120,   121,
     122,   123,   124,   125,  1194,   112,    -1,    -1,    -1,   197,
     117,    -1,   119,   120,   121,   122,   123,   124,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   613,    -1,    -1,
      -1,   523,    -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   167,   168,    -1,   170,  1239,
      -1,    -1,    -1,    -1,    -1,  1245,    -1,    -1,    -1,    -1,
     167,   168,  1194,   170,    -1,    -1,    -1,    -1,    -1,   191,
    1260,  1261,    -1,   575,    -1,   577,    -1,    -1,   200,   877,
      -1,   879,    -1,   881,   191,   671,   672,   885,    -1,   887,
     888,   889,    -1,   200,   680,    -1,    -1,    -1,   112,    -1,
      -1,    -1,    -1,   117,    -1,   119,   120,   121,   122,   123,
     124,   125,    -1,  1245,    -1,   701,    -1,    -1,    -1,    -1,
      -1,    -1,  1800,    -1,    -1,    -1,    -1,    -1,  1260,  1261,
     845,   846,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    -1,   167,   168,    -1,   170,    -1,    -1,    -1,
      -1,  1351,    -1,    -1,  1842,    -1,    -1,    -1,    -1,   671,
     672,  1849,    -1,    -1,    -1,   761,    -1,   191,   680,    -1,
      -1,    59,    60,    -1,    -1,    -1,   200,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,    -1,    -1,    -1,  1884,    -1,    -1,   701,
     796,    -1,    -1,    -1,    -1,    -1,   802,    -1,   804,  1351,
      -1,    -1,    -1,    -1,    -1,    -1,   941,    -1,  1906,    -1,
      -1,    -1,    59,    60,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,   957,    -1,   830,   831,    -1,    -1,    -1,    -1,
      -1,    -1,   838,    -1,    -1,   970,   134,   135,   103,   845,
     846,   847,   848,   849,   850,   851,    -1,    -1,    -1,   761,
      -1,    -1,  1462,   859,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   128,   999,    -1,    -1,    -1,  1966,   875,
      -1,    -1,    -1,  1971,   139,   140,   141,   142,   143,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,
     802,    31,   804,    -1,    -1,   901,    -1,    -1,    -1,   197,
      -1,    -1,   167,   168,    -1,   170,   171,   172,    -1,   915,
      -1,   917,   918,    -1,    -1,    -1,   838,    -1,   830,    -1,
      -1,    -1,    -1,   845,   846,    -1,    -1,  1145,    68,    -1,
     195,   196,    -1,    -1,    -1,   941,   942,  1547,  1073,    -1,
      -1,    81,  1077,    -1,    -1,   951,    -1,    -1,    -1,    -1,
     197,   957,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,   970,    -1,    -1,    -1,    -1,    -1,
      -1,   111,   978,    -1,    -1,   981,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,    -1,    -1,   901,
      -1,    -1,    -1,   999,    -1,    -1,    -1,  1003,   138,   139,
     140,   141,   142,   143,   144,   917,   918,    -1,    -1,    -1,
    1016,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   941,
      -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    19,    20,   957,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,   185,    -1,  1182,   970,  1184,
      -1,   191,    -1,  1059,  1060,   195,   196,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1073,    -1,    -1,
      -1,  1077,    -1,  1079,    -1,    -1,  1211,   999,    -1,  1214,
      -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,  1094,  1095,
    1096,  1097,  1098,  1099,  1100,    -1,    -1,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,
    1126,  1127,  1128,  1129,  1130,    -1,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,  1273,    -1,
      -1,    91,    -1,  1149,  1279,    -1,    -1,  1059,  1060,    -1,
      -1,  1073,    -1,   103,    -1,  1077,    -1,  1079,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,  1182,    -1,  1184,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    31,    -1,  1194,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1211,    -1,    -1,  1214,  1344,
    1345,   161,    -1,    -1,   164,    -1,    -1,   167,   168,    -1,
     170,   171,   172,    -1,   174,  1231,  1232,  1233,  1234,  1235,
    1236,    -1,    -1,  1239,    -1,  1241,    81,    -1,    -1,  1245,
      -1,    -1,    -1,    -1,    -1,   195,    -1,   233,    -1,    -1,
      -1,    -1,    -1,    -1,  1260,  1261,    -1,  1263,   103,    -1,
    1182,    -1,  1184,    -1,    -1,    -1,    -1,  1273,    -1,    -1,
      -1,    -1,    -1,  1279,    -1,    -1,  1282,    -1,  1284,    -1,
     125,    -1,  1194,    -1,    -1,    -1,    -1,    -1,    -1,  1211,
      -1,    -1,  1214,   138,   139,   140,   141,   142,   143,   144,
      -1,    -1,  1308,    -1,    -1,    -1,    -1,    -1,    -1,  1444,
      -1,  1446,    -1,    -1,    -1,    -1,   161,    -1,    -1,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,    -1,    -1,  1245,    -1,    -1,    -1,    -1,  1344,  1345,
      -1,    -1,  1348,    81,    -1,  1351,    -1,    -1,  1260,  1261,
     195,  1273,    -1,    -1,    -1,    -1,    -1,  1279,    -1,  1494,
      -1,    -1,    -1,    31,    -1,   103,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,   368,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   377,    -1,  1399,    -1,  1401,    -1,    -1,   384,    -1,
      68,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
     396,    59,    60,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   407,  1344,  1345,    -1,    -1,  1561,    -1,    -1,   167,
     168,    -1,   170,   171,   172,   103,    -1,    -1,  1444,  1351,
    1446,    -1,    -1,   111,    -1,    -1,  1452,    -1,  1454,    -1,
    1456,    -1,    -1,    -1,    -1,  1461,  1462,   195,   196,  1465,
      -1,  1467,    -1,    -1,  1470,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,   144,  1483,  1484,    -1,
      -1,  1487,    -1,    -1,    -1,    -1,   134,   135,  1494,    -1,
      -1,    -1,    -1,   161,    -1,   481,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    -1,  1444,    -1,  1446,    -1,    -1,   195,   196,    -1,
    1536,    -1,  1538,    -1,  1540,    -1,  1671,   523,    -1,  1545,
      -1,  1547,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1561,    -1,    81,  1564,    -1,
      -1,  1483,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1576,  1577,  1494,    -1,    -1,    -1,    -1,    -1,  1584,   103,
    1586,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   580,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1608,    -1,  1610,    -1,    -1,    -1,    -1,    -1,
    1616,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,   613,    -1,    -1,
      -1,    -1,    -1,  1639,    -1,    -1,    -1,   161,  1773,  1561,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
    1656,  1657,  1658,    -1,    -1,    -1,    -1,  1663,    -1,  1665,
      -1,   185,  1584,    -1,    -1,  1671,    -1,  1673,    -1,    -1,
      -1,   195,   196,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,  1608,    -1,  1610,    78,
      79,    80,    81,    -1,  1616,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,   701,    -1,    -1,    59,    60,
      -1,    30,    31,  1858,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,  1671,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,  1773,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   761,    -1,    -1,   167,   168,
      -1,   170,   171,   172,  1790,    -1,    -1,    -1,  1794,  1795,
      -1,    -1,    -1,   134,   135,    81,    19,    20,    -1,  1934,
      -1,    -1,    -1,  1809,    -1,    -1,   195,    30,    -1,  1815,
     796,  1946,    -1,  1948,    -1,    -1,   802,   103,   804,    -1,
    1826,    -1,    -1,    -1,    -1,    -1,  1832,    -1,    -1,    -1,
    1836,    -1,  1967,    -1,  1969,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,   830,   831,    -1,    -1,    -1,    -1,
      -1,  1773,  1858,   139,   140,   141,   142,   143,    -1,    -1,
      -1,   847,   848,   849,   850,   851,    -1,    -1,  1790,    -1,
      -1,    -1,    -1,   859,    -1,   161,    -1,    68,   164,    -1,
      -1,   167,   168,    -1,   170,   171,   172,  1809,    -1,   875,
      81,    -1,  1898,    -1,   203,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1908,    -1,    -1,    -1,    -1,    -1,    -1,   195,
      -1,    -1,   103,    -1,   200,   901,    -1,    -1,    -1,  1925,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1934,   915,
      -1,   917,   918,    -1,    -1,    -1,  1858,    -1,    -1,    -1,
    1946,    -1,  1948,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,   942,    -1,    -1,    -1,
      -1,  1967,    -1,  1969,    -1,   951,    -1,    -1,    -1,    -1,
     161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,   174,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   978,    -1,   185,   981,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,
     233,    -1,  1934,    -1,    -1,    -1,    -1,  1003,    -1,    -1,
      -1,    -1,    -1,    -1,  1946,    -1,  1948,    -1,    -1,    -1,
    1016,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1967,    -1,  1969,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1059,  1060,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,  1094,  1095,
    1096,  1097,  1098,  1099,  1100,    69,    -1,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,
    1126,  1127,  1128,  1129,  1130,   368,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   377,    -1,    -1,    -1,    -1,    -1,
      -1,   384,    -1,  1149,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   396,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   407,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1194,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,   198,  1231,  1232,  1233,  1234,  1235,
    1236,    69,    -1,  1239,    -1,  1241,    -1,    -1,   481,  1245,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1260,  1261,    -1,  1263,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    -1,  1282,    -1,  1284,    -1,
     523,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,  1308,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,   139,   140,   141,   142,   143,   580,    -1,    -1,
      -1,    -1,  1348,    69,    -1,  1351,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     167,   168,   200,   170,   171,   172,    -1,    -1,    -1,    -1,
     613,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,   196,
      -1,    -1,    -1,  1399,    -1,  1401,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   580,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1452,    -1,  1454,    -1,
    1456,    -1,    -1,    -1,    -1,  1461,  1462,    -1,   701,  1465,
      -1,  1467,    -1,    -1,  1470,    10,    11,    12,   613,    -1,
      -1,    -1,    -1,    -1,   200,    -1,    -1,    -1,  1484,    -1,
      -1,  1487,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,   761,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
    1536,    -1,  1538,    -1,  1540,    81,    -1,    -1,    -1,  1545,
      -1,  1547,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    83,    84,    -1,    -1,   103,  1564,   802,
      -1,   804,    -1,    -1,    -1,   111,   112,   200,    -1,    -1,
    1576,  1577,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
    1586,    -1,    31,    -1,    -1,    -1,    -1,   830,   831,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,    -1,   847,   848,   849,   850,   851,   139,
     140,   141,   142,   143,    -1,    -1,   859,    -1,   164,    68,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
      -1,    -1,    81,  1639,    -1,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,    -1,    -1,    -1,   195,
    1656,  1657,  1658,    -1,   103,   200,    -1,  1663,   901,  1665,
      -1,    -1,    -1,    -1,    -1,   195,    -1,  1673,    -1,   199,
      -1,   201,    -1,    -1,   917,   918,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,   831,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,   847,   848,   849,   850,   851,    -1,   951,    -1,
      -1,    -1,   161,    -1,   859,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,   978,   185,    -1,    10,    11,
      12,    -1,    91,    -1,    -1,    -1,   195,   196,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    30,    31,
    1003,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,  1794,  1795,
     139,   140,   141,   142,   143,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1815,
      -1,    -1,   161,    -1,    -1,   164,  1059,  1060,   167,   168,
    1826,   170,   171,   172,    -1,   174,  1832,    -1,    -1,    -1,
    1836,    -1,    -1,   978,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,
      -1,  1094,  1095,  1096,  1097,  1098,  1099,  1100,    -1,    -1,
    1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,  1124,  1125,  1126,  1127,  1128,  1129,  1130,    -1,    -1,
      -1,    -1,  1898,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1908,    -1,    -1,    -1,  1149,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1925,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1094,
    1095,  1194,    -1,  1098,    -1,    -1,    -1,    -1,  1103,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,
    1125,  1126,  1127,  1128,  1129,  1130,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1239,    -1,    -1,    -1,
      -1,    -1,  1245,    -1,  1149,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1260,  1261,    -1,
    1263,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,    -1,  1282,
      -1,  1284,    -1,    10,    11,    12,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    30,    31,  1308,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,   138,   139,   140,   141,   142,   143,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,  1351,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,    -1,  1263,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1282,    -1,  1284,
     185,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
     195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1308,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,  1452,
      -1,  1454,    -1,  1456,    -1,    -1,    -1,    -1,  1461,  1462,
      -1,    -1,  1465,    -1,  1467,    -1,    -1,  1470,    10,    11,
      12,    -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1547,    -1,    -1,  1452,    -1,  1454,
      -1,  1456,    -1,    -1,    -1,    -1,  1461,    -1,    -1,    -1,
    1465,  1564,  1467,    -1,    -1,  1470,    -1,    -1,     3,     4,
       5,     6,     7,    -1,   200,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,  1656,  1657,  1658,    91,    -1,   200,  1564,
    1663,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,  1672,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
     185,  1656,  1657,  1658,    -1,   190,   191,   192,  1663,    -1,
     195,   196,    -1,    10,    11,    12,   201,   202,    -1,   204,
     205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,  1815,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,  1826,    -1,    -1,    -1,    -1,    -1,  1832,
      -1,    -1,    -1,  1836,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,  1860,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    69,  1898,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
    1815,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1826,    -1,    -1,    -1,    -1,    48,  1832,    50,    51,
      -1,  1836,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,   200,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,
      -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
     112,   113,   114,  1898,   116,   117,   118,   119,   120,   121,
     122,   123,   124,    -1,   126,   127,   128,   129,   130,   131,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,   197,   145,   146,   147,   148,   149,    -1,    -1,
      -1,   153,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,   173,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,
      -1,    -1,    -1,   185,   186,    -1,   188,    -1,   190,   191,
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
     109,   110,   111,   112,   113,   114,    -1,   116,   117,   118,
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
      91,    92,    93,    94,    95,    96,    -1,    98,    -1,   100,
      -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,    -1,   113,   114,    -1,   116,    -1,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,   127,   128,    -1,   130,
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,   153,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,   173,    -1,    -1,   176,    -1,    -1,   179,    -1,
      -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,
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
      98,    -1,   100,   101,    -1,   103,   104,    -1,    -1,    -1,
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
      99,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
     109,   110,   111,    -1,   113,   114,    -1,   116,    -1,   118,
     119,   120,   121,   122,   123,   124,    -1,   126,   127,   128,
      -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,   153,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,   173,    -1,    -1,   176,    -1,    -1,
     179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,   198,
     199,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
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
      93,    94,    -1,    96,    97,    98,    -1,   100,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      27,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   198,    -1,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    31,    -1,    13,
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
      -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,
     124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
     174,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,
      -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,
      -1,   195,   196,    -1,    -1,    -1,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
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
     191,   192,    -1,    -1,   195,   196,    -1,    -1,   199,    -1,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
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
     168,    -1,   170,   171,   172,    -1,   174,    -1,   176,    -1,
      -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
      -1,    -1,    -1,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   103,    -1,    -1,    -1,    -1,   108,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,
      -1,    -1,   200,   185,    -1,    -1,    -1,    -1,   190,   191,
     192,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
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
     196,    -1,   198,    11,    12,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    69,    -1,    56,    -1,    58,    59,    60,    61,    62,
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
      -1,    -1,   195,   196,    -1,   198,    -1,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,
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
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,
      -1,    -1,   179,    -1,    -1,   198,    -1,    -1,   185,    -1,
      -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,
     197,    -1,    -1,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    32,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
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
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
      -1,    -1,    -1,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
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
     179,    -1,    -1,   198,    -1,    -1,   185,    -1,    -1,    -1,
      -1,   190,   191,   192,    -1,    -1,   195,   196,    10,    11,
      12,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
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
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,     3,     4,
     176,     6,     7,   179,    -1,    10,    11,    12,    13,   185,
      -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,
     196,    -1,    -1,    28,    29,   201,   202,    -1,   204,   205,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    81,   128,    -1,   130,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,     6,     7,    -1,   161,    10,    11,    12,
      13,    -1,   167,   168,    -1,   170,   171,   172,   173,    -1,
     175,    -1,    -1,   178,    -1,    28,    29,    -1,    -1,    -1,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,
     195,    -1,    -1,    -1,   199,    -1,   201,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    57,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,   195,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    81,   128,    -1,   130,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,   161,    10,
      11,    12,    13,    -1,   167,   168,    -1,   170,   171,   172,
     173,    -1,   175,    -1,    -1,   178,    -1,    28,    29,    -1,
      31,    -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    -1,   199,    -1,   201,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    68,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,    -1,    -1,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     161,    91,    -1,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,   173,   103,   175,    -1,    -1,   178,     3,     4,
      -1,     6,     7,    -1,   185,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,   200,
      -1,    -1,    -1,    28,    29,    -1,    31,    -1,    -1,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   161,    57,    -1,   164,    -1,    -1,   167,   168,    -1,
     170,   171,   172,    68,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   195,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,    -1,    -1,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    83,
      -1,    85,    -1,    -1,    -1,    -1,   161,    -1,    -1,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,   173,   103,
     175,    -1,    -1,   178,     3,     4,    -1,     6,     7,    -1,
     185,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
     195,   196,    -1,    -1,    -1,   200,    -1,    -1,    -1,    28,
      29,    -1,    31,    -1,    -1,   139,   140,   141,   142,   143,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    57,    -1,
      -1,    -1,    -1,   167,   168,    -1,   170,   171,   172,    68,
      -1,    69,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   195,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
      -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,   173,   103,   175,    -1,    -1,   178,
       3,     4,    -1,     6,     7,    -1,   185,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,
      -1,   139,   140,   141,   142,   143,    -1,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    57,    57,   164,    -1,    -1,   167,
     168,    -1,   170,   171,   172,    68,    -1,    69,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,    -1,    -1,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,
      -1,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
     173,   103,   175,    -1,    -1,   178,    -1,     3,     4,    -1,
       6,     7,   185,   186,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    29,    -1,    31,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    57,   164,    -1,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    68,    -1,    69,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   195,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,    -1,    -1,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,   173,   103,   175,
      -1,    -1,   178,     3,     4,     5,     6,     7,    -1,   185,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   195,
     196,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    57,    57,    -1,
      -1,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      69,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     195,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   161,   162,   163,    -1,    81,    -1,   167,   168,    -1,
     170,   171,   172,   173,    -1,   175,   176,    -1,   178,    -1,
      -1,    -1,    -1,    -1,    -1,   185,   186,   103,   188,    -1,
     190,   191,    -1,     3,     4,   195,     6,     7,    -1,    -1,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   125,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,
      -1,    -1,   138,   139,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   161,    -1,    57,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
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
     128,    -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,   161,    10,    11,    12,    13,    -1,   167,
     168,    -1,   170,   171,   172,   173,    -1,   175,    -1,    -1,
     178,    -1,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,    -1,    -1,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,    -1,    -1,
      -1,   167,   168,    -1,   170,   171,   172,   173,    -1,   175,
      -1,    -1,   178,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,   198,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,   198,
      -1,    -1,    -1,    56,    -1,    58,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   197,    -1,    -1,    -1,
     103,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,    69,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,   156,    38,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,   197,   170,   171,   172,
      -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   185,    -1,    -1,    -1,    70,   190,    -1,    -1,
      -1,    -1,   195,   196,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,   136,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    69,    -1,    -1,    -1,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
      50,    51,   176,    -1,    -1,    -1,    56,    -1,    58,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   190,    -1,    -1,    -1,
      70,   195,   196,    -1,    -1,    -1,    -1,   201,    78,    79,
      80,    81,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,    69,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    70,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,   185,    83,    84,    -1,    -1,
     190,    -1,    -1,    -1,    91,   195,   196,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    70,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,   190,    -1,    -1,    -1,    91,   195,   196,
      -1,    -1,   199,    -1,   201,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    70,
      71,    -1,   176,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,   190,    -1,    -1,    -1,
      91,   195,   196,    -1,    -1,    -1,    -1,   201,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    70,    -1,    -1,   176,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,   190,
      -1,    -1,    -1,    91,   195,   196,    -1,    -1,    -1,    -1,
     201,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
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
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,   176,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   190,    -1,    -1,    -1,    -1,
     195,   196,    -1,    -1,    30,    31,   201,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     136,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   136,    33,
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
      -1,    31,   136,    -1,    34,    35,    36,    37,    38,    39,
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
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,   136,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      69,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,   136,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
     190,    91,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,
      -1,   201,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
     190,    91,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,
      -1,   201,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   103,   145,   146,   147,   148,   149,
      -1,    -1,   111,   112,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,    -1,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,
     190,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,
      -1,   201,   161,    -1,    -1,   164,    -1,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   195,    30,    31,    32,
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
      47,    48,    49,    50,    51,    52,    53,    54,    55,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    12,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    69,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    69,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69
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
     458,   459,   460,   461,   462,   464,   477,   479,   481,   119,
     120,   121,   137,   161,   171,   196,   213,   246,   327,   348,
     453,   348,   196,   348,   348,   348,   348,   108,   348,   348,
     439,   440,   348,   348,   348,   348,    81,    83,    91,   119,
     139,   140,   141,   142,   143,   156,   196,   224,   367,   408,
     411,   416,   453,   456,   453,   348,   348,   348,   348,   348,
     348,   348,   348,    38,   348,   468,   469,   119,   130,   196,
     224,   259,   408,   409,   410,   412,   416,   450,   451,   452,
     460,   465,   466,   348,   196,   337,   413,   196,   337,   358,
     338,   348,   232,   337,   196,   196,   196,   337,   198,   348,
     213,   198,   348,     3,     4,     6,     7,    10,    11,    12,
      13,    28,    29,    31,    57,    68,    71,    72,    73,    74,
      75,    76,    77,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   128,   130,   131,
     132,   133,   137,   138,   144,   161,   165,   173,   175,   178,
     185,   196,   213,   214,   215,   226,   482,   502,   503,   506,
     198,   343,   345,   348,   199,   239,   348,   111,   112,   161,
     164,   186,   216,   217,   218,   219,   223,    83,   201,   293,
     294,    83,   295,   121,   130,   120,   130,   196,   196,   196,
     196,   213,   265,   485,   196,   196,    70,    70,    70,    70,
      70,   338,    83,    90,   157,   158,   159,   474,   475,   164,
     199,   223,   223,   213,   266,   485,   165,   196,   485,   485,
      83,   192,   199,   359,    28,   336,   340,   348,   349,   453,
     457,   228,   199,    90,   414,   474,    90,   474,   474,    32,
     164,   181,   486,   196,     9,   198,    38,   245,   165,   264,
     485,   119,   191,   246,   328,   198,   198,   198,   198,   198,
     198,   198,   198,    10,    11,    12,    30,    31,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    57,    69,   198,    70,    70,   199,   160,   131,   171,
     173,   186,   188,   267,   326,   327,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    59,
      60,   134,   135,   443,    70,   199,   448,   196,   196,    70,
     199,   201,   461,   196,   245,   246,    14,   348,   198,   136,
      49,   213,   438,    90,   336,   349,   160,   453,   136,   203,
       9,   423,   260,   336,   349,   453,   486,   160,   196,   415,
     443,   448,   197,   348,    32,   230,     8,   360,     9,   198,
     230,   231,   338,   339,   348,   213,   279,   234,   198,   198,
     198,   138,   144,   506,   506,   181,   505,   196,   111,   506,
      14,   160,   138,   144,   161,   213,   215,   198,   198,   198,
     240,   115,   178,   198,   216,   218,   216,   218,   216,   218,
     223,   216,   218,   199,     9,   424,   198,   102,   164,   199,
     453,     9,   198,    14,     9,   198,   130,   130,   453,   478,
     338,   336,   349,   453,   456,   457,   197,   181,   257,   137,
     453,   467,   468,   348,   368,   369,   338,   389,   389,   368,
     389,   198,    70,   443,   157,   475,    82,   348,   453,    90,
     157,   475,   223,   212,   198,   199,   252,   262,   398,   400,
      91,   196,   201,   361,   362,   364,   407,   411,   459,   461,
     479,    14,   102,   480,   355,   356,   357,   289,   290,   441,
     442,   197,   197,   197,   197,   197,   200,   229,   230,   247,
     254,   261,   441,   348,   202,   204,   205,   213,   487,   488,
     506,    38,   174,   291,   292,   348,   482,   196,   485,   255,
     245,   348,   348,   348,   348,    32,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     412,   348,   348,   463,   463,   348,   470,   471,   130,   199,
     214,   215,   460,   461,   265,   213,   266,   485,   485,   264,
     246,    38,   340,   343,   345,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   165,   199,
     213,   444,   445,   446,   447,   460,   463,   348,   291,   291,
     463,   348,   467,   245,   197,   348,   196,   437,     9,   423,
     197,   197,    38,   348,    38,   348,   415,   197,   197,   197,
     460,   291,   199,   213,   444,   445,   460,   197,   228,   283,
     199,   345,   348,   348,    94,    32,   230,   277,   198,    27,
     102,    14,     9,   197,    32,   199,   280,   506,    31,    91,
     174,   226,   499,   500,   501,   196,     9,    50,    51,    56,
      58,    70,   138,   139,   140,   141,   142,   143,   185,   196,
     224,   375,   378,   381,   384,   387,   393,   408,   416,   417,
     419,   420,   213,   504,   228,   196,   238,   199,   198,   199,
     198,   199,   198,   102,   164,   199,   198,   111,   112,   164,
     219,   220,   221,   222,   223,   219,   213,   348,   294,   417,
      83,     9,   197,   197,   197,   197,   197,   197,   197,   198,
      50,    51,   495,   497,   498,   132,   270,   196,     9,   197,
     197,   136,   203,     9,   423,     9,   423,   203,   203,   203,
     203,    83,    85,   213,   476,   213,    70,   200,   200,   209,
     211,    32,   133,   269,   180,    54,   165,   180,   402,   349,
     136,     9,   423,   197,   160,   506,   506,    14,   360,   289,
     228,   193,     9,   424,   506,   507,   443,   448,   443,   200,
       9,   423,   182,   453,   348,   197,     9,   424,    14,   352,
     248,   132,   268,   196,   485,   348,    32,   203,   203,   136,
     200,     9,   423,   348,   486,   196,   258,   253,   263,    14,
     480,   256,   245,    71,   453,   348,   486,   203,   200,   197,
     197,   203,   200,   197,    50,    51,    70,    78,    79,    80,
      91,   138,   139,   140,   141,   142,   143,   156,   185,   213,
     376,   379,   382,   385,   388,   408,   419,   426,   428,   429,
     433,   436,   213,   453,   453,   136,   268,   443,   448,   197,
     348,   284,    75,    76,   285,   228,   337,   228,   339,   102,
      38,   137,   274,   453,   417,   213,    32,   230,   278,   198,
     281,   198,   281,     9,   423,    91,   226,   136,   160,     9,
     423,   197,   174,   487,   488,   489,   487,   417,   417,   417,
     417,   417,   422,   425,   196,    70,    70,    70,    70,    70,
     196,   417,   160,   199,    10,    11,    12,    31,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      69,   160,   486,   200,   408,   199,   242,   218,   218,   218,
     213,   218,   219,   219,   223,     9,   424,   200,   200,    14,
     453,   198,   182,     9,   423,   213,   271,   408,   199,   467,
     137,   453,    14,   348,   348,   203,   348,   200,   209,   506,
     271,   199,   401,    14,   197,   348,   361,   460,   198,   506,
     193,   200,    32,   493,   442,    38,    83,   174,   444,   445,
     447,   444,   445,   506,    38,   174,   348,   417,   289,   196,
     408,   269,   353,   249,   348,   348,   348,   200,   196,   291,
     270,    32,   269,   506,    14,   268,   485,   412,   200,   196,
      14,    78,    79,    80,   213,   427,   427,   429,   431,   432,
      52,   196,    70,    70,    70,    70,    70,    90,   157,   196,
     160,     9,   423,   197,   437,    38,   348,   269,   200,    75,
      76,   286,   337,   230,   200,   198,    95,   198,   274,   453,
     196,   136,   273,    14,   228,   281,   105,   106,   107,   281,
     200,   506,   182,   136,   160,   506,   213,   174,   499,     9,
     197,   423,   136,   203,     9,   423,   422,   370,   371,   417,
     390,   417,   418,   390,   370,   390,   361,   363,   365,   197,
     130,   214,   417,   472,   473,   417,   417,   417,    32,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   504,    83,   243,   200,   200,   200,   200,
     222,   198,   417,   498,   102,   103,   494,   496,     9,   299,
     197,   196,   340,   345,   348,   136,   203,   200,   480,   299,
     166,   179,   199,   397,   404,   166,   199,   403,   136,   198,
     493,   506,   360,   507,    83,   174,    14,    83,   486,   453,
     348,   197,   289,   199,   289,   196,   136,   196,   291,   197,
     199,   506,   199,   198,   506,   269,   250,   415,   291,   136,
     203,     9,   423,   428,   431,   372,   373,   429,   391,   429,
     430,   391,   372,   391,   157,   361,   434,   435,    81,   429,
     453,   199,   337,    32,    77,   230,   198,   339,   273,   467,
     274,   197,   417,   101,   105,   198,   348,    32,   198,   282,
     200,   182,   506,   213,   136,   174,    32,   197,   417,   417,
     197,   203,     9,   423,   136,   203,     9,   423,   203,   203,
     203,   136,     9,   423,   197,   136,   200,     9,   423,   417,
      32,   197,   228,   198,   198,   198,   198,   213,   506,   506,
     494,   408,     6,   112,   117,   120,   125,   167,   168,   170,
     200,   300,   325,   326,   327,   332,   333,   334,   335,   441,
     467,   348,   200,   199,   200,    54,   348,   348,   348,   360,
      38,    83,   174,    14,    83,   348,   196,   493,   197,   299,
     197,   289,   348,   291,   197,   299,   480,   299,   198,   199,
     196,   197,   429,   429,   197,   203,     9,   423,   136,   203,
       9,   423,   203,   203,   203,   136,   197,     9,   423,   299,
      32,   228,   198,   197,   197,   197,   235,   198,   198,   282,
     228,   136,   506,   506,   136,   417,   417,   417,   417,   361,
     417,   417,   417,   199,   200,   496,   132,   133,   186,   214,
     483,   506,   272,   408,   112,   335,    31,   125,   138,   144,
     165,   171,   309,   310,   311,   312,   408,   169,   317,   318,
     128,   196,   213,   319,   320,   301,   246,   506,     9,   198,
       9,   198,   198,   480,   326,   197,   296,   165,   399,   200,
     200,    83,   174,    14,    83,   348,   291,   117,   350,   493,
     200,   493,   197,   197,   200,   199,   200,   299,   289,   136,
     429,   429,   429,   429,   361,   200,   228,   233,   236,    32,
     230,   276,   228,   506,   197,   417,   136,   136,   136,   228,
     408,   408,   485,    14,   214,     9,   198,   199,   483,   480,
     312,   181,   199,     9,   198,     3,     4,     5,     6,     7,
      10,    11,    12,    13,    27,    28,    29,    57,    71,    72,
      73,    74,    75,    76,    77,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   137,   138,   145,   146,
     147,   148,   149,   161,   162,   163,   173,   175,   176,   178,
     185,   186,   188,   190,   191,   213,   405,   406,     9,   198,
     165,   169,   213,   320,   321,   322,   198,    83,   331,   245,
     302,   483,   483,    14,   246,   200,   297,   298,   483,    14,
      83,   348,   197,   196,   493,   198,   199,   323,   350,   493,
     296,   200,   197,   429,   136,   136,    32,   230,   275,   276,
     228,   417,   417,   417,   200,   198,   198,   417,   408,   305,
     506,   313,   314,   416,   310,    14,    32,    51,   315,   318,
       9,    36,   197,    31,    50,    53,    14,     9,   198,   215,
     484,   331,    14,   506,   245,   198,    14,   348,    38,    83,
     396,   199,   228,   493,   323,   200,   493,   429,   429,   228,
      99,   241,   200,   213,   226,   306,   307,   308,     9,   423,
       9,   423,   200,   417,   406,   406,    68,   316,   321,   321,
      31,    50,    53,   417,    83,   181,   196,   198,   417,   485,
     417,    83,     9,   424,   228,   200,   199,   323,    97,   198,
     115,   237,   160,   102,   506,   182,   416,   172,    14,   495,
     303,   196,    38,    83,   197,   200,   228,   198,   196,   178,
     244,   213,   326,   327,   182,   417,   182,   287,   288,   442,
     304,    83,   200,   408,   242,   175,   213,   198,   197,     9,
     424,   122,   123,   124,   329,   330,   287,    83,   272,   198,
     493,   442,   507,   197,   197,   198,   195,   490,   329,    38,
      83,   174,   493,   199,   491,   492,   506,   198,   199,   324,
     507,    83,   174,    14,    83,   490,   228,     9,   424,    14,
     494,   228,    38,    83,   174,    14,    83,   348,   324,   200,
     492,   506,   200,    83,   174,    14,    83,   348,    14,    83,
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
     454,   454,   455,   455,   455,   455,   455,   455,   455,   455,
     455,   456,   457,   457,   458,   458,   459,   459,   459,   460,
     460,   461,   461,   461,   462,   462,   462,   463,   463,   464,
     464,   465,   465,   465,   465,   465,   465,   466,   466,   466,
     466,   466,   467,   467,   467,   467,   467,   467,   468,   468,
     469,   469,   469,   469,   469,   469,   469,   469,   470,   470,
     471,   471,   471,   471,   472,   472,   473,   473,   473,   473,
     474,   474,   474,   474,   475,   475,   475,   475,   475,   475,
     476,   476,   476,   477,   477,   477,   477,   477,   477,   477,
     477,   477,   477,   477,   478,   478,   479,   479,   480,   480,
     481,   481,   481,   481,   482,   482,   483,   483,   484,   484,
     485,   485,   486,   486,   487,   487,   488,   489,   489,   489,
     489,   490,   490,   491,   491,   492,   492,   493,   493,   494,
     494,   495,   496,   496,   497,   497,   497,   497,   498,   498,
     498,   499,   499,   499,   499,   500,   500,   501,   501,   501,
     501,   502,   503,   504,   504,   505,   505,   506,   506,   506,
     506,   506,   506,   506,   506,   506,   506,   506,   507,   507
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
       4,     3,     1,     1,     1,     1,     1,     3,     3,     4,
       4,     3,     1,     1,     7,     9,     7,     6,     8,     1,
       2,     4,     4,     1,     1,     1,     4,     1,     0,     1,
       2,     1,     1,     1,     3,     3,     3,     0,     1,     1,
       3,     3,     2,     3,     6,     0,     1,     4,     2,     0,
       5,     3,     3,     1,     6,     4,     4,     2,     2,     0,
       5,     3,     3,     1,     2,     0,     5,     3,     3,     1,
       2,     2,     1,     2,     1,     4,     3,     3,     6,     3,
       1,     1,     1,     4,     4,     4,     4,     4,     4,     2,
       2,     4,     2,     2,     1,     3,     3,     3,     0,     2,
       5,     6,     6,     7,     1,     2,     1,     2,     1,     4,
       1,     4,     3,     0,     1,     3,     2,     3,     1,     1,
       0,     0,     3,     1,     3,     3,     2,     0,     2,     2,
       2,     2,     1,     2,     4,     2,     5,     3,     1,     1,
       0,     3,     4,     5,     6,     3,     1,     3,     2,     1,
       0,     4,     1,     3,     2,     4,     5,     2,     2,     1,
       1,     1,     1,     3,     2,     1,     8,     6,     1,     0
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
#line 6902 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 754 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 6910 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 761 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 6916 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 762 "hphp.y" /* yacc.c:1646  */
    { }
#line 6922 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 765 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 6928 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 766 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6934 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 767 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6940 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 768 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6946 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 769 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6952 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 770 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 6958 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 771 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 6966 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 6973 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 6979 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6985 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 6991 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 779 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6997 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7005 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 784 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7014 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 789 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7023 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 794 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7030 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 797 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7037 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 800 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7045 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 804 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7053 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 808 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7061 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 812 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7069 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 816 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7077 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 819 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7084 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 824 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7090 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 825 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7096 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 826 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7102 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 827 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7108 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7114 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 829 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7120 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 830 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7126 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7132 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7138 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7144 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7150 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7156 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7162 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 915 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7168 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 917 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7174 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 922 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7180 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 923 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7187 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 929 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7193 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 933 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7199 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 934 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7205 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 936 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7211 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 938 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7217 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 943 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7223 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 944 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7230 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 950 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7236 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 954 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7243 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 956 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7250 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 958 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7257 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 963 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7263 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 965 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7269 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 968 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7275 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 970 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7281 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 971 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7287 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 976 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7296 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 983 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7305 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 991 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7312 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 994 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7319 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 1000 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7325 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1001 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7331 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7337 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1005 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7343 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7349 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7355 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1010 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7361 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1014 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7367 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1019 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7373 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1020 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7380 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1022 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7388 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1026 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7395 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1029 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7403 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1033 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7410 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1035 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7418 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7425 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1040 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7433 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7439 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1044 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7445 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1045 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7451 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1046 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7457 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1047 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7463 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1048 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7469 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1049 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7475 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1050 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7481 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1051 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7487 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1052 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7493 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1053 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7499 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1054 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7505 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7511 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1056 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7517 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1057 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7524 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1061 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7531 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1063 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7539 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1068 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7546 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1070 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 7554 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1074 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 7562 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7568 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 7574 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1087 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7580 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1088 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 7586 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1089 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 7592 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1090 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 7601 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1094 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7607 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1095 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7613 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1096 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7619 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1097 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7625 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1098 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7631 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1099 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7637 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1100 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7643 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1101 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7649 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1102 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7655 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1103 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 7661 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1104 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7667 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1105 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 7677 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1113 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);}
#line 7683 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1114 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7689 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1123 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 7695 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1124 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7701 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1128 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7708 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1130 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7716 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1136 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7722 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1137 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7728 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1141 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 7734 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1142 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7740 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1146 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 7746 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7755 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1158 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7764 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1165 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7773 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1171 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7782 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1178 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7791 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1184 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7800 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1192 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7807 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1196 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 7813 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1200 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7820 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1204 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 7826 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 1210 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7833 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 7851 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 1228 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7858 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 7876 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1245 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7883 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1248 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7891 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1253 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7898 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1256 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7906 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1262 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 7912 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1265 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 7918 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1269 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7925 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1272 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7936 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1280 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7943 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1283 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7954 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1291 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7960 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1292 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 7967 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1296 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7973 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1299 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7979 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1302 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 7985 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1303 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 7991 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1304 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 7999 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1307 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 8005 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1308 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 8011 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1312 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8017 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1313 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8023 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1316 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8029 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1317 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8035 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1320 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8041 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1321 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8047 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1324 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8053 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1326 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8059 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1329 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8065 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1331 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8071 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1335 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8077 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1336 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8083 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1339 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8089 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1340 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8095 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1341 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8101 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1345 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8107 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1347 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8113 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1350 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8119 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1352 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8125 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1355 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8131 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8137 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1360 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8143 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1362 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8149 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1366 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8155 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1368 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8162 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1373 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8168 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1374 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8174 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1375 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8180 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1376 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8186 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1381 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8192 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1383 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8198 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1384 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8204 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1387 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8210 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1388 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8216 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1393 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8222 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1394 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8228 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1399 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8234 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1400 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8240 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1403 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8246 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1404 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8252 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1407 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8258 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1408 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8264 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1416 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8271 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1422 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8278 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1428 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8286 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1432 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8292 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1436 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8299 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1441 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8306 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1446 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8314 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1449 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8320 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1455 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8327 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1459 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8334 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1464 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8341 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1469 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8348 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1474 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8355 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1479 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8362 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1485 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8369 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1491 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8376 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1499 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8383 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1504 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8390 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1509 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8398 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1513 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8404 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1516 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8411 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1520 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 8418 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1524 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 8426 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1527 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8432 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1532 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8439 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1535 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8446 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1539 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8453 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1543 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8460 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1547 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8467 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1551 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8474 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1556 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8481 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1561 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8488 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1567 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8494 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1568 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8500 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1571 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,false);}
#line 8506 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1572 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),true,false);}
#line 8512 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1573 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,true);}
#line 8518 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1575 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),false, false);}
#line 8524 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1577 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),false,true);}
#line 8530 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1579 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),true, false);}
#line 8536 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1583 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8542 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1584 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 8548 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1587 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8554 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1588 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8560 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1589 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 8566 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1593 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 8572 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1595 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 8578 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1596 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 8584 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1597 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 8590 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1602 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8596 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1603 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8602 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1606 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8609 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1611 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 8615 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1617 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8621 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1618 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8627 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1621 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 8633 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1622 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 8640 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1625 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 8646 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1626 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 8653 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1628 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8660 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1631 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 8667 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1633 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8673 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1636 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8681 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1643 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8690 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1651 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8698 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1658 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8707 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1663 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 8713 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1665 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8719 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1667 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8725 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1669 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 8731 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1671 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 8737 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1672 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 8744 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1675 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 8750 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1678 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8756 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1679 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8762 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1680 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8768 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1686 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 8774 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1691 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 8781 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1694 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 8789 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1701 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 8795 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1702 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 8802 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1707 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 8809 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1710 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 8815 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1717 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 8822 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8828 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8834 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1728 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8840 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1730 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8846 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1732 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8852 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1733 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 8863 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1739 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 8869 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1741 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 8875 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1742 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 8881 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1746 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,0,(yyvsp[0]),0);}
#line 8887 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1748 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),0,(yyvsp[0]),0);}
#line 8893 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1753 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8899 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1756 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8905 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1757 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 8911 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1761 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 8917 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1762 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 8923 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1766 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 8930 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1769 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 8937 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1774 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 8944 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1779 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 8950 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1780 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 8957 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1782 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 8963 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1786 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 8969 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1787 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 8975 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1788 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 8981 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1789 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 8987 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1793 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8993 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1794 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 8999 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1795 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 9005 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1796 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 9011 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1797 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 9017 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1799 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 9023 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1801 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 9029 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1805 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9037 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1808 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9043 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1809 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9049 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1813 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9055 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1814 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9061 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1818 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9067 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1819 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9073 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1822 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9079 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1823 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9085 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1826 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9091 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1827 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9097 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1830 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9103 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1832 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9109 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9115 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9121 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9127 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9133 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9139 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1840 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9145 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9151 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1845 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9157 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9163 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1849 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9169 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1850 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9175 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1851 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9181 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1855 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9187 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1857 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9193 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1858 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9199 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1859 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9205 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1863 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9211 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1865 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9217 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1869 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9223 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1871 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9229 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1875 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9237 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1879 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9244 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1883 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9250 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1887 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9256 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1889 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9262 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1890 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9268 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1891 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9274 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1892 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9280 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1893 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9286 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1896 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9292 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1900 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9298 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1901 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9304 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1905 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9310 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1906 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9316 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 1910 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9322 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 1911 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9328 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 1912 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9334 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 1913 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9340 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 1917 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9346 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 1922 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9352 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 1926 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 9358 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 1930 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9364 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 1934 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 9370 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 1938 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9376 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 1943 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9382 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 1947 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9388 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 1948 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9394 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 1949 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9400 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9406 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 1951 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9412 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 9418 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 9424 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 1958 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 9430 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 1961 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 9436 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 9442 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 9448 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 9454 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 9460 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 1966 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 9466 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 1967 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 9472 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 1968 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 9478 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 1969 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 9484 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 1970 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 9490 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 1971 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 9496 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 1972 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 9502 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 9508 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 1974 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 9514 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 1975 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 9520 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 9526 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 1977 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 9532 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 1978 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 9538 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 1979 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 9544 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 1980 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 9550 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 1981 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 9556 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 9562 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 1983 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 9568 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 1984 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 9574 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 1985 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 9580 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 9586 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 1987 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 9592 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 1988 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 9598 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 1989 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 9604 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 1990 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 9610 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 9616 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 1992 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 9622 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 1993 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 9628 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 9634 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 1995 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 9640 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 1996 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 9646 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 1997 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 9652 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 1998 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 9658 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 9664 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 9670 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2001 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 9676 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2002 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 9682 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 9688 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 9694 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 9701 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 9707 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2008 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 9714 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2010 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 9720 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2012 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 9726 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9732 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 9738 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 9744 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2016 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 9750 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9756 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 9762 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2019 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 9768 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2020 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 9774 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2021 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 9780 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2022 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 9786 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2023 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 9792 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2024 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 9798 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2025 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 9804 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2026 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 9810 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9816 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2028 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9822 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2029 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9828 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2030 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9834 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2031 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9840 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2032 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9846 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2033 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9852 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2034 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9858 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2035 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 9864 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2036 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 9870 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2037 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9876 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2044 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 9882 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2045 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9888 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2050 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9897 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2056 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9908 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2064 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9917 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2070 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9928 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 9941 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 9955 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2098 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9965 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 9979 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2116 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9989 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10005 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10018 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10030 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10040 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10052 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2166 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10058 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2167 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10064 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2169 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10070 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2173 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10077 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2175 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10083 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2182 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10089 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2185 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10095 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2192 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10101 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2195 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10107 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2200 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10113 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2201 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10119 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2206 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10125 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2207 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10131 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2211 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_ARRAY);}
#line 10137 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2215 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10143 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2216 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10149 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2221 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10155 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2222 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10161 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2227 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10167 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2228 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10173 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2233 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10179 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2234 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10185 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2240 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10191 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2242 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10197 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2247 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10203 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2248 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10209 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2254 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10215 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2256 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10221 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2260 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10227 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2264 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10233 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2268 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10239 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2272 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10245 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2276 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10251 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2280 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10257 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2284 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10263 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2288 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10269 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2292 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10275 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2296 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10281 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2300 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10287 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2304 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10293 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2308 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10299 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2312 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10305 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2316 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10311 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2321 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10317 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2322 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10323 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2327 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10329 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2328 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10335 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2333 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10341 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2334 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10347 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2339 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10355 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2346 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10363 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2353 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10369 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2355 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10375 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2359 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10381 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2360 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10387 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2361 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10393 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2362 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10399 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2363 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10405 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2364 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10411 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2365 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10417 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2366 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10423 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2367 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10430 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2369 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10436 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2370 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10442 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2374 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10448 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2375 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 10454 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2376 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 10460 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2377 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 10466 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2384 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 10472 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10486 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10500 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2409 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 10506 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2410 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 10512 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2415 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10518 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2416 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10524 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2419 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),0,(yyvsp[0]),0);}
#line 10530 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2420 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10536 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2423 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10543 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2427 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10551 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2430 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10557 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10569 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2440 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10575 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2441 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10581 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2445 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10587 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2447 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 10593 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 10599 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10605 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10611 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2455 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10617 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2456 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10623 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2457 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10629 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2458 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10635 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10641 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2460 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10647 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10653 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2462 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10659 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2463 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10665 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10671 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2465 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10677 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2466 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10683 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2467 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10689 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2468 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10695 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2469 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10701 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2470 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10707 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2471 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10713 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2472 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10719 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2473 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10725 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2474 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10731 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2475 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10737 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2476 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10743 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2477 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10749 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2478 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10755 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2479 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10761 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2480 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10767 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2481 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10773 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2482 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10779 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10785 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2484 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10791 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10797 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10803 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10809 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10815 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10821 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10827 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2491 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10833 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2492 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10839 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2493 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10845 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2494 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10851 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2495 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10857 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2496 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10863 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2497 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10869 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2498 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10875 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2499 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10881 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2500 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10887 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2501 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10893 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10899 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2503 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10905 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2504 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10911 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2505 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10917 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2506 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10923 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2507 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10929 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2508 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10935 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2509 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10941 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2510 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10947 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2511 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10953 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2512 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10959 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2513 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10965 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2514 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10971 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2515 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10977 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2516 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10983 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10989 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2518 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10995 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2519 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11001 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2520 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11007 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2521 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11013 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2522 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11019 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2523 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11025 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2524 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11031 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2525 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11037 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2526 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11043 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2527 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11049 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2528 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11055 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2529 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11061 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2530 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11067 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2531 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11073 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2532 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11079 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2533 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11085 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2538 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11091 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2542 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11097 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2543 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11103 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2547 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11109 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2548 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11115 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2549 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11121 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2550 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11128 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2552 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11135 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2556 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11141 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2565 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11147 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2568 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11153 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2569 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11160 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2571 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11167 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2581 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11173 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2585 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11179 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2586 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11185 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2587 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11191 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2591 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11197 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2592 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11203 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2593 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11209 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2597 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11215 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2598 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11221 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2599 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11227 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2603 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11233 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2604 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11239 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2608 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11245 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2609 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11251 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2610 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11257 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2611 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11264 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2613 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11270 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2614 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11276 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2615 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 11282 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2616 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 11288 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2617 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 11294 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2618 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 11300 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2619 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 11306 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2620 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 11312 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2621 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 11318 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2624 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11324 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2626 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11330 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2630 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11336 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2631 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11342 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2633 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11348 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2634 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11354 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11360 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2637 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11366 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2638 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11372 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11378 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2640 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11384 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2641 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11390 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11396 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2643 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11402 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2644 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11408 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 11414 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2648 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 11420 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 11426 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2652 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 11432 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2654 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 11438 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2655 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 11444 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2656 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 11450 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2657 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 11456 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2658 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 11462 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 11468 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2660 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 11474 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2661 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 11480 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2662 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 11486 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 11492 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 11498 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2665 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 11504 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2666 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 11510 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2667 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 11516 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 11522 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2669 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11528 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11534 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2672 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 11540 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2674 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 11546 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2676 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 11552 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2678 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 11558 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2679 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 11564 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2681 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 11571 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2683 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 11577 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 11584 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2690 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 11590 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2693 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 11596 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2694 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 11602 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2698 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11608 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2699 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11614 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2705 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11620 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2711 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11626 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11632 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2716 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11638 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2717 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11644 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11650 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11656 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 11662 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 11668 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2723 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 11675 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2728 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11681 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11687 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2733 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11693 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11699 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2737 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 11705 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11711 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2744 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11717 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2746 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11723 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2748 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11729 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2749 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11735 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2753 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11741 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2754 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11747 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2755 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11753 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2758 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11759 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2760 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11765 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2763 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 11771 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2764 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 11777 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2765 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 11783 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2766 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11789 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2770 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 11796 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2773 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 11804 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2780 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11810 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2781 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11816 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2784 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 11824 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2787 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11830 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2788 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11836 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2789 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11842 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2791 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11848 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2792 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11854 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2794 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11860 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2795 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11866 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2796 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11872 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2797 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11878 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2798 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11884 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2799 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2804 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11896 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2805 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11902 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2810 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11908 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2811 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11914 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2816 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11920 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2818 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11926 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2820 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11932 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2821 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11938 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2825 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11944 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2826 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11950 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11956 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2832 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 11962 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2837 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11968 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2840 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11974 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2845 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11980 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2846 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11986 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11992 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2850 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 11999 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2857 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 12005 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2859 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 12011 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2862 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 12017 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2864 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12023 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12029 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2870 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12035 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2871 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12041 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2875 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12047 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2876 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12053 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2880 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12059 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2881 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12065 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2882 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12071 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2886 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12077 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2888 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12083 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2896 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12089 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2897 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12095 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2901 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12101 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2903 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12107 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 2911 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12113 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 2912 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12119 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 2916 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12125 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 2918 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12131 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 2923 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12137 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 2925 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12143 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12157 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12171 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12185 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12199 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 2981 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12205 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 2982 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12211 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 2983 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12217 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 2984 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12223 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 2985 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12229 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 2986 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12235 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12249 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3005 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12255 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3007 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12261 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3009 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12267 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3010 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12273 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3014 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12279 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3015 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12285 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3016 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12291 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12297 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12311 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3034 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12317 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3036 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12323 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3037 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12329 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3046 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12335 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3047 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12341 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3048 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12347 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3049 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12353 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3050 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12359 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3051 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12365 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3052 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12371 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3054 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12377 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3056 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12383 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3060 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12389 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3064 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12395 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3065 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12401 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3071 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12407 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3075 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12413 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3082 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 12419 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3091 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 12425 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3095 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 12431 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3099 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12437 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3102 "hphp.y" /* yacc.c:1646  */
    { _p->onIndirectRef((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 12443 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3108 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12449 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3109 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12455 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3110 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12461 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3114 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12467 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3115 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 12473 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3116 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 12479 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3123 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12485 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3124 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12491 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3129 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 12497 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3130 "hphp.y" /* yacc.c:1646  */
    { (yyval)++;}
#line 12503 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3135 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12509 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3136 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12515 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3137 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12521 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
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
#line 12535 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3151 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12541 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3152 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12547 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3156 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12553 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3157 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12559 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
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
#line 12573 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3169 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12579 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3173 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 12585 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3174 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 12591 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3176 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 12597 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3177 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 12603 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3178 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 12609 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3179 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 12615 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3184 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12621 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3185 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12627 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3189 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12633 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3190 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12639 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3191 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12645 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3192 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12651 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3195 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12657 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3197 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 12663 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3198 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12669 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3199 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 12675 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3204 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12681 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3205 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12687 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3209 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12693 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3210 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12699 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3211 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12705 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3212 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12711 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3217 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12717 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3218 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12723 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3223 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12729 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3225 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12735 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3227 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12741 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3228 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12747 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3232 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 12753 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3234 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 12759 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3235 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 12765 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3237 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 12772 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3242 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12778 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3244 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12784 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
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
#line 12798 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3256 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 12804 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3258 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 12810 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3259 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12816 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3262 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 12822 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3263 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 12828 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3264 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 12834 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3268 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 12840 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3269 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 12846 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3270 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12852 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3271 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12858 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3272 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12864 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3273 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12870 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3274 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 12876 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3275 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 12882 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3276 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 12888 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3277 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 12894 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3278 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 12900 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3282 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12906 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3283 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12912 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3288 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12918 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3290 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12924 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3304 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12932 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3309 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 12940 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3313 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12948 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3318 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 12956 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3324 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12962 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3325 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12968 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3329 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12974 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3330 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12980 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3336 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12986 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3340 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 12992 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3346 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12998 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3350 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 13005 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13011 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3358 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13017 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3362 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 13025 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3365 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 13032 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3371 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13038 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3376 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]); }
#line 13044 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3377 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13050 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3378 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13056 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1030:
#line 3379 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13062 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3400 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13068 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1038:
#line 3401 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13074 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3410 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13080 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3421 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13086 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3423 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13092 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3427 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13098 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3430 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13104 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3434 "hphp.y" /* yacc.c:1646  */
    {}
#line 13110 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3435 "hphp.y" /* yacc.c:1646  */
    {}
#line 13116 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3436 "hphp.y" /* yacc.c:1646  */
    {}
#line 13122 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3442 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13129 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3447 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13139 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3456 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13145 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3462 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13154 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3470 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13160 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3471 "hphp.y" /* yacc.c:1646  */
    { }
#line 13166 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3477 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13172 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3479 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13178 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3480 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13188 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3485 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13195 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3491 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("array"); }
#line 13202 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3496 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13208 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3501 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13216 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3505 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13222 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3510 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 13228 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3512 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 13234 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3518 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 13241 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3520 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 13249 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3523 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13255 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3524 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13263 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3527 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13271 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3530 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13277 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3533 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 13285 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3536 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13292 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3538 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 13301 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3544 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 13310 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1077:
#line 3550 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 13320 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1078:
#line 3558 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13326 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1079:
#line 3559 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13332 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;


#line 13336 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}
