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
#define YYLAST   18236

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  206
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  302
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1075
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1976

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
     769,   770,   771,   774,   776,   776,   778,   778,   780,   782,
     785,   788,   792,   796,   800,   805,   806,   807,   808,   809,
     810,   811,   812,   813,   814,   815,   816,   817,   821,   822,
     823,   824,   825,   826,   827,   828,   829,   830,   831,   832,
     833,   834,   835,   836,   837,   838,   839,   840,   841,   842,
     843,   844,   845,   846,   847,   848,   849,   850,   851,   852,
     853,   854,   855,   856,   857,   858,   859,   860,   861,   862,
     863,   864,   865,   866,   867,   868,   869,   870,   871,   872,
     873,   874,   875,   876,   877,   878,   879,   880,   881,   882,
     883,   887,   891,   892,   896,   897,   902,   904,   909,   914,
     915,   916,   918,   923,   925,   930,   935,   937,   939,   944,
     945,   949,   950,   952,   956,   963,   970,   974,   980,   982,
     985,   986,   987,   988,   991,   992,   996,  1001,  1001,  1007,
    1007,  1014,  1013,  1019,  1019,  1024,  1025,  1026,  1027,  1028,
    1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,
    1042,  1040,  1049,  1047,  1054,  1064,  1058,  1068,  1066,  1070,
    1071,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1094,  1094,  1099,  1105,  1109,  1109,  1117,
    1118,  1122,  1123,  1127,  1133,  1131,  1146,  1143,  1159,  1156,
    1173,  1172,  1181,  1179,  1191,  1190,  1209,  1207,  1226,  1225,
    1234,  1232,  1243,  1243,  1250,  1249,  1261,  1259,  1272,  1273,
    1277,  1280,  1283,  1284,  1285,  1288,  1289,  1292,  1294,  1297,
    1298,  1301,  1302,  1305,  1306,  1310,  1311,  1316,  1317,  1320,
    1321,  1322,  1326,  1327,  1331,  1332,  1336,  1337,  1341,  1342,
    1347,  1348,  1354,  1355,  1356,  1357,  1360,  1363,  1365,  1368,
    1369,  1373,  1375,  1378,  1381,  1384,  1385,  1388,  1389,  1393,
    1399,  1405,  1412,  1414,  1419,  1424,  1430,  1434,  1438,  1442,
    1447,  1452,  1457,  1462,  1468,  1477,  1482,  1487,  1493,  1495,
    1499,  1503,  1508,  1512,  1515,  1518,  1522,  1526,  1530,  1534,
    1539,  1547,  1549,  1552,  1553,  1554,  1555,  1557,  1559,  1564,
    1565,  1568,  1569,  1570,  1574,  1575,  1577,  1578,  1582,  1584,
    1587,  1591,  1597,  1599,  1602,  1602,  1606,  1605,  1609,  1611,
    1614,  1617,  1615,  1632,  1628,  1643,  1645,  1647,  1649,  1651,
    1653,  1655,  1659,  1660,  1661,  1664,  1670,  1674,  1680,  1683,
    1688,  1690,  1695,  1700,  1704,  1705,  1709,  1710,  1712,  1714,
    1720,  1721,  1723,  1727,  1728,  1733,  1737,  1738,  1742,  1743,
    1747,  1749,  1755,  1760,  1761,  1763,  1767,  1768,  1769,  1770,
    1774,  1775,  1776,  1777,  1778,  1779,  1781,  1786,  1789,  1790,
    1794,  1795,  1799,  1800,  1803,  1804,  1807,  1808,  1811,  1812,
    1816,  1817,  1818,  1819,  1820,  1821,  1822,  1826,  1827,  1830,
    1831,  1832,  1835,  1837,  1839,  1840,  1843,  1845,  1849,  1851,
    1855,  1859,  1863,  1868,  1869,  1871,  1872,  1873,  1874,  1877,
    1881,  1882,  1886,  1887,  1891,  1892,  1893,  1894,  1898,  1902,
    1907,  1911,  1915,  1919,  1923,  1928,  1929,  1930,  1931,  1932,
    1936,  1938,  1939,  1940,  1943,  1944,  1945,  1946,  1947,  1948,
    1949,  1950,  1951,  1952,  1953,  1954,  1955,  1956,  1957,  1958,
    1959,  1960,  1961,  1962,  1963,  1964,  1965,  1966,  1967,  1968,
    1969,  1970,  1971,  1972,  1973,  1974,  1975,  1976,  1977,  1978,
    1979,  1980,  1981,  1982,  1983,  1984,  1985,  1986,  1988,  1989,
    1991,  1992,  1994,  1995,  1996,  1997,  1998,  1999,  2000,  2001,
    2002,  2003,  2004,  2005,  2006,  2007,  2008,  2009,  2010,  2011,
    2012,  2013,  2014,  2015,  2016,  2017,  2018,  2022,  2026,  2031,
    2030,  2045,  2043,  2061,  2060,  2079,  2078,  2097,  2096,  2114,
    2114,  2129,  2129,  2147,  2148,  2149,  2154,  2156,  2160,  2164,
    2170,  2174,  2180,  2182,  2186,  2188,  2192,  2196,  2197,  2201,
    2203,  2207,  2209,  2213,  2215,  2219,  2222,  2227,  2229,  2233,
    2236,  2241,  2245,  2249,  2253,  2257,  2261,  2265,  2269,  2273,
    2277,  2281,  2285,  2289,  2293,  2297,  2301,  2303,  2307,  2309,
    2313,  2315,  2319,  2326,  2333,  2335,  2340,  2341,  2342,  2343,
    2344,  2345,  2346,  2347,  2348,  2350,  2351,  2355,  2356,  2357,
    2358,  2362,  2368,  2377,  2390,  2391,  2394,  2397,  2400,  2401,
    2404,  2408,  2411,  2414,  2421,  2422,  2426,  2427,  2429,  2434,
    2435,  2436,  2437,  2438,  2439,  2440,  2441,  2442,  2443,  2444,
    2445,  2446,  2447,  2448,  2449,  2450,  2451,  2452,  2453,  2454,
    2455,  2456,  2457,  2458,  2459,  2460,  2461,  2462,  2463,  2464,
    2465,  2466,  2467,  2468,  2469,  2470,  2471,  2472,  2473,  2474,
    2475,  2476,  2477,  2478,  2479,  2480,  2481,  2482,  2483,  2484,
    2485,  2486,  2487,  2488,  2489,  2490,  2491,  2492,  2493,  2494,
    2495,  2496,  2497,  2498,  2499,  2500,  2501,  2502,  2503,  2504,
    2505,  2506,  2507,  2508,  2509,  2510,  2511,  2512,  2513,  2514,
    2518,  2523,  2524,  2528,  2529,  2530,  2531,  2533,  2537,  2538,
    2549,  2550,  2552,  2554,  2566,  2567,  2568,  2572,  2573,  2574,
    2578,  2579,  2580,  2583,  2585,  2589,  2590,  2591,  2592,  2594,
    2595,  2596,  2597,  2598,  2599,  2600,  2601,  2602,  2603,  2606,
    2611,  2612,  2613,  2615,  2616,  2618,  2619,  2620,  2621,  2622,
    2623,  2624,  2625,  2626,  2628,  2630,  2632,  2634,  2636,  2637,
    2638,  2639,  2640,  2641,  2642,  2643,  2644,  2645,  2646,  2647,
    2648,  2649,  2650,  2651,  2652,  2654,  2656,  2658,  2660,  2661,
    2664,  2665,  2669,  2673,  2675,  2679,  2680,  2684,  2690,  2693,
    2697,  2698,  2699,  2700,  2701,  2702,  2703,  2708,  2710,  2714,
    2715,  2718,  2719,  2723,  2726,  2728,  2730,  2734,  2735,  2736,
    2737,  2740,  2744,  2745,  2746,  2747,  2751,  2753,  2760,  2761,
    2762,  2763,  2768,  2769,  2770,  2771,  2773,  2774,  2776,  2777,
    2778,  2779,  2780,  2784,  2786,  2790,  2792,  2795,  2798,  2800,
    2802,  2805,  2807,  2811,  2813,  2816,  2819,  2825,  2827,  2830,
    2831,  2836,  2839,  2843,  2843,  2848,  2851,  2852,  2856,  2857,
    2861,  2862,  2863,  2867,  2869,  2877,  2878,  2882,  2884,  2892,
    2893,  2897,  2898,  2903,  2905,  2910,  2921,  2935,  2947,  2962,
    2963,  2964,  2965,  2966,  2967,  2968,  2978,  2987,  2989,  2991,
    2995,  2996,  2997,  2998,  2999,  3015,  3016,  3018,  3027,  3028,
    3029,  3030,  3031,  3032,  3033,  3034,  3036,  3041,  3045,  3046,
    3050,  3053,  3060,  3064,  3073,  3080,  3082,  3088,  3090,  3091,
    3095,  3096,  3097,  3104,  3105,  3110,  3111,  3116,  3117,  3118,
    3119,  3130,  3133,  3136,  3137,  3138,  3139,  3150,  3154,  3155,
    3156,  3158,  3159,  3160,  3164,  3166,  3169,  3171,  3172,  3173,
    3174,  3177,  3179,  3180,  3184,  3186,  3189,  3191,  3192,  3193,
    3197,  3199,  3202,  3205,  3207,  3209,  3213,  3214,  3216,  3217,
    3223,  3224,  3226,  3236,  3238,  3240,  3243,  3244,  3245,  3249,
    3250,  3251,  3252,  3253,  3254,  3255,  3256,  3257,  3258,  3259,
    3263,  3264,  3268,  3270,  3278,  3280,  3284,  3288,  3293,  3297,
    3305,  3306,  3310,  3311,  3317,  3318,  3327,  3328,  3336,  3339,
    3343,  3346,  3351,  3356,  3358,  3359,  3360,  3363,  3365,  3371,
    3372,  3376,  3377,  3381,  3382,  3386,  3387,  3390,  3395,  3396,
    3400,  3403,  3405,  3409,  3415,  3416,  3417,  3421,  3425,  3433,
    3438,  3450,  3452,  3456,  3459,  3461,  3466,  3471,  3477,  3480,
    3485,  3490,  3492,  3499,  3501,  3504,  3505,  3508,  3511,  3512,
    3517,  3519,  3523,  3529,  3539,  3540
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

#define YYPACT_NINF -1607

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1607)))

