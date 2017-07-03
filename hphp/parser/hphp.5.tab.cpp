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
#define YYLAST   18102

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  206
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  302
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1076
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1979

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
    2863,  2867,  2868,  2869,  2873,  2875,  2883,  2884,  2888,  2890,
    2898,  2899,  2903,  2904,  2909,  2911,  2916,  2927,  2941,  2953,
    2968,  2969,  2970,  2971,  2972,  2973,  2974,  2984,  2993,  2995,
    2997,  3001,  3002,  3003,  3004,  3005,  3021,  3022,  3024,  3033,
    3034,  3035,  3036,  3037,  3038,  3039,  3040,  3042,  3047,  3051,
    3052,  3056,  3059,  3066,  3070,  3079,  3086,  3088,  3094,  3096,
    3097,  3101,  3102,  3103,  3110,  3111,  3116,  3117,  3122,  3123,
    3124,  3125,  3136,  3139,  3142,  3143,  3144,  3145,  3156,  3160,
    3161,  3162,  3164,  3165,  3166,  3170,  3172,  3175,  3177,  3178,
    3179,  3180,  3183,  3185,  3186,  3190,  3192,  3195,  3197,  3198,
    3199,  3203,  3205,  3208,  3211,  3213,  3215,  3219,  3220,  3222,
    3223,  3229,  3230,  3232,  3242,  3244,  3246,  3249,  3250,  3251,
    3255,  3256,  3257,  3258,  3259,  3260,  3261,  3262,  3263,  3264,
    3265,  3269,  3270,  3274,  3276,  3284,  3286,  3290,  3294,  3299,
    3303,  3311,  3312,  3316,  3317,  3323,  3324,  3333,  3334,  3342,
    3345,  3349,  3352,  3357,  3362,  3364,  3365,  3366,  3369,  3371,
    3377,  3378,  3382,  3383,  3387,  3388,  3392,  3393,  3396,  3401,
    3402,  3406,  3409,  3411,  3415,  3421,  3422,  3423,  3427,  3431,
    3439,  3444,  3456,  3458,  3462,  3465,  3467,  3472,  3477,  3483,
    3486,  3491,  3496,  3498,  3505,  3507,  3510,  3511,  3514,  3517,
    3518,  3523,  3525,  3529,  3535,  3545,  3546
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

#define YYPACT_NINF -1626

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1626)))

