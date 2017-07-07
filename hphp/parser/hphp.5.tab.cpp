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
#define YYLAST   18292

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  206
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  302
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1077
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1982

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
     788,   793,   796,   799,   803,   807,   811,   816,   817,   818,
     819,   820,   821,   822,   823,   824,   825,   826,   827,   828,
     832,   833,   834,   835,   836,   837,   838,   839,   840,   841,
     842,   843,   844,   845,   846,   847,   848,   849,   850,   851,
     852,   853,   854,   855,   856,   857,   858,   859,   860,   861,
     862,   863,   864,   865,   866,   867,   868,   869,   870,   871,
     872,   873,   874,   875,   876,   877,   878,   879,   880,   881,
     882,   883,   884,   885,   886,   887,   888,   889,   890,   891,
     892,   893,   894,   898,   902,   903,   907,   908,   913,   915,
     920,   925,   926,   927,   929,   934,   936,   941,   946,   948,
     950,   955,   956,   960,   961,   963,   967,   974,   981,   985,
     991,   993,   996,   997,   998,   999,  1002,  1003,  1007,  1012,
    1012,  1018,  1018,  1025,  1024,  1030,  1030,  1035,  1036,  1037,
    1038,  1039,  1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,
    1048,  1049,  1053,  1051,  1060,  1058,  1065,  1075,  1069,  1079,
    1077,  1081,  1082,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
    1093,  1094,  1095,  1096,  1097,  1105,  1105,  1110,  1116,  1120,
    1120,  1128,  1129,  1133,  1134,  1138,  1144,  1142,  1157,  1154,
    1170,  1167,  1184,  1183,  1192,  1190,  1202,  1201,  1220,  1218,
    1237,  1236,  1245,  1243,  1254,  1254,  1261,  1260,  1272,  1270,
    1283,  1284,  1288,  1291,  1294,  1295,  1296,  1299,  1300,  1303,
    1305,  1308,  1309,  1312,  1313,  1316,  1317,  1321,  1322,  1327,
    1328,  1331,  1332,  1333,  1337,  1338,  1342,  1343,  1347,  1348,
    1352,  1353,  1358,  1359,  1365,  1366,  1367,  1368,  1371,  1374,
    1376,  1379,  1380,  1384,  1386,  1389,  1392,  1395,  1396,  1399,
    1400,  1404,  1410,  1416,  1423,  1425,  1430,  1435,  1441,  1445,
    1449,  1453,  1458,  1463,  1468,  1473,  1479,  1488,  1493,  1498,
    1504,  1506,  1510,  1514,  1519,  1523,  1526,  1529,  1533,  1537,
    1541,  1545,  1550,  1558,  1560,  1563,  1564,  1565,  1566,  1568,
    1570,  1575,  1576,  1579,  1580,  1581,  1585,  1586,  1588,  1589,
    1593,  1595,  1598,  1602,  1608,  1610,  1613,  1613,  1617,  1616,
    1620,  1622,  1625,  1628,  1626,  1643,  1639,  1654,  1656,  1658,
    1660,  1662,  1664,  1666,  1670,  1671,  1672,  1675,  1681,  1685,
    1691,  1694,  1699,  1701,  1706,  1711,  1715,  1716,  1720,  1721,
    1723,  1725,  1731,  1732,  1734,  1738,  1739,  1744,  1748,  1749,
    1753,  1754,  1758,  1760,  1766,  1771,  1772,  1774,  1778,  1779,
    1780,  1781,  1785,  1786,  1787,  1788,  1789,  1790,  1792,  1797,
    1800,  1801,  1805,  1806,  1810,  1811,  1814,  1815,  1818,  1819,
    1822,  1823,  1827,  1828,  1829,  1830,  1831,  1832,  1833,  1837,
    1838,  1841,  1842,  1843,  1846,  1848,  1850,  1851,  1854,  1856,
    1860,  1862,  1866,  1870,  1874,  1879,  1880,  1882,  1883,  1884,
    1885,  1888,  1892,  1893,  1897,  1898,  1902,  1903,  1904,  1905,
    1909,  1913,  1918,  1922,  1926,  1930,  1934,  1939,  1940,  1941,
    1942,  1943,  1947,  1949,  1950,  1951,  1954,  1955,  1956,  1957,
    1958,  1959,  1960,  1961,  1962,  1963,  1964,  1965,  1966,  1967,
    1968,  1969,  1970,  1971,  1972,  1973,  1974,  1975,  1976,  1977,
    1978,  1979,  1980,  1981,  1982,  1983,  1984,  1985,  1986,  1987,
    1988,  1989,  1990,  1991,  1992,  1993,  1994,  1995,  1996,  1997,
    1999,  2000,  2002,  2003,  2005,  2006,  2007,  2008,  2009,  2010,
    2011,  2012,  2013,  2014,  2015,  2016,  2017,  2018,  2019,  2020,
    2021,  2022,  2023,  2024,  2025,  2026,  2027,  2028,  2029,  2033,
    2037,  2042,  2041,  2056,  2054,  2072,  2071,  2090,  2089,  2108,
    2107,  2125,  2125,  2140,  2140,  2158,  2159,  2160,  2165,  2167,
    2171,  2175,  2181,  2185,  2191,  2193,  2197,  2199,  2203,  2207,
    2208,  2212,  2214,  2218,  2220,  2224,  2226,  2230,  2233,  2238,
    2240,  2244,  2247,  2252,  2256,  2260,  2264,  2268,  2272,  2276,
    2280,  2284,  2288,  2292,  2296,  2300,  2304,  2308,  2312,  2314,
    2318,  2320,  2324,  2326,  2330,  2337,  2344,  2346,  2351,  2352,
    2353,  2354,  2355,  2356,  2357,  2358,  2359,  2361,  2362,  2366,
    2367,  2368,  2369,  2373,  2379,  2388,  2401,  2402,  2405,  2408,
    2411,  2412,  2415,  2419,  2422,  2425,  2432,  2433,  2437,  2438,
    2440,  2445,  2446,  2447,  2448,  2449,  2450,  2451,  2452,  2453,
    2454,  2455,  2456,  2457,  2458,  2459,  2460,  2461,  2462,  2463,
    2464,  2465,  2466,  2467,  2468,  2469,  2470,  2471,  2472,  2473,
    2474,  2475,  2476,  2477,  2478,  2479,  2480,  2481,  2482,  2483,
    2484,  2485,  2486,  2487,  2488,  2489,  2490,  2491,  2492,  2493,
    2494,  2495,  2496,  2497,  2498,  2499,  2500,  2501,  2502,  2503,
    2504,  2505,  2506,  2507,  2508,  2509,  2510,  2511,  2512,  2513,
    2514,  2515,  2516,  2517,  2518,  2519,  2520,  2521,  2522,  2523,
    2524,  2525,  2529,  2534,  2535,  2539,  2540,  2541,  2542,  2544,
    2548,  2549,  2560,  2561,  2563,  2565,  2577,  2578,  2579,  2583,
    2584,  2585,  2589,  2590,  2591,  2594,  2596,  2600,  2601,  2602,
    2603,  2605,  2606,  2607,  2608,  2609,  2610,  2611,  2612,  2613,
    2614,  2617,  2622,  2623,  2624,  2626,  2627,  2629,  2630,  2631,
    2632,  2633,  2634,  2635,  2636,  2637,  2639,  2641,  2643,  2645,
    2647,  2648,  2649,  2650,  2651,  2652,  2653,  2654,  2655,  2656,
    2657,  2658,  2659,  2660,  2661,  2662,  2663,  2665,  2667,  2669,
    2671,  2672,  2675,  2676,  2680,  2684,  2686,  2690,  2691,  2695,
    2701,  2704,  2708,  2709,  2710,  2711,  2712,  2713,  2714,  2719,
    2721,  2725,  2726,  2729,  2730,  2734,  2737,  2739,  2741,  2745,
    2746,  2747,  2748,  2751,  2755,  2756,  2757,  2758,  2762,  2764,
    2771,  2772,  2773,  2774,  2779,  2780,  2781,  2782,  2784,  2785,
    2787,  2788,  2789,  2790,  2791,  2795,  2797,  2801,  2803,  2806,
    2809,  2811,  2813,  2816,  2818,  2822,  2824,  2827,  2830,  2836,
    2838,  2841,  2842,  2847,  2850,  2854,  2854,  2859,  2862,  2863,
    2867,  2868,  2872,  2873,  2874,  2878,  2880,  2888,  2889,  2893,
    2895,  2903,  2904,  2908,  2909,  2914,  2916,  2921,  2932,  2946,
    2958,  2973,  2974,  2975,  2976,  2977,  2978,  2979,  2989,  2998,
    3000,  3002,  3006,  3007,  3008,  3009,  3010,  3026,  3027,  3029,
    3038,  3039,  3040,  3041,  3042,  3043,  3044,  3045,  3047,  3052,
    3056,  3057,  3061,  3064,  3071,  3075,  3084,  3091,  3093,  3099,
    3101,  3102,  3106,  3107,  3108,  3115,  3116,  3121,  3122,  3127,
    3128,  3129,  3130,  3141,  3144,  3147,  3148,  3149,  3150,  3161,
    3165,  3166,  3167,  3169,  3170,  3171,  3175,  3177,  3180,  3182,
    3183,  3184,  3185,  3188,  3190,  3191,  3195,  3197,  3200,  3202,
    3203,  3204,  3208,  3210,  3213,  3216,  3218,  3220,  3224,  3225,
    3227,  3228,  3234,  3235,  3237,  3247,  3249,  3251,  3254,  3255,
    3256,  3260,  3261,  3262,  3263,  3264,  3265,  3266,  3267,  3268,
    3269,  3270,  3274,  3275,  3279,  3281,  3289,  3291,  3295,  3299,
    3304,  3308,  3316,  3317,  3321,  3322,  3328,  3329,  3338,  3339,
    3347,  3350,  3354,  3357,  3362,  3367,  3369,  3370,  3371,  3374,
    3376,  3382,  3383,  3387,  3388,  3392,  3393,  3397,  3398,  3401,
    3406,  3407,  3411,  3414,  3416,  3420,  3426,  3427,  3428,  3432,
    3436,  3444,  3449,  3461,  3463,  3467,  3470,  3472,  3477,  3482,
    3488,  3491,  3496,  3501,  3503,  3510,  3512,  3515,  3516,  3519,
    3522,  3523,  3528,  3530,  3534,  3540,  3550,  3551
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

#define YYPACT_NINF -1623

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1623)))