#define YYTABLE_NINF -1059

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1059)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1607,   183, -1607, -1607,  5685, 13602, 13602,    16, 13602, 13602,
   13602, 13602, 11369, 13602, -1607, 13602, 13602, 13602, 13602, 16831,
   16831, 13602, 13602, 13602, 13602, 13602, 13602, 13602, 13602, 11572,
    3239, 13602,   207,   231, -1607, -1607, -1607,   360, -1607,   188,
   -1607, -1607, -1607,   404, 13602, -1607,   231,   346,   356,   379,
   -1607,   231, 11775, 14904, 11978, -1607, 14480, 10354,   357, 13602,
   17552,   285,    48,   314,   280, -1607, -1607, -1607,   383,   385,
     391,   406, -1607, 14904,   411,   414,   545,   562,   574,   591,
     595, -1607, -1607, -1607, -1607, -1607, 13602,   570,  2079, -1607,
   -1607, 14904, -1607, -1607, -1607, -1607, 14904, -1607, 14904, -1607,
     482,   474, 14904, 14904, -1607,   217, -1607, -1607, 12181, -1607,
   -1607,   476,   706,   714,   714, -1607,   159,   504,   203,   484,
   -1607,    87, -1607,   640, -1607, -1607, -1607, -1607, 14375,   620,
   -1607, -1607,   485,   510,   524,   526,   532,   536,   550,   566,
   15672, -1607, -1607, -1607, -1607,   148,   703,   725,   730,   737,
     739, -1607,   743,   762, -1607,   174,   631, -1607,   674,    -6,
   -1607,   970,   146, -1607, -1607,  2900,   139,   644,   153, -1607,
     155,   177,   647,   187, -1607,    42, -1607,   774, -1607,   688,
   -1607, -1607,   653,   689, -1607, 13602, -1607,   640,   620, 17829,
    3436, 17829, 13602, 17829, 17829, 18093, 18093,   654, 16107, 17829,
     806, 14904,   788,   788,   109,   788, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607, -1607, -1607,    60, 13602,   678, -1607, -1607,
     700,   665,   429,   671,   429,   788,   788,   788,   788,   788,
     788,   788,   788, 16831, 16350,   663,   861,   688, -1607, 13602,
     678, -1607,   718, -1607,   723,   693, -1607,   168, -1607, -1607,
   -1607,   429,   139, -1607, 12384, -1607, -1607, 13602,  9136,   866,
      95, 17829, 10151, -1607, 13602, 13602, 14904, -1607, -1607, 15720,
     692, -1607, 15768, -1607, -1607, -1607, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607,  3881, -1607,  3881, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607,    99,    88,   689, -1607, -1607, -1607, -1607,
     696,  2350,    91, -1607, -1607,   736,   884, -1607,   744, 15199,
   -1607,   701,   705, 15838, -1607,    32, 15886, 14551, 14551, 14904,
     708,   896,   713, -1607,    55, -1607, 16419,    98, -1607,   898,
     101,   792, -1607,   794, -1607, 16831, 13602, 13602,   728,   746,
   -1607, -1607, 16522, 11572, 13602, 13602, 13602, 13602, 13602,   111,
     490,   463, -1607, 13805, 16831,   633, -1607, 14904, -1607,   242,
     504, -1607, -1607, -1607, -1607, 17441,   914,   828, -1607, -1607,
   -1607,    61, 13602,   734,   740, 17829,   756,   760,   759,  5888,
   13602,    78,   732,   722,    78,   551,   493, -1607, 14904,  3881,
     768, 10557, 14480, -1607, -1607,  2533, -1607, -1607, -1607, -1607,
   -1607,   640, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607,
   -1607, 13602, 13602, 13602, 13602, 12587, 13602, 13602, 13602, 13602,
   13602, 13602, 13602, 13602, 13602, 13602, 13602, 13602, 13602, 13602,
   13602, 13602, 13602, 13602, 13602, 13602, 13602, 13602, 13602, 17541,
   13602, -1607, 13602, 13602, 13602,  4742, 14904, 14904, 14904, 14904,
   14904, 14375,   829,   822,  5052, 13602, 13602, 13602, 13602, 13602,
   13602, 13602, 13602, 13602, 13602, 13602, 13602, -1607, -1607, -1607,
   -1607,  4068, 13602, 13602, -1607, 10557, 10557, 13602, 13602,   476,
     175, 16522,   769,   640, 12790, 15934, -1607, 13602, -1607,   773,
     949,   813,   776,   780, 13976,   429, 12993, -1607, 13196, -1607,
     693,   785,   786,  1263, -1607,   145, 10557, -1607, 15083, -1607,
   -1607, 16004, -1607, -1607, 10760, -1607, 13602, -1607,   890,  9339,
     976,   795, 13789,   980,   128,    66, -1607, -1607, -1607,   816,
   -1607, -1607, -1607,  3881, -1607,  2521,   809,   998, 16275, 14904,
   -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607,   812,
   -1607, -1607,   815,   819,   821,   823,    69, 17615, 14727, -1607,
   -1607, 14904, 14904, 13602,   429,   285, -1607, 16275,   939, -1607,
   -1607, -1607,   429,   129,   130,   835,   836,  2173,   362,   837,
     827,   539,   905,   847,   429,   131,   849, 17000,   841,  1038,
    1041,   848,   850,   851,   853, -1607, 14199, 14904, -1607, -1607,
     988,  3141,    30, -1607, -1607, -1607,   504, -1607, -1607, -1607,
    1027,   928,   883,   212,   904, 13602,   476,   929,  1057,   870,
   -1607,   908, -1607,   175, -1607,  3881,  3881,  1059,   866,    61,
   -1607,   878,  1069, -1607,  3881,   218, -1607,   466,   191, -1607,
   -1607, -1607, -1607, -1607, -1607, -1607,  1270,  3850, -1607, -1607,
   -1607, -1607,  1073,   901, -1607, 16831, 13602,   888,  1077, 17829,
    1082, -1607, -1607,   966,  4131, 11963, 18012, 18093, 18130, 13602,
   17781, 18167, 11551, 12971, 13376, 12563, 13972, 14305, 14305, 14305,
   14305,  1769,  1769,  1769,  1769,  1769,  1191,  1191,   831,   831,
     831,   109,   109,   109, -1607,   788, 17829,   902,   903, 17048,
     899,  1095,    -8, 13602,    -4,   678,     9,   175, -1607, -1607,
   -1607,  1093,   828, -1607,   640, 16625, -1607, -1607, -1607, 18093,
   18093, 18093, 18093, 18093, 18093, 18093, 18093, 18093, 18093, 18093,
   18093, 18093, -1607, 13602,     1,   176, -1607, -1607,   678,     8,
     906,  3958,   911,   913,   909,  4058,   132,   917, -1607, 17829,
   16378, -1607, 14904, -1607,   218,   471, 16831, 17829, 16831, 17104,
     966,   218,   429,   194,   954,   918, 13602, -1607,   331, -1607,
   -1607, -1607,  8933,   166, -1607, -1607, 17829, 17829,   231, -1607,
   -1607, -1607, 13602,  1015, 16151, 16275, 14904,  9542,   920,   924,
   -1607,  1121,  4501,   996, -1607,   973, -1607,  1125,   938,  2913,
    3881, 16275, 16275, 16275, 16275, 16275,   943,  1070,  1072,  1074,
    1076,  1078,   951, 16275,    14, -1607, -1607, -1607, -1607, -1607,
   -1607,   -24, -1607, 17923, -1607, -1607,    46, -1607,  6091, 14022,
     950, 14727, -1607, 14727, -1607, 14904, 14904, 14727, 14727, 14904,
   -1607,  1142,   957, -1607,   312, -1607, -1607,  4144, -1607, 17923,
    1145, 16831,   964, -1607, -1607, -1607, -1607, -1607, -1607, -1607,
   -1607, -1607,   981,  1155, 14904, 14022,   971, 16522, 16728,  1151,
   -1607, 13602, -1607, 13602, -1607, 13602, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607, -1607,   972, -1607, 13602, -1607, -1607,  5279,
   -1607,  3881, 14022,   977, -1607, -1607, -1607, -1607,  1166,   984,
   13602, 17441, -1607, -1607,  4742,   986, -1607,  3881, -1607,   992,
    6294,  1137,    58, -1607, -1607,    89,  4068, -1607, 15083, -1607,
    3881, -1607, -1607,   429, 17829, -1607, 10963, -1607, 16275,    80,
     990, 14022,   928, -1607, -1607, 18167, 13602, -1607, -1607, 13602,
   -1607, 13602, -1607,  4967,   993, 10557,   905,  1161,   928,  3881,
    1184,   966, 14904, 17541,   429, 11353,  1003, -1607, -1607,   324,
    1005, -1607, -1607,  1193,  3682,  3682, 16378, -1607, -1607, -1607,
    1158,  1016,  1141,  1143,  1149,  1150,  1162,    63,  1019,   459,
   -1607, -1607, -1607, -1607, -1607,  1061, -1607, -1607, -1607, -1607,
    1222,  1036,   773,   429,   429, 13399,   928, 15083, -1607, -1607,
   11759,   670,   231, 10151, -1607,  6497,  1037,  6700,  1040, 16151,
   16831,  1043,  1098,   429, 17923,  1233, -1607, -1607, -1607, -1607,
      79, -1607,    53,  3881,  1054,  1113,  1091,  3881, 14904,  3578,
   -1607, -1607, -1607,  1243, -1607,  1056,  1073,   652,   652,  1187,
    1187,  4386,  1055,  1248, 16275, 16275, 16275, 16275, 16275, 16275,
   17441,  2768, 15351, 16275, 16275, 16275, 16275, 16032, 16275, 16275,
   16275, 16275, 16275, 16275, 16275, 16275, 16275, 16275, 16275, 16275,
   16275, 16275, 16275, 16275, 16275, 16275, 16275, 16275, 16275, 16275,
   16275, 14904, -1607, -1607,  1179, -1607, -1607,  1065,  1066, -1607,
   -1607, -1607,   448, 17615, -1607,  1071, -1607, 16275,   429, -1607,
   -1607,    68, -1607,   615,  1258, -1607, -1607,   134,  1080,   429,
   11166, 17829, 17152, -1607,  3240, -1607,  5482,   828,  1258, -1607,
     636,   238, -1607, 17829,  1154,  1099, -1607,  1096,  1137, -1607,
    3881,   866,  3881,   260,  1284,  1216,   348, -1607,   678,   349,
   -1607, -1607, 16831, 13602, 17829, 17923,  1103,    80, -1607,  1102,
      80,  1106, 18167, 17829, 17208,  1110, 10557,  1111,  1108,  3881,
    1112,  1114,  3881,   928, -1607,   693,   224, 10557, 13602, -1607,
   -1607, -1607, -1607, -1607, -1607,  1173,  1115,  1305,  1224, 16378,
   16378, 16378, 16378, 16378, 16378,  1159, -1607, 17441,    77, 16378,
   -1607, -1607, -1607, 16831, 17829,  1120, -1607,   231,  1288,  1244,
   10151, -1607, -1607, -1607,  1127, 13602,  1098,   429, 16522, 16151,
    1129, 16275,  6903,   606,  1131, 13602,    84,    56, -1607,  1148,
   -1607,  3881, 14904, -1607,  1195, -1607, -1607,  3481,  1300,  1136,
   16275, -1607, 16275, -1607,  1138,  1153,  1325, 17256,  1157, 17923,
    1330,  1160,  1163,  1167,  1204,  1335,  1156, -1607, -1607, -1607,
   17311,  1152,  1339, 17968, 18056, 10537, 16275, 17877, 12769, 13174,
    3095,  4999, 14481, 14657, 14657, 14657, 14657,  3454,  3454,  3454,
    3454,  3454,   897,   897,   652,   652,   652,  1187,  1187,  1187,
    1187, -1607,  1164, -1607,  1174,  1175, -1607, -1607, 17923, 14904,
    3881,  3881, -1607,   615, 14022,   150, -1607, 16522, -1607, -1607,
   18093, 13602,  1165, -1607,  1181,   565, -1607,    67, 13602, -1607,
   -1607, -1607, 13602, -1607, 13602, -1607,   866, -1607, -1607,   117,
    1340,  1275, 13602, -1607,  1168,   429, 17829,  1137,  1177, -1607,
    1186,    80, 13602, 10557,  1188, -1607, -1607,   828, -1607, -1607,
    1180,  1196,  1190, -1607,  1199, 16378, -1607, 16378, -1607, -1607,
    1206,  1201,  1353,  1251,  1203, -1607,  1398,  1205,  1208,  1209,
   -1607,  1273,  1217,  1404, -1607, -1607,   429, -1607,  1384, -1607,
    1226, -1607, -1607,  1229,  1232,   135, -1607, -1607, 17923,  1234,
    1235, -1607,  4336, -1607, -1607, -1607, -1607, -1607, -1607,  1295,
    3881, -1607,  3881, -1607, 17923, 17359, -1607, -1607, 16275, -1607,
   16275, -1607, 16275, -1607, -1607, -1607, -1607, 16275, 17441, -1607,
   -1607, 16275, -1607, 16275, -1607, 10943, 16275,  1236,  7106, -1607,
   -1607,   615, -1607, -1607, -1607, -1607,   625, 14656, 14022,  1324,
   -1607,  2335,  1265,  2455, -1607, -1607, -1607,   829,  4571,   113,
     114,  1240,   828,   822,   136, 17829, -1607, -1607, -1607,  1279,
   12368, 13586, 17829, -1607,   329,  1426,  1363, 13602, -1607, 17829,
   10557,  1331,  1137,  1004,  1137,  1250, 17829,  1252, -1607,  1105,
    1255,  1298, -1607, -1607,    80, -1607, -1607,  1314, -1607, -1607,
   16378, -1607, 16378, -1607, 16378, -1607, -1607, -1607, -1607, 16378,
   -1607, 17441, -1607,  1736, -1607,  8933, -1607, -1607, -1607, -1607,
    9745, -1607, -1607, -1607,  8933,  3881, -1607,  1261, 16275, 17414,
   17923, 17923, 17923,  1319, 17923, 17462, 10943, -1607, -1607,   615,
   14022, 14022, 14904, -1607,  1448, 15503,    85, -1607, 14656,   828,
    2847, -1607,  1282, -1607,   115,  1268,   121, -1607, 15009, -1607,
   -1607, -1607,   123, -1607, -1607,  4143, -1607,  1266, -1607,  1386,
     640, -1607, 14833, -1607, 14833, -1607, -1607,  1456,   829, -1607,
   14128, -1607, -1607, -1607, -1607,  1457,  1391, 13602, -1607, 17829,
    1278,  1280,  1137,   589, -1607,  1331,  1137, -1607, -1607, -1607,
   -1607,  1986,  1283, 16378,  1341, -1607, -1607, -1607,  1343, -1607,
    8933,  9948,  9745, -1607, -1607, -1607,  8933, -1607, -1607, 17923,
   16275, 16275, 16275,  7309,  1285,  1286, -1607, 16275, -1607, 14022,
   -1607, -1607, -1607, -1607, -1607,  3881,  3791,  2335, -1607, -1607,
   -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607,
     642, -1607,  1265, -1607, -1607, -1607, -1607, -1607,   106,   358,
   -1607,  1467,   124, 15199,  1386,  1468, -1607,  3881,   640, -1607,
   -1607,  1289,  1472, 13602, -1607, 17829, -1607,   138,  1291, -1607,
   -1607, -1607,  1137,   589, 14304, -1607,  1137, -1607, 16378, 16378,
   -1607, -1607, -1607, -1607,  7512, 17923, 17923, 17923, -1607, -1607,
   -1607, 17923, -1607,  2128,  1479,  1482,  1294, -1607, -1607, 16275,
   15009, 15009,  1431, -1607,  4143,  4143,   772, -1607, -1607, -1607,
   16275,  1417, -1607,  1320,  1306,   125, 16275, -1607, 14904, -1607,
   16275, 17829,  1420, -1607,  1495, -1607,  7715,  1309, -1607, -1607,
     589, -1607, -1607,  7918,  1311,  1395, -1607,  1409,  1352, -1607,
   -1607,  1412,  3881,  1333,  3791, -1607, -1607, 17923, -1607, -1607,
    1344, -1607,  1481, -1607, -1607, -1607, -1607, 17923,  1508,   539,
   -1607, -1607, 17923,  1327, 17923, -1607,   143,  1328,  8121, -1607,
   -1607, -1607,  1329, -1607,  1332,  1348, 14904,   822,  1347, -1607,
   -1607, -1607, 16275,  1350,    81, -1607,  1447, -1607, -1607, -1607,
    8324, -1607, 14022,   950, -1607,  1358, 14904,   630, -1607, 17923,
   -1607,  1337,  1526,   638,    81, -1607, -1607,  1458, -1607, 14022,
    1342, -1607,  1137,   104, -1607, -1607, -1607, -1607,  3881, -1607,
    1345,  1354,   126, -1607,  1355,   638,   156,  1137,  1346, -1607,
    3881,   600,  3881,   399,  1525,  1464,  1355, -1607,  1540, -1607,
     555, -1607, -1607, -1607,   165,  1539,  1471, 13602, -1607,   600,
    8527,  3881, -1607,  3881, -1607,  8730,   472,  1542,  1474, 13602,
   -1607, 17829, -1607, -1607, -1607, -1607, -1607,  1545,  1477, 13602,
   -1607, 17829, 13602, -1607, 17829, 17829
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,     0,   434,     0,   863,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   955,
     943,     0,   729,     0,   735,   736,   737,    25,   801,   930,
     931,   158,   159,   738,     0,   139,     0,     0,     0,     0,
      26,     0,     0,     0,     0,   193,     0,     0,     0,     0,
       0,     0,   403,   404,   405,   402,   401,   400,     0,     0,
       0,     0,   222,     0,     0,     0,    33,    34,    36,    37,
      35,   742,   744,   745,   739,   740,     0,     0,     0,   746,
     741,     0,   712,    28,    29,    30,    32,    31,     0,   743,
       0,     0,     0,     0,   747,   406,   541,    27,     0,   157,
     129,   935,   730,     0,     0,     4,   119,   121,   800,     0,
     711,     0,     6,   192,     7,     9,     8,    10,     0,     0,
     398,   447,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   445,   918,   919,   523,   517,   518,   519,   520,   521,
     522,   428,   526,     0,   427,   890,   713,   720,     0,   803,
     516,   397,   893,   894,   905,   446,     0,     0,   449,   448,
     891,   892,   889,   925,   929,     0,   506,   802,    11,   403,
     404,   405,     0,     0,    32,     0,   119,   192,     0,   995,
     446,   996,     0,   998,   999,   525,   442,     0,   435,   440,
       0,     0,   488,   489,   490,   491,    25,   930,   738,   715,
      33,    34,    36,    37,    35,     0,     0,  1019,   911,   713,
       0,   714,   467,     0,   469,   507,   508,   509,   510,   511,
     512,   513,   515,     0,   959,     0,   810,   725,   212,     0,
    1019,   425,   724,   718,     0,   734,   714,   938,   939,   945,
     937,   726,     0,   426,     0,   728,   514,     0,     0,     0,
       0,   431,     0,   137,   433,     0,     0,   143,   145,     0,
       0,   147,     0,    71,    72,    77,    78,    63,    64,    55,
      75,    86,    87,     0,    58,     0,    62,    70,    68,    89,
      81,    80,    53,    76,    96,    97,    54,    92,    51,    93,
      52,    94,    50,    98,    85,    90,    95,    82,    83,    57,
      84,    88,    49,    79,    65,    99,    73,    66,    56,    43,
      44,    45,    46,    47,    48,    67,   101,   100,   103,    60,
      41,    42,    69,  1066,  1067,    61,  1071,    40,    59,    91,
       0,     0,   119,   102,  1010,  1065,     0,  1068,     0,     0,
     149,     0,     0,     0,   183,     0,     0,     0,     0,     0,
       0,   812,     0,   107,   109,   311,     0,     0,   310,   316,
       0,     0,   223,     0,   226,     0,     0,     0,     0,  1016,
     208,   220,   951,   955,   560,   587,   587,   560,   587,     0,
     980,     0,   749,     0,     0,     0,   978,     0,    16,     0,
     123,   200,   214,   221,   617,   553,     0,  1004,   533,   535,
     537,   867,   434,   447,     0,     0,   445,   446,   448,     0,
       0,   731,     0,   732,     0,     0,     0,   182,     0,     0,
     125,   302,     0,    24,   191,     0,   219,   204,   218,   403,
     406,   192,   399,   172,   173,   174,   175,   176,   178,   179,
     181,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   943,
       0,   171,   934,   934,   965,     0,     0,     0,     0,     0,
       0,     0,     0,   396,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   466,   468,   868,
     869,     0,   934,     0,   881,   302,   302,   934,     0,   936,
     926,   951,     0,   192,     0,     0,   151,     0,   865,   860,
     810,     0,   447,   445,     0,   963,     0,   558,   809,   954,
     734,   447,   445,   446,   125,     0,   302,   424,     0,   883,
     727,     0,   129,   262,     0,   540,     0,   154,     0,     0,
     432,     0,     0,     0,     0,     0,   146,   170,   148,  1066,
    1067,  1063,  1064,     0,  1070,  1056,     0,     0,     0,     0,
      74,    39,    61,    38,  1011,   177,   180,   150,   129,     0,
     167,   169,     0,     0,     0,     0,   110,     0,   811,   108,
      18,     0,   104,     0,   312,     0,   152,     0,     0,   153,
     224,   225,  1000,     0,     0,   447,   445,   446,   449,   448,
       0,  1046,   232,     0,   952,     0,     0,     0,     0,   810,
     810,     0,     0,     0,     0,   155,     0,     0,   748,   979,
     801,     0,     0,   977,   806,   976,   122,     5,    13,    14,
       0,   230,     0,     0,   546,     0,     0,     0,   810,     0,
     722,     0,   721,   716,   547,     0,     0,     0,     0,   867,
     129,     0,   812,   866,  1075,   423,   437,   502,   899,   917,
     134,   128,   130,   131,   132,   133,   397,     0,   524,   804,
     805,   120,   810,     0,  1020,     0,     0,     0,   812,   303,
       0,   529,   194,   228,     0,   472,   474,   473,   485,     0,
       0,   505,   470,   471,   475,   477,   476,   494,   495,   492,
     493,   496,   497,   498,   499,   500,   486,   487,   479,   480,
     478,   481,   482,   484,   501,   483,   933,     0,     0,   969,
       0,   810,  1003,     0,  1002,  1019,   896,   925,   210,   202,
     216,     0,  1004,   206,   192,     0,   438,   441,   443,   451,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   871,     0,   870,   873,   895,   877,  1019,   874,
       0,     0,     0,     0,     0,     0,     0,     0,   997,   436,
     858,   862,   809,   864,     0,   717,     0,   958,     0,   957,
     228,     0,   717,   942,   941,     0,     0,   870,   873,   940,
     874,   429,   264,   266,   129,   544,   543,   430,     0,   129,
     246,   138,   433,     0,     0,     0,     0,     0,   258,   258,
     144,   810,     0,     0,  1055,     0,  1052,   810,     0,  1026,
       0,     0,     0,     0,     0,   808,     0,    33,    34,    36,
      37,    35,     0,     0,   751,   755,   756,   757,   758,   759,
     761,     0,   750,   127,   799,   760,  1019,  1069,     0,     0,
       0,     0,    19,     0,    20,     0,   105,     0,     0,     0,
     116,   812,     0,   114,   109,   106,   111,     0,   309,   317,
     314,     0,     0,   989,   994,   991,   990,   993,   992,    12,
    1044,  1045,     0,   810,     0,     0,     0,   951,   948,     0,
     557,     0,   571,   809,   559,   809,   586,   574,   580,   583,
     577,   988,   987,   986,     0,   982,     0,   983,   985,     0,
       5,     0,     0,     0,   611,   612,   620,   619,     0,   445,
       0,   809,   552,   556,     0,     0,  1005,     0,   534,     0,
       0,  1033,   867,   288,  1074,     0,     0,   882,     0,   932,
     809,  1022,  1018,   304,   305,   710,   811,   301,     0,   867,
       0,     0,   230,   531,   196,   504,     0,   594,   595,     0,
     592,   809,   964,     0,     0,   302,   232,     0,   230,     0,
       0,   228,     0,   943,   452,     0,     0,   879,   880,   897,
     898,   927,   928,     0,     0,     0,   846,   817,   818,   819,
     826,     0,    33,    34,    36,    37,    35,     0,     0,   832,
     838,   839,   840,   841,   842,     0,   830,   828,   829,   852,
     810,     0,   860,   962,   961,     0,   230,     0,   884,   733,
       0,   268,     0,     0,   135,     0,     0,     0,     0,     0,
       0,     0,   238,   239,   250,     0,   129,   248,   164,   258,
       0,   258,     0,   809,     0,     0,     0,     0,     0,   809,
    1054,  1057,  1025,   810,  1024,     0,   810,   782,   783,   780,
     781,   816,     0,   810,   808,   564,   589,   589,   564,   589,
     555,     0,     0,   971,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1060,   184,     0,   187,   168,     0,     0,   112,
     117,   118,   110,   811,   115,     0,   313,     0,  1001,   156,
    1017,  1046,  1037,  1041,   231,   233,   323,     0,     0,   949,
       0,   562,     0,   981,     0,    17,     0,  1004,   229,   323,
       0,     0,   717,   549,     0,   723,  1006,     0,  1033,   538,
       0,     0,  1075,     0,   293,   291,   873,   885,  1019,   873,
     886,  1021,     0,     0,   306,   126,     0,   867,   227,     0,
     867,     0,   503,   968,   967,     0,   302,     0,     0,     0,
       0,     0,     0,   230,   198,   734,   872,   302,     0,   822,
     823,   824,   825,   833,   834,   850,     0,   810,     0,   846,
     568,   591,   591,   568,   591,     0,   821,   854,     0,   809,
     857,   859,   861,     0,   956,     0,   872,     0,     0,     0,
       0,   265,   545,   140,     0,   433,   238,   240,   951,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   252,     0,
    1061,     0,     0,  1047,     0,  1053,  1051,   809,     0,     0,
       0,   753,   809,   807,     0,     0,   810,     0,     0,   796,
     810,     0,     0,     0,     0,   810,     0,   762,   797,   798,
     975,     0,   810,   765,   767,   766,     0,     0,   763,   764,
     768,   770,   769,   786,   787,   784,   785,   788,   789,   790,
     791,   792,   777,   778,   772,   773,   771,   774,   775,   776,
     779,  1059,     0,   129,     0,     0,   113,    21,   315,     0,
       0,     0,  1038,  1043,     0,   397,   953,   951,   439,   444,
     450,     0,     0,    15,     0,   397,   623,     0,     0,   625,
     618,   621,     0,   616,     0,  1008,     0,  1034,   542,     0,
     294,     0,     0,   289,     0,   308,   307,  1033,     0,   323,
       0,   867,     0,   302,     0,   923,   323,  1004,   323,  1007,
       0,     0,     0,   453,     0,     0,   836,   809,   845,   827,
       0,     0,   810,     0,     0,   844,   810,     0,     0,     0,
     820,     0,     0,   810,   831,   851,   960,   323,     0,   129,
       0,   261,   247,     0,     0,     0,   237,   160,   251,     0,
       0,   254,     0,   259,   260,   129,   253,  1062,  1048,     0,
       0,  1023,     0,  1073,   815,   814,   752,   572,   809,   563,
       0,   575,   809,   588,   581,   584,   578,     0,   809,   554,
     754,     0,   593,   809,   970,   794,     0,     0,     0,    22,
      23,  1040,  1035,  1036,  1039,   234,     0,     0,     0,   404,
     395,     0,     0,     0,   209,   322,   324,     0,   394,     0,
       0,     0,  1004,   397,     0,   561,   984,   319,   215,   614,
       0,     0,   548,   536,     0,   297,   287,     0,   290,   296,
     302,   528,  1033,   397,  1033,     0,   966,     0,   922,   397,
       0,   397,  1009,   323,   867,   920,   849,   848,   835,   573,
     809,   567,     0,   576,   809,   590,   582,   585,   579,     0,
     837,   809,   853,   397,   129,   267,   136,   141,   162,   241,
       0,   249,   255,   129,   257,     0,  1049,     0,     0,     0,
     566,   795,   551,     0,   974,   973,   793,   129,   188,  1042,
       0,     0,     0,  1012,     0,     0,     0,   235,     0,  1004,
       0,   360,   356,   362,   712,    32,     0,   350,     0,   355,
     359,   372,     0,   370,   375,     0,   374,     0,   373,     0,
     192,   326,     0,   328,     0,   329,   330,     0,     0,   950,
       0,   615,   613,   624,   622,   298,     0,     0,   285,   295,
       0,     0,  1033,     0,   205,   528,  1033,   924,   211,   319,
     217,   397,     0,     0,     0,   570,   843,   856,     0,   213,
     263,     0,     0,   129,   244,   161,   256,  1050,  1072,   813,
       0,     0,     0,     0,     0,     0,   422,     0,  1013,     0,
     340,   344,   419,   420,   354,     0,     0,     0,   335,   673,
     674,   672,   675,   676,   693,   695,   694,   664,   636,   634,
     635,   654,   669,   670,   630,   641,   642,   644,   643,   663,
     647,   645,   646,   648,   649,   650,   651,   652,   653,   655,
     656,   657,   658,   659,   660,   662,   661,   631,   632,   633,
     637,   638,   640,   678,   679,   683,   684,   685,   686,   687,
     688,   671,   690,   680,   681,   682,   665,   666,   667,   668,
     691,   692,   696,   698,   697,   699,   700,   677,   702,   701,
     704,   706,   705,   639,   709,   707,   708,   703,   689,   629,
     367,   626,     0,   336,   388,   389,   387,   380,     0,   381,
     337,   414,     0,     0,     0,     0,   418,     0,   192,   201,
     318,     0,     0,     0,   286,   300,   921,     0,     0,   390,
     129,   195,  1033,     0,     0,   207,  1033,   847,     0,     0,
     129,   242,   142,   163,     0,   565,   550,   972,   186,   338,
     339,   417,   236,     0,   810,   810,     0,   363,   351,     0,
       0,     0,   369,   371,     0,     0,   376,   383,   384,   382,
       0,     0,   325,  1014,     0,     0,     0,   421,     0,   320,
       0,   299,     0,   609,   812,   129,     0,     0,   197,   203,
       0,   569,   855,     0,     0,   165,   341,   119,     0,   342,
     343,     0,   809,     0,   809,   365,   361,   366,   627,   628,
       0,   352,   385,   386,   378,   379,   377,   415,   412,  1046,
     331,   327,   416,     0,   321,   610,   811,     0,     0,   391,
     129,   199,     0,   245,     0,   190,     0,   397,     0,   357,
     364,   368,     0,     0,   867,   333,     0,   607,   527,   530,
       0,   243,     0,     0,   166,   348,     0,   396,   358,   413,
    1015,     0,   812,   408,   867,   608,   532,     0,   189,     0,
       0,   347,  1033,   867,   272,   411,   410,   409,  1075,   407,
       0,     0,     0,   346,  1027,   408,     0,  1033,     0,   345,
       0,     0,  1075,     0,   277,   275,  1027,   129,   812,  1029,
       0,   392,   129,   332,     0,   278,     0,     0,   273,     0,
       0,   811,  1028,     0,  1032,     0,     0,   281,   271,     0,
     274,   280,   334,   185,  1030,  1031,   393,   282,     0,     0,
     269,   279,     0,   270,   284,   283
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1607, -1607, -1607,  -565, -1607, -1607, -1607,   501,   -47,   -41,
     469, -1607,  -285,  -519, -1607, -1607,   428,    -2,  1776, -1607,
    1771, -1607,  -155, -1607,    52, -1607, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607, -1607, -1607, -1607,  -331, -1607, -1607,  -162,
     147,    26, -1607, -1607, -1607, -1607, -1607, -1607,    31, -1607,
   -1607, -1607, -1607, -1607, -1607,    34, -1607, -1607,  1075,  1081,
    1083,   -95,  -710,  -887,   578,   641,  -346,   322,  -962, -1607,
     -62, -1607, -1607, -1607, -1607,  -755,   152, -1607, -1607, -1607,
   -1607,  -332, -1607,  -605, -1607,  -471, -1607, -1607,   975, -1607,
     -44, -1607, -1607, -1071, -1607, -1607, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607, -1607,   -80, -1607,     6, -1607, -1607, -1607,
   -1607, -1607,  -164, -1607,   108,  -981, -1607, -1606,  -367, -1607,
    -133,   107,  -127,  -342, -1607,  -170, -1607, -1607, -1607,   116,
     -17,     0,   149,  -739,   -81, -1607, -1607,    27, -1607,   -14,
   -1607, -1607,    -5,   -37,   -29, -1607, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607, -1607,  -609,  -876, -1607, -1607, -1607, -1607,
   -1607,  1882,  1200, -1607,   500, -1607,   366, -1607, -1607, -1607,
   -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607,
   -1607, -1607, -1607,    57,  -604,  -826, -1607, -1607, -1607, -1607,
   -1607,   431, -1607, -1607, -1607, -1607, -1607, -1607, -1607, -1607,
    -964,  -376,  2694,    23, -1607,  1322,  -409, -1607, -1607,  -490,
    3451,  3512, -1607,  -306, -1607, -1607,   511,   -21,  -641, -1607,
   -1607,   592,   376,  -691, -1607,   380, -1607, -1607, -1607, -1607,
   -1607,   568, -1607, -1607, -1607,    10,  -885,  -159,  -434,  -431,
   -1607,   645,  -106, -1607, -1607,    25,    38,   629, -1607, -1607,
    1619,   -15, -1607,  -371,    82,    83, -1607,   180, -1607, -1607,
   -1607,  -479,  1215, -1607, -1607, -1607, -1607, -1607,   658,   577,
   -1607, -1607, -1607,  -364,  -676, -1607,  1171,  -923, -1607,   -66,
    -200,     2,   765, -1607,  -330, -1607,  -344, -1073, -1264,  -251,
     151, -1607,   468,   542, -1607, -1607, -1607, -1607,   492, -1607,
       7, -1124
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   115,   930,   647,   186,  1563,   745,
     360,   361,   362,   363,   881,   882,   883,   117,   118,   119,
     120,   121,   419,   681,   682,   559,   262,  1631,   565,  1540,
    1632,  1875,   870,   355,   588,  1835,  1126,  1323,  1894,   435,
     187,   683,   970,  1191,  1382,   125,   650,   987,   684,   703,
     991,   622,   986,   241,   540,   685,   651,   988,   437,   380,
     402,   128,   972,   933,   906,  1144,  1566,  1250,  1052,  1782,
    1635,   821,  1058,   564,   830,  1060,  1425,   813,  1041,  1044,
    1239,  1901,  1902,   671,   672,   697,   698,   367,   368,   370,
    1600,  1760,  1761,  1335,  1475,  1589,  1754,  1884,  1904,  1793,
    1839,  1840,  1841,  1576,  1577,  1578,  1579,  1795,  1796,  1802,
    1851,  1582,  1583,  1587,  1747,  1748,  1749,  1771,  1943,  1476,
    1477,   188,   130,  1918,  1919,  1752,  1479,  1480,  1481,  1482,
     131,   255,   560,   561,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,  1612,   142,   969,  1190,   143,   668,
     669,   670,   259,   411,   555,   657,   658,  1285,   659,  1286,
     144,   145,   628,   629,  1275,  1276,  1391,  1392,   146,   855,
    1020,   147,   856,  1021,   148,   857,  1022,   149,   858,  1023,
     150,   859,  1024,   631,  1278,  1394,   151,   860,   152,   153,
    1824,   154,   652,  1602,   653,  1160,   938,  1353,  1350,  1740,
    1741,   155,   156,   157,   244,   158,   245,   256,   422,   547,
     159,  1279,  1280,   864,   865,   160,  1082,   961,   599,  1083,
    1027,  1213,  1028,  1395,  1396,  1216,  1217,  1030,  1402,  1403,
    1031,   791,   530,   200,   201,   686,   674,   511,  1176,  1177,
     777,   778,   957,   162,   247,   163,   164,   190,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   737,   175,   251,
     252,   625,   235,   236,   740,   741,  1291,  1292,   395,   396,
     924,   176,   613,   177,   667,   178,   346,  1762,  1814,   381,
     430,   692,   693,  1075,  1931,  1938,  1939,  1171,  1332,   902,
    1333,   903,   904,   836,   837,   838,   347,   348,   867,   574,
    1565,   955
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     189,   191,   442,   193,   194,   195,   196,   198,   199,   343,
     202,   203,   204,   205,   161,   344,   225,   226,   227,   228,
     229,   230,   231,   232,   234,   522,   253,   414,   492,   660,
     124,   953,   403,   258,   662,   126,   406,   407,   127,   261,
     544,   664,   786,   352,   782,   783,   263,   269,  1359,   272,
     800,   267,   353,   243,   356,   248,   122,   967,   364,   948,
     514,   442,   438,   349,   949,  1164,   491,  1172,   249,  1464,
     734,   416,   593,   595,  1062,   805,   990,   775,   880,   885,
     776,   261,   929,  1048,   351,  1189,   399,  1246,  1345,   400,
    1036,   413,   548,   418,  1649,  1356,   432,   -39,   828,  1029,
     -38,  1200,   -39,   415,   556,   -38,    14,   605,   -74,    14,
     608,   129,   250,   -74,   808,  1804,  1423,   809,   900,   901,
     556,  1489,  1592,  1594,  -353,   207,    40,  1173,    14,    14,
    1657,   369,  1742,  1811,  1811,  1649,  1092,   826,   891,   556,
     908,   908,  1805,   908,   908,   908,   549,   589,   416,  1235,
     531,   123,    14,  1225,  -714,  1494,  1466,   601,  1404,  1255,
    1256,   390,  1255,  1256,   509,   510,   479,  1828,   413,  -723,
     418,   875,  1174,  -103, -1019,  1093,  1822,  -102,   480,   533,
     415,  1886,  -875,     3,  1254,  1255,  1256,   525,  -103,  -878,
    -912,   427,  -102,   260,  1933,   429,  -539,  -875,    14,   532,
    1495,   418,   542,  1956,  -878,   985,  1121,  1288,  -715,   512,
     590,   415,   192, -1019,  1284,   539,  -903,   392,  -596,   602,
    1226,  1823,   541,  -603,  1871,  -901,  1887,   429,   207,    40,
     928,   637,   404,   876,   415,   389,   393,   394,  -944,  1934,
    1134,  1042,  1043,   519,  -900,   517,  -904,  -902,  1957,   551,
    -809,  -811,   551,  1258,  -292,  -811,  1426,   517,   520,   261,
     562,  -907,  1467,  1175,  -947,   829,   935,  1468,   493,   439,
    1469,   181,    65,    66,    67,  1470,   441,  -292,  -276,   704,
     573,  1203,  1424,  1650,  1651,   433,   -39,  1416,   512,   -38,
     571,  1496,   572,   557,  1501,  1464,   606,   -74,  1503,   609,
     408,  -811,   343,  1806,  1253,  1509,  1257,  1511,   584,   635,
     553,  1593,  1595,  -353,   558,  1215,  1381,  1471,  1472,  1658,
    1473,  1743,  1812,  1861,  1929,   827,   892,   893,   909,  1003,
    1935,  1336,  1539,  1599,  -722,   523,  1533,  -721,   513,  1958,
     616,   440,  -909,  1360,  -911,  -903,   111,  -716,   577,   516,
    1474,  1401,   509,   510,  -901,   364,   364,   596,  1130,  1131,
     615,   787,   619, -1019,  1186,  1156,   442,  -944,   365,   702,
    -910,   261,   415,  -900,   518,  -904,  -902,   936,   234,   627,
     261,   261,   627,   261,   429,   343,   518,  -913,   641,  1807,
    -907,   344,   937,  -947,  -906,   646,  1397,   812,  1399, -1019,
     373,  -946, -1019,   254,  1351,  -876,   428,   198,  1808,   409,
     374,  1809,  1605,   756,   601,   687,   410,   513,  -887,  -888,
    -876,   673,   403,   751,   752,   438,   699,   257,  1147,  1613,
    -606,  1615,  -603,   868,  1361,   371,   694,  1352,   744,   349,
     648,   649,  1621,   632,   372,   634,   705,   706,   707,   708,
     710,   711,   712,   713,   714,   715,   716,   717,   718,   719,
     720,   721,   722,   723,   724,   725,   726,   727,   728,   729,
     730,   731,   732,   733,  -604,   735,   428,   736,   736,   739,
     758,  1344,  1945,  1281,  1026,  1283,   366,   343,   663,   759,
     760,   761,   762,   763,   764,   765,   766,   767,   768,   769,
     770,   771,   243,  1606,   248,   116,  1413,   736,   781,   793,
     699,   699,   736,   785,  1197,   950,   956,   249,   958,   759,
    -916,   757,   789,  -906,  1179,   614,   129,  1180,  1215,  1393,
    -946,   797,  1393,   799,   630,   630,  -605,   630,  1405,  1768,
     815,   699,   264,  1773,  1564,   984,   390,  -887,  -888,   816,
     875,   817,   265,   492,   270,  1967,   354,   342,   516,   897,
     636,   250,  1358,   509,   510,   660,   123,   746,   747,  1953,
     662,  1466,  1553,  1946,   379,   266,   390,   664,   996,   375,
     831,   376,  1368,   643,  1205,  1370,  1127,   377,  1128,   900,
     901,   491,   992,   779,   747,   884,   884,   401,   887,   379,
     509,   510,   378,   379,   379,   509,   510,   382,   914,   916,
     383,   820,   428,    14,   880,   384,   746,   747,   939,  -119,
     638,   393,   394,  -119,   509,   510,  -717,   804,   747,   379,
     810,   747,   385,   165,   390,   956,   958,   942,   974,   754,
    -119,   643,  1037,   958,   386,  1652,  1968,   404,   222,   224,
     415,   393,   394,   390,   544,  1628,  1799,  1330,  1331,  1045,
     391,   387,  -914,   738,  1047,   388,  1122,  -914,   428,  1755,
     405,  1756,   945,   946,  1800,   420,  1954,  1467,   434,   673,
     431,   954,  1468,   443,   439,  1469,   181,    65,    66,    67,
    1470,   964,   780,  1801,  1516,  1038,  1517,   784,   690,  1827,
    1026,  1510,   529,  1830,   975,  1117,  1118,  1119,   444,   393,
     394,  1420,  1255,  1256,   660,  1383,   390,  1330,  1331,   662,
     982,  1120,   445,   643,   446,  1374,   664,   392,   393,   394,
     447,    55,  1471,  1472,   448,  1473,  1384,   417,   983,   439,
     180,   181,    65,    66,    67,  1237,  1238,  1493,   449,   439,
     180,   181,    65,    66,    67,   689,   440,  1560,  1561,   116,
    1915,  1916,  1917,   116,   450,  1488,  1505,   563,   995,  1415,
     423,   425,   426,  -597,   524,   495,   496,   497,   498,   499,
     500,   501,   502,   503,   504,   505,   506,  1769,  1770,   390,
     644,   393,   394,   493,  1926,  -598,   421,   390,  1941,  1942,
    -599,  1040,  1346,  1854,   424,   390,  1597,  -600,  1944,  -601,
    1064,   440,   643,   482,   417,  1347,  1070,   261,  1046,   507,
     508,   440,  1855,  1852,  1853,  1856,   592,   594,  1911,  1624,
     484,  1625,   483,  1626,   485,  1348,  1848,  1849,  1627,  1924,
     515,  1073,  1076,  -908,  -602,   417,   694,   694,  -715,   521,
     583,   660,   526,   397,  1936,   528,   662,   480,  1484,   429,
     534,  -912,   535,   664,   393,   394,   537,   516,   543,   884,
     538,   884,   393,   394,   554,   884,   884,  1132,  -713,  1057,
     393,   394,  1142,   545,   476,   477,   478,   165,   479,   546,
     567,   165,   575,  1653,   509,   510, -1058,   744,   578,   585,
     480,  1252,  1507,   586,   579,   598,  1151,   597,  1152,  1622,
     817,   600,   607,  1026,  1026,  1026,  1026,  1026,  1026,   129,
     116,  1154,   610,  1026,   611,   620,  1204,   621,   665,   691,
     666,   675,  1777,   342,   688,  1163,   379,   676,  1157,   161,
      55,   439,   180,   181,    65,    66,    67,  1114,  1115,  1116,
    1117,  1118,  1119,   677,  1167,   124,   679,   678,   792,   123,
     126,  1184,   673,   127,  -124,   701,  1120,  1181,   639,   790,
     638,  1192,   645,   794,  1193,   129,  1194,   795,  1364,   673,
     699,   122,   801,   802,   818,   556,   583,   379,   749,   379,
     379,   379,   379,   822,   825,   604,  1201,   573,   639,  1903,
     645,   639,   645,   645,   612,   839,   617,   840,   869,  1230,
    1466,   624,   774,   440,   871,   123,   243,   872,   248,  1903,
     873,   874,   890,   642,   663,   899,  1165,   747,  1925,  1610,
    1234,   249,   894,   895,   898,   583,   129,   905,   779,   747,
     810,   747,  1240,   907,   912,  1289,   910,   913,   165,   807,
     915,   917,    14,   918,   919,  1269,   920,   129,   926,   931,
     116,   932,  1273,   934,  -738,   940,   941,   943,   944,  1338,
    1259,   951,   660,   947,  1263,   250,   123,   662,   952,  1026,
     866,  1026,   960,   962,   664,   965,   966,  1831,  1832,   439,
      63,    64,    65,    66,    67,  1241,   968,   123,   971,   980,
      72,   486,   886,   691,   981,   977,   978,   989,   999,   997,
    1000,  1466,  1001,   973,  -719,  1039,  1467,  1049,  1059,   810,
     747,  1468,  1061,   439,  1469,   181,    65,    66,    67,  1470,
    1063,   884,  1067,  1068,  1069,  1071,  1339,   923,   925,  1084,
    1085,   487,  1086,   488,  1087,  1340,  1088,  1090,  1089,  1125,
     624,  1133,   129,    14,   129,   660,   489,  1135,   490,  1137,
     662,   440,  1139,  1140,  1141,  1150,   161,   664,  1458,  1170,
    1146,  1471,  1472,   663,  1473,  1153,  1159,  1357,  1366,   954,
    1161,  1162,   124,  1867,  1166,  1168,  1187,   126,   165,  1196,
     127,   699,   123,  1199,   123,   440,  1388,   673,  1202,  1207,
     673,  -915,   699,  1340,  1614,   379,  1377,  1208,   122,  1380,
    1218,  1220,  1219,  1221,  1026,  1227,  1026,  1467,  1026,  1222,
    1223,  1228,  1468,  1026,   439,  1469,   181,    65,    66,    67,
    1470,  1229,  1224,  1231,  1249,  1243,  1260,  1408,  1245,  1248,
     261,   473,   474,   475,   476,   477,   478,  1251,   479,  1261,
    1422,  1262,  1267,  1268,  1535,  1439,  1120,  1272,  1271,  1443,
     480,  1914,  1322,   129,  1449,  1324,  1325,  1334,  1428,  1327,
    1544,  1454,  1471,  1472,  1181,  1473,  1337,   524,   495,   496,
     497,   498,   499,   500,   501,   502,   503,   504,   505,   506,
    1354,  1019,  1411,  1032,  1355,   985,   440,  1952,  1362,  1363,
    1367,  1369,  1371,   123,  1466,  1618,  1373,  1376,  1375,  1385,
     663,  1378,  1379,   116,  1387,  1010,  1400,  1026,  1386,  1407,
    1409,  1410,   507,   508,   963,  1412,  1417,  1055,   116,  1421,
    1427,  1430,  1432,  1433,  1438,  1436,  1485,  1462,  1463,  1442,
    1447,   220,   220,  1490,  1448,  1483,    14,  1491,  1453,  1492,
    1598,   442,  1452,  1450,  1497,  1483,  1437,  1499,  1498,   129,
    1441,  1457,  1520,  1444,  1500,  1486,  1445,  1506,   699,   116,
    1446,  1521,  1459,  1460,  1502,  1525,  1129,   691,  1512,  1630,
    1487,   673,  1532,  1504,   994,  1508,  1514,  1522,  1636,   439,
      63,    64,    65,    66,    67,  1513,  1515,   509,   510,   123,
      72,   486,  1643,  1518,  1519,  1143,  1523,  1524,  1526,  1529,
    1467,  1527,  1528,  1531,  1530,  1468,  1534,   439,  1469,   181,
      65,    66,    67,  1470,  1536,  1033,  1537,  1034,  1753,  1538,
     116,  1545,  1541,  1542,  1581,  1557,  1568,  1546,  1596,  1547,
    1607,   165,  1478,   488,  1601,   583,  1608,  1616,  1611,  1617,
    1623,   116,  1478,  1053,  1619,  1641,   165,   774,  1638,   807,
     803,   440,  1647,  1655,  1750,  1471,  1472,  1656,  1473,  1751,
    1757,  1763,  1026,  1026,  1764,  1766,  1767,  1778,  1784,  1779,
    1776,  1810,  1816,  1789,  1790,  1591,  1820,  1819,  1842,   440,
    1825,  1844,  1609,   379,  1846,   699,  1646,   165,  1620,  1850,
    1858,  1859,  1860,  1865,  1866,  1212,  1212,  1019,  1870,  1873,
    1874,  -349,  1876,  1483,  1877,  1879,  1881,  1805,  1648,  1483,
    1138,  1483,  1882,  1885,   673,  1888,  1893,  1891,  1892,  1898,
    1905,   663,  1900,  1909,  1912,  1913,   624,  1149,   807,  1947,
    1923,  1921,  1927,  1483,   116,  1937,   116,  1948,   116,  1951,
    1930,  1928,  1637,  1959,  1960,   220,  1969,  1970,   165,  1972,
    1973,  1326,  1908,  1922,  1198,   129,   753,   748,  1414,  1264,
    1783,   750,  1920,  1158,  1543,  1774,  1654,  1798,  1803,   165,
     888,  1588,  1962,  1932,  1815,  1569,  1772,   633,  1282,  1398,
     493,  1349,  1634,   583,  1389,  1274,  1818,  1214,   626,  1390,
    1232,  1178,  1765,   700,  1074,   123,  1949,  1964,  1883,  1329,
    1478,  1266,  1559,  1321,   663,  1826,  1478,     0,  1478,     0,
       0,     0,   866,     0,  1590,  1833,     0,     0,     0,     0,
       0,  1483,     0,     0,     0,     0,     0,     0,   223,   223,
    1478,     0,   129,     0,     0,     0,     0,     0,     0,     0,
       0,   129,     0,     0,     0,     0,     0,   116,     0,     0,
       0,     0,  1794,     0,     0,     0,     0,     0,     0,     0,
    1868,     0,   165,     0,   165,     0,   165,     0,  1053,  1247,
       0,     0,   123,  1781,  1634,     0,     0,     0,   220,     0,
       0,   123,     0,     0,     0,     0,     0,   220,     0,     0,
       0,     0,     0,     0,   220,     0,   343,     0,     0,     0,
       0,     0,  1813,     0,     0,  1890,   220,     0,     0,     0,
    1019,  1019,  1019,  1019,  1019,  1019,     0,   661,  1478,     0,
    1019,     0,     0,     0,     0,     0,     0,   129,     0,     0,
       0,   116,  1466,   129,  1896,  1758,     0,     0,     0,     0,
     129,     0,  1863,   116,     0,     0,     0,     0,  1821,     0,
       0,     0,     0,  1429,  1817,     0,     0,     0,     0,     0,
     442,     0,     0,  1843,  1845,     0,     0,   123,     0,     0,
       0,     0,  1950,   123,    14,   165,     0,  1955,     0,     0,
     123,     0,     0,     0,     0,   217,   217,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   240,     0,     0,     0,
       0,  1365, -1059, -1059, -1059, -1059, -1059,   471,   472,   473,
     474,   475,   476,   477,   478,     0,   479,   345,     0,     0,
    1461,     0,   240,     0,     0,     0,     0,     0,   480,     0,
       0,     0,     0,   220,     0,     0,     0,     0,  1467,  1878,
       0,     0,   223,  1468,     0,   439,  1469,   181,    65,    66,
      67,  1470,  1406,     0,     0,     0,     0,     0,     0,   165,
       0,     0,     0,     0,     0,     0,     0,   624,  1053,     0,
       0,   165,     0,     0,     0,     0,  1019,     0,  1019,     0,
       0,   129,     0,     0,   673,     0,     0,     0,     0,     0,
       0,   218,   218,  1471,  1472,     0,  1473,     0,     0,     0,
       0,     0,     0,     0,   673,     0,     0,     0,     0,     0,
       0,     0,     0,   673,     0,   954,     0,   440,     0,     0,
       0,   123,     0,   129,     0,     0,  1629,  1940,     0,   954,
     129,     0,  1961,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1971,     0,     0,     0,  1940,   116,
    1965,     0,     0,     0,  1974,     0,   624,  1975,   342,     0,
       0,     0,     0,   123,  1586,   129,     0,     0,     0,     0,
     123,     0,     0,     0,  1897,   223,     0,     0,     0,     0,
       0,     0,  1466,     0,   223,     0,   618,   129,     0,     0,
       0,   223,     0,     0,     0,     0,     0,     0,     0,   217,
       0,     0,     0,   223,     0,   123,     0,   220,     0,     0,
       0,  1019,     0,  1019,     0,  1019,     0,     0,     0,     0,
    1019,     0,     0,     0,    14,     0,   116,   123,     0,     0,
       0,   116,     0,     0,     0,   116,     0,     0,     0,     0,
       0,     0,     0,     0,   345,     0,   345,   129,     0,   240,
       0,   240,   129,   379,     0,     0,   583,     0,     0,   342,
       0,     0,     0,     0,     0,     0,     0,   220,     0,  1739,
       0,     0,     0,     0,     0,     0,  1746,   165,     0,     0,
       0,     0,     0,   342,     0,   342,     0,   123,  1467,     0,
       0,   342,   123,  1468,     0,   439,  1469,   181,    65,    66,
      67,  1470,   345,     0,     0,   218,     0,   240,   220,     0,
     220,     0,     0,     0,  1019,     0,     0,     0,     0,     0,
       0,   116,   116,   116,     0,     0,     0,   116,     0,     0,
     223,     0,   217,     0,   116,     0,   220,     0,     0,     0,
       0,   217,     0,  1471,  1472,     0,  1473,     0,   217,     0,
     206,     0,     0,     0,   165,     0,     0,     0,     0,   165,
     217,     0,     0,   165,     0,     0,     0,   440,     0,     0,
       0,   217,    50,     0,     0,     0,  1775,   524,   495,   496,
     497,   498,   499,   500,   501,   502,   503,   504,   505,   506,
     345,     0,     0,   345,     0,   240,     0,     0,   240,   206,
       0,     0,     0,   220,     0,     0,     0,     0,   210,   211,
     212,   213,   214,     0,     0,     0,     0,     0,     0,   220,
     220,    50,   507,   508,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   397,     0,     0,    93,    94,   218,    95,
     184,    97,     0,     0,   583,   240,     0,   218,     0,   165,
     165,   165,     0,   661,   218,   165,     0,   210,   211,   212,
     213,   214,   165,     0,   107,   342,   218,     0,   398,  1019,
    1019,     0,     0,     0,     0,   116,     0,     0,     0,   183,
       0,     0,    91,     0,  1837,    93,    94,   217,    95,   184,
      97,  1739,  1739,     0,     0,  1746,  1746,   509,   510,     0,
       0,     0,     0,     0,   223,     0,     0,     0,     0,   379,
       0,     0,     0,   107,     0,     0,     0,   116,  1836,     0,
       0,     0,     0,     0,   116,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   345,     0,   835,     0,     0,   240,
       0,   240,     0,     0,   854,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1570,     0,     0,   116,
     896,   220,   220,     0,   223,     0,     0,  1895,     0,     0,
       0,   283,     0,   854,     0,     0,     0,     0,     0,     0,
       0,   116,     0,     0,     0,     0,     0,  1910,     0,     0,
       0,     0,     0,   218,     0,     0,     0,     0,     0,     0,
       0,     0,   661,   165,     0,   223,   206,   223,   285,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   206,     0,     0,     0,     0,   345,   345,    50,     0,
       0,   240,   240,   223,     0,   345,     0,     0,     0,     0,
     240,   116,     0,    50,     0,   165,   116,     0,     0,     0,
    1571,   576,   165,     0,     0,     0,     0,     0,     0,     0,
       0,   217,     0,  1572,   210,   211,   212,   213,   214,  1573,
       0,     0,     0,     0,     0,     0,     0,     0,   569,   210,
     211,   212,   213,   214,   570,     0,   183,   165,     0,    91,
    1574,     0,    93,    94,   220,    95,  1575,    97,     0,     0,
     223,   183,     0,     0,    91,   336,     0,    93,    94,   165,
      95,   184,    97,     0,     0,     0,   223,   223,     0,     0,
     107,   217,     0,     0,     0,   340,   206,     0,     0,     0,
       0,     0,     0,     0,     0,   107,   341,     0,     0,   661,
       0,     0,   832,     0,     0,   220,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,   240,     0,     0,     0,
     220,   220,   217,     0,   217,     0,     0,   218,     0,   165,
       0,     0,     0,  1584,   165,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   210,   211,   212,   213,   214,     0,
     217,   854,   206,  1066,     0,     0,     0,     0,   240,     0,
     345,   345,   833,     0,   206,   240,   240,   854,   854,   854,
     854,   854,    93,    94,    50,    95,   184,    97,     0,   854,
       0,     0,     0,     0,     0,     0,    50,   218,     0,     0,
       0,     0,     0,     0,     0,   240,     0,     0,     0,     0,
     107,  1585,     0,     0,     0,     0,     0,     0,     0,   220,
     210,   211,   212,   213,   214,     0,     0,   217,   223,   223,
       0,     0,   210,   211,   212,   213,   214,     0,   218,     0,
     218,   240,   183,   217,   217,    91,     0,     0,    93,    94,
       0,    95,   184,    97,     0,   834,     0,     0,     0,     0,
      93,    94,   345,    95,   184,    97,   218,   240,   240,     0,
       0,     0,     0,   219,   219,     0,   107,   217,   345,     0,
       0,     0,     0,   240,   242,     0,     0,     0,   107,   701,
       0,   345,     0,     0,     0,     0,   240,     0,     0,     0,
       0,     0,     0,     0,   854,     0,     0,   240,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     345,     0,     0,     0,     0,   240,     0,     0,     0,   240,
     661,     0,     0,   218,     0,     0,     0,     0,  1094,  1095,
    1096,     0,   240,     0,     0,     0,     0,     0,     0,   218,
     218,     0,     0,     0,     0,     0,     0,     0,     0,  1097,
       0,   223,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,     0,   217,   217,     0,     0,     0,
       0,     0,     0,     0,   345,     0,     0,  1120,   345,   240,
     835,     0,     0,   240,     0,   240,     0,     0,     0,     0,
       0,     0,   223,   661,     0,     0,     0,     0,     0,     0,
     854,   854,   854,   854,   854,   854,   217,   223,   223,   854,
     854,   854,   854,   854,   854,   854,   854,   854,   854,   854,
     854,   854,   854,   854,   854,   854,   854,   854,   854,   854,
     854,   854,   854,   854,   854,   854,   854,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   854,   494,   495,   496,   497,   498,   499,
     500,   501,   502,   503,   504,   505,   506,   219,   206,     0,
       0,   218,   218,     0,     0,     0,     0,     0,     0,     0,
       0,   345,     0,   345,   283,     0,   240,     0,   240,     0,
      50,     0,     0,     0,     0,     0,   223,     0,   217,   507,
     508,     0,     0,     0,     0,  1287,     0,     0,     0,     0,
     345,     0,  1571,   345,     0,   240,     0,     0,   240,     0,
       0,   285,     0,     0,     0,  1572,   210,   211,   212,   213,
     214,  1573,     0,     0,   206,   240,   240,   240,   240,   240,
     240,     0,     0,   217,     0,   240,     0,     0,   183,   217,
       0,    91,    92,     0,    93,    94,    50,    95,  1575,    97,
       0,     0,     0,     0,   217,   217,     0,   854,     0,     0,
       0,     0,   345,     0,   509,   510,     0,   240,   345,     0,
       0,     0,   107,   240,     0,     0,   854,     0,   854,     0,
       0,   569,   210,   211,   212,   213,   214,   570,     0,     0,
     219,     0,     0,     0,   218,     0,     0,     0,     0,   219,
       0,     0,   854,     0,   183,     0,   219,    91,   336,     0,
      93,    94,     0,    95,   184,    97,     0,  1072,   219,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   340,   219,
       0,   345,   345,     0,     0,     0,   240,   240,   107,   341,
     240,     0,     0,   217,     0,   218,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     218,   218,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,   451,   452,   453,     0,     0,     0,     0,     0,     0,
       0,   240,     0,   240,  1120,     0,     0,     0,     0,     0,
       0,   454,   455,   242,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,     0,   479,     0,
       0,   345,     0,   345,     0,     0,   240,     0,   240,     0,
     480,     0,     0,     0,   854,   219,   854,     0,   854,   218,
       0,     0,     0,   854,   217,     0,     0,   854,     0,   854,
       0,     0,   854,     0,     0,     0,     0,     0,   345,     0,
       0,     0,     0,   240,   240,     0,     0,   240,     0,   345,
     451,   452,   453,     0,   240,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     454,   455,   861,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   240,   479,   240,     0,
     240,   861,     0,     0,     0,   240,     0,   217,     0,   480,
       0,     0,     0,     0,     0,     0,   345,    34,    35,    36,
     206,   240,   207,    40,   854,     0,     0,     0,     0,     0,
     208,     0,     0,     0,     0,     0,   240,   240,     0,   345,
       0,   927,    50,     0,   240,     0,   240,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   237,     0,
       0,     0,     0,   345,     0,   345,     0,     0,   240,   238,
     240,   345,     0,     0,     0,     0,   240,     0,   210,   211,
     212,   213,   214,     0,    81,    82,    83,    84,    85,   219,
       0,     0,     0,     0,     0,   215,     0,     0,     0,   240,
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,     0,     0,     0,    99,   854,   854,   854,     0,
       0,     0,     0,   854,     0,   240,   345,     0,     0,   104,
       0,   240,     0,   240,   107,   239,     0,     0,     0,     0,
     111,     0,     0,  1342,     0,     0,     0,     0,     0,   219,
     524,   495,   496,   497,   498,   499,   500,   501,   502,   503,
     504,   505,   506,     0,     0,     0,     0,     0,     0,     0,
     221,   221,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   246,     0,     0,  1025,     0,     0,     0,     0,     0,
     219,     0,   219,     0,     0,   507,   508, -1059, -1059, -1059,
   -1059, -1059,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
       0,     0,   283,     0,     0,     0,     0,     0,   219,   861,
       0,     0,     0,  1120,     0,     0,     0,     0,   345,     0,
       0,     0,     0,   240,     0,   861,   861,   861,   861,   861,
       0,     0,     0,     0,     0,   345,     0,   861,     0,   285,
     240,     0,     0,     0,   240,   240,     0,     0,     0,     0,
       0,     0,   206,  1124,  1838,     0,     0,     0,     0,   240,
     509,   510,     0,     0,     0,   854,     0,     0,     0,     0,
       0,     0,     0,     0,    50,   219,   854,     0,     0,     0,
       0,     0,   854,     0,     0,     0,   854,     0,     0,  1145,
       0,   219,   219,     0,     0,     0,     0,     0,     0,   832,
       0,     0,     0,   345,     0,     0,     0,     0,   240,   569,
     210,   211,   212,   213,   214,   570,  1145,     0,     0,     0,
       0,     0,     0,     0,     0,   219,     0,     0,     0,     0,
       0,     0,   183,     0,     0,    91,   336,     0,    93,    94,
       0,    95,   184,    97,     0,  1431,     0,     0,   854,   206,
       0,     0,   861,     0,     0,  1188,   340,     0,   240,   833,
       0,     0,     0,     0,     0,     0,   107,   341,     0,     0,
       0,    50,     0,     0,   221,   240,     0,   242,     0,   345,
       0,     0,     0,     0,   240,     0,     0,     0,     0,     0,
    1025,   345,     0,   345,     0,     0,   240,     0,   240,     0,
       0,     0,     0,     0,     0,     0,     0,   210,   211,   212,
     213,   214,   345,     0,   345,     0,     0,   240,     0,   240,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   183,
       0,     0,    91,   219,   219,    93,    94,     0,    95,   184,
      97,     0,  1265,     0,     0,     0,     0,     0,     0,     0,
    1209,  1210,  1211,   206,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   107,     0,     0,     0,     0,   861,   861,
     861,   861,   861,   861,   219,    50,     0,   861,   861,   861,
     861,   861,   861,   861,   861,   861,   861,   861,   861,   861,
     861,   861,   861,   861,   861,   861,   861,   861,   861,   861,
     861,   861,   861,   861,   861,     0,     0,   221,     0,     0,
       0,   210,   211,   212,   213,   214,   221,     0,     0,     0,
       0,   861,     0,   221,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   221,     0,     0,     0,    93,
      94,     0,    95,   184,    97,     0,   246,     0,     0,     0,
     451,   452,   453,     0,     0,     0,     0,     0,     0,    34,
      35,    36,     0,     0,     0,     0,   219,   107,     0,     0,
     454,   455,   208,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,     0,   479,     0,     0,
       0,     0,   283,  1025,  1025,  1025,  1025,  1025,  1025,   480,
       0,   219,     0,  1025,     0,     0,     0,   219,     0,     0,
     246,     0,     0,     0,     0,     0,    81,    82,    83,    84,
      85,     0,   219,   219,     0,   861,     0,   215,     0,   285,
       0,     0,     0,    89,    90,     0,     0,     0,     0,     0,
       0,     0,   206,     0,   861,     0,   861,    99,   451,   452,
     453,     0,   221,     0,     0,     0,     0,     0,     0,     0,
       0,   104,     0,     0,    50,     0,     0,     0,   454,   455,
     861,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,     0,   479,     0,     0,     0,   569,
     210,   211,   212,   213,   214,   570,     0,   480,  1465,   862,
       0,   219,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   183,     0,     0,    91,   336,     0,    93,    94,
     959,    95,   184,    97,     0,     0,     0,     0,   862,     0,
       0,     0,     0,     0,     0,     0,   340,     0,   451,   452,
     453,     0,     0,     0,     0,     0,   107,   341,     0,  1025,
       0,  1025,     0,     0,     0,     0,     0,     0,   454,   455,
     863,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,     0,   479,     0,     0,     0,   889,
       0,     0,     0,     0,     0,     0,     0,   480,     0,     0,
       0,     0,   861,     0,   861,     0,   861,     0,     0,     0,
       0,   861,   219,     0,     0,   861,   221,   861,     0,   206,
     861,   207,    40,     0,   451,   452,   453,     0,   998,     0,
       0,     0,  1567,     0,     0,  1580,     0,     0,     0,     0,
       0,    50,     0,     0,   454,   455,     0,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,     0,     0,     0,     0,   221,   210,   211,   212,
     213,   214,   206,   480,  1025,     0,  1025,     0,  1025,     0,
       0,     0,     0,  1025,   206,   219,     0,     0,     0,     0,
       0,     0,     0,   772,    50,    93,    94,     0,    95,   184,
      97,     0,   861,     0,     0,     0,    50,   221,     0,   221,
       0,     0,     0,     0,  1644,  1645,     0,     0,  1002,     0,
       0,     0,     0,   107,  1580,     0,     0,   773,     0,   111,
     210,   211,   212,   213,   214,   221,   862,     0,     0,     0,
       0,     0,   210,   211,   212,   213,   214,     0,     0,     0,
       0,     0,   862,   862,   862,   862,   862,     0,    93,    94,
       0,    95,   184,    97,   862,     0,     0,     0,  1744,     0,
      93,    94,  1745,    95,   184,    97,     0,  1025,     0,     0,
       0,     0,     0,     0,     0,     0,   107,   973,     0,     0,
       0,     0,     0,     0,   861,   861,   861,  1054,   107,  1585,
       0,   861,   221,  1792,  1136,     0,   451,   452,   453,     0,
       0,  1580,     0,  1077,  1078,  1079,  1080,  1081,   221,   221,
       0,     0,     0,     0,     0,  1091,   454,   455,  1423,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   246,   479,     0,     0,  1094,  1095,  1096,     0,
       0,     0,     0,     0,     0,   480,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1097,     0,   862,
    1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,     0,     0,   246,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1120,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1025,  1025,     0,     0,     0,     0,     0,     0,
    1185,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   861,     0,     0,     0,     0,     0,     0,
     221,   221,     0,     0,   861,     0,     0,     0,     0,     0,
     861,     0,     0,     0,   861,     0,     0,     0,     0,     0,
       0,     0,  1270,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1424,   862,   862,   862,   862,   862,
     862,   246,     0,     0,   862,   862,   862,   862,   862,   862,
     862,   862,   862,   862,   862,   862,   862,   862,   862,   862,
     862,   862,   862,   862,   862,   862,   862,   862,   862,   862,
     862,   862,     0,     0,     0,     0,   861,     0,     0,     0,
       0,     0,   206,     0,     0,     0,  1907,     0,   862,     0,
       0,     0,  1065,     0,     0,     0,  1081,  1277,     0,     0,
    1277,     0,   283,  1567,    50,  1290,  1293,  1294,  1295,  1297,
    1298,  1299,  1300,  1301,  1302,  1303,  1304,  1305,  1306,  1307,
    1308,  1309,  1310,  1311,  1312,  1313,  1314,  1315,  1316,  1317,
    1318,  1319,  1320,   221,     0,     0,     0,     0,     0,   285,
     210,   211,   212,   213,   214,     0,     0,     0,     0,  1328,
       0,     0,   206,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   183,     0,     0,    91,     0,     0,    93,    94,
       0,    95,   184,    97,    50,     0,     0,     0,   246,     0,
       0,     0,  -396,     0,   221,     0,     0,     0,     0,     0,
     439,   180,   181,    65,    66,    67,   107,     0,     0,   221,
     221,     0,   862,     0,     0,     0,     0,     0,     0,   569,
     210,   211,   212,   213,   214,   570,     0,     0,     0,     0,
       0,   862,     0,   862,     0,     0,     0,     0,     0,     0,
       0,     0,   183,     0,     0,    91,   336,     0,    93,    94,
       0,    95,   184,    97,     0,   273,   274,   862,   275,   276,
       0,     0,   277,   278,   279,   280,   340,     0,     0,     0,
       0,     0,   440,  1418,     0,     0,   107,   341,     0,     0,
     281,   282,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1434,     0,  1435,     0,     0,     0,   221,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   284,
       0,     0,     0,     0,     0,     0,     0,     0,  1455,     0,
       0,     0,     0,   286,   287,   288,   289,   290,   291,   292,
       0,     0,     0,   206,     0,   207,    40,     0,     0,     0,
       0,     0,     0,     0,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,    50,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,     0,
     327,     0,   742,   329,   330,   331,     0,     0,     0,   332,
     580,   210,   211,   212,   213,   214,   581,     0,     0,   862,
       0,   862,     0,   862,     0,     0,     0,     0,   862,   246,
       0,     0,   862,   582,   862,     0,     0,   862,     0,    93,
      94,     0,    95,   184,    97,   337,     0,   338,     0,     0,
     339,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   107,     0,     0,
       0,   743,     0,   111,     0,     0,     0,     0,     0,     0,
    1549,     0,  1550,     0,  1551,     0,     0,     0,     0,  1552,
       0,     0,     0,  1554,     0,  1555,     0,     0,  1556,     0,
       0,     0,     0,     0,     0,     0,     0,   451,   452,   453,
       0,     0,   246,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   454,   455,   862,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   480,  1102,  1103,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,     5,     6,     7,     8,     9,
    1639,     0,     0,     0,     0,    10,     0,     0,  1120,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
     412,    13,     0,     0,     0,     0,     0,     0,     0,     0,
     755,   862,   862,   862,     0,     0,     0,     0,   862,     0,
       0,     0,    15,    16,     0,     0,     0,  1797,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,  1785,  1786,  1787,    50,     0,     0,     0,  1791,
       0,     0,     0,    55,     0,     0,     0,  1195,     0,     0,
       0,   179,   180,   181,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   182,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     862,     0,     0,   111,   112,     0,   113,   114,     0,     0,
       0,   862,     0,     0,     0,     0,     0,   862,     0,     0,
       0,   862,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,  1880,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,  1847,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1857,     0,     0,     0,     0,    14,  1862,    15,
      16,     0,  1864,   862,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,    56,    57,    58,  1899,    59,    60,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
      88,    89,    90,    91,    92,     0,    93,    94,     0,    95,
      96,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,   103,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1155,
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
     109,   110,  1343,   111,   112,     0,   113,   114,     5,     6,
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
     106,     0,     0,   107,   108,     0,   109,   110,   680,   111,
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
     110,  1123,   111,   112,     0,   113,   114,     5,     6,     7,
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
     108,     0,   109,   110,  1169,   111,   112,     0,   113,   114,
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
       0,     0,   107,   108,     0,   109,   110,  1242,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,  1244,    47,     0,    48,     0,
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
       0,    48,     0,    49,  1419,     0,    50,    51,     0,     0,
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
       0,   107,   108,     0,   109,   110,  1558,   111,   112,     0,
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
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1788,
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
      48,  1834,    49,     0,     0,    50,    51,     0,     0,     0,
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
     107,   108,     0,   109,   110,  1869,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,  1872,    48,     0,    49,     0,
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
     110,  1889,   111,   112,     0,   113,   114,     5,     6,     7,
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
     108,     0,   109,   110,  1906,   111,   112,     0,   113,   114,
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
       0,     0,   107,   108,     0,   109,   110,  1963,   111,   112,
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
    1966,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
       0,     0,     0,    11,    12,    13,     0,     0,   552,     0,
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
       0,   819,     0,     0,     0,     0,     0,     0,     0,     0,
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
      12,    13,     0,     0,  1056,     0,     0,     0,     0,     0,
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
       0,     0,    11,    12,    13,     0,     0,  1633,     0,     0,
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
    1780,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,    11,     0,    13,     0,     0,     0,     0,     0,     0,
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
     185,     0,   350,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,  1097,     0,
      10,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,     0,     0,   695,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1120,    15,    16,     0,
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
       0,   696,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   185,     0,     0,     0,     0,   111,   112,
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
       0,    55,     0,     0,     0,     0,     0,     0,     0,   179,
     180,   181,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   182,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   185,     0,     0,   814,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,  1098,  1099,  1100,
    1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,     0,
       0,  1182,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1120,    15,    16,     0,     0,     0,     0,    17,
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
      93,    94,     0,    95,   184,    97,     0,  1183,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   185,
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
       0,     0,     0,     0,     0,   179,   180,   181,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   182,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,   451,   452,   453,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   454,   455,     0,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,     0,
     479,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,   480,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,   197,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,   179,   180,
     181,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   182,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,  1206,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   185,     0,     0,     0,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,     0,   479,     0,
     233,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     480,     0,    15,    16,     0,     0,     0,     0,    17,     0,
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
       0,     0,   104,   105,   106,     0,     0,   107,   185,   451,
     452,   453,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   454,
     455,     0,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,     0,   479,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,   480,     0,
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
       0,    99,     0,     0,   100,     0,     0,     0,     0,  1236,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   185,     0,   268,   452,   453,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,   454,   455,     0,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,     0,
     479,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,   480,     0,    17,     0,    18,    19,    20,    21,
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
     179,   180,   181,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   182,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   183,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   184,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   108,   451,   452,
     453,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   454,   455,
       0,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,     0,   479,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,   480,     0,     0,
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
      99,     0,     0,   100,     0,     0,     0,     0,  1603,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     185,   550,     0,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   709,
     479,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   480,     0,     0,     0,     0,    15,    16,     0,
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
       0,     0,     0,    10,  1099,  1100,  1101,  1102,  1103,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,     0,     0,     0,   755,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1120,     0,
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
       9,     0,     0,     0,     0,     0,    10,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,     0,   479,     0,
       0,   796,     0,     0,     0,     0,     0,     0,     0,     0,
     480,     0,     0,    15,    16,     0,     0,     0,     0,    17,
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
    1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
       0,     0,     0,     0,   798,     0,     0,     0,     0,     0,
       0,     0,     0,  1120,     0,     0,    15,    16,     0,     0,
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
       0,     0,    10,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,     0,   479,     0,     0,     0,  1233,     0,     0,
       0,     0,     0,     0,     0,   480,     0,     0,     0,    15,
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
     105,   106,     0,     0,   107,   185,   451,   452,   453,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   454,   455,     0,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,     0,   479,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   480,     0,     0,    17,     0,
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
       0,   100,     0,     0,     0,     0,  1604,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   185,   451,
     452,   453,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,   823,     0,    10,   454,
     455,     0,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,     0,   479,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,   480,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,   640,    39,    40,
       0,   824,     0,     0,     0,     0,    43,     0,     0,     0,
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
     114,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,     0,   479,
       0,     0,     0,   284,     0,     0,     0,     0,     0,     0,
       0,   480,     0,     0,     0,     0,     0,   286,   287,   288,
     289,   290,   291,   292,     0,     0,     0,   206,     0,   207,
      40,     0,     0,     0,     0,     0,     0,     0,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,    50,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   206,   327,     0,   328,   329,   330,   331,
       0,     0,     0,   332,   580,   210,   211,   212,   213,   214,
     581,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,   273,   274,     0,   275,   276,     0,   582,   277,   278,
     279,   280,     0,    93,    94,     0,    95,   184,    97,   337,
       0,   338,     0,     0,   339,     0,   281,   282,     0,   283,
       0,   210,   211,   212,   213,   214,     0,     0,     0,     0,
       0,   107,     0,     0,     0,   743,     0,   111,     0,     0,
       0,     0,     0,   183,     0,   284,    91,    92,     0,    93,
      94,     0,    95,   184,    97,     0,   285,     0,     0,   286,
     287,   288,   289,   290,   291,   292,     0,     0,     0,   206,
       0,     0,     0,     0,     0,     0,     0,   107,     0,     0,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,    50,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,     0,   327,     0,     0,   329,
     330,   331,     0,     0,     0,   332,   333,   210,   211,   212,
     213,   214,   334,     0,     0,     0,     0,     0,     0,     0,
     206,     0,   921,     0,   922,     0,     0,     0,     0,   335,
       0,     0,    91,   336,     0,    93,    94,     0,    95,   184,
      97,   337,    50,   338,     0,     0,   339,   273,   274,     0,
     275,   276,     0,   340,   277,   278,   279,   280,     0,     0,
       0,     0,     0,   107,   341,     0,     0,     0,  1759,     0,
       0,     0,   281,   282,     0,   283,     0,     0,   210,   211,
     212,   213,   214,     0, -1059, -1059, -1059, -1059,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   284,   479,     0,     0,     0,    93,    94,     0,    95,
     184,    97,   285,     0,   480,   286,   287,   288,   289,   290,
     291,   292,     0,     0,     0,   206,     0,     0,     0,     0,
       0,     0,     0,     0,   107,     0,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,    50,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,     0,   327,     0,     0,   329,   330,   331,     0,     0,
       0,   332,   333,   210,   211,   212,   213,   214,   334,     0,
       0,     0,     0,     0,     0,     0,   206,     0,     0,     0,
       0,     0,     0,     0,     0,   335,     0,     0,    91,   336,
       0,    93,    94,     0,    95,   184,    97,   337,    50,   338,
       0,     0,   339,   273,   274,     0,   275,   276,     0,   340,
     277,   278,   279,   280,     0,     0,     0,     0,     0,   107,
     341,     0,     0,     0,  1829,     0,     0,     0,   281,   282,
       0,   283,     0,     0,   210,   211,   212,   213,   214,     0,
    1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,   284,     0,     0,
     436,     0,    93,    94,     0,    95,   184,    97,   285,     0,
    1120,   286,   287,   288,   289,   290,   291,   292,     0,     0,
       0,   206,     0,     0,     0,     0,     0,     0,     0,     0,
     107,     0,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,    50,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,     0,   327,     0,
     328,   329,   330,   331,     0,     0,     0,   332,   333,   210,
     211,   212,   213,   214,   334,     0,     0,     0,     0,     0,
       0,     0,   206,     0,     0,     0,     0,     0,     0,     0,
       0,   335,     0,     0,    91,   336,     0,    93,    94,     0,
      95,   184,    97,   337,    50,   338,     0,     0,   339,   273,
     274,     0,   275,   276,     0,   340,   277,   278,   279,   280,
       0,     0,     0,     0,     0,   107,   341,     0,     0,     0,
       0,     0,     0,     0,   281,   282,     0,   283,     0,     0,
     210,   211,   212,   213,   214,     0, -1059, -1059, -1059, -1059,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,   284,     0,   359,     0,     0,    93,    94,
       0,    95,   184,    97,   285,     0,  1120,   286,   287,   288,
     289,   290,   291,   292,     0,     0,     0,   206,     0,     0,
       0,     0,     0,     0,     0,     0,   107,     0,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,    50,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,     0,   327,     0,     0,   329,   330,   331,
       0,     0,     0,   332,   333,   210,   211,   212,   213,   214,
     334,     0,     0,     0,     0,     0,     0,     0,   206,     0,
       0,     0,     0,     0,     0,     0,     0,   335,     0,     0,
      91,   336,     0,    93,    94,     0,    95,   184,    97,   337,
      50,   338,     0,     0,   339,     0,   273,   274,     0,   275,
     276,   340,  1562,   277,   278,   279,   280,     0,     0,     0,
       0,   107,   341,     0,     0,     0,     0,     0,     0,     0,
       0,   281,   282,     0,   283,     0,   210,   211,   212,   213,
     214,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     284,   879,     0,     0,    93,    94,     0,    95,   184,    97,
       0,   285,     0,     0,   286,   287,   288,   289,   290,   291,
     292,     0,     0,     0,   206,     0,     0,     0,     0,     0,
       0,     0,   107,     0,     0,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,    50,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
       0,   327,     0,     0,   329,   330,   331,     0,     0,     0,
     332,   333,   210,   211,   212,   213,   214,   334,     0,     0,
       0,     0,     0,     0,     0,   206,     0,     0,     0,     0,
       0,     0,     0,     0,   335,     0,     0,    91,   336,     0,
      93,    94,     0,    95,   184,    97,   337,    50,   338,     0,
       0,   339,  1659,  1660,  1661,  1662,  1663,     0,   340,  1664,
    1665,  1666,  1667,     0,     0,     0,     0,     0,   107,   341,
       0,     0,     0,     0,     0,     0,  1668,  1669,  1670,     0,
       0,     0,     0,   210,   211,   212,   213,   214,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1671,     0,     0,     0,
       0,    93,    94,     0,    95,   184,    97,     0,     0,     0,
    1672,  1673,  1674,  1675,  1676,  1677,  1678,     0,     0,     0,
     206,     0,     0,     0,     0,     0,     0,     0,     0,   107,
       0,  1679,  1680,  1681,  1682,  1683,  1684,  1685,  1686,  1687,
    1688,  1689,    50,  1690,  1691,  1692,  1693,  1694,  1695,  1696,
    1697,  1698,  1699,  1700,  1701,  1702,  1703,  1704,  1705,  1706,
    1707,  1708,  1709,  1710,  1711,  1712,  1713,  1714,  1715,  1716,
    1717,  1718,  1719,     0,     0,     0,  1720,  1721,   210,   211,
     212,   213,   214,     0,  1722,  1723,  1724,  1725,  1726,     0,
       0,     0,     0,     0,   206,     0,   207,    40,     0,     0,
    1727,  1728,  1729,     0,     0,     0,    93,    94,     0,    95,
     184,    97,  1730,     0,  1731,  1732,    50,  1733,     0,     0,
       0,     0,     0,     0,  1734,  1735,     0,  1736,     0,  1737,
    1738,     0,   273,   274,   107,   275,   276,     0,     0,   277,
     278,   279,   280,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   210,   211,   212,   213,   214,   281,   282,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   772,     0,
      93,    94,     0,    95,   184,    97,   284,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     286,   287,   288,   289,   290,   291,   292,     0,   107,     0,
     206,     0,   806,     0,   111,     0,     0,     0,     0,     0,
       0,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,    50,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,     0,   327,     0,   328,
     329,   330,   331,     0,     0,     0,   332,   580,   210,   211,
     212,   213,   214,   581,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   273,   274,     0,   275,   276,     0,
     582,   277,   278,   279,   280,     0,    93,    94,     0,    95,
     184,    97,   337,     0,   338,     0,     0,   339,     0,   281,
     282,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   107,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   284,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   286,   287,   288,   289,   290,   291,   292,     0,
       0,     0,   206,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,    50,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,     0,   327,
       0,  1288,   329,   330,   331,     0,     0,     0,   332,   580,
     210,   211,   212,   213,   214,   581,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   273,   274,     0,   275,
     276,     0,   582,   277,   278,   279,   280,     0,    93,    94,
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
       0,   327,     0,     0,   329,   330,   331,     0,     0,     0,
     332,   580,   210,   211,   212,   213,   214,   581,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   582,     0,     0,     0,     0,     0,
      93,    94,     0,    95,   184,    97,   337,     0,   338,     0,
       0,   339,   451,   452,   453,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   107,     0,
       0,     0,   454,   455,     0,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,     0,   479,
     451,   452,   453,     0,     0,     0,     0,     0,     0,     0,
       0,   480,     0,     0,     0,     0,     0,     0,     0,     0,
     454,   455,     0,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,     0,   479,   451,   452,
     453,     0,     0,     0,     0,     0,     0,     0,     0,   480,
       0,     0,     0,     0,     0,     0,     0,     0,   454,   455,
       0,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,     0,   479,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   480,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   451,   452,
     453,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   454,   455,
     481,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,     0,   479,   451,   452,   453,     0,
       0,     0,     0,     0,     0,     0,     0,   480,     0,     0,
       0,     0,     0,     0,     0,     0,   454,   455,   566,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,     0,   479,   451,   452,   453,     0,     0,     0,
       0,     0,     0,     0,     0,   480,     0,     0,     0,     0,
       0,     0,     0,     0,   454,   455,   568,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   480,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   451,   452,   453,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   454,   455,   587,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,     0,     0,  1296,     0,     0,     0,     0,     0,
       0,     0,     0,   480,     0,     0,     0,     0,     0,     0,
       0,     0,   841,   842,   591,     0,     0,     0,   843,     0,
     844,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   845,     0,     0,     0,     0,     0,     0,     0,
      34,    35,    36,   206,     0,     0,     0,   451,   452,   453,
       0,     0,     0,   208,     0,     0,     0,     0,     0,     0,
       0,   788,     0,     0,     0,    50,     0,   454,   455,     0,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,     0,     0,     0,     0,     0,
     846,   847,   848,   849,   850,   851,   480,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   215,  1050,
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,   811,    95,   184,    97,     0,     0,     0,    99,     0,
       0,     0,     0,     0,     0,     0,     0,   852,     0,     0,
       0,    29,   104,     0,     0,     0,     0,   107,   853,    34,
      35,    36,   206,     0,   207,    40,     0,     0,     0,     0,
       0,     0,   208,   527,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     209,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1051,    75,
     210,   211,   212,   213,   214,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,   215,     0,     0,
       0,     0,   183,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   184,    97,     0,   841,   842,    99,     0,     0,
       0,   843,     0,   844,     0,     0,     0,     0,     0,     0,
       0,   104,     0,     0,     0,   845,   107,   216,     0,     0,
       0,     0,   111,    34,    35,    36,   206,     0,     0,     0,
     451,   452,   453,     0,     0,     0,   208,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
     454,   455,     0,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,     0,   479,     0,     0,
       0,     0,     0,   846,   847,   848,   849,   850,   851,   480,
      81,    82,    83,    84,    85,     0,     0,     0,  1004,  1005,
       0,   215,     0,     0,     0,     0,   183,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   184,    97,  1006,     0,
       0,    99,     0,     0,     0,     0,  1007,  1008,  1009,   206,
     852,     0,     0,     0,     0,   104,     0,     0,     0,  1010,
     107,   853,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,   536,     0,     0,    29,
       0,     0,     0,     0,     0,     0,     0,    34,    35,    36,
     206,     0,   207,    40,     0,     0,     0,     0,     0,     0,
     208,     0,     0,     0,     0,     0,  1011,  1012,  1013,  1014,
    1015,  1016,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1017,     0,     0,     0,   209,   183,
       0,     0,    91,    92,     0,    93,    94,     0,    95,   184,
      97,     0,     0,     0,     0,     0,     0,    75,   210,   211,
     212,   213,   214,  1018,    81,    82,    83,    84,    85,     0,
       0,     0,     0,   107,     0,   215,     0,     0,     0,     0,
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,    29,     0,     0,    99,     0,     0,     0,     0,
      34,    35,    36,   206,     0,   207,    40,     0,     0,   104,
       0,     0,     0,   208,   107,   216,     0,     0,   603,     0,
     111,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   209,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   623,
      75,   210,   211,   212,   213,   214,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   215,     0,
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,    29,   993,     0,    99,     0,
       0,     0,     0,    34,    35,    36,   206,     0,   207,    40,
       0,     0,   104,     0,     0,     0,   208,   107,   216,     0,
       0,     0,     0,   111,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   209,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,   210,   211,   212,   213,   214,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,   215,     0,     0,     0,     0,   183,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   184,    97,    29,     0,
       0,    99,     0,     0,     0,     0,    34,    35,    36,   206,
       0,   207,    40,     0,     0,   104,     0,     0,     0,   208,
     107,   216,     0,     0,     0,     0,   111,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   209,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1148,    75,   210,   211,   212,
     213,   214,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,   215,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,    29,     0,     0,    99,     0,     0,     0,     0,    34,
      35,    36,   206,     0,   207,    40,     0,     0,   104,     0,
       0,     0,   208,   107,   216,     0,     0,     0,     0,   111,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     209,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
     210,   211,   212,   213,   214,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,   215,     0,     0,
       0,     0,   183,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   184,    97,     0,     0,     0,    99,     0,     0,
     451,   452,   453,     0,     0,     0,     0,     0,     0,     0,
       0,   104,     0,     0,     0,     0,   107,   216,     0,     0,
     454,   455,   111,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,     0,   479,   451,   452,
     453,     0,     0,     0,     0,     0,     0,     0,     0,   480,
       0,     0,     0,     0,     0,     0,     0,     0,   454,   455,
       0,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,     0,   479,     0,     0,     0,     0,
       0,     0,     0,     0,   451,   452,   453,   480,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   454,   455,   911,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,   451,   452,   453,     0,     0,     0,     0,     0,
       0,     0,     0,   480,     0,     0,     0,     0,     0,     0,
       0,     0,   454,   455,   979,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,     0,   479,
       0,     0,     0,     0,     0,     0,     0,     0,   451,   452,
     453,   480,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   454,   455,
    1035,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,     0,   479,  1094,  1095,  1096,     0,
       0,     0,     0,     0,     0,     0,     0,   480,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1097,  1341,     0,
    1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1094,  1095,  1096,     0,  1120,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1097,     0,  1372,  1098,  1099,  1100,  1101,  1102,
    1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,     0,     0,  1094,
    1095,  1096,     0,     0,     0,     0,     0,     0,     0,     0,
    1120,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1097,     0,  1440,  1098,  1099,  1100,  1101,  1102,  1103,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1094,  1095,  1096,     0,  1120,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1097,     0,  1451,  1098,  1099,
    1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
       0,     0,  1094,  1095,  1096,     0,     0,     0,     0,     0,
       0,     0,     0,  1120,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1097,     0,  1548,  1098,  1099,  1100,  1101,
    1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,     0,    34,
      35,    36,   206,     0,   207,    40,     0,     0,     0,     0,
       0,  1120,   654,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
    1640,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     209,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     210,   211,   212,   213,   214,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,   215,  1642,     0,
       0,     0,   183,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   184,    97,     0,     0,     0,    99,     0,    34,
      35,    36,   206,     0,   207,    40,     0,     0,     0,     0,
       0,   104,   208,   206,     0,     0,   107,   655,     0,     0,
       0,     0,   656,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
     237,     0,     0,   357,   358,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     210,   211,   212,   213,   214,     0,    81,    82,    83,    84,
      85,   210,   211,   212,   213,   214,   206,   215,     0,     0,
       0,     0,   183,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   184,    97,     0,     0,   359,    99,    50,    93,
      94,     0,    95,   184,    97,     0,   877,   878,     0,     0,
       0,   104,     0,     0,     0,     0,   107,   239,     0,     0,
       0,     0,   111,     0,     0,     0,     0,   107,     0,     0,
       0,     0,     0,     0,   210,   211,   212,   213,   214,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   879,
       0,     0,    93,    94,     0,    95,   184,    97,     0,     0,
       0,   451,   452,   453,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     107,   454,   455,   976,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,     0,   479,   451,
     452,   453,     0,     0,     0,     0,     0,     0,     0,     0,
     480,     0,     0,     0,     0,     0,     0,     0,     0,   454,
     455,     0,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,     0,   479,  1094,  1095,  1096,
       0,     0,     0,     0,     0,     0,     0,     0,   480,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1097,  1456,
       0,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1094,  1095,  1096,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1120,     0,     0,     0,
       0,     0,     0,     0,  1097,     0,     0,  1098,  1099,  1100,
    1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1095,
    1096,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1120,     0,     0,     0,     0,     0,     0,  1097,
       0,     0,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,   453,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1120,     0,     0,
       0,     0,   454,   455,     0,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,  1096,   479,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   480,     0,     0,     0,     0,     0,  1097,     0,     0,
    1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   454,   455,  1120,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,     0,
     479,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   455,   480,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,     0,   479,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   480,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   480
};