#define YYTABLE_NINF -1060

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1060)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1626,   161, -1626, -1626,  5763, 13477, 13477,   -48, 13477, 13477,
   13477, 13477, 11244, 13477, -1626, 13477, 13477, 13477, 13477, 16697,
   16697, 13477, 13477, 13477, 13477, 13477, 13477, 13477, 13477, 11447,
   17362, 13477,   -31,    -1, -1626, -1626, -1626,   151, -1626,   217,
   -1626, -1626, -1626,   173, 13477, -1626,    -1,   232,   242,   285,
   -1626,    -1, 11650,  3559, 11853, -1626, 14507,  4928,   -21, 13477,
   17603,    43,   282,    62,   273, -1626, -1626, -1626,   349,   362,
     385,   403, -1626,  3559,   411,   419,   566,   576,   587,   612,
     633, -1626, -1626, -1626, -1626, -1626, 13477,   521,  1696, -1626,
   -1626,  3559, -1626, -1626, -1626, -1626,  3559, -1626,  3559, -1626,
     541,   520,  3559,  3559, -1626,   164, -1626, -1626, 12056, -1626,
   -1626,   512,   485,   594,   594, -1626,   692,   564,   641,   535,
   -1626,    89, -1626,   695, -1626, -1626, -1626, -1626,  2197,   509,
   -1626, -1626,   537,   540,   556,   586,   591,   595,   610,   614,
   15699, -1626, -1626, -1626, -1626,   170,   736,   740,   746,   750,
     755, -1626,   757,   759, -1626,    46,   634, -1626,   679,   208,
   -1626,  2909,   142, -1626, -1626,  3173,    98,   648,   178, -1626,
     133,   143,   650,   186, -1626,   327, -1626,   777, -1626,   689,
   -1626, -1626,   656,   690, -1626, 13477, -1626,   695,   509, 17852,
    3872, 17852, 13477, 17852, 17852, 15037, 15037,   655, 16134, 17852,
     808,  3559,   789,   789,   583,   789, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626, -1626, -1626,    60, 13477,   678, -1626, -1626,
     703,   669,    50,   670,    50,   789,   789,   789,   789,   789,
     789,   789,   789, 16697, 16257,   664,   862,   689, -1626, 13477,
     678, -1626,   712, -1626,   714,   683, -1626,   134, -1626, -1626,
   -1626,    50,    98, -1626, 12259, -1626, -1626, 13477,  9214,   873,
     101, 17852, 10229, -1626, 13477, 13477,  3559, -1626, -1626, 15747,
     684, -1626, 15795, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626,  2883, -1626,  2883, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626,    83,    93,   690, -1626, -1626, -1626, -1626,
     687,  4067,   100, -1626, -1626,   726,   874, -1626,   727, 15226,
   -1626,   693,   694, 15865, -1626,    73, 15913, 14049, 14049, 14402,
    3559,   696,   884,   699, -1626,   387, -1626, 16285,   106, -1626,
     880,   108,   769, -1626,   770, -1626, 16697, 13477, 13477,   705,
     722, -1626, -1626, 16388, 11447, 13477, 13477, 13477, 13477, 13477,
     110,   306,   464, -1626, 13680, 16697,   529, -1626,  3559, -1626,
     463,   564, -1626, -1626, -1626, -1626, 17462,   891,   805, -1626,
   -1626, -1626,    99, 13477,   711,   719, 17852,   720,  1892,   723,
    5966, 13477,    70,   707,   602,    70,   434,   497, -1626,  3559,
    2883,   715, 10432, 14507, -1626, -1626,  1344, -1626, -1626, -1626,
   -1626, -1626,   695, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, 13477, 13477, 13477, 13477, 12462, 13477, 13477, 13477,
   13477, 13477, 13477, 13477, 13477, 13477, 13477, 13477, 13477, 13477,
   13477, 13477, 13477, 13477, 13477, 13477, 13477, 13477, 13477, 13477,
   17562, 13477, -1626, 13477, 13477, 13477, 13851,  3559,  3559,  3559,
    3559,  3559,  2197,   802,   599,  5136, 13477, 13477, 13477, 13477,
   13477, 13477, 13477, 13477, 13477, 13477, 13477, 13477, -1626, -1626,
   -1626, -1626,  3929, 13477, 13477, -1626, 10432, 10432, 13477, 13477,
     512,   135, 16388,   725,   695, 12665, 15961, -1626, 13477, -1626,
     730,   919,   773,   734,   735, 14003,    50, 12868, -1626, 13071,
   -1626,   683,   737,   738,  2192, -1626,   329, 10432, -1626,  4381,
   -1626, -1626, 16031, -1626, -1626, 10635, -1626, 13477, -1626,   842,
    9417,   928,   743, 13664,   926,    85,    56, -1626, -1626, -1626,
     761, -1626, -1626, -1626,  2883, -1626,  3176,   753,   946, 16182,
    3559, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
     760, -1626, -1626,   763,   771,   768,   772,  3559,   778,   414,
     427, 17638, 14402, -1626, -1626,  3559,  3559, 13477,    50,    43,
   -1626, 16182,   875, -1626, -1626, -1626,    50,    97,   127,   762,
     784,  2922,   152,   785,   787,   490,   836,   790,    50,   128,
     786, 16866,   788,   978,   985,   792,   793,   796,   799, -1626,
    3185,  3559, -1626, -1626,   936,  3106,   483, -1626, -1626, -1626,
     564, -1626, -1626, -1626,   977,   877,   832,   323,   853, 13477,
     512,   878,  1007,   821, -1626,   859, -1626,   135, -1626,  2883,
    2883,  1006,   873,    99, -1626,   828,  1017, -1626,  2883,   202,
   -1626,   474,   144, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
     657,  3376, -1626, -1626, -1626, -1626,  1020,   848, -1626, 16697,
   13477,   834,  1023, 17852,  1021, -1626, -1626,   904,  2789, 11838,
   17989, 15037, 14684, 13477, 17804, 14861, 11426, 12846, 13251,  3720,
   12438, 14508, 14508, 14508, 14508,  3454,  3454,  3454,  3454,  3454,
    1100,  1100,   716,   716,   716,   583,   583,   583, -1626,   789,
   17852,   838,   841, 16914,   837,  1030,   218, 13477,   219,   678,
     211,   135, -1626, -1626, -1626,  1031,   805, -1626,   695, 16491,
   -1626, -1626, -1626, 15037, 15037, 15037, 15037, 15037, 15037, 15037,
   15037, 15037, 15037, 15037, 15037, 15037, -1626, 13477,   221,   140,
   -1626, -1626,   678,   352,   843,  3427,   850,   851,   854,  3811,
     129,   857, -1626, 17852,  3833, -1626,  3559, -1626,   202,   402,
   16697, 17852, 16697, 16970,   904,   202,    50,   145,   894,   866,
   13477, -1626,   165, -1626, -1626, -1626,  9011,   665, -1626, -1626,
   17852, 17852,    -1, -1626, -1626, -1626, 13477,   954,  4728, 16182,
    3559,  9620,   867,   868, -1626,  1058, 14226,   933, -1626,   913,
   -1626,  1066,   879,  4282,  2883, 16182, 16182, 16182, 16182, 16182,
     883,  1012,  1013,  1014,  1016,  1018,   896, 16182,    10, -1626,
   -1626, -1626, -1626, -1626, -1626,    17, -1626, 17900, -1626, -1626,
      12, -1626,  6169, 13897,   888, 14402, -1626, 14402, -1626,   437,
   -1626,  3559,  3559, 14402, 14402, -1626,  1081,   895, -1626, -1626,
   -1626,  4004, -1626, 17900,  1080, 16697,   899, -1626, -1626, -1626,
   -1626, -1626, -1626, -1626, -1626, -1626,   916,  1091,  3559, 13897,
     902, 16388, 16594,  1088, -1626, 13477, -1626, 13477, -1626, 13477,
   -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626,   900, -1626,
   13477, -1626, -1626,  5357, -1626,  2883, 13897,   905, -1626, -1626,
   -1626, -1626,  1093,   908, 13477, 17462, -1626, -1626, 13851,   911,
   -1626,  2883, -1626,   918,  6372,  1085,    65, -1626, -1626,    80,
    3929, -1626,  4381, -1626,  2883, -1626, -1626,    50, 17852, -1626,
   10838, -1626, 16182,    74,   924, 13897,   877, -1626, -1626, 14861,
   13477, -1626, -1626, 13477, -1626, 13477, -1626,  4257,   925, 10432,
     836,  1090,   877,  2883,  1098,   904,  3559, 17562,    50,  4702,
     927, -1626, -1626,   162,   930, -1626, -1626,  1110,  2292,  2292,
    3833, -1626, -1626, -1626,  1075,   932,  1059,  1060,  1062,  1063,
    1065,    66,   942,   389, -1626, -1626, -1626, -1626, -1626,   980,
   -1626, -1626, -1626, -1626,  1133,   949,   730,    50,    50, 13274,
     877,  4381, -1626, -1626, 11228,   673,    -1, 10229, -1626,  6575,
     951,  6778,   961,  4728, 16697,   964,  1011,    50, 17900,  1149,
   -1626, -1626, -1626, -1626,   339, -1626,    38,  2883,   983,  1032,
    1015,  2883,  3559,  3483, -1626, -1626, -1626,  1157, -1626,   970,
    1020,   742,   742,  1102,  1102, 17122,   973,  1171, 16182, 16182,
   16182, 16182, 16182, 16182, 17462,  2761, 15378, 16182, 16182, 16182,
   16182, 16059, 16182, 16182, 16182, 16182, 16182, 16182, 16182, 16182,
   16182, 16182, 16182, 16182, 16182, 16182, 16182, 16182, 16182, 16182,
   16182, 16182, 16182, 16182, 16182,  3559, -1626, -1626,  1101, -1626,
   -1626,   988,   991, -1626, -1626, -1626, 17638, -1626,   995, -1626,
   16182,    50, -1626, -1626,   139, -1626,   663,  1186, -1626, -1626,
     130,  1000,    50, 11041, 17852, 17018, -1626,  2808, -1626,  5560,
     805,  1186, -1626,   226,     3, -1626, 17852,  1061,  1002, -1626,
    1001,  1085, -1626,  2883,   873,  2883,    52,  1187,  1119,   171,
   -1626,   678,   176, -1626, -1626, 16697, 13477, 17852, 17900,  1010,
      74, -1626,  1004,    74,  1008, 14861, 17852, 17074,  1019, 10432,
    1022,  1009,  2883,  1024,  1027,  2883,   877, -1626,   683,   454,
   10432, 13477, -1626, -1626, -1626, -1626, -1626, -1626,  1073,  1035,
    1201,  1121,  3833,  3833,  3833,  3833,  3833,  3833,  1056, -1626,
   17462,    94,  3833, -1626, -1626, -1626, 16697, 17852,  1028, -1626,
      -1,  1182,  1140, 10229, -1626, -1626, -1626,  1041, 13477,  1011,
      50, 16388,  4728,  1025, 16182,  6981,   608,  1042, 13477,    68,
      53, -1626,  1038, -1626,  2883,  3559, -1626,  1094, -1626, -1626,
    4400,  1189,  1044, 16182, -1626, 16182, -1626,  1045,  1049,  1235,
   17177,  1052, 17900,  1238,  1054,  1055,  1057,  1123,  1253,  1067,
   -1626, -1626, -1626, 17225,  1068,  1254, 17945, 18033, 10412, 16182,
    4359, 12644, 13049, 13849,  4614, 16366, 16469, 16469, 16469, 16469,
    3482,  3482,  3482,  3482,  3482,  1181,  1181,   742,   742,   742,
    1102,  1102,  1102,  1102, -1626,  1070, -1626,  1071,  1074, -1626,
   -1626, 17900,  3559,  2883,  2883, -1626,   663, 13897,  1337, -1626,
   16388, -1626, -1626, 15037, 13477,  1076, -1626,  1072,  1543, -1626,
     104, 13477, -1626, -1626, -1626, 13477, -1626, 13477, -1626,   873,
   -1626, -1626,   116,  1251,  1192, 13477, -1626,  1082,    50, 17852,
    1085,  1083, -1626,  1084,    74, 13477, 10432,  1086, -1626, -1626,
     805, -1626, -1626,  1079,  1092,  1097, -1626,  1087,  3833, -1626,
    3833, -1626, -1626,  1099,  1095,  1273,  1151,  1104, -1626,  1280,
    1105,  1107,  1108, -1626,  1154,  1103,  1285, -1626, -1626,    50,
   -1626,  1263, -1626,  1106, -1626, -1626,  1116,  1117,   131, -1626,
   -1626, 17900,  1120,  1122, -1626, 13461, -1626, -1626, -1626, -1626,
   -1626, -1626,  1161,  2883, -1626,  2883, -1626, 17900, 17280, -1626,
   -1626, 16182, -1626, 16182, -1626, 16182, -1626, -1626, -1626, -1626,
   16182, 17462, -1626, -1626, 16182, -1626, 16182, -1626, 10818, 16182,
    1118,  7184, -1626, -1626,   663, -1626, -1626, -1626, -1626,   642,
   14683, 13897,  1191, -1626,  2586,  1130,  5207, -1626, -1626, -1626,
     802,  3553,   111,   112,  1124,   805,   599,   132, 17852, -1626,
   -1626, -1626,  1136, 11634, 12243, 17852, -1626,    91,  1291,  1232,
   13477, -1626, 17852, 10432,  1202,  1085,  1984,  1085,  1126, 17852,
    1127, -1626,  2069,  1129,  2193, -1626, -1626,    74, -1626, -1626,
    1190, -1626, -1626,  3833, -1626,  3833, -1626,  3833, -1626, -1626,
   -1626, -1626,  3833, -1626, 17462, -1626,  2282, -1626,  9011, -1626,
   -1626, -1626, -1626,  9823, -1626, -1626, -1626,  9011,  2883, -1626,
    1128, 16182, 17328, 17900, 17900, 17900,  1194, 17900, 17383, 10818,
   -1626, -1626,   663, 13897, 13897,  3559, -1626,  1317, 15530,    78,
   -1626, 14683,   805, 15120, -1626,  1156, -1626,   115,  1139,   118,
   -1626, 15036, -1626, -1626, -1626,   119, -1626, -1626,  4686, -1626,
    1134, -1626,  1252,   695, -1626, 14860, -1626, 14860, -1626, -1626,
    1326,   802, -1626, 14155, -1626, -1626, -1626, -1626,  1327,  1259,
   13477, -1626, 17852,  1147,  1152,  1085,   593, -1626,  1202,  1085,
   -1626, -1626, -1626, -1626,  2406,  1148,  3833,  1211, -1626, -1626,
   -1626,  1213, -1626,  9011, 10026,  9823, -1626, -1626, -1626,  9011,
   -1626, -1626, 17900, 16182, 16182, 16182,  7387,  1155,  1158, -1626,
   16182, -1626, 13897, -1626, -1626, -1626, -1626, -1626,  2883,   798,
    2586, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626,   150, -1626,  1130, -1626, -1626, -1626, -1626,
   -1626,    87,   126, -1626,  1340,   120, 15226,  1252,  1341, -1626,
    2883,   695, -1626, -1626,  1159,  1345, 13477, -1626, 17852, -1626,
     117,  1162, -1626, -1626, -1626,  1085,   593, 14331, -1626,  1085,
   -1626,  3833,  3833, -1626, -1626, -1626, -1626,  7590, 17900, 17900,
   17900, -1626, -1626, -1626, 17900, -1626,  2520,  1349,  1354,  1164,
   -1626, -1626, 16182, 15036, 15036,  1297, -1626,  4686,  4686,   356,
   -1626, -1626, -1626, 16182,  1283, -1626,  1188,  1172,   121, 16182,
   -1626,  3559, -1626, 16182, 17852,  1284, -1626,  1363, -1626,  7793,
    1174, -1626, -1626,   593, -1626, -1626,  7996,  1176,  1260, -1626,
    1274,  1218, -1626, -1626,  1277,  2883,  1198,   798, -1626, -1626,
   17900, -1626, -1626,  1210, -1626,  1347, -1626, -1626, -1626, -1626,
   17900,  1370,   490, -1626, -1626, 17900,  1193, 17900, -1626,   124,
    1195,  8199, -1626, -1626, -1626,  1196, -1626,  1197,  1220,  3559,
     599,  1209, -1626, -1626, -1626, 16182,  1214,    77, -1626,  1323,
   -1626, -1626, -1626,  8402, -1626, 13897,   888, -1626,  1233,  3559,
     552, -1626, 17900, -1626,  1215,  1398,   640,    77, -1626, -1626,
    1328, -1626, 13897,  1212, -1626,  1085,   123, -1626, -1626, -1626,
   -1626,  2883, -1626,  1216,  1217,   125, -1626,  1221,   640,   148,
    1085,  1227, -1626,  2883,   604,  2883,   311,  1401,  1336,  1221,
   -1626,  1418, -1626,   416, -1626, -1626, -1626,   156,  1414,  1346,
   13477, -1626,   604,  8605,  2883, -1626,  2883, -1626,  8808,   313,
    1416,  1348, 13477, -1626, 17852, -1626, -1626, -1626, -1626, -1626,
    1420,  1352, 13477, -1626, 17852, 13477, -1626, 17852, 17852
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,     0,   435,     0,   864,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   956,
     944,     0,   730,     0,   736,   737,   738,    26,   802,   931,
     932,   159,   160,   739,     0,   140,     0,     0,     0,     0,
      27,     0,     0,     0,     0,   194,     0,     0,     0,     0,
       0,     0,   404,   405,   406,   403,   402,   401,     0,     0,
       0,     0,   223,     0,     0,     0,    34,    35,    37,    38,
      36,   743,   745,   746,   740,   741,     0,     0,     0,   747,
     742,     0,   713,    29,    30,    31,    33,    32,     0,   744,
       0,     0,     0,     0,   748,   407,   542,    28,     0,   158,
     130,   936,   731,     0,     0,     4,   120,   122,   801,     0,
     712,     0,     6,   193,     7,     9,     8,    10,     0,     0,
     399,   448,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   446,   919,   920,   524,   518,   519,   520,   521,   522,
     523,   429,   527,     0,   428,   891,   714,   721,     0,   804,
     517,   398,   894,   895,   906,   447,     0,     0,   450,   449,
     892,   893,   890,   926,   930,     0,   507,   803,    11,   404,
     405,   406,     0,     0,    33,     0,   120,   193,     0,   996,
     447,   997,     0,   999,  1000,   526,   443,     0,   436,   441,
       0,     0,   489,   490,   491,   492,    26,   931,   739,   716,
      34,    35,    37,    38,    36,     0,     0,  1020,   912,   714,
       0,   715,   468,     0,   470,   508,   509,   510,   511,   512,
     513,   514,   516,     0,   960,     0,   811,   726,   213,     0,
    1020,   426,   725,   719,     0,   735,   715,   939,   940,   946,
     938,   727,     0,   427,     0,   729,   515,     0,     0,     0,
       0,   432,     0,   138,   434,     0,     0,   144,   146,     0,
       0,   148,     0,    72,    73,    78,    79,    64,    65,    56,
      76,    87,    88,     0,    59,     0,    63,    71,    69,    90,
      82,    81,    54,    77,    97,    98,    55,    93,    52,    94,
      53,    95,    51,    99,    86,    91,    96,    83,    84,    58,
      85,    89,    50,    80,    66,   100,    74,    67,    57,    44,
      45,    46,    47,    48,    49,    68,   102,   101,   104,    61,
      42,    43,    70,  1067,  1068,    62,  1072,    41,    60,    92,
       0,     0,   120,   103,  1011,  1066,     0,  1069,     0,     0,
     150,     0,     0,     0,   184,     0,     0,     0,     0,     0,
       0,     0,   813,     0,   108,   110,   312,     0,     0,   311,
     317,     0,     0,   224,     0,   227,     0,     0,     0,     0,
    1017,   209,   221,   952,   956,   561,   588,   588,   561,   588,
       0,   981,     0,   750,     0,     0,     0,   979,     0,    16,
       0,   124,   201,   215,   222,   618,   554,     0,  1005,   534,
     536,   538,   868,   435,   448,     0,     0,   446,   447,   449,
       0,     0,   732,     0,   733,     0,     0,     0,   183,     0,
       0,   126,   303,     0,    25,   192,     0,   220,   205,   219,
     404,   407,   193,   400,   173,   174,   175,   176,   177,   179,
     180,   182,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     944,     0,   172,   935,   935,   966,     0,     0,     0,     0,
       0,     0,     0,     0,   397,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   467,   469,
     869,   870,     0,   935,     0,   882,   303,   303,   935,     0,
     937,   927,   952,     0,   193,     0,     0,   152,     0,   866,
     861,   811,     0,   448,   446,     0,   964,     0,   559,   810,
     955,   735,   448,   446,   447,   126,     0,   303,   425,     0,
     884,   728,     0,   130,   263,     0,   541,     0,   155,     0,
       0,   433,     0,     0,     0,     0,     0,   147,   171,   149,
    1067,  1068,  1064,  1065,     0,  1071,  1057,     0,     0,     0,
       0,    75,    40,    62,    39,  1012,   178,   181,   151,   130,
       0,   168,   170,     0,     0,     0,     0,     0,     0,   110,
     111,     0,   812,   109,    18,     0,   105,     0,   313,     0,
     153,     0,     0,   154,   225,   226,  1001,     0,     0,   448,
     446,   447,   450,   449,     0,  1047,   233,     0,   953,     0,
       0,     0,     0,   811,   811,     0,     0,     0,     0,   156,
       0,     0,   749,   980,   802,     0,     0,   978,   807,   977,
     123,     5,    13,    14,     0,   231,     0,     0,   547,     0,
       0,     0,   811,     0,   723,     0,   722,   717,   548,     0,
       0,     0,     0,   868,   130,     0,   813,   867,  1076,   424,
     438,   503,   900,   918,   135,   129,   131,   132,   133,   134,
     398,     0,   525,   805,   806,   121,   811,     0,  1021,     0,
       0,     0,   813,   304,     0,   530,   195,   229,     0,   473,
     475,   474,   486,     0,     0,   506,   471,   472,   476,   478,
     477,   495,   496,   493,   494,   497,   498,   499,   500,   501,
     487,   488,   480,   481,   479,   482,   483,   485,   502,   484,
     934,     0,     0,   970,     0,   811,  1004,     0,  1003,  1020,
     897,   926,   211,   203,   217,     0,  1005,   207,   193,     0,
     439,   442,   444,   452,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   872,     0,   871,   874,
     896,   878,  1020,   875,     0,     0,     0,     0,     0,     0,
       0,     0,   998,   437,   859,   863,   810,   865,     0,   718,
       0,   959,     0,   958,   229,     0,   718,   943,   942,     0,
       0,   871,   874,   941,   875,   430,   265,   267,   130,   545,
     544,   431,     0,   130,   247,   139,   434,     0,     0,     0,
       0,     0,   259,   259,   145,   811,     0,     0,  1056,     0,
    1053,   811,     0,  1027,     0,     0,     0,     0,     0,   809,
       0,    34,    35,    37,    38,    36,     0,     0,   752,   756,
     757,   758,   759,   760,   762,     0,   751,   128,   800,   761,
    1020,  1070,     0,     0,     0,     0,    20,     0,    21,   111,
      19,     0,   106,     0,     0,   117,   813,     0,   115,   107,
     112,     0,   310,   318,   315,     0,     0,   990,   995,   992,
     991,   994,   993,    12,  1045,  1046,     0,   811,     0,     0,
       0,   952,   949,     0,   558,     0,   572,   810,   560,   810,
     587,   575,   581,   584,   578,   989,   988,   987,     0,   983,
       0,   984,   986,     0,     5,     0,     0,     0,   612,   613,
     621,   620,     0,   446,     0,   810,   553,   557,     0,     0,
    1006,     0,   535,     0,     0,  1034,   868,   289,  1075,     0,
       0,   883,     0,   933,   810,  1023,  1019,   305,   306,   711,
     812,   302,     0,   868,     0,     0,   231,   532,   197,   505,
       0,   595,   596,     0,   593,   810,   965,     0,     0,   303,
     233,     0,   231,     0,     0,   229,     0,   944,   453,     0,
       0,   880,   881,   898,   899,   928,   929,     0,     0,     0,
     847,   818,   819,   820,   827,     0,    34,    35,    37,    38,
      36,     0,     0,   833,   839,   840,   841,   842,   843,     0,
     831,   829,   830,   853,   811,     0,   861,   963,   962,     0,
     231,     0,   885,   734,     0,   269,     0,     0,   136,     0,
       0,     0,     0,     0,     0,     0,   239,   240,   251,     0,
     130,   249,   165,   259,     0,   259,     0,   810,     0,     0,
       0,     0,     0,   810,  1055,  1058,  1026,   811,  1025,     0,
     811,   783,   784,   781,   782,   817,     0,   811,   809,   565,
     590,   590,   565,   590,   556,     0,     0,   972,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1061,   185,     0,   188,
     169,     0,     0,   113,   118,   119,   812,   116,     0,   314,
       0,  1002,   157,  1018,  1047,  1038,  1042,   232,   234,   324,
       0,     0,   950,     0,   563,     0,   982,     0,    17,     0,
    1005,   230,   324,     0,     0,   718,   550,     0,   724,  1007,
       0,  1034,   539,     0,     0,  1076,     0,   294,   292,   874,
     886,  1020,   874,   887,  1022,     0,     0,   307,   127,     0,
     868,   228,     0,   868,     0,   504,   969,   968,     0,   303,
       0,     0,     0,     0,     0,     0,   231,   199,   735,   873,
     303,     0,   823,   824,   825,   826,   834,   835,   851,     0,
     811,     0,   847,   569,   592,   592,   569,   592,     0,   822,
     855,     0,   810,   858,   860,   862,     0,   957,     0,   873,
       0,     0,     0,     0,   266,   546,   141,     0,   434,   239,
     241,   952,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   253,     0,  1062,     0,     0,  1048,     0,  1054,  1052,
     810,     0,     0,     0,   754,   810,   808,     0,     0,   811,
       0,     0,   797,   811,     0,     0,     0,     0,   811,     0,
     763,   798,   799,   976,     0,   811,   766,   768,   767,     0,
       0,   764,   765,   769,   771,   770,   787,   788,   785,   786,
     789,   790,   791,   792,   793,   778,   779,   773,   774,   772,
     775,   776,   777,   780,  1060,     0,   130,     0,     0,   114,
      22,   316,     0,     0,     0,  1039,  1044,     0,   398,   954,
     952,   440,   445,   451,     0,     0,    15,     0,   398,   624,
       0,     0,   626,   619,   622,     0,   617,     0,  1009,     0,
    1035,   543,     0,   295,     0,     0,   290,     0,   309,   308,
    1034,     0,   324,     0,   868,     0,   303,     0,   924,   324,
    1005,   324,  1008,     0,     0,     0,   454,     0,     0,   837,
     810,   846,   828,     0,     0,   811,     0,     0,   845,   811,
       0,     0,     0,   821,     0,     0,   811,   832,   852,   961,
     324,     0,   130,     0,   262,   248,     0,     0,     0,   238,
     161,   252,     0,     0,   255,     0,   260,   261,   130,   254,
    1063,  1049,     0,     0,  1024,     0,  1074,   816,   815,   753,
     573,   810,   564,     0,   576,   810,   589,   582,   585,   579,
       0,   810,   555,   755,     0,   594,   810,   971,   795,     0,
       0,     0,    23,    24,  1041,  1036,  1037,  1040,   235,     0,
       0,     0,   405,   396,     0,     0,     0,   210,   323,   325,
       0,   395,     0,     0,     0,  1005,   398,     0,   562,   985,
     320,   216,   615,     0,     0,   549,   537,     0,   298,   288,
       0,   291,   297,   303,   529,  1034,   398,  1034,     0,   967,
       0,   923,   398,     0,   398,  1010,   324,   868,   921,   850,
     849,   836,   574,   810,   568,     0,   577,   810,   591,   583,
     586,   580,     0,   838,   810,   854,   398,   130,   268,   137,
     142,   163,   242,     0,   250,   256,   130,   258,     0,  1050,
       0,     0,     0,   567,   796,   552,     0,   975,   974,   794,
     130,   189,  1043,     0,     0,     0,  1013,     0,     0,     0,
     236,     0,  1005,     0,   361,   357,   363,   713,    33,     0,
     351,     0,   356,   360,   373,     0,   371,   376,     0,   375,
       0,   374,     0,   193,   327,     0,   329,     0,   330,   331,
       0,     0,   951,     0,   616,   614,   625,   623,   299,     0,
       0,   286,   296,     0,     0,  1034,     0,   206,   529,  1034,
     925,   212,   320,   218,   398,     0,     0,     0,   571,   844,
     857,     0,   214,   264,     0,     0,   130,   245,   162,   257,
    1051,  1073,   814,     0,     0,     0,     0,     0,     0,   423,
       0,  1014,     0,   341,   345,   420,   421,   355,     0,     0,
       0,   336,   674,   675,   673,   676,   677,   694,   696,   695,
     665,   637,   635,   636,   655,   670,   671,   631,   642,   643,
     645,   644,   664,   648,   646,   647,   649,   650,   651,   652,
     653,   654,   656,   657,   658,   659,   660,   661,   663,   662,
     632,   633,   634,   638,   639,   641,   679,   680,   684,   685,
     686,   687,   688,   689,   672,   691,   681,   682,   683,   666,
     667,   668,   669,   692,   693,   697,   699,   698,   700,   701,
     678,   703,   702,   705,   707,   706,   640,   710,   708,   709,
     704,   690,   630,   368,   627,     0,   337,   389,   390,   388,
     381,     0,   382,   338,   415,     0,     0,     0,     0,   419,
       0,   193,   202,   319,     0,     0,     0,   287,   301,   922,
       0,     0,   391,   130,   196,  1034,     0,     0,   208,  1034,
     848,     0,     0,   130,   243,   143,   164,     0,   566,   551,
     973,   187,   339,   340,   418,   237,     0,   811,   811,     0,
     364,   352,     0,     0,     0,   370,   372,     0,     0,   377,
     384,   385,   383,     0,     0,   326,  1015,     0,     0,     0,
     422,     0,   321,     0,   300,     0,   610,   813,   130,     0,
       0,   198,   204,     0,   570,   856,     0,     0,   166,   342,
     120,     0,   343,   344,     0,   810,     0,   810,   366,   362,
     367,   628,   629,     0,   353,   386,   387,   379,   380,   378,
     416,   413,  1047,   332,   328,   417,     0,   322,   611,   812,
       0,     0,   392,   130,   200,     0,   246,     0,   191,     0,
     398,     0,   358,   365,   369,     0,     0,   868,   334,     0,
     608,   528,   531,     0,   244,     0,     0,   167,   349,     0,
     397,   359,   414,  1016,     0,   813,   409,   868,   609,   533,
       0,   190,     0,     0,   348,  1034,   868,   273,   412,   411,
     410,  1076,   408,     0,     0,     0,   347,  1028,   409,     0,
    1034,     0,   346,     0,     0,  1076,     0,   278,   276,  1028,
     130,   813,  1030,     0,   393,   130,   333,     0,   279,     0,
       0,   274,     0,     0,   812,  1029,     0,  1033,     0,     0,
     282,   272,     0,   275,   281,   335,   186,  1031,  1032,   394,
     283,     0,     0,   270,   280,     0,   271,   285,   284
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1626, -1626, -1626,  -589, -1626, -1626, -1626,   177,    49,   -42,
     466, -1626,  -281,  -533, -1626, -1626,   300,   197,  1446, -1626,
    1801, -1626,  -522, -1626,     5, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626, -1626, -1626, -1626,  -459, -1626, -1626,  -172,
     107,    24, -1626, -1626, -1626, -1626, -1626, -1626,    31, -1626,
   -1626, -1626, -1626, -1626, -1626,    33, -1626, -1626,   947,   953,
     952,  -111,  -709,  -891,   455,   508,  -466,   199,  -974, -1626,
    -184, -1626, -1626, -1626, -1626,  -751,    27, -1626, -1626, -1626,
   -1626,  -452, -1626,  -616, -1626,  -443, -1626, -1626,   855, -1626,
    -155, -1626, -1626, -1079, -1626, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626, -1626,  -192, -1626,  -103, -1626, -1626, -1626,
   -1626, -1626,  -274, -1626,     1,  -973, -1626, -1625,  -479, -1626,
    -159,    57,  -104,  -453, -1626,  -279, -1626, -1626, -1626,     8,
     -19,    -3,    47,  -733,   -76, -1626, -1626,     9, -1626,   -12,
   -1626, -1626,    -5,   -43,  -137, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626, -1626,  -608,  -875, -1626, -1626, -1626, -1626,
   -1626,   334,  1109, -1626,   390, -1626,   262, -1626, -1626, -1626,
   -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
   -1626, -1626, -1626,   271,  -397,  -518, -1626, -1626, -1626, -1626,
   -1626,   326, -1626, -1626, -1626, -1626, -1626, -1626, -1626, -1626,
    -962,  -362,  2693,     6, -1626,   794,  -408, -1626, -1626,  -503,
    3587,  3511, -1626,  -663, -1626, -1626,   404,    -9,  -649, -1626,
   -1626,   481,   278,   515, -1626,   279, -1626, -1626, -1626, -1626,
   -1626,   467, -1626, -1626, -1626,    95,  -915,  -108,  -441,  -437,
   -1626,   546,  -106, -1626, -1626,    25,    26,   622, -1626, -1626,
    1096,   -18, -1626,  -360,   102,  -117, -1626,   -87, -1626, -1626,
   -1626,  -436,  1137, -1626, -1626, -1626, -1626, -1626,   632,   526,
   -1626, -1626, -1626,  -355,  -676, -1626,  1077, -1209, -1626,   -69,
    -177,     7,   677, -1626,  -439, -1626,  -446,  -860, -1294,  -349,
      58, -1626,   379,   451, -1626, -1626, -1626, -1626,   401, -1626,
    1750, -1135
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   115,   934,   651,   186,  1566,   749,
     361,   362,   363,   364,   886,   887,   888,   117,   118,   119,
     120,   121,   420,   685,   686,   560,   262,  1634,   566,  1543,
    1635,  1878,   874,   355,   589,  1838,  1130,  1326,  1897,   436,
     187,   687,   974,  1194,  1385,   125,   654,   991,   688,   707,
     995,   626,   990,   241,   541,   689,   655,   992,   438,   381,
     403,   128,   976,   937,   910,  1147,  1569,  1253,  1056,  1785,
    1638,   825,  1062,   565,   834,  1064,  1428,   817,  1045,  1048,
    1242,  1904,  1905,   675,   676,   701,   702,   368,   369,   371,
    1603,  1763,  1764,  1338,  1478,  1592,  1757,  1887,  1907,  1796,
    1842,  1843,  1844,  1579,  1580,  1581,  1582,  1798,  1799,  1805,
    1854,  1585,  1586,  1590,  1750,  1751,  1752,  1774,  1946,  1479,
    1480,   188,   130,  1921,  1922,  1755,  1482,  1483,  1484,  1485,
     131,   255,   561,   562,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,  1615,   142,   973,  1193,   143,   672,
     673,   674,   259,   412,   556,   661,   662,  1288,   663,  1289,
     144,   145,   632,   633,  1278,  1279,  1394,  1395,   146,   859,
    1024,   147,   860,  1025,   148,   861,  1026,   149,   862,  1027,
     150,   863,  1028,   635,  1281,  1397,   151,   864,   152,   153,
    1827,   154,   656,  1605,   657,  1163,   942,  1356,  1353,  1743,
    1744,   155,   156,   157,   244,   158,   245,   256,   423,   548,
     159,  1282,  1283,   868,   869,   160,  1086,   965,   603,  1087,
    1031,  1216,  1032,  1398,  1399,  1219,  1220,  1034,  1405,  1406,
    1035,   795,   531,   200,   201,   690,   678,   512,  1179,  1180,
     781,   782,   961,   162,   247,   163,   164,   190,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   741,   175,   251,
     252,   629,   235,   236,   744,   745,  1294,  1295,   396,   397,
     928,   176,   617,   177,   671,   178,   346,  1765,  1817,   382,
     431,   696,   697,  1079,  1934,  1941,  1942,  1174,  1335,   906,
    1336,   907,   908,   840,   841,   842,   347,   348,   871,   575,
    1568,   959
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     189,   191,   493,   193,   194,   195,   196,   198,   199,   122,
     202,   203,   204,   205,   344,   523,   225,   226,   227,   228,
     229,   230,   231,   232,   234,   443,   253,   957,   124,   404,
     258,   816,   415,   407,   408,   126,   243,   127,   804,   261,
    1362,  1175,  1467,   263,   664,   352,   666,   269,   267,   272,
     492,   668,   353,   971,   356,   248,   249,   953,   521,   439,
     515,   129,   933,   545,   952,   417,   351,   872,   885,   889,
    1167,   779,   738,   786,   787,   780,   594,   596,   598,  1249,
     994,   261,  1066,  1348,   443,  1192,   790,  1652,   832,   414,
     419,   260,   -75,  1052,   830,  1040,  1807,   -75,   433,   161,
    1426,  1203,   -40,   416,   809,   343,   895,   -40,   812,   -39,
     557,   123,   813,    14,   -39,   609,  -901,   612,  1176,   557,
    1595,  1597,    14,  1808,  -354,    14,   366,  1660,  1745,  1814,
    1814,  1030,   250,   390,  1652,  1363,   557,   912,   912,   912,
     912,   912,   417,   549,  1258,  1259,   550,    14,   192,  1238,
     532,  1831,   954,   391,  1497,  1825,  1228,  1810,  1492,  1258,
    1259,     3,  1889,  1177,  1802,   254,   414,   419,   513,  1354,
   -1020,    14,  1125,   534,  1608,  1407,  1811,  1096,   354,  1812,
     416,   116,  1803,   372,   510,   511,  1936,   526,   590,   904,
     905,   430,   373,   430,  1959,   257,   543,   533,   419,  1498,
    1826,  1804,  1355,  -902,  -945,   518,  -723,  1890,  1874, -1020,
    -905,   416,  -904,  -903,  -908,  -948,  1097,   393,   494,  1287,
     542,  -607,  -604,  1229,  1291,  -540,  1364,   540,   394,   395,
     270,  1937,  -907,   342,   416,  -947,   442,  1137,  1261,  1960,
    -597,  -888,  -911,  -605,   367,  -901,  -889,   409,  -604,   552,
     380,   591,   552,  1429,  1178,   833,   518,   365,  -812,   261,
     563,  1567,  -812,   554,   574,  1609,  1427,   559,  1467,   405,
     708,  -293,   513,   402,  -277,   380,  1653,  1654,  1419,   380,
     380,   -75,   831,   641,  1809,   400,  1206,   434,   401,   667,
    1499,   -40,  -293,  1506,   896,   524,  1049,   514,   -39,   558,
    1512,  1051,  1514,  -722,   610,   380,   613,   585,   639,  1596,
    1598,  1359,  1256,  -354,  1260,  1384,  1661,  1746,  1815,  1864,
    -812,  -810,  1938,  1932,   897,   913,  1007,  1339,  1542,  1602,
    1961,  1536,  -902,  -945,   519,   620,   510,   511,  -910,  -905,
    -914,  -904,  -903,  -908,  -948,  1159,  -717,  1030,   517,   901,
    1134,  1135,   791,   218,   218,  1404,   410,  1189,  -917,   619,
     623,  -907,  1655,   411,  -947,   370,  -912,   706,  -715,   751,
    -888,  -724,   261,   416,   517,  -889,   640,   939,   530,   234,
     631,   261,   261,   631,   261,   519,  1758,  1857,  1759,   645,
     443,   344,  1349,   374,  1948,   751,  1970,   742,   343,  -104,
    -103,   514,  -876,   375,  -913,  1350,  1858,   989,   198,  1859,
     207,    40,   207,    40,  -104,  -103,   691,  -876,   751,   760,
     404,   755,   756,   439,   618,  1351,   784,   703,   264,   751,
    1956,   788,   751,   634,   634,   116,   634,  1624,   265,   116,
     510,   511,  -716,   564,  1257,  1258,  1259,   709,   710,   711,
     712,   714,   715,   716,   717,   718,   719,   720,   721,   722,
     723,   724,   725,   726,   727,   728,   729,   730,   731,   732,
     733,   734,   735,   736,   737,  1150,   739,   129,   740,   740,
     743,   266,   343,   762,  1347,  1949,   243,  1971,   940,   605,
     763,   764,   765,   766,   767,   768,   769,   770,   771,   772,
     773,   774,   775,   941,   761,   248,   249,   677,   740,   785,
    1504,   703,   703,   740,   789,  1416,   605,   391,  1333,  1334,
     763,  1182,   797,   793,   647,  1183,   584,   123,   520,   881,
     111,   493,   801,  -879,   803,   748,   510,   511,  1255,   881,
     904,   905,   703,   819,  -606,   376,  1200,   391,  -879,  -120,
     820,   606,   821,  -120,   365,   365,   599,   600,   377,  1030,
    1030,  1030,  1030,  1030,  1030,   824,  1361,   218,   391,  1030,
    -120,   960,   988,   962,  1371,   422,  1556,  1373,   429,   492,
     391,   378,   250,   664,   343,   666,   996,   647,   750,  1208,
     668,   882,   394,   395,  1131,   650,  1132,   116,  -915,   379,
     758,   429,   891,   885,   391,  1000,   695,   383,   510,   511,
     342,   392,   391,   380,   783,   384,   943,   510,   511,   647,
      55,   642,   394,   395,   918,   920,   165,   429,   440,   180,
     181,    65,    66,    67,  -718,  -877,   385,   750,   693,   978,
     480,   222,   224,   394,   395,  1616,   386,  1618,   808,  1957,
    -877,   814,   481,   946,   416,   394,   395,   387,   636,  1631,
     638,   652,   653,   584,   380,   753,   380,   380,   380,   380,
    -915,   440,   180,   181,    65,    66,    67,   391,   393,   394,
     395,   545,   388,   932,   425,   391,   648,   394,   395,   778,
     960,   962,   647,  1126,  1284,   968,  1286,  1041,   962,  1042,
     441,   218,   694,   389,  1513,  1386,   405,  1400,   979,  1402,
     218,   421,   584,  1423,  1258,  1259,   406,   218,   440,   180,
     181,    65,    66,    67,   428,  1030,   811,  1030,   429,   218,
     418,   432,   664,   435,   666,   444,   986,   116,   445,   668,
    1046,  1047,   987,   441,   424,   426,   427,   494,  1240,  1241,
    1914,  1496,   394,   395,   446,  1771,  1377,   870,  1508,  1776,
     394,   395,  1918,  1919,  1920,  1333,  1334,  1387,   677,   477,
     478,   479,   999,   480,  1563,  1564,   440,    63,    64,    65,
      66,    67,   890,   695,   447,   481,  1929,    72,   487,   448,
     441,  1772,  1773,   449,   879,  1121,  1122,  1123,   599,   599,
    1947, -1020,  1944,  1945,  1461,  1044,  -598,   418,   450,  1600,
    -599,  1124,   451,   220,   220,  1418,  -600,   927,   929,  1050,
    -601,   261,   430,   593,   595,  -602,  1068,   483,   667,   484,
     489,   751,  1074,   485,  1855,  1856,  1061, -1020,   418,   486,
   -1020,  1851,  1852,   751,   516,   751,  -909,  -603,   441,  -716,
    1077,  1080,   522,   527,   398,   536,   218,   529,   481,   430,
    1030,   544,  1030,   535,  1030,  -913,   517,   538,   664,  1030,
     666,   539,  -714,   129,   546,   668,    34,    35,    36,   547,
     165,   555,   568,   576,   165,   380, -1059,   580,   579,   208,
    1538,   586,   587,   602,   611,   601,  1656,   604,  1145,   614,
     615,  1625,   624,   625,  1487,   669,  1547,   670,   679,   692,
    1154,  -125,  1155,    55,   821,  1830,   680,   681,   643,  1833,
     683,   705,   649,   123,   751,  1157,   794,  1207,   796,   129,
     642,   798,   799,  1510,   805,   806,   822,   557,   122,  1166,
     829,   826,   574,    81,    82,    83,    84,    85,   643,   843,
     649,   643,   649,   649,   215,   844,   873,   124,   894,   898,
      89,    90,   875,  1030,   126,  1187,   127,   877,   909,   876,
     878,  1023,  1906,  1036,    99,  1195,   880,   667,  1196,   123,
    1197,   899,   902,   914,   703,   903,   911,   917,   104,   608,
     129,   916,  1906,   116,   919,   921,   922,   748,   616,   923,
     621,  1928,   924,   243,  1367,   628,   930,  1059,   116,   935,
     936,   129,   938,  -739,   944,  1633,   945,   646,   947,   948,
     951,   955,   248,   249,  1639,  1233,   956,   220,   161,   964,
     966,   969,   970,   218,  1237,   972,   975,   984,  1646,   985,
     123,   981,   165,  1243,   982,   993,  1001,  1003,  1004,   116,
    1168,   677,  1244,   977,  -720,  1927,  1053,  1005,  1133,   695,
    1613,   123,   783,  1043,   814,  1063,  1065,  1067,   677,  1071,
    1939,  1272,   599,  1072,   599,  1073,  1075,  1341,  1276,  1088,
     599,   599,  1089,  1090,  1091,  1146,  1092,  1129,  1093,   664,
    1136,   666,  1094,   218,  1140,  1138,   668,  1142,  1143,   250,
    1144,  1149,  1153,  1156,  1162,  1165,   129,  1164,   129,  1169,
     116,  1171,  1205,   667,  1787,   223,   223,  1173,  1030,  1030,
    1190,  1199,  1202,  1210,  1211,   584,  -916,  1221,  1222,  1223,
    1224,   116,  1225,  1226,   218,  1227,   218,   778,  1230,   811,
    1231,  1342,  1232,   814,   628,  1292,  1234,  1252,  1343,  1246,
     474,   475,   476,   477,   478,   479,   123,   480,   123,  1248,
    1251,   220,   218,  1254,   122,  1263,  1270,  1271,  1264,   481,
     220,  1124,   664,   380,   666,  1265,  1274,   220,  1870,   668,
    1275,  1369,   165,   124,  1325,  1215,  1215,  1023,  1327,   220,
     126,  1328,   127,  1330,   703,  1337,  1340,  1357,   989,  1358,
     665,  1365,  1366,  1372,  1374,   703,  1343,  1370,  1379,  1388,
    1390,  1391,  1014,  1403,  1412,  1376,   129,  1413,   811,  1378,
    1430,  1435,  1420,  1381,   116,  1382,   116,  1410,   116,   218,
    1433,  1118,  1119,  1120,  1121,  1122,  1123,  1411,  1389,  1415,
    1424,  1436,  1439,   261,  1441,   218,   218,  1445,  1414,  1267,
    1124,  1829,  1440,  1425,   161,  1444,  1917,  1447,  1448,  1450,
    1449,  1836,  1451,  1456,  1453,  1500,   123,  1460,  1455,  1462,
    1442,  1490,  1463,   584,  1446,  1501,  1489,  1515,  1503,  1452,
    1505,  1507,  1523,  1511,  1518,   677,  1457,  1525,   677,  1527,
    1532,  1516,  1955,  1517,  1534,  1537,  1521,  1548,  1522,  1584,
    1533,  1604,   870,  1571,  1539,  1610,  1871,  1526,  1529,  1033,
    1530,  1531,   129,  1540,  1541,  1611,   220,  1560,  1544,  1614,
    1545,   967,  1599,  1619,  1620,  1641,  1626,  1601,  1622,   223,
    1644,  1650,  1753,   599,   667,  1754,   116,  1658,  1659,  1488,
    1760,  1766,  1767,  1469,  1769,  1779,  1493,  1781,  1770,  1782,
    1494,  1893,  1495,  1792,  1813,  1819,  1793,  1822,  1845,  1823,
    1502,  1828,   123,  1847,  1849,  1853,  1861,  1868,  1863,  1862,
    1509,   703,  1869,  1873,  1876,  1877,  -350,   443,  1879,  1880,
    1882,   998,  1884,  1808,  1885,    14,  1524,   218,   218,  1888,
    1528,  1901,  1891,  1895,  1894,  1481,  1903,  1535,  1896,  1023,
    1023,  1023,  1023,  1023,  1023,  1481,  1908,  1916,  1912,  1023,
    1926,  1924,  1915,  1930,  1931,  1950,  1933,   667,  1953,  1951,
     116,  1756,  1037,  1958,  1038,   206,  1940,  1954,  1962,  1963,
    1972,  1973,   116,  1486,  1975,  1976,  1329,  1911,   165,   757,
     752,   754,  1432,  1486,  1161,  1201,  1925,    50,  1417,  1470,
    1057,  1786,  1546,   165,  1471,  1923,   440,  1472,   181,    65,
      66,    67,  1473,   223,   892,   217,   217,  1777,  1801,   677,
    1657,  1806,   223,  1965,   622,  1935,   240,  1591,  1818,   223,
    1572,  1775,  1285,   210,   211,   212,   213,   214,  1401,  1352,
    1217,   223,  1277,   220,   165,  1612,  1649,   637,   703,  1392,
    1952,  1393,   240,  1235,  1474,  1475,  1181,  1476,  1967,  1464,
     704,    93,    94,  1886,    95,   184,    97,  1141,   129,   218,
    1078,   630,  1562,  1332,  1269,  1218,  1324,     0,   441,     0,
       0,     0,     0,   628,  1152,     0,     0,  1477,     0,   107,
     705,     0,     0,   494,     0,     0,     0,     0,  1637,  1469,
       0,     0,     0,   220,     0,   165,     0,     0,     0,     0,
       0,     0,     0,  1481,     0,  1023,     0,  1023,   123,  1481,
     218,  1481,     0,     0,     0,     0,   165,     0,     0,     0,
       0,     0,     0,     0,     0,   218,   218,  1593,     0,  1821,
       0,    14,     0,  1481,   220,   129,   220,     0,     0,     0,
       0,  1486,     0,     0,   129,  1768,     0,  1486,     0,  1486,
       0,     0,   677,     0,     0,     0,     0,  1651,   223,     0,
       0,     0,   220,     0,     0,     0,     0,     0,     0,     0,
       0,  1486,     0,     0,     0,     0,     0,     0,   116,  1784,
    1637,     0,     0,     0,     0,   123,     0,   342,     0,     0,
       0,     0,     0,  1589,   123,  1470,     0,     0,     0,     0,
    1471,     0,   440,  1472,   181,    65,    66,    67,  1473,   165,
       0,   165,     0,   165,   218,  1057,  1250,     0,     0,   217,
       0,  1481,     0,     0,     0,     0,     0,     0,     0,   220,
     129,     0,     0,     0,     0,     0,   129,     0,     0,     0,
    1023,     0,  1023,   129,  1023,   220,   220,     0,  1761,  1023,
    1474,  1475,     0,  1476,  1816,   116,     0,     0,     0,  1486,
     116,  1899,     0,     0,   116,     0,     0,     0,     0,   240,
       0,   240,     0,     0,   441,     0,     0,  1218,  1396,   665,
     123,  1396,   380,  1491,     0,   584,   123,  1408,   342,     0,
       0,     0,  1866,   123,     0,     0,     0,     0,  1742,     0,
       0,  1824,     0,     0,     0,  1749,     0,     0,     0,     0,
       0,     0,   342,     0,   342,     0,     0,   206,     0,     0,
     342,   165,     0,     0,     0,     0,     0,   240,  1846,  1848,
       0,     0,     0,     0,     0,   223,   443,     0,     0,    50,
       0,     0,     0,  1023,     0,   343,   349,  1368,     0,     0,
     116,   116,   116,   217,     0,     0,   116,     0,     0,     0,
       0,     0,   217,   116,     0,     0,     0,     0,     0,   217,
       0,     0,     0,     0,     0,   210,   211,   212,   213,   214,
       0,   217,     0,     0,   129,     0,     0,   220,   220,     0,
       0,     0,   217,     0,     0,   223,     0,   345,  1409,     0,
     398,     0,     0,    93,    94,   165,    95,   184,    97,     0,
       0,     0,     0,   628,  1057,     0,   240,   165,     0,   240,
       0,     0,     0,     0,     0,     0,   129,     0,   665,     0,
       0,   107,     0,   129,   123,   399,   223,     0,   223,     0,
       0,     0,     0,  1519,     0,  1520,   525,   496,   497,   498,
     499,   500,   501,   502,   503,   504,   505,   506,   507,     0,
       0,     0,     0,     0,   223,     0,   240,     0,   129,     0,
       0,     0,     0,   584,     0,     0,   123,  1900,     0,     0,
       0,     0,     0,   123,     0,  1964,     0,     0,     0,     0,
     129,   508,   509,     0,   342,     0,     0,  1974,  1023,  1023,
       0,     0,   628,     0,   116,     0,     0,  1977,   217,     0,
    1978,     0,     0,  1840,     0,     0,     0,     0,   123,   220,
    1742,  1742,   677,     0,  1749,  1749,     0,     0,     0,     0,
    1469,   223,     0,     0,     0,     0,     0,     0,   380,     0,
     123,     0,   677,     0,     0,     0,   116,   223,   223,     0,
     129,   677,     0,   116,     0,   129,     0,     0,     0,     0,
     240,     0,   240,     0,   665,   858,   510,   511,     0,     0,
     220,     0,    14,   572,     0,   573,     0,     0,  1627,     0,
    1628,     0,  1629,     0,     0,   220,   220,  1630,   116,     0,
       0,     0,     0,     0,     0,     0,  1898,   858,     0,     0,
     123,     0,     0,     0,     0,   123,     0,     0,     0,     0,
     116,     0,     0,     0,     0,  1469,  1913,     0,     0,     0,
       0,     0,     0,   165,   345,     0,   345,     0,     0,   682,
       0,   578,     0,     0,     0,     0,  1470,     0,     0,     0,
       0,  1471,     0,   440,  1472,   181,    65,    66,    67,  1473,
       0,     0,     0,     0,     0,   240,   240,    14,     0,     0,
       0,     0,     0,     0,   240,     0,     0,     0,     0,     0,
     116,     0,     0,     0,   220,   116,     0,     0,     0,     0,
       0,  1780,   345,     0,     0,   217,     0,     0,     0,   223,
     223,  1474,  1475,     0,  1476,     0,     0,     0,     0,     0,
     165,     0,     0,     0,     0,   165,     0,     0,     0,   165,
       0,     0,     0,     0,     0,   441,     0,     0,     0,     0,
     698,  1470,     0,   349,  1617,     0,  1471,     0,   440,  1472,
     181,    65,    66,    67,  1473,     0,     0,     0,     0,  1469,
       0,     0,     0,     0,     0,   217,   525,   496,   497,   498,
     499,   500,   501,   502,   503,   504,   505,   506,   507,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   345,     0,     0,   345,     0,  1474,  1475,     0,  1476,
     240,    14,     0,     0,     0,   665,   217,     0,   217,     0,
       0,   508,   509,     0,     0,   165,   165,   165,     0,     0,
     441,   165,     0,     0,     0,     0,     0,     0,   165,  1621,
       0,     0,     0,     0,   217,   858,     0,     0,   206,     0,
       0,   223,   240,     0,     0,     0,     0,     0,  1469,   240,
     240,   858,   858,   858,   858,   858,  1834,  1835,     0,     0,
      50,     0,     0,   858,     0,  1470,     0,     0,     0,     0,
    1471,     0,   440,  1472,   181,    65,    66,    67,  1473,   240,
       0,     0,     0,     0,   835,     0,   510,   511,   665,     0,
      14,     0,   223,     0,     0,     0,   210,   211,   212,   213,
     214,   217,     0,     0,     0,     0,     0,   223,   223,     0,
       0,     0,     0,     0,     0,   240,     0,   217,   217,     0,
    1474,  1475,   437,  1476,    93,    94,     0,    95,   184,    97,
    1212,  1213,  1214,   206,     0,   345,     0,   839,     0,     0,
       0,   240,   240,     0,   441,     0,     0,     0,     0,   807,
       0,   217,   107,  1623,  1470,    50,     0,   240,     0,  1471,
       0,   440,  1472,   181,    65,    66,    67,  1473,     0,   165,
     240,     0,  1469,     0,     0,     0,     0,     0,   858,   949,
     950,   240,     0,     0,     0,     0,     0,     0,   958,     0,
       0,   210,   211,   212,   213,   214,   223,     0,     0,   240,
       0,     0,     0,   240,     0,     0,     0,     0,     0,  1474,
    1475,   165,  1476,     0,    14,     0,   240,     0,   165,    93,
      94,     0,    95,   184,    97,     0,     0,     0,     0,     0,
     345,   345,     0,   441,     0,     0,     0,     0,     0,   345,
       0,     0,  1632,     0,     0,     0,     0,   107,     0,     0,
       0,     0,     0,   165,     0,     0,     0,     0,     0,   217,
     217,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   240,     0,   165,     0,   240,  1470,   240,
       0,     0,     0,  1471,     0,   440,  1472,   181,    65,    66,
      67,  1473,     0,     0,   858,   858,   858,   858,   858,   858,
     217,     0,     0,   858,   858,   858,   858,   858,   858,   858,
     858,   858,   858,   858,   858,   858,   858,   858,   858,   858,
     858,   858,   858,   858,   858,   858,   858,   858,   858,   858,
     858,     0,     0,  1474,  1475,   165,  1476,     0,     0,     0,
     165,     0,     0,     0,     0,     0,   858,     0,     0,     0,
       0,     0,     0,   698,   698,     0,     0,   441,     0,     0,
       0,   206,     0,     0,     0,     0,  1778,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1573,     0,   240,
       0,   240,     0,    50,     0,     0,     0,     0,     0,     0,
       0,   217,     0,     0,     0,     0,     0,  1070,     0,     0,
       0,     0,     0,     0,   345,   345,     0,     0,   240,     0,
       0,   240,     0,     0,     0,     0,     0,     0,     0,   210,
     211,   212,   213,   214,     0,     0,     0,   206,   240,   240,
     240,   240,   240,   240,     0,     0,   217,     0,   240,     0,
       0,   183,   217,     0,    91,  1160,     0,    93,    94,    50,
      95,   184,    97,     0,     0,     0,     0,   217,   217,     0,
     858,  1170,     0,     0,     0,     0,     0,     0,     0,     0,
     240,  1574,   219,   219,  1184,   107,   240,     0,     0,   858,
    1839,   858,     0,   242,  1575,   210,   211,   212,   213,   214,
    1576,     0,     0,     0,     0,     0,   345,     0,     0,     0,
       0,     0,     0,  1204,     0,   858,     0,   183,     0,     0,
      91,  1577,   345,    93,    94,     0,    95,  1578,    97,     0,
       0,     0,     0,     0,     0,   345,     0,     0,     0,     0,
       0,  1098,  1099,  1100,     0,     0,     0,     0,     0,   240,
     240,   107,     0,   240,     0,     0,   217,     0,     0,     0,
       0,     0,  1101,     0,   345,  1102,  1103,  1104,  1105,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1262,   452,   453,
     454,  1266,     0,     0,     0,     0,     0,     0,     0,     0,
    1124,     0,     0,     0,   240,     0,   240,     0,   455,   456,
       0,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,     0,   480,     0,     0,   345,     0,
     206,     0,   345,     0,   839,     0,     0,   481,     0,   240,
       0,   240,     0,     0,     0,     0,     0,   858,     0,   858,
       0,   858,    50,     0,     0,     0,   858,   217,     0,     0,
     858,     0,   858,     0,     0,   858,     0,     0,     0,     0,
       0,     0,     0,     0,   283,     0,   240,   240,     0,     0,
     240,     0,     0,  1360,     0,   958,   219,   240,   210,   211,
     212,   213,   214,     0,     0,     0,   525,   496,   497,   498,
     499,   500,   501,   502,   503,   504,   505,   506,   507,     0,
       0,   285,  1380,     0,     0,  1383,    93,    94,  1290,    95,
     184,    97,     0,     0,   206,     0,     0,     0,     0,   240,
       0,   240,     0,   240,   345,     0,   345,     0,   240,     0,
     217,   508,   509,     0,   107,   977,    50,     0,     0,     0,
       0,     0,     0,     0,   240,     0,     0,   858,     0,     0,
       0,     0,     0,   345,     0,     0,   345,     0,     0,   240,
     240,  1345,     0,     0,  1431,     0,     0,   240,     0,   240,
    1184,   570,   210,   211,   212,   213,   214,   571,   440,    63,
      64,    65,    66,    67,     0,     0,     0,     0,     0,    72,
     487,   240,     0,   240,   183,     0,     0,    91,   336,   240,
      93,    94,     0,    95,   184,    97,   510,   511,     0,     0,
     219,     0,     0,     0,     0,   345,     0,     0,   340,   219,
       0,   345,   240,     0,     0,     0,   219,     0,   107,   341,
     488,     0,   489,  1465,  1466,     0,     0,     0,   219,   858,
     858,   858,     0,     0,     0,   490,   858,   491,   240,   219,
     441,     0,     0,     0,   240,     0,   240,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   452,   453,   454,   900,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   345,   345,   455,   456,     0,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,     0,   480,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   242,     0,   481,     0,     0,     0,     0,
       0,     0,     0,  1549,     0,  1550,     0,   495,   496,   497,
     498,   499,   500,   501,   502,   503,   504,   505,   506,   507,
       0,     0,     0,     0,     0,     0,   240,   836,     0,     0,
       0,     0,     0,     0,     0,   219,     0,     0,     0,     0,
       0,     0,     0,   240,     0,     0,     0,   240,   240,     0,
       0,  1594,   508,   509,   345,     0,   345,     0,     0,     0,
       0,     0,   240,     0,     0,     0,     0,     0,   858,     0,
       0,     0,     0,     0,     0,     0,     0,   206,     0,   858,
       0,     0,     0,     0,     0,   858,   206,   837,   925,   858,
     926,   345,   865,     0,     0,     0,     0,     0,     0,    50,
       0,     0,   345,     0,     0,     0,     0,     0,    50,     0,
       0,   240,     0,     0,     0,     0,     0,     0,  1640,     0,
       0,     0,     0,     0,   865,     0,   931,   510,   511,     0,
       0,     0,     0,     0,     0,   210,   211,   212,   213,   214,
       0,     0,     0,     0,   210,   211,   212,   213,   214,     0,
       0,   858,     0,     0,     0,     0,     0,   183,     0,     0,
      91,   240,     0,    93,    94,     0,    95,   184,    97,   345,
     838,     0,    93,    94,     0,    95,   184,    97,   240,     0,
       0,     0,     0,     0,     0,     0,     0,   240,     0,     0,
       0,   107,   345,     0,     0,     0,     0,     0,     0,   240,
     107,   240,     0,     0,     0,     0,   452,   453,   454,     0,
       0,     0,   219,     0,     0,     0,   345,     0,   345,     0,
     240,     0,   240,     0,   345,     0,   455,   456,  1797,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,     0,   480,     0,     0,     0,   452,   453,   454,
       0,     0,     0,     0,     0,   481,     0,     0,     0,     0,
       0,     0,   219,     0,     0,     0,     0,   455,   456,   345,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,     0,   480,     0,     0,  1029,     0,     0,
       0,     0,     0,   219,     0,   219,   481, -1060, -1060, -1060,
   -1060, -1060,   472,   473,   474,   475,   476,   477,   478,   479,
    1820,   480,     0,     0,   836,     0,     0,     0,     0,     0,
       0,   219,   865,   481,     0, -1060, -1060, -1060, -1060, -1060,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,   865,   865,
     865,   865,   865,     0,     0,     0,     0,     0,     0,     0,
     865,  1124,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   345,     0,     0,   206,     0,  1128,     0,     0,     0,
       0,     0,     0,     0,   837,     0,   963,     0,   345,     0,
       0,     0,     0,     0,   283,     0,    50,     0,   219,     0,
       0,     0,     0,     0,     0,  1881,     0,  1841,     0,     0,
       0,     0,  1148,     0,   219,   219,   221,   221,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   246,     0,     0,
       0,   285,   210,   211,   212,   213,   214,  1002,     0,  1148,
       0,     0,     0,     0,   206,     0,     0,     0,   219,     0,
     206,     0,     0,     0,   183,     0,   345,    91,     0,     0,
      93,    94,     0,    95,   184,    97,    50,  1268,     0,     0,
       0,     0,    50,     0,  -397,   865,     0,     0,  1191,     0,
       0,   958,   440,   180,   181,    65,    66,    67,   107,     0,
       0,     0,     0,  1943,     0,   958,     0,     0,     0,     0,
     242,   570,   210,   211,   212,   213,   214,   571,   210,   211,
     212,   213,   214,  1029,  1943,     0,  1968,     0,     0,     0,
       0,     0,     0,     0,   183,     0,     0,    91,   336,     0,
      93,    94,   345,    95,   184,    97,    93,    94,     0,    95,
     184,    97,     0,     0,   345,     0,   345,     0,   340,     0,
       0,     0,     0,     0,   441,     0,   219,   219,   107,   341,
       0,     0,     0,     0,   107,   345,     0,   345,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,     0,   480,     0,     0,
       0,   865,   865,   865,   865,   865,   865,   219,     0,   481,
     865,   865,   865,   865,   865,   865,   865,   865,   865,   865,
     865,   865,   865,   865,   865,   865,   865,   865,   865,   865,
     865,   865,   865,   865,   865,   865,   865,   865,     0,     0,
     221,   452,   453,   454,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   865,     0,     0,     0,     0,     0,     0,
       0,   455,   456,     0,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,     0,   480,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   219,     0,
     481,     0,     0,  1008,  1009,     0,   525,   496,   497,   498,
     499,   500,   501,   502,   503,   504,   505,   506,   507,     0,
       0,     0,     0,  1010,     0,     0,     0,     0,     0,     0,
       0,  1011,  1012,  1013,   206,  1029,  1029,  1029,  1029,  1029,
    1029,     0,     0,   219,  1014,  1029,     0,     0,     0,   219,
       0,   508,   509,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,   219,   219,     0,   865,     0,     0,
       0,     0,     0,     0,   221,     0,     0,     0,     0,     0,
       0,     0,     0,   221,     0,     0,   865,     0,   865,     0,
     221,  1015,  1016,  1017,  1018,  1019,  1020,     0,     0,     0,
       0,     0,   221,     0,     0,     0,     0,     0,     0,  1021,
       0,     0,   865,   246,   183,     0,     0,    91,    92,     0,
      93,    94,     0,    95,   184,    97,   510,   511,     0,     0,
     206,  1006,   207,    40,   452,   453,   454,     0,  1022,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   107,     0,
    1468,     0,    50,   219,   455,   456,     0,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
       0,   480,     0,     0,     0,     0,     0,   246,   210,   211,
     212,   213,   214,   481,     0,     0,     0,     0,     0,     0,
       0,  1029,     0,  1029,     0,     0,     0,     0,     0,     0,
     867,     0,     0,     0,   776,     0,    93,    94,   283,    95,
     184,    97,     0,     0,     0,     0,     0,     0,     0,   221,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   893,     0,   107,     0,     0,     0,   777,     0,
     111,     0,     0,     0,   865,   285,   865,     0,   865,     0,
       0,     0,     0,   865,   219,     0,     0,   865,   206,   865,
       0,     0,   865,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1570,     0,   866,  1583,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,   577,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   866,     0,
       0,     0,     0,     0,  1139,   570,   210,   211,   212,   213,
     214,   571,     0,     0,     0,     0,  1029,     0,  1029,     0,
    1029,     0,     0,     0,     0,  1029,     0,   219,   183,     0,
       0,    91,   336,     0,    93,    94,     0,    95,   184,    97,
       0,     0,     0,     0,   865,     0,     0,     0,     0,     0,
       0,     0,   340,     0,     0,     0,  1647,  1648,     0,     0,
       0,     0,   107,   341,     0,     0,  1583,   452,   453,   454,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   221,   455,   456,     0,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   283,   480,     0,     0,     0,     0,  1029,
       0,     0,     0,     0,     0,     0,   481,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   865,   865,   865,     0,
    1058,     0,     0,   865,     0,  1795,   221,     0,     0,     0,
     285,     0,     0,  1583,     0,     0,  1081,  1082,  1083,  1084,
    1085,     0,     0,   206,     0,     0,     0,     0,  1095,  1098,
    1099,  1100,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,   221,     0,   221,
    1101,  1459,     0,  1102,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,   221,   866,     0,     0,     0,
     570,   210,   211,   212,   213,   214,   571,     0,  1124,     0,
       0,   283,   866,   866,   866,   866,   866,     0,     0,     0,
       0,     0,     0,   183,   866,     0,    91,   336,     0,    93,
      94,     0,    95,   184,    97,     0,  1076,  1198,     0,     0,
       0,     0,   206,     0,   207,    40,     0,   340,   285,     0,
       0,     0,     0,     0,  1029,  1029,     0,   107,   341,     0,
       0,   206,   221,  1188,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   865,     0,     0,   221,   221,
       0,     0,     0,    50,     0,     0,   865,     0,     0,     0,
       0,     0,   865,     0,     0,     0,   865,     0,     0,     0,
     210,   211,   212,   213,   214,     0,     0,     0,     0,     0,
       0,     0,   246,     0,     0,     0,     0,     0,   570,   210,
     211,   212,   213,   214,   571,     0,   776,     0,    93,    94,
       0,    95,   184,    97,     0,     0,     0,     0,     0,   866,
       0,   183,     0,     0,    91,   336,     0,    93,    94,     0,
      95,   184,    97,     0,  1434,     0,   107,     0,   865,     0,
     810,     0,   111,     0,   246,   340,     0,     0,  1910,     0,
       0,     0,     0,     0,     0,   107,   341,     0,     0,  1085,
    1280,     0,     0,  1280,     0,  1570,     0,     0,  1293,  1296,
    1297,  1298,  1300,  1301,  1302,  1303,  1304,  1305,  1306,  1307,
    1308,  1309,  1310,  1311,  1312,  1313,  1314,  1315,  1316,  1317,
    1318,  1319,  1320,  1321,  1322,  1323,     0,     0,     0,     0,
     221,   221,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1331,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
       0,     0,     0,     0,     0,   866,   866,   866,   866,   866,
     866,   246,     0,  1124,   866,   866,   866,   866,   866,   866,
     866,   866,   866,   866,   866,   866,   866,   866,   866,   866,
     866,   866,   866,   866,   866,   866,   866,   866,   866,   866,
     866,   866,   452,   453,   454,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   866,     0,     0,
       0,     0,   455,   456,     0,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,     0,   480,
       0,     0,     0,     0,     0,  1421,  1054,   206,     0,     0,
       0,   481,   221,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1437,     0,  1438,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,    29,     0,
       0,     0,     0,     0,     0,     0,    34,    35,    36,   206,
    1458,   207,    40,     0,     0,     0,     0,   246,     0,   208,
       0,     0,     0,   221,     0,   210,   211,   212,   213,   214,
       0,    50,     0,     0,     0,     0,     0,     0,   221,   221,
       0,   866,     0,     0,     0,     0,     0,   209,     0,     0,
       0,  1747,     0,    93,    94,  1748,    95,   184,    97,     0,
     866,     0,   866,     0,     0,  1055,    75,   210,   211,   212,
     213,   214,     0,    81,    82,    83,    84,    85,     0,     0,
       0,   107,  1588,     0,   215,     0,   866,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,     0,  1209,     0,    99,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   104,     0,
       0,     0,     0,   107,   216,     0,     0,   221,     0,   111,
       0,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1552,     0,  1553,    11,  1554,    13,     0,     0,
       0,  1555,     0,     0,     0,  1557,     0,  1558,     0,     0,
    1559,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,   866,     0,
     866,    50,   866,     0,     0,     0,     0,   866,   246,    55,
       0,   866,     0,   866,     0,     0,   866,   179,   180,   181,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,  1642,     0,     0,   182,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,   246,     0,   107,   185,     0,   350,     0,     0,   111,
     112,     0,   113,   114,     0,     0,     0,     0,   866,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,  1788,  1789,  1790,     0,     0,     0,
       0,  1794,     0,    11,   413,    13,     0,     0,     0,     0,
       0,     0,     0,     0,   759,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
     866,   866,   866,     0,     0,     0,     0,   866,     0,    50,
       0,     0,     0,     0,     0,     0,  1800,    55,     0,     0,
       0,     0,     0,     0,     0,   179,   180,   181,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   182,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,   206,     0,
       0,     0,    87,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,     0,
      50,     0,    99,  1850,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,  1860,     0,   104,   105,   106,     0,
    1865,   107,   108,     0,  1867,  1587,     0,   111,   112,     0,
     113,   114,     0,     0,     0,     0,   210,   211,   212,   213,
     214,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,    93,    94,     0,    95,   184,    97,
       0,     0,     0,     0,    11,    12,    13,     0,     0,   866,
       0,     0,     0,     0,     0,     0,  1902,     0,     0,     0,
     866,     0,   107,  1588,     0,    14,   866,    15,    16,     0,
     866,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,  1883,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,    56,
      57,    58,   866,    59,    60,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,    88,    89,
      90,    91,    92,     0,    93,    94,     0,    95,    96,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,   103,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,  1158,   111,   112,
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
      54,    55,    56,    57,    58,     0,    59,    60,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,    88,    89,    90,    91,    92,     0,    93,    94,     0,
      95,    96,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,   103,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    1346,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
       0,    52,    53,    54,    55,    56,    57,    58,     0,    59,
      60,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,    88,    89,    90,    91,    92,     0,
      93,    94,     0,    95,    96,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,   103,     0,   104,   105,   106,     0,     0,   107,   108,
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
       0,     0,    87,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,   684,   111,   112,     0,
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
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1127,
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
     109,   110,  1172,   111,   112,     0,   113,   114,     5,     6,
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
     107,   108,     0,   109,   110,  1245,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,  1247,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
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
       0,    49,  1422,     0,    50,    51,     0,     0,     0,    52,
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
     108,     0,   109,   110,  1561,   111,   112,     0,   113,   114,
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
       0,     0,   107,   108,     0,   109,   110,  1791,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,  1837,
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
       0,     0,     0,     0,   183,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   184,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   108,
       0,   109,   110,  1872,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,  1875,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,    98,
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
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1892,
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
     109,   110,  1909,   111,   112,     0,   113,   114,     5,     6,
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
     107,   108,     0,   109,   110,  1966,   111,   112,     0,   113,
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
     106,     0,     0,   107,   108,     0,   109,   110,  1969,   111,
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
       0,     0,   183,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   184,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   108,     0,   109,
     110,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,   553,     0,     0,     0,
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
       0,     0,     0,     0,    11,    12,    13,     0,     0,   823,
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
       0,     0,  1060,     0,     0,     0,     0,     0,     0,     0,
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
      11,    12,    13,     0,     0,  1636,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,   180,   181,    65,    66,    67,     0,    68,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   183,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   184,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   108,
       0,   109,   110,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,  1783,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,   180,   181,    65,    66,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,     0,
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
      55,     0,    57,    58,     0,    59,     0,    61,    62,   180,
     181,    65,    66,    67,     0,    68,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,  1101,     0,    10,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,     0,     0,
     699,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1124,    15,    16,     0,     0,     0,     0,    17,     0,
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
      94,     0,    95,   184,    97,     0,   700,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   185,     0,
       0,     0,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,     0,     0,     0,     0,     0,     0,     0,
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
     107,   185,     0,     0,   818,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,  1122,  1123,     0,     0,  1185,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1124,    15,    16,
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
      97,     0,  1186,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   185,     0,     0,     0,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,   413,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,   197,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,   179,   180,   181,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   182,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   183,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   184,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,  1239,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     185,     0,     0,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,     0,   480,     0,   233,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   481,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
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
       0,     0,   107,   185,   452,   453,   454,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   455,   456,     0,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
       0,   480,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,   481,     0,     0,    17,     0,    18,    19,
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
       0,     0,     0,     0,  1606,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   185,     0,   268,   453,
     454,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,   455,   456,
       0,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,     0,   480,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,   481,     0,    17,
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
       0,   271,     0,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   413,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
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
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,   179,   180,
     181,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   182,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,  1607,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   185,   551,     0,     0,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   713,   480,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   481,     0,     0,
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
       7,     8,     9,     0,     0,     0,     0,     0,    10,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
       0,     0,     0,   759,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1124,     0,    15,    16,     0,     0,     0,
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
       0,    10,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,     0,   480,     0,     0,   800,     0,     0,     0,
       0,     0,     0,     0,     0,   481,     0,     0,    15,    16,
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
     106,     0,     0,   107,   185,     0,     0,     0,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,     0,     0,     0,     0,   802,
       0,     0,     0,     0,     0,     0,     0,     0,  1124,     0,
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
       0,   104,   105,   106,     0,     0,   107,   185,     0,     0,
       0,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,     0,   480,     0,
       0,     0,  1236,     0,     0,     0,     0,     0,     0,     0,
     481,     0,     0,     0,    15,    16,     0,     0,     0,     0,
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
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     185,   452,   453,   454,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   455,   456,  1426,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,     0,   480,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
     481,     0,     0,    17,     0,    18,    19,    20,    21,    22,
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
       0,     0,     0,    99,     0,     0,   100,     0,     0,  1427,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   185,   452,   453,   454,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,   827,     0,    10,   455,   456,     0,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
       0,   480,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,   481,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,   644,    39,    40,     0,   828,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,   179,
     180,   181,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   182,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,     0,   273,   274,    99,   275,   276,   100,
       0,   277,   278,   279,   280,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   185,     0,     0,   281,
     282,   111,   112,     0,   113,   114,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,     0,     0,     0,   284,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1124,     0,
       0,     0,   286,   287,   288,   289,   290,   291,   292,     0,
       0,     0,   206,     0,   207,    40,     0,     0,     0,     0,
       0,     0,     0,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,    50,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   206,   327,
       0,   746,   329,   330,   331,     0,     0,     0,   332,   581,
     210,   211,   212,   213,   214,   582,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,   273,   274,     0,   275,
     276,     0,   583,   277,   278,   279,   280,     0,    93,    94,
       0,    95,   184,    97,   337,     0,   338,     0,     0,   339,
       0,   281,   282,     0,     0,     0,   210,   211,   212,   213,
     214,     0,     0,     0,     0,     0,   107,     0,     0,     0,
     747,     0,   111,     0,     0,     0,     0,     0,   183,     0,
     284,    91,    92,     0,    93,    94,     0,    95,   184,    97,
       0,     0,     0,     0,   286,   287,   288,   289,   290,   291,
     292,     0,     0,     0,   206,     0,   207,    40,     0,     0,
       0,     0,   107,     0,     0,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,    50,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     206,   327,     0,   328,   329,   330,   331,     0,     0,     0,
     332,   581,   210,   211,   212,   213,   214,   582,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,   273,   274,
       0,   275,   276,     0,   583,   277,   278,   279,   280,     0,
      93,    94,     0,    95,   184,    97,   337,     0,   338,     0,
       0,   339,     0,   281,   282,     0,   283,     0,   210,   211,
     212,   213,   214,     0,     0,     0,     0,     0,   107,     0,
       0,     0,   747,     0,   111,     0,     0,     0,     0,     0,
       0,     0,   284,   360,     0,     0,    93,    94,     0,    95,
     184,    97,     0,   285,     0,     0,   286,   287,   288,   289,
     290,   291,   292,     0,     0,     0,   206,     0,     0,     0,
       0,     0,     0,     0,   107,     0,     0,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,    50,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,     0,   327,     0,     0,   329,   330,   331,     0,
       0,     0,   332,   333,   210,   211,   212,   213,   214,   334,
       0,     0,     0,     0,     0,     0,     0,   206,     0,     0,
       0,     0,     0,     0,     0,     0,   335,  1069,     0,    91,
     336,     0,    93,    94,     0,    95,   184,    97,   337,    50,
     338,     0,     0,   339,   273,   274,     0,   275,   276,     0,
     340,   277,   278,   279,   280,     0,     0,     0,     0,     0,
     107,   341,     0,     0,     0,  1762,     0,     0,     0,   281,
     282,     0,   283,     0,     0,   210,   211,   212,   213,   214,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   183,   284,     0,
      91,     0,     0,    93,    94,     0,    95,   184,    97,   285,
       0,     0,   286,   287,   288,   289,   290,   291,   292,     0,
       0,     0,   206,     0,     0,     0,     0,     0,     0,     0,
       0,   107,     0,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,    50,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,     0,   327,
       0,     0,   329,   330,   331,     0,     0,     0,   332,   333,
     210,   211,   212,   213,   214,   334,     0,     0,     0,     0,
       0,     0,     0,   206,     0,     0,     0,     0,     0,     0,
       0,     0,   335,     0,     0,    91,   336,     0,    93,    94,
       0,    95,   184,    97,   337,    50,   338,     0,     0,   339,
     273,   274,     0,   275,   276,     0,   340,   277,   278,   279,
     280,     0,     0,     0,     0,     0,   107,   341,     0,     0,
       0,  1832,     0,     0,     0,   281,   282,     0,   283,     0,
       0,   210,   211,   212,   213,   214,     0, -1060, -1060, -1060,
   -1060,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   284,   480,   597,     0,     0,    93,
      94,     0,    95,   184,    97,   285,     0,   481,   286,   287,
     288,   289,   290,   291,   292,     0,     0,     0,   206,     0,
       0,     0,     0,     0,     0,     0,     0,   107,     0,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
      50,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,     0,   327,     0,   328,   329,   330,
     331,     0,     0,     0,   332,   333,   210,   211,   212,   213,
     214,   334,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   335,     0,
       0,    91,   336,     0,    93,    94,     0,    95,   184,    97,
     337,     0,   338,     0,     0,   339,   273,   274,     0,   275,
     276,     0,   340,   277,   278,   279,   280,     0,     0,     0,
       0,     0,   107,   341,     0,     0,     0,     0,     0,     0,
       0,   281,   282,     0,   283,   456,     0,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     284,   480,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   285,     0,   481,   286,   287,   288,   289,   290,   291,
     292,     0,     0,     0,   206,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,    50,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
       0,   327,     0,     0,   329,   330,   331,     0,     0,     0,
     332,   333,   210,   211,   212,   213,   214,   334,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   335,     0,     0,    91,   336,     0,
      93,    94,     0,    95,   184,    97,   337,     0,   338,     0,
       0,   339,     0,   273,   274,     0,   275,   276,   340,  1565,
     277,   278,   279,   280,     0,     0,     0,     0,   107,   341,
       0,     0,     0,     0,     0,     0,     0,     0,   281,   282,
       0,   283,     0,     0,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   284,   480,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   285,     0,
     481,   286,   287,   288,   289,   290,   291,   292,     0,     0,
       0,   206,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,    50,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,     0,   327,     0,
       0,   329,   330,   331,     0,     0,     0,   332,   333,   210,
     211,   212,   213,   214,   334,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   335,     0,     0,    91,   336,     0,    93,    94,     0,
      95,   184,    97,   337,     0,   338,     0,     0,   339,  1662,
    1663,  1664,  1665,  1666,     0,   340,  1667,  1668,  1669,  1670,
       0,     0,     0,     0,     0,   107,   341,     0,     0,     0,
       0,     0,     0,  1671,  1672,  1673,     0,   455,   456,     0,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,  1674,   480,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   481,  1675,  1676,  1677,
    1678,  1679,  1680,  1681,     0,     0,     0,   206,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1682,  1683,
    1684,  1685,  1686,  1687,  1688,  1689,  1690,  1691,  1692,    50,
    1693,  1694,  1695,  1696,  1697,  1698,  1699,  1700,  1701,  1702,
    1703,  1704,  1705,  1706,  1707,  1708,  1709,  1710,  1711,  1712,
    1713,  1714,  1715,  1716,  1717,  1718,  1719,  1720,  1721,  1722,
       0,     0,     0,  1723,  1724,   210,   211,   212,   213,   214,
       0,  1725,  1726,  1727,  1728,  1729,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1730,  1731,  1732,
       0,   206,     0,    93,    94,     0,    95,   184,    97,  1733,
       0,  1734,  1735,     0,  1736,     0,     0,     0,     0,     0,
       0,  1737,  1738,    50,  1739,     0,  1740,  1741,     0,   273,
     274,   107,   275,   276,     0,     0,   277,   278,   279,   280,
       0,     0,     0,     0,     0,  1574,     0,     0,     0,     0,
       0,     0,     0,     0,   281,   282,     0,     0,  1575,   210,
     211,   212,   213,   214,  1576,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   183,     0,   284,    91,    92,     0,    93,    94,     0,
      95,  1578,    97,     0,     0,     0,     0,   286,   287,   288,
     289,   290,   291,   292,     0,     0,     0,   206,     0,     0,
       0,     0,     0,     0,     0,   107,     0,     0,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,    50,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,     0,   327,     0,   328,   329,   330,   331,
       0,     0,     0,   332,   581,   210,   211,   212,   213,   214,
     582,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   273,   274,     0,   275,   276,     0,   583,   277,   278,
     279,   280,     0,    93,    94,     0,    95,   184,    97,   337,
       0,   338,     0,     0,   339,     0,   281,   282,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   107,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   284,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   286,
     287,   288,   289,   290,   291,   292,     0,     0,     0,   206,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,    50,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,     0,   327,     0,  1291,   329,
     330,   331,     0,     0,     0,   332,   581,   210,   211,   212,
     213,   214,   582,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   273,   274,     0,   275,   276,     0,   583,
     277,   278,   279,   280,     0,    93,    94,     0,    95,   184,
      97,   337,     0,   338,     0,     0,   339,     0,   281,   282,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   107,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   284,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   286,   287,   288,   289,   290,   291,   292,     0,     0,
       0,   206,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,    50,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,     0,   327,     0,
       0,   329,   330,   331,     0,     0,     0,   332,   581,   210,
     211,   212,   213,   214,   582,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   583,     0,     0,     0,     0,     0,    93,    94,     0,
      95,   184,    97,   337,     0,   338,     0,     0,   339,   452,
     453,   454,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   107,     0,     0,     0,   455,
     456,     0,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,     0,   480,   452,   453,   454,
       0,     0,     0,     0,     0,     0,     0,     0,   481,     0,
       0,     0,     0,     0,     0,     0,     0,   455,   456,     0,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,     0,   480,   452,   453,   454,     0,     0,
       0,     0,     0,     0,     0,     0,   481,     0,     0,     0,
       0,     0,     0,     0,     0,   455,   456,     0,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,     0,   480,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   481,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   452,   453,   454,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   455,   456,   482,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,     0,   480,   452,   453,   454,     0,     0,     0,     0,
       0,     0,     0,     0,   481,     0,     0,     0,     0,     0,
       0,     0,     0,   455,   456,   567,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,     0,
     480,   452,   453,   454,     0,     0,     0,     0,     0,     0,
       0,     0,   481,     0,     0,     0,     0,     0,     0,     0,
       0,   455,   456,   569,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,     0,   480,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     481,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   452,   453,   454,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   455,   456,   588,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,     0,   480,     0,
       0,  1299,     0,     0,     0,     0,     0,     0,     0,     0,
     481,     0,     0,     0,     0,     0,     0,     0,     0,   845,
     846,   592,     0,     0,     0,   847,     0,   848,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   849,
       0,     0,     0,     0,     0,     0,     0,    34,    35,    36,
     206,     0,     0,     0,   452,   453,   454,     0,     0,     0,
     208,     0,     0,     0,     0,     0,     0,     0,   792,     0,
       0,     0,    50,     0,   455,   456,     0,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
       0,   480,     0,     0,     0,     0,     0,   850,   851,   852,
     853,   854,   855,   481,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   215,     0,     0,     0,     0,
     183,    89,    90,    91,    92,     0,    93,    94,   815,    95,
     184,    97,   845,   846,     0,    99,     0,     0,   847,     0,
     848,     0,     0,     0,   856,     0,     0,     0,     0,   104,
       0,     0,   849,     0,   107,   857,     0,     0,     0,     0,
      34,    35,    36,   206,     0,     0,     0,   452,   453,   454,
     528,     0,     0,   208,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,   455,   456,     0,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,     0,   480,     0,     0,     0,     0,     0,
     850,   851,   852,   853,   854,   855,   481,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   215,     0,
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,    29,     0,     0,    99,     0,
       0,     0,     0,    34,    35,    36,   206,   856,   207,    40,
       0,     0,   104,     0,     0,     0,   208,   107,   857,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,   537,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   209,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,     0,    75,   210,   211,   212,   213,   214,     0,
      81,    82,    83,    84,    85,  1124,     0,     0,     0,     0,
       0,   215,     0,     0,     0,     0,   183,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   184,    97,    29,     0,
       0,    99,     0,     0,     0,     0,    34,    35,    36,   206,
       0,   207,    40,     0,     0,   104,     0,     0,     0,   208,
     107,   216,     0,     0,   607,     0,   111,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   209, -1060, -1060,
   -1060, -1060,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,   627,    75,   210,   211,   212,
     213,   214,     0,    81,    82,    83,    84,    85,  1124,     0,
       0,     0,     0,     0,   215,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,    29,   997,     0,    99,     0,     0,     0,     0,    34,
      35,    36,   206,     0,   207,    40,     0,     0,   104,     0,
       0,     0,   208,   107,   216,     0,     0,     0,     0,   111,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     209,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
     210,   211,   212,   213,   214,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,   215,     0,     0,
       0,     0,   183,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   184,    97,    29,     0,     0,    99,     0,     0,
       0,     0,    34,    35,    36,   206,     0,   207,    40,     0,
       0,   104,     0,     0,     0,   208,   107,   216,     0,     0,
       0,     0,   111,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   209,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1151,    75,   210,   211,   212,   213,   214,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
     215,     0,     0,     0,     0,   183,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   184,    97,    29,     0,     0,
      99,     0,     0,     0,     0,    34,    35,    36,   206,     0,
     207,    40,     0,     0,   104,     0,     0,     0,   208,   107,
     216,     0,     0,     0,     0,   111,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   209,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    75,   210,   211,   212,   213,
     214,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,   215,     0,     0,     0,     0,   183,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   184,    97,
       0,     0,     0,    99,     0,     0,   452,   453,   454,     0,
       0,     0,     0,     0,     0,     0,     0,   104,     0,     0,
       0,     0,   107,   216,     0,     0,   455,   456,   111,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,     0,   480,   452,   453,   454,     0,     0,     0,
       0,     0,     0,     0,     0,   481,     0,     0,     0,     0,
       0,     0,     0,     0,   455,   456,     0,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
       0,   480,     0,     0,     0,     0,     0,     0,     0,     0,
     452,   453,   454,   481,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     455,   456,   915,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,     0,   480,   452,   453,
     454,     0,     0,     0,     0,     0,     0,     0,     0,   481,
       0,     0,     0,     0,     0,     0,     0,     0,   455,   456,
     983,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,     0,   480,     0,     0,     0,     0,
       0,     0,     0,     0,   452,   453,   454,   481,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   455,   456,  1039,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
       0,   480,  1098,  1099,  1100,     0,     0,     0,     0,     0,
       0,     0,     0,   481,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1101,  1344,     0,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1098,  1099,  1100,
       0,  1124,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1101,     0,
    1375,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,
    1121,  1122,  1123,     0,     0,  1098,  1099,  1100,     0,     0,
       0,     0,     0,     0,     0,     0,  1124,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1101,     0,  1273,  1102,
    1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1098,  1099,  1100,     0,  1124,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1101,     0,  1443,  1102,  1103,  1104,  1105,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,  1122,  1123,     0,     0,  1098,  1099,
    1100,     0,     0,     0,     0,     0,     0,     0,     0,  1124,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1101,
       0,  1454,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,  1122,  1123,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1098,  1099,  1100,     0,  1124,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1101,     0,  1551,  1102,  1103,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,     0,
      34,    35,    36,   206,     0,   207,    40,     0,     0,     0,
       0,     0,  1124,   208,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1643,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   237,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   238,     0,     0,     0,     0,     0,     0,     0,
       0,   210,   211,   212,   213,   214,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   215,  1645,
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,     0,     0,     0,    99,     0,
      34,    35,    36,   206,     0,   207,    40,     0,     0,     0,
       0,     0,   104,   658,     0,     0,     0,   107,   239,     0,
       0,     0,     0,   111,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   209,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   210,   211,   212,   213,   214,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   215,     0,
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,     0,     0,     0,    99,     0,
      34,    35,    36,   206,     0,   207,    40,     0,     0,     0,
       0,     0,   104,   208,     0,     0,     0,   107,   659,     0,
       0,     0,     0,   660,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   237,     0,     0,   206,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   210,   211,   212,   213,   214,    50,    81,    82,    83,
      84,    85,     0,     0,   357,   358,     0,     0,   215,   206,
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,     0,     0,     0,    99,     0,
       0,    50,   210,   211,   212,   213,   214,     0,     0,   883,
     884,     0,   104,     0,     0,     0,     0,   107,   239,     0,
       0,     0,     0,   111,   359,     0,     0,   360,     0,     0,
      93,    94,     0,    95,   184,    97,     0,   210,   211,   212,
     213,   214,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   107,     0,
       0,     0,   597,     0,     0,    93,    94,     0,    95,   184,
      97,     0,     0,     0,   452,   453,   454,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   107,   455,   456,   980,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
       0,   480,   452,   453,   454,     0,     0,     0,     0,     0,
       0,     0,     0,   481,     0,     0,     0,     0,     0,     0,
       0,     0,   455,   456,     0,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,     0,   480,
    1098,  1099,  1100,     0,     0,     0,     0,     0,     0,     0,
       0,   481,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1101,     0,     0,  1102,  1103,  1104,  1105,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,  1122,  1123,  1099,  1100,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1124,
       0,     0,     0,     0,     0,     0,  1101,     0,     0,  1102,
    1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,   454,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1124,     0,     0,     0,     0,   455,
     456,     0,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,  1100,   480,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   481,     0,
       0,     0,     0,     0,  1101,     0,     0,  1102,  1103,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1124
};