#define YYTABLE_NINF -1061

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1061)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1623,   166, -1623, -1623,  5845, 13762, 13762,   -19, 13762, 13762,
   13762, 13762,  5087, 13762, -1623, 13762, 13762, 13762, 13762, 16907,
   16907, 13762, 13762, 13762, 13762, 13762, 13762, 13762, 13762, 11732,
   17620, 13762,   179,   206, -1623, -1623, -1623,   121, -1623,   284,
   -1623, -1623, -1623,   253, 13762, -1623,   206,   225,   244,   249,
   -1623,   206, 11935, 14334, 12138, -1623, 14792, 10717,   307, 13762,
   16073,   170,    76,   612,    58, -1623, -1623, -1623,   345,   349,
     353,   356, -1623, 14334,   380,   384,   577,   582,   587,   601,
     638, -1623, -1623, -1623, -1623, -1623, 13762,   464,  2306, -1623,
   -1623, 14334, -1623, -1623, -1623, -1623, 14334, -1623, 14334, -1623,
     523,   515, 14334, 14334, -1623,   333, -1623, -1623, 12341, -1623,
   -1623,   530,   477,   610,   610, -1623,   707,   625,   219,   584,
   -1623,    99, -1623,   764, -1623, -1623, -1623, -1623,  4761,   562,
   -1623, -1623,   608,   628,   637,   641,   644,   654,   662,   664,
    5014, -1623, -1623, -1623, -1623,   167,   752,   795,   797,   801,
     802, -1623,   803,   804, -1623,   142,   676, -1623,   716,    -7,
   -1623,  1189,   135, -1623, -1623,  1564,    56,   681,   174, -1623,
     138,    62,   684,   173, -1623,    92, -1623,   811, -1623,   723,
   -1623, -1623,   689,   722, -1623, 13762, -1623,   764,   562, 18040,
    3648, 18040, 13762, 18040, 18040, 15322, 15322,   690, 16426, 18040,
     840, 14334,   822,   822,   618,   822, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623, -1623, -1623,    49, 13762,   711, -1623, -1623,
     733,   699,    67,   700,    67,   822,   822,   822,   822,   822,
     822,   822,   822, 16907, 17076,   694,   889,   723, -1623, 13762,
     711, -1623,   739, -1623,   740,   705, -1623,   143, -1623, -1623,
   -1623,    67,    56, -1623, 12544, -1623, -1623, 13762,  9296,   895,
     103, 18040, 10311, -1623, 13762, 13762, 14334, -1623, -1623, 11919,
     706, -1623, 12528, -1623, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623,  4362, -1623,  4362, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623,    88,    96,   722, -1623, -1623, -1623, -1623,
     709,  1759,    97, -1623, -1623,   746,   893, -1623,   748, 15511,
   -1623,   713,   714, 13746, -1623,    29, 15984,  4561,  4561, 14182,
   14334, 14182,   715,   907,   720, -1623,    60, -1623, 16495,   105,
   -1623,   905,   108,   790, -1623,   791, -1623, 16907, 13762, 13762,
     725,   742, -1623, -1623, 16598, 11732, 13762, 13762, 13762, 13762,
   13762,   109,   494,   361, -1623, 13965, 16907,   519, -1623, 14334,
   -1623,   -13,   625, -1623, -1623, -1623, -1623, 17720,   911,   825,
   -1623, -1623, -1623,    61, 13762,   735,   738, 18040,   743,  2182,
     744,  6048, 13762,   364,   728,   624,   364,   346,   475, -1623,
   14334,  4362,   747, 10920, 14792, -1623, -1623,  2083, -1623, -1623,
   -1623, -1623, -1623,   764, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623, 13762, 13762, 13762, 13762, 12747, 13762, 13762,
   13762, 13762, 13762, 13762, 13762, 13762, 13762, 13762, 13762, 13762,
   13762, 13762, 13762, 13762, 13762, 13762, 13762, 13762, 13762, 13762,
   13762, 17820, 13762, -1623, 13762, 13762, 13762, 14136, 14334, 14334,
   14334, 14334, 14334,  4761,   827,   471, 10514, 13762, 13762, 13762,
   13762, 13762, 13762, 13762, 13762, 13762, 13762, 13762, 13762, -1623,
   -1623, -1623, -1623,  1715, 13762, 13762, -1623, 10920, 10920, 13762,
   13762,   530,   148, 16598,   749,   764, 12950, 16032, -1623, 13762,
   -1623,   751,   927,   787,   753,   754, 14288,    67, 13153, -1623,
   13356, -1623,   705,   756,   759,  2305, -1623,   163, 10920, -1623,
    2710, -1623, -1623, 16080, -1623, -1623, 11123, -1623, 13762, -1623,
     854,  9499,   940,   760, 13949,   943,    86,    59, -1623, -1623,
   -1623,   779, -1623, -1623, -1623,  4362, -1623,  2422,   766,   954,
   16351, 14334, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623,   768, -1623, -1623,   767,   774,   777,   786, 14334,   788,
     274,   374,   792,  4269, 14182, -1623, -1623, 14334, 14334, 13762,
      67,   170, -1623, 16351,   882, -1623, -1623, -1623,    67,    91,
     124,   780,   794,  2812,    75,   796,   809,   580,   853,   793,
      67,   125,   805, 17124,   789,  1002,  1003,   814,   816,   817,
     819, -1623,  4934, 14334, -1623, -1623,   944,  3288,    39, -1623,
   -1623, -1623,   625, -1623, -1623, -1623,   991,   891,   845,   197,
     867, 13762,   530,   892,  1020,   833, -1623,   872, -1623,   148,
   -1623,  4362,  4362,  1019,   895,    61, -1623,   841,  1029, -1623,
    4362,   130, -1623,   478,   140, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623,  1026,  3461, -1623, -1623, -1623, -1623,  1030,   858,
   -1623, 16907, 13762,   846,  1033, 18040,  1031, -1623, -1623,   912,
    4203, 12123, 18223, 15322, 14617, 13762, 17992, 14793, 11711, 13131,
   13536,  3341, 14961, 15138, 15138, 15138, 15138,  1689,  1689,  1689,
    1689,  1689,  1778,  1778,   718,   718,   718,   618,   618,   618,
   -1623,   822, 18040,   844,   849, 17180,   848,  1044,   238, 13762,
     246,   711,   419,   148, -1623, -1623, -1623,  1041,   825, -1623,
     764, 16701, -1623, -1623, -1623, 15322, 15322, 15322, 15322, 15322,
   15322, 15322, 15322, 15322, 15322, 15322, 15322, 15322, -1623, 13762,
     462,   149, -1623, -1623,   711,   528,   855,  3568,   860,   862,
     863,  3946,   127,   869, -1623, 18040, 16454, -1623, 14334, -1623,
     130,    35, 16907, 18040, 16907, 17228,   912,   130,    67,   151,
     903,   873, 13762, -1623,   157, -1623, -1623, -1623,  9093,   616,
   -1623, -1623, 18040, 18040,   206, -1623, -1623, -1623, 13762,   967,
   16227, 16351, 14334,  9702,   874,   877, -1623,  1068,  4661,   946,
   -1623,   923, -1623,  1075,   888,  2982,  4362, 16351, 16351, 16351,
   16351, 16351,   899,  1016,  1021,  1028,  1039,  1040,   900, 16351,
     406, -1623, -1623, -1623, -1623, -1623, -1623,   227, -1623, 18134,
   -1623, -1623,   239, -1623,  6251,  4283,   901, 14182, -1623, 14182,
   -1623,   468, -1623, 14334, 14334, -1623, 14182, 14182, -1623,  1092,
     914, -1623, -1623, -1623,  4214, -1623, 18134,  1097, 16907,   917,
   -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623,   930,
    1108, 14334,  4283,   920, 16598, 16804,  1102, -1623, 13762, -1623,
   13762, -1623, 13762, -1623, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623,   918, -1623, 13762, -1623, -1623,  5439, -1623,  4362,  4283,
     921, -1623, -1623, -1623, -1623,  1111,   925, 13762, 17720, -1623,
   -1623, 14136,   932, -1623,  4362, -1623,   938,  6454,  1106,    45,
   -1623, -1623,   126,  1715, -1623,  2710, -1623,  4362, -1623, -1623,
      67, 18040, -1623, 11326, -1623, 16351,    71,   956,  4283,   891,
   -1623, -1623, 14793, 13762, -1623, -1623, 13762, -1623, 13762, -1623,
    4282,   957, 10920,   853,  1122,   891,  4362,  1141,   912, 14334,
   17820,    67,  4840,   962, -1623, -1623,   147,   963, -1623, -1623,
    1150,  3929,  3929, 16454, -1623, -1623, -1623,  1114,   972,  1099,
    1101,  1103,  1104,  1105,    66,   981,   229, -1623, -1623, -1623,
   -1623, -1623,  1018, -1623, -1623, -1623, -1623,  1171,   985,   751,
      67,    67, 13559,   891,  2710, -1623, -1623,  4936,   640,   206,
   10311, -1623,  6657,   993,  6860,   994, 16227, 16907,   987,  1057,
      67, 18134,  1180, -1623, -1623, -1623, -1623,   520, -1623,    40,
    4362,  1022,  1062,  1042,  4362, 14334,  3550, -1623, -1623, -1623,
    1191, -1623,  1004,  1030,   665,   665,  1134,  1134, 17387,  1007,
    1196, 16351, 16351, 16351, 16351, 16351, 16351, 17720,  3207, 15663,
   16351, 16351, 16351, 16351, 16108, 16351, 16351, 16351, 16351, 16351,
   16351, 16351, 16351, 16351, 16351, 16351, 16351, 16351, 16351, 16351,
   16351, 16351, 16351, 16351, 16351, 16351, 16351, 16351, 14334, -1623,
   -1623,  1123, -1623, -1623,  1011,  1012, -1623, -1623, -1623,  4269,
   -1623,  1024, -1623, 16351,    67, -1623, -1623,   117, -1623,   658,
    1205, -1623, -1623,   128,  1023,    67, 11529, 18040, 17284, -1623,
    2948, -1623,  5642,   825,  1205, -1623,   480,   450, -1623, 18040,
    1080,  1027, -1623,  1032,  1106, -1623,  4362,   895,  4362,   308,
    1204,  1142,   162, -1623,   711,   187, -1623, -1623, 16907, 13762,
   18040, 18134,  1035,    71, -1623,  1025,    71,  1037, 14793, 18040,
   17332,  1043, 10920,  1045,  1038,  4362,  1048,  1046,  4362,   891,
   -1623,   705,   544, 10920, 13762, -1623, -1623, -1623, -1623, -1623,
   -1623,  1090,  1047,  1219,  1145, 16454, 16454, 16454, 16454, 16454,
   16454,  1081, -1623, 17720,    69, 16454, -1623, -1623, -1623, 16907,
   18040,  1049, -1623,   206,  1208,  1164, 10311, -1623, -1623, -1623,
    1051, 13762,  1057,    67, 16598, 16227,  1054, 16351,  7063,   650,
    1055, 13762,    64,    48, -1623,  1077, -1623,  4362, 14334, -1623,
    1124, -1623, -1623,  3971,  1229,  1065, 16351, -1623, 16351, -1623,
    1066,  1061,  1256, 17435,  1069, 18134,  1262,  1074,  1076,  1082,
    1144,  1269,  1086, -1623, -1623, -1623, 17490,  1084,  1278, 18179,
   15507, 10900, 16351, 18088, 12929, 13334, 14134, 12723, 16308, 16679,
   16679, 16679, 16679,  3490,  3490,  3490,  3490,  3490,   741,   741,
     665,   665,   665,  1134,  1134,  1134,  1134, -1623,  1093, -1623,
    1091,  1094, -1623, -1623, 18134, 14334,  4362,  4362, -1623,   658,
    4283,  1346, -1623, 16598, -1623, -1623, 15322, 13762,  1100, -1623,
    1115,  1985, -1623,    98, 13762, -1623, -1623, -1623, 13762, -1623,
   13762, -1623,   895, -1623, -1623,   145,  1285,  1218, 13762, -1623,
    1110,    67, 18040,  1106,  1118, -1623,  1120,    71, 13762, 10920,
    1121, -1623, -1623,   825, -1623, -1623,  1109,  1125,  1126, -1623,
    1128, 16454, -1623, 16454, -1623, -1623,  1130,  1131,  1293,  1168,
    1132, -1623,  1319,  1133,  1135,  1137, -1623,  1193,  1136,  1332,
   -1623, -1623,    67, -1623,  1312, -1623,  1147, -1623, -1623,  1149,
    1151,   131, -1623, -1623, 18134,  1152,  1156, -1623,  4163, -1623,
   -1623, -1623, -1623, -1623, -1623,  1211,  4362, -1623,  4362, -1623,
   18134, 17538, -1623, -1623, 16351, -1623, 16351, -1623, 16351, -1623,
   -1623, -1623, -1623, 16351, 17720, -1623, -1623, 16351, -1623, 16351,
   -1623, 11306, 16351,  1157,  7266, -1623, -1623,   658, -1623, -1623,
   -1623, -1623,   667, 14968,  4283,  1246, -1623,   673,  1190,  1953,
   -1623, -1623, -1623,   827,  3651,   111,   113,  1166,   825,   471,
     133, 18040, -1623, -1623, -1623,  1200,  5296,  5371, 18040, -1623,
     311,  1353,  1286, 13762, -1623, 18040, 10920,  1251,  1106,  2070,
    1106,  1173, 18040,  1175, -1623,  2169,  1177,  2226, -1623, -1623,
      71, -1623, -1623,  1242, -1623, -1623, 16454, -1623, 16454, -1623,
   16454, -1623, -1623, -1623, -1623, 16454, -1623, 17720, -1623,  2502,
   -1623,  9093, -1623, -1623, -1623, -1623,  9905, -1623, -1623, -1623,
    9093,  4362, -1623,  1182, 16351, 17593, 18134, 18134, 18134,  1247,
   18134, 17641, 11306, -1623, -1623,   658,  4283,  4283, 14334, -1623,
    1370, 15815,    78, -1623, 14968,   825,  4428, -1623,  1206, -1623,
     114,  1186,   115, -1623, 15321, -1623, -1623, -1623,   116, -1623,
   -1623,  1337, -1623,  1197, -1623,  1305,   764, -1623, 15145, -1623,
   15145, -1623, -1623,  1375,   827, -1623, 14440, -1623, -1623, -1623,
   -1623,  1377,  1313, 13762, -1623, 18040,  1213,  1215,  1106,   634,
   -1623,  1251,  1106, -1623, -1623, -1623, -1623,  2597,  1223, 16454,
    1289, -1623, -1623, -1623,  1290, -1623,  9093, 10108,  9905, -1623,
   -1623, -1623,  9093, -1623, -1623, 18134, 16351, 16351, 16351,  7469,
    1231,  1232, -1623, 16351, -1623,  4283, -1623, -1623, -1623, -1623,
   -1623,  4362,  1252,   673, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623, -1623, -1623, -1623,   381, -1623,  1190, -1623,
   -1623, -1623, -1623, -1623,   107,   129, -1623,  1407,   118, 15511,
    1305,  1413, -1623,  4362,   764, -1623, -1623,  1233,  1420, 13762,
   -1623, 18040, -1623,   120,  1240, -1623, -1623, -1623,  1106,   634,
   14616, -1623,  1106, -1623, 16454, 16454, -1623, -1623, -1623, -1623,
    7672, 18134, 18134, 18134, -1623, -1623, -1623, 18134, -1623,   965,
    1432,  1434,  1244, -1623, -1623, 16351, 15321, 15321,  1379, -1623,
    1337,  1337,   546, -1623, -1623, -1623, 16351,  1366, -1623,  1270,
    1258,   119, 16351, -1623, 14334, -1623, 16351, 18040,  1369, -1623,
    1446, -1623,  7875,  1257, -1623, -1623,   634, -1623, -1623,  8078,
    1259,  1345, -1623,  1360,  1304, -1623, -1623,  1371,  4362,  1299,
    1252, -1623, -1623, 18134, -1623, -1623,  1303, -1623,  1448, -1623,
   -1623, -1623, -1623, 18134,  1468,   580, -1623, -1623, 18134,  1291,
   18134, -1623,   146,  1288,  8281, -1623, -1623, -1623,  1292, -1623,
    1295,  1308, 14334,   471,  1307, -1623, -1623, -1623, 16351,  1310,
      73, -1623,  1410, -1623, -1623, -1623,  8484, -1623,  4283,   901,
   -1623,  1320, 14334,   627, -1623, 18134, -1623,  1300,  1487,   655,
      73, -1623, -1623,  1427, -1623,  4283,  1317, -1623,  1106,    81,
   -1623, -1623, -1623, -1623,  4362, -1623,  1306,  1314,   122, -1623,
    1322,   655,   152,  1106,  1321, -1623,  4362,   648,  4362,   314,
    1508,  1440,  1322, -1623,  1515, -1623,   242, -1623, -1623, -1623,
     158,  1512,  1445, 13762, -1623,   648,  8687,  4362, -1623,  4362,
   -1623,  8890,   315,  1516,  1453, 13762, -1623, 18040, -1623, -1623,
   -1623, -1623, -1623,  1517,  1456, 13762, -1623, 18040, 13762, -1623,
   18040, 18040
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,     0,   436,     0,   865,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   957,
     945,     0,   731,     0,   737,   738,   739,    27,   803,   932,
     933,   160,   161,   740,     0,   141,     0,     0,     0,     0,
      28,     0,     0,     0,     0,   195,     0,     0,     0,     0,
       0,     0,   405,   406,   407,   404,   403,   402,     0,     0,
       0,     0,   224,     0,     0,     0,    35,    36,    38,    39,
      37,   744,   746,   747,   741,   742,     0,     0,     0,   748,
     743,     0,   714,    30,    31,    32,    34,    33,     0,   745,
       0,     0,     0,     0,   749,   408,   543,    29,     0,   159,
     131,   937,   732,     0,     0,     4,   121,   123,   802,     0,
     713,     0,     6,   194,     7,     9,     8,    10,     0,     0,
     400,   449,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   447,   920,   921,   525,   519,   520,   521,   522,   523,
     524,   430,   528,     0,   429,   892,   715,   722,     0,   805,
     518,   399,   895,   896,   907,   448,     0,     0,   451,   450,
     893,   894,   891,   927,   931,     0,   508,   804,    11,   405,
     406,   407,     0,     0,    34,     0,   121,   194,     0,   997,
     448,   998,     0,  1000,  1001,   527,   444,     0,   437,   442,
       0,     0,   490,   491,   492,   493,    27,   932,   740,   717,
      35,    36,    38,    39,    37,     0,     0,  1021,   913,   715,
       0,   716,   469,     0,   471,   509,   510,   511,   512,   513,
     514,   515,   517,     0,   961,     0,   812,   727,   214,     0,
    1021,   427,   726,   720,     0,   736,   716,   940,   941,   947,
     939,   728,     0,   428,     0,   730,   516,     0,     0,     0,
       0,   433,     0,   139,   435,     0,     0,   145,   147,     0,
       0,   149,     0,    73,    74,    79,    80,    65,    66,    57,
      77,    88,    89,     0,    60,     0,    64,    72,    70,    91,
      83,    82,    55,    78,    98,    99,    56,    94,    53,    95,
      54,    96,    52,   100,    87,    92,    97,    84,    85,    59,
      86,    90,    51,    81,    67,   101,    75,    68,    58,    45,
      46,    47,    48,    49,    50,    69,   103,   102,   105,    62,
      43,    44,    71,  1068,  1069,    63,  1073,    42,    61,    93,
       0,     0,   121,   104,  1012,  1067,     0,  1070,     0,     0,
     151,     0,     0,     0,   185,     0,     0,     0,     0,     0,
       0,     0,     0,   814,     0,   109,   111,   313,     0,     0,
     312,   318,     0,     0,   225,     0,   228,     0,     0,     0,
       0,  1018,   210,   222,   953,   957,   562,   589,   589,   562,
     589,     0,   982,     0,   751,     0,     0,     0,   980,     0,
      16,     0,   125,   202,   216,   223,   619,   555,     0,  1006,
     535,   537,   539,   869,   436,   449,     0,     0,   447,   448,
     450,     0,     0,   733,     0,   734,     0,     0,     0,   184,
       0,     0,   127,   304,     0,    26,   193,     0,   221,   206,
     220,   405,   408,   194,   401,   174,   175,   176,   177,   178,
     180,   181,   183,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   945,     0,   173,   936,   936,   967,     0,     0,     0,
       0,     0,     0,     0,     0,   398,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   468,
     470,   870,   871,     0,   936,     0,   883,   304,   304,   936,
       0,   938,   928,   953,     0,   194,     0,     0,   153,     0,
     867,   862,   812,     0,   449,   447,     0,   965,     0,   560,
     811,   956,   736,   449,   447,   448,   127,     0,   304,   426,
       0,   885,   729,     0,   131,   264,     0,   542,     0,   156,
       0,     0,   434,     0,     0,     0,     0,     0,   148,   172,
     150,  1068,  1069,  1065,  1066,     0,  1072,  1058,     0,     0,
       0,     0,    76,    41,    63,    40,  1013,   179,   182,   152,
     131,     0,   169,   171,     0,     0,     0,     0,     0,     0,
     111,   112,     0,     0,   813,   110,    18,     0,   106,     0,
     314,     0,   154,     0,     0,   155,   226,   227,  1002,     0,
       0,   449,   447,   448,   451,   450,     0,  1048,   234,     0,
     954,     0,     0,     0,     0,   812,   812,     0,     0,     0,
       0,   157,     0,     0,   750,   981,   803,     0,     0,   979,
     808,   978,   124,     5,    13,    14,     0,   232,     0,     0,
     548,     0,     0,     0,   812,     0,   724,     0,   723,   718,
     549,     0,     0,     0,     0,   869,   131,     0,   814,   868,
    1077,   425,   439,   504,   901,   919,   136,   130,   132,   133,
     134,   135,   399,     0,   526,   806,   807,   122,   812,     0,
    1022,     0,     0,     0,   814,   305,     0,   531,   196,   230,
       0,   474,   476,   475,   487,     0,     0,   507,   472,   473,
     477,   479,   478,   496,   497,   494,   495,   498,   499,   500,
     501,   502,   488,   489,   481,   482,   480,   483,   484,   486,
     503,   485,   935,     0,     0,   971,     0,   812,  1005,     0,
    1004,  1021,   898,   927,   212,   204,   218,     0,  1006,   208,
     194,     0,   440,   443,   445,   453,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   873,     0,
     872,   875,   897,   879,  1021,   876,     0,     0,     0,     0,
       0,     0,     0,     0,   999,   438,   860,   864,   811,   866,
       0,   719,     0,   960,     0,   959,   230,     0,   719,   944,
     943,     0,     0,   872,   875,   942,   876,   431,   266,   268,
     131,   546,   545,   432,     0,   131,   248,   140,   435,     0,
       0,     0,     0,     0,   260,   260,   146,   812,     0,     0,
    1057,     0,  1054,   812,     0,  1028,     0,     0,     0,     0,
       0,   810,     0,    35,    36,    38,    39,    37,     0,     0,
     753,   757,   758,   759,   760,   761,   763,     0,   752,   129,
     801,   762,  1021,  1071,     0,     0,     0,     0,    21,     0,
      22,   112,    19,     0,   107,    20,     0,     0,   118,   814,
       0,   116,   108,   113,     0,   311,   319,   316,     0,     0,
     991,   996,   993,   992,   995,   994,    12,  1046,  1047,     0,
     812,     0,     0,     0,   953,   950,     0,   559,     0,   573,
     811,   561,   811,   588,   576,   582,   585,   579,   990,   989,
     988,     0,   984,     0,   985,   987,     0,     5,     0,     0,
       0,   613,   614,   622,   621,     0,   447,     0,   811,   554,
     558,     0,     0,  1007,     0,   536,     0,     0,  1035,   869,
     290,  1076,     0,     0,   884,     0,   934,   811,  1024,  1020,
     306,   307,   712,   813,   303,     0,   869,     0,     0,   232,
     533,   198,   506,     0,   596,   597,     0,   594,   811,   966,
       0,     0,   304,   234,     0,   232,     0,     0,   230,     0,
     945,   454,     0,     0,   881,   882,   899,   900,   929,   930,
       0,     0,     0,   848,   819,   820,   821,   828,     0,    35,
      36,    38,    39,    37,     0,     0,   834,   840,   841,   842,
     843,   844,     0,   832,   830,   831,   854,   812,     0,   862,
     964,   963,     0,   232,     0,   886,   735,     0,   270,     0,
       0,   137,     0,     0,     0,     0,     0,     0,     0,   240,
     241,   252,     0,   131,   250,   166,   260,     0,   260,     0,
     811,     0,     0,     0,     0,     0,   811,  1056,  1059,  1027,
     812,  1026,     0,   812,   784,   785,   782,   783,   818,     0,
     812,   810,   566,   591,   591,   566,   591,   557,     0,     0,
     973,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1062,
     186,     0,   189,   170,     0,     0,   114,   119,   120,   813,
     117,     0,   315,     0,  1003,   158,  1019,  1048,  1039,  1043,
     233,   235,   325,     0,     0,   951,     0,   564,     0,   983,
       0,    17,     0,  1006,   231,   325,     0,     0,   719,   551,
       0,   725,  1008,     0,  1035,   540,     0,     0,  1077,     0,
     295,   293,   875,   887,  1021,   875,   888,  1023,     0,     0,
     308,   128,     0,   869,   229,     0,   869,     0,   505,   970,
     969,     0,   304,     0,     0,     0,     0,     0,     0,   232,
     200,   736,   874,   304,     0,   824,   825,   826,   827,   835,
     836,   852,     0,   812,     0,   848,   570,   593,   593,   570,
     593,     0,   823,   856,     0,   811,   859,   861,   863,     0,
     958,     0,   874,     0,     0,     0,     0,   267,   547,   142,
       0,   435,   240,   242,   953,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   254,     0,  1063,     0,     0,  1049,
       0,  1055,  1053,   811,     0,     0,     0,   755,   811,   809,
       0,     0,   812,     0,     0,   798,   812,     0,     0,     0,
       0,   812,     0,   764,   799,   800,   977,     0,   812,   767,
     769,   768,     0,     0,   765,   766,   770,   772,   771,   788,
     789,   786,   787,   790,   791,   792,   793,   794,   779,   780,
     774,   775,   773,   776,   777,   778,   781,  1061,     0,   131,
       0,     0,   115,    23,   317,     0,     0,     0,  1040,  1045,
       0,   399,   955,   953,   441,   446,   452,     0,     0,    15,
       0,   399,   625,     0,     0,   627,   620,   623,     0,   618,
       0,  1010,     0,  1036,   544,     0,   296,     0,     0,   291,
       0,   310,   309,  1035,     0,   325,     0,   869,     0,   304,
       0,   925,   325,  1006,   325,  1009,     0,     0,     0,   455,
       0,     0,   838,   811,   847,   829,     0,     0,   812,     0,
       0,   846,   812,     0,     0,     0,   822,     0,     0,   812,
     833,   853,   962,   325,     0,   131,     0,   263,   249,     0,
       0,     0,   239,   162,   253,     0,     0,   256,     0,   261,
     262,   131,   255,  1064,  1050,     0,     0,  1025,     0,  1075,
     817,   816,   754,   574,   811,   565,     0,   577,   811,   590,
     583,   586,   580,     0,   811,   556,   756,     0,   595,   811,
     972,   796,     0,     0,     0,    24,    25,  1042,  1037,  1038,
    1041,   236,     0,     0,     0,   406,   397,     0,     0,     0,
     211,   324,   326,     0,   396,     0,     0,     0,  1006,   399,
       0,   563,   986,   321,   217,   616,     0,     0,   550,   538,
       0,   299,   289,     0,   292,   298,   304,   530,  1035,   399,
    1035,     0,   968,     0,   924,   399,     0,   399,  1011,   325,
     869,   922,   851,   850,   837,   575,   811,   569,     0,   578,
     811,   592,   584,   587,   581,     0,   839,   811,   855,   399,
     131,   269,   138,   143,   164,   243,     0,   251,   257,   131,
     259,     0,  1051,     0,     0,     0,   568,   797,   553,     0,
     976,   975,   795,   131,   190,  1044,     0,     0,     0,  1014,
       0,     0,     0,   237,     0,  1006,     0,   362,   358,   364,
     714,    34,     0,   352,     0,   357,   361,   374,     0,   372,
     377,     0,   376,     0,   375,     0,   194,   328,     0,   330,
       0,   331,   332,     0,     0,   952,     0,   617,   615,   626,
     624,   300,     0,     0,   287,   297,     0,     0,  1035,     0,
     207,   530,  1035,   926,   213,   321,   219,   399,     0,     0,
       0,   572,   845,   858,     0,   215,   265,     0,     0,   131,
     246,   163,   258,  1052,  1074,   815,     0,     0,     0,     0,
       0,     0,   424,     0,  1015,     0,   342,   346,   421,   422,
     356,     0,     0,     0,   337,   675,   676,   674,   677,   678,
     695,   697,   696,   666,   638,   636,   637,   656,   671,   672,
     632,   643,   644,   646,   645,   665,   649,   647,   648,   650,
     651,   652,   653,   654,   655,   657,   658,   659,   660,   661,
     662,   664,   663,   633,   634,   635,   639,   640,   642,   680,
     681,   685,   686,   687,   688,   689,   690,   673,   692,   682,
     683,   684,   667,   668,   669,   670,   693,   694,   698,   700,
     699,   701,   702,   679,   704,   703,   706,   708,   707,   641,
     711,   709,   710,   705,   691,   631,   369,   628,     0,   338,
     390,   391,   389,   382,     0,   383,   339,   416,     0,     0,
       0,     0,   420,     0,   194,   203,   320,     0,     0,     0,
     288,   302,   923,     0,     0,   392,   131,   197,  1035,     0,
       0,   209,  1035,   849,     0,     0,   131,   244,   144,   165,
       0,   567,   552,   974,   188,   340,   341,   419,   238,     0,
     812,   812,     0,   365,   353,     0,     0,     0,   371,   373,
       0,     0,   378,   385,   386,   384,     0,     0,   327,  1016,
       0,     0,     0,   423,     0,   322,     0,   301,     0,   611,
     814,   131,     0,     0,   199,   205,     0,   571,   857,     0,
       0,   167,   343,   121,     0,   344,   345,     0,   811,     0,
     811,   367,   363,   368,   629,   630,     0,   354,   387,   388,
     380,   381,   379,   417,   414,  1048,   333,   329,   418,     0,
     323,   612,   813,     0,     0,   393,   131,   201,     0,   247,
       0,   192,     0,   399,     0,   359,   366,   370,     0,     0,
     869,   335,     0,   609,   529,   532,     0,   245,     0,     0,
     168,   350,     0,   398,   360,   415,  1017,     0,   814,   410,
     869,   610,   534,     0,   191,     0,     0,   349,  1035,   869,
     274,   413,   412,   411,  1077,   409,     0,     0,     0,   348,
    1029,   410,     0,  1035,     0,   347,     0,     0,  1077,     0,
     279,   277,  1029,   131,   814,  1031,     0,   394,   131,   334,
       0,   280,     0,     0,   275,     0,     0,   813,  1030,     0,
    1034,     0,     0,   283,   273,     0,   276,   282,   336,   187,
    1032,  1033,   395,   284,     0,     0,   271,   281,     0,   272,
     286,   285
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1623, -1623, -1623,  -578, -1623, -1623, -1623,   177,     3,   -31,
     451, -1623,  -269,  -533, -1623, -1623,   390,    47,  1634, -1623,
    1107, -1623,  -460, -1623,    23, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623, -1623, -1623, -1623,  -359, -1623, -1623,  -159,
     161,    25, -1623, -1623, -1623, -1623, -1623, -1623,    33, -1623,
   -1623, -1623, -1623, -1623, -1623,    34, -1623, -1623,  1050,  1056,
    1052,  -103,  -724,  -894,   552,   609,  -368,   298,  -973, -1623,
     -86, -1623, -1623, -1623, -1623,  -758,   134, -1623, -1623, -1623,
   -1623,  -357, -1623,  -618, -1623,  -450, -1623, -1623,   947, -1623,
     -71, -1623, -1623, -1086, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623, -1623,  -108, -1623,   -17, -1623, -1623, -1623,
   -1623, -1623,  -191, -1623,    82,  -962, -1623, -1622,  -392, -1623,
    -152,    10,  -127,  -366, -1623,  -196, -1623, -1623, -1623,   100,
     -24,     0,   181,  -765,   -66, -1623, -1623,    15, -1623,   -12,
   -1623, -1623,    -5,   -44,   -55, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623, -1623,  -621,  -893, -1623, -1623, -1623, -1623,
   -1623,   890,  1178, -1623,   476, -1623,   343, -1623, -1623, -1623,
   -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623,
   -1623, -1623, -1623,   347,  -351,  -442, -1623, -1623, -1623, -1623,
   -1623,   407, -1623, -1623, -1623, -1623, -1623, -1623, -1623, -1623,
    -956,  -376,  2828,     6, -1623,  2123,  -412, -1623, -1623,  -494,
    3666,  3570, -1623,    68, -1623, -1623,   485,   -16,  -663, -1623,
   -1623,   581,   368,  -530, -1623,   370, -1623, -1623, -1623, -1623,
   -1623,   558, -1623, -1623, -1623,    95,  -916,  -147,  -437,  -435,
   -1623,   636,  -104, -1623, -1623,    26,    36,   620, -1623, -1623,
     178,   -22, -1623,  -375,    50,   176, -1623,  -118, -1623, -1623,
   -1623,  -473,  1216, -1623, -1623, -1623, -1623, -1623,   717,   339,
   -1623, -1623, -1623,  -367,  -685, -1623,  1172, -1186, -1623,   -68,
    -166,     9,   755, -1623,  -340, -1623,  -350,  -981, -1274,  -256,
     144, -1623,   466,   538, -1623, -1623, -1623, -1623,   488, -1623,
    1936, -1134
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   115,   937,   653,   186,  1569,   751,
     362,   363,   364,   365,   889,   890,   891,   117,   118,   119,
     120,   121,   421,   687,   688,   561,   262,  1637,   567,  1546,
    1638,  1881,   876,   355,   590,  1841,  1133,  1329,  1900,   437,
     187,   689,   977,  1197,  1388,   125,   656,   994,   690,   709,
     998,   628,   993,   241,   542,   691,   657,   995,   439,   382,
     404,   128,   979,   940,   913,  1150,  1572,  1256,  1059,  1788,
    1641,   827,  1065,   566,   836,  1067,  1431,   819,  1048,  1051,
    1245,  1907,  1908,   677,   678,   703,   704,   369,   370,   372,
    1606,  1766,  1767,  1341,  1481,  1595,  1760,  1890,  1910,  1799,
    1845,  1846,  1847,  1582,  1583,  1584,  1585,  1801,  1802,  1808,
    1857,  1588,  1589,  1593,  1753,  1754,  1755,  1777,  1949,  1482,
    1483,   188,   130,  1924,  1925,  1758,  1485,  1486,  1487,  1488,
     131,   255,   562,   563,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,  1618,   142,   976,  1196,   143,   674,
     675,   676,   259,   413,   557,   663,   664,  1291,   665,  1292,
     144,   145,   634,   635,  1281,  1282,  1397,  1398,   146,   861,
    1027,   147,   862,  1028,   148,   863,  1029,   149,   864,  1030,
     150,   865,  1031,   637,  1284,  1400,   151,   866,   152,   153,
    1830,   154,   658,  1608,   659,  1166,   945,  1359,  1356,  1746,
    1747,   155,   156,   157,   244,   158,   245,   256,   424,   549,
     159,  1285,  1286,   870,   871,   160,  1089,   968,   605,  1090,
    1034,  1219,  1035,  1401,  1402,  1222,  1223,  1037,  1408,  1409,
    1038,   797,   532,   200,   201,   692,   680,   513,  1182,  1183,
     783,   784,   964,   162,   247,   163,   164,   190,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   743,   175,   251,
     252,   631,   235,   236,   746,   747,  1297,  1298,   397,   398,
     931,   176,   619,   177,   673,   178,   346,  1768,  1820,   383,
     432,   698,   699,  1082,  1937,  1944,  1945,  1177,  1338,   909,
    1339,   910,   911,   842,   843,   844,   347,   348,   873,   576,
    1571,   962
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     189,   191,   444,   193,   194,   195,   196,   198,   199,   494,
     202,   203,   204,   205,   129,   960,   225,   226,   227,   228,
     229,   230,   231,   232,   234,   344,   253,   122,   524,   124,
     405,   666,   668,   258,   408,   409,   243,   126,   127,   261,
     670,   974,   416,  1178,  1365,   352,   263,   269,   806,   272,
     792,   267,   353,   955,   356,  1170,   248,   956,   493,   343,
     440,   444,   516,  1055,   418,  1470,   249,   788,   789,   740,
     888,   892,   351,   997,   546,   936,   781,  1069,   782,  1351,
     250,   261,  1043,  1252,   415,  1195,   420,  1655,   595,   597,
     599,   834,   602,    14,   818,   832,  1429,   -76,   811,   161,
     898,  1206,   -76,   417,   550,   -41,   -40,   366,   434,    14,
     -41,   -40,   558,   814,   611,   815,  1810,   614,   558,    14,
    1598,    14,  1600,  -355,  1663,  1748,   514,  1817,  1817,    14,
     874,  1655,  -904,   558,   915,   401,   915,   915,   402,   533,
     915,   418,   915,  1811,   591,  -605,  1261,  1262,   551,  1241,
    1410,   430,  1495,  -716,  1261,  1262,  1231,  1834,  1828,   371,
    1813,   415,   607,   420,  1179,   123,     3,   907,   908,   511,
     512,   495,   535,   511,   512,   207,    40,   192,   375,  1814,
     417,   116,  1815,  1500,  1892,   654,   655,   527,   376,  -914,
    1939,  -608,   534,  1362,   420,   544,  1962,   223,   223,  1294,
     514,   511,   512,  1829,  1290,  -905,   394,   592,  -903,  1180,
    -909,   417,  -902,  -946,  1877,   543,   957,  -908,   519,  -906,
     541,  -949,  -723,  1232,   608,   260,  1140,  -948,  1501,  1893,
     270,  -916,  -889,   342,   417,  1940,  -717,  -598,  -813,   935,
    1264,  1963,  -813,   519,  -605,   643,   207,    40,  1432,   553,
     381,   942,   553,   367,  -294,   515,  1959,  -890,   835,   261,
     564,  -904,  1430,   406,   511,   512,  1036,   391,  -294,   575,
    -278,   518,   904,   403,  1209,   381,  1656,  1657,  -813,   381,
     381,   555,  1422,   833,   710,   560,   -76,  1570,   899,  1509,
     443,  1470,  -541,   521,   -41,   -40,  1515,   435,  1517,  -811,
    1181,   559,  -724,   612,  1812,   381,   615,   641,  1259,  1599,
    1263,  1601,  -355,  1664,  1749,  1387,  1818,  1867,   586,  1502,
    1935,   900,   916,  -606,  1010,  1342,  1941,  1539,  1545,   515,
    1605,  -911,  1964,  -718,  -905,   622,  -915,  -903,  -912,  -909,
    1407,  -902,  -946,  -918,  1336,  1337,  -908,   520,  -906,   525,
    -949,   522,   343,  1137,  1138,   621,  -948,   625,  1192,  1162,
    1052,  -889,   943,  -913,   111,  1054,   793,   744,   444,   708,
     518,   368,   520,   261,   417,   254,   607,   944,   531, -1021,
     234,   633,   261,   261,   633,   261,  -890,  1099,  1658,  -121,
     647,  1366,  1507,  -121,  1611,  1805,   786,  1951,  1973,  1128,
     431,   790,   257,   344,   366,   366,   600,   601,   600,   198,
    -121,   223,  1761,  1806,  1762, -1021,   410,   693, -1021,  -105,
     431,   264,   405,   757,   758,   440,  1100,  -104,   705,   392,
     762,   129,  1807,  1627,  -105,   116,   649,   343,   430,   116,
     265,  1153,  -104,   565,   392,   266,   652,   392,   711,   712,
     713,   714,   716,   717,   718,   719,   720,   721,   722,   723,
     724,   725,   726,   727,   728,   729,   730,   731,   732,   733,
     734,   735,   736,   737,   738,   739,   883,   741,  1350,   742,
     742,   745,  1367,  1221,   764,  1612,  1419,   243,  1952,  1974,
     750,   765,   766,   767,   768,   769,   770,   771,   772,   773,
     774,   775,   776,   777,   395,   396,   354,   248,   679,   742,
     787,   763,   705,   705,   742,   791,   799,   249,   644,   395,
     396,   765,   395,   396,   795,   411,   585,  1619,  1185,  1621,
    1186,   250,   412,   803,   963,   805,   965,   752,   884,   343,
     494,   377,  1203,   705,   821,   378,   223,   392,  -607,   379,
     695,   822,   380,   823,   393,   223,  1364,   624,   392,   620,
     392,  1559,   223,   785,   642,   649, -1021,   423,   636,   636,
     883,   636,   666,   668,   223,  1374,   384,  1860,  1376,  -725,
     385,   670,   123,   669,   826,   991,   752,   431,  1211,   493,
     441,   180,   181,    65,    66,    67,  1861,   810,   116,  1862,
     816,   999,   392,  1258,   894, -1021,   888,   697,  1134,   649,
    1135,   342,   511,   512,   381,   992,  1357,   946,  1003,   921,
     923,   394,   395,   396,   165,  1260,  1261,  1262,   511,   512,
     907,   908,   430,   395,   396,   395,   396,  1774,  -719,   222,
     224,  1779,   981,  -877,  1634,   881,  1352,   386,   949,  1358,
     600,   600,   387,   963,   965,   760,   417,   388,  -877,  1353,
    1044,   965,   442,   753,   585,   381,   755,   381,   381,   381,
     381,   389,  1960,    55,  -916,   481,   650,   395,   396,  1354,
     696,   441,   180,   181,    65,    66,    67,   482,   406,   753,
     780,  1049,  1050,   392,   546,  1221,  1399,   971,  1516,  1399,
     426,   223,   495,  1045,  1576,  1411,  1129,   392,   390,  -880,
     982,   407,   753,   585,   649,  1243,  1244,  1389,  1124,  1125,
    1126,   666,   668,   753,  -880,  -878,   753,   813,   419,   422,
     670,   989,   645,   373,  1127,   638,   651,   640,   116,   429,
    -878,  1499,   374,  1287,   990,  1289,   441,   180,   181,    65,
      66,    67,  1380,   442,   206,  1426,  1261,  1262,   872,  1511,
    1336,  1337,   645,  1390,   651,   645,   651,   651,   395,   396,
     679,   478,   479,   480,  1002,   481,    50,  1921,  1922,  1923,
     433,  1421,   395,   396,   893,   697,  1403,   482,  1405,   430,
    1932,  1121,  1122,  1123,  1124,  1125,  1126,  1833,  1577,  1566,
    1567,  1836,   436,  1603,  1950,   419,   445,  1047,   594,   596,
    1127,  1578,   210,   211,   212,   213,   214,  1579,   442,   930,
     932,  1071,  -599,   261,  1053,  1917,   446,  1077,   129,   425,
     427,   428,  1775,  1776,   183,   447,   419,    91,  1580,   448,
      93,    94,   449,    95,  1581,    97,  1947,  1948,  1858,  1859,
    1854,  1855,   450,   537,  1080,  1083,  1064,   666,   668,   545,
     451,  1522,   452,  1523,  1033,  -600,   670,  -601,   107,  1464,
    1490,  -602,  -603,   484,   485,   486,   487,   517,   165,   223,
    -910,  -604,   165,  -717,   129,   523,   399,   381,   528,   530,
    1659,   482,   431,   536,  1148,  -914,   518,   539,   540,  -715,
     547,   548,  1628,   556,   569,   577, -1060,   580,   581,   218,
     218,   587,   588,  1157,   603,  1158,   604,   823,   606,   613,
     616,   617,   626,   627,   600,   671,   600,   672,  1160,  1513,
     694,  1210,   681,   600,   600,   682,   798,  1930,    55,   223,
     683,   685,  1169,  -126,   644,   707,   129,   796,   824,   558,
     800,   801,  1942,   807,   750,  1541,   808,   831,   828,   122,
     575,   124,   845,   846,   875,   897,   877,   129,  1190,   126,
     127,  1550,   878,  1026,  1909,  1039,   879,   901,  1198,   123,
     223,  1199,   223,  1200,   880,   912,   882,   705,   610,   914,
     885,   902,   919,   905,  1909,   116,  1630,   618,  1631,   623,
    1632,  1171,   917,  1931,   630,  1633,   243,   906,   223,  1062,
     116,   920,   922,   785,   933,   816,   648,   924,  1370,   925,
     926,  1236,   927,   938,   939,   941,   248,  -740,   947,   948,
     950,   161,   951,   954,   958,   123,   249,  1240,   959,   967,
     969,   165,   973,   972,   978,   975,   206,   984,   987,  1246,
     250,   116,   985,   988,   679,   996,  1616,  1006,  1004,  1007,
    1136,   697,   129,  -721,   129,   980,  1008,  1275,    50,  1056,
    1046,   679,  1066,  1247,  1279,  1068,   223,  1070,   666,   668,
    1636,  1033,  1074,  1075,  1076,  1078,  1092,   670,  1149,  1642,
    1344,  1093,   223,   223,   816,  1091,  1097,   123,  1094,  1783,
    1132,  1139,  1295,  1649,   210,   211,   212,   213,   214,  1095,
    1096,  1143,  1146,   116,  1141,  1145,  1156,  1147,   123,  1152,
    1165,  1159,  1168,   218,   669,  1167,   183,   753,   585,    91,
    1172,  1174,    93,    94,   116,    95,   184,    97,  1176,   753,
     780,   753,   813,   630,  1345,   441,    63,    64,    65,    66,
      67,  1346,  1193,  1202,  1205,  1208,    72,   488,  1213,  -917,
     107,   666,   668,   345,  1214,  1842,  1224,  1873,  1225,  1226,
     670,  1227,   129,  1228,  1229,  1230,   381,  1233,  1234,  1790,
    1235,   165,  1237,  1254,  1372,   122,   600,   124,  1218,  1218,
    1026,  1249,  1251,  1255,  1257,   126,   127,   705,  1267,   490,
    1273,  1274,  1268,  1127,  1266,  1278,  1328,  1394,   705,  1346,
    1277,  1330,  1331,   123,  1340,   123,  1360,   442,  1368,  1343,
     753,   813,  1333,   992,  1375,  1369,  1391,   116,  1393,   116,
    1361,   116,  1373,  1377,   223,   223,  1017,  1382,  1406,  1379,
    1415,  1416,  1381,  1414,  1385,  1920,   261,  1384,  1413,  1418,
    1392,  1423,  1270,  1427,  1837,  1838,  1428,   161,   218,  1433,
    1436,  1438,  1439,  1442,  1443,  1444,  1445,   218,   129,  1417,
    1449,  1448,  1447,   669,   218,  1455,   585,  1450,  1454,  1451,
    1453,  1958,  1460,  1456,  1458,  1452,   218,  1459,   679,  1465,
    1463,   679,  1466,  1033,  1033,  1033,  1033,  1033,  1033,  1503,
    1492,  1504,  1526,  1033,  1528,   872,  1506,  1518,   441,    63,
      64,    65,    66,    67,  1493,  1508,  1832,  1510,  1514,    72,
     488,   970,  1520,   123,  1519,  1521,  1839,  1524,  1530,  1535,
      34,    35,    36,  1536,  1525,  1529,  1532,  1604,  1533,   116,
    1534,  1537,  1491,   208,  1540,  1542,  1543,  1551,  1544,  1496,
    1547,  1484,  1472,  1497,  1548,  1498,  1563,   444,  1574,  1587,
     489,  1484,   490,  1505,  1602,  1607,   223,  1613,  1617,  1614,
    1622,  1874,  1623,  1512,   705,   491,  1625,   492,  1629,  1644,
     442,  1001,  1527,  1647,  1653,  1662,  1531,  1661,  1757,  1763,
     345,  1769,   345,  1538,    14,  1756,  1770,    81,    82,    83,
      84,    85,  1026,  1026,  1026,  1026,  1026,  1026,   215,   669,
    1772,  1773,  1026,   218,    89,    90,  1896,   223,   206,   123,
    1782,  1816,  1040,   116,  1041,  1784,  1785,  1822,    99,  1795,
    1796,  1825,   223,   223,  1826,   116,  1489,  1759,   165,  1831,
      50,  1848,   104,  1850,  1852,  1435,  1489,  1856,   345,  1864,
    1060,  1865,  1871,   165,  1866,  1872,  1876,  1879,  1473,  1033,
    1880,  1033,  -351,  1474,  1882,   441,  1475,   181,    65,    66,
      67,  1476,   679,  1883,   129,  1887,   210,   211,   212,   213,
     214,  1885,  1888,  1956,  1811,  1894,  1899,  1891,  1961,  1904,
    1897,  1898,  1906,  1911,   165,  1915,  1919,  1918,  1615,   495,
    1652,   705,  1750,  1933,    93,    94,  1751,    95,   184,    97,
    1927,  1934,  1467,  1477,  1478,  1929,  1479,  1936,  1144,  1484,
    1943,   223,  1953,  1954,  1957,  1484,  1965,  1484,  1966,  1332,
    1975,  1978,   107,  1591,   630,  1155,  1976,   442,   345,  1979,
    1914,   345,   756,   759,   754,  1204,  1480,  1928,  1164,  1484,
    1420,   129,  1789,  1926,  1780,  1804,   165,  1809,   895,  1660,
     129,  1594,  1549,  1968,  1821,  1938,  1778,   639,  1026,  1640,
    1026,  1288,  1404,  1355,  1654,  1575,  1280,   165,   496,   497,
     498,   499,   500,   501,   502,   503,   504,   505,   506,   507,
     508,   218,  1395,  1220,  1033,  1396,  1033,  1238,  1033,  1184,
    1081,   632,  1955,  1033,  1489,  1824,   706,  1970,  1771,  1889,
    1489,  1565,  1489,  1335,  1272,   679,  1327,     0,     0,     0,
       0,     0,     0,   509,   510,   123,     0,     0,     0,     0,
     669,     0,     0,     0,  1489,     0,     0,  1484,     0,     0,
       0,   116,     0,     0,  1596,     0,   129,     0,     0,     0,
     342,   218,   129,   217,   217,     0,  1592,     0,     0,   129,
    1787,  1640,     0,     0,   240,     0,     0,     0,     0,     0,
     165,     0,   165,     0,   165,     0,  1060,  1253,     0,     0,
       0,     0,   345,     0,   841,     0,     0,     0,     0,     0,
     240,     0,   218,     0,   218,     0,     0,  1033,   511,   512,
       0,     0,   123,  1026,     0,  1026,     0,  1026,     0,     0,
       0,   123,  1026,   669,     0,     0,     0,     0,   116,     0,
     218,     0,  1489,   116,     0,     0,     0,   116,  1819,     0,
       0,  1902, -1061, -1061, -1061, -1061, -1061,   473,   474,   475,
     476,   477,   478,   479,   480,   381,   481,     0,   585,     0,
       0,   342,     0,     0,     0,     0,  1869,     0,   482,     0,
       0,  1745,   343,     0,  1827,  1764,     0,     0,  1752,     0,
       0,     0,     0,     0,     0,   342,   444,   342,   345,   345,
       0,     0,   165,   342,  1849,  1851,     0,   345,   218,     0,
     283,     0,     0,     0,     0,     0,   206,   123,   207,    40,
     129,     0,     0,   123,   218,   218,  1026,     0,  1371,     0,
     123,     0,     0,   116,   116,   116,     0,     0,    50,   116,
       0,     0,     0,     0,     0,     0,   116,   285,   475,   476,
     477,   478,   479,   480,     0,   481,     0,     0,     0,     0,
     206,     0,   129,     0,     0,     0,     0,   482,     0,   129,
       0,     0,  1033,  1033,   210,   211,   212,   213,   214,  1412,
       0,     0,    50,     0,     0,     0,   165,   217,     0,     0,
     578,     0,     0,     0,   630,  1060,     0,     0,   165,     0,
     778,     0,    93,    94,   129,    95,   184,    97,     0,     0,
       0,     0,     0,  1903,     0,     0,     0,   571,   210,   211,
     212,   213,   214,   572,     0,     0,   129,     0,     0,     0,
     107,     0,     0,     0,   779,     0,   111,   240,     0,   240,
     183,     0,     0,    91,   336,     0,    93,    94,     0,    95,
     184,    97,     0,     0,     0,     0,   585,     0,     0,     0,
       0,     0,     0,     0,   340,  1073,   218,   218,  1967,     0,
       0,   123,   345,   345,   107,   341,     0,   342,     0,     0,
    1977,  1026,  1026,   630,     0,     0,   129,   116,     0,     0,
    1980,   129,     0,  1981,     0,   240,  1843,     0,     0,     0,
       0,     0,     0,  1745,  1745,   679,     0,  1752,  1752,     0,
       0,  1472,   349,   123,     0,     0,     0,     0,     0,     0,
     123,   381,   217,     0,     0,   679,     0,     0,     0,   116,
       0,   217,     0,     0,   679,     0,   116,     0,   217,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     217,     0,     0,    14,   206,   123,     0,     0,     0,     0,
       0,   217,     0,     0,     0,   345,     0,     0,     0,     0,
       0,   116,     0,     0,     0,     0,    50,   123,     0,  1901,
       0,   345,     0,     0,     0,   240,     0,     0,   240,     0,
       0,     0,     0,   116,   345,     0,  1472,     0,   218,  1916,
       0,  1590,     0,     0,   165,     0,     0,     0,     0,     0,
       0,     0,   210,   211,   212,   213,   214,  1473,     0,     0,
       0,     0,  1474,   345,   441,  1475,   181,    65,    66,    67,
    1476,     0,     0,     0,     0,   240,     0,   123,    14,     0,
      93,    94,   123,    95,   184,    97,     0,     0,     0,   218,
       0,     0,     0,   116,     0,     0,     0,     0,   116,     0,
       0,     0,   220,   220,   218,   218,     0,     0,   107,  1591,
       0,     0,  1477,  1478,     0,  1479,     0,   217,     0,     0,
       0,   165,     0,     0,   206,     0,   165,     0,     0,     0,
     165,     0,     0,     0,     0,  1472,   442,   345,     0,     0,
       0,   345,  1473,   841,     0,  1494,    50,  1474,     0,   441,
    1475,   181,    65,    66,    67,  1476,   526,   497,   498,   499,
     500,   501,   502,   503,   504,   505,   506,   507,   508,   240,
       0,   240,     0,     0,   860,     0,     0,    14,     0,   573,
       0,   574,   210,   211,   212,   213,   214,     0,     0,     0,
       0,     0,  1472,   218,     0,     0,     0,  1477,  1478,     0,
    1479,   509,   510,     0,     0,     0,     0,   860,     0,     0,
      93,    94,     0,    95,   184,    97,   165,   165,   165,     0,
       0,   442,   165,     0,     0,     0,     0,     0,     0,   165,
    1620,     0,     0,     0,    14,     0,     0,   579,   107,   707,
       0,  1473,     0,   345,     0,   345,  1474,     0,   441,  1475,
     181,    65,    66,    67,  1476,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   240,   240,     0,     0,     0,
       0,     0,   345,     0,   240,   345,   511,   512,     0,   526,
     497,   498,   499,   500,   501,   502,   503,   504,   505,   506,
     507,   508,     0,     0,     0,   217,  1477,  1478,  1473,  1479,
       0,     0,     0,  1474,     0,   441,  1475,   181,    65,    66,
      67,  1476,     0,     0,     0,     0,   220,     0,     0,     0,
     442,     0,     0,     0,   509,   510,     0,   700,     0,  1624,
     349,     0,     0,     0,   345,     0,     0,     0,     0,   684,
     345,     0,     0,     0,     0,     0,     0,   206,     0,     0,
       0,     0,     0,  1477,  1478,   217,  1479,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
     165,     0,     0,     0,     0,     0,     0,   442,     0,     0,
       0,     0,     0,     0,     0,     0,  1626,     0,     0,     0,
     240,     0,     0,     0,     0,     0,   217,     0,   217,   511,
     512,     0,     0,   345,   345,   210,   211,   212,   213,   214,
       0,     0,   165,   838,     0,     0,     0,     0,     0,   165,
       0,     0,     0,     0,   217,   860,     0,     0,     0,     0,
     399,     0,   240,    93,    94,     0,    95,   184,    97,   240,
     240,   860,   860,   860,   860,   860,     0,     0,     0,     0,
       0,   220,     0,   860,   165,     0,     0,     0,     0,     0,
     220,   107,   809,   206,     0,   400,     0,   220,  1472,   240,
       0,   837,     0,   839,     0,     0,   165,     0,     0,   220,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
     667,     0,   217,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   345,     0,   345,   240,     0,   217,   217,
      14,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   210,   211,   212,   213,   214,     0,     0,     0,     0,
       0,     0,   240,   240,     0,     0,   165,     0,     0,     0,
     345,   165,   217,   183,     0,     0,    91,     0,   240,    93,
      94,   345,    95,   184,    97,     0,   840,     0,     0,     0,
       0,   240,     0,  1472,     0,     0,     0,   952,   953,   860,
       0,     0,   240,     0,  1473,     0,   961,   107,     0,  1474,
       0,   441,  1475,   181,    65,    66,    67,  1476,     0,     0,
     240,     0,     0,     0,   240,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,   220,   240,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   345,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1477,
    1478,     0,  1479,     0,     0,     0,     0,     0,     0,     0,
       0,   345,     0,     0,     0,     0,     0,     0,     0,     0,
     217,   217,     0,   442,     0,     0,     0,     0,     0,     0,
       0,     0,  1635,     0,   240,   345,     0,   345,   240,  1473,
     240,     0,     0,   345,  1474,     0,   441,  1475,   181,    65,
      66,    67,  1476,     0,     0,   860,   860,   860,   860,   860,
     860,   217,     0,     0,   860,   860,   860,   860,   860,   860,
     860,   860,   860,   860,   860,   860,   860,   860,   860,   860,
     860,   860,   860,   860,   860,   860,   860,   860,   860,   860,
     860,   860,     0,     0,  1477,  1478,     0,  1479,   345,     0,
       0,     0,     0,     0,     0,     0,     0,   860,     0,     0,
       0,   700,   700,     0,     0,     0,     0,     0,   442,     0,
       0,   206,     0,   207,    40,     0,     0,  1781,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     240,     0,   240,    50,     0,     0,     0,     0,     0,     0,
       0,     0,   217,     0,   220,     0,   526,   497,   498,   499,
     500,   501,   502,   503,   504,   505,   506,   507,   508,   240,
       0,     0,   240,     0,     0,     0,     0,   219,   219,   210,
     211,   212,   213,   214,     0,     0,     0,     0,   242,   240,
     240,   240,   240,   240,   240,     0,     0,   217,     0,   240,
     345,   509,   510,   217,  1163,   778,     0,    93,    94,     0,
      95,   184,    97,     0,   220,     0,     0,   345,   217,   217,
    1173,   860,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   240,     0,  1187,     0,   107,  1844,   240,     0,   812,
     860,   111,   860,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   220,     0,   220,     0,     0,
       0,     0,  1207,     0,     0,     0,   860,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   511,   512,     0,     0,
       0,     0,     0,   220,     0,   345,     0,     0,   453,   454,
     455,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     240,   240,     0,     0,   240,     0,     0,   217,   456,   457,
       0,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,  1265,     0,     0,   903,
    1269,     0,     0,   283,     0,     0,     0,   482,     0,     0,
       0,   220,     0,     0,     0,   240,     0,   240,     0,     0,
       0,   345,     0,     0,     0,     0,     0,   220,   220,     0,
       0,     0,     0,   345,     0,   345,     0,     0,     0,     0,
     285,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   219,     0,   206,   345,     0,   345,     0,     0,     0,
     240,   667,   240,     0,     0,     0,     0,     0,   860,     0,
     860,     0,   860,     0,     0,    50,     0,   860,   217,     0,
       0,   860,     0,   860,     0,     0,   860,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   240,   240,     0,
       0,   240,  1363,     0,   961,     0,     0,     0,   240,     0,
     571,   210,   211,   212,   213,   214,   572,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1383,     0,   183,  1386,     0,    91,   336,     0,    93,
      94,  1348,    95,   184,    97,     0,  1079,     0,     0,     0,
     240,     0,   240,     0,   240,     0,     0,   340,     0,   240,
       0,   217,     0,     0,     0,     0,     0,   107,   341,   220,
     220,     0,     0,     0,     0,   240,     0,     0,   860,     0,
       0,     0,     0,     0,     0,     0,   219,     0,     0,     0,
     240,   240,     0,  1434,     0,   219,     0,     0,   240,  1187,
     240,     0,   219,     0,     0,     0,     0,  1101,  1102,  1103,
     667,     0,     0,     0,   219,     0,     0,     0,     0,     0,
       0,     0,   240,     0,   240,   219,     0,     0,  1104,     0,
     240,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1124,  1125,  1126,   240,     0,     0,     0,     0,     0,     0,
       0,     0,  1468,  1469,     0,     0,  1127,     0,     0,     0,
     860,   860,   860,     0,     0,     0,     0,   860,     0,   240,
       0,     0,     0,     0,     0,   240,     0,   240,   453,   454,
     455,     0,     0,     0,     0,     0,     0,     0,     0,   242,
       0,   220,     0,     0,     0,     0,     0,     0,   456,   457,
       0,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,     0,     0,     0,     0,
       0,   219,     0,     0,     0,     0,   667,   482,     0,     0,
       0,     0,   220,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1552,     0,  1553,     0,     0,   220,   220,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   240,   481,     0,
       0,     0,     0,     0,  1293,     0,     0,     0,   867,     0,
     482,     0,     0,     0,   240,     0,     0,     0,   240,   240,
    1597,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   240,     0,     0,     0,     0,     0,   860,
       0,   867,     0,     0,     0,     0,     0,     0,     0,     0,
     860,     0,     0,     0,     0,     0,   860,     0,     0,     0,
     860,     0,     0,     0,     0,     0,   220,     0,     0,     0,
       0,   453,   454,   455,     0,     0,     0,     0,     0,     0,
       0,     0,   240,     0,     0,     0,     0,  1643,   934,     0,
       0,   456,   457,     0,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,     0,
       0,     0,   860,     0,     0,     0,     0,     0,     0,   219,
     482,     0,   240, -1061, -1061, -1061, -1061, -1061,  1119,  1120,
    1121,  1122,  1123,  1124,  1125,  1126,     0,     0,     0,   240,
       0,     0,     0,     0,     0,     0,     0,     0,   240,  1127,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     240,     0,   240,     0,     0,     0,     0,   667,   453,   454,
     455,   838,     0,     0,     0,     0,     0,     0,     0,   219,
       0,   240,     0,   240,     0,     0,     0,  1800,   456,   457,
       0,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,  1032,   481,     0,     0,     0,     0,
     219,   206,   219,     0,     0,     0,     0,   482,     0,     0,
       0,   839,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,   219,   867,
     667,   966,   526,   497,   498,   499,   500,   501,   502,   503,
     504,   505,   506,   507,   508,   867,   867,   867,   867,   867,
       0,     0,   283,     0,     0,   221,   221,   867,     0,   210,
     211,   212,   213,   214,     0,     0,   246,     0,     0,  1823,
       0,     0,     0,  1131,     0,     0,     0,   509,   510,     0,
       0,   183,     0,     0,    91,     0,     0,    93,    94,   285,
      95,   184,    97,     0,  1271,     0,   219,     0,     0,     0,
       0,     0,   206,     0,     0,     0,     0,     0,     0,     0,
    1151,     0,   219,   219,     0,   107,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,  -398,     0,     0,     0,     0,  1151,  1005,     0,
     441,   180,   181,    65,    66,    67,   219,     0,     0,     0,
       0,     0,   511,   512,  1884,     0,     0,     0,     0,   571,
     210,   211,   212,   213,   214,   572,     0,     0,     0,     0,
       0,     0,     0,   867,     0,     0,  1194,     0,     0,     0,
       0,     0,   183,     0,     0,    91,   336,     0,    93,    94,
       0,    95,   184,    97,     0,     0,     0,     0,   242,     0,
       0,     0,     0,     0,     0,     0,   340,     0,     0,     0,
       0,  1032,   442,     0,     0,     0,   107,   341,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     961,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1946,     0,   961,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   219,   219,     0,     0,     0,     0,
       0,     0,     0,  1946,     0,  1971,     0,     0,     0,   221,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   867,
     867,   867,   867,   867,   867,   219,     0,     0,   867,   867,
     867,   867,   867,   867,   867,   867,   867,   867,   867,   867,
     867,   867,   867,   867,   867,   867,   867,   867,   867,   867,
     867,   867,   867,   867,   867,   867,   453,   454,   455,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   867,     0,     0,     0,     0,   456,   457,     0,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,   283,   481,     0,     0,     0,  1215,  1216,  1217,
     206,     0,     0,     0,     0,   482,   219,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,   221,     0,     0,     0,     0,   285,
       0,     0,     0,   221,     0,     0,     0,     0,     0,     0,
     221,     0,   206,  1032,  1032,  1032,  1032,  1032,  1032,     0,
       0,   219,   221,  1032,     0,     0,     0,   219,   210,   211,
     212,   213,   214,   246,    50,     0,     0,     0,     0,     0,
       0,     0,   219,   219,     0,   867,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    93,    94,     0,    95,
     184,    97,     0,     0,   867,     0,   867,     0,     0,   571,
     210,   211,   212,   213,   214,   572,     0,     0,     0,     0,
       0,     0,     0,     0,   107,     0,     0,     0,     0,     0,
     867,     0,   183,     0,     0,    91,   336,     0,    93,    94,
       0,    95,   184,    97,     0,  1437,  1009,   246,     0,     0,
     869,     0,     0,     0,     0,     0,   340,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   107,   341,  1471,     0,
       0,   219,     0,   453,   454,   455,     0,     0,     0,     0,
       0,     0,     0,   896,     0,     0,     0,     0,     0,   221,
       0,     0,     0,   456,   457,  1429,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,  1032,
     481,  1032,     0,     0,   453,   454,   455,     0,     0,     0,
       0,     0,   482,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   456,   457,   868,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,   867,     0,   867,     0,   867,     0,     0,   868,
       0,   867,   219,   482,   206,   867,     0,   867,     0,     0,
     867,     0,   453,   454,   455,     0,     0,     0,     0,     0,
       0,     0,  1573,     0,     0,  1586,    50,     0,     0,     0,
       0,     0,   456,   457,     0,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
       0,     0,   210,   211,   212,   213,   214,     0,     0,     0,
     206,   482,     0,     0,  1032,     0,  1032,     0,  1032,     0,
       0,  1430,     0,  1032,   206,   219,     0,   221,     0,     0,
      93,    94,    50,    95,   184,    97,     0,     0,     0,     0,
     886,   887,   867,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,   283,  1650,  1651,     0,     0,   107,   980,
       0,  1061,     0,     0,  1586,     0,     0,     0,   210,   211,
     212,   213,   214,     0,  1142,     0,     0,  1084,  1085,  1086,
    1087,  1088,   210,   211,   212,   213,   214,   221,     0,  1098,
     285,     0,     0,   598,     0,     0,    93,    94,     0,    95,
     184,    97,     0,   206,   183,     0,     0,    91,    92,     0,
      93,    94,     0,    95,   184,    97,     0,  1032,     0,     0,
       0,     0,     0,     0,   107,    50,     0,     0,   221,     0,
     221,     0,     0,     0,   867,   867,   867,     0,   107,     0,
       0,   867,  1201,  1798,     0,     0,     0,     0,     0,     0,
       0,  1586,     0,     0,     0,     0,   221,   868,     0,     0,
     571,   210,   211,   212,   213,   214,   572,     0,     0,   206,
       0,     0,     0,   868,   868,   868,   868,   868,     0,     0,
       0,     0,     0,   183,     0,   868,    91,   336,     0,    93,
      94,    50,    95,   184,    97,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1191,     0,   340,     0,     0,
       0,     0,     0,  1577,     0,     0,     0,   107,   341,     0,
       0,     0,     0,     0,   221,     0,  1578,   210,   211,   212,
     213,   214,  1579,     0,     0,     0,     0,     0,     0,     0,
     221,   221,     0,     0,     0,     0,     0,     0,     0,   183,
       0,     0,    91,    92,     0,    93,    94,     0,    95,  1581,
      97,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1032,  1032,   246,     0,     0,     0,     0,     0,
       0,     0,     0,   107,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   867,     0,     0,     0,     0,     0,     0,
       0,   868,   206,     0,   867,     0,     0,     0,     0,     0,
     867,     0,     0,     0,   867,     0,     0,     0,     0,     0,
       0,  1088,  1283,     0,    50,  1283,   246,     0,     0,     0,
    1296,  1299,  1300,  1301,  1303,  1304,  1305,  1306,  1307,  1308,
    1309,  1310,  1311,  1312,  1313,  1314,  1315,  1316,  1317,  1318,
    1319,  1320,  1321,  1322,  1323,  1324,  1325,  1326,     0,     0,
     210,   211,   212,   213,   214,     0,     0,     0,     0,     0,
       0,     0,     0,  1334,     0,     0,   867,     0,     0,     0,
       0,     0,   221,   221,     0,   360,  1913,     0,    93,    94,
       0,    95,   184,    97,     0,     0,     0,     0,     0,     0,
       0,     0,   206,  1573,     0,     0,     0,     0,     0,     0,
       0,     0,  1072,     0,     0,     0,   107,   868,   868,   868,
     868,   868,   868,   246,    50,     0,   868,   868,   868,   868,
     868,   868,   868,   868,   868,   868,   868,   868,   868,   868,
     868,   868,   868,   868,   868,   868,   868,   868,   868,   868,
     868,   868,   868,   868,     0,     0,     0,     0,     0,     0,
     210,   211,   212,   213,   214,     0,     0,     0,     0,   868,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   183,     0,     0,    91,     0,  1424,    93,    94,
       0,    95,   184,    97,     0,     0,     0,     0,     0,     0,
       0,     0,   206,     0,     0,     0,  1440,     0,  1441,     0,
     453,   454,   455,     0,   221,     0,   107,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
     456,   457,  1461,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,     0,   481,     0,   246,
     210,   211,   212,   213,   214,   221,     0,     0,     0,   482,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     221,   221,     0,   868,     0,     0,   438,     0,    93,    94,
       0,    95,   184,    97,     0,     0,     0,     0,     0,     0,
       0,     0,   868,     0,   868,     0,   453,   454,   455,     0,
       0,     0,     0,     0,     0,     0,   107,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   456,   457,   868,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   482,     0,     0,     0,   221,
       0,     0,     0,     0,  1555,   206,  1556,   928,  1557,   929,
       0,     0,     0,  1558,   453,   454,   455,  1560,     0,  1561,
       0,     0,  1562,     0,     0,     0,     0,    50,     0,     0,
    1212,     0,     0,     0,   456,   457,     0,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,     0,   210,   211,   212,   213,   214,     0,     0,
       0,     0,     0,   482,     0,     0,     0,     0,     0,     0,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,    93,    94,     0,    95,   184,    97,     0,     0,     0,
     868,     0,   868,     0,   868,     0,     0,     0,     0,   868,
     246,     0,     0,   868,  1645,   868,     0,     0,   868,   107,
       0,     0,     0,     0,     0,     0,  1242,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,   197,     0,     0,    55,     0,
       0,     0,     0,   246,     0,     0,   179,   180,   181,    65,
      66,    67,   483,     0,    69,    70,  1791,  1792,  1793,     0,
     868,     0,     0,  1797,   182,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   183,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   184,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   185,     0,     0,     0,     0,   111,   112,
       0,   113,   114,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   453,   454,   455,     0,
       0,     0,   868,   868,   868,     0,     0,     0,     0,   868,
       0,     0,     0,     0,     0,     0,   456,   457,  1803,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   482,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1853,     0,     0,     0,     0,
       0,   453,   454,   455,     0,     0,  1863,     0,     0,     0,
       0,     0,  1868,     0,     0,     0,  1870,     0,     0,     0,
       0,   456,   457,     0,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     482,     0,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,  1905,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,   868,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   868,     0,     0,     0,     0,    14,   868,    15,
      16,     0,   868,     0,     0,    17,  1609,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,  1886,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,    56,    57,    58,   868,    59,    60,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,    71,    72,
      73,  1610,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
      88,    89,    90,    91,    92,     0,    93,    94,     0,    95,
      96,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,   103,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1161,
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
     109,   110,  1349,   111,   112,     0,   113,   114,     5,     6,
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
       0,    86,     0,     0,    87,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   109,   110,   686,   111,
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
     110,  1130,   111,   112,     0,   113,   114,     5,     6,     7,
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
     108,     0,   109,   110,  1175,   111,   112,     0,   113,   114,
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
       0,     0,   107,   108,     0,   109,   110,  1248,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,  1250,    47,     0,    48,     0,
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
       0,    48,     0,    49,  1425,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   183,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   184,    97,    98,     0,     0,    99,
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
       0,     0,    87,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,  1564,   111,   112,     0,
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
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1794,
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
      48,  1840,    49,     0,     0,    50,    51,     0,     0,     0,
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
     107,   108,     0,   109,   110,  1875,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,  1878,    48,     0,    49,     0,
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
     110,  1895,   111,   112,     0,   113,   114,     5,     6,     7,
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
     108,     0,   109,   110,  1912,   111,   112,     0,   113,   114,
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
       0,     0,   107,   108,     0,   109,   110,  1969,   111,   112,
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
    1972,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
       0,   109,   110,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,   554,     0,
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
       0,   825,     0,     0,     0,     0,     0,     0,     0,     0,
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
      12,    13,     0,     0,  1063,     0,     0,     0,     0,     0,
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
       0,     0,    11,    12,    13,     0,     0,  1639,     0,     0,
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
    1786,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,    11,   414,    13,     0,     0,     0,     0,     0,     0,
       0,     0,   761,     0,     0,     0,     0,     0,     0,     0,
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
     108,     0,     0,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,     0,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
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
       0,     0,   107,   185,     0,   350,     0,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,  1104,     0,    10,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,
    1121,  1122,  1123,  1124,  1125,  1126,     0,     0,   701,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1127,
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
      95,   184,    97,     0,   702,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   185,     0,     0,     0,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,   820,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,
    1125,  1126,     0,     0,  1188,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1127,    15,    16,     0,     0,
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
    1189,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   185,     0,     0,     0,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,   414,     0,     0,
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
     105,   106,     0,     0,   107,   108,     0,     0,     0,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,     0,
     233,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     482,     0,    15,    16,     0,     0,     0,     0,    17,     0,
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
       0,     0,   104,   105,   106,     0,     0,   107,   185,   453,
     454,   455,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   456,
     457,     0,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,   482,     0,
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
       0,    99,     0,     0,   100,     0,     0,   568,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   185,     0,   268,   454,   455,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,   456,   457,     0,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,   482,     0,    17,     0,    18,    19,    20,    21,
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
     106,     0,     0,   107,   185,     0,   271,     0,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   414,
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
       0,   104,   105,   106,     0,     0,   107,   108,   453,   454,
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
      99,     0,     0,   100,     0,     0,   570,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     185,   552,     0,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,   715,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1127,     0,     0,     0,     0,    15,    16,     0,
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
       0,     0,     0,    10,  1106,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,  1125,  1126,     0,     0,     0,   761,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1127,     0,
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
       9,     0,     0,     0,     0,     0,    10,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,     0,
       0,   802,     0,     0,     0,     0,     0,     0,     0,     0,
     482,     0,     0,    15,    16,     0,     0,     0,     0,    17,
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
       0,     0,     0,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,
       0,     0,     0,     0,   804,     0,     0,     0,     0,     0,
       0,     0,     0,  1127,     0,     0,    15,    16,     0,     0,
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
       0,     0,    10,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,     0,     0,     0,  1239,     0,     0,
       0,     0,     0,     0,     0,   482,     0,     0,     0,    15,
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
     105,   106,     0,     0,   107,   185,   453,   454,   455,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   456,   457,     0,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   482,     0,     0,    17,     0,
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
       0,   100,     0,     0,   589,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   185,   453,
     454,   455,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,   829,     0,    10,   456,
     457,     0,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,   482,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,   646,    39,    40,
       0,   830,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,   179,   180,   181,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   182,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   183,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   184,    97,     0,   273,
     274,    99,   275,   276,   100,     0,   277,   278,   279,   280,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   185,     0,     0,   281,   282,   111,   112,     0,   113,
     114,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,
       0,     0,     0,   284,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1127,     0,     0,     0,   286,   287,   288,
     289,   290,   291,   292,     0,     0,     0,   206,     0,   207,
      40,     0,     0,     0,     0,     0,     0,     0,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,    50,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   206,   327,     0,   748,   329,   330,   331,
       0,     0,     0,   332,   582,   210,   211,   212,   213,   214,
     583,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,   273,   274,     0,   275,   276,     0,   584,   277,   278,
     279,   280,     0,    93,    94,     0,    95,   184,    97,   337,
       0,   338,     0,     0,   339,     0,   281,   282,     0,     0,
       0,   210,   211,   212,   213,   214,     0,     0,     0,     0,
       0,   107,     0,     0,     0,   749,     0,   111,     0,     0,
       0,     0,     0,     0,     0,   284,   598,     0,     0,    93,
      94,     0,    95,   184,    97,     0,     0,     0,     0,   286,
     287,   288,   289,   290,   291,   292,     0,     0,     0,   206,
       0,   207,    40,     0,     0,     0,     0,   107,     0,     0,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,    50,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   206,   327,     0,   328,   329,
     330,   331,     0,     0,     0,   332,   582,   210,   211,   212,
     213,   214,   583,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,   273,   274,     0,   275,   276,     0,   584,
     277,   278,   279,   280,     0,    93,    94,     0,    95,   184,
      97,   337,     0,   338,     0,     0,   339,     0,   281,   282,
       0,   283,     0,   210,   211,   212,   213,   214,     0,     0,
       0,     0,     0,   107,     0,     0,     0,   749,     0,   111,
       0,     0,     0,     0,     0,     0,     0,   284,     0,     0,
       0,    93,    94,     0,    95,   184,    97,     0,   285,     0,
       0,   286,   287,   288,   289,   290,   291,   292,     0,     0,
       0,   206,     0,     0,     0,     0,     0,     0,     0,   107,
       0,     0,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,    50,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,     0,   327,     0,
       0,   329,   330,   331,     0,     0,     0,   332,   333,   210,
     211,   212,   213,   214,   334,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   335,     0,     0,    91,   336,     0,    93,    94,     0,
      95,   184,    97,   337,     0,   338,     0,     0,   339,   273,
     274,     0,   275,   276,     0,   340,   277,   278,   279,   280,
       0,     0,     0,     0,     0,   107,   341,     0,     0,     0,
    1765,     0,     0,     0,   281,   282,     0,   283,   457,     0,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,   284,   481,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   285,     0,   482,   286,   287,   288,
     289,   290,   291,   292,     0,     0,     0,   206,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,    50,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,     0,   327,     0,     0,   329,   330,   331,
       0,     0,     0,   332,   333,   210,   211,   212,   213,   214,
     334,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   335,     0,     0,
      91,   336,     0,    93,    94,     0,    95,   184,    97,   337,
       0,   338,     0,     0,   339,   273,   274,     0,   275,   276,
       0,   340,   277,   278,   279,   280,     0,     0,     0,     0,
       0,   107,   341,     0,     0,     0,  1835,     0,     0,     0,
     281,   282,     0,   283,     0,     0,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,   284,
     481,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     285,     0,   482,   286,   287,   288,   289,   290,   291,   292,
       0,     0,     0,   206,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,    50,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,     0,
     327,     0,   328,   329,   330,   331,     0,     0,     0,   332,
     333,   210,   211,   212,   213,   214,   334,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   335,     0,     0,    91,   336,     0,    93,
      94,     0,    95,   184,    97,   337,     0,   338,     0,     0,
     339,   273,   274,     0,   275,   276,     0,   340,   277,   278,
     279,   280,     0,     0,     0,     0,     0,   107,   341,     0,
       0,     0,     0,     0,     0,     0,   281,   282,     0,   283,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,     0,
       0,     0,     0,     0,     0,   284,     0,     0,     0,     0,
     482,     0,     0,     0,     0,     0,   285,     0,     0,   286,
     287,   288,   289,   290,   291,   292,     0,     0,     0,   206,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,    50,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,     0,   327,     0,     0,   329,
     330,   331,     0,     0,     0,   332,   333,   210,   211,   212,
     213,   214,   334,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   335,
       0,     0,    91,   336,     0,    93,    94,     0,    95,   184,
      97,   337,     0,   338,     0,     0,   339,     0,   273,   274,
       0,   275,   276,   340,  1568,   277,   278,   279,   280,     0,
       0,     0,     0,   107,   341,     0,     0,     0,     0,     0,
       0,     0,     0,   281,   282,     0,   283, -1061, -1061, -1061,
   -1061,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,     0,     0,     0,     0,
       0,     0,   284,     0,     0,     0,     0,   482,     0,     0,
       0,     0,     0,   285,     0,     0,   286,   287,   288,   289,
     290,   291,   292,     0,     0,     0,   206,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,    50,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,     0,   327,     0,     0,   329,   330,   331,     0,
       0,     0,   332,   333,   210,   211,   212,   213,   214,   334,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   335,     0,     0,    91,
     336,     0,    93,    94,     0,    95,   184,    97,   337,     0,
     338,     0,     0,   339,  1665,  1666,  1667,  1668,  1669,     0,
     340,  1670,  1671,  1672,  1673,     0,     0,     0,     0,     0,
     107,   341,     0,     0,     0,     0,     0,     0,  1674,  1675,
    1676,     0,   456,   457,     0,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,  1677,   481,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   482,  1678,  1679,  1680,  1681,  1682,  1683,  1684,     0,
       0,     0,   206,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1685,  1686,  1687,  1688,  1689,  1690,  1691,
    1692,  1693,  1694,  1695,    50,  1696,  1697,  1698,  1699,  1700,
    1701,  1702,  1703,  1704,  1705,  1706,  1707,  1708,  1709,  1710,
    1711,  1712,  1713,  1714,  1715,  1716,  1717,  1718,  1719,  1720,
    1721,  1722,  1723,  1724,  1725,     0,     0,     0,  1726,  1727,
     210,   211,   212,   213,   214,     0,  1728,  1729,  1730,  1731,
    1732,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1733,  1734,  1735,     0,     0,     0,    93,    94,
       0,    95,   184,    97,  1736,     0,  1737,  1738,     0,  1739,
       0,     0,     0,     0,     0,     0,  1740,  1741,     0,  1742,
       0,  1743,  1744,     0,   273,   274,   107,   275,   276,  1103,
       0,   277,   278,   279,   280,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1104,   281,
     282,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1124,  1125,  1126,     0,     0,     0,     0,     0,   284,     0,
       0,     0,     0,     0,     0,     0,  1127,     0,     0,     0,
       0,     0,   286,   287,   288,   289,   290,   291,   292,     0,
       0,     0,   206,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,    50,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,     0,   327,
       0,   328,   329,   330,   331,     0,     0,     0,   332,   582,
     210,   211,   212,   213,   214,   583,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   273,   274,     0,   275,
     276,     0,   584,   277,   278,   279,   280,     0,    93,    94,
       0,    95,   184,    97,   337,     0,   338,     0,     0,   339,
       0,   281,   282,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   107,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     284,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   286,   287,   288,   289,   290,   291,
     292,     0,     0,     0,   206,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,    50,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
       0,   327,     0,  1294,   329,   330,   331,     0,     0,     0,
     332,   582,   210,   211,   212,   213,   214,   583,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   273,   274,
       0,   275,   276,     0,   584,   277,   278,   279,   280,     0,
      93,    94,     0,    95,   184,    97,   337,     0,   338,     0,
       0,   339,     0,   281,   282,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   107,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   284,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   286,   287,   288,   289,
     290,   291,   292,     0,     0,     0,   206,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,    50,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,     0,   327,     0,     0,   329,   330,   331,     0,
       0,     0,   332,   582,   210,   211,   212,   213,   214,   583,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   584,     0,     0,     0,
       0,     0,    93,    94,     0,    95,   184,    97,   337,     0,
     338,     0,     0,   339,   453,   454,   455,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     107,     0,     0,     0,   456,   457,     0,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,   453,   454,   455,     0,     0,     0,     0,     0,
       0,     0,     0,   482,     0,     0,     0,     0,     0,     0,
       0,     0,   456,   457,     0,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
     453,   454,   455,     0,     0,     0,     0,     0,     0,     0,
       0,   482,     0,     0,     0,     0,     0,     0,     0,     0,
     456,   457,     0,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,     0,   481,     0,     0,
    1302,     0,     0,     0,     0,     0,     0,     0,     0,   482,
       0,     0,     0,     0,   206,     0,     0,     0,   847,   848,
       0,     0,     0,     0,   849,     0,   850,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,   851,     0,
       0,     0,   593,     0,   357,   358,    34,    35,    36,   206,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   208,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,   210,   211,   212,   213,   214,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   794,
       0,     0,     0,     0,   359,     0,     0,   360,     0,     0,
      93,    94,     0,    95,   184,    97,   852,   853,   854,   855,
     856,   857,     0,    81,    82,    83,    84,    85,     0,   361,
       0,     0,     0,     0,   215,  1057,     0,     0,   107,   183,
      89,    90,    91,    92,     0,    93,    94,   817,    95,   184,
      97,     0,     0,     0,    99,     0,     0,     0,     0,     0,
       0,     0,     0,   858,     0,     0,     0,    29,   104,     0,
       0,     0,     0,   107,   859,    34,    35,    36,   206,     0,
     207,    40,     0,     0,     0,     0,     0,     0,   208,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   209,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,  1124,  1125,  1126,  1058,    75,   210,   211,   212,   213,
     214,     0,    81,    82,    83,    84,    85,  1127,     0,     0,
       0,     0,     0,   215,     0,     0,     0,     0,   183,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   184,    97,
       0,   847,   848,    99,     0,     0,     0,   849,     0,   850,
       0,     0,     0,     0,     0,     0,     0,   104,     0,     0,
       0,   851,   107,   216,     0,     0,     0,     0,   111,    34,
      35,    36,   206,     0,     0,     0,   453,   454,   455,     0,
       0,     0,   208,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,   456,   457,     0,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,     0,     0,     0,     0,     0,   852,
     853,   854,   855,   856,   857,   482,    81,    82,    83,    84,
      85,     0,     0,     0,  1011,  1012,     0,   215,     0,     0,
       0,     0,   183,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   184,    97,  1013,     0,     0,    99,     0,     0,
       0,     0,  1014,  1015,  1016,   206,   858,     0,     0,     0,
       0,   104,     0,     0,     0,  1017,   107,   859,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,   529,     0,     0,    29,     0,     0,     0,     0,
       0,     0,     0,    34,    35,    36,   206,     0,   207,    40,
       0,     0,     0,     0,     0,     0,   208,     0,     0,     0,
       0,     0,  1018,  1019,  1020,  1021,  1022,  1023,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1024,     0,     0,     0,   209,   183,     0,     0,    91,    92,
       0,    93,    94,     0,    95,   184,    97,     0,     0,     0,
       0,     0,     0,    75,   210,   211,   212,   213,   214,  1025,
      81,    82,    83,    84,    85,     0,     0,     0,     0,   107,
       0,   215,     0,     0,     0,     0,   183,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   184,    97,    29,     0,
       0,    99,     0,     0,     0,     0,    34,    35,    36,   206,
       0,   207,    40,     0,     0,   104,     0,     0,     0,   208,
     107,   216,     0,     0,   609,     0,   111,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   209, -1061, -1061,
   -1061, -1061,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,  1125,  1126,   629,    75,   210,   211,   212,
     213,   214,     0,    81,    82,    83,    84,    85,  1127,     0,
       0,     0,     0,     0,   215,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,    29,  1000,     0,    99,     0,     0,     0,     0,    34,
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
       0,  1154,    75,   210,   211,   212,   213,   214,     0,    81,
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
       0,     0,     0,    99,     0,     0,   453,   454,   455,     0,
       0,     0,     0,     0,     0,     0,     0,   104,     0,     0,
       0,     0,   107,   216,     0,     0,   456,   457,   111,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,   453,   454,   455,     0,     0,     0,
       0,     0,     0,     0,     0,   482,     0,     0,     0,     0,
       0,     0,     0,     0,   456,   457,     0,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,     0,     0,     0,     0,     0,     0,     0,     0,
     453,   454,   455,   482,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     456,   457,   538,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,     0,   481,   453,   454,
     455,     0,     0,     0,     0,     0,     0,     0,     0,   482,
       0,     0,     0,     0,     0,     0,     0,     0,   456,   457,
     918,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,     0,     0,     0,     0,
       0,     0,     0,     0,   453,   454,   455,   482,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   456,   457,   986,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,   453,   454,   455,     0,     0,     0,     0,     0,
       0,     0,     0,   482,     0,     0,     0,     0,     0,     0,
       0,     0,   456,   457,  1042,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
       0,     0,     0,     0,     0,     0,     0,  1101,  1102,  1103,
       0,   482,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1104,     0,
    1347,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1124,  1125,  1126,     0,     0,  1101,  1102,  1103,     0,     0,
       0,     0,     0,     0,     0,     0,  1127,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1104,     0,  1378,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,
    1126,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1101,  1102,  1103,     0,  1127,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1104,     0,  1276,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,
    1121,  1122,  1123,  1124,  1125,  1126,     0,     0,  1101,  1102,
    1103,     0,     0,     0,     0,     0,     0,     0,     0,  1127,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1104,
       0,  1446,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,  1124,  1125,  1126,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1101,  1102,  1103,     0,  1127,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1104,     0,  1457,  1105,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,     0,
       0,  1101,  1102,  1103,     0,     0,     0,     0,     0,     0,
       0,     0,  1127,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1104,     0,  1554,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,  1122,  1123,  1124,  1125,  1126,     0,    34,    35,
      36,   206,     0,   207,    40,     0,     0,     0,     0,     0,
    1127,   208,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,  1646,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   237,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     238,     0,     0,     0,     0,     0,     0,     0,     0,   210,
     211,   212,   213,   214,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   215,  1648,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   210,
     211,   212,   213,   214,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   215,     0,     0,     0,
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,     0,     0,     0,    99,     0,     0,     0,
       0,     0,   453,   454,   455,     0,     0,     0,     0,     0,
     104,     0,     0,     0,     0,   107,   239,     0,     0,     0,
       0,   111,   456,   457,   983,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
     453,   454,   455,     0,     0,     0,     0,     0,     0,     0,
       0,   482,     0,     0,     0,     0,     0,     0,     0,     0,
     456,   457,     0,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,     0,   481,  1101,  1102,
    1103,     0,     0,     0,     0,     0,     0,     0,     0,   482,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1104,
    1462,     0,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,  1124,  1125,  1126,  1101,  1102,  1103,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1127,     0,     0,
       0,     0,     0,     0,     0,  1104,     0,     0,  1105,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,
    1102,  1103,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1127,     0,     0,     0,     0,     0,     0,
    1104,     0,     0,  1105,  1106,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,  1125,  1126,   455,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1127,     0,
       0,     0,     0,   456,   457,     0,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   482
};

