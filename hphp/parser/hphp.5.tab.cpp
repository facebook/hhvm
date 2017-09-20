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
#define YYLAST   18149

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  206
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  302
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1079
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1994

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
    1166,  1163,  1180,  1177,  1194,  1193,  1202,  1200,  1212,  1211,
    1230,  1228,  1247,  1246,  1255,  1253,  1264,  1264,  1271,  1270,
    1282,  1280,  1293,  1294,  1298,  1301,  1304,  1305,  1306,  1309,
    1310,  1313,  1315,  1318,  1319,  1322,  1323,  1326,  1327,  1331,
    1332,  1337,  1338,  1341,  1342,  1343,  1347,  1348,  1352,  1353,
    1357,  1358,  1362,  1363,  1368,  1369,  1375,  1376,  1377,  1378,
    1381,  1384,  1386,  1389,  1390,  1394,  1396,  1399,  1402,  1405,
    1406,  1409,  1410,  1414,  1420,  1426,  1433,  1435,  1440,  1445,
    1451,  1455,  1459,  1463,  1468,  1473,  1478,  1483,  1489,  1498,
    1503,  1508,  1514,  1516,  1520,  1524,  1529,  1533,  1536,  1539,
    1543,  1547,  1551,  1555,  1560,  1568,  1570,  1573,  1574,  1575,
    1576,  1578,  1580,  1585,  1586,  1589,  1590,  1591,  1595,  1596,
    1598,  1599,  1603,  1605,  1608,  1612,  1618,  1620,  1623,  1623,
    1627,  1626,  1630,  1632,  1635,  1638,  1636,  1652,  1649,  1663,
    1665,  1667,  1669,  1671,  1673,  1675,  1679,  1680,  1681,  1684,
    1690,  1694,  1700,  1703,  1708,  1710,  1715,  1720,  1724,  1725,
    1729,  1730,  1732,  1734,  1740,  1741,  1743,  1747,  1748,  1753,
    1757,  1758,  1762,  1763,  1767,  1769,  1775,  1780,  1781,  1783,
    1787,  1788,  1789,  1790,  1794,  1795,  1796,  1797,  1798,  1799,
    1801,  1806,  1809,  1810,  1814,  1815,  1819,  1820,  1823,  1824,
    1827,  1828,  1831,  1832,  1836,  1837,  1838,  1839,  1840,  1841,
    1842,  1846,  1847,  1850,  1851,  1852,  1855,  1857,  1859,  1860,
    1863,  1865,  1869,  1871,  1875,  1879,  1883,  1888,  1889,  1891,
    1892,  1893,  1894,  1897,  1901,  1902,  1906,  1907,  1911,  1912,
    1913,  1914,  1918,  1922,  1927,  1931,  1935,  1939,  1943,  1948,
    1949,  1950,  1951,  1952,  1956,  1958,  1959,  1960,  1963,  1964,
    1965,  1966,  1967,  1968,  1969,  1970,  1971,  1972,  1973,  1974,
    1975,  1976,  1977,  1978,  1979,  1980,  1981,  1982,  1983,  1984,
    1985,  1986,  1987,  1988,  1989,  1990,  1991,  1992,  1993,  1994,
    1995,  1996,  1997,  1998,  1999,  2000,  2001,  2002,  2003,  2004,
    2005,  2006,  2008,  2009,  2011,  2012,  2014,  2015,  2016,  2017,
    2018,  2019,  2020,  2021,  2022,  2023,  2024,  2025,  2026,  2027,
    2028,  2029,  2030,  2031,  2032,  2033,  2034,  2035,  2036,  2037,
    2038,  2042,  2046,  2051,  2050,  2065,  2063,  2081,  2080,  2099,
    2098,  2117,  2116,  2134,  2134,  2149,  2149,  2167,  2168,  2169,
    2174,  2176,  2180,  2184,  2190,  2194,  2200,  2202,  2206,  2208,
    2212,  2216,  2217,  2221,  2223,  2227,  2229,  2233,  2235,  2239,
    2242,  2247,  2249,  2253,  2256,  2261,  2265,  2269,  2273,  2277,
    2281,  2285,  2289,  2293,  2297,  2301,  2305,  2309,  2313,  2317,
    2321,  2323,  2327,  2329,  2333,  2335,  2339,  2346,  2353,  2355,
    2360,  2361,  2362,  2363,  2364,  2365,  2366,  2367,  2368,  2370,
    2371,  2375,  2376,  2377,  2378,  2382,  2388,  2397,  2410,  2411,
    2414,  2417,  2420,  2421,  2424,  2428,  2431,  2434,  2441,  2442,
    2446,  2447,  2449,  2454,  2455,  2456,  2457,  2458,  2459,  2460,
    2461,  2462,  2463,  2464,  2465,  2466,  2467,  2468,  2469,  2470,
    2471,  2472,  2473,  2474,  2475,  2476,  2477,  2478,  2479,  2480,
    2481,  2482,  2483,  2484,  2485,  2486,  2487,  2488,  2489,  2490,
    2491,  2492,  2493,  2494,  2495,  2496,  2497,  2498,  2499,  2500,
    2501,  2502,  2503,  2504,  2505,  2506,  2507,  2508,  2509,  2510,
    2511,  2512,  2513,  2514,  2515,  2516,  2517,  2518,  2519,  2520,
    2521,  2522,  2523,  2524,  2525,  2526,  2527,  2528,  2529,  2530,
    2531,  2532,  2533,  2534,  2538,  2543,  2544,  2548,  2549,  2550,
    2551,  2553,  2557,  2558,  2569,  2570,  2572,  2574,  2586,  2587,
    2588,  2592,  2593,  2594,  2598,  2599,  2600,  2603,  2605,  2609,
    2610,  2611,  2612,  2614,  2615,  2616,  2617,  2618,  2619,  2620,
    2621,  2622,  2623,  2626,  2631,  2632,  2633,  2635,  2636,  2638,
    2639,  2640,  2641,  2642,  2643,  2644,  2645,  2646,  2648,  2650,
    2652,  2654,  2656,  2657,  2658,  2659,  2660,  2661,  2662,  2663,
    2664,  2665,  2666,  2667,  2668,  2669,  2670,  2671,  2672,  2674,
    2676,  2678,  2680,  2681,  2684,  2685,  2689,  2693,  2695,  2699,
    2700,  2704,  2710,  2713,  2717,  2718,  2719,  2720,  2721,  2722,
    2723,  2728,  2730,  2734,  2735,  2738,  2739,  2743,  2746,  2748,
    2750,  2754,  2755,  2756,  2757,  2760,  2764,  2765,  2766,  2767,
    2771,  2773,  2780,  2781,  2782,  2783,  2788,  2789,  2790,  2791,
    2793,  2794,  2796,  2797,  2798,  2799,  2800,  2804,  2806,  2810,
    2812,  2815,  2818,  2820,  2822,  2825,  2827,  2831,  2833,  2836,
    2839,  2845,  2847,  2850,  2851,  2856,  2859,  2863,  2863,  2868,
    2871,  2872,  2876,  2877,  2881,  2882,  2883,  2887,  2889,  2897,
    2898,  2902,  2904,  2912,  2913,  2917,  2918,  2923,  2925,  2930,
    2941,  2955,  2967,  2982,  2983,  2984,  2985,  2986,  2987,  2988,
    2998,  3007,  3009,  3011,  3015,  3016,  3017,  3018,  3019,  3035,
    3036,  3038,  3047,  3048,  3049,  3050,  3051,  3052,  3053,  3054,
    3056,  3061,  3065,  3066,  3070,  3073,  3080,  3084,  3093,  3100,
    3102,  3108,  3110,  3111,  3115,  3116,  3117,  3124,  3125,  3130,
    3131,  3136,  3137,  3138,  3139,  3150,  3153,  3156,  3157,  3158,
    3159,  3170,  3174,  3175,  3176,  3178,  3179,  3180,  3184,  3186,
    3189,  3191,  3192,  3193,  3194,  3197,  3199,  3200,  3204,  3206,
    3209,  3211,  3212,  3213,  3217,  3219,  3222,  3225,  3227,  3229,
    3233,  3234,  3236,  3237,  3243,  3244,  3246,  3256,  3258,  3260,
    3263,  3264,  3265,  3269,  3270,  3271,  3272,  3273,  3274,  3275,
    3276,  3277,  3278,  3279,  3283,  3284,  3288,  3290,  3298,  3300,
    3304,  3308,  3313,  3317,  3325,  3326,  3330,  3331,  3337,  3338,
    3347,  3348,  3356,  3359,  3363,  3366,  3371,  3376,  3378,  3379,
    3380,  3383,  3385,  3391,  3392,  3396,  3397,  3401,  3402,  3406,
    3407,  3410,  3415,  3416,  3420,  3423,  3425,  3429,  3435,  3436,
    3437,  3441,  3445,  3453,  3458,  3470,  3472,  3476,  3479,  3481,
    3486,  3491,  3497,  3500,  3505,  3510,  3512,  3519,  3521,  3524,
    3525,  3528,  3531,  3532,  3537,  3539,  3543,  3549,  3559,  3560
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

#define YYPACT_NINF -1685

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1685)))