static const yytype_int16 yycheck[] =
{
       5,     6,   161,     8,     9,    10,    11,    12,    13,     4,
      15,    16,    17,    18,    56,   187,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   129,    31,   676,     4,    98,
      33,   553,   108,   102,   103,     4,    30,     4,   541,    44,
    1175,   956,  1336,    46,   406,    57,   406,    52,    51,    54,
     161,   406,    57,   702,    59,    30,    30,   673,   175,   128,
     166,     4,   651,   240,   672,   108,    57,   589,   601,   602,
     945,   512,   480,   516,   517,   512,   357,   358,   359,  1053,
     756,    86,   833,  1162,   188,   976,   522,     9,    32,   108,
     108,    44,     9,   826,     9,   804,     9,    14,     9,     4,
      32,   992,     9,   108,   547,    56,     9,    14,   549,     9,
       9,     4,   549,    48,    14,     9,    70,     9,    38,     9,
       9,     9,    48,    36,     9,    48,    83,     9,     9,     9,
       9,   794,    30,    86,     9,    83,     9,     9,     9,     9,
       9,     9,   185,   251,   106,   107,   252,    48,   196,  1040,
      90,  1776,   674,    83,    38,    38,    90,    31,    54,   106,
     107,     0,    38,    83,    14,   196,   185,   185,    70,   166,
     160,    48,   160,   216,    83,    81,    50,   160,   199,    53,
     185,     4,    32,   121,   134,   135,    38,   192,   115,    50,
      51,   181,   130,   181,    38,   196,   239,   216,   216,    83,
      83,    51,   199,    70,    70,    70,   160,    83,  1833,   199,
      70,   216,    70,    70,    70,    70,   199,   157,   161,  1094,
     239,    70,    70,   157,   130,     8,   174,   236,   158,   159,
      53,    83,    70,    56,   239,    70,   129,   886,   200,    83,
      70,    70,   196,    70,   201,   199,    70,    83,    70,   254,
      73,   178,   257,   200,   174,   199,    70,    60,   193,   264,
     265,  1470,   197,   258,   181,   174,   198,   262,  1562,   165,
     442,   197,    70,    96,   197,    98,   198,   199,  1252,   102,
     103,   198,   197,   391,   197,    88,   995,   198,    91,   406,
     174,   198,   193,  1372,   197,   188,   818,   199,   198,   198,
    1379,   823,  1381,   160,   198,   128,   198,   349,   198,   198,
     198,  1171,  1063,   198,  1065,  1206,   198,   198,   198,   198,
     197,   182,   174,   198,   197,   197,   197,   197,   197,   197,
     174,  1410,   199,   199,   199,   378,   134,   135,   196,   199,
     196,   199,   199,   199,   199,   934,   160,  1010,   196,   197,
     883,   884,   524,    19,    20,  1230,   192,   973,   196,   378,
     378,   199,  1571,   199,   199,    83,   196,   436,   160,   486,
     199,   160,   377,   378,   196,   199,    70,    54,   201,   384,
     385,   386,   387,   388,   389,   199,  1595,    31,  1597,   394,
     494,   433,   166,   120,    83,   512,    83,   484,   349,   181,
     181,   199,   181,   130,   196,   179,    50,   196,   413,    53,
      83,    84,    83,    84,   196,   196,   421,   196,   535,   495,
     489,   490,   491,   492,   377,   199,   513,   432,   196,   546,
      14,   518,   549,   386,   387,   258,   389,  1516,   196,   262,
     134,   135,   160,   266,   105,   106,   107,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   911,   481,   420,   483,   484,
     485,   196,   433,   495,  1160,   174,   480,   174,   165,   102,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   180,   495,   480,   480,   412,   513,   514,
    1370,   516,   517,   518,   519,  1248,   102,    83,   102,   103,
     525,   962,   531,   528,    90,   962,   349,   420,   201,   102,
     201,   690,   537,   181,   539,   486,   134,   135,  1060,   102,
      50,    51,   547,   555,    70,   196,   989,    83,   196,   160,
     555,   164,   557,   164,   357,   358,   359,   360,   196,  1222,
    1223,  1224,  1225,  1226,  1227,   560,  1174,   233,    83,  1232,
     181,   679,   749,   681,  1190,    90,  1451,  1193,   164,   690,
      83,   196,   480,   945,   535,   945,   758,    90,   486,   997,
     945,   164,   158,   159,   875,   398,   877,   420,   196,   196,
     493,   164,   607,  1136,    83,   782,   429,   196,   134,   135,
     433,    90,    83,   436,   512,   196,   659,   134,   135,    90,
     111,   157,   158,   159,   633,   634,     4,   164,   119,   120,
     121,   122,   123,   124,   160,   181,    70,   535,   204,   708,
      57,    19,    20,   158,   159,  1505,    70,  1507,   546,  1943,
     196,   549,    69,   662,   659,   158,   159,    70,   387,  1534,
     389,   198,   199,   486,   487,   488,   489,   490,   491,   492,
     196,   119,   120,   121,   122,   123,   124,    83,   157,   158,
     159,   858,    70,   200,    90,    83,   157,   158,   159,   512,
     798,   799,    90,   870,  1091,   700,  1093,   805,   806,   805,
     191,   367,   205,    70,  1380,  1208,   165,  1225,   713,  1227,
     376,   199,   535,   105,   106,   107,   196,   383,   119,   120,
     121,   122,   123,   124,    32,  1388,   549,  1390,   164,   395,
     108,   196,  1094,    38,  1094,   198,   745,   560,   198,  1094,
      75,    76,   747,   191,   112,   113,   114,   690,    75,    76,
     198,  1359,   158,   159,   198,  1615,  1199,   580,  1374,  1619,
     158,   159,   122,   123,   124,   102,   103,  1210,   673,    53,
      54,    55,   777,    57,   132,   133,   119,   120,   121,   122,
     123,   124,   605,   606,   198,    69,  1921,   130,   131,   198,
     191,   198,   199,   198,   597,    53,    54,    55,   601,   602,
    1935,   160,   198,   199,  1326,   810,    70,   185,   198,  1485,
      70,    69,   198,    19,    20,  1251,    70,   640,   641,   822,
      70,   826,   181,   357,   358,    70,   835,    70,   945,    70,
     173,   948,   841,   199,  1807,  1808,   831,   196,   216,   160,
     199,  1803,  1804,   960,   196,   962,   196,    70,   191,   160,
     843,   844,   196,   198,   164,   233,   522,    49,    69,   181,
    1523,   239,  1525,   160,  1527,   196,   196,   203,  1230,  1532,
    1230,     9,   160,   816,   160,  1230,    78,    79,    80,   196,
     258,     8,   198,   196,   262,   708,   160,   160,    14,    91,
    1412,   198,   198,     9,    14,   199,  1572,   198,   907,   130,
     130,  1517,   197,   181,  1340,    14,  1428,   102,   197,   202,
     915,   196,   917,   111,   919,  1775,   197,   197,   392,  1779,
     197,   196,   396,   816,  1041,   930,   196,   996,     9,   872,
     157,   197,   197,  1376,   197,   197,    94,     9,   933,   944,
      14,   198,   181,   145,   146,   147,   148,   149,   422,   196,
     424,   425,   426,   427,   156,     9,   196,   933,    83,   197,
     162,   163,   199,  1626,   933,   970,   933,   199,   132,   198,
     198,   794,  1887,   796,   176,   980,   198,  1094,   983,   872,
     985,   197,   197,   197,   989,   198,   196,     9,   190,   367,
     933,   203,  1907,   816,     9,   203,   203,   948,   376,   203,
     378,  1916,   203,   997,  1181,   383,    70,   830,   831,    32,
     133,   954,   180,   160,   136,  1537,     9,   395,   197,   160,
      14,   193,   997,   997,  1546,  1034,     9,   233,   933,     9,
     182,   197,     9,   699,  1039,    14,   132,   200,  1560,     9,
     933,   203,   420,  1046,   203,    14,   203,   197,   197,   872,
     948,   956,  1047,   196,   160,  1915,   102,   203,   881,   882,
    1503,   954,   960,   197,   962,   198,   198,     9,   973,   136,
    1930,  1080,   875,   160,   877,     9,   197,  1153,  1087,   196,
     883,   884,    70,    70,    70,   908,    70,   199,    70,  1451,
       9,  1451,   196,   759,    14,   200,  1451,   198,   182,   997,
       9,   199,    14,   203,   199,   197,  1049,    14,  1051,   198,
     933,   193,    14,  1230,  1636,    19,    20,    32,  1781,  1782,
     196,   196,    32,   196,    14,   948,   196,    52,   196,    70,
      70,   954,    70,    70,   800,    70,   802,   960,   196,   962,
     160,  1153,     9,  1041,   522,  1096,   197,   136,  1153,   198,
      50,    51,    52,    53,    54,    55,  1049,    57,  1051,   198,
     196,   367,   828,    14,  1159,   182,     9,   197,   136,    69,
     376,    69,  1534,   996,  1534,   160,   203,   383,  1827,  1534,
       9,  1186,   560,  1159,    83,  1008,  1009,  1010,   200,   395,
    1159,   200,  1159,   198,  1199,     9,   196,   136,   196,   198,
     406,    14,    83,   199,   196,  1210,  1211,   197,   199,   136,
       9,  1220,    91,   157,    32,   196,  1159,    77,  1041,   197,
     182,    32,   197,   199,  1047,   198,  1049,   199,  1051,   895,
     136,    50,    51,    52,    53,    54,    55,  1240,   203,   198,
     198,   197,   197,  1248,     9,   911,   912,     9,  1243,  1072,
      69,  1773,   203,  1258,  1159,   203,  1905,   203,   203,   136,
     203,  1783,     9,     9,   197,    14,  1159,   197,   200,   198,
    1279,   199,   198,  1096,  1283,    83,   200,   198,   196,  1288,
     197,   197,     9,   197,   197,  1190,  1295,   136,  1193,     9,
     136,   199,  1941,   196,     9,    32,   197,   136,   203,   169,
     197,   165,  1125,   112,   198,    14,  1828,   203,   203,   794,
     203,   203,  1255,   197,   197,    83,   522,   199,   198,   117,
     198,   699,   198,   197,   197,   197,   136,  1486,   199,   233,
     136,    14,   198,  1136,  1451,    83,  1159,   181,   199,  1344,
      14,    14,    83,     6,   197,   197,  1351,   136,   196,   136,
    1355,  1873,  1357,   198,    14,    14,   198,   198,     9,    14,
    1365,   199,  1255,     9,   200,    68,    83,    83,   196,   181,
    1375,  1376,     9,   199,   198,   115,   102,  1481,   160,   102,
     182,   759,   172,    36,    14,    48,  1395,  1053,  1054,   196,
    1399,   182,   197,   196,   198,  1338,   182,  1406,   178,  1222,
    1223,  1224,  1225,  1226,  1227,  1348,    83,     9,   175,  1232,
     198,    83,   197,   197,   197,    14,   195,  1534,  1940,    83,
    1243,  1593,   800,  1945,   802,    81,   199,     9,    14,    83,
      14,    83,  1255,  1338,    14,    83,  1136,  1896,   816,   492,
     487,   489,  1265,  1348,   936,   990,  1912,   103,  1249,   112,
     828,  1635,  1425,   831,   117,  1907,   119,   120,   121,   122,
     123,   124,   125,   367,   609,    19,    20,  1622,  1660,  1374,
    1573,  1745,   376,  1952,   378,  1928,    30,  1476,  1757,   383,
    1472,  1618,  1092,   139,   140,   141,   142,   143,  1226,  1163,
    1009,   395,  1088,   699,   872,  1500,  1565,   388,  1503,  1221,
    1939,  1222,    56,  1036,   167,   168,   960,   170,  1954,  1332,
     433,   167,   168,  1862,   170,   171,   172,   895,  1461,  1185,
     843,   384,  1464,  1144,  1073,  1010,  1125,    -1,   191,    -1,
      -1,    -1,    -1,   911,   912,    -1,    -1,   200,    -1,   195,
     196,    -1,    -1,  1486,    -1,    -1,    -1,    -1,  1543,     6,
      -1,    -1,    -1,   759,    -1,   933,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1506,    -1,  1388,    -1,  1390,  1461,  1512,
    1236,  1514,    -1,    -1,    -1,    -1,   954,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1251,  1252,  1480,    -1,  1761,
      -1,    48,    -1,  1536,   800,  1538,   802,    -1,    -1,    -1,
      -1,  1506,    -1,    -1,  1547,  1610,    -1,  1512,    -1,  1514,
      -1,    -1,  1517,    -1,    -1,    -1,    -1,  1568,   522,    -1,
      -1,    -1,   828,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1536,    -1,    -1,    -1,    -1,    -1,    -1,  1461,  1634,
    1635,    -1,    -1,    -1,    -1,  1538,    -1,  1470,    -1,    -1,
      -1,    -1,    -1,  1476,  1547,   112,    -1,    -1,    -1,    -1,
     117,    -1,   119,   120,   121,   122,   123,   124,   125,  1047,
      -1,  1049,    -1,  1051,  1340,  1053,  1054,    -1,    -1,   233,
      -1,  1624,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   895,
    1633,    -1,    -1,    -1,    -1,    -1,  1639,    -1,    -1,    -1,
    1523,    -1,  1525,  1646,  1527,   911,   912,    -1,  1601,  1532,
     167,   168,    -1,   170,  1756,  1538,    -1,    -1,    -1,  1624,
    1543,  1880,    -1,    -1,  1547,    -1,    -1,    -1,    -1,   283,
      -1,   285,    -1,    -1,   191,    -1,    -1,  1222,  1223,   945,
    1633,  1226,  1565,   200,    -1,  1568,  1639,  1232,  1571,    -1,
      -1,    -1,  1821,  1646,    -1,    -1,    -1,    -1,  1581,    -1,
      -1,  1766,    -1,    -1,    -1,  1588,    -1,    -1,    -1,    -1,
      -1,    -1,  1595,    -1,  1597,    -1,    -1,    81,    -1,    -1,
    1603,  1159,    -1,    -1,    -1,    -1,    -1,   341,  1797,  1798,
      -1,    -1,    -1,    -1,    -1,   699,  1900,    -1,    -1,   103,
      -1,    -1,    -1,  1626,    -1,  1756,    56,  1185,    -1,    -1,
    1633,  1634,  1635,   367,    -1,    -1,  1639,    -1,    -1,    -1,
      -1,    -1,   376,  1646,    -1,    -1,    -1,    -1,    -1,   383,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,   395,    -1,    -1,  1787,    -1,    -1,  1053,  1054,    -1,
      -1,    -1,   406,    -1,    -1,   759,    -1,    56,  1236,    -1,
     164,    -1,    -1,   167,   168,  1243,   170,   171,   172,    -1,
      -1,    -1,    -1,  1251,  1252,    -1,   430,  1255,    -1,   433,
      -1,    -1,    -1,    -1,    -1,    -1,  1829,    -1,  1094,    -1,
      -1,   195,    -1,  1836,  1787,   199,   800,    -1,   802,    -1,
      -1,    -1,    -1,  1388,    -1,  1390,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    -1,    -1,   828,    -1,   480,    -1,  1871,    -1,
      -1,    -1,    -1,  1756,    -1,    -1,  1829,  1880,    -1,    -1,
      -1,    -1,    -1,  1836,    -1,  1950,    -1,    -1,    -1,    -1,
    1893,    59,    60,    -1,  1777,    -1,    -1,  1962,  1781,  1782,
      -1,    -1,  1340,    -1,  1787,    -1,    -1,  1972,   522,    -1,
    1975,    -1,    -1,  1796,    -1,    -1,    -1,    -1,  1871,  1185,
    1803,  1804,  1887,    -1,  1807,  1808,    -1,    -1,    -1,    -1,
       6,   895,    -1,    -1,    -1,    -1,    -1,    -1,  1821,    -1,
    1893,    -1,  1907,    -1,    -1,    -1,  1829,   911,   912,    -1,
    1953,  1916,    -1,  1836,    -1,  1958,    -1,    -1,    -1,    -1,
     574,    -1,   576,    -1,  1230,   579,   134,   135,    -1,    -1,
    1236,    -1,    48,   283,    -1,   285,    -1,    -1,  1523,    -1,
    1525,    -1,  1527,    -1,    -1,  1251,  1252,  1532,  1871,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1879,   611,    -1,    -1,
    1953,    -1,    -1,    -1,    -1,  1958,    -1,    -1,    -1,    -1,
    1893,    -1,    -1,    -1,    -1,     6,  1899,    -1,    -1,    -1,
      -1,    -1,    -1,  1461,   283,    -1,   285,    -1,    -1,   197,
      -1,   341,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,
      -1,   117,    -1,   119,   120,   121,   122,   123,   124,   125,
      -1,    -1,    -1,    -1,    -1,   669,   670,    48,    -1,    -1,
      -1,    -1,    -1,    -1,   678,    -1,    -1,    -1,    -1,    -1,
    1953,    -1,    -1,    -1,  1340,  1958,    -1,    -1,    -1,    -1,
      -1,  1626,   341,    -1,    -1,   699,    -1,    -1,    -1,  1053,
    1054,   167,   168,    -1,   170,    -1,    -1,    -1,    -1,    -1,
    1538,    -1,    -1,    -1,    -1,  1543,    -1,    -1,    -1,  1547,
      -1,    -1,    -1,    -1,    -1,   191,    -1,    -1,    -1,    -1,
     430,   112,    -1,   433,   200,    -1,   117,    -1,   119,   120,
     121,   122,   123,   124,   125,    -1,    -1,    -1,    -1,     6,
      -1,    -1,    -1,    -1,    -1,   759,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   430,    -1,    -1,   433,    -1,   167,   168,    -1,   170,
     794,    48,    -1,    -1,    -1,  1451,   800,    -1,   802,    -1,
      -1,    59,    60,    -1,    -1,  1633,  1634,  1635,    -1,    -1,
     191,  1639,    -1,    -1,    -1,    -1,    -1,    -1,  1646,   200,
      -1,    -1,    -1,    -1,   828,   829,    -1,    -1,    81,    -1,
      -1,  1185,   836,    -1,    -1,    -1,    -1,    -1,     6,   843,
     844,   845,   846,   847,   848,   849,  1781,  1782,    -1,    -1,
     103,    -1,    -1,   857,    -1,   112,    -1,    -1,    -1,    -1,
     117,    -1,   119,   120,   121,   122,   123,   124,   125,   873,
      -1,    -1,    -1,    -1,   574,    -1,   134,   135,  1534,    -1,
      48,    -1,  1236,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   895,    -1,    -1,    -1,    -1,    -1,  1251,  1252,    -1,
      -1,    -1,    -1,    -1,    -1,   909,    -1,   911,   912,    -1,
     167,   168,   165,   170,   167,   168,    -1,   170,   171,   172,
      78,    79,    80,    81,    -1,   574,    -1,   576,    -1,    -1,
      -1,   935,   936,    -1,   191,    -1,    -1,    -1,    -1,   197,
      -1,   945,   195,   200,   112,   103,    -1,   951,    -1,   117,
      -1,   119,   120,   121,   122,   123,   124,   125,    -1,  1787,
     964,    -1,     6,    -1,    -1,    -1,    -1,    -1,   972,   669,
     670,   975,    -1,    -1,    -1,    -1,    -1,    -1,   678,    -1,
      -1,   139,   140,   141,   142,   143,  1340,    -1,    -1,   993,
      -1,    -1,    -1,   997,    -1,    -1,    -1,    -1,    -1,   167,
     168,  1829,   170,    -1,    48,    -1,  1010,    -1,  1836,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,    -1,    -1,
     669,   670,    -1,   191,    -1,    -1,    -1,    -1,    -1,   678,
      -1,    -1,   200,    -1,    -1,    -1,    -1,   195,    -1,    -1,
      -1,    -1,    -1,  1871,    -1,    -1,    -1,    -1,    -1,  1053,
    1054,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1067,    -1,  1893,    -1,  1071,   112,  1073,
      -1,    -1,    -1,   117,    -1,   119,   120,   121,   122,   123,
     124,   125,    -1,    -1,  1088,  1089,  1090,  1091,  1092,  1093,
    1094,    -1,    -1,  1097,  1098,  1099,  1100,  1101,  1102,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1124,    -1,    -1,   167,   168,  1953,   170,    -1,    -1,    -1,
    1958,    -1,    -1,    -1,    -1,    -1,  1140,    -1,    -1,    -1,
      -1,    -1,    -1,   843,   844,    -1,    -1,   191,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,   200,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,  1173,
      -1,  1175,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1185,    -1,    -1,    -1,    -1,    -1,   836,    -1,    -1,
      -1,    -1,    -1,    -1,   843,   844,    -1,    -1,  1202,    -1,
      -1,  1205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    81,  1222,  1223,
    1224,  1225,  1226,  1227,    -1,    -1,  1230,    -1,  1232,    -1,
      -1,   161,  1236,    -1,   164,   935,    -1,   167,   168,   103,
     170,   171,   172,    -1,    -1,    -1,    -1,  1251,  1252,    -1,
    1254,   951,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1264,   125,    19,    20,   964,   195,  1270,    -1,    -1,  1273,
     200,  1275,    -1,    30,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,   935,    -1,    -1,    -1,
      -1,    -1,    -1,   993,    -1,  1299,    -1,   161,    -1,    -1,
     164,   165,   951,   167,   168,    -1,   170,   171,   172,    -1,
      -1,    -1,    -1,    -1,    -1,   964,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,  1333,
    1334,   195,    -1,  1337,    -1,    -1,  1340,    -1,    -1,    -1,
      -1,    -1,    31,    -1,   993,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,  1067,    10,    11,
      12,  1071,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,  1388,    -1,  1390,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,  1067,    -1,
      81,    -1,  1071,    -1,  1073,    -1,    -1,    69,    -1,  1433,
      -1,  1435,    -1,    -1,    -1,    -1,    -1,  1441,    -1,  1443,
      -1,  1445,   103,    -1,    -1,    -1,  1450,  1451,    -1,    -1,
    1454,    -1,  1456,    -1,    -1,  1459,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,  1470,  1471,    -1,    -1,
    1474,    -1,    -1,  1173,    -1,  1175,   233,  1481,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    68,  1202,    -1,    -1,  1205,   167,   168,   197,   170,
     171,   172,    -1,    -1,    81,    -1,    -1,    -1,    -1,  1523,
      -1,  1525,    -1,  1527,  1173,    -1,  1175,    -1,  1532,    -1,
    1534,    59,    60,    -1,   195,   196,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1548,    -1,    -1,  1551,    -1,    -1,
      -1,    -1,    -1,  1202,    -1,    -1,  1205,    -1,    -1,  1563,
    1564,   203,    -1,    -1,  1264,    -1,    -1,  1571,    -1,  1573,
    1270,   138,   139,   140,   141,   142,   143,   144,   119,   120,
     121,   122,   123,   124,    -1,    -1,    -1,    -1,    -1,   130,
     131,  1595,    -1,  1597,   161,    -1,    -1,   164,   165,  1603,
     167,   168,    -1,   170,   171,   172,   134,   135,    -1,    -1,
     367,    -1,    -1,    -1,    -1,  1264,    -1,    -1,   185,   376,
      -1,  1270,  1626,    -1,    -1,    -1,   383,    -1,   195,   196,
     171,    -1,   173,  1333,  1334,    -1,    -1,    -1,   395,  1643,
    1644,  1645,    -1,    -1,    -1,   186,  1650,   188,  1652,   406,
     191,    -1,    -1,    -1,  1658,    -1,  1660,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,   197,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1333,  1334,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   480,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1433,    -1,  1435,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,    -1,    -1,    -1,  1760,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   522,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1777,    -1,    -1,    -1,  1781,  1782,    -1,
      -1,  1481,    59,    60,  1433,    -1,  1435,    -1,    -1,    -1,
      -1,    -1,  1796,    -1,    -1,    -1,    -1,    -1,  1802,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,  1813,
      -1,    -1,    -1,    -1,    -1,  1819,    81,    91,    83,  1823,
      85,  1470,   579,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,  1481,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,  1845,    -1,    -1,    -1,    -1,    -1,    -1,  1548,    -1,
      -1,    -1,    -1,    -1,   611,    -1,   200,   134,   135,    -1,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,
      -1,  1885,    -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,
     164,  1895,    -1,   167,   168,    -1,   170,   171,   172,  1548,
     174,    -1,   167,   168,    -1,   170,   171,   172,  1912,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1921,    -1,    -1,
      -1,   195,  1571,    -1,    -1,    -1,    -1,    -1,    -1,  1933,
     195,  1935,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,   699,    -1,    -1,    -1,  1595,    -1,  1597,    -1,
    1954,    -1,  1956,    -1,  1603,    -1,    30,    31,  1658,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,   759,    -1,    -1,    -1,    -1,    30,    31,  1658,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,   794,    -1,    -1,
      -1,    -1,    -1,   800,    -1,   802,    69,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    1760,    57,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,   828,   829,    69,    -1,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   845,   846,
     847,   848,   849,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     857,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1760,    -1,    -1,    81,    -1,   873,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,   200,    -1,  1777,    -1,
      -1,    -1,    -1,    -1,    31,    -1,   103,    -1,   895,    -1,
      -1,    -1,    -1,    -1,    -1,  1845,    -1,  1796,    -1,    -1,
      -1,    -1,   909,    -1,   911,   912,    19,    20,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      -1,    68,   139,   140,   141,   142,   143,   200,    -1,   936,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,   945,    -1,
      81,    -1,    -1,    -1,   161,    -1,  1845,   164,    -1,    -1,
     167,   168,    -1,   170,   171,   172,   103,   174,    -1,    -1,
      -1,    -1,   103,    -1,   111,   972,    -1,    -1,   975,    -1,
      -1,  1921,   119,   120,   121,   122,   123,   124,   195,    -1,
      -1,    -1,    -1,  1933,    -1,  1935,    -1,    -1,    -1,    -1,
     997,   138,   139,   140,   141,   142,   143,   144,   139,   140,
     141,   142,   143,  1010,  1954,    -1,  1956,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,    -1,
     167,   168,  1921,   170,   171,   172,   167,   168,    -1,   170,
     171,   172,    -1,    -1,  1933,    -1,  1935,    -1,   185,    -1,
      -1,    -1,    -1,    -1,   191,    -1,  1053,  1054,   195,   196,
      -1,    -1,    -1,    -1,   195,  1954,    -1,  1956,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,  1088,  1089,  1090,  1091,  1092,  1093,  1094,    -1,    69,
    1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,    -1,    -1,
     233,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1140,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1185,    -1,
      69,    -1,    -1,    50,    51,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,  1222,  1223,  1224,  1225,  1226,
    1227,    -1,    -1,  1230,    91,  1232,    -1,    -1,    -1,  1236,
      -1,    59,    60,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1251,  1252,    -1,  1254,    -1,    -1,
      -1,    -1,    -1,    -1,   367,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   376,    -1,    -1,  1273,    -1,  1275,    -1,
     383,   138,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      -1,    -1,   395,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,  1299,   406,   161,    -1,    -1,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,   134,   135,    -1,    -1,
      81,   200,    83,    84,    10,    11,    12,    -1,   185,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,
    1337,    -1,   103,  1340,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,   480,   139,   140,
     141,   142,   143,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1388,    -1,  1390,    -1,    -1,    -1,    -1,    -1,    -1,
     579,    -1,    -1,    -1,   165,    -1,   167,   168,    31,   170,
     171,   172,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   522,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   611,    -1,   195,    -1,    -1,    -1,   199,    -1,
     201,    -1,    -1,    -1,  1441,    68,  1443,    -1,  1445,    -1,
      -1,    -1,    -1,  1450,  1451,    -1,    -1,  1454,    81,  1456,
      -1,    -1,  1459,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1471,    -1,   579,  1474,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   611,    -1,
      -1,    -1,    -1,    -1,   200,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,  1523,    -1,  1525,    -1,
    1527,    -1,    -1,    -1,    -1,  1532,    -1,  1534,   161,    -1,
      -1,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,    -1,  1551,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   185,    -1,    -1,    -1,  1563,  1564,    -1,    -1,
      -1,    -1,   195,   196,    -1,    -1,  1573,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   699,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    31,    57,    -1,    -1,    -1,    -1,  1626,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1643,  1644,  1645,    -1,
     829,    -1,    -1,  1650,    -1,  1652,   759,    -1,    -1,    -1,
      68,    -1,    -1,  1660,    -1,    -1,   845,   846,   847,   848,
     849,    -1,    -1,    81,    -1,    -1,    -1,    -1,   857,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,   800,    -1,   802,
      31,    32,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,   828,   829,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,   144,    -1,    69,    -1,
      -1,    31,   845,   846,   847,   848,   849,    -1,    -1,    -1,
      -1,    -1,    -1,   161,   857,    -1,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,   174,   200,    -1,    -1,
      -1,    -1,    81,    -1,    83,    84,    -1,   185,    68,    -1,
      -1,    -1,    -1,    -1,  1781,  1782,    -1,   195,   196,    -1,
      -1,    81,   895,   972,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1802,    -1,    -1,   911,   912,
      -1,    -1,    -1,   103,    -1,    -1,  1813,    -1,    -1,    -1,
      -1,    -1,  1819,    -1,    -1,    -1,  1823,    -1,    -1,    -1,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   945,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,    -1,    -1,   972,
      -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,   174,    -1,   195,    -1,  1885,    -1,
     199,    -1,   201,    -1,   997,   185,    -1,    -1,  1895,    -1,
      -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,  1088,
    1089,    -1,    -1,  1092,    -1,  1912,    -1,    -1,  1097,  1098,
    1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,  1124,    -1,    -1,    -1,    -1,
    1053,  1054,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1140,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,  1088,  1089,  1090,  1091,  1092,
    1093,  1094,    -1,    69,  1097,  1098,  1099,  1100,  1101,  1102,
    1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,  1124,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1140,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,  1254,    38,    81,    -1,    -1,
      -1,    69,  1185,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1273,    -1,  1275,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
    1299,    83,    84,    -1,    -1,    -1,    -1,  1230,    -1,    91,
      -1,    -1,    -1,  1236,    -1,   139,   140,   141,   142,   143,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,  1251,  1252,
      -1,  1254,    -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,
      -1,   165,    -1,   167,   168,   169,   170,   171,   172,    -1,
    1273,    -1,  1275,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,   195,   196,    -1,   156,    -1,  1299,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,   200,    -1,   176,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   190,    -1,
      -1,    -1,    -1,   195,   196,    -1,    -1,  1340,    -1,   201,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1441,    -1,  1443,    27,  1445,    29,    -1,    -1,
      -1,  1450,    -1,    -1,    -1,  1454,    -1,  1456,    -1,    -1,
    1459,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1441,    -1,
    1443,   103,  1445,    -1,    -1,    -1,    -1,  1450,  1451,   111,
      -1,  1454,    -1,  1456,    -1,    -1,  1459,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,  1551,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,
      -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,
     192,  1534,    -1,   195,   196,    -1,   198,    -1,    -1,   201,
     202,    -1,   204,   205,    -1,    -1,    -1,    -1,  1551,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,  1643,  1644,  1645,    -1,    -1,    -1,
      -1,  1650,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
    1643,  1644,  1645,    -1,    -1,    -1,    -1,  1650,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,  1659,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,
     124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    81,    -1,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
     103,    -1,   176,  1802,    -1,   179,    -1,    -1,    -1,    -1,
      -1,   185,    -1,    -1,  1813,    -1,   190,   191,   192,    -1,
    1819,   195,   196,    -1,  1823,   128,    -1,   201,   202,    -1,
     204,   205,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,  1802,
      -1,    -1,    -1,    -1,    -1,    -1,  1885,    -1,    -1,    -1,
    1813,    -1,   195,   196,    -1,    48,  1819,    50,    51,    -1,
    1823,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,  1847,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,   112,
     113,   114,  1885,   116,   117,   118,   119,   120,   121,   122,
     123,   124,    -1,   126,   127,   128,   129,   130,   131,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
     153,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
     173,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,
      -1,    -1,   185,   186,    -1,   188,    -1,   190,   191,   192,
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
     110,   111,   112,   113,   114,    -1,   116,   117,   118,   119,
     120,   121,   122,   123,   124,    -1,   126,   127,   128,   129,
     130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,   153,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,   173,    -1,    -1,   176,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,   185,   186,    -1,   188,    -1,
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
      -1,   108,   109,   110,   111,   112,   113,   114,    -1,   116,
     117,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,   153,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,   173,    -1,    -1,   176,
      -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,   186,
      -1,   188,    -1,   190,   191,   192,    -1,    -1,   195,   196,
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
      92,    93,    94,    95,    96,    -1,    98,    -1,   100,    -1,
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
      -1,   100,   101,    -1,   103,   104,    -1,    -1,    -1,   108,
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
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    99,
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
      94,    -1,    96,    97,    98,    -1,   100,    -1,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,
     114,    -1,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,   153,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,   173,
      -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,
      -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,    -1,
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
      27,    28,    29,    -1,    -1,    32,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    31,    -1,    13,    34,    35,    36,    37,
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
      -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     195,   196,    -1,    -1,   199,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,   174,    -1,   176,    -1,    -1,   179,    -1,    -1,
      -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,
     192,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
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
      -1,    -1,   108,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
     176,    -1,    -1,   179,    -1,    -1,    -1,    -1,   200,   185,
      -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,
     196,    -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    38,    -1,    -1,    -1,    -1,
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
      -1,    -1,   195,   196,    10,    11,    12,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,
      -1,    -1,    -1,    -1,   200,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,   198,    11,
      12,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    69,    -1,    56,
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
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,    -1,
      -1,    -1,    -1,   200,   185,    -1,    -1,    -1,    -1,   190,
     191,   192,    -1,    -1,   195,   196,   197,    -1,    -1,    -1,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    32,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
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
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    50,    51,    -1,    -1,    -1,
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
      -1,    13,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    50,    51,
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
      -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
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
      -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,    -1,
      -1,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
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
      13,    30,    31,    32,    33,    34,    35,    36,    37,    38,
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
      -1,    -1,   195,   196,    10,    11,    12,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    27,    -1,    13,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    69,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,   102,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,     3,     4,   176,     6,     7,   179,
      -1,    10,    11,    12,    13,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,    -1,    28,
      29,   201,   202,    -1,   204,   205,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
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
     199,    -1,   201,    -1,    -1,    -1,    -1,    -1,   161,    -1,
      57,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    83,    84,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      81,   128,    -1,   130,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,   161,    10,    11,    12,    13,    -1,
     167,   168,    -1,   170,   171,   172,   173,    -1,   175,    -1,
      -1,   178,    -1,    28,    29,    -1,    31,    -1,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,   195,    -1,
      -1,    -1,   199,    -1,   201,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    57,   164,    -1,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    68,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   195,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,    -1,    -1,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   161,    91,    -1,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,   173,   103,
     175,    -1,    -1,   178,     3,     4,    -1,     6,     7,    -1,
     185,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
     195,   196,    -1,    -1,    -1,   200,    -1,    -1,    -1,    28,
      29,    -1,    31,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,    57,    -1,
     164,    -1,    -1,   167,   168,    -1,   170,   171,   172,    68,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   195,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
      -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,   173,   103,   175,    -1,    -1,   178,
       3,     4,    -1,     6,     7,    -1,   185,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,
      -1,   200,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,
      -1,   139,   140,   141,   142,   143,    -1,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    57,    57,   164,    -1,    -1,   167,
     168,    -1,   170,   171,   172,    68,    -1,    69,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,    -1,   130,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,
      -1,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
     173,    -1,   175,    -1,    -1,   178,     3,     4,    -1,     6,
       7,    -1,   185,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    29,    -1,    31,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      57,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    69,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,   173,    -1,   175,    -1,
      -1,   178,    -1,     3,     4,    -1,     6,     7,   185,   186,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,   195,   196,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,
      -1,    31,    -1,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    57,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      69,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,    -1,
      -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,   173,    -1,   175,    -1,    -1,   178,     3,
       4,     5,     6,     7,    -1,   185,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    57,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,   162,   163,
      -1,    81,    -1,   167,   168,    -1,   170,   171,   172,   173,
      -1,   175,   176,    -1,   178,    -1,    -1,    -1,    -1,    -1,
      -1,   185,   186,   103,   188,    -1,   190,   191,    -1,     3,
       4,   195,     6,     7,    -1,    -1,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   125,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    29,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   161,    -1,    57,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,    -1,   130,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,   161,    10,    11,
      12,    13,    -1,   167,   168,    -1,   170,   171,   172,   173,
      -1,   175,    -1,    -1,   178,    -1,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,   168,    -1,
     170,   171,   172,   173,    -1,   175,    -1,    -1,   178,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,    30,
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
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,   198,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,   198,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,   198,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,   198,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,   198,    -1,    -1,    -1,    56,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,    -1,
      -1,    -1,   103,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,    69,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,   197,   170,
     171,   172,    50,    51,    -1,   176,    -1,    -1,    56,    -1,
      58,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,
      -1,    -1,    70,    -1,   195,   196,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    -1,    -1,    10,    11,    12,
     136,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,    69,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    70,    -1,    -1,   176,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,   185,    83,    84,
      -1,    -1,   190,    -1,    -1,    -1,    91,   195,   196,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    69,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    70,    -1,
      -1,   176,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,   190,    -1,    -1,    -1,    91,
     195,   196,    -1,    -1,   199,    -1,   201,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    69,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    70,    71,    -1,   176,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,   190,    -1,
      -1,    -1,    91,   195,   196,    -1,    -1,    -1,    -1,   201,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    70,    -1,    -1,   176,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,   190,    -1,    -1,    -1,    91,   195,   196,    -1,    -1,
      -1,    -1,   201,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    70,    -1,    -1,
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
      -1,    -1,    -1,   176,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   190,    -1,    -1,
      -1,    -1,   195,   196,    -1,    -1,    30,    31,   201,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,   136,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     136,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   136,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,   136,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
     136,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   136,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,   136,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,   136,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,   136,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      -1,    -1,    69,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   136,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,   136,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      -1,    -1,   190,    91,    -1,    -1,    -1,   195,   196,    -1,
      -1,    -1,    -1,   201,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      -1,    -1,   190,    91,    -1,    -1,    -1,   195,   196,    -1,
      -1,    -1,    -1,   201,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   103,   145,   146,   147,
     148,   149,    -1,    -1,   111,   112,    -1,    -1,   156,    81,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      -1,   103,   139,   140,   141,   142,   143,    -1,    -1,   111,
     112,    -1,   190,    -1,    -1,    -1,    -1,   195,   196,    -1,
      -1,    -1,    -1,   201,   161,    -1,    -1,   164,    -1,    -1,
     167,   168,    -1,   170,   171,   172,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,
      -1,    -1,   164,    -1,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   195,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      51,    52,    53,    54,    55,    12,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     164,   216,   217,   218,   219,   223,    83,   201,   293,   294,
      83,   295,   121,   130,   120,   130,   196,   196,   196,   196,
     213,   265,   485,   196,   196,    70,    70,    70,    70,    70,
     338,    83,    90,   157,   158,   159,   474,   475,   164,   199,
     223,   223,   213,   266,   485,   165,   196,   485,   485,    83,
     192,   199,   359,    28,   336,   340,   348,   349,   453,   457,
     228,   199,    90,   414,   474,    90,   474,   474,    32,   164,
     181,   486,   196,     9,   198,    38,   245,   165,   264,   485,
     119,   191,   246,   328,   198,   198,   198,   198,   198,   198,
     198,   198,    10,    11,    12,    30,    31,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      57,    69,   198,    70,    70,   199,   160,   131,   171,   173,
     186,   188,   267,   326,   327,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    59,    60,
     134,   135,   443,    70,   199,   448,   196,   196,    70,   199,
     201,   461,   196,   245,   246,    14,   348,   198,   136,    49,
     213,   438,    90,   336,   349,   160,   453,   136,   203,     9,
     423,   260,   336,   349,   453,   486,   160,   196,   415,   443,
     448,   197,   348,    32,   230,     8,   360,     9,   198,   230,
     231,   338,   339,   348,   213,   279,   234,   198,   198,   198,
     138,   144,   506,   506,   181,   505,   196,   111,   506,    14,
     160,   138,   144,   161,   213,   215,   198,   198,   198,   240,
     115,   178,   198,   216,   218,   216,   218,   164,   218,   223,
     223,   199,     9,   424,   198,   102,   164,   199,   453,     9,
     198,    14,     9,   198,   130,   130,   453,   478,   338,   336,
     349,   453,   456,   457,   197,   181,   257,   137,   453,   467,
     468,   348,   368,   369,   338,   389,   389,   368,   389,   198,
      70,   443,   157,   475,    82,   348,   453,    90,   157,   475,
     223,   212,   198,   199,   252,   262,   398,   400,    91,   196,
     201,   361,   362,   364,   407,   411,   459,   461,   479,    14,
     102,   480,   355,   356,   357,   289,   290,   441,   442,   197,
     197,   197,   197,   197,   200,   229,   230,   247,   254,   261,
     441,   348,   202,   204,   205,   213,   487,   488,   506,    38,
     174,   291,   292,   348,   482,   196,   485,   255,   245,   348,
     348,   348,   348,    32,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   412,   348,
     348,   463,   463,   348,   470,   471,   130,   199,   214,   215,
     460,   461,   265,   213,   266,   485,   485,   264,   246,    38,
     340,   343,   345,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   165,   199,   213,   444,
     445,   446,   447,   460,   463,   348,   291,   291,   463,   348,
     467,   245,   197,   348,   196,   437,     9,   423,   197,   197,
      38,   348,    38,   348,   415,   197,   197,   197,   460,   291,
     199,   213,   444,   445,   460,   197,   228,   283,   199,   345,
     348,   348,    94,    32,   230,   277,   198,    27,   102,    14,
       9,   197,    32,   199,   280,   506,    31,    91,   174,   226,
     499,   500,   501,   196,     9,    50,    51,    56,    58,    70,
     138,   139,   140,   141,   142,   143,   185,   196,   224,   375,
     378,   381,   384,   387,   393,   408,   416,   417,   419,   420,
     213,   504,   228,   196,   238,   199,   198,   199,   198,   223,
     198,   102,   164,   111,   112,   219,   220,   221,   222,   219,
     213,   348,   294,   417,    83,     9,   197,   197,   197,   197,
     197,   197,   197,   198,    50,    51,   495,   497,   498,   132,
     270,   196,     9,   197,   197,   136,   203,     9,   423,     9,
     423,   203,   203,   203,   203,    83,    85,   213,   476,   213,
      70,   200,   200,   209,   211,    32,   133,   269,   180,    54,
     165,   180,   402,   349,   136,     9,   423,   197,   160,   506,
     506,    14,   360,   289,   228,   193,     9,   424,   506,   507,
     443,   448,   443,   200,     9,   423,   182,   453,   348,   197,
       9,   424,    14,   352,   248,   132,   268,   196,   485,   348,
      32,   203,   203,   136,   200,     9,   423,   348,   486,   196,
     258,   253,   263,    14,   480,   256,   245,    71,   453,   348,
     486,   203,   200,   197,   197,   203,   200,   197,    50,    51,
      70,    78,    79,    80,    91,   138,   139,   140,   141,   142,
     143,   156,   185,   213,   376,   379,   382,   385,   388,   408,
     419,   426,   428,   429,   433,   436,   213,   453,   453,   136,
     268,   443,   448,   197,   348,   284,    75,    76,   285,   228,
     337,   228,   339,   102,    38,   137,   274,   453,   417,   213,
      32,   230,   278,   198,   281,   198,   281,     9,   423,    91,
     226,   136,   160,     9,   423,   197,   174,   487,   488,   489,
     487,   417,   417,   417,   417,   417,   422,   425,   196,    70,
      70,    70,    70,    70,   196,   417,   160,   199,    10,    11,
      12,    31,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    69,   160,   486,   200,   408,   199,
     242,   218,   218,   213,   219,   219,     9,   424,   200,   200,
      14,   453,   198,   182,     9,   423,   213,   271,   408,   199,
     467,   137,   453,    14,   348,   348,   203,   348,   200,   209,
     506,   271,   199,   401,    14,   197,   348,   361,   460,   198,
     506,   193,   200,    32,   493,   442,    38,    83,   174,   444,
     445,   447,   444,   445,   506,    38,   174,   348,   417,   289,
     196,   408,   269,   353,   249,   348,   348,   348,   200,   196,
     291,   270,    32,   269,   506,    14,   268,   485,   412,   200,
     196,    14,    78,    79,    80,   213,   427,   427,   429,   431,
     432,    52,   196,    70,    70,    70,    70,    70,    90,   157,
     196,   160,     9,   423,   197,   437,    38,   348,   269,   200,
      75,    76,   286,   337,   230,   200,   198,    95,   198,   274,
     453,   196,   136,   273,    14,   228,   281,   105,   106,   107,
     281,   200,   506,   182,   136,   160,   506,   213,   174,   499,
       9,   197,   423,   136,   203,     9,   423,   422,   370,   371,
     417,   390,   417,   418,   390,   370,   390,   361,   363,   365,
     197,   130,   214,   417,   472,   473,   417,   417,   417,    32,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   504,    83,   243,   200,   200,   222,
     198,   417,   498,   102,   103,   494,   496,     9,   299,   197,
     196,   340,   345,   348,   136,   203,   200,   480,   299,   166,
     179,   199,   397,   404,   166,   199,   403,   136,   198,   493,
     506,   360,   507,    83,   174,    14,    83,   486,   453,   348,
     197,   289,   199,   289,   196,   136,   196,   291,   197,   199,
     506,   199,   198,   506,   269,   250,   415,   291,   136,   203,
       9,   423,   428,   431,   372,   373,   429,   391,   429,   430,
     391,   372,   391,   157,   361,   434,   435,    81,   429,   453,
     199,   337,    32,    77,   230,   198,   339,   273,   467,   274,
     197,   417,   101,   105,   198,   348,    32,   198,   282,   200,
     182,   506,   213,   136,   174,    32,   197,   417,   417,   197,
     203,     9,   423,   136,   203,     9,   423,   203,   203,   203,
     136,     9,   423,   197,   136,   200,     9,   423,   417,    32,
     197,   228,   198,   198,   213,   506,   506,   494,   408,     6,
     112,   117,   120,   125,   167,   168,   170,   200,   300,   325,
     326,   327,   332,   333,   334,   335,   441,   467,   348,   200,
     199,   200,    54,   348,   348,   348,   360,    38,    83,   174,
      14,    83,   348,   196,   493,   197,   299,   197,   289,   348,
     291,   197,   299,   480,   299,   198,   199,   196,   197,   429,
     429,   197,   203,     9,   423,   136,   203,     9,   423,   203,
     203,   203,   136,   197,     9,   423,   299,    32,   228,   198,
     197,   197,   197,   235,   198,   198,   282,   228,   136,   506,
     506,   136,   417,   417,   417,   417,   361,   417,   417,   417,
     199,   200,   496,   132,   133,   186,   214,   483,   506,   272,
     408,   112,   335,    31,   125,   138,   144,   165,   171,   309,
     310,   311,   312,   408,   169,   317,   318,   128,   196,   213,
     319,   320,   301,   246,   506,     9,   198,     9,   198,   198,
     480,   326,   197,   296,   165,   399,   200,   200,    83,   174,
      14,    83,   348,   291,   117,   350,   493,   200,   493,   197,
     197,   200,   199,   200,   299,   289,   136,   429,   429,   429,
     429,   361,   200,   228,   233,   236,    32,   230,   276,   228,
     506,   197,   417,   136,   136,   136,   228,   408,   408,   485,
      14,   214,     9,   198,   199,   483,   480,   312,   181,   199,
       9,   198,     3,     4,     5,     6,     7,    10,    11,    12,
      13,    27,    28,    29,    57,    71,    72,    73,    74,    75,
      76,    77,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   137,   138,   145,   146,   147,   148,   149,
     161,   162,   163,   173,   175,   176,   178,   185,   186,   188,
     190,   191,   213,   405,   406,     9,   198,   165,   169,   213,
     320,   321,   322,   198,    83,   331,   245,   302,   483,   483,
      14,   246,   200,   297,   298,   483,    14,    83,   348,   197,
     196,   493,   198,   199,   323,   350,   493,   296,   200,   197,
     429,   136,   136,    32,   230,   275,   276,   228,   417,   417,
     417,   200,   198,   198,   417,   408,   305,   506,   313,   314,
     416,   310,    14,    32,    51,   315,   318,     9,    36,   197,
      31,    50,    53,    14,     9,   198,   215,   484,   331,    14,
     506,   245,   198,    14,   348,    38,    83,   396,   199,   228,
     493,   323,   200,   493,   429,   429,   228,    99,   241,   200,
     213,   226,   306,   307,   308,     9,   423,     9,   423,   200,
     417,   406,   406,    68,   316,   321,   321,    31,    50,    53,
     417,    83,   181,   196,   198,   417,   485,   417,    83,     9,
     424,   228,   200,   199,   323,    97,   198,   115,   237,   160,
     102,   506,   182,   416,   172,    14,   495,   303,   196,    38,
      83,   197,   200,   228,   198,   196,   178,   244,   213,   326,
     327,   182,   417,   182,   287,   288,   442,   304,    83,   200,
     408,   242,   175,   213,   198,   197,     9,   424,   122,   123,
     124,   329,   330,   287,    83,   272,   198,   493,   442,   507,
     197,   197,   198,   195,   490,   329,    38,    83,   174,   493,
     199,   491,   492,   506,   198,   199,   324,   507,    83,   174,
      14,    83,   490,   228,     9,   424,    14,   494,   228,    38,
      83,   174,    14,    83,   348,   324,   200,   492,   506,   200,
      83,   174,    14,    83,   348,    14,    83,   348,   348
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
     453,   454,   454,   454,   454,   454,   454,   454,   454,   455,
     455,   455,   455,   455,   455,   455,   455,   455,   456,   457,
     457,   458,   458,   459,   459,   459,   460,   460,   461,   461,
     461,   462,   462,   462,   463,   463,   464,   464,   465,   465,
     465,   465,   465,   465,   466,   466,   466,   466,   466,   467,
     467,   467,   467,   467,   467,   468,   468,   469,   469,   469,
     469,   469,   469,   469,   469,   470,   470,   471,   471,   471,
     471,   472,   472,   473,   473,   473,   473,   474,   474,   474,
     474,   475,   475,   475,   475,   475,   475,   476,   476,   476,
     477,   477,   477,   477,   477,   477,   477,   477,   477,   477,
     477,   478,   478,   479,   479,   480,   480,   481,   481,   481,
     481,   482,   482,   483,   483,   484,   484,   485,   485,   486,
     486,   487,   487,   488,   489,   489,   489,   489,   490,   490,
     491,   491,   492,   492,   493,   493,   494,   494,   495,   496,
     496,   497,   497,   497,   497,   498,   498,   498,   499,   499,
     499,   499,   500,   500,   501,   501,   501,   501,   502,   503,
     504,   504,   505,   505,   506,   506,   506,   506,   506,   506,
     506,   506,   506,   506,   506,   507,   507
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
       3,     1,     1,     1,     1,     3,     1,     4,     3,     1,
       1,     1,     1,     1,     3,     3,     4,     4,     3,     1,
       1,     7,     9,     7,     6,     8,     1,     2,     4,     4,
       1,     1,     1,     4,     1,     0,     1,     2,     1,     1,
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
#line 751 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
#line 6838 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 754 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 6846 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 761 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 6852 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 762 "hphp.y" /* yacc.c:1646  */
    { }
#line 6858 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 765 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 6864 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 766 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6870 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 767 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6876 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 768 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6882 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 769 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6888 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 770 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 6894 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 771 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 6902 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 6909 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 6915 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6921 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 6927 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 779 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6933 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6941 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 784 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6950 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 789 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6957 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 792 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6964 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 795 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6972 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 799 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6980 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 803 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6988 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 806 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 6995 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 811 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7001 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 812 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7007 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 813 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7013 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 814 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7019 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 815 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7025 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 816 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7031 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 817 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7037 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 818 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7043 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 819 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7049 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 820 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7055 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 821 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7061 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 822 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7067 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 823 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7073 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 902 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7079 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 904 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7085 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 909 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7091 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 910 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7098 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 916 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7104 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 920 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7110 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 921 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7116 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 923 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7122 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 925 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7128 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 930 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7134 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 931 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7141 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 937 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7147 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 941 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7154 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 943 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7161 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 945 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7168 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 950 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7174 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 952 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7180 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 955 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7186 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 957 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7192 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 958 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7198 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 963 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7207 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 970 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7216 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 978 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7223 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 981 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7230 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 987 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7236 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 988 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7242 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 991 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7248 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 992 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7254 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 993 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7260 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 994 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7266 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 997 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7272 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1001 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7278 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1006 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7284 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1007 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7291 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1009 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7299 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1013 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7306 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1016 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7314 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1020 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7321 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1022 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7329 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1025 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7336 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1027 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7344 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1030 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7350 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1031 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7356 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1032 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7362 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1033 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7368 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1034 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7374 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1035 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7380 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1036 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7386 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1037 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7392 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7398 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1039 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7404 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1040 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7410 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1041 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7416 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1042 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7422 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7428 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1044 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7435 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1048 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7442 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1050 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7450 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7457 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1057 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 7465 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1061 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 7473 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1070 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7479 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1071 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 7485 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1074 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7491 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1075 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 7497 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 7503 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1077 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 7512 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1081 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7518 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1082 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7524 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7530 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7536 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1085 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7542 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7548 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1087 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7554 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1088 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7560 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1089 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7566 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1090 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 7572 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1091 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7578 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1092 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 7588 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1100 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);}
#line 7594 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1101 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7600 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1110 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 7606 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1111 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7612 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1115 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7619 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1117 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7627 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1123 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7633 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1124 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7639 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1128 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 7645 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1129 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7651 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1133 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 7657 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1139 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7666 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1145 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7675 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7684 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1158 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7693 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1165 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7702 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1171 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7711 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1179 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7718 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1183 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 7724 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1187 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7731 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1191 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 7737 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1197 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7744 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 7762 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1215 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7769 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 7787 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 1232 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7794 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 1235 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7802 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1240 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7809 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1243 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7817 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1249 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 7823 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1252 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 7829 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1256 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7836 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1259 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7847 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1267 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7854 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1270 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7865 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1278 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7871 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1279 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 7878 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1283 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7884 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1286 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1289 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 7896 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1290 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 7902 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1291 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 7910 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1294 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 7916 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1295 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 7922 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1299 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7928 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1300 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7934 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1303 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7940 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1304 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7946 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1307 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7952 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1308 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7958 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1311 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 7964 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1313 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 7970 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1316 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 7976 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1318 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 7982 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1322 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7988 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1323 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7994 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1326 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8000 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1327 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8006 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1328 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8012 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1332 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8018 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1334 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8024 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1337 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8030 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1339 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8036 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1342 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8042 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1344 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8048 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1347 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8054 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1349 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8060 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1353 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8066 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1355 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8073 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1360 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8079 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1361 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8085 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1362 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8091 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1363 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8097 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1368 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8103 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1370 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8109 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1371 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8115 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1374 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8121 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1375 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8127 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1380 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8133 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1381 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8139 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1386 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8145 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1387 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8151 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1390 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8157 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1391 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8163 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1394 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8169 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1395 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8175 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1403 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8182 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1409 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8189 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1415 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8197 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1419 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8203 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1423 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8210 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1428 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8217 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1433 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8225 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1436 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8231 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1442 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8238 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1446 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8245 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1451 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8252 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1456 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8259 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1461 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8266 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8273 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1472 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8280 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1478 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8287 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1486 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8294 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1491 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8301 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1496 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8309 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1500 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8315 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1503 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8322 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1507 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 8329 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1511 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 8337 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1514 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8343 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1519 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8350 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1522 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8357 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1526 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8364 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1530 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8371 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1534 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8378 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1538 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8385 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1543 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8392 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1548 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8399 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1554 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8405 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1555 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8411 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1558 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,false);}
#line 8417 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1559 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),true,false);}
#line 8423 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1560 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,true);}
#line 8429 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1562 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),false, false);}
#line 8435 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1564 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),false,true);}
#line 8441 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1566 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),true, false);}
#line 8447 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1570 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8453 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1571 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 8459 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1574 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8465 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1575 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8471 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1576 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 8477 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1580 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 8483 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1582 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 8489 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1583 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 8495 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1584 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 8501 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1589 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8507 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1590 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8513 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1593 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8520 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1598 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 8526 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1604 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8532 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1605 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8538 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1608 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 8544 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1609 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 8551 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1612 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 8557 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1613 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 8564 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1615 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8571 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1618 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 8578 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1620 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8584 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1623 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8592 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1630 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8601 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1638 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8609 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1645 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8618 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1650 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 8624 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1652 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8630 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1654 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8636 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1656 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 8642 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1658 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 8648 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1659 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 8655 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1662 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 8661 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1665 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8667 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1666 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8673 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1667 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8679 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1673 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 8685 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1678 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 8692 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1681 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 8700 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1688 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 8706 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1689 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 8713 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1694 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 8720 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1697 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 8726 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1704 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 8733 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1706 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8739 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1710 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8745 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1715 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8751 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1717 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8757 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8763 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1720 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 8774 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1726 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 8780 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1728 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 8786 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1729 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 8792 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1733 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,0,(yyvsp[0]),0);}
#line 8798 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1735 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),0,(yyvsp[0]),0);}
#line 8804 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1740 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8810 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1743 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8816 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1744 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 8822 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1748 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 8828 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1749 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 8834 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1753 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 8841 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1756 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 8848 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1761 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 8855 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1766 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 8861 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1767 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 8868 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1769 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 8874 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1773 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 8880 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1774 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 8886 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1775 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 8892 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1776 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 8898 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1780 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8904 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1781 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 8910 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1782 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 8916 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1783 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 8922 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1784 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 8928 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1786 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 8934 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1788 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 8940 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1792 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 8948 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1795 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 8954 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1796 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 8960 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1800 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8966 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1801 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 8972 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1805 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8978 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1806 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 8984 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1809 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8990 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1810 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8996 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1813 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9002 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1814 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9008 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1817 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9014 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1819 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9020 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1822 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9026 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1823 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9032 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1824 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9038 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1825 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9044 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1826 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9050 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1827 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9056 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9062 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9068 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1833 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9074 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9080 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9086 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9092 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1842 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9098 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1844 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9104 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1845 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9110 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9116 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1850 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9122 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1852 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9128 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1856 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9134 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1858 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9140 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1862 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9148 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1866 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9155 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1870 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9161 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1874 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9167 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1876 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9173 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1877 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9179 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1878 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9185 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1879 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9191 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1880 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9197 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1883 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9203 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1887 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9209 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1888 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9215 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1892 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9221 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1893 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9227 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1897 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9233 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1898 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9239 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1899 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9245 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 1900 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9251 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 1904 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9257 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 1909 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9263 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 1913 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 9269 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 1917 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9275 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 1921 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 9281 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 1925 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9287 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 1930 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9293 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 1934 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9299 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 1935 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9305 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 1936 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9311 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 1937 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9317 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 1938 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9323 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 1943 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 9329 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 1944 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 9335 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 1945 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 9341 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 1948 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 9347 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 1949 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 9353 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 9359 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 1951 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 9365 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 1952 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 9371 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 1953 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 9377 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 1954 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 9383 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 1955 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 9389 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 9395 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 9401 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 1958 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 9407 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 1959 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 9413 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 1960 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 9419 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 1961 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 9425 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 9431 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 9437 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 9443 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 9449 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 1966 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 9455 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 1967 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 9461 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 1968 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 9467 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 1969 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 9473 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 1970 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 9479 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 1971 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 9485 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 1972 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 9491 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 9497 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 1974 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 9503 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 1975 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 9509 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 9515 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 1977 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 9521 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 1978 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 9527 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 1979 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 9533 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 1980 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 9539 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 1981 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 9545 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 9551 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 1983 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 9557 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 1984 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 9563 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 1985 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 9569 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 9575 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 1987 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 9581 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 1988 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 9587 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 1989 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 9593 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 1990 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 9599 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 9605 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 1992 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 9612 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 9618 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 1995 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 9625 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 1997 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 9631 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 9637 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9643 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2001 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 9649 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2002 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 9655 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 9661 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9667 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 9673 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2006 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 9679 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 9685 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2008 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 9691 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2009 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 9697 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2010 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 9703 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2011 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 9709 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2012 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 9715 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 9721 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9727 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9733 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2016 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9739 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9745 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9751 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2019 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9757 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2020 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9763 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2021 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9769 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2022 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 9775 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2023 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 9781 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2024 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9787 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2031 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 9793 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2032 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9799 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2037 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9808 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2043 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9819 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2051 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9828 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2057 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9839 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 9852 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 9866 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2085 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9876 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 9890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2103 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9900 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 9916 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 9929 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 9941 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2135 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9951 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 9963 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2153 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 9969 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2154 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 9975 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2156 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9981 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2160 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 9988 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2162 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9994 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2169 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10000 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2172 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10006 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2179 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10012 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2182 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10018 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2187 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10024 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2188 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10030 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2193 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10036 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2194 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10042 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2198 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_ARRAY);}
#line 10048 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2202 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10054 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2203 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10060 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10066 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2209 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10072 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2214 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10078 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2215 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10084 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2220 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10090 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2221 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10096 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2227 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10102 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2229 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10108 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2234 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10114 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2235 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10120 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2241 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10126 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2243 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10132 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2247 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10138 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2251 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10144 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2255 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10150 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2259 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10156 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2263 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10162 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2267 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10168 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2271 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10174 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2275 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10180 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2279 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10186 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2283 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10192 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2287 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10198 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2291 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10204 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2295 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10210 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2299 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10216 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2303 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10222 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2308 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10228 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2309 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10234 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2314 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10240 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2315 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10246 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2320 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10252 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2321 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10258 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2326 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10266 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2333 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10274 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2340 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10280 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2342 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10286 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2346 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10292 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2347 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10298 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2348 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10304 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2349 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10310 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2350 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10316 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2351 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10322 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2352 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10328 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2353 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10334 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2354 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10341 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2356 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10347 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10353 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2361 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10359 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2362 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 10365 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2363 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 10371 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2364 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 10377 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2371 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 10383 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10397 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10411 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2396 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 10417 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2397 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 10423 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2402 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10429 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2403 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10435 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2406 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),0,(yyvsp[0]),0);}
#line 10441 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2407 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10447 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2410 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10454 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2414 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10462 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2417 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10468 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10480 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2427 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10486 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2428 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10492 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2432 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10498 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2434 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 10504 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2436 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 10510 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2440 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10516 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2441 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10522 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2442 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10528 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2443 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10534 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2444 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10540 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2445 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10546 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2446 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10552 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2447 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10558 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2448 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10564 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10570 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2450 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10576 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2451 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10582 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2452 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10588 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10594 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10600 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2455 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10606 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2456 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10612 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2457 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10618 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2458 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10624 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10630 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2460 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10636 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10642 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2462 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10648 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2463 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10654 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10660 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2465 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10666 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2466 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10672 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2467 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10678 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2468 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10684 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2469 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10690 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2470 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10696 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2471 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10702 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2472 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10708 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2473 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10714 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2474 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10720 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2475 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10726 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2476 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10732 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2477 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10738 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2478 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10744 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2479 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10750 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2480 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10756 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2481 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10762 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2482 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10768 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10774 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2484 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10780 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10786 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10792 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10798 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10804 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10810 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10816 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2491 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10822 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2492 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10828 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2493 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10834 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2494 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10840 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2495 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10846 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2496 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10852 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2497 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10858 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2498 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10864 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2499 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10870 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2500 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10876 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2501 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10882 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10888 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2503 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10894 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2504 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10900 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2505 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10906 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2506 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10912 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2507 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10918 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2508 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10924 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2509 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10930 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2510 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10936 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2511 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10942 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2512 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10948 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2513 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10954 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2514 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10960 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2515 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10966 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2516 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10972 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10978 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2518 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10984 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2519 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10990 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2520 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10996 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2525 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11002 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2529 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11008 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2530 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11014 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2534 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11020 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2535 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11026 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2536 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11032 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2537 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11039 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2539 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11046 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2543 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11052 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2552 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11058 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2555 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11064 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2556 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11071 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2558 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11078 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2568 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11084 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2572 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11090 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11096 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2574 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11102 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2578 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11108 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2579 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11114 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2580 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11120 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2584 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11126 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2585 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11132 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2586 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11138 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2590 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11144 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2591 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11150 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2595 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11156 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2596 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11162 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2597 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11168 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2598 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11175 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2600 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11181 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2601 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11187 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2602 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 11193 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2603 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 11199 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2604 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 11205 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2605 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 11211 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2606 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 11217 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2607 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 11223 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2608 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 11229 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2611 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11235 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2613 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11241 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2617 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11247 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2618 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11253 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2620 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11259 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2621 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11265 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2623 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11271 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2624 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11277 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2625 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11283 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2626 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11289 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2627 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11295 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2628 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11301 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2629 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11307 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2630 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11313 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2631 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11319 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2633 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 11325 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2635 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 11331 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2637 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 11337 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 11343 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2641 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 11349 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 11355 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2643 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 11361 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2644 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 11367 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2645 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 11373 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 11379 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2647 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 11385 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2648 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 11391 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2649 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 11397 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 11403 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2651 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 11409 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2652 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 11415 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2653 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 11421 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2654 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 11427 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2655 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 11433 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2656 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11439 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2657 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11445 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 11451 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2661 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 11457 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 11463 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2665 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 11469 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2666 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 11475 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 11482 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 11488 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2673 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 11495 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2677 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 11501 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2680 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 11507 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2681 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 11513 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2685 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11519 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11525 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11531 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2698 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11537 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2699 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11543 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2703 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11549 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2704 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11555 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2705 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11561 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2706 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11567 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2707 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 11573 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2708 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 11579 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2710 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 11586 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2715 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11592 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2716 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11598 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11604 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11610 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2724 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 11616 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11622 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2731 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11628 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2733 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11634 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2735 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11640 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11646 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11652 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11658 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2742 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11664 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2745 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11670 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2747 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11676 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2750 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 11682 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2751 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 11688 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2752 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 11694 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2753 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11700 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2757 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 11707 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2760 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 11715 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2767 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11721 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2768 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11727 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2771 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 11735 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2774 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11741 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2775 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11747 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2776 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11753 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2778 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11759 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2779 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11765 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2781 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11771 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2782 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11777 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2783 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11783 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2784 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11789 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2785 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11795 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2786 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11801 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2791 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11807 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2792 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11813 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2797 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11819 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2798 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11825 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2803 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11831 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2805 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11837 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2807 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11843 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2808 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11849 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2812 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11855 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2813 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11861 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2818 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11867 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2819 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 11873 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2824 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11879 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2827 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11885 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11891 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2833 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11897 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2836 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11903 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2837 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 11910 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2844 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 11916 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2846 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 11922 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 11928 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2851 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11934 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2854 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11940 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2857 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11946 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2858 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11952 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2862 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 11958 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2863 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 11964 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 11970 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2868 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 11976 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2869 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 11982 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2873 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11988 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2875 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 11994 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2883 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12000 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2884 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12006 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2888 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12012 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2890 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12018 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2898 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12024 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2899 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12030 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2903 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12036 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 2905 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12042 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 2910 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12048 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 2912 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12054 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12068 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12082 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12096 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12110 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 2968 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12116 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 2969 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12122 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 2970 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12128 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 2971 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12134 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 2972 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12140 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 2973 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12146 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12160 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 2992 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12166 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 2994 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12172 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 2996 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12178 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 2997 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12184 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3001 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12190 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3002 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12196 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3003 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12202 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12208 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12222 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3021 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12228 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3023 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12234 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3024 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12240 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3033 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12246 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3034 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12252 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3035 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12258 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3036 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12264 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3037 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12270 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3038 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12276 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3039 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12282 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3041 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12288 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3043 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12294 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3047 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12300 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3051 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12306 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3052 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12312 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3058 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12318 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3062 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12324 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3069 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 12330 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3078 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 12336 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3082 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 12342 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3086 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12348 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3089 "hphp.y" /* yacc.c:1646  */
    { _p->onIndirectRef((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 12354 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3095 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12360 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3096 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12366 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3097 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12372 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3101 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12378 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3102 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 12384 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3103 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 12390 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3110 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12396 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3111 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12402 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3116 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 12408 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3117 "hphp.y" /* yacc.c:1646  */
    { (yyval)++;}
#line 12414 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3122 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12420 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3123 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12426 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3124 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12432 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
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
#line 12446 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3138 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12452 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3139 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12458 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3143 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12464 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3144 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12470 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
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
#line 12484 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3156 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12490 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3160 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 12496 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3161 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 12502 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3163 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 12508 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3164 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 12514 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3165 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 12520 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3166 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 12526 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3171 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12532 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3172 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12538 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3176 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12544 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3177 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12550 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3178 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12556 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3179 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12562 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3182 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12568 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3184 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 12574 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3185 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12580 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3186 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 12586 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3191 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12592 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3192 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12598 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3196 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12604 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3197 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12610 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3198 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12616 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3199 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12622 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3204 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12628 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3205 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12634 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3210 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12640 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3212 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12646 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3214 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12652 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3215 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12658 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3219 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 12664 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3221 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 12670 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3222 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 12676 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3224 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 12683 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3229 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12689 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3231 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12695 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
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
#line 12709 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3243 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 12715 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3245 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 12721 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3246 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12727 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3249 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 12733 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3250 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 12739 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3251 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 12745 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3255 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 12751 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3256 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 12757 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3257 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12763 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3258 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12769 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3259 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12775 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3260 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12781 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3261 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 12787 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3262 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 12793 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3263 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 12799 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3264 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 12805 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3265 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 12811 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3269 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12817 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3270 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12823 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3275 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12829 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3277 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12835 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3291 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12843 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3296 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 12851 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3300 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12859 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3305 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 12867 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3311 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12873 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3312 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12879 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3316 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12885 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3317 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12891 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3323 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12897 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3327 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 12903 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3333 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12909 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3337 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 12916 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3344 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12922 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3345 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12928 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3349 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 12936 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3352 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 12943 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3358 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12949 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3363 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]); }
#line 12955 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3364 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12961 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3365 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12967 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3366 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12973 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1034:
#line 3387 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12979 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1035:
#line 3388 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 12985 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1038:
#line 3397 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12991 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3408 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 12997 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3410 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13003 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3414 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13009 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3417 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13015 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3421 "hphp.y" /* yacc.c:1646  */
    {}
#line 13021 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3422 "hphp.y" /* yacc.c:1646  */
    {}
#line 13027 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3423 "hphp.y" /* yacc.c:1646  */
    {}
#line 13033 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3429 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13040 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3434 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13050 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3443 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13056 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3449 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13065 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3457 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13071 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3458 "hphp.y" /* yacc.c:1646  */
    { }
#line 13077 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3464 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13083 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3466 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13089 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3467 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13099 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3472 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13106 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3478 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("array"); }
#line 13113 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13119 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3488 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13127 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3492 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13133 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3497 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 13139 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3499 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 13145 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3505 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 13152 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3507 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 13160 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3510 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13166 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3511 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13174 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3514 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13182 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13188 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3520 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 13196 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3523 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13203 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3525 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 13212 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3531 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 13221 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3537 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 13231 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3545 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13237 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3546 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13243 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;


#line 13247 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}