static const yytype_int16 yycheck[] =
{
       5,     6,   129,     8,     9,    10,    11,    12,    13,    56,
      15,    16,    17,    18,     4,    56,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   187,    31,   108,   161,   405,
       4,   672,    98,    33,   405,     4,   102,   103,     4,    44,
     240,   405,   521,    57,   515,   516,    46,    52,  1172,    54,
     540,    51,    57,    30,    59,    30,     4,   698,    60,   668,
     166,   188,   128,    56,   669,   941,   161,   952,    30,  1333,
     479,   108,   357,   358,   829,   546,   752,   511,   597,   598,
     511,    86,   647,   822,    57,   972,    88,  1049,  1159,    91,
     800,   108,   251,   108,     9,  1168,     9,     9,    32,   790,
       9,   988,    14,   108,     9,    14,    48,     9,     9,    48,
       9,     4,    30,    14,   548,     9,    32,   548,    50,    51,
       9,    54,     9,     9,     9,    83,    84,    38,    48,    48,
       9,    83,     9,     9,     9,     9,   160,     9,     9,     9,
       9,     9,    36,     9,     9,     9,   252,   115,   185,  1036,
      90,     4,    48,    90,   160,    38,     6,   102,    81,   106,
     107,    83,   106,   107,   134,   135,    57,  1773,   185,   160,
     185,   102,    83,   181,   160,   199,    38,   181,    69,   216,
     185,    38,   181,     0,   105,   106,   107,   192,   196,   181,
     196,    32,   196,    44,    38,   181,     8,   196,    48,   216,
      83,   216,   239,    38,   196,   196,   160,   130,   160,    70,
     178,   216,   196,   199,  1090,   236,    70,   157,    70,   164,
     157,    83,   239,    70,  1830,    70,    83,   181,    83,    84,
     200,   390,   165,   164,   239,    86,   158,   159,    70,    83,
     881,    75,    76,   201,    70,    70,    70,    70,    83,   254,
     182,   193,   257,   200,   193,   197,   200,    70,   175,   264,
     265,    70,   112,   174,    70,   199,    54,   117,   161,   119,
     120,   121,   122,   123,   124,   125,   129,   197,   197,   441,
     181,   991,   198,   198,   199,   198,   198,  1249,    70,   198,
     283,   174,   285,   198,  1367,  1559,   198,   198,  1369,   198,
      83,   197,   349,   197,  1059,  1376,  1061,  1378,   349,   198,
     258,   198,   198,   198,   262,  1006,  1203,   167,   168,   198,
     170,   198,   198,   198,   198,   197,   197,   197,   197,   197,
     174,   197,   197,   197,   160,   188,  1407,   160,   199,   174,
     377,   191,   196,    83,   196,   199,   201,   160,   341,   196,
     200,  1227,   134,   135,   199,   357,   358,   359,   877,   878,
     377,   523,   377,   160,   969,   930,   493,   199,    83,   435,
     196,   376,   377,   199,   199,   199,   199,   165,   383,   384,
     385,   386,   387,   388,   181,   432,   199,   196,   393,    31,
     199,   432,   180,   199,    70,   397,  1222,   552,  1224,   196,
     120,    70,   199,   196,   166,   181,   164,   412,    50,   192,
     130,    53,    83,   494,   102,   420,   199,   199,    70,    70,
     196,   411,   488,   489,   490,   491,   431,   196,   907,  1502,
      70,  1504,    70,   588,   174,   121,   429,   199,   485,   432,
     198,   199,  1513,   386,   130,   388,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,    70,   480,   164,   482,   483,   484,
     494,  1157,    83,  1087,   790,  1089,   201,   534,   405,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   479,   174,   479,     4,  1245,   512,   513,   530,
     515,   516,   517,   518,   985,   670,   675,   479,   677,   524,
     196,   494,   527,   199,   958,   376,   419,   958,  1219,  1220,
     199,   536,  1223,   538,   385,   386,    70,   388,  1229,  1612,
     554,   546,   196,  1616,  1467,   745,    83,   199,   199,   554,
     102,   556,   196,   686,    53,    83,   199,    56,   196,   197,
      70,   479,  1171,   134,   135,   941,   419,   485,   485,    14,
     941,     6,  1448,   174,    73,   196,    83,   941,   778,   196,
     573,   196,  1187,    90,   993,  1190,   871,   196,   873,    50,
      51,   686,   754,   511,   511,   597,   598,    96,   603,    98,
     134,   135,   196,   102,   103,   134,   135,   196,   629,   630,
     196,   559,   164,    48,  1133,    70,   534,   534,   655,   160,
     157,   158,   159,   164,   134,   135,   160,   545,   545,   128,
     548,   548,    70,     4,    83,   794,   795,   658,   704,   492,
     181,    90,   801,   802,    70,  1568,   174,   165,    19,    20,
     655,   158,   159,    83,   854,  1531,    14,   102,   103,   814,
      90,    70,   196,   483,   819,    70,   866,   196,   164,  1592,
     196,  1594,   665,   666,    32,   199,  1940,   112,    38,   669,
     196,   674,   117,   198,   119,   120,   121,   122,   123,   124,
     125,   696,   512,    51,  1385,   801,  1387,   517,   205,  1772,
    1006,  1377,   201,  1776,   709,    53,    54,    55,   198,   158,
     159,   105,   106,   107,  1090,  1205,    83,   102,   103,  1090,
     741,    69,   198,    90,   198,  1196,  1090,   157,   158,   159,
     198,   111,   167,   168,   198,   170,  1207,   108,   743,   119,
     120,   121,   122,   123,   124,    75,    76,  1356,   198,   119,
     120,   121,   122,   123,   124,   204,   191,   132,   133,   258,
     122,   123,   124,   262,   198,   200,  1371,   266,   773,  1248,
     112,   113,   114,    70,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,   198,   199,    83,
     157,   158,   159,   686,  1918,    70,    90,    83,   198,   199,
      70,   806,   166,    31,    90,    83,  1482,    70,  1932,    70,
     831,   191,    90,    70,   185,   179,   837,   822,   818,    59,
      60,   191,    50,  1804,  1805,    53,   357,   358,   198,  1520,
     199,  1522,    70,  1524,   160,   199,  1800,  1801,  1529,  1912,
     196,   839,   840,   196,    70,   216,   839,   840,   160,   196,
     349,  1227,   198,   164,  1927,    49,  1227,    69,  1337,   181,
     160,   196,   233,  1227,   158,   159,   203,   196,   239,   871,
       9,   873,   158,   159,     8,   877,   878,   879,   160,   827,
     158,   159,   903,   160,    53,    54,    55,   258,    57,   196,
     198,   262,   196,  1569,   134,   135,   160,   944,    14,   198,
      69,  1056,  1373,   198,   160,     9,   911,   199,   913,  1514,
     915,   198,    14,  1219,  1220,  1221,  1222,  1223,  1224,   812,
     419,   926,   130,  1229,   130,   197,   992,   181,    14,   428,
     102,   197,  1623,   432,   202,   940,   435,   197,   931,   929,
     111,   119,   120,   121,   122,   123,   124,    50,    51,    52,
      53,    54,    55,   197,   947,   929,   197,   197,     9,   812,
     929,   966,   952,   929,   196,   196,    69,   960,   391,   196,
     157,   976,   395,   197,   979,   868,   981,   197,  1178,   969,
     985,   929,   197,   197,    94,     9,   485,   486,   487,   488,
     489,   490,   491,   198,    14,   366,   989,   181,   421,  1884,
     423,   424,   425,   426,   375,   196,   377,     9,   196,  1030,
       6,   382,   511,   191,   199,   868,   993,   198,   993,  1904,
     199,   198,    83,   394,   941,   198,   944,   944,  1913,  1500,
    1035,   993,   197,   197,   197,   534,   929,   132,   956,   956,
     958,   958,  1042,   196,   203,  1092,   197,     9,   419,   548,
       9,   203,    48,   203,   203,  1076,   203,   950,    70,    32,
     559,   133,  1083,   180,   160,   136,     9,   197,   160,  1150,
    1063,   193,  1448,    14,  1067,   993,   929,  1448,     9,  1385,
     579,  1387,     9,   182,  1448,   197,     9,  1778,  1779,   119,
     120,   121,   122,   123,   124,  1043,    14,   950,   132,   200,
     130,   131,   601,   602,     9,   203,   203,    14,   197,   203,
     197,     6,   203,   196,   160,   197,   112,   102,   198,  1037,
    1037,   117,   198,   119,   120,   121,   122,   123,   124,   125,
       9,  1133,   136,   160,     9,   197,  1150,   636,   637,   196,
      70,   171,    70,   173,    70,  1150,    70,   196,    70,   199,
     521,     9,  1045,    48,  1047,  1531,   186,   200,   188,    14,
    1531,   191,   198,   182,     9,    14,  1156,  1531,  1323,    32,
     199,   167,   168,  1090,   170,   203,   199,  1170,  1183,  1172,
      14,   197,  1156,  1824,   198,   193,   196,  1156,   559,   196,
    1156,  1196,  1045,    32,  1047,   191,  1217,  1187,    14,   196,
    1190,   196,  1207,  1208,   200,   704,  1199,    14,  1156,  1202,
      52,    70,   196,    70,  1520,   196,  1522,   112,  1524,    70,
      70,   160,   117,  1529,   119,   120,   121,   122,   123,   124,
     125,     9,    70,   197,   136,   198,   182,  1237,   198,   196,
    1245,    50,    51,    52,    53,    54,    55,    14,    57,   136,
    1255,   160,     9,   197,  1409,  1276,    69,     9,   203,  1280,
      69,  1902,    83,  1156,  1285,   200,   200,     9,  1261,   198,
    1425,  1292,   167,   168,  1267,   170,   196,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     136,   790,  1240,   792,   198,   196,   191,  1938,    14,    83,
     197,   199,   196,  1156,     6,   200,   196,   199,   197,   136,
    1227,   199,   198,   812,     9,    91,   157,  1623,   203,   199,
      32,    77,    59,    60,   695,   198,   197,   826,   827,   198,
     182,   136,    32,   197,     9,   197,  1341,  1330,  1331,     9,
     136,    19,    20,  1348,     9,  1335,    48,  1352,     9,  1354,
    1483,  1478,   200,   197,    14,  1345,   203,  1362,    83,  1252,
     203,   197,     9,   203,   196,   200,   203,  1372,  1373,   868,
     203,  1392,   198,   198,   197,  1396,   875,   876,   198,  1534,
     199,  1371,  1403,   197,   755,   197,   196,   136,  1543,   119,
     120,   121,   122,   123,   124,   199,   197,   134,   135,  1252,
     130,   131,  1557,   197,   203,   904,   203,     9,   203,   136,
     112,   203,   203,     9,   197,   117,    32,   119,   120,   121,
     122,   123,   124,   125,   198,   796,   197,   798,  1590,   197,
     929,   136,   198,   198,   169,   199,   112,  1430,   198,  1432,
      14,   812,  1335,   173,   165,   944,    83,   197,   117,   197,
     136,   950,  1345,   824,   199,   136,   827,   956,   197,   958,
     197,   191,    14,   181,   198,   167,   168,   199,   170,    83,
      14,    14,  1778,  1779,    83,   197,   196,   136,  1633,   136,
     197,    14,    14,   198,   198,  1478,    14,   198,     9,   191,
     199,     9,  1497,   992,   200,  1500,  1562,   868,   200,    68,
      83,   181,   196,    83,     9,  1004,  1005,  1006,   199,   198,
     115,   102,   160,  1503,   102,   182,   172,    36,  1565,  1509,
     891,  1511,    14,   196,  1514,   197,   178,   198,   196,   182,
      83,  1448,   182,   175,   197,     9,   907,   908,  1037,    14,
     198,    83,   197,  1533,  1043,   199,  1045,    83,  1047,     9,
     195,   197,  1545,    14,    83,   233,    14,    83,   929,    14,
      83,  1133,  1893,  1909,   986,  1458,   491,   486,  1246,  1068,
    1632,   488,  1904,   932,  1422,  1619,  1570,  1657,  1742,   950,
     605,  1473,  1949,  1925,  1754,  1469,  1615,   387,  1088,  1223,
    1483,  1160,  1540,  1092,  1218,  1084,  1758,  1005,   383,  1219,
    1032,   956,  1607,   432,   839,  1458,  1936,  1951,  1859,  1141,
    1503,  1069,  1461,  1121,  1531,  1770,  1509,    -1,  1511,    -1,
      -1,    -1,  1121,    -1,  1477,  1780,    -1,    -1,    -1,    -1,
      -1,  1621,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,
    1533,    -1,  1535,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1544,    -1,    -1,    -1,    -1,    -1,  1156,    -1,    -1,
      -1,    -1,  1655,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1825,    -1,  1043,    -1,  1045,    -1,  1047,    -1,  1049,  1050,
      -1,    -1,  1535,  1631,  1632,    -1,    -1,    -1,   366,    -1,
      -1,  1544,    -1,    -1,    -1,    -1,    -1,   375,    -1,    -1,
      -1,    -1,    -1,    -1,   382,    -1,  1753,    -1,    -1,    -1,
      -1,    -1,  1753,    -1,    -1,  1870,   394,    -1,    -1,    -1,
    1219,  1220,  1221,  1222,  1223,  1224,    -1,   405,  1621,    -1,
    1229,    -1,    -1,    -1,    -1,    -1,    -1,  1630,    -1,    -1,
      -1,  1240,     6,  1636,  1877,  1598,    -1,    -1,    -1,    -1,
    1643,    -1,  1818,  1252,    -1,    -1,    -1,    -1,  1763,    -1,
      -1,    -1,    -1,  1262,  1757,    -1,    -1,    -1,    -1,    -1,
    1897,    -1,    -1,  1794,  1795,    -1,    -1,  1630,    -1,    -1,
      -1,    -1,  1937,  1636,    48,  1156,    -1,  1942,    -1,    -1,
    1643,    -1,    -1,    -1,    -1,    19,    20,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,
      -1,  1182,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    56,    -1,    -1,
    1329,    -1,    56,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,   521,    -1,    -1,    -1,    -1,   112,  1842,
      -1,    -1,   233,   117,    -1,   119,   120,   121,   122,   123,
     124,   125,  1233,    -1,    -1,    -1,    -1,    -1,    -1,  1240,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1248,  1249,    -1,
      -1,  1252,    -1,    -1,    -1,    -1,  1385,    -1,  1387,    -1,
      -1,  1784,    -1,    -1,  1884,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    20,   167,   168,    -1,   170,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1904,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1913,    -1,  1918,    -1,   191,    -1,    -1,
      -1,  1784,    -1,  1826,    -1,    -1,   200,  1930,    -1,  1932,
    1833,    -1,  1947,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1959,    -1,    -1,    -1,  1951,  1458,
    1953,    -1,    -1,    -1,  1969,    -1,  1337,  1972,  1467,    -1,
      -1,    -1,    -1,  1826,  1473,  1868,    -1,    -1,    -1,    -1,
    1833,    -1,    -1,    -1,  1877,   366,    -1,    -1,    -1,    -1,
      -1,    -1,     6,    -1,   375,    -1,   377,  1890,    -1,    -1,
      -1,   382,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   233,
      -1,    -1,    -1,   394,    -1,  1868,    -1,   695,    -1,    -1,
      -1,  1520,    -1,  1522,    -1,  1524,    -1,    -1,    -1,    -1,
    1529,    -1,    -1,    -1,    48,    -1,  1535,  1890,    -1,    -1,
      -1,  1540,    -1,    -1,    -1,  1544,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   283,    -1,   285,  1950,    -1,   283,
      -1,   285,  1955,  1562,    -1,    -1,  1565,    -1,    -1,  1568,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   755,    -1,  1578,
      -1,    -1,    -1,    -1,    -1,    -1,  1585,  1458,    -1,    -1,
      -1,    -1,    -1,  1592,    -1,  1594,    -1,  1950,   112,    -1,
      -1,  1600,  1955,   117,    -1,   119,   120,   121,   122,   123,
     124,   125,   341,    -1,    -1,   233,    -1,   341,   796,    -1,
     798,    -1,    -1,    -1,  1623,    -1,    -1,    -1,    -1,    -1,
      -1,  1630,  1631,  1632,    -1,    -1,    -1,  1636,    -1,    -1,
     521,    -1,   366,    -1,  1643,    -1,   824,    -1,    -1,    -1,
      -1,   375,    -1,   167,   168,    -1,   170,    -1,   382,    -1,
      81,    -1,    -1,    -1,  1535,    -1,    -1,    -1,    -1,  1540,
     394,    -1,    -1,  1544,    -1,    -1,    -1,   191,    -1,    -1,
      -1,   405,   103,    -1,    -1,    -1,   200,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     429,    -1,    -1,   432,    -1,   429,    -1,    -1,   432,    81,
      -1,    -1,    -1,   891,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   907,
     908,   103,    59,    60,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   164,    -1,    -1,   167,   168,   366,   170,
     171,   172,    -1,    -1,  1753,   479,    -1,   375,    -1,  1630,
    1631,  1632,    -1,   941,   382,  1636,    -1,   139,   140,   141,
     142,   143,  1643,    -1,   195,  1774,   394,    -1,   199,  1778,
    1779,    -1,    -1,    -1,    -1,  1784,    -1,    -1,    -1,   161,
      -1,    -1,   164,    -1,  1793,   167,   168,   521,   170,   171,
     172,  1800,  1801,    -1,    -1,  1804,  1805,   134,   135,    -1,
      -1,    -1,    -1,    -1,   695,    -1,    -1,    -1,    -1,  1818,
      -1,    -1,    -1,   195,    -1,    -1,    -1,  1826,   200,    -1,
      -1,    -1,    -1,    -1,  1833,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   573,    -1,   575,    -1,    -1,   573,
      -1,   575,    -1,    -1,   578,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,  1868,
     197,  1049,  1050,    -1,   755,    -1,    -1,  1876,    -1,    -1,
      -1,    31,    -1,   607,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1890,    -1,    -1,    -1,    -1,    -1,  1896,    -1,    -1,
      -1,    -1,    -1,   521,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1090,  1784,    -1,   796,    81,   798,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,   665,   666,   103,    -1,
      -1,   665,   666,   824,    -1,   674,    -1,    -1,    -1,    -1,
     674,  1950,    -1,   103,    -1,  1826,  1955,    -1,    -1,    -1,
     125,   111,  1833,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   695,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,   161,  1868,    -1,   164,
     165,    -1,   167,   168,  1182,   170,   171,   172,    -1,    -1,
     891,   161,    -1,    -1,   164,   165,    -1,   167,   168,  1890,
     170,   171,   172,    -1,    -1,    -1,   907,   908,    -1,    -1,
     195,   755,    -1,    -1,    -1,   185,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,  1227,
      -1,    -1,    31,    -1,    -1,  1233,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   790,    -1,    -1,    -1,
    1248,  1249,   796,    -1,   798,    -1,    -1,   695,    -1,  1950,
      -1,    -1,    -1,   128,  1955,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,
     824,   825,    81,   832,    -1,    -1,    -1,    -1,   832,    -1,
     839,   840,    91,    -1,    81,   839,   840,   841,   842,   843,
     844,   845,   167,   168,   103,   170,   171,   172,    -1,   853,
      -1,    -1,    -1,    -1,    -1,    -1,   103,   755,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   869,    -1,    -1,    -1,    -1,
     195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1337,
     139,   140,   141,   142,   143,    -1,    -1,   891,  1049,  1050,
      -1,    -1,   139,   140,   141,   142,   143,    -1,   796,    -1,
     798,   905,   161,   907,   908,   164,    -1,    -1,   167,   168,
      -1,   170,   171,   172,    -1,   174,    -1,    -1,    -1,    -1,
     167,   168,   931,   170,   171,   172,   824,   931,   932,    -1,
      -1,    -1,    -1,    19,    20,    -1,   195,   941,   947,    -1,
      -1,    -1,    -1,   947,    30,    -1,    -1,    -1,   195,   196,
      -1,   960,    -1,    -1,    -1,    -1,   960,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   968,    -1,    -1,   971,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     989,    -1,    -1,    -1,    -1,   989,    -1,    -1,    -1,   993,
    1448,    -1,    -1,   891,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,  1006,    -1,    -1,    -1,    -1,    -1,    -1,   907,
     908,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,  1182,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,  1049,  1050,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1063,    -1,    -1,    69,  1067,  1063,
    1069,    -1,    -1,  1067,    -1,  1069,    -1,    -1,    -1,    -1,
      -1,    -1,  1233,  1531,    -1,    -1,    -1,    -1,    -1,    -1,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1248,  1249,  1093,
    1094,  1095,  1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1137,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,   233,    81,    -1,
      -1,  1049,  1050,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1170,    -1,  1172,    31,    -1,  1170,    -1,  1172,    -1,
     103,    -1,    -1,    -1,    -1,    -1,  1337,    -1,  1182,    59,
      60,    -1,    -1,    -1,    -1,   197,    -1,    -1,    -1,    -1,
    1199,    -1,   125,  1202,    -1,  1199,    -1,    -1,  1202,    -1,
      -1,    68,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    81,  1219,  1220,  1221,  1222,  1223,
    1224,    -1,    -1,  1227,    -1,  1229,    -1,    -1,   161,  1233,
      -1,   164,   165,    -1,   167,   168,   103,   170,   171,   172,
      -1,    -1,    -1,    -1,  1248,  1249,    -1,  1251,    -1,    -1,
      -1,    -1,  1261,    -1,   134,   135,    -1,  1261,  1267,    -1,
      -1,    -1,   195,  1267,    -1,    -1,  1270,    -1,  1272,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
     366,    -1,    -1,    -1,  1182,    -1,    -1,    -1,    -1,   375,
      -1,    -1,  1296,    -1,   161,    -1,   382,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,   174,   394,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,   405,
      -1,  1330,  1331,    -1,    -1,    -1,  1330,  1331,   195,   196,
    1334,    -1,    -1,  1337,    -1,  1233,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1248,  1249,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1385,    -1,  1387,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,   479,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,  1430,    -1,  1432,    -1,    -1,  1430,    -1,  1432,    -1,
      69,    -1,    -1,    -1,  1438,   521,  1440,    -1,  1442,  1337,
      -1,    -1,    -1,  1447,  1448,    -1,    -1,  1451,    -1,  1453,
      -1,    -1,  1456,    -1,    -1,    -1,    -1,    -1,  1467,    -1,
      -1,    -1,    -1,  1467,  1468,    -1,    -1,  1471,    -1,  1478,
      10,    11,    12,    -1,  1478,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,   578,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,  1520,    57,  1522,    -1,
    1524,   607,    -1,    -1,    -1,  1529,    -1,  1531,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,  1545,    78,    79,    80,
      81,  1545,    83,    84,  1548,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,  1560,  1561,    -1,  1568,
      -1,   200,   103,    -1,  1568,    -1,  1570,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,
      -1,    -1,    -1,  1592,    -1,  1594,    -1,    -1,  1592,   130,
    1594,  1600,    -1,    -1,    -1,    -1,  1600,    -1,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,   695,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,  1623,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,   176,  1640,  1641,  1642,    -1,
      -1,    -1,    -1,  1647,    -1,  1649,  1655,    -1,    -1,   190,
      -1,  1655,    -1,  1657,   195,   196,    -1,    -1,    -1,    -1,
     201,    -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,   755,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,   790,    -1,    -1,    -1,    -1,    -1,
     796,    -1,   798,    -1,    -1,    59,    60,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,   824,   825,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,  1757,    -1,
      -1,    -1,    -1,  1757,    -1,   841,   842,   843,   844,   845,
      -1,    -1,    -1,    -1,    -1,  1774,    -1,   853,    -1,    68,
    1774,    -1,    -1,    -1,  1778,  1779,    -1,    -1,    -1,    -1,
      -1,    -1,    81,   869,  1793,    -1,    -1,    -1,    -1,  1793,
     134,   135,    -1,    -1,    -1,  1799,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,   891,  1810,    -1,    -1,    -1,
      -1,    -1,  1816,    -1,    -1,    -1,  1820,    -1,    -1,   905,
      -1,   907,   908,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,  1842,    -1,    -1,    -1,    -1,  1842,   138,
     139,   140,   141,   142,   143,   144,   932,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   941,    -1,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,   174,    -1,    -1,  1882,    81,
      -1,    -1,   968,    -1,    -1,   971,   185,    -1,  1892,    91,
      -1,    -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,
      -1,   103,    -1,    -1,   233,  1909,    -1,   993,    -1,  1918,
      -1,    -1,    -1,    -1,  1918,    -1,    -1,    -1,    -1,    -1,
    1006,  1930,    -1,  1932,    -1,    -1,  1930,    -1,  1932,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,  1951,    -1,  1953,    -1,    -1,  1951,    -1,  1953,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,
      -1,    -1,   164,  1049,  1050,   167,   168,    -1,   170,   171,
     172,    -1,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,   103,    -1,  1093,  1094,  1095,
    1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,    -1,    -1,   366,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   375,    -1,    -1,    -1,
      -1,  1137,    -1,   382,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   394,    -1,    -1,    -1,   167,
     168,    -1,   170,   171,   172,    -1,   405,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      79,    80,    -1,    -1,    -1,    -1,  1182,   195,    -1,    -1,
      30,    31,    91,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    31,  1219,  1220,  1221,  1222,  1223,  1224,    69,
      -1,  1227,    -1,  1229,    -1,    -1,    -1,  1233,    -1,    -1,
     479,    -1,    -1,    -1,    -1,    -1,   145,   146,   147,   148,
     149,    -1,  1248,  1249,    -1,  1251,    -1,   156,    -1,    68,
      -1,    -1,    -1,   162,   163,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,  1270,    -1,  1272,   176,    10,    11,
      12,    -1,   521,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   190,    -1,    -1,   103,    -1,    -1,    -1,    30,    31,
    1296,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,    69,  1334,   578,
      -1,  1337,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,
     200,   170,   171,   172,    -1,    -1,    -1,    -1,   607,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   185,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,   195,   196,    -1,  1385,
      -1,  1387,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     578,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,   607,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,  1438,    -1,  1440,    -1,  1442,    -1,    -1,    -1,
      -1,  1447,  1448,    -1,    -1,  1451,   695,  1453,    -1,    81,
    1456,    83,    84,    -1,    10,    11,    12,    -1,   200,    -1,
      -1,    -1,  1468,    -1,    -1,  1471,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,   755,   139,   140,   141,
     142,   143,    81,    69,  1520,    -1,  1522,    -1,  1524,    -1,
      -1,    -1,    -1,  1529,    81,  1531,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   165,   103,   167,   168,    -1,   170,   171,
     172,    -1,  1548,    -1,    -1,    -1,   103,   796,    -1,   798,
      -1,    -1,    -1,    -1,  1560,  1561,    -1,    -1,   200,    -1,
      -1,    -1,    -1,   195,  1570,    -1,    -1,   199,    -1,   201,
     139,   140,   141,   142,   143,   824,   825,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      -1,    -1,   841,   842,   843,   844,   845,    -1,   167,   168,
      -1,   170,   171,   172,   853,    -1,    -1,    -1,   165,    -1,
     167,   168,   169,   170,   171,   172,    -1,  1623,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,
      -1,    -1,    -1,    -1,  1640,  1641,  1642,   825,   195,   196,
      -1,  1647,   891,  1649,   200,    -1,    10,    11,    12,    -1,
      -1,  1657,    -1,   841,   842,   843,   844,   845,   907,   908,
      -1,    -1,    -1,    -1,    -1,   853,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   941,    57,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   968,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,   993,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1778,  1779,    -1,    -1,    -1,    -1,    -1,    -1,
     968,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1799,    -1,    -1,    -1,    -1,    -1,    -1,
    1049,  1050,    -1,    -1,  1810,    -1,    -1,    -1,    -1,    -1,
    1816,    -1,    -1,    -1,  1820,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   198,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,    -1,    -1,  1093,  1094,  1095,  1096,  1097,  1098,
    1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,    -1,    -1,    -1,    -1,  1882,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,  1892,    -1,  1137,    -1,
      -1,    -1,    91,    -1,    -1,    -1,  1084,  1085,    -1,    -1,
    1088,    -1,    31,  1909,   103,  1093,  1094,  1095,  1096,  1097,
    1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1182,    -1,    -1,    -1,    -1,    -1,    68,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,  1137,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    -1,   164,    -1,    -1,   167,   168,
      -1,   170,   171,   172,   103,    -1,    -1,    -1,  1227,    -1,
      -1,    -1,   111,    -1,  1233,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,   195,    -1,    -1,  1248,
    1249,    -1,  1251,    -1,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,  1270,    -1,  1272,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,     3,     4,  1296,     6,     7,
      -1,    -1,    10,    11,    12,    13,   185,    -1,    -1,    -1,
      -1,    -1,   191,  1251,    -1,    -1,   195,   196,    -1,    -1,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1270,    -1,  1272,    -1,    -1,    -1,  1337,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1296,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,    -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,  1438,
      -1,  1440,    -1,  1442,    -1,    -1,    -1,    -1,  1447,  1448,
      -1,    -1,  1451,   161,  1453,    -1,    -1,  1456,    -1,   167,
     168,    -1,   170,   171,   172,   173,    -1,   175,    -1,    -1,
     178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,
      -1,   199,    -1,   201,    -1,    -1,    -1,    -1,    -1,    -1,
    1438,    -1,  1440,    -1,  1442,    -1,    -1,    -1,    -1,  1447,
      -1,    -1,    -1,  1451,    -1,  1453,    -1,    -1,  1456,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,  1531,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,  1548,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,     3,     4,     5,     6,     7,
    1548,    -1,    -1,    -1,    -1,    13,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      38,  1640,  1641,  1642,    -1,    -1,    -1,    -1,  1647,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,  1656,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1640,  1641,  1642,   103,    -1,    -1,    -1,  1647,
      -1,    -1,    -1,   111,    -1,    -1,    -1,   200,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
    1799,    -1,    -1,   201,   202,    -1,   204,   205,    -1,    -1,
      -1,  1810,    -1,    -1,    -1,    -1,    -1,  1816,    -1,    -1,
      -1,  1820,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,  1844,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,  1799,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1810,    -1,    -1,    -1,    -1,    48,  1816,    50,
      51,    -1,  1820,  1882,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    -1,    96,    -1,    98,    -1,   100,
      -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,   114,  1882,   116,   117,   118,   119,   120,
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
       3,     4,     5,     6,     7,    -1,    -1,    -1,    31,    -1,
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
       7,    -1,    -1,    -1,    -1,    -1,    13,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    50,    51,    -1,    -1,    -1,    -1,    56,
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
      -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,   200,
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
     176,    -1,    -1,   179,    -1,    -1,    -1,    -1,   200,   185,
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
      -1,   179,    -1,    -1,    -1,    -1,   200,   185,    -1,    -1,
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
     142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    83,    -1,    85,    -1,    -1,    -1,    -1,   161,
      -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,   173,   103,   175,    -1,    -1,   178,     3,     4,    -1,
       6,     7,    -1,   185,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,   195,   196,    -1,    -1,    -1,   200,    -1,
      -1,    -1,    28,    29,    -1,    31,    -1,    -1,   139,   140,
     141,   142,   143,    -1,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    57,    57,    -1,    -1,    -1,   167,   168,    -1,   170,
     171,   172,    68,    -1,    69,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   195,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,    -1,    -1,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,   173,   103,   175,
      -1,    -1,   178,     3,     4,    -1,     6,     7,    -1,   185,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   195,
     196,    -1,    -1,    -1,   200,    -1,    -1,    -1,    28,    29,
      -1,    31,    -1,    -1,   139,   140,   141,   142,   143,    -1,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    57,    -1,    -1,
     165,    -1,   167,   168,    -1,   170,   171,   172,    68,    -1,
      69,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     195,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,    -1,
     130,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,   173,   103,   175,    -1,    -1,   178,     3,
       4,    -1,     6,     7,    -1,   185,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,    -1,
     139,   140,   141,   142,   143,    -1,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    57,    -1,   164,    -1,    -1,   167,   168,
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
      57,   164,    -1,    -1,   167,   168,    -1,   170,   171,   172,
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
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    83,    84,    -1,    -1,
     161,   162,   163,    -1,    -1,    -1,   167,   168,    -1,   170,
     171,   172,   173,    -1,   175,   176,   103,   178,    -1,    -1,
      -1,    -1,    -1,    -1,   185,   186,    -1,   188,    -1,   190,
     191,    -1,     3,     4,   195,     6,     7,    -1,    -1,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   165,    -1,
     167,   168,    -1,   170,   171,   172,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,   195,    -1,
      81,    -1,   199,    -1,   201,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,    -1,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
     161,    10,    11,    12,    13,    -1,   167,   168,    -1,   170,
     171,   172,   173,    -1,   175,    -1,    -1,   178,    -1,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   198,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   198,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   198,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,   198,    -1,    -1,    -1,    56,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   197,    -1,    -1,    -1,   103,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,    69,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    38,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,   197,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    70,   190,    -1,    -1,    -1,    -1,   195,   196,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,   136,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    50,    51,   176,    -1,    -1,
      -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   190,    -1,    -1,    -1,    70,   195,   196,    -1,    -1,
      -1,    -1,   201,    78,    79,    80,    81,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,    69,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    50,    51,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    70,    -1,
      -1,   176,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
     185,    -1,    -1,    -1,    -1,   190,    -1,    -1,    -1,    91,
     195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,   136,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,   119,   161,
      -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   185,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,   195,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    70,    -1,    -1,   176,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,   190,
      -1,    -1,    -1,    91,   195,   196,    -1,    -1,   199,    -1,
     201,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    70,    71,    -1,   176,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,   190,    -1,    -1,    -1,    91,   195,   196,    -1,
      -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    70,    -1,
      -1,   176,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,   190,    -1,    -1,    -1,    91,
     195,   196,    -1,    -1,    -1,    -1,   201,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    70,    -1,    -1,   176,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,   190,    -1,
      -1,    -1,    91,   195,   196,    -1,    -1,    -1,    -1,   201,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   190,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,
      30,    31,   201,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     136,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,   136,    -1,
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
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,   136,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    69,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,   156,   136,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,   190,    91,    81,    -1,    -1,   195,   196,    -1,    -1,
      -1,    -1,   201,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
     119,    -1,    -1,   111,   112,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,   139,   140,   141,   142,   143,    81,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,   164,   176,   103,   167,
     168,    -1,   170,   171,   172,    -1,   111,   112,    -1,    -1,
      -1,   190,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,
      -1,    -1,   201,    -1,    -1,    -1,    -1,   195,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   164,
      -1,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     195,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    32,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    12,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    69,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    69,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69
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
     198,   343,   345,   348,   199,   239,   348,   111,   112,   164,
     216,   217,   218,   219,   223,    83,   201,   293,   294,    83,
     295,   121,   130,   120,   130,   196,   196,   196,   196,   213,
     265,   485,   196,   196,    70,    70,    70,    70,    70,   338,
      83,    90,   157,   158,   159,   474,   475,   164,   199,   223,
     223,   213,   266,   485,   165,   196,   485,   485,    83,   192,
     199,   359,    28,   336,   340,   348,   349,   453,   457,   228,
     199,    90,   414,   474,    90,   474,   474,    32,   164,   181,
     486,   196,     9,   198,    38,   245,   165,   264,   485,   119,
     191,   246,   328,   198,   198,   198,   198,   198,   198,   198,
     198,    10,    11,    12,    30,    31,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    57,
      69,   198,    70,    70,   199,   160,   131,   171,   173,   186,
     188,   267,   326,   327,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    59,    60,   134,
     135,   443,    70,   199,   448,   196,   196,    70,   199,   201,
     461,   196,   245,   246,    14,   348,   198,   136,    49,   213,
     438,    90,   336,   349,   160,   453,   136,   203,     9,   423,
     260,   336,   349,   453,   486,   160,   196,   415,   443,   448,
     197,   348,    32,   230,     8,   360,     9,   198,   230,   231,
     338,   339,   348,   213,   279,   234,   198,   198,   198,   138,
     144,   506,   506,   181,   505,   196,   111,   506,    14,   160,
     138,   144,   161,   213,   215,   198,   198,   198,   240,   115,
     178,   198,   216,   218,   216,   218,   223,   199,     9,   424,
     198,   102,   164,   199,   453,     9,   198,    14,     9,   198,
     130,   130,   453,   478,   338,   336,   349,   453,   456,   457,
     197,   181,   257,   137,   453,   467,   468,   348,   368,   369,
     338,   389,   389,   368,   389,   198,    70,   443,   157,   475,
      82,   348,   453,    90,   157,   475,   223,   212,   198,   199,
     252,   262,   398,   400,    91,   196,   201,   361,   362,   364,
     407,   411,   459,   461,   479,    14,   102,   480,   355,   356,
     357,   289,   290,   441,   442,   197,   197,   197,   197,   197,
     200,   229,   230,   247,   254,   261,   441,   348,   202,   204,
     205,   213,   487,   488,   506,    38,   174,   291,   292,   348,
     482,   196,   485,   255,   245,   348,   348,   348,   348,    32,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   412,   348,   348,   463,   463,   348,
     470,   471,   130,   199,   214,   215,   460,   461,   265,   213,
     266,   485,   485,   264,   246,    38,   340,   343,   345,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   165,   199,   213,   444,   445,   446,   447,   460,
     463,   348,   291,   291,   463,   348,   467,   245,   197,   348,
     196,   437,     9,   423,   197,   197,    38,   348,    38,   348,
     415,   197,   197,   197,   460,   291,   199,   213,   444,   445,
     460,   197,   228,   283,   199,   345,   348,   348,    94,    32,
     230,   277,   198,    27,   102,    14,     9,   197,    32,   199,
     280,   506,    31,    91,   174,   226,   499,   500,   501,   196,
       9,    50,    51,    56,    58,    70,   138,   139,   140,   141,
     142,   143,   185,   196,   224,   375,   378,   381,   384,   387,
     393,   408,   416,   417,   419,   420,   213,   504,   228,   196,
     238,   199,   198,   199,   198,   102,   164,   111,   112,   164,
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
      69,   160,   486,   200,   408,   199,   242,   218,   218,   213,
     219,   219,   223,     9,   424,   200,   200,    14,   453,   198,
     182,     9,   423,   213,   271,   408,   199,   467,   137,   453,
      14,   348,   348,   203,   348,   200,   209,   506,   271,   199,
     401,    14,   197,   348,   361,   460,   198,   506,   193,   200,
      32,   493,   442,    38,    83,   174,   444,   445,   447,   444,
     445,   506,    38,   174,   348,   417,   289,   196,   408,   269,
     353,   249,   348,   348,   348,   200,   196,   291,   270,    32,
     269,   506,    14,   268,   485,   412,   200,   196,    14,    78,
      79,    80,   213,   427,   427,   429,   431,   432,    52,   196,
      70,    70,    70,    70,    70,    90,   157,   196,   160,     9,
     423,   197,   437,    38,   348,   269,   200,    75,    76,   286,
     337,   230,   200,   198,    95,   198,   274,   453,   196,   136,
     273,    14,   228,   281,   105,   106,   107,   281,   200,   506,
     182,   136,   160,   506,   213,   174,   499,     9,   197,   423,
     136,   203,     9,   423,   422,   370,   371,   417,   390,   417,
     418,   390,   370,   390,   361,   363,   365,   197,   130,   214,
     417,   472,   473,   417,   417,   417,    32,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   504,    83,   243,   200,   200,   222,   198,   417,   498,
     102,   103,   494,   496,     9,   299,   197,   196,   340,   345,
     348,   136,   203,   200,   480,   299,   166,   179,   199,   397,
     404,   166,   199,   403,   136,   198,   493,   506,   360,   507,
      83,   174,    14,    83,   486,   453,   348,   197,   289,   199,
     289,   196,   136,   196,   291,   197,   199,   506,   199,   198,
     506,   269,   250,   415,   291,   136,   203,     9,   423,   428,
     431,   372,   373,   429,   391,   429,   430,   391,   372,   391,
     157,   361,   434,   435,    81,   429,   453,   199,   337,    32,
      77,   230,   198,   339,   273,   467,   274,   197,   417,   101,
     105,   198,   348,    32,   198,   282,   200,   182,   506,   213,
     136,   174,    32,   197,   417,   417,   197,   203,     9,   423,
     136,   203,     9,   423,   203,   203,   203,   136,     9,   423,
     197,   136,   200,     9,   423,   417,    32,   197,   228,   198,
     198,   213,   506,   506,   494,   408,     6,   112,   117,   120,
     125,   167,   168,   170,   200,   300,   325,   326,   327,   332,
     333,   334,   335,   441,   467,   348,   200,   199,   200,    54,
     348,   348,   348,   360,    38,    83,   174,    14,    83,   348,
     196,   493,   197,   299,   197,   289,   348,   291,   197,   299,
     480,   299,   198,   199,   196,   197,   429,   429,   197,   203,
       9,   423,   136,   203,     9,   423,   203,   203,   203,   136,
     197,     9,   423,   299,    32,   228,   198,   197,   197,   197,
     235,   198,   198,   282,   228,   136,   506,   506,   136,   417,
     417,   417,   417,   361,   417,   417,   417,   199,   200,   496,
     132,   133,   186,   214,   483,   506,   272,   408,   112,   335,
      31,   125,   138,   144,   165,   171,   309,   310,   311,   312,
     408,   169,   317,   318,   128,   196,   213,   319,   320,   301,
     246,   506,     9,   198,     9,   198,   198,   480,   326,   197,
     296,   165,   399,   200,   200,    83,   174,    14,    83,   348,
     291,   117,   350,   493,   200,   493,   197,   197,   200,   199,
     200,   299,   289,   136,   429,   429,   429,   429,   361,   200,
     228,   233,   236,    32,   230,   276,   228,   506,   197,   417,
     136,   136,   136,   228,   408,   408,   485,    14,   214,     9,
     198,   199,   483,   480,   312,   181,   199,     9,   198,     3,
       4,     5,     6,     7,    10,    11,    12,    13,    27,    28,
      29,    57,    71,    72,    73,    74,    75,    76,    77,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     137,   138,   145,   146,   147,   148,   149,   161,   162,   163,
     173,   175,   176,   178,   185,   186,   188,   190,   191,   213,
     405,   406,     9,   198,   165,   169,   213,   320,   321,   322,
     198,    83,   331,   245,   302,   483,   483,    14,   246,   200,
     297,   298,   483,    14,    83,   348,   197,   196,   493,   198,
     199,   323,   350,   493,   296,   200,   197,   429,   136,   136,
      32,   230,   275,   276,   228,   417,   417,   417,   200,   198,
     198,   417,   408,   305,   506,   313,   314,   416,   310,    14,
      32,    51,   315,   318,     9,    36,   197,    31,    50,    53,
      14,     9,   198,   215,   484,   331,    14,   506,   245,   198,
      14,   348,    38,    83,   396,   199,   228,   493,   323,   200,
     493,   429,   429,   228,    99,   241,   200,   213,   226,   306,
     307,   308,     9,   423,     9,   423,   200,   417,   406,   406,
      68,   316,   321,   321,    31,    50,    53,   417,    83,   181,
     196,   198,   417,   485,   417,    83,     9,   424,   228,   200,
     199,   323,    97,   198,   115,   237,   160,   102,   506,   182,
     416,   172,    14,   495,   303,   196,    38,    83,   197,   200,
     228,   198,   196,   178,   244,   213,   326,   327,   182,   417,
     182,   287,   288,   442,   304,    83,   200,   408,   242,   175,
     213,   198,   197,     9,   424,   122,   123,   124,   329,   330,
     287,    83,   272,   198,   493,   442,   507,   197,   197,   198,
     195,   490,   329,    38,    83,   174,   493,   199,   491,   492,
     506,   198,   199,   324,   507,    83,   174,    14,    83,   490,
     228,     9,   424,    14,   494,   228,    38,    83,   174,    14,
      83,   348,   324,   200,   492,   506,   200,    83,   174,    14,
      83,   348,    14,    83,   348,   348
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   206,   208,   207,   209,   209,   210,   210,   210,   210,
     210,   210,   210,   210,   211,   210,   212,   210,   210,   210,
     210,   210,   210,   210,   210,   213,   213,   213,   213,   213,
     213,   213,   213,   213,   213,   213,   213,   213,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   215,   215,   216,   216,   217,   217,   218,   219,
     219,   219,   219,   220,   220,   221,   222,   222,   222,   223,
     223,   224,   224,   224,   225,   226,   227,   227,   228,   228,
     229,   229,   229,   229,   230,   230,   230,   231,   230,   232,
     230,   233,   230,   234,   230,   230,   230,   230,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     235,   230,   236,   230,   230,   237,   230,   238,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   230,   230,   240,   239,   241,   241,   243,   242,   244,
     244,   245,   245,   246,   248,   247,   249,   247,   250,   247,
     252,   251,   253,   251,   255,   254,   256,   254,   257,   254,
     258,   254,   260,   259,   262,   261,   263,   261,   264,   264,
     265,   266,   267,   267,   267,   267,   267,   268,   268,   269,
     269,   270,   270,   271,   271,   272,   272,   273,   273,   274,
     274,   274,   275,   275,   276,   276,   277,   277,   278,   278,
     279,   279,   280,   280,   280,   280,   281,   281,   281,   282,
     282,   283,   283,   284,   284,   285,   285,   286,   286,   287,
     287,   287,   287,   287,   287,   287,   287,   288,   288,   288,
     288,   288,   288,   288,   288,   289,   289,   289,   289,   289,
     289,   289,   289,   290,   290,   290,   290,   290,   290,   290,
     290,   291,   291,   292,   292,   292,   292,   292,   292,   293,
     293,   294,   294,   294,   295,   295,   295,   295,   296,   296,
     297,   298,   299,   299,   301,   300,   302,   300,   300,   300,
     300,   303,   300,   304,   300,   300,   300,   300,   300,   300,
     300,   300,   305,   305,   305,   306,   307,   307,   308,   308,
     309,   309,   310,   310,   311,   311,   312,   312,   312,   312,
     312,   312,   312,   313,   313,   314,   315,   315,   316,   316,
     317,   317,   318,   319,   319,   319,   320,   320,   320,   320,
     321,   321,   321,   321,   321,   321,   321,   322,   322,   322,
     323,   323,   324,   324,   325,   325,   326,   326,   327,   327,
     328,   328,   328,   328,   328,   328,   328,   329,   329,   330,
     330,   330,   331,   331,   331,   331,   332,   332,   333,   333,
     334,   334,   335,   336,   336,   336,   336,   336,   336,   337,
     338,   338,   339,   339,   340,   340,   340,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   348,   348,   348,   348,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   350,   350,   352,
     351,   353,   351,   355,   354,   356,   354,   357,   354,   358,
     354,   359,   354,   360,   360,   360,   361,   361,   362,   362,
     363,   363,   364,   364,   365,   365,   366,   367,   367,   368,
     368,   369,   369,   370,   370,   371,   371,   372,   372,   373,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,   389,   389,   390,   390,
     391,   391,   392,   393,   394,   394,   395,   395,   395,   395,
     395,   395,   395,   395,   395,   395,   395,   396,   396,   396,
     396,   397,   398,   398,   399,   399,   400,   400,   401,   401,
     402,   403,   403,   404,   404,   404,   405,   405,   405,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     407,   408,   408,   409,   409,   409,   409,   409,   410,   410,
     411,   411,   411,   411,   412,   412,   412,   413,   413,   413,
     414,   414,   414,   415,   415,   416,   416,   416,   416,   416,
     416,   416,   416,   416,   416,   416,   416,   416,   416,   416,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   418,   418,   419,   420,   420,
     421,   421,   421,   421,   421,   421,   421,   422,   422,   423,
     423,   424,   424,   425,   425,   425,   425,   426,   426,   426,
     426,   426,   427,   427,   427,   427,   428,   428,   429,   429,
     429,   429,   429,   429,   429,   429,   429,   429,   429,   429,
     429,   429,   429,   430,   430,   431,   431,   432,   432,   432,
     432,   433,   433,   434,   434,   435,   435,   436,   436,   437,
     437,   438,   438,   440,   439,   441,   442,   442,   443,   443,
     444,   444,   444,   445,   445,   446,   446,   447,   447,   448,
     448,   449,   449,   450,   450,   451,   451,   452,   452,   453,
     453,   453,   453,   453,   453,   453,   453,   453,   453,   453,
     454,   454,   454,   454,   454,   454,   454,   454,   455,   455,
     455,   455,   455,   455,   455,   455,   455,   456,   457,   457,
     458,   458,   459,   459,   459,   460,   460,   461,   461,   461,
     462,   462,   462,   463,   463,   464,   464,   465,   465,   465,
     465,   465,   465,   466,   466,   466,   466,   466,   467,   467,
     467,   467,   467,   467,   468,   468,   469,   469,   469,   469,
     469,   469,   469,   469,   470,   470,   471,   471,   471,   471,
     472,   472,   473,   473,   473,   473,   474,   474,   474,   474,
     475,   475,   475,   475,   475,   475,   476,   476,   476,   477,
     477,   477,   477,   477,   477,   477,   477,   477,   477,   477,
     478,   478,   479,   479,   480,   480,   481,   481,   481,   481,
     482,   482,   483,   483,   484,   484,   485,   485,   486,   486,
     487,   487,   488,   489,   489,   489,   489,   490,   490,   491,
     491,   492,   492,   493,   493,   494,   494,   495,   496,   496,
     497,   497,   497,   497,   498,   498,   498,   499,   499,   499,
     499,   500,   500,   501,   501,   501,   501,   502,   503,   504,
     504,   505,   505,   506,   506,   506,   506,   506,   506,   506,
     506,   506,   506,   506,   507,   507
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     1,     4,     3,     0,     6,     0,     5,     3,     4,
       4,     6,     7,     7,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     3,     3,     1,     2,     1,
       2,     3,     4,     3,     1,     2,     1,     2,     2,     1,
       3,     1,     3,     2,     2,     2,     5,     4,     2,     0,
       1,     1,     1,     1,     3,     5,     8,     0,     4,     0,
       6,     0,    10,     0,     4,     2,     3,     2,     3,     2,
       3,     3,     3,     3,     3,     3,     5,     1,     1,     1,
       0,     9,     0,    10,     5,     0,    13,     0,     5,     3,
       3,     2,     2,     2,     2,     2,     2,     3,     2,     2,
       3,     2,     2,     0,     4,     9,     0,     0,     4,     2,
       0,     1,     0,     1,     0,     9,     0,    10,     0,    11,
       0,     9,     0,    10,     0,     8,     0,     9,     0,     7,
       0,     8,     0,     8,     0,     7,     0,     8,     1,     1,
       1,     1,     1,     2,     3,     3,     2,     2,     0,     2,
       0,     2,     0,     1,     3,     1,     3,     2,     0,     1,
       2,     4,     1,     4,     1,     4,     1,     4,     1,     4,
       3,     5,     3,     4,     4,     5,     5,     4,     0,     1,
       1,     4,     0,     5,     0,     2,     0,     3,     0,     7,
       8,     6,     2,     5,     6,     4,     0,     4,     5,     7,
       6,     6,     7,     9,     8,     6,     7,     5,     2,     4,
       5,     3,     0,     3,     4,     6,     5,     5,     6,     8,
       7,     2,     0,     1,     2,     2,     3,     4,     4,     3,
       1,     1,     2,     4,     3,     5,     1,     3,     2,     0,
       2,     3,     2,     0,     0,     4,     0,     5,     2,     2,
       2,     0,    11,     0,    12,     3,     3,     3,     4,     4,
       3,     5,     2,     2,     0,     6,     5,     4,     3,     1,
       1,     3,     4,     1,     2,     1,     1,     5,     6,     1,
       1,     4,     1,     1,     3,     2,     2,     0,     2,     0,
       1,     3,     1,     1,     1,     1,     3,     4,     4,     4,
       1,     1,     2,     2,     2,     3,     3,     1,     1,     1,
       1,     3,     1,     3,     1,     1,     1,     0,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       1,     1,     3,     5,     1,     3,     5,     4,     3,     3,
       3,     4,     3,     3,     3,     2,     2,     1,     1,     3,
       3,     1,     1,     0,     1,     2,     4,     3,     3,     6,
       2,     3,     2,     3,     6,     1,     1,     1,     1,     1,
       6,     3,     4,     6,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     4,     3,     1,     2,     2,     2,
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
       3,     2,     4,     2,     4,     5,     5,     5,     5,     1,
       1,     1,     1,     1,     1,     3,     3,     4,     4,     3,
       1,     1,     1,     1,     3,     1,     4,     3,     1,     1,
       1,     1,     1,     3,     3,     4,     4,     3,     1,     1,
       7,     9,     7,     6,     8,     1,     2,     4,     4,     1,
       1,     1,     4,     1,     0,     1,     2,     1,     1,     1,
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
#line 6864 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 754 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 6872 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 761 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 6878 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 762 "hphp.y" /* yacc.c:1646  */
    { }
#line 6884 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 765 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 6890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 766 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6896 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 767 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6902 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 768 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6908 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 769 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6914 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 770 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 6920 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 771 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 6928 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 6935 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 6941 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6947 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 6953 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 779 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6959 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6966 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 783 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6973 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 786 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6980 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 789 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6988 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 793 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6996 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 797 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7004 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 800 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7011 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 805 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7017 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 806 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7023 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 807 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7029 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 808 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7035 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 809 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7041 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 810 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7047 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 811 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7053 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 812 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7059 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 813 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7065 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 814 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7071 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 815 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7077 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 816 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7083 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 817 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7089 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 896 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7095 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 898 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7101 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 903 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7107 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 904 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7114 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 910 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7120 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 914 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7126 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 915 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7132 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 917 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7138 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 919 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7144 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 924 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7150 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 925 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7157 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 931 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7163 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 935 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClass);}
#line 7170 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 937 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7177 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 939 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7184 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 944 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7190 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 946 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7196 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 949 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7202 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 951 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7208 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 952 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7214 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 957 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7223 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 964 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7232 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 972 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7239 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 975 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7246 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 981 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7252 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 982 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7258 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 985 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7264 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 986 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7270 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 987 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7276 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 988 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7282 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 991 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7288 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 995 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7294 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1000 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7300 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1001 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7307 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1003 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7315 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1007 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7322 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1010 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7330 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1014 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7337 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1016 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7345 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1019 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7352 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1021 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7360 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1024 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7366 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1025 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7372 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1026 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7378 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1027 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7384 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1028 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7390 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1029 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7396 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1030 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7402 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1031 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7408 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1032 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7414 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1033 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7420 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1034 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7426 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1035 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7432 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1036 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7438 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1037 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7444 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7451 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1042 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7458 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1044 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7466 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1049 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7473 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1051 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 7481 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 7489 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1064 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7495 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1065 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 7501 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1068 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7507 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1069 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 7513 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1070 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 7519 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1071 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 7528 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1075 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7534 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7540 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1077 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7546 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1078 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7552 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1079 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7558 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1080 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7564 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1081 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7570 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1082 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7576 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7582 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 7588 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1085 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7594 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 7604 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1094 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);}
#line 7610 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1095 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7616 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1104 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 7622 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1105 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7628 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1109 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7635 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1111 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7643 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1117 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7649 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1118 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7655 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1122 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 7661 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1123 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7667 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1127 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 7673 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1133 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7682 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1139 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7691 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1146 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7700 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7709 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1159 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7718 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1165 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7727 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1173 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7734 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1177 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 7740 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1181 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7747 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1185 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 7753 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1191 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7760 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1194 "hphp.y" /* yacc.c:1646  */
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
#line 7778 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1209 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7785 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1212 "hphp.y" /* yacc.c:1646  */
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
#line 7803 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 1226 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7810 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 1229 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7818 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 1234 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7825 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1237 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7833 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1243 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 7839 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1246 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 7845 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1250 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7852 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1253 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7863 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1261 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7870 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1264 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7881 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1272 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7887 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1273 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 7894 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1277 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7900 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1280 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7906 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1283 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 7912 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1284 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 7918 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1285 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 7926 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1288 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 7932 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1289 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 7938 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1293 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7944 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1294 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7950 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1297 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7956 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1298 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7962 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1301 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7968 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1302 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7974 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1305 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 7980 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1307 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 7986 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1310 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 7992 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1312 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 7998 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1316 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8004 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1317 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8010 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1320 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8016 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1321 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8022 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1322 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8028 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1326 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8034 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1328 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8040 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1331 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8046 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1333 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8052 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1336 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8058 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1338 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8064 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1341 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8070 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1343 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8076 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1347 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8082 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1349 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8089 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1354 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8095 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1355 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8101 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1356 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8107 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8113 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1362 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8119 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1364 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8125 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1365 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8131 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1368 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8137 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1369 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8143 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1374 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8149 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1375 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8155 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1380 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8161 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1381 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8167 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1384 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8173 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1385 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8179 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1388 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8185 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1389 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8191 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1397 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8198 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1403 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8205 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1409 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8213 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1413 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8219 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1417 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8226 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1422 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8233 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1427 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8241 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1430 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8247 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1436 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8254 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1440 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8261 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1445 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8268 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1450 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8275 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1455 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8282 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1460 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8289 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8296 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1472 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8303 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1480 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8310 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1485 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8317 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1490 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8325 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1494 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8331 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1497 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8338 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1501 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 8345 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1505 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 8353 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1508 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8359 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1513 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8366 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1516 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8373 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1520 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8380 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1524 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8387 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1528 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8394 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1532 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8401 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1537 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8408 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1542 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8415 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1548 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8421 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1549 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8427 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1552 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,false);}
#line 8433 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1553 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),true,false);}
#line 8439 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1554 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,true);}
#line 8445 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1556 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),false, false);}
#line 8451 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1558 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),false,true);}
#line 8457 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1560 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),true, false);}
#line 8463 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1564 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8469 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1565 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 8475 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1568 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8481 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1569 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8487 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1570 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 8493 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1574 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 8499 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1576 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 8505 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1577 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 8511 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1578 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 8517 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1583 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8523 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1584 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8529 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1587 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8536 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1592 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 8542 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1598 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8548 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1599 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8554 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1602 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 8560 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1603 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 8567 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1606 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 8573 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1607 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 8580 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1609 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8587 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1612 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 8594 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1614 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8600 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1617 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8608 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1624 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8617 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1632 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8625 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1639 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8634 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1644 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 8640 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1646 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8646 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1648 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8652 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1650 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 8658 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1652 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 8664 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1653 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 8671 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1656 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 8677 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1659 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8683 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1660 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8689 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1661 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8695 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1667 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 8701 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1672 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 8708 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1675 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 8716 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1682 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 8722 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1683 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 8729 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1688 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 8736 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1691 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 8742 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1698 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 8749 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1700 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8755 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1704 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8761 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1709 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8767 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1711 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8773 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1713 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8779 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1714 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 8790 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 8796 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1722 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 8802 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 8808 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1727 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,0,(yyvsp[0]),0);}
#line 8814 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1729 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),0,(yyvsp[0]),0);}
#line 8820 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1734 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8826 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1737 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8832 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1738 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 8838 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1742 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 8844 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1743 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 8850 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1747 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 8857 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1750 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 8864 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1755 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 8871 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1760 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 8877 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1761 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 8884 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1763 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 8890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1767 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 8896 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1768 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 8902 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1769 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 8908 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1770 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 8914 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1774 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8920 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1775 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 8926 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1776 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 8932 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1777 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 8938 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1778 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 8944 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1780 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 8950 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1782 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 8956 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1786 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 8964 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1789 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 8970 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1790 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 8976 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1794 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8982 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1795 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 8988 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1799 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8994 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1800 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9000 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1803 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9006 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1804 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9012 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1807 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9018 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1808 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9024 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1811 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9030 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1813 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9036 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1816 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9042 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1817 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9048 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1818 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9054 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1819 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9060 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1820 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9066 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1821 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9072 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1822 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9078 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1826 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9084 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1827 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9090 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1830 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9096 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9102 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9108 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1836 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9114 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1838 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9120 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1839 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9126 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1840 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9132 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1844 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9138 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9144 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1850 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9150 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1852 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9156 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1856 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9164 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1860 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9171 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1864 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9177 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1868 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9183 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1870 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9189 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1871 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9195 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1872 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9201 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1873 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9207 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1874 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9213 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1877 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9219 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1881 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9225 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1882 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9231 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1886 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9237 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1887 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9243 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1891 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9249 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1892 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9255 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1893 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9261 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1894 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9267 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 1898 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9273 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 1903 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9279 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 1907 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 9285 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 1911 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9291 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 1915 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 9297 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 1919 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9303 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 1924 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9309 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 1928 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9315 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 1929 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9321 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 1930 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9327 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 1931 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9333 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 1932 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9339 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 1937 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 9345 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 1938 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 9351 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 1939 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 9357 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 1942 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 9363 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 1943 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 9369 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 1944 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 9375 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 1945 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 9381 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 1946 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 9387 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 1947 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 9393 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 1948 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 9399 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 1949 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 9405 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 9411 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 1951 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 9417 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 1952 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 9423 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 1953 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 9429 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 1954 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 9435 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 1955 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 9441 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 9447 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 9453 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 1958 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 9459 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 1959 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 9465 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 1960 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 9471 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 1961 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 9477 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 9483 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 9489 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 9495 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 9501 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 1966 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 9507 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 1967 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 9513 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 1968 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 9519 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 1969 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 9525 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 1970 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 9531 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 1971 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 9537 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 1972 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 9543 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 9549 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 1974 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 9555 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 1975 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 9561 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 9567 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 1977 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 9573 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 1978 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 9579 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 1979 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 9585 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 1980 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 9591 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 1981 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 9597 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 9603 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 1983 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 9609 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 1984 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 9615 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 1985 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 9621 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 9628 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 1988 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 9634 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 1989 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 9641 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 9647 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 1993 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 9653 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9659 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 1995 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 9665 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 1996 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 9671 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 1997 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 9677 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 1998 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9683 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 9689 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 9695 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2001 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 9701 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2002 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 9707 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 9713 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 9719 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 9725 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2006 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 9731 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 9737 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2008 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9743 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2009 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9749 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2010 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9755 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2011 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9761 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2012 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9767 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9773 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9779 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9785 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2016 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 9791 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 9797 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9803 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2025 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 9809 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2026 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9815 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2031 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9824 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2037 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9835 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2045 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9844 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2051 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9855 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2061 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 9868 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2069 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         (yyvsp[-3]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-3]), nullptr, (yyvsp[-3]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-3]),
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9882 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2079 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9892 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2087 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         (yyvsp[-6]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-6]), nullptr, (yyvsp[-6]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-6]),
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9906 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2097 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9916 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2103 "hphp.y" /* yacc.c:1646  */
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
#line 9932 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2114 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 9945 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2122 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9957 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2129 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9967 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2137 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9979 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2147 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 9985 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 9991 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2150 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9997 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2154 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10004 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2156 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10010 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2163 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10016 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2166 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10022 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2173 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10028 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2176 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10034 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2181 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10040 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2182 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10046 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2187 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10052 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2188 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10058 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2192 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_ARRAY);}
#line 10064 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2196 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10070 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2197 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10076 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2202 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10082 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2203 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10088 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10094 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2209 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10100 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2214 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10106 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2215 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10112 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2221 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10118 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2223 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10124 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2228 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10130 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2229 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10136 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2235 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10142 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2237 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10148 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2241 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10154 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2245 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10160 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2249 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10166 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2253 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10172 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2257 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10178 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2261 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10184 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2265 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10190 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2269 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10196 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2273 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10202 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2277 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10208 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2281 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10214 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2285 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10220 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2289 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10226 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2293 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10232 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2297 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10238 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2302 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10244 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2303 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10250 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2308 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10256 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2309 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10262 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2314 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10268 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2315 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10274 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2320 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10282 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2327 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10290 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2334 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10296 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2336 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10302 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2340 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10308 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2341 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10314 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2342 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10320 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2343 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10326 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2344 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10332 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2345 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10338 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2346 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10344 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2347 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10350 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2348 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10357 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2350 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10363 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2351 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10369 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2355 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10375 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2356 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 10381 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2357 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 10387 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2358 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 10393 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2365 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 10399 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2368 "hphp.y" /* yacc.c:1646  */
    { Token t1; _p->onArray(t1,(yyvsp[-1]));
                                         Token t2; _p->onArray(t2,(yyvsp[0]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[-1]),NULL,t1,0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-1]),t2,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),file,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),line,0,0);
                                         (yyval).setText("");}