static const yytype_int16 yycheck[] =
{
       5,     6,   129,     8,     9,    10,    11,    12,    13,   161,
      15,    16,    17,    18,     4,   678,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    56,    31,     4,   187,     4,
      98,   407,   407,    33,   102,   103,    30,     4,     4,    44,
     407,   704,   108,   959,  1178,    57,    46,    52,   542,    54,
     523,    51,    57,   674,    59,   948,    30,   675,   161,    56,
     128,   188,   166,   828,   108,  1339,    30,   517,   518,   481,
     603,   604,    57,   758,   240,   653,   513,   835,   513,  1165,
      30,    86,   806,  1056,   108,   979,   108,     9,   357,   358,
     359,    32,   361,    48,   554,     9,    32,     9,   548,     4,
       9,   995,    14,   108,   251,     9,     9,    60,     9,    48,
      14,    14,     9,   550,     9,   550,     9,     9,     9,    48,
       9,    48,     9,     9,     9,     9,    70,     9,     9,    48,
     590,     9,    70,     9,     9,    88,     9,     9,    91,    90,
       9,   185,     9,    36,   115,    70,   106,   107,   252,  1043,
      81,   164,    54,   160,   106,   107,    90,  1779,    38,    83,
      31,   185,   102,   185,    38,     4,     0,    50,    51,   134,
     135,   161,   216,   134,   135,    83,    84,   196,   120,    50,
     185,     4,    53,    38,    38,   198,   199,   192,   130,   196,
      38,    70,   216,  1174,   216,   239,    38,    19,    20,   130,
      70,   134,   135,    83,  1097,    70,   157,   178,    70,    83,
      70,   216,    70,    70,  1836,   239,   676,    70,    70,    70,
     236,    70,   160,   157,   164,    44,   889,    70,    83,    83,
      53,   196,    70,    56,   239,    83,   160,    70,   193,   200,
     200,    83,   197,    70,    70,   392,    83,    84,   200,   254,
      73,    54,   257,    83,   193,   199,    14,    70,   199,   264,
     265,   199,   198,   165,   134,   135,   796,    86,   197,   181,
     197,   196,   197,    96,   998,    98,   198,   199,   197,   102,
     103,   258,  1255,   197,   443,   262,   198,  1473,   197,  1375,
     129,  1565,     8,   201,   198,   198,  1382,   198,  1384,   182,
     174,   198,   160,   198,   197,   128,   198,   198,  1066,   198,
    1068,   198,   198,   198,   198,  1209,   198,   198,   349,   174,
     198,   197,   197,    70,   197,   197,   174,  1413,   197,   199,
     197,   196,   174,   160,   199,   379,   196,   199,   196,   199,
    1233,   199,   199,   196,   102,   103,   199,   199,   199,   188,
     199,   175,   349,   886,   887,   379,   199,   379,   976,   937,
     820,   199,   165,   196,   201,   825,   525,   485,   495,   437,
     196,   201,   199,   378,   379,   196,   102,   180,   201,   160,
     385,   386,   387,   388,   389,   390,   199,   160,  1574,   160,
     395,    83,  1373,   164,    83,    14,   514,    83,    83,   160,
     181,   519,   196,   434,   357,   358,   359,   360,   361,   414,
     181,   233,  1598,    32,  1600,   196,    83,   422,   199,   181,
     181,   196,   490,   491,   492,   493,   199,   181,   433,    83,
     496,   421,    51,  1519,   196,   258,    90,   434,   164,   262,
     196,   914,   196,   266,    83,   196,   399,    83,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   102,   482,  1163,   484,
     485,   486,   174,  1013,   496,   174,  1251,   481,   174,   174,
     487,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   158,   159,   199,   481,   413,   514,
     515,   496,   517,   518,   519,   520,   532,   481,   157,   158,
     159,   526,   158,   159,   529,   192,   349,  1508,   965,  1510,
     965,   481,   199,   538,   681,   540,   683,   487,   164,   536,
     692,   196,   992,   548,   556,   196,   368,    83,    70,   196,
     204,   556,   196,   558,    90,   377,  1177,   379,    83,   378,
      83,  1454,   384,   513,    70,    90,   160,    90,   387,   388,
     102,   390,   948,   948,   396,  1193,   196,    31,  1196,   160,
     196,   948,   421,   407,   561,   751,   536,   181,  1000,   692,
     119,   120,   121,   122,   123,   124,    50,   547,   421,    53,
     550,   760,    83,  1063,   609,   199,  1139,   430,   877,    90,
     879,   434,   134,   135,   437,   196,   166,   661,   784,   635,
     636,   157,   158,   159,     4,   105,   106,   107,   134,   135,
      50,    51,   164,   158,   159,   158,   159,  1618,   160,    19,
      20,  1622,   710,   181,  1537,   598,   166,    70,   664,   199,
     603,   604,    70,   800,   801,   494,   661,    70,   196,   179,
     807,   808,   191,   487,   487,   488,   489,   490,   491,   492,
     493,    70,  1946,   111,   196,    57,   157,   158,   159,   199,
     205,   119,   120,   121,   122,   123,   124,    69,   165,   513,
     513,    75,    76,    83,   860,  1225,  1226,   702,  1383,  1229,
      90,   523,   692,   807,    31,  1235,   872,    83,    70,   181,
     715,   196,   536,   536,    90,    75,    76,  1211,    53,    54,
      55,  1097,  1097,   547,   196,   181,   550,   550,   108,   199,
    1097,   747,   393,   121,    69,   388,   397,   390,   561,    32,
     196,  1362,   130,  1094,   749,  1096,   119,   120,   121,   122,
     123,   124,  1202,   191,    81,   105,   106,   107,   581,  1377,
     102,   103,   423,  1213,   425,   426,   427,   428,   158,   159,
     675,    53,    54,    55,   779,    57,   103,   122,   123,   124,
     196,  1254,   158,   159,   607,   608,  1228,    69,  1230,   164,
    1924,    50,    51,    52,    53,    54,    55,  1778,   125,   132,
     133,  1782,    38,  1488,  1938,   185,   198,   812,   357,   358,
      69,   138,   139,   140,   141,   142,   143,   144,   191,   642,
     643,   837,    70,   828,   824,   198,   198,   843,   818,   112,
     113,   114,   198,   199,   161,   198,   216,   164,   165,   198,
     167,   168,   198,   170,   171,   172,   198,   199,  1810,  1811,
    1806,  1807,   198,   233,   845,   846,   833,  1233,  1233,   239,
     198,  1391,   198,  1393,   796,    70,  1233,    70,   195,  1329,
    1343,    70,    70,    70,    70,   199,   160,   196,   258,   701,
     196,    70,   262,   160,   874,   196,   164,   710,   198,    49,
    1575,    69,   181,   160,   910,   196,   196,   203,     9,   160,
     160,   196,  1520,     8,   198,   196,   160,    14,   160,    19,
      20,   198,   198,   918,   199,   920,     9,   922,   198,    14,
     130,   130,   197,   181,   877,    14,   879,   102,   933,  1379,
     202,   999,   197,   886,   887,   197,     9,  1918,   111,   761,
     197,   197,   947,   196,   157,   196,   936,   196,    94,     9,
     197,   197,  1933,   197,   951,  1415,   197,    14,   198,   936,
     181,   936,   196,     9,   196,    83,   199,   957,   973,   936,
     936,  1431,   198,   796,  1890,   798,   199,   197,   983,   818,
     802,   986,   804,   988,   198,   132,   198,   992,   368,   196,
     198,   197,   203,   197,  1910,   818,  1526,   377,  1528,   379,
    1530,   951,   197,  1919,   384,  1535,  1000,   198,   830,   832,
     833,     9,     9,   963,    70,   965,   396,   203,  1184,   203,
     203,  1037,   203,    32,   133,   180,  1000,   160,   136,     9,
     197,   936,   160,    14,   193,   874,  1000,  1042,     9,     9,
     182,   421,     9,   197,   132,    14,    81,   203,   200,  1049,
    1000,   874,   203,     9,   959,    14,  1506,   197,   203,   197,
     883,   884,  1052,   160,  1054,   196,   203,  1083,   103,   102,
     197,   976,   198,  1050,  1090,   198,   898,     9,  1454,  1454,
    1540,  1013,   136,   160,     9,   197,    70,  1454,   911,  1549,
    1156,    70,   914,   915,  1044,   196,   196,   936,    70,  1629,
     199,     9,  1099,  1563,   139,   140,   141,   142,   143,    70,
      70,    14,   182,   936,   200,   198,    14,     9,   957,   199,
     199,   203,   197,   233,   948,    14,   161,   951,   951,   164,
     198,   193,   167,   168,   957,   170,   171,   172,    32,   963,
     963,   965,   965,   523,  1156,   119,   120,   121,   122,   123,
     124,  1156,   196,   196,    32,    14,   130,   131,   196,   196,
     195,  1537,  1537,    56,    14,   200,    52,  1830,   196,    70,
    1537,    70,  1162,    70,    70,    70,   999,   196,   160,  1639,
       9,   561,   197,   196,  1189,  1162,  1139,  1162,  1011,  1012,
    1013,   198,   198,   136,    14,  1162,  1162,  1202,   136,   173,
       9,   197,   160,    69,   182,     9,    83,  1223,  1213,  1214,
     203,   200,   200,  1052,     9,  1054,   136,   191,    14,   196,
    1044,  1044,   198,   196,   199,    83,   136,  1050,     9,  1052,
     198,  1054,   197,   196,  1056,  1057,    91,   199,   157,   196,
      32,    77,   197,  1243,   198,  1908,  1251,   199,   199,   198,
     203,   197,  1075,   198,  1784,  1785,  1261,  1162,   368,   182,
     136,    32,   197,   197,   203,     9,  1282,   377,  1258,  1246,
    1286,     9,   203,  1097,   384,  1291,  1099,   203,     9,   203,
     136,  1944,  1298,   197,   200,   203,   396,     9,  1193,   198,
     197,  1196,   198,  1225,  1226,  1227,  1228,  1229,  1230,    14,
     200,    83,     9,  1235,   136,  1128,   196,   198,   119,   120,
     121,   122,   123,   124,   199,   197,  1776,   197,   197,   130,
     131,   701,   196,  1162,   199,   197,  1786,   197,     9,   136,
      78,    79,    80,   197,   203,   203,   203,  1489,   203,  1162,
     203,     9,  1347,    91,    32,   198,   197,   136,   197,  1354,
     198,  1341,     6,  1358,   198,  1360,   199,  1484,   112,   169,
     171,  1351,   173,  1368,   198,   165,  1188,    14,   117,    83,
     197,  1831,   197,  1378,  1379,   186,   199,   188,   136,   197,
     191,   761,  1398,   136,    14,   199,  1402,   181,    83,    14,
     283,    14,   285,  1409,    48,   198,    83,   145,   146,   147,
     148,   149,  1225,  1226,  1227,  1228,  1229,  1230,   156,  1233,
     197,   196,  1235,   523,   162,   163,  1876,  1239,    81,  1258,
     197,    14,   802,  1246,   804,   136,   136,    14,   176,   198,
     198,   198,  1254,  1255,    14,  1258,  1341,  1596,   818,   199,
     103,     9,   190,     9,   200,  1268,  1351,    68,   341,    83,
     830,   181,    83,   833,   196,     9,   199,   198,   112,  1391,
     115,  1393,   102,   117,   160,   119,   120,   121,   122,   123,
     124,   125,  1377,   102,  1464,   172,   139,   140,   141,   142,
     143,   182,    14,  1943,    36,   197,   178,   196,  1948,   182,
     198,   196,   182,    83,   874,   175,     9,   197,  1503,  1489,
    1568,  1506,   165,   197,   167,   168,   169,   170,   171,   172,
      83,   197,  1335,   167,   168,   198,   170,   195,   898,  1509,
     199,  1343,    14,    83,     9,  1515,    14,  1517,    83,  1139,
      14,    14,   195,   196,   914,   915,    83,   191,   431,    83,
    1899,   434,   490,   493,   488,   993,   200,  1915,   939,  1539,
    1252,  1541,  1638,  1910,  1625,  1663,   936,  1748,   611,  1576,
    1550,  1479,  1428,  1955,  1760,  1931,  1621,   389,  1391,  1546,
    1393,  1095,  1229,  1166,  1571,  1475,  1091,   957,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,   701,  1224,  1012,  1526,  1225,  1528,  1039,  1530,   963,
     845,   385,  1942,  1535,  1509,  1764,   434,  1957,  1613,  1865,
    1515,  1467,  1517,  1147,  1076,  1520,  1128,    -1,    -1,    -1,
      -1,    -1,    -1,    59,    60,  1464,    -1,    -1,    -1,    -1,
    1454,    -1,    -1,    -1,  1539,    -1,    -1,  1627,    -1,    -1,
      -1,  1464,    -1,    -1,  1483,    -1,  1636,    -1,    -1,    -1,
    1473,   761,  1642,    19,    20,    -1,  1479,    -1,    -1,  1649,
    1637,  1638,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
    1050,    -1,  1052,    -1,  1054,    -1,  1056,  1057,    -1,    -1,
      -1,    -1,   575,    -1,   577,    -1,    -1,    -1,    -1,    -1,
      56,    -1,   802,    -1,   804,    -1,    -1,  1629,   134,   135,
      -1,    -1,  1541,  1526,    -1,  1528,    -1,  1530,    -1,    -1,
      -1,  1550,  1535,  1537,    -1,    -1,    -1,    -1,  1541,    -1,
     830,    -1,  1627,  1546,    -1,    -1,    -1,  1550,  1759,    -1,
      -1,  1883,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,  1568,    57,    -1,  1571,    -1,
      -1,  1574,    -1,    -1,    -1,    -1,  1824,    -1,    69,    -1,
      -1,  1584,  1759,    -1,  1769,  1604,    -1,    -1,  1591,    -1,
      -1,    -1,    -1,    -1,    -1,  1598,  1903,  1600,   671,   672,
      -1,    -1,  1162,  1606,  1800,  1801,    -1,   680,   898,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    81,  1636,    83,    84,
    1790,    -1,    -1,  1642,   914,   915,  1629,    -1,  1188,    -1,
    1649,    -1,    -1,  1636,  1637,  1638,    -1,    -1,   103,  1642,
      -1,    -1,    -1,    -1,    -1,    -1,  1649,    68,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      81,    -1,  1832,    -1,    -1,    -1,    -1,    69,    -1,  1839,
      -1,    -1,  1784,  1785,   139,   140,   141,   142,   143,  1239,
      -1,    -1,   103,    -1,    -1,    -1,  1246,   233,    -1,    -1,
     111,    -1,    -1,    -1,  1254,  1255,    -1,    -1,  1258,    -1,
     165,    -1,   167,   168,  1874,   170,   171,   172,    -1,    -1,
      -1,    -1,    -1,  1883,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,  1896,    -1,    -1,    -1,
     195,    -1,    -1,    -1,   199,    -1,   201,   283,    -1,   285,
     161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,    -1,  1759,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   185,   838,  1056,  1057,  1953,    -1,
      -1,  1790,   845,   846,   195,   196,    -1,  1780,    -1,    -1,
    1965,  1784,  1785,  1343,    -1,    -1,  1956,  1790,    -1,    -1,
    1975,  1961,    -1,  1978,    -1,   341,  1799,    -1,    -1,    -1,
      -1,    -1,    -1,  1806,  1807,  1890,    -1,  1810,  1811,    -1,
      -1,     6,    56,  1832,    -1,    -1,    -1,    -1,    -1,    -1,
    1839,  1824,   368,    -1,    -1,  1910,    -1,    -1,    -1,  1832,
      -1,   377,    -1,    -1,  1919,    -1,  1839,    -1,   384,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     396,    -1,    -1,    48,    81,  1874,    -1,    -1,    -1,    -1,
      -1,   407,    -1,    -1,    -1,   938,    -1,    -1,    -1,    -1,
      -1,  1874,    -1,    -1,    -1,    -1,   103,  1896,    -1,  1882,
      -1,   954,    -1,    -1,    -1,   431,    -1,    -1,   434,    -1,
      -1,    -1,    -1,  1896,   967,    -1,     6,    -1,  1188,  1902,
      -1,   128,    -1,    -1,  1464,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   112,    -1,    -1,
      -1,    -1,   117,   996,   119,   120,   121,   122,   123,   124,
     125,    -1,    -1,    -1,    -1,   481,    -1,  1956,    48,    -1,
     167,   168,  1961,   170,   171,   172,    -1,    -1,    -1,  1239,
      -1,    -1,    -1,  1956,    -1,    -1,    -1,    -1,  1961,    -1,
      -1,    -1,    19,    20,  1254,  1255,    -1,    -1,   195,   196,
      -1,    -1,   167,   168,    -1,   170,    -1,   523,    -1,    -1,
      -1,  1541,    -1,    -1,    81,    -1,  1546,    -1,    -1,    -1,
    1550,    -1,    -1,    -1,    -1,     6,   191,  1070,    -1,    -1,
      -1,  1074,   112,  1076,    -1,   200,   103,   117,    -1,   119,
     120,   121,   122,   123,   124,   125,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,   575,
      -1,   577,    -1,    -1,   580,    -1,    -1,    48,    -1,   283,
      -1,   285,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      -1,    -1,     6,  1343,    -1,    -1,    -1,   167,   168,    -1,
     170,    59,    60,    -1,    -1,    -1,    -1,   613,    -1,    -1,
     167,   168,    -1,   170,   171,   172,  1636,  1637,  1638,    -1,
      -1,   191,  1642,    -1,    -1,    -1,    -1,    -1,    -1,  1649,
     200,    -1,    -1,    -1,    48,    -1,    -1,   341,   195,   196,
      -1,   112,    -1,  1176,    -1,  1178,   117,    -1,   119,   120,
     121,   122,   123,   124,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   671,   672,    -1,    -1,    -1,
      -1,    -1,  1205,    -1,   680,  1208,   134,   135,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,   701,   167,   168,   112,   170,
      -1,    -1,    -1,   117,    -1,   119,   120,   121,   122,   123,
     124,   125,    -1,    -1,    -1,    -1,   233,    -1,    -1,    -1,
     191,    -1,    -1,    -1,    59,    60,    -1,   431,    -1,   200,
     434,    -1,    -1,    -1,  1267,    -1,    -1,    -1,    -1,   197,
    1273,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,   167,   168,   761,   170,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
    1790,    -1,    -1,    -1,    -1,    -1,    -1,   191,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,    -1,    -1,
     796,    -1,    -1,    -1,    -1,    -1,   802,    -1,   804,   134,
     135,    -1,    -1,  1336,  1337,   139,   140,   141,   142,   143,
      -1,    -1,  1832,    31,    -1,    -1,    -1,    -1,    -1,  1839,
      -1,    -1,    -1,    -1,   830,   831,    -1,    -1,    -1,    -1,
     164,    -1,   838,   167,   168,    -1,   170,   171,   172,   845,
     846,   847,   848,   849,   850,   851,    -1,    -1,    -1,    -1,
      -1,   368,    -1,   859,  1874,    -1,    -1,    -1,    -1,    -1,
     377,   195,   197,    81,    -1,   199,    -1,   384,     6,   875,
      -1,   575,    -1,    91,    -1,    -1,  1896,    -1,    -1,   396,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
     407,    -1,   898,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1436,    -1,  1438,   912,    -1,   914,   915,
      48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,    -1,   938,   939,    -1,    -1,  1956,    -1,    -1,    -1,
    1473,  1961,   948,   161,    -1,    -1,   164,    -1,   954,   167,
     168,  1484,   170,   171,   172,    -1,   174,    -1,    -1,    -1,
      -1,   967,    -1,     6,    -1,    -1,    -1,   671,   672,   975,
      -1,    -1,   978,    -1,   112,    -1,   680,   195,    -1,   117,
      -1,   119,   120,   121,   122,   123,   124,   125,    -1,    -1,
     996,    -1,    -1,    -1,  1000,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,   523,  1013,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1551,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   167,
     168,    -1,   170,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1574,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1056,  1057,    -1,   191,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   200,    -1,  1070,  1598,    -1,  1600,  1074,   112,
    1076,    -1,    -1,  1606,   117,    -1,   119,   120,   121,   122,
     123,   124,   125,    -1,    -1,  1091,  1092,  1093,  1094,  1095,
    1096,  1097,    -1,    -1,  1100,  1101,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,
    1126,  1127,    -1,    -1,   167,   168,    -1,   170,  1661,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1143,    -1,    -1,
      -1,   845,   846,    -1,    -1,    -1,    -1,    -1,   191,    -1,
      -1,    81,    -1,    83,    84,    -1,    -1,   200,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1176,    -1,  1178,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1188,    -1,   701,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,  1205,
      -1,    -1,  1208,    -1,    -1,    -1,    -1,    19,    20,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    -1,    30,  1225,
    1226,  1227,  1228,  1229,  1230,    -1,    -1,  1233,    -1,  1235,
    1763,    59,    60,  1239,   938,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,   761,    -1,    -1,  1780,  1254,  1255,
     954,  1257,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1267,    -1,   967,    -1,   195,  1799,  1273,    -1,   199,
    1276,   201,  1278,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   802,    -1,   804,    -1,    -1,
      -1,    -1,   996,    -1,    -1,    -1,  1302,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,    -1,
      -1,    -1,    -1,   830,    -1,  1848,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1336,  1337,    -1,    -1,  1340,    -1,    -1,  1343,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,  1070,    -1,    -1,   197,
    1074,    -1,    -1,    31,    -1,    -1,    -1,    69,    -1,    -1,
      -1,   898,    -1,    -1,    -1,  1391,    -1,  1393,    -1,    -1,
      -1,  1924,    -1,    -1,    -1,    -1,    -1,   914,   915,    -1,
      -1,    -1,    -1,  1936,    -1,  1938,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   233,    -1,    81,  1957,    -1,  1959,    -1,    -1,    -1,
    1436,   948,  1438,    -1,    -1,    -1,    -1,    -1,  1444,    -1,
    1446,    -1,  1448,    -1,    -1,   103,    -1,  1453,  1454,    -1,
      -1,  1457,    -1,  1459,    -1,    -1,  1462,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1473,  1474,    -1,
      -1,  1477,  1176,    -1,  1178,    -1,    -1,    -1,  1484,    -1,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1205,    -1,   161,  1208,    -1,   164,   165,    -1,   167,
     168,   203,   170,   171,   172,    -1,   174,    -1,    -1,    -1,
    1526,    -1,  1528,    -1,  1530,    -1,    -1,   185,    -1,  1535,
      -1,  1537,    -1,    -1,    -1,    -1,    -1,   195,   196,  1056,
    1057,    -1,    -1,    -1,    -1,  1551,    -1,    -1,  1554,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   368,    -1,    -1,    -1,
    1566,  1567,    -1,  1267,    -1,   377,    -1,    -1,  1574,  1273,
    1576,    -1,   384,    -1,    -1,    -1,    -1,    10,    11,    12,
    1097,    -1,    -1,    -1,   396,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1598,    -1,  1600,   407,    -1,    -1,    31,    -1,
    1606,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,  1629,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1336,  1337,    -1,    -1,    69,    -1,    -1,    -1,
    1646,  1647,  1648,    -1,    -1,    -1,    -1,  1653,    -1,  1655,
      -1,    -1,    -1,    -1,    -1,  1661,    -1,  1663,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   481,
      -1,  1188,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,   523,    -1,    -1,    -1,    -1,  1233,    69,    -1,    -1,
      -1,    -1,  1239,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1436,    -1,  1438,    -1,    -1,  1254,  1255,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,  1763,    57,    -1,
      -1,    -1,    -1,    -1,   197,    -1,    -1,    -1,   580,    -1,
      69,    -1,    -1,    -1,  1780,    -1,    -1,    -1,  1784,  1785,
    1484,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1799,    -1,    -1,    -1,    -1,    -1,  1805,
      -1,   613,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1816,    -1,    -1,    -1,    -1,    -1,  1822,    -1,    -1,    -1,
    1826,    -1,    -1,    -1,    -1,    -1,  1343,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1848,    -1,    -1,    -1,    -1,  1551,   200,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,  1888,    -1,    -1,    -1,    -1,    -1,    -1,   701,
      69,    -1,  1898,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,  1915,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1924,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1936,    -1,  1938,    -1,    -1,    -1,    -1,  1454,    10,    11,
      12,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   761,
      -1,  1957,    -1,  1959,    -1,    -1,    -1,  1661,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,   796,    57,    -1,    -1,    -1,    -1,
     802,    81,   804,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,   830,   831,
    1537,   200,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,   847,   848,   849,   850,   851,
      -1,    -1,    31,    -1,    -1,    19,    20,   859,    -1,   139,
     140,   141,   142,   143,    -1,    -1,    30,    -1,    -1,  1763,
      -1,    -1,    -1,   875,    -1,    -1,    -1,    59,    60,    -1,
      -1,   161,    -1,    -1,   164,    -1,    -1,   167,   168,    68,
     170,   171,   172,    -1,   174,    -1,   898,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     912,    -1,   914,   915,    -1,   195,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,   939,   200,    -1,
     119,   120,   121,   122,   123,   124,   948,    -1,    -1,    -1,
      -1,    -1,   134,   135,  1848,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   975,    -1,    -1,   978,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,    -1,  1000,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,  1013,   191,    -1,    -1,    -1,   195,   196,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1924,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1936,    -1,  1938,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1056,  1057,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1957,    -1,  1959,    -1,    -1,    -1,   233,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1091,
    1092,  1093,  1094,  1095,  1096,  1097,    -1,    -1,  1100,  1101,
    1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,  1125,  1126,  1127,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1143,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    31,    57,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    -1,    -1,    -1,    69,  1188,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,   368,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,   377,    -1,    -1,    -1,    -1,    -1,    -1,
     384,    -1,    81,  1225,  1226,  1227,  1228,  1229,  1230,    -1,
      -1,  1233,   396,  1235,    -1,    -1,    -1,  1239,   139,   140,
     141,   142,   143,   407,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1254,  1255,    -1,  1257,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,  1276,    -1,  1278,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,
    1302,    -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,   174,   200,   481,    -1,    -1,
     580,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   195,   196,  1340,    -1,
      -1,  1343,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   613,    -1,    -1,    -1,    -1,    -1,   523,
      -1,    -1,    -1,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,  1391,
      57,  1393,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   580,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,  1444,    -1,  1446,    -1,  1448,    -1,    -1,   613,
      -1,  1453,  1454,    69,    81,  1457,    -1,  1459,    -1,    -1,
    1462,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1474,    -1,    -1,  1477,   103,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      81,    69,    -1,    -1,  1526,    -1,  1528,    -1,  1530,    -1,
      -1,   198,    -1,  1535,    81,  1537,    -1,   701,    -1,    -1,
     167,   168,   103,   170,   171,   172,    -1,    -1,    -1,    -1,
     111,   112,  1554,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    31,  1566,  1567,    -1,    -1,   195,   196,
      -1,   831,    -1,    -1,  1576,    -1,    -1,    -1,   139,   140,
     141,   142,   143,    -1,   200,    -1,    -1,   847,   848,   849,
     850,   851,   139,   140,   141,   142,   143,   761,    -1,   859,
      68,    -1,    -1,   164,    -1,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    81,   161,    -1,    -1,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,  1629,    -1,    -1,
      -1,    -1,    -1,    -1,   195,   103,    -1,    -1,   802,    -1,
     804,    -1,    -1,    -1,  1646,  1647,  1648,    -1,   195,    -1,
      -1,  1653,   200,  1655,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1663,    -1,    -1,    -1,    -1,   830,   831,    -1,    -1,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,    81,
      -1,    -1,    -1,   847,   848,   849,   850,   851,    -1,    -1,
      -1,    -1,    -1,   161,    -1,   859,   164,   165,    -1,   167,
     168,   103,   170,   171,   172,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   975,    -1,   185,    -1,    -1,
      -1,    -1,    -1,   125,    -1,    -1,    -1,   195,   196,    -1,
      -1,    -1,    -1,    -1,   898,    -1,   138,   139,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     914,   915,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,
      -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1784,  1785,   948,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1805,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   975,    81,    -1,  1816,    -1,    -1,    -1,    -1,    -1,
    1822,    -1,    -1,    -1,  1826,    -1,    -1,    -1,    -1,    -1,
      -1,  1091,  1092,    -1,   103,  1095,  1000,    -1,    -1,    -1,
    1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,    -1,    -1,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1143,    -1,    -1,  1888,    -1,    -1,    -1,
      -1,    -1,  1056,  1057,    -1,   164,  1898,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,  1915,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,   195,  1091,  1092,  1093,
    1094,  1095,  1096,  1097,   103,    -1,  1100,  1101,  1102,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1124,  1125,  1126,  1127,    -1,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,  1143,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    -1,   164,    -1,  1257,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,  1276,    -1,  1278,    -1,
      10,    11,    12,    -1,  1188,    -1,   195,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      30,    31,  1302,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,  1233,
     139,   140,   141,   142,   143,  1239,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1254,  1255,    -1,  1257,    -1,    -1,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1276,    -1,  1278,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,  1302,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,  1343,
      -1,    -1,    -1,    -1,  1444,    81,  1446,    83,  1448,    85,
      -1,    -1,    -1,  1453,    10,    11,    12,  1457,    -1,  1459,
      -1,    -1,  1462,    -1,    -1,    -1,    -1,   103,    -1,    -1,
     200,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
    1444,    -1,  1446,    -1,  1448,    -1,    -1,    -1,    -1,  1453,
    1454,    -1,    -1,  1457,  1554,  1459,    -1,    -1,  1462,   195,
      -1,    -1,    -1,    -1,    -1,    -1,   200,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,   108,    -1,    -1,   111,    -1,
      -1,    -1,    -1,  1537,    -1,    -1,   119,   120,   121,   122,
     123,   124,   198,    -1,   127,   128,  1646,  1647,  1648,    -1,
    1554,    -1,    -1,  1653,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,
      -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,
      -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,   202,
      -1,   204,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,  1646,  1647,  1648,    -1,    -1,    -1,    -1,  1653,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,  1662,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1805,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,  1816,    -1,    -1,    -1,
      -1,    -1,  1822,    -1,    -1,    -1,  1826,    -1,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,  1888,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,  1805,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1816,    -1,    -1,    -1,    -1,    48,  1822,    50,
      51,    -1,  1826,    -1,    -1,    56,   200,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,  1850,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    -1,    96,    -1,    98,    -1,   100,
      -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,   114,  1888,   116,   117,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,   127,   128,   129,   130,
     131,   200,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
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
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     196,    -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,
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
      -1,    31,    -1,    13,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
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
     170,   171,   172,    -1,   174,    -1,   176,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,    -1,    -1,
      -1,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,     3,
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
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
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
     191,   192,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,
      -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,
      -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,
      -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    50,    51,    -1,    -1,
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
      -1,    -1,    13,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    50,
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
     205,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    71,    72,    73,
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
      -1,   175,    -1,    -1,   178,    -1,    28,    29,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,   195,    -1,    -1,    -1,   199,    -1,   201,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,   164,    -1,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,   195,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    81,   128,    -1,   130,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,   161,
      10,    11,    12,    13,    -1,   167,   168,    -1,   170,   171,
     172,   173,    -1,   175,    -1,    -1,   178,    -1,    28,    29,
      -1,    31,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,   195,    -1,    -1,    -1,   199,    -1,   201,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    68,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,    -1,
      -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,   173,    -1,   175,    -1,    -1,   178,     3,
       4,    -1,     6,     7,    -1,   185,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,
     200,    -1,    -1,    -1,    28,    29,    -1,    31,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    57,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    69,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,    -1,    -1,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,   173,
      -1,   175,    -1,    -1,   178,     3,     4,    -1,     6,     7,
      -1,   185,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   195,   196,    -1,    -1,    -1,   200,    -1,    -1,    -1,
      28,    29,    -1,    31,    -1,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    57,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    69,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,    -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   161,    -1,    -1,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,   173,    -1,   175,    -1,    -1,
     178,     3,     4,    -1,     6,     7,    -1,   185,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,   195,   196,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    29,    -1,    31,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,    -1,    -1,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,
      -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,   173,    -1,   175,    -1,    -1,   178,    -1,     3,     4,
      -1,     6,     7,   185,   186,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,    -1,
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
     125,   126,    -1,   128,    -1,    -1,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,   173,    -1,
     175,    -1,    -1,   178,     3,     4,     5,     6,     7,    -1,
     185,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
     195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    57,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   161,   162,   163,    -1,    -1,    -1,   167,   168,
      -1,   170,   171,   172,   173,    -1,   175,   176,    -1,   178,
      -1,    -1,    -1,    -1,    -1,    -1,   185,   186,    -1,   188,
      -1,   190,   191,    -1,     3,     4,   195,     6,     7,    12,
      -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    28,
      29,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
      -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,   161,    10,    11,    12,    13,    -1,   167,   168,
      -1,   170,   171,   172,   173,    -1,   175,    -1,    -1,   178,
      -1,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
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
     125,   126,    -1,   128,    -1,    -1,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,    -1,
      -1,    -1,   167,   168,    -1,   170,   171,   172,   173,    -1,
     175,    -1,    -1,   178,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     195,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    70,    -1,
      -1,    -1,   198,    -1,   111,   112,    78,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,
      -1,    -1,    -1,    -1,   161,    -1,    -1,   164,    -1,    -1,
     167,   168,    -1,   170,   171,   172,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,   186,
      -1,    -1,    -1,    -1,   156,    38,    -1,    -1,   195,   161,
     162,   163,   164,   165,    -1,   167,   168,   197,   170,   171,
     172,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   185,    -1,    -1,    -1,    70,   190,    -1,
      -1,    -1,    -1,   195,   196,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    69,    -1,    -1,
      -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    50,    51,   176,    -1,    -1,    -1,    56,    -1,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   190,    -1,    -1,
      -1,    70,   195,   196,    -1,    -1,    -1,    -1,   201,    78,
      79,    80,    81,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,    69,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    50,    51,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    70,    -1,    -1,   176,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,   185,    -1,    -1,    -1,
      -1,   190,    -1,    -1,    -1,    91,   195,   196,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    -1,    -1,    -1,   119,   161,    -1,    -1,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   185,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,   195,
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
      -1,    -1,    30,    31,   136,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
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
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,   136,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      69,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,   136,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
     190,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,
      -1,   201,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      32,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
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
     240,   115,   178,   198,   216,   218,   216,   218,   164,   218,
     223,   223,   218,   199,     9,   424,   198,   102,   164,   199,
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
     198,   223,   198,   102,   164,   198,   111,   112,   219,   220,
     221,   222,   219,   213,   348,   294,   417,    83,     9,   197,
     197,   197,   197,   197,   197,   197,   198,    50,    51,   495,
     497,   498,   132,   270,   196,     9,   197,   197,   136,   203,
       9,   423,     9,   423,   203,   203,   203,   203,    83,    85,
     213,   476,   213,    70,   200,   200,   209,   211,    32,   133,
     269,   180,    54,   165,   180,   402,   349,   136,     9,   423,
     197,   160,   506,   506,    14,   360,   289,   228,   193,     9,
     424,   506,   507,   443,   448,   443,   200,     9,   423,   182,
     453,   348,   197,     9,   424,    14,   352,   248,   132,   268,
     196,   485,   348,    32,   203,   203,   136,   200,     9,   423,
     348,   486,   196,   258,   253,   263,    14,   480,   256,   245,
      71,   453,   348,   486,   203,   200,   197,   197,   203,   200,
     197,    50,    51,    70,    78,    79,    80,    91,   138,   139,
     140,   141,   142,   143,   156,   185,   213,   376,   379,   382,
     385,   388,   408,   419,   426,   428,   429,   433,   436,   213,
     453,   453,   136,   268,   443,   448,   197,   348,   284,    75,
      76,   285,   228,   337,   228,   339,   102,    38,   137,   274,
     453,   417,   213,    32,   230,   278,   198,   281,   198,   281,
       9,   423,    91,   226,   136,   160,     9,   423,   197,   174,
     487,   488,   489,   487,   417,   417,   417,   417,   417,   422,
     425,   196,    70,    70,    70,    70,    70,   196,   417,   160,
     199,    10,    11,    12,    31,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    69,   160,   486,
     200,   408,   199,   242,   218,   218,   213,   219,   219,     9,
     424,   200,   200,    14,   453,   198,   182,     9,   423,   213,
     271,   408,   199,   467,   137,   453,    14,   348,   348,   203,
     348,   200,   209,   506,   271,   199,   401,    14,   197,   348,
     361,   460,   198,   506,   193,   200,    32,   493,   442,    38,
      83,   174,   444,   445,   447,   444,   445,   506,    38,   174,
     348,   417,   289,   196,   408,   269,   353,   249,   348,   348,
     348,   200,   196,   291,   270,    32,   269,   506,    14,   268,
     485,   412,   200,   196,    14,    78,    79,    80,   213,   427,
     427,   429,   431,   432,    52,   196,    70,    70,    70,    70,
      70,    90,   157,   196,   160,     9,   423,   197,   437,    38,
     348,   269,   200,    75,    76,   286,   337,   230,   200,   198,
      95,   198,   274,   453,   196,   136,   273,    14,   228,   281,
     105,   106,   107,   281,   200,   506,   182,   136,   160,   506,
     213,   174,   499,     9,   197,   423,   136,   203,     9,   423,
     422,   370,   371,   417,   390,   417,   418,   390,   370,   390,
     361,   363,   365,   197,   130,   214,   417,   472,   473,   417,
     417,   417,    32,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   504,    83,   243,
     200,   200,   222,   198,   417,   498,   102,   103,   494,   496,
       9,   299,   197,   196,   340,   345,   348,   136,   203,   200,
     480,   299,   166,   179,   199,   397,   404,   166,   199,   403,
     136,   198,   493,   506,   360,   507,    83,   174,    14,    83,
     486,   453,   348,   197,   289,   199,   289,   196,   136,   196,
     291,   197,   199,   506,   199,   198,   506,   269,   250,   415,
     291,   136,   203,     9,   423,   428,   431,   372,   373,   429,
     391,   429,   430,   391,   372,   391,   157,   361,   434,   435,
      81,   429,   453,   199,   337,    32,    77,   230,   198,   339,
     273,   467,   274,   197,   417,   101,   105,   198,   348,    32,
     198,   282,   200,   182,   506,   213,   136,   174,    32,   197,
     417,   417,   197,   203,     9,   423,   136,   203,     9,   423,
     203,   203,   203,   136,     9,   423,   197,   136,   200,     9,
     423,   417,    32,   197,   228,   198,   198,   213,   506,   506,
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
     210,   210,   210,   210,   210,   210,   210,   213,   213,   213,
     213,   213,   213,   213,   213,   213,   213,   213,   213,   213,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   215,   215,   216,   216,   217,   217,
     218,   219,   219,   219,   219,   220,   220,   221,   222,   222,
     222,   223,   223,   224,   224,   224,   225,   226,   227,   227,
     228,   228,   229,   229,   229,   229,   230,   230,   230,   231,
     230,   232,   230,   233,   230,   234,   230,   230,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   230,   235,   230,   236,   230,   230,   237,   230,   238,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   230,   230,   230,   230,   240,   239,   241,   241,   243,
     242,   244,   244,   245,   245,   246,   248,   247,   249,   247,
     250,   247,   252,   251,   253,   251,   255,   254,   256,   254,
     257,   254,   258,   254,   260,   259,   262,   261,   263,   261,
     264,   264,   265,   266,   267,   267,   267,   267,   267,   268,
     268,   269,   269,   270,   270,   271,   271,   272,   272,   273,
     273,   274,   274,   274,   275,   275,   276,   276,   277,   277,
     278,   278,   279,   279,   280,   280,   280,   280,   281,   281,
     281,   282,   282,   283,   283,   284,   284,   285,   285,   286,
     286,   287,   287,   287,   287,   287,   287,   287,   287,   288,
     288,   288,   288,   288,   288,   288,   288,   289,   289,   289,
     289,   289,   289,   289,   289,   290,   290,   290,   290,   290,
     290,   290,   290,   291,   291,   292,   292,   292,   292,   292,
     292,   293,   293,   294,   294,   294,   295,   295,   295,   295,
     296,   296,   297,   298,   299,   299,   301,   300,   302,   300,
     300,   300,   300,   303,   300,   304,   300,   300,   300,   300,
     300,   300,   300,   300,   305,   305,   305,   306,   307,   307,
     308,   308,   309,   309,   310,   310,   311,   311,   312,   312,
     312,   312,   312,   312,   312,   313,   313,   314,   315,   315,
     316,   316,   317,   317,   318,   319,   319,   319,   320,   320,
     320,   320,   321,   321,   321,   321,   321,   321,   321,   322,
     322,   322,   323,   323,   324,   324,   325,   325,   326,   326,
     327,   327,   328,   328,   328,   328,   328,   328,   328,   329,
     329,   330,   330,   330,   331,   331,   331,   331,   332,   332,
     333,   333,   334,   334,   335,   336,   336,   336,   336,   336,
     336,   337,   338,   338,   339,   339,   340,   340,   340,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   348,   348,
     348,   348,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   350,
     350,   352,   351,   353,   351,   355,   354,   356,   354,   357,
     354,   358,   354,   359,   354,   360,   360,   360,   361,   361,
     362,   362,   363,   363,   364,   364,   365,   365,   366,   367,
     367,   368,   368,   369,   369,   370,   370,   371,   371,   372,
     372,   373,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   389,
     390,   390,   391,   391,   392,   393,   394,   394,   395,   395,
     395,   395,   395,   395,   395,   395,   395,   395,   395,   396,
     396,   396,   396,   397,   398,   398,   399,   399,   400,   400,
     401,   401,   402,   403,   403,   404,   404,   404,   405,   405,
     405,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   407,   408,   408,   409,   409,   409,   409,   409,
     410,   410,   411,   411,   411,   411,   412,   412,   412,   413,
     413,   413,   414,   414,   414,   415,   415,   416,   416,   416,
     416,   416,   416,   416,   416,   416,   416,   416,   416,   416,
     416,   416,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   418,   418,   419,
     420,   420,   421,   421,   421,   421,   421,   421,   421,   422,
     422,   423,   423,   424,   424,   425,   425,   425,   425,   426,
     426,   426,   426,   426,   427,   427,   427,   427,   428,   428,
     429,   429,   429,   429,   429,   429,   429,   429,   429,   429,
     429,   429,   429,   429,   429,   430,   430,   431,   431,   432,
     432,   432,   432,   433,   433,   434,   434,   435,   435,   436,
     436,   437,   437,   438,   438,   440,   439,   441,   442,   442,
     443,   443,   444,   444,   444,   445,   445,   446,   446,   447,
     447,   448,   448,   449,   449,   450,   450,   451,   451,   452,
     452,   453,   453,   453,   453,   453,   453,   453,   453,   453,
     453,   453,   454,   454,   454,   454,   454,   454,   454,   454,
     455,   455,   455,   455,   455,   455,   455,   455,   455,   456,
     457,   457,   458,   458,   459,   459,   459,   460,   460,   461,
     461,   461,   462,   462,   462,   463,   463,   464,   464,   465,
     465,   465,   465,   465,   465,   466,   466,   466,   466,   466,
     467,   467,   467,   467,   467,   467,   468,   468,   469,   469,
     469,   469,   469,   469,   469,   469,   470,   470,   471,   471,
     471,   471,   472,   472,   473,   473,   473,   473,   474,   474,
     474,   474,   475,   475,   475,   475,   475,   475,   476,   476,
     476,   477,   477,   477,   477,   477,   477,   477,   477,   477,
     477,   477,   478,   478,   479,   479,   480,   480,   481,   481,
     481,   481,   482,   482,   483,   483,   484,   484,   485,   485,
     486,   486,   487,   487,   488,   489,   489,   489,   489,   490,
     490,   491,   491,   492,   492,   493,   493,   494,   494,   495,
     496,   496,   497,   497,   497,   497,   498,   498,   498,   499,
     499,   499,   499,   500,   500,   501,   501,   501,   501,   502,
     503,   504,   504,   505,   505,   506,   506,   506,   506,   506,
     506,   506,   506,   506,   506,   506,   507,   507
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     1,     4,     3,     0,     6,     0,     5,     3,     4,
       4,     4,     4,     6,     7,     7,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     3,     3,     1,
       2,     1,     2,     3,     4,     3,     1,     2,     1,     2,
       2,     1,     3,     1,     3,     2,     2,     2,     5,     4,
       2,     0,     1,     1,     1,     1,     3,     5,     8,     0,
       4,     0,     6,     0,    10,     0,     4,     2,     3,     2,
       3,     2,     3,     3,     3,     3,     3,     3,     5,     1,
       1,     1,     0,     9,     0,    10,     5,     0,    13,     0,
       5,     3,     3,     2,     2,     2,     2,     2,     2,     3,
       2,     2,     3,     2,     2,     0,     4,     9,     0,     0,
       4,     2,     0,     1,     0,     1,     0,     9,     0,    10,
       0,    11,     0,     9,     0,    10,     0,     8,     0,     9,
       0,     7,     0,     8,     0,     8,     0,     7,     0,     8,
       1,     1,     1,     1,     1,     2,     3,     3,     2,     2,
       0,     2,     0,     2,     0,     1,     3,     1,     3,     2,
       0,     1,     2,     4,     1,     4,     1,     4,     1,     4,
       1,     4,     3,     5,     3,     4,     4,     5,     5,     4,
       0,     1,     1,     4,     0,     5,     0,     2,     0,     3,
       0,     7,     8,     6,     2,     5,     6,     4,     0,     4,
       5,     7,     6,     6,     7,     9,     8,     6,     7,     5,
       2,     4,     5,     3,     0,     3,     4,     6,     5,     5,
       6,     8,     7,     2,     0,     1,     2,     2,     3,     4,
       4,     3,     1,     1,     2,     4,     3,     5,     1,     3,
       2,     0,     2,     3,     2,     0,     0,     4,     0,     5,
       2,     2,     2,     0,    11,     0,    12,     3,     3,     3,
       4,     4,     3,     5,     2,     2,     0,     6,     5,     4,
       3,     1,     1,     3,     4,     1,     2,     1,     1,     5,
       6,     1,     1,     4,     1,     1,     3,     2,     2,     0,
       2,     0,     1,     3,     1,     1,     1,     1,     3,     4,
       4,     4,     1,     1,     2,     2,     2,     3,     3,     1,
       1,     1,     1,     3,     1,     3,     1,     1,     1,     0,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     1,     1,     3,     5,     1,     3,     5,     4,
       3,     3,     3,     4,     3,     3,     3,     2,     2,     1,
       1,     3,     3,     1,     1,     0,     1,     2,     4,     3,
       3,     6,     2,     3,     2,     3,     6,     1,     1,     1,
       1,     1,     6,     3,     4,     6,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     5,     4,     3,     1,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     1,     5,
       0,     0,    12,     0,    13,     0,     4,     0,     7,     0,
       5,     0,     3,     0,     6,     2,     2,     4,     1,     1,
       5,     3,     5,     3,     2,     0,     2,     0,     4,     4,
       3,     2,     0,     5,     3,     2,     0,     5,     3,     2,
       0,     5,     3,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     2,     0,
       2,     0,     2,     0,     4,     4,     4,     4,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     3,
       4,     1,     2,     4,     2,     6,     0,     1,     4,     0,
       2,     0,     1,     1,     3,     1,     3,     1,     1,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
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
       1,     3,     3,     2,     4,     2,     4,     5,     5,     5,
       5,     1,     1,     1,     1,     1,     1,     3,     3,     4,
       4,     3,     1,     1,     1,     1,     3,     1,     4,     3,
       1,     1,     1,     1,     1,     3,     3,     4,     4,     3,
       1,     1,     7,     9,     7,     6,     8,     1,     2,     4,
       4,     1,     1,     1,     4,     1,     0,     1,     2,     1,
       1,     1,     3,     3,     3,     0,     1,     1,     3,     3,
       2,     3,     6,     0,     1,     4,     2,     0,     5,     3,
       3,     1,     6,     4,     4,     2,     2,     0,     5,     3,
       3,     1,     2,     0,     5,     3,     3,     1,     2,     2,
       1,     2,     1,     4,     3,     3,     6,     3,     1,     1,
       1,     4,     4,     4,     4,     4,     4,     2,     2,     4,
       2,     2,     1,     3,     3,     3,     0,     2,     5,     6,
       6,     7,     1,     2,     1,     2,     1,     4,     1,     4,
       3,     0,     1,     3,     2,     3,     1,     1,     0,     0,
       3,     1,     3,     3,     2,     0,     2,     2,     2,     2,
       1,     2,     4,     2,     5,     3,     1,     1,     0,     3,
       4,     5,     6,     3,     1,     3,     2,     1,     0,     4,
       1,     3,     2,     4,     5,     2,     2,     1,     1,     1,
       1,     3,     2,     1,     8,     6,     1,     0
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
#line 6879 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 754 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 6887 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 761 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 6893 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 762 "hphp.y" /* yacc.c:1646  */
    { }
#line 6899 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 765 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 6905 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 766 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6911 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 767 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6917 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 768 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6923 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 769 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6929 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 770 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 6935 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 771 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 6943 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 6950 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 6956 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6962 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 6968 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 779 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6974 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6982 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 784 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6991 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 789 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7000 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 794 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7007 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 797 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7014 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 800 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7022 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 804 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7030 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 808 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7038 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 811 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7045 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 816 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7051 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 817 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7057 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 818 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7063 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 819 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7069 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 820 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7075 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 821 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7081 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 822 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7087 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 823 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7093 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 824 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7099 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 825 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7105 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 826 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7111 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 827 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7117 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7123 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 907 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7129 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 909 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7135 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 914 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7141 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 915 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7148 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 921 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7154 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 925 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7160 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 926 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7166 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 928 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7172 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 930 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7178 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 935 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7184 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 936 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7191 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 942 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7197 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 946 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7204 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 948 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7211 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 950 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7218 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 955 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7224 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 957 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7230 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 960 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7236 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 962 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7242 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 963 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7248 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 968 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7257 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 975 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7266 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 983 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7273 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 986 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7280 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 992 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7286 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 993 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7292 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 996 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7298 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 997 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7304 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 998 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7310 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 999 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7316 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1002 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7322 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1006 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7328 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1011 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7334 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1012 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7341 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1014 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7349 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1018 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7356 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1021 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7364 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1025 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7371 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1027 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7379 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1030 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7386 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1032 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7394 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1035 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7400 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1036 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7406 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1037 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7412 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7418 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1039 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7424 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1040 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7430 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1041 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7436 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1042 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7442 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7448 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1044 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7454 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1045 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7460 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1046 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7466 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1047 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7472 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1048 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7478 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1049 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7485 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1053 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7492 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7500 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1060 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7507 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1062 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 7515 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1066 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 7523 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1075 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7529 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 7535 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1079 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7541 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1080 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 7547 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1081 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 7553 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1082 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 7562 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7568 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1087 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7574 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1088 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7580 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1089 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7586 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1090 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7592 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1091 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7598 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1092 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7604 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1093 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7610 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1094 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7616 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1095 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 7622 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1096 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7628 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1097 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 7638 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1105 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);}
#line 7644 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1106 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7650 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1115 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 7656 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1116 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7662 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1120 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7669 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1122 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7677 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1128 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7683 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1129 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7689 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1133 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 7695 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1134 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7701 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1138 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 7707 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1144 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7716 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1150 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7725 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1157 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7734 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1163 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7743 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1170 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7752 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1176 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7761 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1184 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7768 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1188 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 7774 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1192 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7781 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1196 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 7787 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1202 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7794 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1205 "hphp.y" /* yacc.c:1646  */
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
#line 7812 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 1220 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7819 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 1223 "hphp.y" /* yacc.c:1646  */
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
#line 7837 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 1237 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7844 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1240 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7852 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1245 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7859 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1248 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7867 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1254 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 7873 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1257 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 7879 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1261 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7886 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1264 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7897 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1272 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7904 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1275 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7915 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1283 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7921 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1284 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 7928 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1288 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7934 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1291 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7940 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1294 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 7946 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1295 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 7952 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1296 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 7960 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1299 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 7966 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1300 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 7972 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1304 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7978 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1305 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7984 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1308 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7990 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1309 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7996 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1312 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8002 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1313 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8008 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1316 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8014 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1318 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8020 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1321 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8026 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1323 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8032 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1327 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8038 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1328 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8044 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1331 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8050 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1332 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8056 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1333 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8062 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1337 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8068 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1339 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8074 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1342 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8080 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1344 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8086 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1347 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8092 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1349 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8098 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1352 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8104 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1354 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8110 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1358 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8116 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1360 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8123 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1365 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8129 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1366 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8135 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1367 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8141 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1368 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8147 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1373 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8153 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1375 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8159 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1376 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8165 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1379 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8171 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1380 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8177 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1385 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8183 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1386 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8189 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1391 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8195 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1392 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8201 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1395 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8207 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1396 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8213 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1399 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8219 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1400 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8225 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1408 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8232 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1414 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8239 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1420 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8247 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1424 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8253 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1428 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8260 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1433 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8267 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1438 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8275 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1441 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8281 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1447 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8288 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1451 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8295 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1456 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8302 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1461 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8309 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8316 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1471 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8323 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1477 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8330 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1483 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8337 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1491 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8344 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1496 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8351 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1501 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8359 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1505 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8365 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1508 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8372 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1512 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 8379 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1516 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 8387 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1519 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8393 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1524 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8400 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1527 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8407 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1531 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8414 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1535 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8421 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1539 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8428 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1543 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8435 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1548 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8442 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1553 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8449 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1559 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8455 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1560 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8461 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1563 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,false);}
#line 8467 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1564 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),true,false);}
#line 8473 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1565 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,true);}
#line 8479 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1567 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),false, false);}
#line 8485 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1569 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),false,true);}
#line 8491 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1571 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),true, false);}
#line 8497 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1575 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8503 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1576 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 8509 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1579 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8515 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1580 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8521 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1581 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 8527 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1585 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 8533 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1587 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 8539 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1588 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 8545 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1589 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 8551 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1594 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8557 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1595 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8563 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1598 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8570 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1603 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 8576 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1609 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8582 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1610 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8588 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1613 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 8594 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1614 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 8601 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1617 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 8607 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1618 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 8614 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1620 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8621 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1623 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 8628 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1625 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8634 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1628 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8642 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1635 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8651 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1643 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8659 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1650 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8668 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1655 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 8674 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1657 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8680 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1659 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8686 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1661 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 8692 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1663 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 8698 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1664 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 8705 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1667 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 8711 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1670 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8717 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1671 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8723 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1672 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8729 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1678 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 8735 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1683 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 8742 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1686 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 8750 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1693 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 8756 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1694 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 8763 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1699 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 8770 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1702 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 8776 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1709 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 8783 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1711 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8789 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1715 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8795 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8801 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1722 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8807 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1724 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8813 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1725 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 8824 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1731 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 8830 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1733 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 8836 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1734 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 8842 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1738 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,0,(yyvsp[0]),0);}
#line 8848 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1740 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),0,(yyvsp[0]),0);}
#line 8854 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1745 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8860 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1748 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8866 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1749 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 8872 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1753 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 8878 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1754 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 8884 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1758 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 8891 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1761 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 8898 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1766 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 8905 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1771 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 8911 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1772 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 8918 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1774 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 8924 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1778 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 8930 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1779 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 8936 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1780 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 8942 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1781 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 8948 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1785 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8954 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1786 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 8960 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1787 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 8966 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1788 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 8972 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1789 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 8978 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1791 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 8984 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1793 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 8990 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1797 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 8998 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1800 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9004 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1801 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9010 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1805 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9016 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1806 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9022 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1810 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9028 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1811 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9034 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1814 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9040 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1815 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9046 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1818 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9052 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1819 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9058 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1822 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9064 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1824 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9070 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1827 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9076 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9082 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1829 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9088 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1830 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9094 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9100 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9106 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9112 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9118 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1838 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9124 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9130 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1842 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9136 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1843 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9142 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1847 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9148 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1849 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9154 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1850 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9160 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1851 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9166 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1855 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9172 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1857 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9178 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1861 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9184 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1863 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9190 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1867 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9198 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1871 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9205 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1875 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9211 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1879 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9217 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1881 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9223 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1882 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9229 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1883 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9235 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1884 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9241 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1885 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9247 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1888 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9253 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1892 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9259 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1893 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9265 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1897 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9271 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1898 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9277 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1902 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9283 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1903 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9289 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 1904 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9295 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 1905 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9301 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 1909 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9307 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 1914 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9313 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 1918 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 9319 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 1922 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9325 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 1926 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 9331 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 1930 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9337 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 1935 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9343 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 1939 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9349 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 1940 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9355 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 1941 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9361 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 1942 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9367 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 1943 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9373 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 1948 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 9379 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 1949 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 9385 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 9391 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 1953 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 9397 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 1954 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 9403 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 1955 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 9409 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 9415 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 9421 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 1958 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 9427 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 1959 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 9433 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 1960 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 9439 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 1961 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 9445 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 9451 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 9457 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 9463 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 9469 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 1966 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 9475 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 1967 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 9481 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 1968 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 9487 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 1969 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 9493 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 1970 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 9499 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 1971 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 9505 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 1972 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 9511 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 9517 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 1974 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 9523 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 1975 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 9529 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 9535 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 1977 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 9541 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 1978 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 9547 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 1979 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 9553 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 1980 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 9559 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 1981 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 9565 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 9571 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 1983 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 9577 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 1984 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 9583 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 1985 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 9589 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 9595 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 1987 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 9601 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 1988 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 9607 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 1989 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 9613 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 1990 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 9619 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 9625 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 1992 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 9631 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 1993 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 9637 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 9643 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 1995 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 9649 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 1996 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 9655 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 1997 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 9662 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 9668 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 9675 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2002 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 9681 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 9687 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9693 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2006 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 9699 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 9705 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2008 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 9711 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2009 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9717 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2010 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 9723 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2011 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 9729 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2012 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 9735 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 9741 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 9747 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 9753 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2016 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 9759 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 9765 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 9771 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2019 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9777 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2020 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9783 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2021 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9789 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2022 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9795 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2023 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9801 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2024 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9807 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2025 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9813 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2026 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9819 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 9825 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2028 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 9831 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2029 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9837 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2036 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 9843 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2037 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9849 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2042 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9858 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2048 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9869 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2056 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9878 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2062 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9889 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2072 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 9902 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2080 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         (yyvsp[-3]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-3]), nullptr, (yyvsp[-3]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-3]),
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9916 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2090 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9926 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2098 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         (yyvsp[-6]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-6]), nullptr, (yyvsp[-6]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-6]),
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9940 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2108 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9950 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2114 "hphp.y" /* yacc.c:1646  */
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
#line 9966 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2125 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 9979 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2133 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9991 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2140 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10001 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10013 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2158 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10019 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2159 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10025 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2161 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10031 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2165 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10038 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2167 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10044 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2174 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10050 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2177 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10056 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2184 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10062 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2187 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10068 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2192 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10074 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2193 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10080 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2198 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10086 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2199 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10092 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2203 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_ARRAY);}
#line 10098 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2207 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10104 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10110 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2213 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10116 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2214 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10122 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2219 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10128 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2220 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10134 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2225 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10140 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2226 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10146 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2232 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10152 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2234 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10158 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2239 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10164 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2240 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10170 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2246 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10176 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2248 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10182 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2252 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10188 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2256 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10194 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2260 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10200 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2264 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10206 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2268 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10212 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2272 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10218 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2276 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10224 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2280 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10230 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2284 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10236 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2288 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10242 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2292 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10248 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2296 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10254 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2300 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10260 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2304 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10266 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2308 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10272 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2313 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10278 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2314 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10284 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2319 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10290 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2320 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10296 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2325 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10302 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2326 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10308 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2331 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10316 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2338 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10324 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2345 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10330 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2347 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10336 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2351 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10342 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2352 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10348 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2353 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10354 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2354 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10360 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2355 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10366 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2356 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10372 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10378 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2358 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10384 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2359 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10391 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2361 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10397 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2362 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10403 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2366 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10409 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2367 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 10415 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2368 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 10421 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2369 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 10427 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2376 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 10433 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2379 "hphp.y" /* yacc.c:1646  */
    { Token t1; _p->onArray(t1,(yyvsp[-1]));
                                         Token t2; _p->onArray(t2,(yyvsp[0]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[-1]),NULL,t1,0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-1]),t2,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),file,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),line,0,0);
                                         (yyval).setText("");}