#define YYTABLE_NINF -1063

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1063)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1685,   185, -1685, -1685,  5309, 13226, 13226,   -36, 13226, 13226,
   13226, 13226, 10993, 13226, -1685, 13226, 13226, 13226, 13226, 16818,
   16818, 13226, 13226, 13226, 13226, 13226, 13226, 13226, 13226, 11196,
   17428, 13226,   -25,    -7, -1685, -1685, -1685,   156, -1685,   190,
   -1685, -1685, -1685,   184, 13226, -1685,    -7,   108,   182,   196,
   -1685,    -7, 11399,  3614, 11602, -1685, 14256,  9978,   200, 13226,
    4187,    44,    53,    89,   306, -1685, -1685, -1685,   247,   322,
     336,   343, -1685,  3614,   346,   353,   287,   485,   501,   567,
     573, -1685, -1685, -1685, -1685, -1685, 13226,    83,  2121, -1685,
   -1685,  3614, -1685, -1685, -1685, -1685,  3614, -1685,  3614, -1685,
     481,   461,  3614,  3614, -1685,   327, -1685, -1685, 11805, -1685,
   -1685,   466,   587,   589,   589, -1685,   655,   528,   387,   523,
   -1685,    96, -1685,   668, -1685, -1685, -1685, -1685,  3541,   529,
   -1685, -1685,   540,   569,   574,   600,   602,   608,   617,   619,
   15544, -1685, -1685, -1685, -1685,    87,   658,   719,   761,   773,
     782, -1685,   789,   792, -1685,    47,   666, -1685,   707,    16,
   -1685,  1617,   147, -1685, -1685,  2861,    97,   696,   148, -1685,
     160,    54,   699,    73, -1685,   351, -1685,   802, -1685,   736,
   -1685, -1685,   702,   715, -1685, 13226, -1685,   668,   529, 17853,
    2878, 17853, 13226, 17853, 17853, 14786, 14786,   701, 16378, 17853,
     853,  3614,   835,   835,   572,   835, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685, -1685,    78, 13226,   724, -1685, -1685,
     746,   711,   499,   712,   499,   835,   835,   835,   835,   835,
     835,   835,   835, 16818, 16987,   706,   901,   736, -1685, 13226,
     724, -1685,   751, -1685,   752,   718, -1685,   161, -1685, -1685,
   -1685,   499,    97, -1685, 12008, -1685, -1685, 13226,  8760,   909,
     102, 17853,  9775, -1685, 13226, 13226,  3614, -1685, -1685, 15592,
     721, -1685, 15662, -1685, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, 15961, -1685, 15961, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685,    82,    99,   715, -1685, -1685, -1685, -1685,
     725,  3887,   100, -1685, -1685,   762,   911, -1685,   766, 14975,
   -1685,   729,   730, 15710, -1685,    25, 15758, 13798, 13798, 13798,
    3614, 13798,   731,   920,   735, -1685,   384, -1685, 16406,   103,
   -1685,   921,   107,   807, -1685,   809, -1685, 16818, 13226, 13226,
     747,   765, -1685, -1685, 16509, 11196, 13226, 13226, 13226, 13226,
   13226,   109,    86,   402, -1685, 13429, 16818,   510, -1685,  3614,
   -1685,   -17,   528, -1685, -1685, -1685, -1685, 17528,   929,   854,
   -1685, -1685, -1685,    71, 13226,   760,   764, 17853,   768,  2642,
     771,  5512, 13226,   537,   756,   591,   537,   451,   331, -1685,
    3614, 15961,   774, 10181, 14256, -1685, -1685,  3240, -1685, -1685,
   -1685, -1685, -1685,   668, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, 13226, 13226, 13226, 13226, 12211, 13226, 13226,
   13226, 13226, 13226, 13226, 13226, 13226, 13226, 13226, 13226, 13226,
   13226, 13226, 13226, 13226, 13226, 13226, 13226, 13226, 13226, 13226,
   13226, 17628, 13226, -1685, 13226, 13226, 13226, 13600,  3614,  3614,
    3614,  3614,  3614,  3541,   861,   580,  4650, 13226, 13226, 13226,
   13226, 13226, 13226, 13226, 13226, 13226, 13226, 13226, 13226, -1685,
   -1685, -1685, -1685,  2076, 13226, 13226, -1685, 10181, 10181, 13226,
   13226,   466,   162, 16509,   777,   668, 12414, 15828, -1685, 13226,
   -1685,   778,   966,   819,   780,   781, 13752,   499, 12617, -1685,
   12820, -1685,   718,   783,   785,  2735, -1685,   357, 10181, -1685,
    3037, -1685, -1685, 15876, -1685, -1685, 10384, -1685, 13226, -1685,
     885,  8963,   976,   790, 13413,   977,   132,    66, -1685, -1685,
   -1685,   812, -1685, -1685, -1685, 15961, -1685,  3156,   799,   990,
   16303,  3614, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685,   806, -1685, -1685,   808,   811,   813,   815,   816,   818,
     435,   821,   825, 17639, 14151, -1685, -1685,  3614,  3614, 13226,
     499,    44, -1685, 16303,   922, -1685, -1685, -1685,   499,   133,
     135,   814,   817,  2829,   201,   827,   828,   355,   874,   822,
     499,   136,   831, 17035,   824,  1008,  1016,   829,   833,   836,
     838, -1685, 13975,  3614, -1685, -1685,   961,  3477,    61, -1685,
   -1685, -1685,   528, -1685, -1685, -1685,  1001,   902,   862,   266,
     883, 13226,   466,   898,  1037,   850, -1685,   889, -1685,   162,
   -1685, 15961, 15961,  1036,   909,    71, -1685,   858,  1043, -1685,
   15961,    59, -1685,   484,   152, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685,  1640,  4208, -1685, -1685, -1685, -1685,  1044,   875,
   -1685, 16818, 13226,   864,  1049, 17853,  1048, -1685, -1685,   927,
    3306, 11587, 18036, 14786, 14433, 13226, 17805, 14610,  4265, 12392,
   12797, 12187, 14081, 14257, 14257, 14257, 14257,  2404,  2404,  2404,
    2404,  2404,   794,   794,   820,   820,   820,   572,   572,   572,
   -1685,   835, 17853,   860,   863, 17091,   867,  1059,    -4, 13226,
     205,   724,   236,   162, -1685, -1685, -1685,  1055,   854, -1685,
     668, 16612, -1685, -1685, -1685, 14786, 14786, 14786, 14786, 14786,
   14786, 14786, 14786, 14786, 14786, 14786, 14786, 14786, -1685, 13226,
     222,   166, -1685, -1685,   724,   385,   868,  4376,   873,   876,
     869,  4451,   139,   878, -1685, 17853,  3491, -1685,  3614, -1685,
      59,    29, 16818, 17853, 16818, 17139,   927,    59,   499,   167,
     930,   884, 13226, -1685,   168, -1685, -1685, -1685,  8557,   502,
   -1685, -1685, 17853, 17853,    -7, -1685, -1685, -1685, 13226,   985,
   16179, 16303,  3614,  9166,   891,   897, -1685,  1088,  4471,   965,
   -1685,   945, -1685,  1097,   910,  3736, 15961, 16303, 16303, 16303,
   16303, 16303,   913,  1041,  1046,  1050,  1051,  1052,   918, 16303,
      -2, -1685, -1685, -1685, -1685, -1685, -1685,     5, -1685, 17947,
   -1685, -1685,    -9, -1685,  5715, 13646,   916, 14151, -1685, 14151,
   -1685, 14151, -1685,  3614,  3614, 14151, -1685, 14151, 14151,  3614,
   -1685,  1109,   928, -1685,   443, -1685, -1685,  4548, -1685, 17947,
    1115, 16818,   932, -1685, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685,   950,  1124,  3614, 13646,   937, 16509, 16715,  1126,
   -1685, 13226, -1685, 13226, -1685, 13226, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685,   935, -1685, 13226, -1685, -1685,  4903,
   -1685, 15961, 13646,   942, -1685, -1685, -1685, -1685,  1128,   947,
   13226, 17528, -1685, -1685, 13600,   948, -1685, 15961, -1685,   952,
    5918,  1116,    85, -1685, -1685,   116,  2076, -1685,  3037, -1685,
   15961, -1685, -1685,   499, 17853, -1685, 10587, -1685, 16303,    58,
     951, 13646,   902, -1685, -1685, 14610, 13226, -1685, -1685, 13226,
   -1685, 13226, -1685, 10977,   953, 10181,   874,  1119,   902, 15961,
    1138,   927,  3614, 17628,   499, 11383,   958, -1685, -1685,   159,
     960, -1685, -1685,  1145,  3234,  3234,  3491, -1685, -1685, -1685,
    1108,   967,  1092,  1094,  1095,  1096,  1098,   285,   971,   558,
   -1685, -1685, -1685, -1685, -1685,  1011, -1685, -1685, -1685, -1685,
    1166,   981,   778,   499,   499, 13023,   902,  3037, -1685, -1685,
   11992,   521,    -7,  9775, -1685,  6121,   984,  6324,   995, 16179,
   16818,   983,  1047,   499, 17947,  1183, -1685, -1685, -1685, -1685,
     676, -1685,   282, 15961,  1018,  1062,  1042, 15961,  3614,  3838,
   -1685, -1685, -1685,  1198, -1685,  1012,  1044,   757,   757,  1141,
    1141,  4822,  1010,  1202, 16303, 16303, 16303, 16303, 16303, 16303,
   17528, 15924, 15127, 16303, 16303, 16303, 16303, 16060, 16303, 16303,
   16303, 16303, 16303, 16303, 16303, 16303, 16303, 16303, 16303, 16303,
   16303, 16303, 16303, 16303, 16303, 16303, 16303, 16303, 16303, 16303,
   16303,  3614, -1685, -1685,  1136, -1685, -1685,  1020,  1021,  1025,
   -1685,  1027, -1685, -1685,   468, 17639, -1685,  1034, -1685, 16303,
     499, -1685, -1685,   155, -1685,   632,  1225, -1685, -1685,   141,
    1039,   499, 10790, 17853, 17195, -1685,  2571, -1685,  5106,   854,
    1225, -1685,   423,     3, -1685, 17853,  1100,  1054, -1685,  1056,
    1116, -1685, 15961,   909, 15961,    42,  1223,  1156,   177, -1685,
     724,   178, -1685, -1685, 16818, 13226, 17853, 17947,  1058,    58,
   -1685,  1057,    58,  1063, 14610, 17853, 17243,  1064, 10181,  1066,
    1065, 15961,  1068,  1067, 15961,   902, -1685,   718,   530, 10181,
   13226, -1685, -1685, -1685, -1685, -1685, -1685,  1111,  1071,  1239,
    1170,  3491,  3491,  3491,  3491,  3491,  3491,  1112, -1685, 17528,
      93,  3491, -1685, -1685, -1685, 16818, 17853,  1077, -1685,    -7,
    1238,  1200,  9775, -1685, -1685, -1685,  1081, 13226,  1047,   499,
   16509, 16179,  1083, 16303,  6527,   680,  1087, 13226,    68,   288,
   -1685,  1099, -1685, 15961,  3614, -1685,  1150, -1685, -1685, 15913,
    1260,  1101, 16303, -1685, 16303, -1685,  1102,  1091,  1286, 16142,
    1093, 17947,  1292,  1103,  1110,  1113,  1167,  1296,  1114, -1685,
   -1685, -1685, 17298,  1117,  1305, 17992, 18080, 10161, 16303, 17901,
   11175, 12595, 13000, 13597, 15957, 16260, 16260, 16260, 16260,  2455,
    2455,  2455,  2455,  2455,   832,   832,   757,   757,   757,  1141,
    1141,  1141,  1141, -1685,  1118, -1685,  1120,  1125,  1127,  1130,
   -1685, -1685, 17947,  3614, 15961, 15961, -1685,   632, 13646,   830,
   -1685, 16509, -1685, -1685, 14786, 13226,  1137, -1685,  1139,  1121,
   -1685,    92, 13226, -1685, -1685, -1685, 13226, -1685, 13226, -1685,
     909, -1685, -1685,   264,  1322,  1236, 13226, -1685,  1144,   499,
   17853,  1116,  1147, -1685,  1148,    58, 13226, 10181,  1149, -1685,
   -1685,   854, -1685, -1685,  1143,  1152,  1153, -1685,  1151,  3491,
   -1685,  3491, -1685, -1685,  1155,  1161,  1344,  1219,  1163, -1685,
    1347,  1169,  1172,  1173, -1685,  1233,  1176,  1370, -1685, -1685,
     499, -1685,  1352, -1685,  1188, -1685, -1685,  1191,  1192,   143,
   -1685, -1685, 17947,  1193,  1195, -1685, 15496, -1685, -1685, -1685,
   -1685, -1685, -1685,  1254, 15961, -1685, 15961, -1685, 17947, 17346,
   -1685, -1685, 16303, -1685, 16303, -1685, 16303, -1685, -1685, -1685,
   -1685, 16303, 17528, -1685, -1685, 16303, -1685, 16303, -1685, 10567,
   16303,  1196,  6730, -1685, -1685, -1685, -1685,   632, -1685, -1685,
   -1685, -1685,   609, 14432, 13646,  1282, -1685,  1699,  1228,   662,
   -1685, -1685, -1685,   861,  3607,   112,   113,  1211,   854,   580,
     144, 17853, -1685, -1685, -1685,  1235, 13210, 15448, 17853, -1685,
     242,  1384,  1325, 13226, -1685, 17853, 10181,  1294,  1116,  1210,
    1116,  1217, 17853,  1218, -1685,  1348,  1221,  1442, -1685, -1685,
      58, -1685, -1685,  1280, -1685, -1685,  3491, -1685,  3491, -1685,
    3491, -1685, -1685, -1685, -1685,  3491, -1685, 17528, -1685,  1767,
   -1685,  8557, -1685, -1685, -1685, -1685,  9369, -1685, -1685, -1685,
    8557, 15961, -1685,  1220, 16303, 17401, 17947, 17947, 17947,  1285,
   17947, 17449, 10567, -1685, -1685,   632, 13646, 13646,  3614, -1685,
    1404, 15279,    90, -1685, 14432,   854, 14869, -1685,  1244, -1685,
     114,  1227,   117, -1685, 14785, -1685, -1685, -1685,   123, -1685,
   -1685,  3054, -1685,  1229, -1685,  1345,   668, -1685, 14609, -1685,
   14609, -1685, -1685,  1416,   861, -1685, 13904, -1685, -1685, -1685,
   -1685,  1417,  1349, 13226, -1685, 17853,  1237,  1240,  1116,  1242,
   -1685,  1294,  1116, -1685, -1685, -1685, -1685,  2013,  1243,  3491,
    1303, -1685, -1685, -1685,  1306, -1685,  8557,  9572,  9369, -1685,
   -1685, -1685,  8557, -1685, -1685, 17947, 16303, 16303, 16303,  6933,
    1246,  1248, -1685, 16303, -1685, 13646, -1685, -1685, -1685, -1685,
   -1685, 15961,  2923,  1699, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685, -1685, -1685,   661, -1685,  1228, -1685,
   -1685, -1685, -1685, -1685,   126,   623, -1685,  1435,   128, 14975,
    1345,  1436, -1685, 15961,   668, -1685, -1685,  1255,  1440, 13226,
   -1685, 17853, -1685,   145,  1256, 15961,   610,  1116,  1242, 14080,
   -1685,  1116, -1685,  3491,  3491, -1685, -1685, -1685, -1685,  7136,
   17947, 17947, 17947, -1685, -1685, -1685, 17947, -1685,  2640,  1447,
    1449,  1259, -1685, -1685, 16303, 14785, 14785,  1393, -1685,  3054,
    3054,   683, -1685, -1685, -1685, 16303,  1379, -1685,  1283,  1267,
     129, 16303, -1685, 14975, -1685, 16303, 17853,  1383, -1685,  1465,
   -1685,  1466, -1685,   462, -1685, -1685, -1685,  1277,   610, -1685,
     610, -1685, -1685,  7339,  1279,  1363, -1685,  1377,  1320, -1685,
   -1685,  1380, 15961,  1299,  2923, -1685, -1685, 17947, -1685, -1685,
    1315, -1685,  1452, -1685, -1685, -1685, -1685, 17947,  1479,   355,
   -1685, -1685, 17947,  1298, 17947, -1685,   325,  1300,  7542, 15961,
   -1685, 15961, -1685,  7745, -1685, -1685, -1685,  1301, -1685,  1302,
    1318,  3614,   580,  1321, -1685, -1685, -1685, 16303,  1324,    80,
   -1685,  1419, -1685, -1685, -1685, -1685, -1685, -1685,  7948, -1685,
   13646,   916, -1685,  1329,  3614,   673, -1685, 17947, -1685,  1310,
    1491,   698,    80, -1685, -1685,  1426, -1685, 13646,  1319, -1685,
    1116,   122, -1685, -1685, -1685, -1685, 15961, -1685,  1316,  1323,
     130, -1685,  1242,   698,   271,  1116,  1326, -1685,   656, 15961,
     273,  1505,  1438,  1242, -1685, -1685, -1685, -1685,   304,  1508,
    1441, 13226, -1685,   656,  8151,  8354,   356,  1513,  1446, 13226,
   -1685, 17853, -1685, -1685, -1685,  1516,  1448, 13226, -1685, 17853,
   13226, -1685, 17853, 17853
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
     626,   302,     0,     0,   289,   299,     0,     0,  1037,  1031,
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
     290,   304,   925,     0,     0,     0,     0,  1037,  1031,     0,
     211,  1037,   851,     0,     0,   133,   246,   146,   167,     0,
     569,   554,   976,   190,   342,   343,   421,   240,     0,   814,
     814,     0,   367,   355,     0,     0,     0,   373,   375,     0,
       0,   380,   387,   388,   386,     0,     0,   329,  1018,     0,
       0,     0,   425,     0,   324,     0,   303,     0,   613,   816,
     133,   816,  1033,     0,   394,   133,   199,     0,     0,   207,
       0,   573,   859,     0,     0,   169,   345,   123,     0,   346,
     347,     0,   813,     0,   813,   369,   365,   370,   631,   632,
       0,   356,   389,   390,   382,   383,   381,   419,   416,  1050,
     335,   331,   420,     0,   325,   614,   815,     0,     0,   815,
    1032,     0,  1036,     0,   133,   201,   203,     0,   249,     0,
     194,     0,   401,     0,   361,   368,   372,     0,     0,   871,
     337,     0,   611,   531,   534,  1034,  1035,   395,     0,   247,
       0,     0,   170,   352,     0,   400,   362,   417,  1019,     0,
     816,   412,   871,   612,   536,     0,   193,     0,     0,   351,
    1037,   871,   276,   415,   414,   413,  1079,   411,     0,     0,
       0,   350,  1031,   412,     0,  1037,     0,   349,     0,  1079,
       0,   281,   279,  1031,   133,   396,   133,   336,     0,   282,
       0,     0,   277,     0,     0,     0,     0,   285,   275,     0,
     278,   284,   338,   189,   397,   286,     0,     0,   273,   283,
       0,   274,   288,   287
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1685, -1685, -1685,  -576, -1685, -1685, -1685,   171,  -404,   -41,
     405, -1685,  -264,  -517, -1685, -1685,   391,   215,  1252, -1685,
    2154, -1685,  -475, -1685,    55, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685, -1685, -1685,  -389, -1685, -1685,  -173,
      62,    21, -1685, -1685, -1685, -1685, -1685, -1685,    24, -1685,
   -1685, -1685, -1685, -1685, -1685,    27, -1685, -1685,  1053,  1061,
    1060,   -98,  -714,  -897,   542,   598,  -396,   286,  -975, -1685,
    -105, -1685, -1685, -1685, -1685,  -755,   111, -1685, -1685, -1685,
   -1685,  -387, -1685,  -617, -1685,  -428, -1685, -1685,   940, -1685,
     -83, -1685, -1685, -1099, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685,  -120, -1685,   -29, -1685, -1685, -1685,
   -1685, -1685,  -202, -1685,    69,  -950, -1685, -1165,  -413, -1685,
    -152,   174,  -127,  -382, -1685,  -201, -1685, -1685, -1685,    88,
     -30,     4,   248,  -740,   -76, -1685, -1685,    12, -1685,   -12,
   -1685, -1685,    -5,   -55,   -56, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685,  -645,  -900, -1685, -1685, -1685, -1685,
   -1685,  1888,  1187, -1685,   479, -1685,   345, -1685, -1685, -1685,
   -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685,
   -1685, -1685, -1685,   300,  -384,  -459, -1685, -1685, -1685, -1685,
   -1685,   406, -1685, -1685, -1685, -1685, -1685, -1685, -1685, -1685,
    -925,  -371,  2158,    26, -1685,   392,  -424, -1685, -1685,  -501,
    3086,  3013, -1685,   881, -1685, -1685,   487,    95,  -640, -1685,
   -1685,   570,   352,   127, -1685,   358, -1685, -1685, -1685, -1685,
   -1685,   545, -1685, -1685, -1685,    98,  -922,  -177,  -443,  -440,
   -1685,   618,  -118, -1685, -1685,    35,    41,    23, -1685, -1685,
    1045,   -26, -1685,  -363,   158,   204, -1685,    84, -1685, -1685,
   -1685,  -490,  1203, -1685, -1685, -1685, -1685, -1685,   726,   330,
   -1685, -1685, -1685,  -339,  -682, -1685,  1157, -1079,  -243,   -68,
    -194,    48,   749, -1685, -1684, -1685,  -297, -1105, -1280,  -284,
     119, -1685,   444,   519, -1685, -1685, -1685, -1685,   469, -1685,
    1201, -1122
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   115,   940,   653,   186,   343,   751,
     362,   363,   364,   365,   891,   892,   893,   117,   118,   119,
     120,   121,   421,   687,   688,   561,   262,  1647,   567,  1556,
    1648,  1900,   876,   355,   590,  1855,  1136,  1335,  1922,   437,
     187,   689,   980,  1203,  1396,   125,   656,   997,   690,   709,
    1001,   628,   996,   241,   542,   691,   657,   998,   439,   382,
     404,   128,   982,   943,   916,  1156,  1582,  1262,  1062,  1797,
    1651,   827,  1068,   566,   836,  1070,  1439,   819,  1051,  1054,
    1251,  1929,  1930,   677,   678,   703,   704,   369,   370,   372,
    1616,  1776,  1777,  1349,  1491,  1605,  1770,  1909,  1932,  1808,
    1859,  1860,  1861,  1592,  1593,  1594,  1595,  1810,  1811,  1817,
    1871,  1598,  1599,  1603,  1763,  1764,  1765,  1846,  1967,  1492,
    1493,   188,   130,  1946,  1947,  1768,  1495,  1496,  1497,  1498,
     131,   255,   562,   563,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,  1628,   142,   979,  1202,   143,   674,
     675,   676,   259,   413,   557,   663,   664,  1297,   665,  1298,
     144,   145,   634,   635,  1287,  1288,  1405,  1406,   146,   861,
    1030,   147,   862,  1031,   148,   863,  1032,   149,   864,  1033,
     150,   865,  1034,   637,  1290,  1408,   151,   866,   152,   153,
    1839,   154,   658,  1618,   659,  1172,   948,  1367,  1364,  1756,
    1757,   155,   156,   157,   244,   158,   245,   256,   424,   549,
     159,  1291,  1292,   870,   871,   160,  1092,   971,   605,  1093,
    1037,  1225,  1038,  1409,  1410,  1228,  1229,  1040,  1416,  1417,
    1041,   797,   532,   200,   201,   692,   680,   513,  1188,  1189,
     783,   784,   967,   162,   247,   163,   164,   190,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   743,   175,   251,
     252,   631,   235,   236,   746,   747,  1303,  1304,   397,   398,
     934,   176,   619,   177,   673,   178,   346,  1778,  1829,   383,
     432,   698,   699,  1085,  1786,  1841,  1842,  1183,  1346,   912,
    1347,   913,   914,   842,   843,   844,   347,   348,   873,   576,
    1581,   965
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     189,   191,   444,   193,   194,   195,   196,   198,   199,   494,
     202,   203,   204,   205,   524,   344,   225,   226,   227,   228,
     229,   230,   231,   232,   234,   124,   253,   165,   126,   958,
     405,   127,   416,   792,   408,   409,   666,   258,   963,   261,
    1184,   806,   222,   224,   668,   352,   546,   269,   516,   272,
     263,  1176,   353,   418,   356,   267,   243,   740,   959,   122,
     440,   444,  1373,   493,   977,   248,   123,  1480,   670,   351,
     781,   249,  1359,   782,   550,  1370,  1000,   939,   415,   818,
    1072,   261,   420,   750,  1258,  1201,   890,   895,  1058,   788,
     789,   -78,  1046,   595,   597,   599,   -78,   602,   834,  1665,
    1437,  1212,   161,   417,  1848,   434,    14,   814,   -43,   -42,
     815,   558,   611,   -43,   -42,   874,   614,  -904,   558,    14,
     811,  1608,  1610,  -357,  -906,  1374,  1673,   367,    14,   514,
     418,   419,  1758,    14,   551,  1819,   371,  1826,  1826,  1665,
     591,   832,   901,   519,   558,   918,  1505,   430,   918,  1247,
     918,  1131,   918,   918,  1185,   415,   642,  -600, -1023,   420,
     192,   535,  1820,   511,   512,  1102,   392,   514,   533,  1365,
      14,   254,   431,   393,  1418,   116,  -718,  -107,   129,   431,
     417,   654,   655,  1837,   544,     3,   534,   527,   250,   257,
     420,   443,  -107,   511,   512,   511,   512, -1023,  -543,  1186,
    1296,   960,  1366,   592,  1103,   910,   911,  -726,   419,   543,
     373,   417,  -916,  -719,  -725,   643,  1375,  -907,  -607,   374,
     511,   512,  -911,  1300,   270,  -918,  -610,   342,  1838,  -910,
    -905,  -948,   519,  -720,   417,   394,  -908,  -951,  -950,   419,
     394,   395,   396,  -914,   381,   368,  -904,  -891,  -892,   553,
     525,  1146,   553,  -906,  -608,  -296,   537,   406,   515,   261,
     564,   938,   545,   575,  -296,   835,  1438,   403,  1958,   381,
     710,  -607,   520,   381,   381,   366,  1517,  -280,  -815,  1973,
     -78,   165,  -815,  -915,  1519,   165,  1430,  1215,  1666,  1667,
    1187,  1525,   260,  1527,   435,  1480,   515,   -43,   -42,   381,
     559,   612,  1510,   401,   264,   615,   402,   641,   586,  1960,
    1609,  1611,  -357,   555,  1265,  1674,  1269,   560,  1395,  -815,
     945,  1759,  1549,  1821,   622,  1621,  1827,  1881,  1957,   833,
     902,   541,   903,   919,   391,   495,  1013,  -813,  1350,  1415,
    1555,  1615,  1976,  -913,   518,  1055,  -907,  1511,  -917,   621,
    1057,  -911,   793,   625,  1961,  -920,  1969,   386,  -910,  -905,
    -948,   520,  1198,  1911,  1168,  -908,  -951,  -950,   444,   708,
    1142,  1143,   531,   261,   417,  1237,  -891,  -892,   265,   522,
     234,   633,   261,   261,   633,   261,  -106,  1977,  1267,  1268,
     647,   610,   266,   344,  1267,  1268,  -727,   518,   907,   354,
     618,  -106,   623,  -879,  1580,   910,   911,   630,  1912,   198,
     410,   220,   220,  1629,   392,  1631,  1622,   693,  -879,   648,
     762,   649,   405,   757,   758,   440,   375,  1159,   705,   116,
    1637,   946,   995,   116,   207,    40,   376,   565,  1512,  1985,
     207,    40,  1238,   377,   165,  1962,   947,  1970,   711,   712,
     713,   714,   716,   717,   718,   719,   720,   721,   722,   723,
     724,   725,   726,   727,   728,   729,   730,   731,   732,   733,
     734,   735,   736,   737,   738,   739,  1891,   741,  1978,   742,
     742,   745,  1270,   123,   764,   392,   607,  1358,  1440,   395,
     396,   765,   766,   767,   768,   769,   770,   771,   772,   773,
     774,   775,   776,   777,   966,  1668,   968,   243,   763,   742,
     787,   679,   705,   705,   742,   791,   248,  1427,   378,   411,
     585,   765,   249,  1784,   795,  1191,   412,  1788,  1192,  1771,
    1986,  1772,   379,   803,   392,   805,   696,   883,  1372,   380,
     494,   649,   384,   705,   821,   607,   630, -1023,   608,   385,
     750,   822,   521,   823,  -609,   387,   760,   994,   111,   644,
     395,   396,  1569,  1892,  1344,  1345,  -882,  1209,   431,   744,
     883,   388,   366,   366,   366,   600,   366,  1052,  1053,  1217,
     666,  -882,  1382, -1023,   165,  1384, -1023,  1002,   668,  1360,
    1006,  1264,   116,   392,   493,   129,  1249,  1250,   786,   884,
     649,   697,  1361,   790,   897,   342,   949,   430,   381,   395,
     396,   669,   670,  1137,   652,  1138,   826,  1139,   511,   512,
     392,  1141,  1362,   966,   968,   220,   620,   799,   890,   481,
    1047,   968,   430,   511,   512,   636,   636,   389,   636,   250,
      55,   482,   984,   390,  -721,   752,   406,  1644,   441,   180,
     181,    65,    66,    67,  1822,   695,   417,   407,   585,   381,
     755,   381,   381,   381,   381,   422,   546,   650,   395,   396,
     392,   785,   392,  1823,   392,  1814,  1824,   423,  1132,   426,
    -918,   649,  1847,  1895,   780,  1896,  1850,   429,   638,  1048,
     640,   753,   430,  1815,   752,   395,   396,   974,  1301,   441,
     180,   181,    65,    66,    67,   810,   436,   585,   816,  1526,
     985,  -880,  1816,  1293,  1874,  1295,  1397,   753,  -123,   433,
     442,   813,  -123,   645,   973,  1509,  -880,   651,  -601,   666,
     924,   926,   116,  1875,  1344,  1345,  1876,   668,   445,  -123,
     753,  1576,  1577,   206,   993,   395,   396,   395,   396,   395,
     396,   753,   872,   645,   753,   651,   645,   651,   651,   952,
     220,   670,   594,   596,   598,    50,   601,   446,  1521,   220,
    1429,   442,   447,   679,  1005,  1411,   220,  1413,   896,   697,
    1388,  1266,  1267,  1268,  1004,  1434,  1267,  1268,   220,  -602,
    1600,  1398,   441,   180,   181,    65,    66,    67,   448,   667,
     449,   210,   211,   212,   213,   214,   450,  1050,  1844,  1845,
    1127,  1128,  1129,   933,   935,   451,  1613,   452,   894,   894,
    1943,  1944,  1945,   261,  1954,  1043,  1130,  1044,  1056,    93,
      94,  -603,    95,   184,    97,  1952,  1482,  1968,   425,   427,
     428,   165,   992,  -604,   475,   476,   477,   478,   479,   480,
    1963,   481,  -605,  1063,  1965,  1966,   165,   107,  1601,   484,
    1472,  1500,   485,   482,   442,   486,   495,   487,   666,  1872,
    1873,  1939,  -606,   478,   479,   480,   668,   481,    14,   399,
     123,   381,  1124,  1125,  1126,  1127,  1128,  1129,  1067,   482,
    1868,  1869,   517,  1083,  1086,  -912,  -719,   165,   523,   528,
     670,  1130,   530,  1669,   482,   431,   536,  -916,   518,   539,
     540,  -717,   547,  1638,   548,   220,  1163,   556,  1164,   569,
     823,   577, -1062,  1039,  1150,   580,   581,   587,   588,   604,
     603,  1166,  1074,   606,  1216,   613,   123,   616,  1080,   617,
     630,  1161,  1483,   671,   626,  1175,   627,  1484,  1551,   441,
    1485,   181,    65,    66,    67,  1486,   672,   681,   694,  1523,
     124,   682,   165,   126,  1560,   683,   127,  1029,   685,  1042,
    -128,  1196,    55,   707,   796,   798,   644,   800,   801,   824,
     807,  1204,   808,   165,  1205,   558,  1206,  1931,   828,   116,
     705,   831,   129,   575,   122,   845,  1378,  1487,  1488,   846,
    1489,   123,   875,  1065,   116,   900,   915,   877,  1154,   878,
    1931,   904,   879,   880,   905,   881,   882,   923,   917,  1953,
     885,   442,   123,   886,   908,   925,   909,   922,   920,   243,
    1490,   936,   927,   941,   950,   942,   928,   161,   248,   929,
    1246,   930,   944,  -742,   249,   116,   951,   953,   129,   954,
     957,   961,   962,   970,  1140,   697,  1252,   972,   976,   981,
     679,   975,   978,   987,   223,   223,   988,   990,   991,   999,
    1009,  1007,  1011,  1010,   983,  1646,   165,   679,   165,  1579,
     165,  1049,  1063,  1259,  1652,  1155,  1352,  1059,  1626,  1069,
    -723,   666,   894,   220,   894,  1071,   894,  1073,  1659,   668,
     894,  1077,   894,   894,  1144,  1078,  1079,  1081,  1253,  1094,
     116,  1095,  1177,   129,  1100,  1135,  1096,   123,  1145,   123,
    1097,  1098,  1099,   670,   785,   585,   816,  1482,  1147,  1149,
    1151,   116,  1152,  1153,   129,  1242,  1158,   780,  1165,   813,
    1162,  1171,  1173,  1227,  1174,  1180,  1178,  1199,  1182,  1208,
    1353,  1211,  1214,   220,  1219,   669,  -919,  1354,   753,  1220,
    1230,   250,  1232,  1231,  1233,  1234,  1235,  1239,  1236,    14,
     753,  1240,   753,   381,  1799,  1241,   666,  1664,  1243,  1260,
    1579,  1281,  1255,  1261,   668,  1224,  1224,  1029,  1285,   124,
    1380,   165,   126,  1257,   220,   127,   220,  1263,  1273,  1887,
    1272,  1890,  1274,   705,  1579,   816,  1579,  1279,   670,  1280,
    1130,  1284,  1579,  1283,   705,  1354,  1482,  1379,   813,  1334,
    1336,  1337,   220,   122,   116,  1338,   116,  1339,   116,   129,
     123,   129,  1341,  1483,  1348,  1351,  1368,  1376,  1484,  1377,
     441,  1485,   181,    65,    66,    67,  1486,  1399,  1401,  1276,
     995,   753,   261,  1422,  1369,  1381,  1383,   349,    14,  1385,
    1387,  1020,  1436,  1389,  1390,  1393,   161,  1392,  1420,  1414,
    1423,   217,   217,   585,  1400,   165,  1421,  1424,   223,  1426,
    1431,  1441,   240,   630,  1063,  1435,  1444,   165,  1487,  1488,
    1942,  1489,  1446,   220,  1451,  1452,  1455,   679,  1447,  1450,
     679,  1456,   872,  1461,   669,  1462,  1458,  1425,   240,   220,
     220,  1464,   442,  1459,  1467,  1471,  1460,  1466,  1473,  1514,
    1853,  1504,  1483,  1474,  1402,  1475,   123,  1484,  1476,   441,
    1485,   181,    65,    66,    67,  1486,  1513,  1502,  1503,   116,
    1516,  1528,   129,   667,  1518,  1520,  1524,  1614,  1531,  1530,
    1501,  1529,  1534,  1536,  1482,  1538,  1540,  1506,  1227,  1407,
     894,  1507,  1407,  1508,  1535,  1888,  1539,   444,  1419,  1545,
    1893,  1515,  1542,  1546,   630,  1543,  1544,  1487,  1488,  1547,
    1489,  1522,   705,  1453,  1550,  1579,  1552,  1457,  1553,  1554,
    1561,  1557,  1463,  1558,  1584,  1573,    14,  1597,  1623,  1468,
    1617,   442,  1029,  1029,  1029,  1029,  1029,  1029,  1624,  1612,
    1630,  1627,  1029,   223,  1632,  1633,  1639,  1654,  1663,  1918,
    1635,  1657,   223,   116,   624,  1671,  1672,  1766,  1767,   223,
    1773,  1779,  1780,  1769,  1782,   116,  1783,  1785,   129,  1793,
    1791,   223,  1794,   669,  1804,  1443,  1805,  1499,  1482,  1825,
    1831,   220,   220,  1834,  1835,  1840,  1862,  1499,  1864,  1866,
    1483,  1870,  1878,  1880,  1879,  1484,  1885,   441,  1485,   181,
      65,    66,    67,  1486,  1886,  1889,  1894,  1898,  1899,  -353,
    1901,  1904,  1902,   679,   573,   217,   574,  1906,  1820,  1974,
      14,  1975,   667,  1907,  1910,   165,  1921,  1913,  1920,  1919,
    1941,  1537,  1933,  1926,  1937,  1541,  1928,  1940,  1625,  1949,
    1662,   705,  1548,  1955,  1477,  1487,  1488,  1951,  1489,  1971,
    1956,  1972,  1979,  1494,  1980,  1964,  1532,  1987,  1533,  1988,
    1990,  1991,  1936,  1494,   123,   240,  1340,   240,  1210,   442,
    1170,  1950,   579,  1798,  1428,  1948,   759,  1559,  1634,   754,
     756,   898,  1789,  1813,  1483,  1606,  1818,  1670,  1604,  1484,
    1982,   441,  1485,   181,    65,    66,    67,  1486,   223,  1830,
    1029,  1959,  1029,  1585,   165,  1787,   639,  1294,  1363,   165,
    1412,  1286,  1403,   165,  1190,  1226,   220,  1244,   632,  1404,
    1883,   706,  1915,   240,  1084,  1908,  1575,  1343,  1278,     0,
    1333,  1833,     0,     0,     0,     0,     0,     0,     0,  1487,
    1488,  1650,  1489,   123,     0,     0,     0,  1499,  1781,     0,
     217,     0,   123,  1499,     0,  1499,     0,     0,   679,   217,
       0,   667,   700,   442,     0,   349,   217,   220,     0,     0,
       0,     0,  1636,   116,     0,     0,   129,  1499,   217,     0,
       0,     0,   220,   220,   342,     0,     0,     0,     0,   217,
    1602,     0,     0,  1640,     0,  1641,   669,  1642,     0,   165,
     165,   165,  1643,   495,     0,   165,  1774,  1036,     0,     0,
       0,     0,   165,   240,     0,     0,   240,     0,     0,     0,
       0,     0,     0,  1494,     0,     0,     0,     0,     0,  1494,
       0,  1494,  1796,  1650,     0,     0,     0,  1029,   123,  1029,
       0,  1029,     0,     0,   123,     0,  1029,     0,     0,     0,
       0,   123,   116,  1494,     0,   129,     0,   116,  1828,     0,
    1586,   116,     0,   240,   129,  1499,   441,    63,    64,    65,
      66,    67,     0,   220,     0,     0,   223,    72,   488,   381,
    1924,   669,   585,     0,     0,   342,     0,     0,     0,   441,
      63,    64,    65,    66,    67,  1755,  1792,     0,     0,     0,
      72,   488,  1762,  1482,  1836,   217,   837,     0,     0,   342,
     206,   342,     0,     0,     0,     0,     0,   342,   489,     0,
     490,     0,  1828,     0,     0,     0,     0,     0,   444,     0,
       0,     0,    50,   491,     0,   492,   223,     0,   442,     0,
    1029,  1494,     0,   490,     0,    14,     0,   116,   116,   116,
     129,     0,   165,   116,  1587,     0,   129,   240,     0,   240,
     116,   442,   860,   129,     0,     0,     0,  1588,   210,   211,
     212,   213,   214,  1589,     0,     0,     0,   223,     0,   223,
       0,     0,     0,     0,   667,     0,     0,     0,     0,     0,
     183,   123,     0,    91,  1590,   860,    93,    94,     0,    95,
    1591,    97,   955,   956,     0,   223,   165,     0,     0,  1483,
       0,   964,     0,     0,  1484,     0,   441,  1485,   181,    65,
      66,    67,  1486,     0,   107,     0,     0,  1036,     0,     0,
       0,     0,     0,     0,  1863,  1865,     0,   218,   218,     0,
       0,   165,     0,     0,     0,   123,   165,     0,     0,     0,
    1851,  1852,     0,   240,   240,     0,     0,     0,     0,     0,
       0,     0,   240,     0,  1487,  1488,     0,  1489,     0,   667,
     585,   165,     0,     0,     0,     0,   223,     0,     0,     0,
     123,     0,     0,   217,     0,   123,     0,     0,   442,     0,
     342,     0,   223,   223,  1029,  1029,  1981,  1645,     0,     0,
     116,     0,     0,   129,  1989,     0,     0,     0,     0,  1857,
     123,     0,  1992,     0,     0,  1993,  1755,  1755,     0,     0,
    1762,  1762,     0,     0,     0,     0,     0,   165,   165,     0,
       0,     0,     0,     0,   585,     0,     0,   679,     0,     0,
       0,     0,     0,   217,     0,     0,     0,     0,     0,  1482,
       0,     0,     0,     0,   116,     0,     0,   129,     0,     0,
     679,     0,     0,     0,     0,     0,   123,   123,     0,   679,
       0,     0,     0,     0,     0,     0,   700,   700,   240,     0,
       0,     0,     0,     0,   217,     0,   217,     0,     0,   116,
       0,    14,   129,     0,   116,     0,     0,   129,     0,     0,
       0,     0,  1923,     0,     0,     0,  1925,     0,     0,     0,
       0,     0,   217,   860,     0,     0,     0,     0,     0,   116,
     240,     0,   129,     0,     0,  1938,     0,   240,   240,   860,
     860,   860,   860,   860,   223,   223,     0,     0,     0,     0,
       0,   860,  1036,  1036,  1036,  1036,  1036,  1036,     0,     0,
       0,   218,  1036,     0,     0,  1483,     0,   240,     0,     0,
    1484,     0,   441,  1485,   181,    65,    66,    67,  1486,     0,
       0,     0,  1169,     0,     0,   116,   116,     0,   129,   129,
       0,     0,     0,   217,     0,     0,     0,   206,  1179,   207,
      40,     0,     0,     0,     0,     0,     0,   240,     0,   217,
     217,  1193,     0,     0,     0,     0,     0,   219,   219,    50,
    1487,  1488,     0,  1489,     0,     0,     0,     0,   242,     0,
       0,     0,     0,   240,   240,     0,     0,     0,     0,     0,
    1213,     0,   206,   217,   442,     0,     0,     0,     0,   240,
     345,     0,     0,  1790,     0,   210,   211,   212,   213,   214,
       0,     0,   240,     0,    50,     0,     0,     0,     0,     0,
     860,     0,     0,   240,     0,     0,     0,     0,     0,   223,
       0,   778,     0,    93,    94,     0,    95,   184,    97,     0,
       0,   240,     0,     0,     0,   240,   218,     0,     0,     0,
     210,   211,   212,   213,   214,   218,     0,     0,   240,     0,
       0,   107,   218,     0,  1271,   779,     0,   111,  1275,     0,
    1036,     0,  1036,     0,   218,   399,     0,     0,    93,    94,
     223,    95,   184,    97,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   223,   223,     0,     0,     0,
       0,   217,   217,     0,     0,     0,   107,     0,     0,     0,
     400,     0,     0,     0,     0,   240,     0,     0,     0,   240,
       0,   240,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   860,   860,   860,   860,
     860,   860,   217,     0,     0,   860,   860,   860,   860,   860,
     860,   860,   860,   860,   860,   860,   860,   860,   860,   860,
     860,   860,   860,   860,   860,   860,   860,   860,   860,   860,
     860,   860,   860,  1371,     0,   964,     0,     0,     0,     0,
       0,   219,     0,     0,     0,     0,   223,     0,     0,     0,
       0,   860,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   218,  1391,     0,     0,  1394,     0,  1036,     0,  1036,
       0,  1036,     0,     0,     0,     0,  1036,     0,     0,     0,
       0,     0,     0,     0,   240,     0,   240,   345,     0,   345,
       0,     0,     0,     0,     0,     0,   217, -1063, -1063, -1063,
   -1063, -1063,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,     0,   240,     0,     0,   240,     0,     0,     0,
       0,     0,     0,   482,  1442,     0,     0,     0,     0,     0,
    1193,     0,     0,   240,   240,   240,   240,   240,   240,     0,
       0,   217,     0,   240,     0,   345,     0,   217, -1063, -1063,
   -1063, -1063, -1063,  1122,  1123,  1124,  1125,  1126,  1127,  1128,
    1129,     0,   217,   217,     0,   860,     0,     0,     0,     0,
    1036,     0,     0,     0,  1130,   240,   219,     0,     0,     0,
       0,   240,     0,     0,   860,   219,   860,     0,     0,     0,
       0,     0,   219,     0,     0,  1478,  1479,     0,     0,     0,
       0,     0,     0,     0,   219,     0,     0,     0,     0,     0,
     860,     0,     0,     0,     0,   219,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   453,   454,   455,     0,   345,     0,     0,   345,   218,
       0,     0,     0,     0,     0,     0,   240,   240,     0,     0,
     240,   456,   457,   217,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   242,
     482,     0,     0,     0,     0,  1562,     0,  1563,     0,   218,
       0,   240,     0,   240,     0,     0,   526,   497,   498,   499,
     500,   501,   502,   503,   504,   505,   506,   507,   508,     0,
       0,     0,     0,     0,  1036,  1036,     0,     0,     0,     0,
       0,   219,     0,     0,     0,     0,     0,     0,     0,     0,
     218,     0,   218,     0,     0,  1607,   240,     0,   240,     0,
       0,   509,   510,     0,   860,     0,   860,     0,   860,     0,
       0,     0,     0,   860,   217,     0,     0,   860,   218,   860,
       0,   206,   860,     0,     0,     0,     0,     0,     0,   345,
       0,   841,     0,     0,     0,   240,   240,     0,   867,   240,
       0,     0,     0,    50,     0,     0,   240,     0,     0,   526,
     497,   498,   499,   500,   501,   502,   503,   504,   505,   506,
     507,   508,  1653,     0,     0,     0,     0,     0,     0,     0,
       0,   867,     0,     0,  1356,     0,   511,   512,     0,   210,
     211,   212,   213,   214,     0,     0,     0,     0,   240,   218,
     240,     0,   240,     0,   509,   510,     0,   240,     0,   217,
       0,   183,     0,     0,    91,   218,   218,    93,    94,     0,
      95,   184,    97,   240,     0,     0,   860,     0,     0,     0,
       0,     0,     0,     0,     0,   345,   345,     0,   240,   240,
       0,     0,     0,     0,   345,   107,   240,     0,   240,   684,
    1856,     0,     0,   526,   497,   498,   499,   500,   501,   502,
     503,   504,   505,   506,   507,   508,     0,     0,     0,   219,
     240,     0,   240,     0,     0,     0,     0,     0,   240,   511,
     512,     0,  1809,     0,     0,   496,   497,   498,   499,   500,
     501,   502,   503,   504,   505,   506,   507,   508,   509,   510,
       0,   240,   526,   497,   498,   499,   500,   501,   502,   503,
     504,   505,   506,   507,   508,     0,     0,     0,   860,   860,
     860,     0,     0,     0,     0,   860,     0,   240,     0,   219,
     509,   510,     0,   240,     0,   240,     0,     0,     0,     0,
       0,     0,   809,     0,     0,     0,     0,   509,   510,     0,
       0,     0,     0,     0,     0,     0,     0,   218,   218,     0,
       0,     0,     0,     0,  1035,     0,     0,     0,     0,     0,
     219,     0,   219,   511,   512,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1832,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1843,     0,   219,   867,
       0,     0,  1076,     0,     0,   511,   512,     0,     0,   345,
     345,    34,    35,    36,     0,   867,   867,   867,   867,   867,
       0,     0,   511,   512,   208,     0,     0,   867,     0,     0,
       0,     0,     0,     0,     0,   240,   906,     0,     0,     0,
       0,     0,     0,  1134,     0,     0,     0,   240,     0,     0,
       0,   240,     0,     0,     0,   240,   240,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   219,
     240,     0,     0,  1903,     0,     0,   860,     0,    81,    82,
      83,    84,    85,  1157,     0,   219,   219,   860,     0,   215,
       0,     0,   218,   860,     0,    89,    90,   860,     0,     0,
    1843,     0,  1916,     0,     0,   345,     0,     0,     0,    99,
    1157,     0,     0,     0,     0,   221,   221,     0,     0,   219,
       0,   345,     0,   104,   240,     0,   246,     0,   206,     0,
     207,    40,     0,     0,   345,     0,     0,     0,     0,     0,
       0,     0,     0,   218,     0,   206,   867,     0,     0,  1200,
      50,   240,     0,   240,     0,     0,     0,   964,   218,   218,
       0,     0,     0,   345,     0,     0,     0,    50,     0,   860,
     964,   242,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   240,     0,  1035,     0,   210,   211,   212,   213,
     214,     0,     0,     0,     0,     0,     0,   838,     0,   240,
       0,     0,     0,   210,   211,   212,   213,   214,   240,     0,
       0,     0,   778,     0,    93,    94,     0,    95,   184,    97,
       0,   240,     0,     0,     0,     0,     0,   219,   219,  1760,
       0,    93,    94,  1761,    95,   184,    97,   345,     0,     0,
       0,   345,   107,   841,     0,     0,   812,   206,   111,   218,
       0,     0,     0,     0,     0,     0,     0,   839,     0,   107,
    1601,     0,   867,   867,   867,   867,   867,   867,   219,    50,
       0,   867,   867,   867,   867,   867,   867,   867,   867,   867,
     867,   867,   867,   867,   867,   867,   867,   867,   867,   867,
     867,   867,   867,   867,   867,   867,   867,   867,   867,     0,
       0,     0,     0,     0,     0,   210,   211,   212,   213,   214,
       0,     0,     0,     0,     0,     0,     0,   867,     0,     0,
       0,     0,  1221,  1222,  1223,   206,     0,   183,     0,   221,
      91,   206,     0,    93,    94,     0,    95,   184,    97,     0,
     840,     0,     0,     0,     0,     0,   345,    50,   345,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,   107,   219,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   345,     0,     0,   345,     0,
       0,     0,     0,   210,   211,   212,   213,   214,     0,   210,
     211,   212,   213,   214,     0,     0,     0,   206,     0,  1035,
    1035,  1035,  1035,  1035,  1035,     0,     0,   219,     0,  1035,
       0,    93,    94,   219,    95,   184,    97,    93,    94,    50,
      95,   184,    97,     0,     0,     0,     0,     0,   219,   219,
       0,   867,     0,     0,     0,     0,     0,   345,     0,   107,
       0,     0,     0,   345,     0,   107,   707,     0,     0,     0,
     867,     0,   867,     0,     0,   210,   211,   212,   213,   214,
       0,     0,     0,     0,   221,     0,     0,     0,     0,     0,
       0,     0,     0,   221,     0,     0,   867,     0,     0,     0,
     221,     0,     0,    93,    94,     0,    95,   184,    97,     0,
       0,     0,   221,     0,     0,     0,     0,   453,   454,   455,
       0,     0,     0,   246,     0,     0,     0,     0,   345,   345,
       0,   107,   983,     0,     0,     0,  1481,   456,   457,   219,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,     0,   481,     0,     0,     0,     0,     0,
       0,  1014,  1015,     0,     0,     0,   482,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1035,     0,  1035,
       0,  1016,     0,     0,     0,     0,     0,   246,     0,  1017,
    1018,  1019,   206,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1020,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   869,    50,     0,     0,     0,   345,     0,
     345,     0,     0,     0,     0,     0,     0,     0,     0,   221,
     867,     0,   867,     0,   867,     0,     0,     0,     0,   867,
     219,     0,   206,   867,     0,   867,   899,     0,   867,  1021,
    1022,  1023,  1024,  1025,  1026,     0,     0,   345,   283,     0,
       0,     0,  1583,     0,    50,  1596,     0,  1027,   345,     0,
       0,     0,   183,     0,     0,    91,    92,     0,    93,    94,
       0,    95,   184,    97,     0,     0,   868,     0,     0,     0,
       0,     0,     0,     0,     0,   285,  1028,   937,     0,     0,
     210,   211,   212,   213,   214,     0,   107,     0,   206,     0,
       0,     0,     0,     0,  1035,   206,  1035,     0,  1035,   868,
       0,     0,     0,  1035,     0,   219,   438,     0,    93,    94,
      50,    95,   184,    97,     0,   345,     0,    50,  -400,     0,
       0,     0,   867,     0,     0,     0,   441,   180,   181,    65,
      66,    67,     0,     0,  1660,  1661,   107,     0,   345,     0,
       0,     0,     0,     0,  1596,   571,   210,   211,   212,   213,
     214,   572,     0,   210,   211,   212,   213,   214,     0,     0,
       0,     0,   345,     0,   345,     0,     0,   283,   183,     0,
     345,    91,   336,     0,    93,    94,     0,    95,   184,    97,
       0,    93,    94,     0,    95,   184,    97,   221,     0,     0,
       0,     0,   340,     0,     0,     0,     0,  1035,   442,     0,
       0,     0,   107,   341,   285,     0,     0,     0,     0,   107,
       0,     0,     0,     0,   867,   867,   867,   206,     0,     0,
       0,   867,     0,  1807,     0,   345,     0,     0,     0,     0,
       0,  1596,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,  1064,     0,     0,   221,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1087,  1088,  1089,  1090,  1091,     0,     0,     0,     0,   838,
       0,     0,  1101,     0,   571,   210,   211,   212,   213,   214,
     572,     0,     0,     0,     0,     0,     0,     0,   221,     0,
     221,     0,     0,     0,     0,     0,     0,   183,     0,     0,
      91,   336,     0,    93,    94,     0,    95,   184,    97,     0,
    1082,     0,     0,     0,     0,     0,   221,   868,   283,   206,
       0,   340,     0,     0,     0,     0,     0,   345,     0,   839,
       0,   107,   341,   868,   868,   868,   868,   868,     0,   345,
       0,    50,     0,   345,     0,   868,     0,     0,     0,     0,
       0,  1035,  1035,     0,     0,   285,     0,     0,     0,     0,
       0,     0,  1858,     0,     0,     0,     0,     0,   206,     0,
       0,     0,   867,     0,     0,     0,     0,   210,   211,   212,
     213,   214,     0,   867,     0,     0,     0,   221,     0,   867,
      50,  1197,     0,   867,     0,     0,     0,     0,   578,   183,
       0,     0,    91,   221,   221,    93,    94,     0,    95,   184,
      97,     0,  1277,     0,     0,     0,   345,     0,     0,     0,
       0,     0,     0,     0,     0,   571,   210,   211,   212,   213,
     214,   572,     0,   107,     0,     0,     0,   246,     0,     0,
       0,     0,     0,   345,     0,   345,     0,     0,   183,     0,
       0,    91,   336,     0,    93,    94,     0,    95,   184,    97,
       0,     0,     0,     0,   868,   867,     0,     0,     0,     0,
       0,     0,   340,     0,     0,     0,     0,     0,  1935,     0,
       0,     0,   107,   341,     0,     0,     0,     0,     0,   246,
       0,     0,     0,     0,     0,  1583,     0,     0,     0,     0,
     345,     0,     0,     0,     0,     0,     0,  1091,  1289,     0,
       0,  1289,     0,   345,     0,     0,  1302,  1305,  1306,  1307,
    1309,  1310,  1311,  1312,  1313,  1314,  1315,  1316,  1317,  1318,
    1319,  1320,  1321,  1322,  1323,  1324,  1325,  1326,  1327,  1328,
    1329,  1330,  1331,  1332,     0,   221,   221,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1342,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     868,   868,   868,   868,   868,   868,   246,     0,     0,   868,
     868,   868,   868,   868,   868,   868,   868,   868,   868,   868,
     868,   868,   868,   868,   868,   868,   868,   868,   868,   868,
     868,   868,   868,   868,   868,   868,   868,     0,   453,   454,
     455,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   868,     0,     0,   456,   457,
       0,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,     0,     0,   206,     0,
       0,     0,     0,     0,     0,     0,  1432,   482,     0,     0,
     221,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,  1448,     0,  1449,   357,   358,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,  1469,   481,     0,     0,   246,   210,   211,   212,   213,
     214,   221,     0,     0,   482,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   221,   221,   359,   868,
       0,   360,     0,     0,    93,    94,     0,    95,   184,    97,
       0,     0,     0,     0,     0,     0,     0,     0,   868,     0,
     868,     0,     0,   361,     0,     0,     0,     0,     0,     0,
       0,     0,   107,     0,     0,     0,   453,   454,   455,     0,
       0,     0,     0,     0,   868,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   456,   457,   969,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,     0,     0,     0,   221,     0,     0,
       0,     0,     0,     0,     0,   482,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   453,   454,   455,     0,  1565,     0,  1566,     0,  1567,
       0,     0,     0,     0,  1568,     0,     0,     0,  1570,     0,
    1571,   456,   457,  1572,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     482,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   868,     0,
     868,     0,   868,     0,     0,     0,     0,   868,   246,     0,
       0,   868,   206,   868,     0,     0,   868,     0,   453,   454,
     455,     0,  1075,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,  1008,  1655,   456,   457,
       0,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,     0,     0,     0,     0,
     210,   211,   212,   213,   214,     0,     0,   482,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   183,   246,     0,    91,     0,     0,    93,    94,
       0,    95,   184,    97,     0,     0,     0,     0,     0,     0,
     868,  1012,     0,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,   107,     0,     0,  1800,
    1801,  1802,     0,     0,     0,     0,  1806,    11,   414,    13,
       0,     0,     0,     0,     0,     0,     0,     0,   761,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,   868,   868,   868,     0,     0,     0,  1148,   868,
       0,     0,     0,    50,     0,     0,     0,     0,  1812,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,   179,
     180,   181,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   182,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,     0,     0,     0,    99,  1867,     0,   100,
       0,     0,  1104,  1105,  1106,   101,     0,     0,  1877,     0,
     104,   105,   106,     0,  1882,   107,   108,     0,  1884,     0,
       0,   111,   112,  1107,   113,   114,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1130,     0,     0,     0,     0,     0,     0,     0,     0,
     868,     0,     0,     0,     0,     0,     5,     6,     7,     8,
       9,   868,     0,     0,     0,     0,    10,   868,     0,     0,
    1927,   868,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1905,    14,     0,    15,    16,     0,     0,     0,  1282,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,   868,    43,    44,    45,    46,     0,    47,
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
       0,   109,   110,  1167,   111,   112,     0,   113,   114,     5,
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
      58,     0,    59,    60,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,    88,    89,    90,
      91,    92,     0,    93,    94,     0,    95,    96,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,   103,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,  1357,   111,   112,     0,
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
      55,    56,    57,    58,     0,    59,    60,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
      88,    89,    90,    91,    92,     0,    93,    94,     0,    95,
      96,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,   103,     0,   104,
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
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,   686,   111,   112,     0,   113,   114,     5,     6,
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
     107,   108,     0,   109,   110,  1133,   111,   112,     0,   113,
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
     106,     0,     0,   107,   108,     0,   109,   110,  1181,   111,
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
     110,  1254,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,  1256,
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
      45,    46,     0,    47,     0,    48,     0,    49,  1433,     0,
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
    1574,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
       0,   109,   110,  1803,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,  1854,    49,     0,     0,    50,
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
      43,    44,    45,    46,     0,    47,  1897,    48,     0,    49,
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
     109,   110,  1914,   111,   112,     0,   113,   114,     5,     6,
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
     107,   108,     0,   109,   110,  1917,   111,   112,     0,   113,
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
     106,     0,     0,   107,   108,     0,   109,   110,  1934,   111,
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
     110,  1983,   111,   112,     0,   113,   114,     5,     6,     7,
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
     108,     0,   109,   110,  1984,   111,   112,     0,   113,   114,
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
       0,     0,   107,   108,     0,   109,   110,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,   554,     0,     0,     0,     0,     0,     0,     0,
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
      11,    12,    13,     0,     0,   825,     0,     0,     0,     0,
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
       0,     0,     0,    11,    12,    13,     0,     0,  1066,     0,
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
       0,  1649,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,  1795,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,   180,   181,    65,    66,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,     0,     0,     0,    99,     0,
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
       0,     0,     0,     0,     0,    11,     0,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
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
     106,     0,     0,   107,   185,     0,   350,     0,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,  1107,     0,    10,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,  1124,  1125,  1126,  1127,  1128,  1129,     0,     0,   701,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1130,    15,    16,     0,     0,     0,     0,    17,     0,    18,
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
       0,    95,   184,    97,     0,   702,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   185,     0,     0,
       0,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
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
     185,     0,     0,   820,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,
    1127,  1128,  1129,     0,     0,  1194,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1130,    15,    16,     0,
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
       0,  1195,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   185,     0,     0,     0,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,   414,     0,
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
       0,   197,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,   179,   180,   181,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     182,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   183,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   184,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,  1207,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   185,
       0,     0,     0,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,
    1129,     0,     0,     0,   233,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1130,     0,    15,    16,     0,     0,
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
       0,   107,   185,   453,   454,   455,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   456,   457,     0,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,   482,     0,     0,    17,     0,    18,    19,    20,
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
       0,     0,     0,  1218,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   185,     0,   268,   454,   455,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,   456,   457,     0,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,     0,   481,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,   482,     0,    17,     0,
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
     271,     0,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   414,     0,     0,     0,     0,     0,     0,
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
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,   179,   180,   181,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   182,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,  1248,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   185,   552,     0,     0,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,   715,   481,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   482,     0,     0,     0,
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
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
       0,     0,   761,     0,     0,     0,     0,     0,     0,     0,
       0,   482,     0,     0,    15,    16,     0,     0,     0,     0,
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
     185,     0,     0,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,
    1129,     0,     0,     0,     0,   802,     0,     0,     0,     0,
       0,     0,     0,     0,  1130,     0,     0,    15,    16,     0,
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
       0,     0,   107,   185,     0,     0,     0,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,     0,   481,     0,     0,     0,   804,     0,
       0,     0,     0,     0,     0,     0,   482,     0,     0,     0,
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
     104,   105,   106,     0,     0,   107,   185,     0,     0,     0,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1124,  1125,  1126,  1127,  1128,  1129,     0,     0,     0,     0,
       0,  1245,     0,     0,     0,     0,     0,     0,     0,  1130,
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
       0,     0,     0,     0,     0,   179,   180,   181,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   182,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
    1619,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   185,   453,   454,   455,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
     829,     0,    10,   456,   457,     0,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,   482,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,   646,    39,    40,     0,   830,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,   179,   180,
     181,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   182,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,     0,   273,   274,    99,   275,   276,   100,     0,
     277,   278,   279,   280,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   185,     0,     0,   281,   282,
     111,   112,     0,   113,   114,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,
    1127,  1128,  1129,     0,     0,     0,     0,   284,     0,     0,
       0,     0,     0,     0,     0,     0,  1130,     0,     0,     0,
       0,   286,   287,   288,   289,   290,   291,   292,     0,     0,
       0,   206,     0,   207,    40,     0,     0,     0,     0,     0,
       0,     0,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,    50,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   206,   327,     0,
     748,   329,   330,   331,     0,     0,     0,   332,   582,   210,
     211,   212,   213,   214,   583,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,   273,   274,     0,   275,   276,
       0,   584,   277,   278,   279,   280,     0,    93,    94,     0,
      95,   184,    97,   337,     0,   338,     0,     0,   339,     0,
     281,   282,     0,     0,     0,   210,   211,   212,   213,   214,
       0,     0,     0,     0,     0,   107,     0,     0,     0,   749,
       0,   111,     0,     0,     0,     0,     0,   183,     0,   284,
      91,    92,     0,    93,    94,     0,    95,   184,    97,     0,
       0,     0,     0,   286,   287,   288,   289,   290,   291,   292,
       0,     0,     0,   206,     0,   207,    40,     0,     0,     0,
       0,   107,     0,     0,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,    50,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   206,
     327,     0,   328,   329,   330,   331,     0,     0,     0,   332,
     582,   210,   211,   212,   213,   214,   583,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,   273,   274,     0,
     275,   276,     0,   584,   277,   278,   279,   280,     0,    93,
      94,     0,    95,   184,    97,   337,     0,   338,     0,     0,
     339,     0,   281,   282,     0,   283,     0,   210,   211,   212,
     213,   214,     0,     0,     0,     0,     0,   107,     0,     0,
       0,   749,     0,   111,     0,     0,     0,     0,     0,     0,
       0,   284,   360,     0,     0,    93,    94,     0,    95,   184,
      97,     0,   285,     0,     0,   286,   287,   288,   289,   290,
     291,   292,     0,     0,     0,   206,     0,     0,     0,     0,
       0,     0,     0,   107,     0,     0,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,    50,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,     0,   327,     0,     0,   329,   330,   331,     0,     0,
       0,   332,   333,   210,   211,   212,   213,   214,   334,     0,
       0,     0,     0,     0,     0,     0,   206,     0,   931,     0,
     932,     0,     0,     0,     0,   335,     0,     0,    91,   336,
       0,    93,    94,     0,    95,   184,    97,   337,    50,   338,
       0,     0,   339,   273,   274,     0,   275,   276,     0,   340,
     277,   278,   279,   280,     0,     0,     0,     0,     0,   107,
     341,     0,     0,     0,  1775,     0,     0,     0,   281,   282,
       0,   283,     0,     0,   210,   211,   212,   213,   214,     0,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   284,   481,     0,
       0,     0,    93,    94,     0,    95,   184,    97,   285,     0,
     482,   286,   287,   288,   289,   290,   291,   292,     0,     0,
       0,   206,     0,     0,     0,     0,     0,     0,     0,     0,
     107,     0,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,    50,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,     0,   327,     0,
       0,   329,   330,   331,     0,     0,     0,   332,   333,   210,
     211,   212,   213,   214,   334,     0,     0,     0,     0,     0,
       0,     0,   206,     0,     0,     0,     0,     0,     0,     0,
       0,   335,     0,     0,    91,   336,     0,    93,    94,     0,
      95,   184,    97,   337,    50,   338,     0,     0,   339,   273,
     274,     0,   275,   276,     0,   340,   277,   278,   279,   280,
       0,     0,     0,     0,     0,   107,   341,     0,     0,     0,
    1849,     0,     0,     0,   281,   282,     0,   283,     0,     0,
     210,   211,   212,   213,   214,     0, -1063, -1063, -1063, -1063,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,   284,   481,   889,     0,     0,    93,    94,
       0,    95,   184,    97,   285,     0,   482,   286,   287,   288,
     289,   290,   291,   292,     0,     0,     0,   206,     0,     0,
       0,     0,     0,     0,     0,     0,   107,     0,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,    50,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,     0,   327,     0,   328,   329,   330,   331,
       0,     0,     0,   332,   333,   210,   211,   212,   213,   214,
     334,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   335,     0,     0,
      91,   336,     0,    93,    94,     0,    95,   184,    97,   337,
       0,   338,     0,     0,   339,   273,   274,     0,   275,   276,
       0,   340,   277,   278,   279,   280,     0,     0,     0,     0,
       0,   107,   341,     0,     0,     0,     0,     0,     0,     0,
     281,   282,     0,   283,   457,     0,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,   284,
     481,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     285,     0,   482,   286,   287,   288,   289,   290,   291,   292,
       0,     0,     0,   206,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,    50,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,     0,
     327,     0,     0,   329,   330,   331,     0,     0,     0,   332,
     333,   210,   211,   212,   213,   214,   334,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   335,     0,     0,    91,   336,     0,    93,
      94,     0,    95,   184,    97,   337,     0,   338,     0,     0,
     339,     0,   273,   274,     0,   275,   276,   340,  1578,   277,
     278,   279,   280,     0,     0,     0,     0,   107,   341,     0,
       0,     0,     0,     0,     0,     0,     0,   281,   282,     0,
     283,     0,     0,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   284,   481,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   285,     0,   482,
     286,   287,   288,   289,   290,   291,   292,     0,     0,     0,
     206,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,    50,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,     0,   327,     0,     0,
     329,   330,   331,     0,     0,     0,   332,   333,   210,   211,
     212,   213,   214,   334,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     335,     0,     0,    91,   336,     0,    93,    94,     0,    95,
     184,    97,   337,     0,   338,     0,     0,   339,  1675,  1676,
    1677,  1678,  1679,     0,   340,  1680,  1681,  1682,  1683,     0,
       0,     0,     0,     0,   107,   341,     0,     0,     0,     0,
       0,     0,  1684,  1685,  1686,     0,   456,   457,     0,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,  1687,   481,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   482,  1688,  1689,  1690,  1691,
    1692,  1693,  1694,     0,     0,     0,   206,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1695,  1696,  1697,
    1698,  1699,  1700,  1701,  1702,  1703,  1704,  1705,    50,  1706,
    1707,  1708,  1709,  1710,  1711,  1712,  1713,  1714,  1715,  1716,
    1717,  1718,  1719,  1720,  1721,  1722,  1723,  1724,  1725,  1726,
    1727,  1728,  1729,  1730,  1731,  1732,  1733,  1734,  1735,     0,
       0,     0,  1736,  1737,   210,   211,   212,   213,   214,     0,
    1738,  1739,  1740,  1741,  1742,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1743,  1744,  1745,     0,
     206,     0,    93,    94,     0,    95,   184,    97,  1746,     0,
    1747,  1748,     0,  1749,     0,     0,     0,     0,     0,     0,
    1750,  1751,    50,  1752,     0,  1753,  1754,     0,   273,   274,
     107,   275,   276,     0,     0,   277,   278,   279,   280,     0,
       0,     0,     0,     0,  1587,     0,     0,     0,     0,     0,
       0,     0,     0,   281,   282,     0,     0,  1588,   210,   211,
     212,   213,   214,  1589,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     183,     0,   284,    91,    92,     0,    93,    94,     0,    95,
    1591,    97,     0,     0,     0,     0,   286,   287,   288,   289,
     290,   291,   292,     0,     0,     0,   206,     0,     0,     0,
       0,     0,     0,     0,   107,     0,     0,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,    50,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,     0,   327,     0,   328,   329,   330,   331,     0,
       0,     0,   332,   582,   210,   211,   212,   213,   214,   583,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     273,   274,     0,   275,   276,     0,   584,   277,   278,   279,
     280,     0,    93,    94,     0,    95,   184,    97,   337,     0,
     338,     0,     0,   339,     0,   281,   282,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     107,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   284,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   286,   287,
     288,   289,   290,   291,   292,     0,     0,     0,   206,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
      50,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,     0,   327,     0,  1300,   329,   330,
     331,     0,     0,     0,   332,   582,   210,   211,   212,   213,
     214,   583,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   273,   274,     0,   275,   276,     0,   584,   277,
     278,   279,   280,     0,    93,    94,     0,    95,   184,    97,
     337,     0,   338,     0,     0,   339,     0,   281,   282,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   107,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   284,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     286,   287,   288,   289,   290,   291,   292,     0,     0,     0,
     206,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,    50,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,     0,   327,     0,     0,
     329,   330,   331,     0,     0,     0,   332,   582,   210,   211,
     212,   213,   214,   583,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     584,     0,     0,     0,     0,     0,    93,    94,     0,    95,
     184,    97,   337,     0,   338,     0,     0,   339,   453,   454,
     455,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   107,     0,     0,     0,   456,   457,
       0,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,   453,   454,   455,     0,
       0,     0,     0,     0,     0,     0,     0,   482,     0,     0,
       0,     0,     0,     0,     0,     0,   456,   457,  1437,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,   453,   454,   455,     0,     0,     0,
       0,     0,     0,     0,     0,   482,     0,     0,     0,     0,
       0,     0,     0,     0,   456,   457,     0,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,   453,   454,   455,     0,     0,     0,     0,     0,
       0,     0,     0,   482,     0,     0,     0,     0,     0,     0,
       0,     0,   456,   457,     0,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,  1620,   481,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   482,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   453,   454,   455,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   456,   457,  1438,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
     453,   454,   455,     0,     0,     0,     0,     0,     0,     0,
       0,   482,     0,     0,     0,     0,     0,     0,     0,     0,
     456,   457,   483,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,     0,   481,   453,   454,
     455,     0,     0,     0,     0,     0,     0,     0,     0,   482,
       0,     0,     0,     0,     0,     0,     0,     0,   456,   457,
     568,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   482,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   453,   454,
     455,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   456,   457,
     570,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,   453,   454,   455,     0,
       0,     0,     0,     0,     0,     0,     0,   482,     0,     0,
       0,     0,     0,     0,     0,     0,   456,   457,   589,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,  1104,  1105,  1106,     0,     0,     0,
       0,     0,     0,     0,   283,   482,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1107,   593,     0,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,
       0,   285,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   283,  1130,   206,     0,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,
    1127,  1128,  1129,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,   794,  1130,     0,     0,   285,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   206,     0,     0,     0,     0,     0,     0,     0,
       0,   571,   210,   211,   212,   213,   214,   572,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,   817,   183,     0,     0,    91,   336,     0,
      93,    94,     0,    95,   184,    97,     0,  1445,     0,     0,
       0,     0,  1308,     0,     0,     0,     0,     0,   340,   571,
     210,   211,   212,   213,   214,   572,     0,     0,   107,   341,
     847,   848,     0,     0,     0,     0,   849,     0,   850,     0,
       0,  1299,   183,     0,     0,    91,   336,     0,    93,    94,
     851,    95,   184,    97,     0,     0,     0,     0,    34,    35,
      36,   206,     0,     0,     0,     0,   340,     0,     0,     0,
       0,   208,  1104,  1105,  1106,     0,   107,   341,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1107,     0,     0,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,   852,   853,
     854,   855,   856,   857,     0,    81,    82,    83,    84,    85,
       0,  1130,     0,     0,     0,     0,   215,  1060,     0,     0,
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,     0,     0,     0,    99,     0,     0,     0,
       0,     0,     0,     0,     0,   858,     0,     0,     0,    29,
     104,     0,     0,     0,     0,   107,   859,    34,    35,    36,
     206,     0,   207,    40,     0,     0,     0,     0,     0,     0,
     208,     0,     0,     0,     0,     0,     0,     0,  1454,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   209, -1063,
   -1063, -1063, -1063,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1124,  1125,  1126,  1127,  1128,  1129,  1061,    75,   210,   211,
     212,   213,   214,     0,    81,    82,    83,    84,    85,  1130,
       0,     0,     0,     0,     0,   215,     0,     0,     0,     0,
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,     0,   847,   848,    99,     0,     0,     0,   849,
       0,   850,     0,     0,     0,     0,     0,     0,     0,   104,
       0,     0,     0,   851,   107,   216,     0,     0,     0,     0,
     111,    34,    35,    36,   206,     0,     0,     0,   453,   454,
     455,     0,     0,     0,   208,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,   456,   457,
       0,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,     0,     0,     0,     0,
       0,   852,   853,   854,   855,   856,   857,   482,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,   215,
       0,     0,     0,     0,   183,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   184,    97,    29,     0,     0,    99,
       0,     0,     0,     0,    34,    35,    36,   206,   858,   207,
      40,     0,     0,   104,     0,     0,     0,   208,   107,   859,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,   529,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   209,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,   210,   211,   212,   213,   214,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,   215,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,    29,
       0,     0,    99,     0,     0,     0,     0,    34,    35,    36,
     206,     0,   207,    40,     0,     0,   104,     0,     0,     0,
     208,   107,   216,     0,     0,   609,     0,   111,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   209,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   629,    75,   210,   211,
     212,   213,   214,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   215,     0,     0,     0,     0,
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,    29,  1003,     0,    99,     0,     0,     0,     0,
      34,    35,    36,   206,     0,   207,    40,     0,     0,   104,
       0,     0,     0,   208,   107,   216,     0,     0,     0,     0,
     111,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   209,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,  1160,    75,   210,   211,   212,   213,   214,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,   215,     0,     0,     0,     0,   183,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   184,    97,    29,     0,
       0,    99,     0,     0,     0,     0,    34,    35,    36,   206,
       0,   207,    40,     0,     0,   104,     0,     0,     0,   208,
     107,   216,     0,     0,     0,     0,   111,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   209,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    75,   210,   211,   212,
     213,   214,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,   215,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,     0,     0,     0,    99,     0,     0,   453,   454,   455,
       0,     0,     0,     0,     0,     0,     0,     0,   104,     0,
       0,     0,     0,   107,   216,     0,     0,   456,   457,   111,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,     0,   481,   453,   454,   455,     0,     0,
       0,     0,     0,     0,     0,     0,   482,     0,     0,     0,
       0,     0,     0,     0,     0,   456,   457,     0,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,     0,     0,     0,     0,     0,     0,     0,
       0,   453,   454,   455,   482,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   456,   457,   538,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,   453,
     454,   455,     0,     0,     0,     0,     0,     0,     0,     0,
     482,     0,     0,     0,     0,     0,     0,     0,     0,   456,
     457,   921,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,     0,     0,     0,
       0,     0,     0,     0,     0,   453,   454,   455,   482,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   456,   457,   989,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,   453,   454,   455,     0,     0,     0,     0,
       0,     0,     0,     0,   482,     0,     0,     0,     0,     0,
       0,     0,     0,   456,   457,  1045,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,     0,     0,     0,     0,     0,     0,  1104,  1105,
    1106,     0,   482,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1107,
       0,  1355,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,
    1126,  1127,  1128,  1129,     0,     0,  1104,  1105,  1106,     0,
       0,     0,     0,     0,     0,     0,     0,  1130,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1107,     0,  1386,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,
    1128,  1129,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1104,  1105,  1106,     0,  1130,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1107,     0,  1465,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,  1124,  1125,  1126,  1127,  1128,  1129,     0,     0,  1104,
    1105,  1106,     0,     0,     0,     0,     0,     0,     0,     0,
    1130,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1107,     0,  1564,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,
    1125,  1126,  1127,  1128,  1129,     0,    34,    35,    36,   206,
       0,   207,    40,     0,     0,     0,     0,     0,  1130,   208,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,  1656,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   237,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   238,     0,
       0,     0,     0,     0,     0,     0,     0,   210,   211,   212,
     213,   214,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,   215,  1658,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,     0,     0,     0,    99,     0,    34,    35,    36,   206,
       0,   207,    40,     0,     0,     0,     0,     0,   104,   660,
       0,     0,     0,   107,   239,     0,     0,     0,     0,   111,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   209,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   210,   211,   212,
     213,   214,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,   215,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,     0,     0,     0,    99,     0,    34,    35,    36,   206,
       0,   207,    40,     0,     0,     0,     0,     0,   104,   208,
     206,     0,     0,   107,   661,     0,     0,     0,     0,   662,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,   237,     0,     0,
     887,   888,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   210,   211,   212,
     213,   214,     0,    81,    82,    83,    84,    85,   210,   211,
     212,   213,   214,     0,   215,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,     0,     0,   889,    99,     0,    93,    94,     0,    95,
     184,    97,     0,     0,     0,   453,   454,   455,   104,     0,
       0,     0,     0,   107,   239,     0,     0,     0,     0,   111,
       0,     0,     0,     0,   107,   456,   457,   986,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,   453,   454,   455,     0,     0,     0,     0,
       0,     0,     0,     0,   482,     0,     0,     0,     0,     0,
       0,     0,     0,   456,   457,     0,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,  1104,  1105,  1106,     0,     0,     0,     0,     0,     0,
       0,     0,   482,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1107,  1470,     0,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,  1124,  1125,  1126,  1127,  1128,  1129,  1104,  1105,  1106,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1130,     0,     0,     0,     0,     0,     0,     0,  1107,     0,
       0,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,
    1127,  1128,  1129,  1105,  1106,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1130,     0,     0,     0,
       0,     0,     0,  1107,     0,     0,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,   455,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1130,     0,     0,     0,     0,   456,   457,     0,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,  1106,   481,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   482,     0,     0,     0,     0,
       0,  1107,     0,     0,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1124,  1125,  1126,  1127,  1128,  1129,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1130
};