#line 10413 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2379 "hphp.y" /* yacc.c:1646  */
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray((yyvsp[-2]),(yyvsp[-5]));
                                         _p->onArray((yyvsp[-1]),(yyvsp[-3]));
                                         _p->onCallParam((yyvsp[-4]),NULL,(yyvsp[-2]),0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-4]),(yyvsp[-1]),0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),file,0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),line,0,0);
                                         (yyval).setText((yyvsp[0]).text());}
#line 10427 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2390 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 10433 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2391 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 10439 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2396 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10445 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2397 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10451 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2400 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),0,(yyvsp[0]),0);}
#line 10457 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2401 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10463 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2404 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10470 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2408 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10478 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2411 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10484 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2414 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 10496 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2421 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10502 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2422 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10508 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2426 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10514 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2428 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 10520 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2430 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 10526 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2434 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10532 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2435 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10538 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2436 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10544 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2437 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10550 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2438 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10556 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2439 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10562 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2440 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10568 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2441 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10574 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2442 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10580 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2443 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10586 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2444 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10592 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2445 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10598 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2446 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10604 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2447 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10610 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2448 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10616 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10622 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2450 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10628 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2451 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10634 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2452 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10640 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10646 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10652 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2455 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10658 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2456 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10664 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2457 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10670 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2458 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10676 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10682 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2460 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10688 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10694 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2462 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10700 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2463 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10706 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10712 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2465 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10718 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2466 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10724 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2467 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10730 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2468 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10736 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2469 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10742 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2470 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10748 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2471 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10754 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2472 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10760 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2473 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10766 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2474 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10772 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2475 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10778 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2476 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10784 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2477 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10790 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2478 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10796 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2479 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10802 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2480 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10808 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2481 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10814 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2482 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10820 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10826 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2484 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10832 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10838 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10844 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10850 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10856 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10862 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10868 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2491 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10874 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2492 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10880 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2493 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10886 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2494 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10892 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2495 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10898 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2496 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10904 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2497 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10910 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2498 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10916 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2499 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10922 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2500 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10928 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2501 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10934 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10940 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2503 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10946 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2504 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10952 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2505 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10958 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2506 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10964 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2507 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10970 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2508 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10976 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2509 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10982 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2510 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10988 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2511 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10994 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2512 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11000 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2513 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11006 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2514 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11012 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2519 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11018 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2523 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11024 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2524 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11030 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2528 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11036 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2529 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11042 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2530 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11048 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2531 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11055 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2533 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11062 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2537 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11068 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2546 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11074 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2549 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11080 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2550 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11087 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2552 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11094 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2562 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11100 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2566 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11106 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2567 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11112 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2568 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11118 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2572 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11124 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11130 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2574 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11136 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2578 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11142 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2579 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11148 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2580 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11154 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2584 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11160 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2585 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11166 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2589 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11172 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2590 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11178 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2591 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11184 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2592 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11191 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2594 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11197 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2595 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11203 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2596 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 11209 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2597 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 11215 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2598 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 11221 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2599 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 11227 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2600 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 11233 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2601 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 11239 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2602 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 11245 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2605 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11251 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2607 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11257 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2611 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11263 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2612 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11269 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2614 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11275 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2615 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11281 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2617 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11287 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2618 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11293 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2619 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11299 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2620 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11305 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2621 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11311 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2622 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11317 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2623 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11323 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2624 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11329 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2625 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11335 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2627 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 11341 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2629 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 11347 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2631 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 11353 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2633 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 11359 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2635 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 11365 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 11371 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2637 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 11377 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2638 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 11383 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 11389 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2640 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 11395 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2641 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 11401 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 11407 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2643 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 11413 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2644 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 11419 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2645 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 11425 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 11431 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2647 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 11437 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2648 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 11443 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2649 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 11449 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11455 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2651 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11461 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2653 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 11467 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2655 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 11473 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2657 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 11479 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 11485 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2660 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 11491 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2662 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 11498 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 11504 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2667 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 11511 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2671 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 11517 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2674 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 11523 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2675 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 11529 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2679 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11535 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2680 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11541 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11547 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11553 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2693 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11559 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2697 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11565 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2698 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11571 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2699 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11577 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2700 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11583 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2701 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 11589 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2702 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 11595 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2704 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 11602 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2709 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11608 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2710 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11614 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2714 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11620 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2715 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11626 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 11632 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11638 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11644 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2727 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11650 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11656 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11662 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11668 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2735 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11674 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11680 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11686 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11692 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2744 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 11698 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2745 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 11704 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2746 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 11710 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2747 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11716 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2751 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 11723 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2754 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 11731 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2761 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11737 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2762 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11743 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2765 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 11751 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2768 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11757 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2769 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11763 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2770 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11769 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2772 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11775 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2773 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11781 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2775 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11787 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2776 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11793 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2777 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11799 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2778 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11805 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2779 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11811 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2780 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11817 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2785 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11823 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2786 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11829 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2791 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11835 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2792 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11841 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2797 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11847 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2799 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11853 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2801 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11859 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2802 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11865 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2806 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11871 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2807 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11877 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2812 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11883 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2813 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 11889 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2818 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11895 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2821 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11901 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2826 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11907 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2827 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11913 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2830 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11919 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2831 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 11926 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2838 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 11932 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2840 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 11938 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2843 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 11944 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2845 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11950 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2848 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11956 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2851 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11962 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2852 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11968 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2856 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 11974 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2857 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 11980 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2861 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 11986 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2862 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 11992 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2863 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 11998 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12004 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2869 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12010 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2877 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12016 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2878 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12022 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2882 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12028 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2884 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12034 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2892 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12040 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2893 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12046 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2897 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12052 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2899 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12058 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 2904 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12064 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 2906 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12070 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 2912 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12084 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
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
#line 12098 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 2938 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12112 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 2950 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12126 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 2962 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12132 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 2963 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12138 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 2964 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12144 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 2965 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12150 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 2966 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12156 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 2967 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12162 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 2969 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12176 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 2986 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12182 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 2988 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12188 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 2990 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12194 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 2991 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12200 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 2995 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12206 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 2996 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12212 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 2997 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12218 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 2998 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12224 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3006 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12238 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3015 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12244 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12250 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12256 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3027 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12262 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3028 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12268 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3029 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12274 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3030 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12280 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3031 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12286 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3032 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12292 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3033 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12298 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3035 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12304 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3037 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12310 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3041 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12316 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3045 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12322 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3046 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12328 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3052 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12334 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3056 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12340 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3063 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 12346 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3072 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 12352 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3076 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 12358 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3080 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12364 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3083 "hphp.y" /* yacc.c:1646  */
    { _p->onIndirectRef((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 12370 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3089 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12376 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3090 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12382 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3091 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12388 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3095 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12394 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3096 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 12400 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3097 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 12406 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3104 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12412 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3105 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12418 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3110 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 12424 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3111 "hphp.y" /* yacc.c:1646  */
    { (yyval)++;}
#line 12430 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3116 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12436 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3117 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12442 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3118 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12448 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3121 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12462 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3132 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12468 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3133 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12474 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3137 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12480 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3138 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12486 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
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
#line 12500 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3150 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12506 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3154 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 12512 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3155 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 12518 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3157 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 12524 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3158 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 12530 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3159 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 12536 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3160 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 12542 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3165 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12548 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3166 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12554 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3170 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12560 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3171 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12566 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3172 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12572 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3173 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12578 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3176 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12584 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3178 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 12590 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3179 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12596 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3180 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 12602 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3185 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12608 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3186 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12614 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3190 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12620 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3191 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12626 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3192 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12632 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3193 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12638 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3198 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12644 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3199 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12650 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3204 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12656 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3206 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12662 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3208 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12668 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3209 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12674 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3213 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 12680 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3215 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 12686 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3216 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 12692 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3218 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 12699 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3223 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12705 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3225 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12711 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3227 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 12725 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3237 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 12731 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3239 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 12737 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3240 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12743 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3243 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 12749 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3244 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 12755 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3245 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 12761 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3249 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 12767 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3250 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 12773 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3251 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12779 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3252 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12785 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3253 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12791 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3254 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12797 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3255 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 12803 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3256 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 12809 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3257 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 12815 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3258 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 12821 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3259 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 12827 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3263 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12833 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3264 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12839 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3269 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12845 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3271 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12851 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3285 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12859 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3290 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 12867 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3294 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12875 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3299 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 12883 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3305 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12889 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3306 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12895 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3310 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12901 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3311 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12907 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3317 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12913 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3321 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 12919 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3327 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12925 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3331 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 12932 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3338 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12938 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3339 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12944 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3343 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 12952 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3346 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 12959 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3352 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12965 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]); }
#line 12971 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3358 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12977 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3359 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12983 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3360 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12989 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1033:
#line 3381 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12995 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1034:
#line 3382 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13001 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3391 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13007 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3402 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13013 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3404 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13019 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3408 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13025 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3411 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13031 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3415 "hphp.y" /* yacc.c:1646  */
    {}
#line 13037 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3416 "hphp.y" /* yacc.c:1646  */
    {}
#line 13043 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3417 "hphp.y" /* yacc.c:1646  */
    {}
#line 13049 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3423 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13056 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3428 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13066 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3437 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13072 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3443 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13081 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3451 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13087 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3452 "hphp.y" /* yacc.c:1646  */
    { }
#line 13093 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3458 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13099 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3460 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13105 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3461 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13115 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3466 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13122 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3472 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("array"); }
#line 13129 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3477 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13135 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3482 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13143 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3486 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13149 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3491 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 13155 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3493 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 13161 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3499 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 13168 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3501 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 13176 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3504 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13182 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3505 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13190 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3508 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13198 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3511 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13204 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3514 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 13212 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3517 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13219 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3519 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 13228 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3525 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 13237 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3531 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 13247 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3539 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13253 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3540 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13259 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;


#line 13263 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 3543 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}