#line 10447 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2390 "hphp.y" /* yacc.c:1646  */
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray((yyvsp[-2]),(yyvsp[-5]));
                                         _p->onArray((yyvsp[-1]),(yyvsp[-3]));
                                         _p->onCallParam((yyvsp[-4]),NULL,(yyvsp[-2]),0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-4]),(yyvsp[-1]),0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),file,0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),line,0,0);
                                         (yyval).setText((yyvsp[0]).text());}
#line 10461 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2401 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 10467 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2402 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 10473 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2407 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10479 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2408 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10485 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2411 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),0,(yyvsp[0]),0);}
#line 10491 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2412 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10497 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2415 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10504 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2419 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10512 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2422 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10518 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2425 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 10530 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2432 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10536 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2433 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10542 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2437 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10548 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2439 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 10554 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2441 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 10560 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2445 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10566 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2446 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10572 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2447 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10578 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2448 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10584 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10590 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2450 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10596 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2451 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10602 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2452 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10608 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10614 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10620 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2455 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10626 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2456 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10632 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2457 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10638 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2458 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10644 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10650 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2460 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10656 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10662 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2462 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10668 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2463 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10674 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10680 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2465 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10686 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2466 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10692 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2467 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10698 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2468 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10704 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2469 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10710 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2470 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10716 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2471 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10722 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2472 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10728 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2473 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10734 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2474 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10740 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2475 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10746 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2476 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10752 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2477 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10758 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2478 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10764 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2479 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10770 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2480 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10776 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2481 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10782 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2482 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10788 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10794 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2484 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10800 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10806 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10812 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10818 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10824 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10830 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10836 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2491 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10842 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2492 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10848 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2493 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10854 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2494 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10860 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2495 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10866 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2496 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10872 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2497 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10878 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2498 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10884 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2499 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2500 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10896 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2501 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10902 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10908 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2503 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10914 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2504 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10920 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2505 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10926 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2506 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10932 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2507 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10938 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2508 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10944 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2509 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10950 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2510 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10956 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2511 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10962 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2512 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10968 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2513 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10974 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2514 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10980 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2515 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10986 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2516 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10992 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10998 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2518 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11004 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2519 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11010 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2520 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11016 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2521 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11022 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2522 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11028 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2523 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11034 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2524 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11040 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2525 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11046 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2530 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11052 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2534 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11058 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2535 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11064 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2539 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11070 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2540 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11076 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2541 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11082 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2542 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11089 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2544 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11096 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2548 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11102 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2557 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11108 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2560 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11114 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2561 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11121 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2563 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11128 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11134 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2577 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11140 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2578 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11146 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2579 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11152 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2583 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11158 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2584 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11164 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2585 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11170 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2589 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11176 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2590 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11182 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2591 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11188 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2595 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11194 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2596 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11200 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2600 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11206 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2601 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11212 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2602 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11218 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2603 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11225 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2605 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11231 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2606 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11237 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2607 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 11243 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2608 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 11249 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2609 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 11255 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2610 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 11261 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2611 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 11267 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2612 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 11273 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2613 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 11279 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2616 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11285 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2618 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11291 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2622 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11297 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2623 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11303 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2625 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11309 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2626 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11315 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2628 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11321 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2629 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11327 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2630 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11333 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2631 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11339 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2632 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11345 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2633 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11351 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2634 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11357 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2635 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11363 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11369 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2638 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 11375 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2640 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 11381 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 11387 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2644 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 11393 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 11399 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2647 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 11405 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2648 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 11411 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2649 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 11417 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 11423 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2651 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 11429 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2652 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 11435 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2653 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 11441 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2654 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 11447 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2655 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 11453 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2656 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 11459 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2657 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 11465 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2658 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 11471 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 11477 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2660 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 11483 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2661 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11489 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2662 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11495 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 11501 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2666 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 11507 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 11513 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 11519 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2671 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 11525 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2673 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 11532 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2675 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 11538 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2678 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 11545 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2682 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 11551 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2685 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 11557 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 11563 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2690 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11569 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2691 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11575 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2697 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11581 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2703 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11587 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2704 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11593 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2708 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11599 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2709 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11605 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2710 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11611 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2711 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11617 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 11623 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2713 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 11629 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2715 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 11636 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11642 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11648 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11654 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2726 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11660 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 11666 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11672 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11678 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11684 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11690 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11696 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2745 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11702 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2746 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11708 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2747 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11714 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2750 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11720 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2752 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11726 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2755 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 11732 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2756 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 11738 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2757 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 11744 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2758 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11750 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2762 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 11757 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2765 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 11765 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2772 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11771 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2773 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11777 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2776 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 11785 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2779 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11791 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2780 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11797 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2781 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11803 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2783 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11809 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2784 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11815 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2786 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11821 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2787 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11827 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2788 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11833 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2789 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11839 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2790 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11845 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2791 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11851 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2796 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11857 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2797 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11863 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2802 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11869 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2803 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11875 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2808 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11881 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2810 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11887 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2812 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11893 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2813 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11899 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2817 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11905 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2818 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11911 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2823 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11917 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2824 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 11923 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2829 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11929 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2832 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11935 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11941 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2838 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11947 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2841 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11953 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2842 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 11960 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 11966 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2851 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 11972 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2854 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 11978 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2856 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11984 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2859 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11990 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2862 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11996 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2863 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12002 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12008 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2868 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12014 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2872 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12020 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2873 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12026 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2874 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12032 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2878 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12038 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2880 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12044 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2888 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12050 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2889 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12056 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2893 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12062 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2895 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12068 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2903 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12074 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2904 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12080 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 2908 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12086 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 2910 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12092 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 2915 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12098 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 2917 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12104 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 2923 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12118 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 2934 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12132 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 2949 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12146 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 2961 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12160 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 2973 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12166 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 2974 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12172 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 2975 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12178 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 2976 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12184 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 2977 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12190 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 2978 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12196 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 2980 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12210 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 2997 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12216 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 2999 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12222 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3001 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12228 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3002 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12234 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12240 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12246 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3008 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12252 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3009 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12258 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12272 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3026 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12278 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3028 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12284 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3029 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12290 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3038 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12296 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3039 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12302 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3040 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12308 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3041 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12314 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3042 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12320 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3043 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12326 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3044 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12332 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3046 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12338 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3048 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12344 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3052 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12350 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3056 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12356 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3057 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12362 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3063 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12368 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3067 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12374 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3074 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 12380 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3083 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 12386 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3087 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 12392 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3091 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12398 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3094 "hphp.y" /* yacc.c:1646  */
    { _p->onIndirectRef((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 12404 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3100 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12410 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3101 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12416 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3102 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12422 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3106 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12428 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3107 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 12434 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3108 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 12440 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3115 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12446 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3116 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12452 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3121 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 12458 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3122 "hphp.y" /* yacc.c:1646  */
    { (yyval)++;}
#line 12464 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3127 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12470 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3128 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12476 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3129 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12482 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3132 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12496 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3143 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12502 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3144 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12508 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3148 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12514 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3149 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12520 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
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
#line 12534 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3161 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12540 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3165 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 12546 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3166 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 12552 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3168 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 12558 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3169 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 12564 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3170 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 12570 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3171 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 12576 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3176 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12582 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3177 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12588 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3181 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12594 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3182 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12600 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3183 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12606 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3184 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12612 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3187 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12618 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3189 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 12624 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3190 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12630 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3191 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 12636 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3196 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12642 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3197 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12648 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3201 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12654 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3202 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12660 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3203 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12666 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3204 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12672 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3209 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12678 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3210 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12684 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3215 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12690 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3217 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12696 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3219 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12702 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3220 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12708 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3224 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 12714 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3226 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 12720 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3227 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 12726 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3229 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 12733 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3234 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12739 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3236 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12745 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3238 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 12759 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3248 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 12765 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3250 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 12771 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3251 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12777 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3254 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 12783 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3255 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 12789 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3256 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 12795 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3260 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 12801 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3261 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 12807 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3262 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12813 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3263 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12819 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3264 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12825 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3265 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12831 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3266 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 12837 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3267 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 12843 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3268 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 12849 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3269 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 12855 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3270 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 12861 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3274 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12867 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3275 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12873 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3280 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12879 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3282 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12885 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3296 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12893 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3301 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 12901 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3305 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12909 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3310 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 12917 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3316 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12923 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3317 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12929 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3321 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12935 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3322 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12941 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3328 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12947 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3332 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 12953 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3338 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12959 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3342 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 12966 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3349 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12972 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3350 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12978 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3354 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 12986 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3357 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 12993 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3363 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12999 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3368 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]); }
#line 13005 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3369 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13011 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3370 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13017 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3371 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13023 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1035:
#line 3392 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13029 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1036:
#line 3393 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13035 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1039:
#line 3402 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13041 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3413 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13047 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3415 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13053 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3419 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13059 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3422 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13065 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3426 "hphp.y" /* yacc.c:1646  */
    {}
#line 13071 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3427 "hphp.y" /* yacc.c:1646  */
    {}
#line 13077 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3428 "hphp.y" /* yacc.c:1646  */
    {}
#line 13083 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3434 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13090 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3439 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13100 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3448 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13106 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3454 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13115 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3462 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13121 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3463 "hphp.y" /* yacc.c:1646  */
    { }
#line 13127 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3469 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13133 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3471 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13139 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3472 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13149 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3477 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13156 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("array"); }
#line 13163 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13169 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3493 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13177 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3497 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13183 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 13189 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3504 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 13195 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3510 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 13202 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3512 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 13210 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3515 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13216 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3516 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13224 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3519 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13232 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3522 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13238 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3525 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 13246 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3528 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13253 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3530 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 13262 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3536 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 13271 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3542 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 13281 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3550 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13287 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1077:
#line 3551 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13293 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;


#line 13297 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 3554 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}