static const yytype_int16 yycheck[] =
{
       5,     6,   129,     8,     9,    10,    11,    12,    13,   161,
      15,    16,    17,    18,   187,    56,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     4,    31,     4,     4,   674,
      98,     4,   108,   523,   102,   103,   407,    33,   678,    44,
     962,   542,    19,    20,   407,    57,   240,    52,   166,    54,
      46,   951,    57,   108,    59,    51,    30,   481,   675,     4,
     128,   188,  1184,   161,   704,    30,     4,  1347,   407,    57,
     513,    30,  1171,   513,   251,  1180,   758,   653,   108,   554,
     835,    86,   108,   487,  1059,   982,   603,   604,   828,   517,
     518,     9,   806,   357,   358,   359,    14,   361,    32,     9,
      32,   998,     4,   108,  1788,     9,    48,   550,     9,     9,
     550,     9,     9,    14,    14,   590,     9,    70,     9,    48,
     548,     9,     9,     9,    70,    83,     9,    83,    48,    70,
     185,   108,     9,    48,   252,     9,    83,     9,     9,     9,
     115,     9,     9,    70,     9,     9,    54,   164,     9,  1046,
       9,   160,     9,     9,    38,   185,    70,    70,   160,   185,
     196,   216,    36,   134,   135,   160,    83,    70,    90,   166,
      48,   196,   181,    90,    81,     4,   160,   181,     4,   181,
     185,   198,   199,    38,   239,     0,   216,   192,    30,   196,
     216,   129,   196,   134,   135,   134,   135,   199,     8,    83,
    1100,   676,   199,   178,   199,    50,    51,   160,   185,   239,
     121,   216,   196,   160,   160,   392,   174,    70,    70,   130,
     134,   135,    70,   130,    53,   196,    70,    56,    83,    70,
      70,    70,    70,   160,   239,   157,    70,    70,    70,   216,
     157,   158,   159,   196,    73,   201,   199,    70,    70,   254,
     188,   891,   257,   199,    70,   197,   233,   165,   199,   264,
     265,   200,   239,   181,   193,   199,   198,    96,  1952,    98,
     443,    70,   199,   102,   103,    60,  1381,   197,   193,  1963,
     198,   258,   197,   196,  1383,   262,  1261,  1001,   198,   199,
     174,  1390,    44,  1392,   198,  1575,   199,   198,   198,   128,
     198,   198,    38,    88,   196,   198,    91,   198,   349,    38,
     198,   198,   198,   258,  1069,   198,  1071,   262,  1215,   197,
      54,   198,  1421,   197,   379,    83,   198,   198,   198,   197,
     197,   236,   197,   197,    86,   161,   197,   182,   197,  1239,
     197,   197,    38,   196,   196,   820,   199,    83,   196,   379,
     825,   199,   525,   379,    83,   196,    83,    70,   199,   199,
     199,   199,   979,    38,   940,   199,   199,   199,   495,   437,
     887,   888,   201,   378,   379,    90,   199,   199,   196,   175,
     385,   386,   387,   388,   389,   390,   181,    83,   106,   107,
     395,   368,   196,   434,   106,   107,   160,   196,   197,   199,
     377,   196,   379,   181,  1483,    50,    51,   384,    83,   414,
      83,    19,    20,  1518,    83,  1520,   174,   422,   196,   396,
     496,    90,   490,   491,   492,   493,   120,   917,   433,   258,
    1529,   165,   196,   262,    83,    84,   130,   266,   174,    83,
      83,    84,   157,   196,   421,   174,   180,   174,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,    14,   482,   174,   484,
     485,   486,   200,   421,   496,    83,   102,  1169,   200,   158,
     159,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   681,  1584,   683,   481,   496,   514,
     515,   413,   517,   518,   519,   520,   481,  1257,   196,   192,
     349,   526,   481,  1628,   529,   968,   199,  1632,   968,  1608,
     174,  1610,   196,   538,    83,   540,   205,   102,  1183,   196,
     692,    90,   196,   548,   556,   102,   523,   160,   164,   196,
     954,   556,   201,   558,    70,    70,   494,   751,   201,   157,
     158,   159,  1462,  1843,   102,   103,   181,   995,   181,   485,
     102,    70,   357,   358,   359,   360,   361,    75,    76,  1003,
     951,   196,  1199,   196,   561,  1202,   199,   760,   951,   166,
     784,  1066,   421,    83,   692,   421,    75,    76,   514,   164,
      90,   430,   179,   519,   609,   434,   661,   164,   437,   158,
     159,   407,   951,   877,   399,   879,   561,   881,   134,   135,
      83,   885,   199,   800,   801,   233,   378,   532,  1145,    57,
     807,   808,   164,   134,   135,   387,   388,    70,   390,   481,
     111,    69,   710,    70,   160,   487,   165,  1547,   119,   120,
     121,   122,   123,   124,    31,   204,   661,   196,   487,   488,
     489,   490,   491,   492,   493,   199,   860,   157,   158,   159,
      83,   513,    83,    50,    83,    14,    53,    90,   872,    90,
     196,    90,  1787,  1848,   513,  1850,  1791,    32,   388,   807,
     390,   487,   164,    32,   536,   158,   159,   702,  1102,   119,
     120,   121,   122,   123,   124,   547,    38,   536,   550,  1391,
     715,   181,    51,  1097,    31,  1099,  1217,   513,   160,   196,
     191,   550,   164,   393,   701,  1370,   196,   397,    70,  1100,
     635,   636,   561,    50,   102,   103,    53,  1100,   198,   181,
     536,   132,   133,    81,   749,   158,   159,   158,   159,   158,
     159,   547,   581,   423,   550,   425,   426,   427,   428,   664,
     368,  1100,   357,   358,   359,   103,   361,   198,  1385,   377,
    1260,   191,   198,   675,   779,  1234,   384,  1236,   607,   608,
    1208,   105,   106,   107,   761,   105,   106,   107,   396,    70,
     128,  1219,   119,   120,   121,   122,   123,   124,   198,   407,
     198,   139,   140,   141,   142,   143,   198,   812,   198,   199,
      53,    54,    55,   642,   643,   198,  1498,   198,   603,   604,
     122,   123,   124,   828,  1946,   802,    69,   804,   824,   167,
     168,    70,   170,   171,   172,  1940,     6,  1959,   112,   113,
     114,   818,   747,    70,    50,    51,    52,    53,    54,    55,
    1955,    57,    70,   830,   198,   199,   833,   195,   196,    70,
    1335,  1351,    70,    69,   191,   199,   692,   160,  1239,  1819,
    1820,   198,    70,    53,    54,    55,  1239,    57,    48,   164,
     818,   710,    50,    51,    52,    53,    54,    55,   833,    69,
    1815,  1816,   196,   845,   846,   196,   160,   874,   196,   198,
    1239,    69,    49,  1585,    69,   181,   160,   196,   196,   203,
       9,   160,   160,  1530,   196,   523,   921,     8,   923,   198,
     925,   196,   160,   796,   901,    14,   160,   198,   198,     9,
     199,   936,   837,   198,  1002,    14,   874,   130,   843,   130,
     917,   918,   112,    14,   197,   950,   181,   117,  1423,   119,
     120,   121,   122,   123,   124,   125,   102,   197,   202,  1387,
     939,   197,   939,   939,  1439,   197,   939,   796,   197,   798,
     196,   976,   111,   196,   196,     9,   157,   197,   197,    94,
     197,   986,   197,   960,   989,     9,   991,  1909,   198,   818,
     995,    14,   818,   181,   939,   196,  1190,   167,   168,     9,
     170,   939,   196,   832,   833,    83,   132,   199,   913,   198,
    1932,   197,   199,   198,   197,   199,   198,     9,   196,  1941,
     199,   191,   960,   198,   197,     9,   198,   203,   197,  1003,
     200,    70,   203,    32,   136,   133,   203,   939,  1003,   203,
    1045,   203,   180,   160,  1003,   874,     9,   197,   874,   160,
      14,   193,     9,     9,   883,   884,  1052,   182,     9,   132,
     962,   197,    14,   203,    19,    20,   203,   200,     9,    14,
     197,   203,   203,   197,   196,  1550,  1053,   979,  1055,  1483,
    1057,   197,  1059,  1060,  1559,   914,  1162,   102,  1516,   198,
     160,  1462,   877,   701,   879,   198,   881,     9,  1573,  1462,
     885,   136,   887,   888,   889,   160,     9,   197,  1053,   196,
     939,    70,   954,   939,   196,   199,    70,  1055,     9,  1057,
      70,    70,    70,  1462,   966,   954,   968,     6,   200,    14,
     198,   960,   182,     9,   960,  1040,   199,   966,   203,   968,
      14,   199,    14,  1016,   197,   193,   198,   196,    32,   196,
    1162,    32,    14,   761,   196,   951,   196,  1162,   954,    14,
      52,  1003,    70,   196,    70,    70,    70,   196,    70,    48,
     966,   160,   968,  1002,  1649,     9,  1547,  1581,   197,   196,
    1584,  1086,   198,   136,  1547,  1014,  1015,  1016,  1093,  1168,
    1195,  1168,  1168,   198,   802,  1168,   804,    14,   136,  1839,
     182,  1841,   160,  1208,  1608,  1047,  1610,     9,  1547,   197,
      69,     9,  1616,   203,  1219,  1220,     6,  1194,  1047,    83,
     200,   200,   830,  1168,  1053,   200,  1055,   200,  1057,  1055,
    1168,  1057,   198,   112,     9,   196,   136,    14,   117,    83,
     119,   120,   121,   122,   123,   124,   125,   136,     9,  1078,
     196,  1047,  1257,  1249,   198,   197,   199,    56,    48,   196,
     196,    91,  1267,   197,   199,   198,  1168,   199,  1245,   157,
      32,    19,    20,  1102,   203,  1252,   199,    77,   233,   198,
     197,   182,    30,  1260,  1261,   198,   136,  1264,   167,   168,
    1930,   170,    32,   901,   203,     9,   203,  1199,   197,   197,
    1202,     9,  1131,   136,  1100,     9,   203,  1252,    56,   917,
     918,   197,   191,   203,     9,   197,   203,   200,   198,    83,
    1795,   200,   112,   198,  1229,   198,  1264,   117,   198,   119,
     120,   121,   122,   123,   124,   125,    14,   200,   199,  1168,
     196,   198,  1168,   951,   197,   197,   197,  1499,   197,   196,
    1355,   199,   197,     9,     6,   136,     9,  1362,  1231,  1232,
    1145,  1366,  1235,  1368,   203,  1840,   203,  1494,  1241,   136,
    1845,  1376,   203,   197,  1351,   203,   203,   167,   168,     9,
     170,  1386,  1387,  1288,    32,  1789,   198,  1292,   197,   197,
     136,   198,  1297,   198,   112,   199,    48,   169,    14,  1304,
     165,   191,  1231,  1232,  1233,  1234,  1235,  1236,    83,   198,
     200,   117,  1241,   368,   197,   197,   136,   197,    14,  1894,
     199,   136,   377,  1252,   379,   181,   199,   198,    83,   384,
      14,    14,    83,  1606,   197,  1264,   196,   195,  1264,   136,
     197,   396,   136,  1239,   198,  1274,   198,  1349,     6,    14,
      14,  1059,  1060,   198,    14,   199,     9,  1359,     9,   200,
     112,    68,    83,   196,   181,   117,    83,   119,   120,   121,
     122,   123,   124,   125,     9,     9,   199,   198,   115,   102,
     160,   182,   102,  1385,   283,   233,   285,   172,    36,  1964,
      48,  1966,  1100,    14,   196,  1472,   178,   197,   196,   198,
       9,  1406,    83,   182,   175,  1410,   182,   197,  1513,    83,
    1578,  1516,  1417,   197,  1343,   167,   168,   198,   170,    14,
     197,    83,    14,  1349,    83,   199,  1399,    14,  1401,    83,
      14,    83,  1921,  1359,  1472,   283,  1145,   285,   996,   191,
     942,  1937,   341,  1648,  1258,  1932,   493,  1436,   200,   488,
     490,   611,  1635,  1673,   112,  1493,  1758,  1586,  1489,   117,
    1973,   119,   120,   121,   122,   123,   124,   125,   523,  1770,
    1399,  1953,  1401,  1485,  1551,  1631,   389,  1098,  1172,  1556,
    1235,  1094,  1230,  1560,   966,  1015,  1194,  1042,   385,  1231,
    1833,   434,  1889,   341,   845,  1879,  1477,  1153,  1079,    -1,
    1131,  1774,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   167,
     168,  1556,   170,  1551,    -1,    -1,    -1,  1519,  1623,    -1,
     368,    -1,  1560,  1525,    -1,  1527,    -1,    -1,  1530,   377,
      -1,  1239,   431,   191,    -1,   434,   384,  1245,    -1,    -1,
      -1,    -1,   200,  1472,    -1,    -1,  1472,  1549,   396,    -1,
      -1,    -1,  1260,  1261,  1483,    -1,    -1,    -1,    -1,   407,
    1489,    -1,    -1,  1536,    -1,  1538,  1462,  1540,    -1,  1646,
    1647,  1648,  1545,  1499,    -1,  1652,  1614,   796,    -1,    -1,
      -1,    -1,  1659,   431,    -1,    -1,   434,    -1,    -1,    -1,
      -1,    -1,    -1,  1519,    -1,    -1,    -1,    -1,    -1,  1525,
      -1,  1527,  1647,  1648,    -1,    -1,    -1,  1536,  1646,  1538,
      -1,  1540,    -1,    -1,  1652,    -1,  1545,    -1,    -1,    -1,
      -1,  1659,  1551,  1549,    -1,  1551,    -1,  1556,  1769,    -1,
      31,  1560,    -1,   481,  1560,  1637,   119,   120,   121,   122,
     123,   124,    -1,  1351,    -1,    -1,   701,   130,   131,  1578,
    1902,  1547,  1581,    -1,    -1,  1584,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,  1594,  1639,    -1,    -1,    -1,
     130,   131,  1601,     6,  1779,   523,   575,    -1,    -1,  1608,
      81,  1610,    -1,    -1,    -1,    -1,    -1,  1616,   171,    -1,
     173,    -1,  1833,    -1,    -1,    -1,    -1,    -1,  1925,    -1,
      -1,    -1,   103,   186,    -1,   188,   761,    -1,   191,    -1,
    1639,  1637,    -1,   173,    -1,    48,    -1,  1646,  1647,  1648,
    1646,    -1,  1799,  1652,   125,    -1,  1652,   575,    -1,   577,
    1659,   191,   580,  1659,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,   802,    -1,   804,
      -1,    -1,    -1,    -1,  1462,    -1,    -1,    -1,    -1,    -1,
     161,  1799,    -1,   164,   165,   613,   167,   168,    -1,   170,
     171,   172,   671,   672,    -1,   830,  1853,    -1,    -1,   112,
      -1,   680,    -1,    -1,   117,    -1,   119,   120,   121,   122,
     123,   124,   125,    -1,   195,    -1,    -1,  1016,    -1,    -1,
      -1,    -1,    -1,    -1,  1809,  1810,    -1,    19,    20,    -1,
      -1,  1888,    -1,    -1,    -1,  1853,  1893,    -1,    -1,    -1,
    1793,  1794,    -1,   671,   672,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   680,    -1,   167,   168,    -1,   170,    -1,  1547,
    1769,  1918,    -1,    -1,    -1,    -1,   901,    -1,    -1,    -1,
    1888,    -1,    -1,   701,    -1,  1893,    -1,    -1,   191,    -1,
    1789,    -1,   917,   918,  1793,  1794,  1971,   200,    -1,    -1,
    1799,    -1,    -1,  1799,  1979,    -1,    -1,    -1,    -1,  1808,
    1918,    -1,  1987,    -1,    -1,  1990,  1815,  1816,    -1,    -1,
    1819,  1820,    -1,    -1,    -1,    -1,    -1,  1974,  1975,    -1,
      -1,    -1,    -1,    -1,  1833,    -1,    -1,  1909,    -1,    -1,
      -1,    -1,    -1,   761,    -1,    -1,    -1,    -1,    -1,     6,
      -1,    -1,    -1,    -1,  1853,    -1,    -1,  1853,    -1,    -1,
    1932,    -1,    -1,    -1,    -1,    -1,  1974,  1975,    -1,  1941,
      -1,    -1,    -1,    -1,    -1,    -1,   845,   846,   796,    -1,
      -1,    -1,    -1,    -1,   802,    -1,   804,    -1,    -1,  1888,
      -1,    48,  1888,    -1,  1893,    -1,    -1,  1893,    -1,    -1,
      -1,    -1,  1901,    -1,    -1,    -1,  1902,    -1,    -1,    -1,
      -1,    -1,   830,   831,    -1,    -1,    -1,    -1,    -1,  1918,
     838,    -1,  1918,    -1,    -1,  1924,    -1,   845,   846,   847,
     848,   849,   850,   851,  1059,  1060,    -1,    -1,    -1,    -1,
      -1,   859,  1231,  1232,  1233,  1234,  1235,  1236,    -1,    -1,
      -1,   233,  1241,    -1,    -1,   112,    -1,   875,    -1,    -1,
     117,    -1,   119,   120,   121,   122,   123,   124,   125,    -1,
      -1,    -1,   941,    -1,    -1,  1974,  1975,    -1,  1974,  1975,
      -1,    -1,    -1,   901,    -1,    -1,    -1,    81,   957,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,   915,    -1,   917,
     918,   970,    -1,    -1,    -1,    -1,    -1,    19,    20,   103,
     167,   168,    -1,   170,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    -1,    -1,   941,   942,    -1,    -1,    -1,    -1,    -1,
     999,    -1,    81,   951,   191,    -1,    -1,    -1,    -1,   957,
      56,    -1,    -1,   200,    -1,   139,   140,   141,   142,   143,
      -1,    -1,   970,    -1,   103,    -1,    -1,    -1,    -1,    -1,
     978,    -1,    -1,   981,    -1,    -1,    -1,    -1,    -1,  1194,
      -1,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
      -1,   999,    -1,    -1,    -1,  1003,   368,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   377,    -1,    -1,  1016,    -1,
      -1,   195,   384,    -1,  1073,   199,    -1,   201,  1077,    -1,
    1399,    -1,  1401,    -1,   396,   164,    -1,    -1,   167,   168,
    1245,   170,   171,   172,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1260,  1261,    -1,    -1,    -1,
      -1,  1059,  1060,    -1,    -1,    -1,   195,    -1,    -1,    -1,
     199,    -1,    -1,    -1,    -1,  1073,    -1,    -1,    -1,  1077,
      -1,  1079,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1094,  1095,  1096,  1097,
    1098,  1099,  1100,    -1,    -1,  1103,  1104,  1105,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,
    1128,  1129,  1130,  1182,    -1,  1184,    -1,    -1,    -1,    -1,
      -1,   233,    -1,    -1,    -1,    -1,  1351,    -1,    -1,    -1,
      -1,  1149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   523,  1211,    -1,    -1,  1214,    -1,  1536,    -1,  1538,
      -1,  1540,    -1,    -1,    -1,    -1,  1545,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1182,    -1,  1184,   283,    -1,   285,
      -1,    -1,    -1,    -1,    -1,    -1,  1194,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,  1211,    -1,    -1,  1214,    -1,    -1,    -1,
      -1,    -1,    -1,    69,  1273,    -1,    -1,    -1,    -1,    -1,
    1279,    -1,    -1,  1231,  1232,  1233,  1234,  1235,  1236,    -1,
      -1,  1239,    -1,  1241,    -1,   341,    -1,  1245,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,  1260,  1261,    -1,  1263,    -1,    -1,    -1,    -1,
    1639,    -1,    -1,    -1,    69,  1273,   368,    -1,    -1,    -1,
      -1,  1279,    -1,    -1,  1282,   377,  1284,    -1,    -1,    -1,
      -1,    -1,   384,    -1,    -1,  1344,  1345,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   396,    -1,    -1,    -1,    -1,    -1,
    1308,    -1,    -1,    -1,    -1,   407,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,   431,    -1,    -1,   434,   701,
      -1,    -1,    -1,    -1,    -1,    -1,  1344,  1345,    -1,    -1,
    1348,    30,    31,  1351,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   481,
      69,    -1,    -1,    -1,    -1,  1444,    -1,  1446,    -1,   761,
      -1,  1399,    -1,  1401,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    -1,    -1,  1793,  1794,    -1,    -1,    -1,    -1,
      -1,   523,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     802,    -1,   804,    -1,    -1,  1494,  1444,    -1,  1446,    -1,
      -1,    59,    60,    -1,  1452,    -1,  1454,    -1,  1456,    -1,
      -1,    -1,    -1,  1461,  1462,    -1,    -1,  1465,   830,  1467,
      -1,    81,  1470,    -1,    -1,    -1,    -1,    -1,    -1,   575,
      -1,   577,    -1,    -1,    -1,  1483,  1484,    -1,   580,  1487,
      -1,    -1,    -1,   103,    -1,    -1,  1494,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,  1561,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   613,    -1,    -1,   203,    -1,   134,   135,    -1,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    -1,  1536,   901,
    1538,    -1,  1540,    -1,    59,    60,    -1,  1545,    -1,  1547,
      -1,   161,    -1,    -1,   164,   917,   918,   167,   168,    -1,
     170,   171,   172,  1561,    -1,    -1,  1564,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   671,   672,    -1,  1576,  1577,
      -1,    -1,    -1,    -1,   680,   195,  1584,    -1,  1586,   197,
     200,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    -1,    -1,   701,
    1608,    -1,  1610,    -1,    -1,    -1,    -1,    -1,  1616,   134,
     135,    -1,  1671,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    59,    60,
      -1,  1639,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,  1656,  1657,
    1658,    -1,    -1,    -1,    -1,  1663,    -1,  1665,    -1,   761,
      59,    60,    -1,  1671,    -1,  1673,    -1,    -1,    -1,    -1,
      -1,    -1,   197,    -1,    -1,    -1,    -1,    59,    60,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1059,  1060,    -1,
      -1,    -1,    -1,    -1,   796,    -1,    -1,    -1,    -1,    -1,
     802,    -1,   804,   134,   135,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1773,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1785,    -1,   830,   831,
      -1,    -1,   838,    -1,    -1,   134,   135,    -1,    -1,   845,
     846,    78,    79,    80,    -1,   847,   848,   849,   850,   851,
      -1,    -1,   134,   135,    91,    -1,    -1,   859,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1773,   197,    -1,    -1,    -1,
      -1,    -1,    -1,   875,    -1,    -1,    -1,  1785,    -1,    -1,
      -1,  1789,    -1,    -1,    -1,  1793,  1794,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   901,
    1808,    -1,    -1,  1862,    -1,    -1,  1814,    -1,   145,   146,
     147,   148,   149,   915,    -1,   917,   918,  1825,    -1,   156,
      -1,    -1,  1194,  1831,    -1,   162,   163,  1835,    -1,    -1,
    1889,    -1,  1891,    -1,    -1,   941,    -1,    -1,    -1,   176,
     942,    -1,    -1,    -1,    -1,    19,    20,    -1,    -1,   951,
      -1,   957,    -1,   190,  1862,    -1,    30,    -1,    81,    -1,
      83,    84,    -1,    -1,   970,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1245,    -1,    81,   978,    -1,    -1,   981,
     103,  1889,    -1,  1891,    -1,    -1,    -1,  1946,  1260,  1261,
      -1,    -1,    -1,   999,    -1,    -1,    -1,   103,    -1,  1907,
    1959,  1003,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1920,    -1,  1016,    -1,   139,   140,   141,   142,
     143,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,  1937,
      -1,    -1,    -1,   139,   140,   141,   142,   143,  1946,    -1,
      -1,    -1,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,  1959,    -1,    -1,    -1,    -1,    -1,  1059,  1060,   165,
      -1,   167,   168,   169,   170,   171,   172,  1073,    -1,    -1,
      -1,  1077,   195,  1079,    -1,    -1,   199,    81,   201,  1351,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,   195,
     196,    -1,  1094,  1095,  1096,  1097,  1098,  1099,  1100,   103,
      -1,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,  1130,    -1,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1149,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    -1,   161,    -1,   233,
     164,    81,    -1,   167,   168,    -1,   170,   171,   172,    -1,
     174,    -1,    -1,    -1,    -1,    -1,  1182,   103,  1184,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   195,  1194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1211,    -1,    -1,  1214,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    81,    -1,  1231,
    1232,  1233,  1234,  1235,  1236,    -1,    -1,  1239,    -1,  1241,
      -1,   167,   168,  1245,   170,   171,   172,   167,   168,   103,
     170,   171,   172,    -1,    -1,    -1,    -1,    -1,  1260,  1261,
      -1,  1263,    -1,    -1,    -1,    -1,    -1,  1273,    -1,   195,
      -1,    -1,    -1,  1279,    -1,   195,   196,    -1,    -1,    -1,
    1282,    -1,  1284,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,   368,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   377,    -1,    -1,  1308,    -1,    -1,    -1,
     384,    -1,    -1,   167,   168,    -1,   170,   171,   172,    -1,
      -1,    -1,   396,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,   407,    -1,    -1,    -1,    -1,  1344,  1345,
      -1,   195,   196,    -1,    -1,    -1,  1348,    30,    31,  1351,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1399,    -1,  1401,
      -1,    70,    -1,    -1,    -1,    -1,    -1,   481,    -1,    78,
      79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   580,   103,    -1,    -1,    -1,  1444,    -1,
    1446,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   523,
    1452,    -1,  1454,    -1,  1456,    -1,    -1,    -1,    -1,  1461,
    1462,    -1,    81,  1465,    -1,  1467,   613,    -1,  1470,   138,
     139,   140,   141,   142,   143,    -1,    -1,  1483,    31,    -1,
      -1,    -1,  1484,    -1,   103,  1487,    -1,   156,  1494,    -1,
      -1,    -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,   580,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,   185,   200,    -1,    -1,
     139,   140,   141,   142,   143,    -1,   195,    -1,    81,    -1,
      -1,    -1,    -1,    -1,  1536,    81,  1538,    -1,  1540,   613,
      -1,    -1,    -1,  1545,    -1,  1547,   165,    -1,   167,   168,
     103,   170,   171,   172,    -1,  1561,    -1,   103,   111,    -1,
      -1,    -1,  1564,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,  1576,  1577,   195,    -1,  1584,    -1,
      -1,    -1,    -1,    -1,  1586,   138,   139,   140,   141,   142,
     143,   144,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,  1608,    -1,  1610,    -1,    -1,    31,   161,    -1,
    1616,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,   167,   168,    -1,   170,   171,   172,   701,    -1,    -1,
      -1,    -1,   185,    -1,    -1,    -1,    -1,  1639,   191,    -1,
      -1,    -1,   195,   196,    68,    -1,    -1,    -1,    -1,   195,
      -1,    -1,    -1,    -1,  1656,  1657,  1658,    81,    -1,    -1,
      -1,  1663,    -1,  1665,    -1,  1671,    -1,    -1,    -1,    -1,
      -1,  1673,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,   831,    -1,    -1,   761,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     847,   848,   849,   850,   851,    -1,    -1,    -1,    -1,    31,
      -1,    -1,   859,    -1,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   802,    -1,
     804,    -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
     174,    -1,    -1,    -1,    -1,    -1,   830,   831,    31,    81,
      -1,   185,    -1,    -1,    -1,    -1,    -1,  1773,    -1,    91,
      -1,   195,   196,   847,   848,   849,   850,   851,    -1,  1785,
      -1,   103,    -1,  1789,    -1,   859,    -1,    -1,    -1,    -1,
      -1,  1793,  1794,    -1,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,  1808,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,  1814,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,  1825,    -1,    -1,    -1,   901,    -1,  1831,
     103,   978,    -1,  1835,    -1,    -1,    -1,    -1,   111,   161,
      -1,    -1,   164,   917,   918,   167,   168,    -1,   170,   171,
     172,    -1,   174,    -1,    -1,    -1,  1862,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,    -1,   195,    -1,    -1,    -1,   951,    -1,    -1,
      -1,    -1,    -1,  1889,    -1,  1891,    -1,    -1,   161,    -1,
      -1,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,    -1,   978,  1907,    -1,    -1,    -1,    -1,
      -1,    -1,   185,    -1,    -1,    -1,    -1,    -1,  1920,    -1,
      -1,    -1,   195,   196,    -1,    -1,    -1,    -1,    -1,  1003,
      -1,    -1,    -1,    -1,    -1,  1937,    -1,    -1,    -1,    -1,
    1946,    -1,    -1,    -1,    -1,    -1,    -1,  1094,  1095,    -1,
      -1,  1098,    -1,  1959,    -1,    -1,  1103,  1104,  1105,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,
    1127,  1128,  1129,  1130,    -1,  1059,  1060,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1094,  1095,  1096,  1097,  1098,  1099,  1100,    -1,    -1,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1124,  1125,  1126,  1127,  1128,  1129,  1130,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1149,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1263,    69,    -1,    -1,
    1194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,  1282,    -1,  1284,   111,   112,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,  1308,    57,    -1,    -1,  1239,   139,   140,   141,   142,
     143,  1245,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1260,  1261,   161,  1263,
      -1,   164,    -1,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1282,    -1,
    1284,    -1,    -1,   186,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,  1308,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   200,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,  1351,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,  1452,    -1,  1454,    -1,  1456,
      -1,    -1,    -1,    -1,  1461,    -1,    -1,    -1,  1465,    -1,
    1467,    30,    31,  1470,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1452,    -1,
    1454,    -1,  1456,    -1,    -1,    -1,    -1,  1461,  1462,    -1,
      -1,  1465,    81,  1467,    -1,    -1,  1470,    -1,    10,    11,
      12,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,   200,  1564,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   161,  1547,    -1,   164,    -1,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,    -1,    -1,    -1,
    1564,   200,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,   195,    -1,    -1,  1656,
    1657,  1658,    -1,    -1,    -1,    -1,  1663,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,  1656,  1657,  1658,    -1,    -1,    -1,   200,  1663,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,  1672,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,  1814,    -1,   179,
      -1,    -1,    10,    11,    12,   185,    -1,    -1,  1825,    -1,
     190,   191,   192,    -1,  1831,   195,   196,    -1,  1835,    -1,
      -1,   201,   202,    31,   204,   205,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1814,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
       7,  1825,    -1,    -1,    -1,    -1,    13,  1831,    -1,    -1,
    1907,  1835,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1864,    48,    -1,    50,    51,    -1,    -1,    -1,   136,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      87,    -1,    -1,  1907,    91,    92,    93,    94,    -1,    96,
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
     104,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
     114,    -1,   116,   117,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,   129,   130,   131,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,   153,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,   173,
      -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,
      -1,   185,   186,    -1,   188,    -1,   190,   191,   192,    -1,
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
     111,   112,   113,   114,    -1,   116,   117,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,   127,   128,   129,   130,
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,   153,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,   173,    -1,    -1,   176,    -1,    -1,   179,    -1,
      -1,    -1,    -1,    -1,   185,   186,    -1,   188,    -1,   190,
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
     199,   200,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    95,
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
      93,    94,    -1,    96,    -1,    98,    -1,   100,   101,    -1,
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
      94,    -1,    96,    -1,    98,    99,   100,    -1,    -1,   103,
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
      91,    92,    93,    94,    -1,    96,    97,    98,    -1,   100,
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
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,    -1,
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
     192,    -1,    -1,   195,   196,    -1,   198,    -1,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
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
     119,   120,   121,   122,   123,   124,    -1,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,   174,    -1,   176,    -1,    -1,
     179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,    -1,
      -1,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     196,    -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
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
      -1,   108,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,
      -1,    -1,   179,    -1,    -1,    -1,    -1,   200,   185,    -1,
      -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,
      -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    50,    51,    -1,    -1,
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
     191,   192,    -1,    -1,   195,   196,    -1,   198,    11,    12,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    69,    -1,    56,    -1,
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
     198,    -1,    -1,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,
      -1,    -1,   200,   185,    -1,    -1,    -1,    -1,   190,   191,
     192,    -1,    -1,   195,   196,   197,    -1,    -1,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    32,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
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
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
     176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,
      -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,
     196,    -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,
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
      -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,
     200,   185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,
      -1,   195,   196,    10,    11,    12,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,     3,     4,   176,     6,     7,   179,    -1,
      10,    11,    12,    13,   185,    -1,    -1,    -1,    -1,   190,
     191,   192,    -1,    -1,   195,   196,    -1,    -1,    28,    29,
     201,   202,    -1,   204,   205,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    81,   128,    -1,
     130,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,   161,    10,    11,    12,    13,    -1,   167,   168,    -1,
     170,   171,   172,   173,    -1,   175,    -1,    -1,   178,    -1,
      28,    29,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,   199,
      -1,   201,    -1,    -1,    -1,    -1,    -1,   161,    -1,    57,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    83,    84,    -1,    -1,    -1,
      -1,   195,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    81,
     128,    -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,   161,    10,    11,    12,    13,    -1,   167,
     168,    -1,   170,   171,   172,   173,    -1,   175,    -1,    -1,
     178,    -1,    28,    29,    -1,    31,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,
      -1,   199,    -1,   201,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,   164,    -1,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    68,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   195,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,    -1,    -1,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    83,    -1,
      85,    -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,   173,   103,   175,
      -1,    -1,   178,     3,     4,    -1,     6,     7,    -1,   185,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   195,
     196,    -1,    -1,    -1,   200,    -1,    -1,    -1,    28,    29,
      -1,    31,    -1,    -1,   139,   140,   141,   142,   143,    -1,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    57,    57,    -1,
      -1,    -1,   167,   168,    -1,   170,   171,   172,    68,    -1,
      69,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     195,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,    -1,
      -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,   173,   103,   175,    -1,    -1,   178,     3,
       4,    -1,     6,     7,    -1,   185,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,
     200,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,    -1,
     139,   140,   141,   142,   143,    -1,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    57,    57,   164,    -1,    -1,   167,   168,
      -1,   170,   171,   172,    68,    -1,    69,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,    -1,   130,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,   173,
      -1,   175,    -1,    -1,   178,     3,     4,    -1,     6,     7,
      -1,   185,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    29,    -1,    31,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    57,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    69,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   161,    -1,    -1,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,   173,    -1,   175,    -1,    -1,
     178,    -1,     3,     4,    -1,     6,     7,   185,   186,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,   195,   196,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,    -1,
      31,    -1,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    57,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    69,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,    -1,    -1,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,   173,    -1,   175,    -1,    -1,   178,     3,     4,
       5,     6,     7,    -1,   185,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   161,   162,   163,    -1,
      81,    -1,   167,   168,    -1,   170,   171,   172,   173,    -1,
     175,   176,    -1,   178,    -1,    -1,    -1,    -1,    -1,    -1,
     185,   186,   103,   188,    -1,   190,   191,    -1,     3,     4,
     195,     6,     7,    -1,    -1,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    29,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     161,    -1,    57,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   195,    -1,    -1,    92,    93,    94,
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
     123,   124,   125,   126,    -1,   128,    -1,   130,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,   161,    10,
      11,    12,    13,    -1,   167,   168,    -1,   170,   171,   172,
     173,    -1,   175,    -1,    -1,   178,    -1,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,    -1,    -1,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     161,    -1,    -1,    -1,    -1,    -1,   167,   168,    -1,   170,
     171,   172,   173,    -1,   175,    -1,    -1,   178,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    32,    33,
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
      48,    49,    50,    51,    52,    53,    54,    55,   200,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,   198,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,   198,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     198,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     198,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   198,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,   198,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    69,    81,    -1,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   197,    69,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   197,   161,    -1,    -1,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,   174,    -1,    -1,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,   185,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,   195,   196,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    -1,
      -1,   197,   161,    -1,    -1,   164,   165,    -1,   167,   168,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    69,
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
      -1,    -1,    -1,    30,    31,   136,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
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
      51,    52,    53,    54,    55,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    69,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,   136,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,   176,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,   190,    91,
      -1,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,   176,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,   190,    91,
      81,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,   119,    -1,    -1,
     111,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,   139,   140,
     141,   142,   143,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,   164,   176,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,    10,    11,    12,   190,    -1,
      -1,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,
      -1,    -1,    -1,    -1,   195,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    32,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    12,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69
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
      83,   348,   197,   196,   493,   195,   490,   350,   493,   296,
     200,   197,   429,   136,   136,    32,   230,   275,   276,   228,
     417,   417,   417,   200,   198,   198,   417,   408,   305,   506,
     313,   314,   416,   310,    14,    32,    51,   315,   318,     9,
      36,   197,    31,    50,    53,    14,     9,   198,   215,   484,
     331,    14,   506,   245,   198,    14,   348,    38,    83,   396,
     199,   491,   492,   506,   198,   199,   323,   493,   490,   200,
     493,   429,   429,   228,    99,   241,   200,   213,   226,   306,
     307,   308,     9,   423,     9,   423,   200,   417,   406,   406,
      68,   316,   321,   321,    31,    50,    53,   417,    83,   181,
     196,   198,   417,   484,   417,    83,     9,   424,   228,     9,
     424,    14,   494,   228,   199,   323,   323,    97,   198,   115,
     237,   160,   102,   506,   182,   416,   172,    14,   495,   303,
     196,    38,    83,   197,   200,   492,   506,   200,   228,   198,
     196,   178,   244,   213,   326,   327,   182,   417,   182,   287,
     288,   442,   304,    83,   200,   408,   242,   175,   213,   198,
     197,     9,   424,   122,   123,   124,   329,   330,   287,    83,
     272,   198,   493,   442,   507,   197,   197,   198,   490,   329,
      38,    83,   174,   493,   199,   198,   199,   324,   507,    83,
     174,    14,    83,   490,   228,   228,    38,    83,   174,    14,
      83,   348,   324,   200,   200,    83,   174,    14,    83,   348,
      14,    83,   348,   348
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
       0,     0,     4,     2,     0,     1,     0,     1,     0,    10,
       0,    11,     0,    11,     0,     9,     0,    10,     0,     8,
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
#line 6852 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 754 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 6860 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 761 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 6866 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 762 "hphp.y" /* yacc.c:1646  */
    { }
#line 6872 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 765 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 6878 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 766 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6884 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 767 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 768 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6896 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 769 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6902 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 770 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 6908 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 771 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 6916 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 6923 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 6929 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6935 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 6941 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 779 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6947 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6955 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 784 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6964 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 789 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6973 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 794 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6980 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 797 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6987 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 800 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6995 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 804 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7003 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 808 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7011 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 812 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7019 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 816 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7027 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 819 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7034 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 824 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7040 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 825 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7046 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 826 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7052 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 827 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7058 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7064 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 829 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7070 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 830 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7076 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7082 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7088 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7094 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7100 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7106 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7112 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 915 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7118 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 917 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7124 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 922 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7130 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 923 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7137 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 929 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7143 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 933 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7149 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 934 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7155 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 936 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7161 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 938 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7167 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 943 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7173 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 944 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7180 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 950 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7186 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 954 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7193 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 956 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7200 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 958 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7207 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 963 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7213 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 965 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7219 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 968 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7225 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 970 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7231 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 971 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7237 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 976 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7246 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 983 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7255 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 991 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7262 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 994 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7269 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 1000 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7275 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1001 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7281 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7287 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1005 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7293 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7299 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7305 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1010 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7311 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1014 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7317 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1019 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7323 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1020 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7330 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1022 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7338 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1026 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7345 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1029 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7353 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1033 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7360 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1035 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7368 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7375 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1040 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7383 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7389 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1044 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7395 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1045 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7401 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1046 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7407 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1047 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7413 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1048 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7419 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1049 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7425 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1050 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7431 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1051 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7437 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1052 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7443 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1053 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7449 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1054 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7455 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7461 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1056 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7467 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1057 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7474 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1061 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7481 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1063 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7489 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1068 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7496 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1070 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 7504 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1074 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 7512 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7518 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 7524 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1087 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7530 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1088 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 7536 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1089 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 7542 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1090 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 7551 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1094 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7557 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1095 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7563 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1096 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7569 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1097 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7575 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1098 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7581 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1099 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7587 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1100 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7593 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1101 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7599 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1102 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7605 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1103 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 7611 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1104 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7617 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1105 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 7627 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1113 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);}
#line 7633 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1114 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7639 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1123 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 7645 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1124 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7651 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1128 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7658 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1130 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7666 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1136 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7672 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1137 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7678 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1141 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 7684 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1142 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7690 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1146 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 7696 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7705 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1159 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7714 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1166 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7723 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1173 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7732 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1180 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7741 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1186 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7750 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1194 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7757 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1198 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 7763 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1202 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7770 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1206 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 7776 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 1212 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7783 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 1215 "hphp.y" /* yacc.c:1646  */
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
#line 7801 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 1230 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7808 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1233 "hphp.y" /* yacc.c:1646  */
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
#line 7826 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1247 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7833 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1250 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7841 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1255 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7848 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1258 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7856 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1264 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 7862 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1267 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 7868 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1271 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7875 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1274 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7886 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1282 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7893 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1285 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7904 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1293 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7910 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1294 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 7917 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1298 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7923 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1301 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7929 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1304 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 7935 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1305 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 7941 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1306 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 7949 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1309 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 7955 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1310 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 7961 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1314 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7967 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1315 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7973 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1318 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7979 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1319 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7985 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1322 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7991 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1323 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7997 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1326 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8003 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1328 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8009 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1331 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8015 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1333 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8021 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1337 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8027 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1338 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8033 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1341 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8039 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1342 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8045 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1343 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8051 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1347 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8057 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1349 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8063 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1352 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8069 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1354 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8075 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8081 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1359 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8087 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1362 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8093 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1364 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8099 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1368 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8105 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1370 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8112 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1375 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8118 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1376 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8124 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1377 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8130 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1378 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8136 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1383 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8142 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1385 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8148 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1386 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8154 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1389 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8160 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1390 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8166 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1395 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8172 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1396 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8178 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1401 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8184 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1402 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8190 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1405 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8196 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1406 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8202 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1409 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8208 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1410 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8214 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1418 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8221 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1424 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8228 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1430 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8236 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1434 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8242 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1438 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8249 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1443 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8256 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1448 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8264 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1451 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8270 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1457 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8277 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1461 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8284 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8291 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1471 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8298 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1476 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8305 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1481 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8312 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1487 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8319 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1493 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8326 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1501 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8333 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1506 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8340 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1511 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8348 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1515 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8354 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1518 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8361 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1522 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 8368 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1526 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 8376 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1529 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8382 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1534 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8389 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1537 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8396 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1541 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8403 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1545 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8410 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1549 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8417 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1553 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8424 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1558 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8431 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1563 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8438 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1569 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8444 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1570 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8450 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1573 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,false);}
#line 8456 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1574 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),true,false);}
#line 8462 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1575 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,true);}
#line 8468 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1577 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),false, false);}
#line 8474 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1579 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),false,true);}
#line 8480 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1581 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),true, false);}
#line 8486 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1585 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8492 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1586 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 8498 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1589 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8504 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1590 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8510 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1591 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 8516 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1595 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 8522 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1597 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 8528 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1598 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 8534 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1599 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 8540 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1604 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8546 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1605 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8552 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1608 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8559 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1613 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 8565 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1619 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8571 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1620 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8577 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1623 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 8583 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1624 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 8590 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1627 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 8596 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1628 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 8603 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1630 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8610 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1633 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 8617 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1635 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8623 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1638 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8631 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1645 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8640 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1652 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8648 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1659 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8657 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1664 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 8663 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1666 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8669 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1668 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8675 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1670 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 8681 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1672 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 8687 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1673 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 8694 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1676 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 8700 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1679 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8706 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1680 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8712 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1681 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8718 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1687 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 8724 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1692 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 8731 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1695 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 8739 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1702 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 8745 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1703 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 8752 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1708 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 8759 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1711 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 8765 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1718 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 8772 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8778 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1724 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8784 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1729 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8790 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1731 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8796 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1733 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8802 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1734 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 8813 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1740 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 8819 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1742 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 8825 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1743 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 8831 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1747 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,0,(yyvsp[0]),0);}
#line 8837 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1749 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),0,(yyvsp[0]),0);}
#line 8843 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1754 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8849 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1757 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8855 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1758 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 8861 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1762 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 8867 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1763 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 8873 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1767 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 8880 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1770 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 8887 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1775 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 8894 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1780 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 8900 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1781 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 8907 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1783 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 8913 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1787 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 8919 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1788 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 8925 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1789 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 8931 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1790 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 8937 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1794 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8943 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1795 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 8949 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1796 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 8955 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1797 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 8961 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1798 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 8967 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1800 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 8973 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1802 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 8979 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1806 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 8987 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1809 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 8993 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1810 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 8999 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1814 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9005 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1815 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9011 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1819 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9017 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1820 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9023 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1823 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9029 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1824 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9035 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1827 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9041 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1828 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9047 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1831 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9053 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1833 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9059 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9065 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9071 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9077 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9083 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1840 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9089 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9095 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1842 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9101 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9107 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1847 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9113 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1850 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9119 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1851 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9125 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1852 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9131 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1856 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9137 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1858 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9143 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1859 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9149 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1860 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9155 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1864 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9161 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1866 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9167 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1870 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9173 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1872 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9179 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1876 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9187 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1880 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9194 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1884 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9200 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1888 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9206 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1890 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9212 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1891 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9218 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1892 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9224 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1893 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9230 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1894 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9236 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1897 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9242 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1901 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9248 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1902 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9254 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1906 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9260 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1907 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9266 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 1911 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9272 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 1912 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9278 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 1913 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9284 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 1914 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9290 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 1918 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9296 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 1923 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9302 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 1927 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 9308 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 1931 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9314 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 1935 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 9320 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 1939 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9326 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 1944 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9332 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 1948 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9338 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 1949 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9344 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9350 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 1951 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9356 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 1952 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9362 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 9368 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 1958 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 9374 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 1959 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 9380 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 9386 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 9392 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 9398 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 9404 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 1966 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 9410 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 1967 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 9416 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 1968 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 9422 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 1969 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 9428 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 1970 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 9434 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 1971 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 9440 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 1972 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 9446 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 9452 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 1974 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 9458 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 1975 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 9464 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 9470 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 1977 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 9476 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 1978 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 9482 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 1979 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 9488 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 1980 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 9494 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 1981 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 9500 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 9506 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 1983 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 9512 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 1984 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 9518 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 1985 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 9524 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 9530 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 1987 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 9536 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 1988 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 9542 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 1989 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 9548 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 1990 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 9554 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 9560 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 1992 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 9566 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 1993 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 9572 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 9578 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 1995 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 9584 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 1996 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 9590 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 1997 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 9596 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 1998 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 9602 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 9608 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 9614 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2001 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 9620 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2002 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 9626 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 9632 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 9638 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 9644 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2006 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 9651 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2008 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 9657 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2009 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 9664 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2011 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 9670 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 9676 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9682 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 9688 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2016 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 9694 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 9700 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9706 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2019 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 9712 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2020 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 9718 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2021 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 9724 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2022 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 9730 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2023 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 9736 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2024 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 9742 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2025 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 9748 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2026 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 9754 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 9760 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2028 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9766 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2029 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9772 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2030 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9778 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2031 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9784 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2032 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9790 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2033 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9796 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2034 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9802 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2035 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9808 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2036 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 9814 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2037 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 9820 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2038 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9826 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2045 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 9832 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2046 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9838 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2051 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9847 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2057 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9858 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2065 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9867 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2071 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9878 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2081 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 9891 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2089 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         (yyvsp[-3]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-3]), nullptr, (yyvsp[-3]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-3]),
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9905 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2099 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9915 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2107 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         (yyvsp[-6]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-6]), nullptr, (yyvsp[-6]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-6]),
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9929 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2117 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9939 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2123 "hphp.y" /* yacc.c:1646  */
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
#line 9955 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2134 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 9968 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2142 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9980 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2149 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9990 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2157 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10002 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2167 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10008 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2168 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10014 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2170 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10020 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2174 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10027 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2176 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10033 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2183 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10039 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2186 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10045 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2193 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10051 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2196 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10057 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2201 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10063 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2202 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10069 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2207 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10075 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10081 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2212 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_ARRAY);}
#line 10087 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2216 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10093 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2217 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10099 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2222 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10105 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2223 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10111 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2228 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10117 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2229 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10123 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2234 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10129 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2235 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10135 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2241 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10141 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2243 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10147 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2248 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10153 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2249 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10159 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2255 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10165 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2257 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10171 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2261 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10177 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2265 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10183 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2269 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10189 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2273 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10195 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2277 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10201 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2281 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10207 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2285 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10213 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2289 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10219 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2293 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10225 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2297 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10231 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2301 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10237 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2305 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10243 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2309 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10249 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2313 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10255 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2317 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10261 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2322 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10267 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2323 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10273 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2328 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10279 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2329 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10285 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2334 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10291 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2335 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10297 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2340 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10305 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2347 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10313 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2354 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10319 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2356 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10325 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2360 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10331 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2361 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10337 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2362 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10343 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2363 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10349 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2364 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10355 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2365 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10361 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2366 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10367 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2367 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10373 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2368 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10380 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2370 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10386 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2371 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10392 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2375 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10398 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2376 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 10404 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2377 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 10410 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2378 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 10416 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2385 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 10422 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2388 "hphp.y" /* yacc.c:1646  */
    { Token t1; _p->onArray(t1,(yyvsp[-1]));
                                         Token t2; _p->onArray(t2,(yyvsp[0]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[-1]),NULL,t1,0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-1]),t2,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),file,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),line,0,0);
                                         (yyval).setText("");}
#line 10436 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2399 "hphp.y" /* yacc.c:1646  */
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray((yyvsp[-2]),(yyvsp[-5]));
                                         _p->onArray((yyvsp[-1]),(yyvsp[-3]));
                                         _p->onCallParam((yyvsp[-4]),NULL,(yyvsp[-2]),0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-4]),(yyvsp[-1]),0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),file,0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),line,0,0);
                                         (yyval).setText((yyvsp[0]).text());}
#line 10450 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2410 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 10456 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2411 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 10462 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2416 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10468 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2417 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10474 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2420 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),0,(yyvsp[0]),0);}
#line 10480 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2421 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10486 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2424 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10493 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2428 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10501 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2431 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10507 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2434 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 10519 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2441 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10525 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2442 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10531 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2446 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10537 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2448 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 10543 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2450 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 10549 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10555 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2455 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10561 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2456 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10567 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2457 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10573 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2458 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10579 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10585 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2460 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10591 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10597 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2462 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10603 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2463 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10609 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10615 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2465 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10621 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2466 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10627 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2467 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10633 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2468 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10639 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2469 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10645 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2470 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10651 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2471 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10657 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2472 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10663 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2473 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10669 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2474 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10675 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2475 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10681 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2476 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10687 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2477 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10693 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2478 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10699 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2479 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10705 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2480 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10711 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2481 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10717 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2482 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10723 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10729 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2484 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10735 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10741 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10747 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10753 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10759 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10765 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10771 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2491 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10777 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2492 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10783 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2493 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10789 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2494 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10795 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2495 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10801 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2496 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10807 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2497 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10813 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2498 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10819 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2499 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10825 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2500 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10831 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2501 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10837 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10843 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2503 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10849 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2504 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10855 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2505 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10861 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2506 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10867 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2507 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10873 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2508 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10879 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2509 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10885 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2510 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10891 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2511 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10897 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2512 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10903 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2513 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10909 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2514 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10915 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2515 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10921 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2516 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10927 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10933 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2518 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10939 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2519 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10945 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2520 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10951 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2521 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10957 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2522 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10963 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2523 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10969 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2524 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10975 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2525 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10981 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2526 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10987 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2527 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10993 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2528 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10999 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2529 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11005 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2530 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11011 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2531 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11017 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2532 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11023 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2533 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11029 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2534 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11035 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2539 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11041 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2543 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11047 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2544 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11053 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2548 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11059 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2549 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11065 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2550 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11071 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2551 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11078 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2553 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11085 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2557 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11091 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2566 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11097 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2569 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11103 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2570 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11110 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2572 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11117 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2582 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11123 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2586 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11129 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2587 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11135 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2588 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11141 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2592 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11147 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2593 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11153 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2594 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11159 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2598 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11165 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2599 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11171 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2600 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11177 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2604 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11183 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2605 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11189 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2609 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11195 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2610 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11201 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2611 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11207 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2612 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11214 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2614 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11220 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2615 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11226 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2616 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 11232 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2617 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 11238 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2618 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 11244 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2619 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 11250 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2620 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 11256 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2621 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 11262 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2622 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 11268 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2625 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11274 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2627 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11280 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2631 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11286 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2632 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11292 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2634 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11298 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2635 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11304 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2637 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11310 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2638 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11316 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11322 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2640 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11328 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2641 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11334 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11340 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2643 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11346 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2644 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11352 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2645 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11358 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2647 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 11364 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2649 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 11370 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2651 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 11376 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2653 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 11382 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2655 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 11388 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2656 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 11394 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2657 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 11400 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2658 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 11406 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 11412 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2660 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 11418 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2661 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 11424 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2662 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 11430 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 11436 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 11442 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2665 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 11448 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2666 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 11454 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2667 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 11460 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 11466 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2669 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 11472 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11478 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2671 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11484 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2673 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 11490 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2675 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 11496 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2677 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 11502 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2679 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 11508 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2680 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 11514 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2682 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 11521 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2684 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 11527 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2687 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 11534 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2691 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 11540 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2694 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 11546 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2695 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 11552 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2699 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11558 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2700 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11564 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2706 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11570 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11576 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2713 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11582 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2717 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11588 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11594 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11600 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11606 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 11612 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2722 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 11618 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2724 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 11625 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11631 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11637 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11643 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2735 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11649 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 11655 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11661 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2745 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11667 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2747 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11673 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2749 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11679 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2750 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11685 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2754 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11691 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2755 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11697 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2756 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11703 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2759 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11709 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2761 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11715 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2764 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 11721 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2765 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 11727 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2766 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 11733 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2767 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11739 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2771 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 11746 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2774 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 11754 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2781 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11760 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2782 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11766 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2785 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 11774 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2788 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11780 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2789 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11786 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2790 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11792 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2792 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11798 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2793 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11804 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2795 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11810 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2796 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11816 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2797 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11822 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2798 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11828 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2799 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11834 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2800 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11840 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2805 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11846 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2806 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11852 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2811 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11858 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2812 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11864 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2817 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11870 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2819 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11876 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2821 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11882 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2822 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11888 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2826 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11894 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2827 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11900 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11906 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2833 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 11912 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2838 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11918 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2841 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11924 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2846 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11930 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2847 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11936 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2850 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11942 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2851 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 11949 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2858 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 11955 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2860 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 11961 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2863 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 11967 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2865 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11973 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2868 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11979 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2871 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11985 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2872 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11991 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2876 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 11997 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2877 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12003 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2881 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12009 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2882 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12015 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2883 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12021 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2887 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12027 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2889 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12033 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2897 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12039 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2898 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12045 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2902 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12051 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2904 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12057 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 2912 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12063 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 2913 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12069 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 2917 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12075 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 2919 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12081 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 2924 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12087 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 2926 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12093 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 2932 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12107 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 2943 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12121 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 2958 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12135 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 2970 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12149 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 2982 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12155 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 2983 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12161 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 2984 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12167 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 2985 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12173 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 2986 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12179 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 2987 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12185 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 2989 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12199 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3006 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12205 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3008 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12211 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3010 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12217 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3011 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12223 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3015 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12229 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3016 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12235 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12241 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12247 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3026 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12261 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3035 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12267 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3037 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12273 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3038 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12279 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3047 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12285 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3048 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12291 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3049 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12297 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3050 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12303 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3051 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12309 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3052 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12315 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3053 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12321 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3055 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12327 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3057 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12333 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3061 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12339 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3065 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12345 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3066 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12351 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3072 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12357 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3076 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12363 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3083 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 12369 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3092 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 12375 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3096 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 12381 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3100 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12387 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3103 "hphp.y" /* yacc.c:1646  */
    { _p->onIndirectRef((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 12393 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3109 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12399 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3110 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12405 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3111 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12411 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3115 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12417 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3116 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 12423 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3117 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 12429 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3124 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12435 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3125 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12441 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3130 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 12447 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3131 "hphp.y" /* yacc.c:1646  */
    { (yyval)++;}
#line 12453 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3136 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12459 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3137 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12465 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3138 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12471 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3141 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12485 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3152 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12491 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3153 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12497 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3157 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12503 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3158 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12509 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3161 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12523 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3170 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12529 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3174 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 12535 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3175 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 12541 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3177 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 12547 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3178 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 12553 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3179 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 12559 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3180 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 12565 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3185 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12571 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3186 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12577 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3190 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12583 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3191 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12589 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3192 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12595 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3193 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12601 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3196 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12607 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3198 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 12613 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3199 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12619 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3200 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 12625 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3205 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12631 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3206 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12637 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3210 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12643 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3211 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12649 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3212 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12655 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3213 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12661 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3218 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12667 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3219 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12673 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3224 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12679 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3226 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12685 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3228 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12691 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3229 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12697 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3233 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 12703 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3235 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 12709 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3236 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 12715 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3238 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 12722 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3243 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12728 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3245 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12734 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3247 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 12748 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3257 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 12754 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3259 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 12760 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3260 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12766 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3263 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 12772 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3264 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 12778 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3265 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 12784 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3269 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 12790 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3270 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 12796 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3271 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12802 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3272 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12808 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3273 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12814 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3274 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12820 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3275 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 12826 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3276 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 12832 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3277 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 12838 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3278 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 12844 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3279 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 12850 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3283 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12856 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3284 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12862 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3289 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12868 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3291 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12874 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3305 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12882 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3310 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 12890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3314 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12898 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3319 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 12906 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3325 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12912 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3326 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12918 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3330 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12924 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3331 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12930 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3337 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12936 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3341 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 12942 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3347 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12948 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3351 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 12955 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3358 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12961 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3359 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12967 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3363 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 12975 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3366 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 12982 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3372 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12988 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3377 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]); }
#line 12994 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3378 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13000 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3379 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13006 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1030:
#line 3380 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13012 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3401 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13018 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1038:
#line 3402 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13024 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3411 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13030 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3422 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13036 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3424 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13042 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3428 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13048 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3431 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13054 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3435 "hphp.y" /* yacc.c:1646  */
    {}
#line 13060 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3436 "hphp.y" /* yacc.c:1646  */
    {}
#line 13066 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3437 "hphp.y" /* yacc.c:1646  */
    {}
#line 13072 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3443 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13079 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3448 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13089 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3457 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13095 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3463 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13104 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3471 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13110 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3472 "hphp.y" /* yacc.c:1646  */
    { }
#line 13116 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3478 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13122 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3480 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13128 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3481 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13138 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3486 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13145 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3492 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("array"); }
#line 13152 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3497 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13158 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3502 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13166 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3506 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13172 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3511 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 13178 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3513 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 13184 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3519 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 13191 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3521 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 13199 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3524 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13205 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3525 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13213 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3528 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13221 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3531 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13227 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3534 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 13235 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3537 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13242 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3539 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 13251 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3545 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 13260 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1077:
#line 3551 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 13270 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1078:
#line 3559 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13276 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1079:
#line 3560 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13282 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;


#line 13286 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 3563 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}
