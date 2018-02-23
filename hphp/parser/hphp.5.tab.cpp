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

using ParamMode = HPHP::ParamMode;

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

  _p->onXhpAttributesEnd();
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
  Token arr1; _p->onExprListElem(arr1, nullptr, num);

  Token arr2;
  switch (type.num()) {
    case 5: /* class */ {
      Token cls; _p->onScalar(cls, T_CONSTANT_ENCAPSED_STRING, type);
      _p->onExprListElem(arr2, &arr1, cls);
      break;
    }
    case 7: /* enum */ {
      Token arr;   _p->onVArray(arr, type);
      _p->onExprListElem(arr2, &arr1, arr);
      break;
    }
    default: {
      Token tnull; scalar_null(_p, tnull);
      _p->onExprListElem(arr2, &arr1, tnull);
      break;
    }
  }

  Token arr3; _p->onExprListElem(arr3, &arr2, def);
  Token arr4; _p->onExprListElem(arr4, &arr3, req);
  _p->onVArray(out, arr4);
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
  Token arrAttributes; _p->onDArray(arrAttributes, attributes);

  Token dummy;

  Token stmts0;
  {
    _p->onStatementListStart(stmts0);
  }
  Token stmts1;
  {
    // static $_ = null;
    Token null;    scalar_null(_p, null);
    Token var;     var.set(T_VARIABLE, "_");
    Token decl;    _p->onStaticVariable(decl, 0, var, &null);
    Token sdecl;   _p->onStatic(sdecl, decl);
    _p->addStatement(stmts1, stmts0, sdecl);
  }
  Token stmts2;
  {
    // if ($_ === null) {
    //   $_ = __SystemLib\\merge_xhp_attr_declarations(
    //          parent::__xhpAttributeDeclaration(),
    //          attributes
    //        );
    // }
    Token parent;  parent.set(T_STRING, "parent");
    Token cls;     _p->onName(cls, parent, Parser::StringName);
    Token fname2;   fname2.setText("__xhpAttributeDeclaration");
    Token param1;  _p->onCall(param1, 0, fname2, dummy, &cls);
    Token params1; _p->onCallParam(params1, NULL, param1, ParamMode::In, false);

    for (unsigned int i = 0; i < classes.size(); i++) {
      Token parent2;  parent2.set(T_STRING, classes[i]);
      Token cls2;     _p->onName(cls2, parent2, Parser::StringName);
      Token fname3;   fname3.setText("__xhpAttributeDeclaration");
      Token param;   _p->onCall(param, 0, fname3, dummy, &cls2);

      Token params; _p->onCallParam(params, &params1, param, ParamMode::In,
                                    false);
      params1 = params;
    }

    Token params2; _p->onCallParam(params2, &params1, arrAttributes,
                                   ParamMode::In, false);

    Token name;    name.set(T_STRING, "__SystemLib\\merge_xhp_attr_declarations");
                   name = name.num() | 2; // WTH???
    Token call;    _p->onCall(call, 0, name, params2, NULL);
    Token tvar;    tvar.set(T_VARIABLE, "_");
    Token var;     _p->onSimpleVariable(var, tvar);
    Token assign;  _p->onAssign(assign, var, call, 0);
    Token exp;     _p->onExpStatement(exp, assign);
    Token block;   _p->onBlock(block, exp);

    Token tvar2;   tvar2.set(T_VARIABLE, "_");
    Token var2;    _p->onSimpleVariable(var2, tvar2);
    Token null;    scalar_null(_p, null);
    Token cond;    BEXP(cond, var2, null, T_IS_IDENTICAL);
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
    Token arr;     _p->onDArray(arr, categories);
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
  Token arr1; _p->onExprListElem(arr1, &arr, num);

  Token name;
  if (tag.num() == 3 || tag.num() == 4) {
    _p->onScalar(name, T_CONSTANT_ENCAPSED_STRING, tag);
  } else if (tag.num() >= 0) {
    scalar_null(_p, name);
  } else {
    HPHP_PARSER_ERROR("XHP: unknown children declaration", _p);
  }
  Token arr2; _p->onExprListElem(arr2, &arr1, name);
  arr = arr2;
}

static void xhp_children_decl(Parser *_p, Token &out, Token &op1, int op,
                              Token *op2) {
  Token num; scalar_num(_p, num, op);
  Token arr; _p->onExprListElem(arr, nullptr, num);

  if (op2) {
    Token arr1; _p->onExprListElem(arr1, &arr,  op1);
    Token arr2; _p->onExprListElem(arr2, &arr1, *op2);
    _p->onVArray(out, arr2);
  } else {
    xhp_children_decl_tag(_p, arr, op1);
    _p->onVArray(out, arr);
  }
}

static void xhp_children_paren(Parser *_p, Token &out, Token exp, int op) {
  Token num;  scalar_num(_p, num, op);
  Token arr1; _p->onExprListElem(arr1, nullptr, num);

  Token num5; scalar_num(_p, num5, 5);
  Token arr2; _p->onExprListElem(arr2, &arr1, num5);

  Token arr3; _p->onExprListElem(arr3, &arr2, exp);
  _p->onVArray(out, arr3);
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

#line 662 "hphp.5.tab.cpp" /* yacc.c:339  */

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
    T_INOUT = 323,
    T_HASHBANG = 324,
    T_CHARACTER = 325,
    T_BAD_CHARACTER = 326,
    T_ENCAPSED_AND_WHITESPACE = 327,
    T_CONSTANT_ENCAPSED_STRING = 328,
    T_ECHO = 329,
    T_DO = 330,
    T_WHILE = 331,
    T_ENDWHILE = 332,
    T_FOR = 333,
    T_ENDFOR = 334,
    T_FOREACH = 335,
    T_ENDFOREACH = 336,
    T_DECLARE = 337,
    T_ENDDECLARE = 338,
    T_AS = 339,
    T_SUPER = 340,
    T_SWITCH = 341,
    T_ENDSWITCH = 342,
    T_CASE = 343,
    T_DEFAULT = 344,
    T_BREAK = 345,
    T_GOTO = 346,
    T_CONTINUE = 347,
    T_FUNCTION = 348,
    T_CONST = 349,
    T_RETURN = 350,
    T_TRY = 351,
    T_CATCH = 352,
    T_THROW = 353,
    T_USING = 354,
    T_USE = 355,
    T_GLOBAL = 356,
    T_STATIC = 357,
    T_ABSTRACT = 358,
    T_FINAL = 359,
    T_PRIVATE = 360,
    T_PROTECTED = 361,
    T_PUBLIC = 362,
    T_VAR = 363,
    T_UNSET = 364,
    T_ISSET = 365,
    T_EMPTY = 366,
    T_HALT_COMPILER = 367,
    T_CLASS = 368,
    T_INTERFACE = 369,
    T_EXTENDS = 370,
    T_IMPLEMENTS = 371,
    T_OBJECT_OPERATOR = 372,
    T_NULLSAFE_OBJECT_OPERATOR = 373,
    T_DOUBLE_ARROW = 374,
    T_LIST = 375,
    T_ARRAY = 376,
    T_DICT = 377,
    T_VEC = 378,
    T_VARRAY = 379,
    T_DARRAY = 380,
    T_KEYSET = 381,
    T_CALLABLE = 382,
    T_CLASS_C = 383,
    T_METHOD_C = 384,
    T_FUNC_C = 385,
    T_LINE = 386,
    T_FILE = 387,
    T_COMMENT = 388,
    T_DOC_COMMENT = 389,
    T_OPEN_TAG = 390,
    T_OPEN_TAG_WITH_ECHO = 391,
    T_CLOSE_TAG = 392,
    T_WHITESPACE = 393,
    T_START_HEREDOC = 394,
    T_END_HEREDOC = 395,
    T_DOLLAR_OPEN_CURLY_BRACES = 396,
    T_CURLY_OPEN = 397,
    T_DOUBLE_COLON = 398,
    T_NAMESPACE = 399,
    T_NS_C = 400,
    T_DIR = 401,
    T_NS_SEPARATOR = 402,
    T_XHP_LABEL = 403,
    T_XHP_TEXT = 404,
    T_XHP_ATTRIBUTE = 405,
    T_XHP_CATEGORY = 406,
    T_XHP_CATEGORY_LABEL = 407,
    T_XHP_CHILDREN = 408,
    T_ENUM = 409,
    T_XHP_REQUIRED = 410,
    T_TRAIT = 411,
    T_ELLIPSIS = 412,
    T_INSTEADOF = 413,
    T_TRAIT_C = 414,
    T_HH_ERROR = 415,
    T_FINALLY = 416,
    T_XHP_TAG_LT = 417,
    T_XHP_TAG_GT = 418,
    T_TYPELIST_LT = 419,
    T_TYPELIST_GT = 420,
    T_UNRESOLVED_LT = 421,
    T_COLLECTION = 422,
    T_SHAPE = 423,
    T_TUPLE = 424,
    T_TYPE = 425,
    T_UNRESOLVED_TYPE = 426,
    T_NEWTYPE = 427,
    T_UNRESOLVED_NEWTYPE = 428,
    T_COMPILER_HALT_OFFSET = 429,
    T_ASYNC = 430,
    T_LAMBDA_OP = 431,
    T_LAMBDA_CP = 432,
    T_UNRESOLVED_OP = 433,
    T_WHERE = 434
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

#line 906 "hphp.5.tab.cpp" /* yacc.c:358  */

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
#define YYLAST   20331

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  209
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  316
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1132
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  2112

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   434

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,   207,     2,   205,    55,    38,   208,
     199,   200,    53,    50,     9,    51,    52,    54,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    32,   201,
      43,    14,    45,    31,    68,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    70,     2,   204,    37,     2,   206,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   202,    36,   203,    58,     2,     2,     2,
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
     194,   195,   196,   197,   198
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   760,   760,   760,   769,   771,   774,   775,   776,   777,
     778,   779,   780,   783,   785,   785,   787,   787,   789,   792,
     797,   802,   805,   808,   812,   816,   820,   824,   828,   833,
     834,   835,   836,   837,   838,   839,   840,   841,   842,   843,
     844,   845,   849,   850,   851,   852,   853,   854,   855,   856,
     857,   858,   859,   860,   861,   862,   863,   864,   865,   866,
     867,   868,   869,   870,   871,   872,   873,   874,   875,   876,
     877,   878,   879,   880,   881,   882,   883,   884,   885,   886,
     887,   888,   889,   890,   891,   892,   893,   894,   895,   896,
     897,   898,   899,   900,   901,   902,   903,   904,   905,   906,
     907,   908,   909,   910,   911,   912,   913,   917,   918,   922,
     923,   927,   928,   933,   935,   940,   945,   946,   947,   949,
     954,   956,   961,   966,   968,   970,   975,   976,   980,   981,
     983,   987,   994,  1001,  1005,  1011,  1013,  1017,  1018,  1024,
    1026,  1030,  1032,  1037,  1038,  1039,  1040,  1043,  1044,  1048,
    1053,  1053,  1059,  1059,  1066,  1065,  1071,  1071,  1076,  1077,
    1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,
    1088,  1089,  1090,  1094,  1092,  1101,  1099,  1106,  1116,  1110,
    1120,  1118,  1122,  1126,  1130,  1134,  1138,  1142,  1146,  1151,
    1152,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,  1167,  1189,  1195,  1196,  1205,  1207,  1211,  1212,
    1213,  1217,  1218,  1222,  1222,  1227,  1233,  1237,  1237,  1245,
    1246,  1250,  1251,  1255,  1261,  1259,  1276,  1273,  1291,  1288,
    1306,  1305,  1314,  1312,  1324,  1323,  1342,  1340,  1359,  1358,
    1367,  1365,  1376,  1376,  1383,  1382,  1394,  1392,  1405,  1406,
    1410,  1413,  1416,  1417,  1418,  1421,  1422,  1425,  1427,  1430,
    1431,  1434,  1435,  1438,  1439,  1443,  1444,  1449,  1450,  1453,
    1454,  1455,  1459,  1460,  1464,  1465,  1469,  1470,  1474,  1475,
    1480,  1481,  1487,  1488,  1489,  1490,  1493,  1496,  1498,  1501,
    1502,  1506,  1508,  1511,  1514,  1517,  1518,  1521,  1522,  1526,
    1532,  1538,  1545,  1547,  1552,  1557,  1563,  1567,  1572,  1577,
    1582,  1588,  1594,  1600,  1606,  1612,  1619,  1629,  1634,  1639,
    1645,  1647,  1651,  1655,  1660,  1664,  1668,  1672,  1676,  1681,
    1686,  1691,  1696,  1701,  1707,  1716,  1717,  1718,  1722,  1724,
    1727,  1729,  1731,  1733,  1735,  1738,  1741,  1744,  1750,  1751,
    1754,  1755,  1756,  1760,  1761,  1763,  1764,  1768,  1770,  1773,
    1777,  1783,  1785,  1788,  1791,  1795,  1797,  1800,  1803,  1801,
    1818,  1815,  1830,  1832,  1834,  1836,  1838,  1840,  1842,  1846,
    1847,  1848,  1851,  1857,  1861,  1867,  1870,  1875,  1877,  1882,
    1887,  1891,  1892,  1896,  1897,  1899,  1901,  1907,  1908,  1910,
    1914,  1915,  1920,  1924,  1925,  1929,  1930,  1934,  1936,  1942,
    1947,  1948,  1950,  1954,  1955,  1956,  1957,  1961,  1962,  1963,
    1964,  1965,  1966,  1968,  1973,  1976,  1977,  1981,  1982,  1986,
    1987,  1990,  1991,  1994,  1995,  1998,  1999,  2003,  2004,  2005,
    2006,  2007,  2008,  2009,  2013,  2014,  2017,  2018,  2019,  2022,
    2024,  2026,  2027,  2030,  2032,  2036,  2038,  2042,  2046,  2050,
    2055,  2059,  2060,  2062,  2063,  2064,  2065,  2068,  2069,  2073,
    2074,  2078,  2079,  2080,  2081,  2085,  2089,  2094,  2098,  2102,
    2106,  2110,  2115,  2119,  2120,  2121,  2122,  2123,  2127,  2131,
    2133,  2134,  2135,  2138,  2139,  2140,  2141,  2142,  2143,  2144,
    2145,  2146,  2147,  2148,  2149,  2150,  2151,  2152,  2153,  2154,
    2155,  2156,  2157,  2158,  2159,  2160,  2161,  2162,  2163,  2164,
    2165,  2166,  2167,  2168,  2169,  2170,  2171,  2172,  2173,  2174,
    2175,  2176,  2177,  2178,  2179,  2180,  2181,  2183,  2184,  2186,
    2187,  2189,  2190,  2191,  2192,  2193,  2194,  2195,  2196,  2197,
    2198,  2199,  2200,  2201,  2202,  2203,  2204,  2205,  2206,  2207,
    2208,  2209,  2210,  2211,  2212,  2213,  2214,  2218,  2222,  2227,
    2226,  2242,  2240,  2259,  2258,  2279,  2278,  2298,  2297,  2316,
    2316,  2333,  2333,  2352,  2353,  2354,  2359,  2361,  2365,  2369,
    2375,  2379,  2385,  2387,  2391,  2393,  2397,  2401,  2402,  2406,
    2408,  2412,  2414,  2418,  2420,  2424,  2427,  2432,  2434,  2438,
    2441,  2446,  2450,  2454,  2458,  2462,  2466,  2470,  2474,  2478,
    2482,  2486,  2490,  2494,  2498,  2502,  2506,  2510,  2514,  2518,
    2520,  2524,  2526,  2530,  2532,  2536,  2543,  2550,  2552,  2557,
    2558,  2559,  2560,  2561,  2562,  2563,  2564,  2565,  2566,  2568,
    2569,  2573,  2574,  2575,  2576,  2580,  2586,  2599,  2616,  2617,
    2620,  2621,  2623,  2628,  2629,  2632,  2636,  2639,  2642,  2649,
    2650,  2654,  2655,  2657,  2662,  2663,  2664,  2665,  2666,  2667,
    2668,  2669,  2670,  2671,  2672,  2673,  2674,  2675,  2676,  2677,
    2678,  2679,  2680,  2681,  2682,  2683,  2684,  2685,  2686,  2687,
    2688,  2689,  2690,  2691,  2692,  2693,  2694,  2695,  2696,  2697,
    2698,  2699,  2700,  2701,  2702,  2703,  2704,  2705,  2706,  2707,
    2708,  2709,  2710,  2711,  2712,  2713,  2714,  2715,  2716,  2717,
    2718,  2719,  2720,  2721,  2722,  2723,  2724,  2725,  2726,  2727,
    2728,  2729,  2730,  2731,  2732,  2733,  2734,  2735,  2736,  2737,
    2738,  2739,  2740,  2741,  2742,  2743,  2744,  2748,  2753,  2754,
    2758,  2759,  2760,  2761,  2763,  2767,  2768,  2779,  2780,  2782,
    2784,  2796,  2797,  2798,  2802,  2803,  2804,  2808,  2809,  2810,
    2813,  2815,  2819,  2820,  2821,  2822,  2824,  2825,  2826,  2827,
    2828,  2829,  2830,  2831,  2832,  2833,  2836,  2841,  2842,  2843,
    2845,  2846,  2848,  2849,  2850,  2851,  2852,  2853,  2854,  2855,
    2856,  2857,  2859,  2861,  2863,  2865,  2867,  2868,  2869,  2870,
    2871,  2872,  2873,  2874,  2875,  2876,  2877,  2878,  2879,  2880,
    2881,  2882,  2883,  2885,  2887,  2889,  2891,  2892,  2895,  2896,
    2900,  2904,  2906,  2910,  2911,  2915,  2921,  2924,  2928,  2929,
    2930,  2931,  2932,  2933,  2934,  2939,  2941,  2945,  2946,  2949,
    2950,  2954,  2957,  2959,  2961,  2965,  2966,  2967,  2968,  2971,
    2975,  2976,  2977,  2978,  2982,  2984,  2991,  2992,  2993,  2994,
    2999,  3000,  3001,  3002,  3004,  3005,  3007,  3008,  3009,  3010,
    3011,  3012,  3016,  3018,  3022,  3024,  3027,  3030,  3032,  3034,
    3037,  3039,  3043,  3045,  3048,  3051,  3057,  3059,  3062,  3063,
    3068,  3071,  3075,  3075,  3080,  3083,  3084,  3088,  3089,  3093,
    3094,  3095,  3099,  3101,  3109,  3110,  3114,  3116,  3124,  3125,
    3129,  3131,  3132,  3137,  3139,  3144,  3155,  3169,  3181,  3196,
    3197,  3198,  3199,  3200,  3201,  3202,  3212,  3221,  3223,  3225,
    3229,  3233,  3234,  3235,  3236,  3237,  3253,  3254,  3264,  3265,
    3266,  3267,  3268,  3269,  3270,  3275,  3277,  3282,  3286,  3287,
    3291,  3294,  3298,  3305,  3309,  3318,  3325,  3327,  3333,  3335,
    3336,  3340,  3341,  3342,  3349,  3350,  3355,  3356,  3361,  3362,
    3363,  3364,  3375,  3378,  3381,  3382,  3383,  3384,  3395,  3399,
    3400,  3401,  3403,  3404,  3405,  3409,  3411,  3414,  3416,  3417,
    3418,  3419,  3422,  3424,  3425,  3429,  3431,  3434,  3436,  3437,
    3438,  3442,  3444,  3447,  3450,  3452,  3454,  3458,  3459,  3461,
    3462,  3468,  3469,  3471,  3481,  3483,  3485,  3488,  3489,  3490,
    3494,  3495,  3496,  3497,  3498,  3499,  3500,  3501,  3502,  3503,
    3504,  3508,  3509,  3513,  3515,  3523,  3525,  3529,  3533,  3538,
    3542,  3550,  3551,  3555,  3556,  3562,  3563,  3572,  3573,  3581,
    3584,  3588,  3591,  3596,  3601,  3604,  3607,  3609,  3611,  3613,
    3617,  3619,  3620,  3621,  3624,  3626,  3632,  3633,  3637,  3638,
    3642,  3643,  3647,  3648,  3651,  3656,  3657,  3661,  3664,  3666,
    3670,  3676,  3677,  3678,  3682,  3686,  3694,  3699,  3711,  3713,
    3717,  3720,  3722,  3727,  3732,  3738,  3741,  3746,  3751,  3753,
    3760,  3762,  3765,  3766,  3769,  3772,  3773,  3778,  3780,  3784,
    3790,  3800,  3801
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
  "T_VARIABLE", "T_PIPE_VAR", "T_NUM_STRING", "T_INLINE_HTML", "T_INOUT",
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
  "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION", "T_SHAPE", "T_TUPLE",
  "T_TYPE", "T_UNRESOLVED_TYPE", "T_NEWTYPE", "T_UNRESOLVED_NEWTYPE",
  "T_COMPILER_HALT_OFFSET", "T_ASYNC", "T_LAMBDA_OP", "T_LAMBDA_CP",
  "T_UNRESOLVED_OP", "T_WHERE", "'('", "')'", "';'", "'{'", "'}'", "']'",
  "'$'", "'`'", "'\"'", "'\\''", "$accept", "start", "$@1",
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
  "parameter_list", "non_empty_parameter_list", "inout_variable",
  "function_call_parameter_list", "non_empty_fcall_parameter_list",
  "global_var_list", "global_var", "static_var_list",
  "enum_statement_list", "enum_statement", "enum_constant_declaration",
  "class_statement_list", "class_statement", "$@26", "$@27", "trait_rules",
  "trait_precedence_rule", "trait_alias_rule", "trait_alias_rule_method",
  "xhp_attribute_stmt", "xhp_attribute_decl",
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
  "closure_expression", "$@28", "$@29", "lambda_expression", "$@30",
  "$@31", "$@32", "$@33", "$@34", "lambda_body", "shape_keyname",
  "non_empty_shape_pair_list", "non_empty_static_shape_pair_list",
  "shape_pair_list", "static_shape_pair_list", "shape_literal",
  "array_literal", "dict_pair_list", "non_empty_dict_pair_list",
  "static_dict_pair_list", "non_empty_static_dict_pair_list",
  "static_dict_pair_list_ae", "non_empty_static_dict_pair_list_ae",
  "dict_literal", "static_dict_literal", "static_dict_literal_ae",
  "vec_literal", "static_vec_literal", "static_vec_literal_ae",
  "keyset_literal", "static_keyset_literal", "static_keyset_literal_ae",
  "tuple_literal", "static_tuple_literal", "static_tuple_literal_ae",
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
  "non_empty_user_attribute_list", "user_attribute_list", "$@35",
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
  "hh_non_empty_type_list", "hh_type_list", "hh_non_empty_func_type_list",
  "hh_func_type_list", "opt_type_constraint_where_clause",
  "non_empty_constraint_list", "hh_generalised_constraint",
  "opt_return_type", "hh_constraint", "hh_typevar_list",
  "hh_non_empty_constraint_list", "hh_non_empty_typevar_list",
  "hh_typevar_variance", "hh_shape_member_type",
  "hh_non_empty_shape_member_list", "hh_shape_member_list",
  "hh_shape_type", "hh_access_type_start", "hh_access_type",
  "array_typelist", "hh_type", "hh_type_opt", YY_NULLPTR
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
     426,   427,   428,   429,   430,   431,   432,   433,   434,    40,
      41,    59,   123,   125,    93,    36,    96,    34,    39
};
# endif

#define YYPACT_NINF -1719

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1719)))

#define YYTABLE_NINF -1133

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1133)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1719,   191, -1719, -1719,  5547, 15069, 15069,    24, 15069, 15069,
   15069, 15069, 12670, 15069, -1719, 15069, 15069, 15069, 15069, 18371,
   18371, 15069, 15069, 15069, 15069, 15069, 15069, 15069, 15069, 12849,
   19169, 15069,    28,    32, -1719, -1719, -1719,   231, -1719,   200,
   -1719, -1719, -1719,   336, 15069, -1719,    32,   212,   218,   225,
   -1719,    32, 13028, 16876, 13207, -1719, 16087, 11533,   236, 15069,
   19418,   110,    70,   329,   429, -1719, -1719, -1719,   251,   258,
     307,   326, -1719, 16876,   339,   343,   475,   495,   498,   509,
     523, -1719, -1719, -1719, -1719, -1719, 15069,   582,  4596, -1719,
   -1719, 16876, -1719, -1719, -1719, -1719, 16876, -1719, 16876, -1719,
     432,   404,   437, 16876, 16876, -1719,   188, -1719, -1719, 13413,
   -1719, -1719,   442,   626,   654,   654, -1719,   615,   485,   617,
     463, -1719,    94, -1719,   478,   561,   648, -1719, -1719, -1719,
   -1719,  5012,   599, -1719,   130, -1719,   512,   515,   530,   539,
     543,   554,   576,   596,  5388, -1719, -1719, -1719, -1719, -1719,
     139,   625,   637,   640,   677,   713,   729, -1719,   736,   748,
   -1719,   203,   621, -1719,   674,    16, -1719,  1584,   187, -1719,
   -1719,  2389,   130,   130,   641,   176, -1719,   164,   112,   646,
     198, -1719,   362, -1719,   780, -1719,   691, -1719, -1719,   660,
     711, -1719, 15069, -1719,   648,   599, 19706,  4164, 19706, 15069,
   19706, 19706,  5219,  5219,   681, 18544, 19706,   835, 16876,   822,
     822,   153,   822, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719, -1719,   111, 15069,   718, -1719, -1719,   744,   710,   328,
     715,   328,   822,   822,   822,   822,   822,   822,   822,   822,
   18371, 18592,   717,   908,   691, -1719, 15069,   718, -1719,   757,
   -1719,   760,   725, -1719,   172, -1719, -1719, -1719,   328,   130,
   -1719, 13592, -1719, -1719, 15069, 10100,   917,    97, 19706, 11130,
   -1719, 15069, 15069, 16876, -1719, -1719, 17307,   726, -1719, 17355,
   -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   17731, -1719, 17731, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719, -1719,    96,   105,   711, -1719, -1719, -1719, -1719,   731,
   -1719, 17587,   126, -1719, -1719,   766,   922, -1719,   776, 16828,
   15069, -1719,   740,   749, 17403, -1719,    55, 17452, 15469, 15469,
   15469, 16876, 15469,   750,   942,   753, -1719,   278, -1719, 18038,
      98, -1719,   947,   115,   824, -1719,   839, -1719, 18371, 15069,
   15069,   775,   794, -1719, -1719, 18114, 12849, 15069, 15069, 15069,
   15069, 15069,   116,   506,   426, -1719, 15248, 18371,   644, -1719,
   16876, -1719,    37,   485, -1719, -1719, -1719, -1719, 19271, 15069,
     966,   881, -1719, -1719, -1719,   104, 15069,   786,   792, 19706,
     796,  1949,   799,  6186, 15069,   368,   795,   678,   368,   647,
     507, -1719, 16876, 17731,   803, 11712, 16087, -1719, 13798,   801,
     801,   801,   801, -1719, -1719, 15801, -1719, -1719, -1719, -1719,
   -1719,   648, -1719, 15069, 15069, -1719, -1719, -1719, -1719, -1719,
   -1719, -1719, -1719, -1719, 15069, 15069, 15069, 15069, 13977, 15069,
   15069, 15069, 15069, 15069, 15069, 15069, 15069, 15069, 15069, 15069,
   15069, 15069, 15069, 15069, 15069, 15069, 15069, 15069, 15069, 15069,
   15069, 15069, 19347, 15069, -1719, 15069, 15069, 15069, 15421, 16876,
   16876, 16876, 16876, 16876,  5012,   892,   649, 11336, 15069, 15069,
   15069, 15069, 15069, 15069, 15069, 15069, 15069, 15069, 15069, 15069,
   -1719, -1719, -1719, -1719,  4093, -1719, -1719, 11712, 11712, 15069,
   15069,   442,   177, 18114,   806,   648, 14156, 17500, -1719, 15069,
   -1719,   807,   999,   850,   811,   812, 15575,   328, 14335, -1719,
   14514, -1719,   725,   814,   817,  2375, -1719,   365, 11712, -1719,
    4347, -1719, -1719, 17548, -1719, -1719, 12088, -1719, 15069, -1719,
     924, 10306,  1012,   821, 19584,  1010,   108,    88, -1719, -1719,
   -1719,   842, -1719, -1719, -1719, 17731, -1719,   622,   828,  1019,
   17962, 16876, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719,   830, -1719, -1719,   831,   833,   834,   837,   841,   840,
     331,   845,   843, 16338, 16528, -1719, -1719, 16876, 16876, 15069,
     328,   110, -1719, 17962,   952, -1719, -1719, -1719,   328,   140,
     148,   854,   856,  2761,   170,   857,   868,   563,   939,   875,
     328,   149,   876, 18653,   874,  1071,  1072,   879,   884,   885,
     887, -1719,  4222, 16876, -1719, -1719,  1022,  2965,   458, -1719,
   -1719, -1719,   485, -1719, -1719, -1719,  1063,   963,   921,    82,
     943, 15069,   442,   968,  1100,   911, -1719,   950, -1719,   177,
   -1719,   914, 17731, 17731,  1101,   917,   104, -1719,   920,  1110,
   -1719, 17635,   100, -1719,   390,   208, -1719, -1719, -1719, -1719,
   -1719, -1719, -1719,   764,  3107, -1719, -1719, -1719, -1719,  1115,
     945, -1719, 18371,   533, 15069,   927,  1125, 19706,  1121,   151,
    1130,   940,   941,   944, 19706,   946,  2833,  6392, -1719, -1719,
   -1719, -1719, -1719, -1719,  1016, 15980, 19706,   948,  3155, 19845,
   19936,  5219, 20017, 15069, 19658, 20090, 15422, 20195, 20262, 15909,
   16636, 19354, 19354, 19354, 19354,  1517,  1517,  1517,  1517,  1517,
     912,   912,   919,   919,   919,   153,   153,   153, -1719,   822,
     951,   953, 18701,   956,  1147,    18, 15069,    22,   718,    54,
     177, -1719, -1719, -1719,  1146,   881, -1719,   648, 18219, -1719,
   -1719, -1719,  5219,  5219,  5219,  5219,  5219,  5219,  5219,  5219,
    5219,  5219,  5219,  5219,  5219, -1719, 15069,   397,   185, -1719,
   -1719,   718,   424,   962,   964,   961,  3290,   152,   971, -1719,
   19706,  5398, -1719, 16876, -1719,   328,    53, 18371, 19706, 18371,
   18762,  1016,   367,   328,   202,  1005,   973, 15069, -1719,   217,
   -1719, -1719, -1719,  6598,   658, -1719, -1719, 19706, 19706,    32,
   -1719, -1719, -1719, 15069,  1074, 17886, 17962, 16876, 10512,   977,
     980, -1719,  1165, 16159,  1047, -1719,  1024, -1719,  1178,   988,
    5215, 17731, 17962, 17962, 17962, 17962, 17962,   990,  1120,  1122,
    1123,  1124,  1127,   996,  1000, 17962,    12, -1719, -1719, -1719,
   -1719, -1719, -1719, -1719,    42, -1719, 19800, -1719, -1719,    45,
   -1719,  6804,  3854,  1003, 16528, -1719, 16528, -1719, 16528, -1719,
   16876, 16876, 16528, -1719, 16528, 16528, 16876, -1719,  1191,  1004,
   -1719,   344, -1719, -1719,  3361, -1719, 19800,  1187, 18371,  1007,
   -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719,  1025,
    1197, 16876,  3854,  1011, 18114, 18295,  1198, -1719, 15069, -1719,
   15069, -1719, 15069, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719,  1013, -1719, 15069, -1719, -1719,  5774, -1719, 17731,  3854,
    1017, -1719, -1719, -1719, -1719,  1048,  1211,  1026, 15069, 19271,
   -1719, -1719, 15421, -1719,  1028, -1719, 17731, -1719,  1034,  7010,
    1199,   163, -1719, 17731, -1719,   142,  4093,  4093, -1719, 17731,
   -1719, -1719,   328, -1719, -1719,  1163, 19706, -1719, 11891, -1719,
   17962, 13798,   801, 13798, -1719,   801,   801, -1719, 12294, -1719,
   -1719,  7216, -1719,   103,  1035,  3854,   963, -1719, -1719, -1719,
   -1719, 20090, 15069, -1719, -1719, 15069, -1719, 15069, -1719,  3848,
    1036, 11712,   939,  1204,   963, 17731,  1223,  1016, 16876, 19347,
     328,  4076,  1042,   216,  1043, -1719, -1719,  1230,  1498,  1498,
    5398, -1719, -1719, -1719,  1196,  1050,  1182,  1183,  1185,  1189,
    1190,   340,  1064,  1065,   492, -1719, -1719, -1719, -1719, -1719,
   -1719,  1107, -1719, -1719, -1719, -1719,  1253,  1073,   807,   328,
     328, 14693,   963,  4347, -1719,  4347, -1719,  4305,   734,    32,
   11130, -1719,  7422,  1077,  7628,  1078, 17886, 18371,  1075,  1134,
     328, 19800,  1261, -1719, -1719, -1719, -1719,   650, -1719,    57,
   17731,  1097,  1144,  1126, 17731, 16876,  2783, -1719, -1719, 17731,
    1274,  1094,  1128,  1129,  1115,   639,   639,  1231,  1231, 18919,
    1095,  1292, 17962, 17962, 17962, 17962, 17962, 17962, 19271, 17962,
    3619, 16982, 17962, 17962, 17962, 17962, 17810, 17962, 17962, 17962,
   17962, 17962, 17962, 17962, 17962, 17962, 17962, 17962, 17962, 17962,
   17962, 17962, 17962, 17962, 17962, 17962, 17962, 17962, 17962, 17962,
   16876, -1719, -1719,  1219, -1719, -1719,  1103,  1104,  1105, -1719,
    1109, -1719, -1719,   436, 16338, -1719,  1113, -1719, 17962,   328,
   -1719, -1719,   169, -1719,   709,  1306, -1719, -1719,   154,  1117,
     328, 12491, 19706, 18810, -1719,  2620, -1719,  5980,   881,  1306,
   -1719,   381, 15069,    61, -1719, 19706,  1179,  1132, -1719,  1118,
    1199, -1719, -1719, -1719, 14890, 17731,   917, 17683,  1237,    85,
    1307,  1241,   223, -1719,   718,   224, -1719,   718, -1719, 15069,
   18371,   533, 15069, 19706, 19800, -1719, -1719, -1719,  4393, -1719,
   -1719, -1719, -1719, -1719, -1719,  1133,   103, -1719,  1135,   103,
    1136, 20090, 19706, 18871,  1139, 11712,  1142,  1137, 17731,  1141,
    1131, 17731,   963, -1719,   725,   483, 11712, 15069, -1719, -1719,
   -1719, -1719, -1719, -1719,  1206,  1145,  1320,  1242,  5398,  5398,
    5398,  5398,  5398,  5398,  1186, -1719, 19271,  5398,   118,  5398,
   -1719, -1719, -1719, 18371, 19706,  1148, -1719,    32,  1316,  1275,
   11130, -1719, -1719, -1719,  1153, 15069,  1134,   328, 18114, 17886,
    1155, 17962,  7834,   720,  1156, 15069,   123,   355, -1719,  1172,
   -1719, 17731, 16876, -1719,  1220, -1719, -1719, -1719, 17539, -1719,
    1327, -1719,  1160, 17962, -1719, 17962, -1719,  1161,  1158,  1354,
   18979,  1162, 19800,  1355,  1164,  1166,  1170,  1238,  1360,  1175,
    1180, -1719, -1719, -1719, 19025,  1176,  1369, 19892, 19980, 20053,
   17962, 19754, 20161, 20229, 17894, 16088,  3502, 18302, 18302, 18302,
   18302,  2177,  2177,  2177,  2177,  2177,   998,   998,   639,   639,
     639,  1231,  1231,  1231,  1231, -1719,  1181, -1719,  1193,  1194,
    1195,  1200, -1719, -1719, 19800, 16876, 17731, 17731, -1719,   709,
    3854,  1613, -1719, 18114, -1719, -1719,  5219, 15069,  1184, -1719,
    1188,  1948, -1719,   121, 15069, -1719, -1719,  4570, -1719, 15069,
   -1719, 15069, -1719,   917, 13798,  1205,   240,   801,   240,   237,
   -1719, -1719, 17731,   147, -1719,  1368,  1301, 15069, -1719,  1192,
    1208,  1207,   328,  1163, 19706,  1199,  1210, -1719,  1212,   103,
   15069, 11712,  1213, -1719, -1719,   881, -1719, -1719,  1215,  1221,
    1209, -1719,  1222,  5398, -1719,  5398, -1719, -1719,  1224,  1217,
    1377,  1250,  1226, -1719,  1383,  1227,  1229,  1232, -1719,  1259,
    1234,  1389,  1235, -1719, -1719,   328, -1719,  1405, -1719,  1239,
   -1719, -1719,  1245,  1247,   157, -1719, -1719, 19800,  1251,  1254,
   -1719,  5182, -1719, -1719, -1719, -1719, -1719, -1719,  1300, 17731,
   17731,  1128,  1272, 17731, -1719, 19800, 19085, -1719, -1719, 17962,
   -1719, 17962, -1719, 17962, -1719, -1719, -1719, -1719, 17962, 19271,
   -1719, -1719, -1719, 17962, -1719, 17962, -1719, 20126, 17962,  1249,
    8040, -1719, -1719, -1719, -1719,   709, -1719, -1719, -1719, -1719,
     696, 16266,  3854,  1341, -1719,  3596,  1286,  5194, -1719, -1719,
    1375,   892,  4749,   119,   128,  1260,   881,   649,   158, 19706,
   -1719, -1719, -1719,  1293,  4955, -1719,  5003, 19706, -1719,  3438,
   -1719,  6392,  1381,    89,  1451,  1384, 15069, -1719, 19706, 11712,
   11712, -1719,  1350,  1199,  1983,  1199,  1268, 19706,  1270, -1719,
    2160,  1277,  2253, -1719, -1719,   103, -1719, -1719,  1339, -1719,
   -1719,  5398, -1719,  5398, -1719,  5398, -1719, -1719, -1719, -1719,
    5398, -1719, 19271, -1719, -1719,  2382, -1719,  8246, -1719, -1719,
   -1719, -1719, 10718, -1719, -1719, -1719,  6598, 17731, -1719, -1719,
   -1719,  1280, 17962, 19131, 19800, 19800, 19800,  1343, 19800, 19191,
   20126, -1719, -1719,   709,  3854,  3854, 16876, -1719,  1470, 17136,
      80, -1719, 16266,   881, 16721, -1719,  1302, -1719,   129,  1285,
     132, -1719, 16635, -1719, -1719, -1719,   134, -1719, -1719,  2373,
   -1719,  1287, -1719,  1475,   135,   648,  1375, 16456, -1719, 16456,
   -1719, -1719,  1476,   892, -1719, 15729, -1719, -1719, -1719, -1719,
    2693, -1719,  1477,  1409, 15069, -1719, 19706,  1295,  1304,  1294,
    1199,  1303, -1719,  1350,  1199, -1719, -1719, -1719, -1719,  2410,
    1305,  5398,  1364, -1719, -1719, -1719,  1370, -1719,  6598, 10924,
   10718, -1719, -1719, -1719,  6598, -1719, -1719, 19800, 17962, 17962,
   17962,  8452,  1309,  1310, -1719, 17962, -1719,  3854, -1719, -1719,
   -1719, -1719, -1719, 17731,  1349,  3596, -1719, -1719, -1719, -1719,
   -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
     546, -1719,  1286, -1719, -1719, -1719, -1719, -1719,    91,   377,
   -1719, 17962,  1423, -1719, 16828,   138,  1504, -1719, 17731,   648,
   -1719, -1719,  1311,  1507, 15069, -1719, 19706, -1719, -1719,   160,
    1322, 17731,   664,  1199,  1303, 15908, -1719,  1199, -1719,  5398,
    5398, -1719, -1719, -1719, -1719,  8658, 19800, 19800, 19800, -1719,
   -1719, -1719, 19800, -1719,  5620,  1520,  1521,  1323, -1719, -1719,
   17962, 16635, 16635,  1466, -1719,  2373,  2373,   382, -1719, -1719,
   -1719, 19800,  1522,  1352,  1338, -1719, 17962, -1719, 16828, -1719,
   17962, 19706,  1455, -1719,  1530, -1719,  1531, -1719,   363, -1719,
   -1719, -1719,  1345,   664, -1719,   664, -1719, -1719,  8864,  1342,
    1433, -1719,  1447,  1390, -1719, -1719,  1450, 17731,  1371,  1349,
   -1719, -1719, 19800, -1719, -1719,  1380, -1719,  1537, -1719, -1719,
   -1719, -1719, 17962,   563, -1719, 19800,  1358, 19800, -1719,   186,
    1382,  9070, 17731, -1719, 17731, -1719,  9276, -1719, -1719, -1719,
    1379, -1719,  1385,  1401, 16876,   649,  1403, -1719, -1719, -1719,
   19800,  1404,   125, -1719,  1506, -1719, -1719, -1719, -1719, -1719,
   -1719,  9482, -1719,  3854,  1003, -1719,  1413, 16876,   628, -1719,
   -1719,  1392,  1585,   737,   125, -1719, -1719,  1510, -1719,  3854,
    1394, -1719,  1199,   131, -1719, 17731, -1719, -1719, -1719, 17731,
   -1719,  1396,  1397,   141, -1719,  1303,   768,  1515,   150,  1199,
    1398, -1719,   673, 17731, 17731, -1719,   101,  1589,  1524,  1303,
   -1719, -1719, -1719, -1719,  1525,   168,  1595,  1529, 15069, -1719,
     673,  9688,  9894, -1719,   113,  1599,  1533, 15069, -1719, 19706,
   -1719, -1719, -1719,  1603,  1540, 15069, -1719, 19706, 15069, -1719,
   19706, 19706
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   204,   471,     0,   912,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1006,
     994,     0,   776,     0,   782,   783,   784,    29,   849,   981,
     982,   171,   172,   785,     0,   152,     0,     0,     0,     0,
      30,     0,     0,     0,     0,   223,     0,     0,     0,     0,
       0,     0,   440,   441,   442,   439,   438,   437,     0,     0,
       0,     0,   252,     0,     0,     0,    37,    38,    40,    41,
      39,   789,   791,   792,   786,   787,     0,     0,     0,   793,
     788,     0,   759,    32,    33,    34,    36,    35,     0,   790,
       0,     0,     0,     0,     0,   794,   443,   581,    31,     0,
     170,   140,   986,   777,     0,     0,     4,   126,   128,   848,
       0,   758,     0,     6,     0,     0,   222,     7,     9,     8,
      10,     0,     0,   435,     0,   485,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   541,   483,   968,   969,   563,
     556,   557,   558,   559,   562,   560,   561,   466,   566,     0,
     465,   940,   760,   767,     0,   851,   555,   434,   943,   944,
     956,   484,     0,     0,     0,   487,   486,   941,   942,   939,
     976,   980,     0,   545,   850,    11,   440,   441,   442,     0,
       0,    36,     0,   126,   222,     0,  1046,   484,  1047,     0,
    1049,  1050,   565,   479,     0,   472,   477,     0,     0,   527,
     528,   529,   530,    29,   981,   785,   762,    37,    38,    40,
      41,    39,     0,     0,  1070,   961,   760,     0,   761,   506,
       0,   508,   546,   547,   548,   549,   550,   551,   552,   554,
       0,  1010,     0,   858,   772,   242,     0,  1070,   463,   771,
     765,     0,   781,   761,   989,   990,   996,   988,   773,     0,
     464,     0,   775,   553,     0,   205,     0,     0,   468,   205,
     150,   470,     0,     0,   156,   158,     0,     0,   160,     0,
      75,    76,    82,    83,    67,    68,    59,    80,    91,    92,
       0,    62,     0,    66,    74,    72,    94,    86,    85,    57,
     108,    81,   101,   102,    58,    97,    55,    98,    56,    99,
      54,   103,    90,    95,   100,    87,    88,    61,    89,    93,
      53,    84,    69,   104,    77,   106,    70,    60,    47,    48,
      49,    50,    51,    52,    71,   107,   105,   110,    64,    45,
      46,    73,  1123,  1124,    65,  1128,    44,    63,    96,     0,
      79,     0,   126,   109,  1061,  1122,     0,  1125,     0,     0,
       0,   162,     0,     0,     0,   213,     0,     0,     0,     0,
       0,     0,     0,     0,   860,     0,   114,   116,   350,     0,
       0,   349,   355,     0,     0,   253,     0,   256,     0,     0,
       0,     0,  1067,   238,   250,  1002,  1006,   600,   630,   630,
     600,   630,     0,  1031,     0,   796,     0,     0,     0,  1029,
       0,    16,     0,   130,   230,   244,   251,   660,   593,   630,
       0,  1055,   573,   575,   577,   916,   471,   485,     0,     0,
     483,   484,   486,   205,     0,   778,     0,   779,     0,     0,
       0,   202,     0,     0,   132,   339,     0,    28,     0,     0,
       0,     0,     0,   203,   221,     0,   249,   234,   248,   440,
     443,   222,   436,   985,     0,   932,   192,   193,   194,   195,
     196,   198,   199,   201,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   994,     0,   191,   985,   985,  1016,     0,     0,
       0,     0,     0,     0,     0,     0,   433,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     505,   507,   917,   918,     0,   931,   930,   339,   339,   985,
       0,   987,   977,  1002,     0,   222,     0,     0,   164,     0,
     914,   909,   858,     0,   485,   483,     0,  1014,     0,   598,
     857,  1005,   781,   485,   483,   484,   132,     0,   339,   462,
       0,   933,   774,     0,   140,   292,     0,   580,     0,   167,
       0,   205,   469,     0,     0,     0,     0,     0,   159,   190,
     161,  1123,  1124,  1120,  1121,     0,  1127,  1113,     0,     0,
       0,     0,    78,    43,    65,    42,  1062,   197,   200,   163,
     140,     0,   180,   189,     0,     0,     0,     0,     0,     0,
     117,     0,     0,     0,   859,   115,    18,     0,   111,     0,
     351,     0,   165,     0,     0,   166,   254,   255,  1051,     0,
       0,   485,   483,   484,   487,   486,     0,  1103,   262,     0,
    1003,     0,     0,     0,     0,   858,   858,     0,     0,     0,
       0,   168,     0,     0,   795,  1030,   849,     0,     0,  1028,
     854,  1027,   129,     5,    13,    14,     0,   260,     0,     0,
     586,     0,     0,     0,   858,     0,   769,     0,   768,   763,
     587,     0,     0,     0,     0,     0,   916,   136,     0,   860,
     915,  1132,   461,   474,   488,   949,   967,   147,   139,   143,
     144,   145,   146,   434,     0,   564,   852,   853,   127,   858,
       0,  1071,     0,     0,     0,     0,   860,   340,     0,     0,
       0,   485,   209,   210,   208,   483,   484,   205,   184,   182,
     183,   185,   569,   224,   258,     0,   984,     0,     0,   511,
     513,   512,   524,     0,     0,   544,   509,   510,   514,   516,
     515,   533,   534,   531,   532,   535,   536,   537,   538,   539,
     525,   526,   518,   519,   517,   520,   521,   523,   540,   522,
       0,     0,  1020,     0,   858,  1054,     0,  1053,  1070,   946,
     976,   240,   232,   246,     0,  1055,   236,   222,     0,   475,
     478,   480,   490,   493,   494,   495,   496,   497,   498,   499,
     500,   501,   502,   503,   504,   920,     0,   919,   922,   945,
     926,  1070,   923,     0,     0,     0,     0,     0,     0,  1048,
     473,   907,   911,   857,   913,   460,   764,     0,  1009,     0,
    1008,   258,     0,   764,   993,   992,     0,     0,   919,   922,
     991,   923,   482,   294,   296,   136,   584,   583,   467,     0,
     140,   276,   151,   470,     0,     0,     0,     0,   205,   288,
     288,   157,   858,     0,     0,  1112,     0,  1109,   858,     0,
    1083,     0,     0,     0,     0,     0,   856,     0,    37,    38,
      40,    41,    39,     0,     0,     0,   798,   802,   803,   804,
     807,   805,   806,   809,     0,   797,   134,   847,   808,  1070,
    1126,   205,     0,     0,     0,    21,     0,    22,     0,    19,
       0,   112,     0,    20,     0,     0,     0,   123,   860,     0,
     121,   116,   113,   118,     0,   348,   356,   353,     0,     0,
    1040,  1045,  1042,  1041,  1044,  1043,    12,  1101,  1102,     0,
     858,     0,     0,     0,  1002,   999,     0,   597,     0,   611,
     857,   599,   857,   629,   614,   623,   626,   617,  1039,  1038,
    1037,     0,  1033,     0,  1034,  1036,   205,     5,     0,     0,
       0,   655,   656,   665,   664,     0,     0,   483,     0,   857,
     592,   596,     0,   620,     0,  1056,     0,   574,     0,   205,
    1090,   916,   320,  1132,  1131,     0,     0,     0,   983,   857,
    1073,  1069,   342,   336,   337,   341,   343,   757,   859,   338,
       0,     0,     0,     0,   460,     0,     0,   488,     0,   950,
     212,   205,   142,   916,     0,     0,   260,   571,   226,   928,
     929,   543,     0,   637,   638,     0,   635,   857,  1015,     0,
       0,   339,   262,     0,   260,     0,     0,   258,     0,   994,
     491,     0,     0,   947,   948,   978,   979,     0,     0,     0,
     895,   865,   866,   867,   874,     0,    37,    38,    40,    41,
      39,     0,     0,     0,   880,   886,   887,   888,   891,   889,
     890,     0,   878,   876,   877,   901,   858,     0,   909,  1013,
    1012,     0,   260,     0,   934,     0,   780,     0,   298,     0,
     205,   148,   205,     0,   205,     0,     0,     0,     0,   268,
     269,   280,     0,   140,   278,   177,   288,     0,   288,     0,
     857,     0,     0,     0,     0,     0,   857,  1111,  1114,  1079,
     858,     0,  1074,     0,   858,   830,   831,   828,   829,   864,
       0,   858,   856,   604,   632,   632,   604,   632,   595,   632,
       0,     0,  1022,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1117,   214,     0,   217,   181,     0,     0,     0,   119,
       0,   124,   125,   117,   859,   122,     0,   352,     0,  1052,
     169,  1068,  1103,  1094,  1098,   261,   263,   362,     0,     0,
    1000,     0,   602,     0,  1032,     0,    17,   205,  1055,   259,
     362,     0,     0,     0,   764,   589,     0,   770,  1057,     0,
    1090,   578,   135,   137,     0,     0,     0,  1132,     0,     0,
     325,   323,   922,   935,  1070,   922,   936,  1070,  1072,   985,
       0,     0,     0,   344,   133,   207,   209,   210,   484,   188,
     206,   186,   187,   211,   141,     0,   916,   257,     0,   916,
       0,   542,  1019,  1018,     0,   339,     0,     0,     0,     0,
       0,     0,   260,   228,   781,   921,   339,     0,   870,   871,
     872,   873,   881,   882,   899,     0,   858,     0,   895,   608,
     634,   634,   608,   634,     0,   869,   903,   634,     0,   857,
     906,   908,   910,     0,  1007,     0,   921,     0,     0,     0,
     205,   295,   585,   153,     0,   470,   268,   270,  1002,     0,
       0,     0,   205,     0,     0,     0,     0,     0,   282,     0,
    1118,     0,     0,  1104,     0,  1110,  1108,  1075,   857,  1081,
       0,  1082,     0,     0,   800,   857,   855,     0,     0,   858,
       0,     0,   844,   858,     0,     0,     0,     0,   858,     0,
       0,   810,   845,   846,  1026,     0,   858,   813,   815,   814,
       0,     0,   811,   812,   816,   818,   817,   834,   835,   832,
     833,   836,   837,   838,   839,   840,   825,   826,   820,   821,
     819,   822,   823,   824,   827,  1116,     0,   140,     0,     0,
       0,     0,   120,    23,   354,     0,     0,     0,  1095,  1100,
       0,   434,  1004,  1002,   476,   481,   489,     0,     0,    15,
       0,   434,   668,     0,     0,   670,   663,     0,   666,     0,
     662,     0,  1059,     0,     0,     0,     0,   541,     0,   487,
    1091,   582,  1132,     0,   326,   327,     0,     0,   321,     0,
       0,     0,   346,   347,   345,  1090,     0,   362,     0,   916,
       0,   339,     0,   974,   362,  1055,   362,  1058,     0,     0,
       0,   492,     0,     0,   884,   857,   894,   875,     0,     0,
     858,     0,     0,   893,   858,     0,     0,     0,   868,     0,
       0,   858,     0,   879,   900,  1011,   362,     0,   140,     0,
     291,   277,     0,     0,     0,   267,   173,   281,     0,     0,
     284,     0,   289,   290,   140,   283,  1119,  1105,     0,     0,
    1078,  1077,     0,     0,  1130,   863,   862,   799,   612,   857,
     603,     0,   615,   857,   631,   624,   627,   618,     0,   857,
     594,   801,   621,     0,   636,   857,  1021,   842,     0,     0,
     205,    24,    25,    26,    27,  1097,  1092,  1093,  1096,   264,
       0,     0,     0,   441,   432,     0,     0,     0,   239,   361,
       0,     0,   431,     0,     0,     0,  1055,   434,     0,   601,
    1035,   358,   245,   658,     0,   661,     0,   588,   576,   484,
     138,   205,     0,     0,   330,   319,     0,   322,   329,   339,
     339,   335,   568,  1090,   434,  1090,     0,  1017,     0,   973,
     434,     0,   434,  1060,   362,   916,   970,   898,   897,   883,
     613,   857,   607,     0,   616,   857,   633,   625,   628,   619,
       0,   885,   857,   902,   622,   434,   140,   205,   149,   154,
     175,   271,   205,   279,   285,   140,   287,     0,  1106,  1076,
    1080,     0,     0,     0,   606,   843,   591,     0,  1025,  1024,
     841,   140,   218,  1099,     0,     0,     0,  1063,     0,     0,
       0,   265,     0,  1055,     0,   397,   393,   399,   759,    36,
       0,   387,     0,   392,   396,   409,     0,   407,   412,     0,
     411,     0,   410,   451,     0,   222,     0,     0,   365,     0,
     366,   367,     0,     0,  1001,     0,   659,   657,   669,   667,
       0,   331,   332,     0,     0,   317,   328,     0,     0,     0,
    1090,  1084,   235,   568,  1090,   975,   241,   358,   247,   434,
       0,     0,     0,   610,   892,   905,     0,   243,   293,   205,
     205,   140,   274,   174,   286,  1107,  1129,   861,     0,     0,
       0,   205,     0,     0,   459,     0,  1064,     0,   377,   381,
     456,   457,   391,     0,     0,     0,   372,   718,   719,   717,
     720,   721,   738,   740,   739,   709,   681,   679,   680,   699,
     714,   715,   675,   686,   687,   689,   688,   756,   708,   692,
     690,   691,   693,   694,   695,   696,   697,   698,   700,   701,
     702,   703,   704,   705,   707,   706,   676,   677,   678,   682,
     683,   685,   755,   723,   724,   728,   729,   730,   731,   732,
     733,   716,   735,   725,   726,   727,   710,   711,   712,   713,
     736,   737,   741,   743,   742,   744,   745,   722,   747,   746,
     749,   751,   750,   684,   754,   752,   753,   748,   734,   674,
     404,   671,     0,   373,   425,   426,   424,   417,     0,   418,
     374,     0,     0,   363,     0,     0,     0,   455,     0,   222,
     231,   357,     0,     0,     0,   318,   334,   971,   972,     0,
       0,     0,     0,  1090,  1084,     0,   237,  1090,   896,     0,
       0,   140,   272,   155,   176,   205,   605,   590,  1023,   216,
     375,   376,   454,   266,     0,   858,   858,     0,   400,   388,
       0,     0,     0,   406,   408,     0,     0,   413,   420,   421,
     419,   452,   449,  1065,     0,   364,     0,   458,     0,   359,
       0,   333,     0,   653,   860,   136,   860,  1086,     0,   427,
     136,   225,     0,     0,   233,     0,   609,   904,   205,     0,
     178,   378,   126,     0,   379,   380,     0,   857,     0,   857,
     402,   398,   403,   672,   673,     0,   389,   422,   423,   415,
     416,   414,     0,  1103,   368,   453,     0,   360,   654,   859,
       0,   205,   859,  1085,     0,  1089,   205,   136,   227,   229,
       0,   275,     0,   220,     0,   434,     0,   394,   401,   405,
     450,     0,   916,   370,     0,   651,   567,   570,  1087,  1088,
     428,   205,   273,     0,     0,   179,   385,     0,   433,   395,
    1066,     0,   860,   445,   916,   652,   572,     0,   219,     0,
       0,   384,  1090,   916,   302,  1132,   448,   447,   446,  1132,
     444,     0,     0,     0,   383,  1084,   445,     0,     0,  1090,
       0,   382,     0,  1132,  1132,   308,     0,   307,   305,  1084,
     140,   429,   136,   369,     0,     0,   309,     0,     0,   303,
       0,   205,   205,   313,     0,   312,   301,     0,   304,   311,
     371,   215,   430,   314,     0,     0,   299,   310,     0,   300,
     316,   315
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1719, -1719, -1719,  -599, -1719, -1719, -1719,   552,  -449,   -47,
     574, -1719,  -277,  -502, -1719, -1719,   420,  1434,  1927, -1719,
    2249, -1719,  -789, -1719,  -539, -1719,  -703,    11, -1719, -1719,
   -1719, -1719, -1719, -1719, -1719, -1719,  -929, -1719, -1719,  -925,
    -321, -1719, -1719, -1719,  -409, -1719, -1719,  -169,   173,    39,
   -1719, -1719, -1719, -1719, -1719, -1719,    44, -1719, -1719, -1719,
   -1719, -1719, -1719,    63, -1719, -1719,  1116,  1138,  1140,   -73,
    -753,  -946,   575,   652,  -420,   296, -1017, -1719,  -127, -1719,
   -1719, -1719, -1719,  -783,   107, -1719, -1719, -1719, -1719,  -408,
   -1719,  -645, -1719,   376,  -436, -1719, -1719,  1015, -1719,   -99,
   -1719, -1719, -1104, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719,  -135, -1719,   -45, -1719, -1719, -1719, -1719, -1719,  -219,
   -1719,    67, -1036, -1719, -1291,  -432, -1719,  -138,   144,  -118,
    -407, -1719,   -56, -1719, -1719, -1719,    86,   -93,  -107,   -17,
    -781,   -76, -1719, -1719,    29, -1719,   102,  -364, -1719,     9,
      -5,   -86,   -77,   -80, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719, -1719, -1719,  -620,  -913, -1719, -1719, -1719, -1719, -1719,
     488,  1265, -1719,   518, -1719,   364, -1719, -1719, -1719, -1719,
   -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719, -1719, -1719, -1719, -1719,   211,  -377,  -476, -1719, -1719,
   -1719, -1719, -1719,   446, -1719, -1719, -1719, -1719, -1719, -1719,
   -1719, -1719, -1027,  -368,  2891,    34, -1719,   193,  -439, -1719,
   -1719,  -518,  3539,  3962, -1719,   380, -1719, -1719,   526,  -522,
    -643, -1719, -1719,   611,   374,   106, -1719,   375, -1719, -1719,
   -1719, -1719, -1719,   584, -1719, -1719, -1719,   244,  -943,  -145,
    -425,  -416, -1719,   -94,  -136, -1719, -1719,    35,    38,   680,
     -83, -1719, -1719,   528,   -81, -1719,  -357,   162,    93, -1719,
    -427, -1719, -1719, -1719,  -486,  1288, -1719, -1719, -1719, -1719,
   -1719,   756,   494, -1719, -1719, -1719,  -356,  -723, -1719,  1240,
   -1355,  -254,   -58,  -150,   819, -1719, -1719, -1719, -1718, -1719,
    -307, -1059, -1349,  -295,   127, -1719,   500,   568, -1719, -1719,
   -1719, -1719,   525, -1719,  3392,  -827
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   116,   977,   673,   193,   353,   788,
     373,   374,   375,   376,   928,   929,   930,   118,   119,   120,
     121,   122,   999,  1242,   433,  1031,   708,   709,   581,   269,
    1759,   587,  1662,  1760,  2013,   913,   124,   125,   729,   730,
     738,   366,   610,  1970,  1195,  1417,  2035,   455,   194,   710,
    1034,  1280,  1490,   128,   676,  1053,   711,   744,  1057,   648,
    1052,   248,   562,   712,   677,  1054,   457,   393,   415,   131,
    1036,   980,   953,  1215,  1690,  1340,  1119,  1913,  1763,   862,
    1125,   586,   871,  1127,  1534,   854,  1108,  1111,  1329,  2041,
    2042,   698,   699,  1015,   725,   726,   380,   381,   383,  1725,
    1891,  1892,  1431,  1589,  2022,  2044,  1924,  1974,  1975,  1976,
    1700,  1701,  1702,  1703,  1926,  1927,  1933,  1986,  1706,  1707,
    1711,  1877,  1878,  1879,  1961,  2083,  1590,  1591,   195,   133,
    2059,  2060,  1714,  1593,  1594,  1595,  1596,   134,   135,   656,
     583,   136,   137,   138,   139,   140,   141,   142,   143,   262,
     144,   145,   146,  1740,   147,  1033,  1279,   148,   695,   696,
     697,   266,   425,   577,   683,   684,  1378,   685,  1379,   149,
     150,   654,   655,  1368,  1369,  1499,  1500,   151,   897,  1085,
     152,   898,  1086,   153,   899,  1087,   154,   900,  1088,   155,
     901,  1089,   156,   902,  1090,   657,  1371,  1502,   157,   903,
     158,   159,  1954,   160,   678,  1727,   679,  1231,   986,  1450,
    1446,  1870,  1871,   161,   162,   163,   251,   164,   252,   263,
     436,   569,   165,  1372,  1373,   907,   908,   166,  1150,   561,
     625,  1151,  1093,  1302,  1094,  1503,  1504,  1305,  1306,  1096,
    1510,  1511,  1097,   832,   552,   207,   208,   713,   701,   534,
    1252,  1253,   820,   821,   465,   168,   254,   169,   170,   197,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     747,   182,   258,   259,   651,   242,   243,   783,   784,  1385,
    1386,   408,   409,   971,   183,   639,   184,   694,   185,   356,
    1893,  1944,   394,   444,   719,   720,  1140,  1141,  1902,  1956,
    1957,  1246,  1428,   949,  1429,   950,   951,   877,   878,   879,
     357,   358,   910,   596,  1004,  1005
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     196,   198,   427,   200,   201,   202,   203,   205,   206,   354,
     209,   210,   211,   212,   462,   123,   232,   233,   234,   235,
     236,   237,   238,   239,   241,   544,   260,   267,   432,   515,
     834,   449,   430,   428,  1032,   853,   535,   536,   450,   268,
     416,   451,   265,   127,   841,   420,   421,   276,   129,   279,
     686,   998,   364,  1143,   367,   270,  1002,   827,  1247,   787,
     274,   688,   690,   778,   250,   255,  1112,   130,   256,   402,
    1244,   911,  1056,   458,   976,   997,  1236,   462,   780,   781,
    1578,   268,  1115,  1019,   733,   427,   362,  1129,  1102,  1777,
    1278,   615,   617,   619,   514,   622,  1265,   566,  1270,  1336,
    1935,   823,   824,   446,   429,   -78,   578,   631,  1289,   818,
     -78,   432,   825,   570,   -43,   430,   554,   867,   819,   -43,
     869,   927,   932,   571,   634,   578,  1441,  1936,  1717,   739,
     740,   741,   846,   961,   963,   -42,   982,  1719,  -390,   563,
     -42,  1785,   432,  1872,  1882,   849,   555,  1882,   132,   938,
    1777,    14,    14,   382,   850,  1532,  1325,   578,   955,   363,
    1021,   955,   990,   955,  1345,  1346,   955,   955,  1465,   564,
    -460,   611,  1732,    14, -1070,  1603,  1248,   126,  -761,    14,
    1249,  1453,  -953,  1244,  2086,  1613,  1963,   429,  2076,   532,
     533,     3,   257,   378,   547,   443,  2103,  1010,  1952,  1513,
     463,  -110,   553,   442,  1161,  -109,  2094,  1190,  -579,  -639,
     502,    14,   227,   227, -1070,  -962,  -770,  -110,   429,   947,
     948,  -109,   503,   199,  2024,  1250,  1688,   261,   443,  1448,
    1614,   264,  -762,  2077,  -952,   612,   532,   533,   674,   675,
    -647,   429,  -995,  1953,  1162,  1377,  -647,   539,   167,   983,
    1382,  2095,  -964,  1051,   582,  -955,   573,  -954,   663,   573,
    1348,  1466,  1048,  1449,   984,  1733,   268,   584,   539,  2025,
     405,   422,  -998,  -951,  -768,   542,   575,  2087,  -950,   595,
     580,  1778,  1779,   641,   985,  1205,  -957,  -997,   417,  2104,
     870,  1937,   745,  -937,  -938,   447,  1243,   -78,   579,   632,
    -324,  -650,  -460,  -324,  1292,   461,   -43,  -647,   868,   645,
     463,   516,   606,   642,  -953,   379,   635,   661,  1251,  1455,
    1718,  1114,  1525,  1615,  1533,  -306,  2078,   -42,  1274,  1720,
    -390,  -859,   464,  1786,  1578,  1873,  1883,  1780,  -961,  1945,
     939,   731,  2071,  1343,  2096,  1347,  1489,  2072,   940,   956,
    1131,  1022,  1067,  -857,  1432,   203,  1137,  1661,  1724,  -859,
    -763,  2090,  1886,  -859,  1887,  -769,  -952,   432,   545,   538,
     944,   735,   640,  1624,  -995,   538,   828,  2004,  1227,   540,
    1630,   627,  1632,   423,   268,   429,  -959,  -955,  1275,  -954,
     424,   241,   653,   268,   268,   653,   268,   743,   462,   354,
     540,   667,  -960,  1509,  -998,  -951,  -648,  -963,  1938,  1243,
    -950,   271,  1655,  1989,   268,  -966,  1622,   272,  -957,  -997,
    1463,   205,  1201,  1202,   273,  -937,  -938,  1939,  1213,   714,
    1940,  1314,  1990,   227,   920,  1991,   538,   463,   365,   737,
     727,   799,  1611,   734,   628,   214,    40,   627,   214,    40,
     388,   403,   384,   416,   794,   795,   458,   389,   746,   748,
    -649,   385,  1345,  1346,   532,   533,  1426,  1427,  1218,   749,
     750,   751,   752,   754,   755,   756,   757,   758,   759,   760,
     761,   762,   763,   764,   765,   766,   767,   768,   769,   770,
     771,   772,   773,   774,   775,   776,   777,   921,   779,  1315,
     746,   746,   782,   532,   533,  1440,   390,   225,   225,   403,
     442,   689,   802,   803,   804,   805,   806,   807,   808,   809,
     810,   811,   812,   813,   814,   391,   532,   533,   406,   407,
    1749,  1542,   727,   727,   746,   826,   250,   255,   395,   920,
     256,   802,   396,   787,   830,   397,   800,   230,   230,  1442,
     732,   386,  -764,   838,  1522,   840,   117,  1006,  1535,  1007,
    1930,   387,  1443,   727,  1741,   398,  1743,   541,   399,   464,
     112,   857,   227,   858,  1320,   515,   662,   132,  1931,   400,
    -924,   227,  1255,  1444,  1342,   664,   406,   407,   227,  -964,
     403,  1256,   861,   401,   532,   533,  -924,  1932,   669,   417,
     227,   790,   442,   418,   987,   277,   126,  -927,   352,  2005,
     658,   687,   660,   947,   948,  1286,  1013,  1014,  1359,   801,
    1294,   686,  1362,  -927,   934,   392,  1461,   790,  1058,  1366,
     691,  1476,   688,   690,  1478,  1612,   419,  1196,  1050,  1197,
     514,  1198,   532,   533,   434,  1200,  1677,   441,   414,   790,
     392,   442,   452,   873,  -126,   392,   392,  1267,  -126,  1267,
     790,   975,   445,   790,   257,   403,  -925,   406,   407,   700,
     789,  1062,  2008,   404,  2009,  -126,   429,   448,   856,   453,
    1255,  1900,  -925,   392,   171,  1904,   454,  1038,   797,  1256,
    1006,  1007,  1186,  1187,  1188,  -640,   822,  1103,  1105,   229,
     231,  1269,   927,   213,  1271,  1272,  1104,  -641,  1189,   403,
    -642,    55,  1383,   466,   874,   717,   467,   435,   789,  1016,
     459,   187,   188,    65,    66,    67,    50,   403,   225,   845,
     403,   468,   851,  1109,  1110,   669,   227,   403,   669,  1756,
     469,   405,   406,   407,   470,   438,   566,  -645,  1041,   459,
     187,   188,    65,    66,    67,   471,  1344,  1345,  1346,  1191,
     551,   403,  1631,   217,   218,   219,   220,   221,   230,   669,
     459,   187,   188,    65,    66,    67,  1491,   472,  1374, -1070,
    1376,  1049,  1380,  -643,  1496,   190,   406,   407,    91,   431,
     686,    93,    94,   460,    95,   191,    97,   473,   875,  -644,
     443,   688,   690,   670,   406,   407,   505,   406,   407,  1327,
    1328,  1061,  1426,  1427,   406,   407, -1070,   117,   506, -1070,
     108,   117,   460,   507,  2055,   585,  1529,  1345,  1346,  2051,
    1684,  1685,  1471,  1608,  1626,  1505,   508,  1507,   406,   407,
     537,  1512,  1107,   460,  1962,  -958,   582,  1550,  1965,  1482,
    -646,  1554,  1524,  -762,   716,  2073,  1560,   516,   268,   543,
    1492,  2056,  2057,  2058,  1566,  1959,  1960,   225,  1113,   437,
     439,   440,   431,  1722,  2081,  2082,   225,   410,  1570,  1124,
    1267,   132,   548,   225,   550,   459,    63,    64,    65,    66,
      67,   503,  2056,  2057,  2058,   225,    72,   509,   665,  1987,
    1988,   443,   671,   431,  1983,  1984,   556,   230,  1032,  -962,
     126,   605,  1254,  1257,   538,   227,   230,   560,   644,  -760,
     557,   559,   567,   230,   568,   576,   565,   589, -1115,   665,
     597,   671,   665,   671,   671,   230,   600,  1095,   601,   511,
     700,   607,   614,   616,   618,   171,   621,  1598,   686,   171,
     608,   624,   623,  1222,   626,  1223,   636,   858,   460,   688,
     690,   633,   496,   497,   498,   499,   500,   501,  1225,   502,
    1781,   637,   499,   500,   501,   646,   502,   647,  1642,  1657,
     692,   503,  1646,  1235,   693,   117,   702,   123,   503,  1653,
    1750,   227,   703,  2065,   718,  1666,   704,   132,   352,   706,
    1293,   715,  -131,   737,    55,   742,   831,   392,   833,   664,
    2079,   835,   836,  1263,   842,   127,   734,   843,   734,   859,
     129,   578,   863,   802,   866,   595,   126,   880,   881,   912,
     227,   225,   227,   914,   915,   937,   916,  1281,   917,   130,
    1282,   919,  1283,   918,   923,  1628,   727,   922,  1183,  1184,
    1185,  1186,  1187,  1188,   941,   132,   942,   945,   227,   630,
     605,   392,   792,   392,   392,   392,   392,  1189,   638,   946,
     643,   230,  1244,   952,   954,   650,   957,  1244,   959,  2043,
     960,   962,   689,   964,   126,   790,   817,   668,   965,   966,
     733,   967,   973,   250,   255,   978,  1324,   256,   979,   790,
     790,  2043,  1244,   981,  1469,  -785,   988,  1470,   605,   989,
    2066,   991,   992,   171,   993,   996,  1000,  1758,  1330,  1001,
     132,  1331,   848,  1266,  1009,  1266,  1764,  1017,   736,  1011,
     801,   227,  1687,   117,  1018,  1020,   739,   740,   741,  1023,
    1024,  1025,  1771,   132,  1026,  1434,  1027,   227,   227,   126,
    1035,  1456,  1039,   909,  1237,  1043,  1047,  1044,  1457,  1046,
    1055,  1458,  1063,  1244,  1064,  1065,  2001,  -766,   822,   822,
    1037,  2006,   126,  1106,  1130,   132,  1304,  1116,  1126,   933,
     718,  1128,   687,  1737,  1738,  1134,  1135,  1136,  1138,  1152,
    1153,   686,  1154,  1155,  1156,  1158,   790,  1157,   790,  1159,
    1204,  1208,   688,   690,   126,  1194,  1212,  1206,  1210,  1211,
     225,  1092,  1221,  1217,   970,   972,  1436,  1224,  2031,  1230,
     167,   257,  1915,   650,  1232,  1233,  1234,  1447,  2067,  1238,
    1240,  1245,  2068,  1259,  1276,  1285,  1288,  1291,   123,   734,
    1776,  1296,  -965,  1687,  1297,   700,  2084,  2085,  1307,  1308,
     230,   689,  1309,  1310,   746,  1311,   132,  1474,   132,  1312,
    1313,   171,  1319,  1316,  1317,   851,   127,   851,  1687,  1318,
    1687,   129,  1339,  1321,  1338,  1341,  1687,   700,  1333,  1335,
     727,  1350,  1351,  1358,   686,   126,   225,   126,  1352,   117,
     130,   727,  1436,  2092,  1360,   688,   690,   392,  1243,  1364,
    1189,  1365,  1416,  1243, -1131,  1361,  1418,  1419,  1420,   227,
     227,  2000,  1421,  2003,  1423,  1430,  1433,  1451,   582,  1452,
    1464,  1467,   535,  1435,  1468,   225,   230,   225,  1243,  1495,
     268,  1051,  1487,  1475,  1074,  1479,  1517,  1477,  1481,  1484,
    1531,  1520,  1483,  1486,  1493,  1508,  1266,   427,  1518,  1494,
    1516,   687,  1519,   225,  1521,  1526,  1536,  1530,  1539,  1543,
    1544,  1547,  1548,  1549,  1553,   230,  1552,   230,  1555,  1559,
    1556,   132,  1968,   432,  1557,  1561,  1558,   430,  1565,  1564,
    1562,  1569,  1616,  1084,  1617,  1098,  1641,  1600,  1643,  1243,
    1601,  1619,  1645,   230,  1571,  1572,  1573,  1650,  1652,  2054,
     126,  1574,  1012,  1978,  1980,   117,  1610,  1620,  1635,   689,
    1623,  1621,  1625,  1629,  1304,  1501,  1633,   171,  1501,  1122,
     117,  1640,  1636,  1634,  1639,  1514,   225,    34,    35,    36,
    1644,  1647,  1599,  1648,  1651,  1654,  1649,  1656,  1667,  1604,
    1658,   215,   225,   225,  1606,  1659,  1607,  1660,  1670,   734,
    1092,  1681,  1663,   227,  1692,  1664,  1687,  1705,  1713,  1723,
    1726,  1721,  1618,   117,  1731,  1734,   230,  1735,  1744,  1739,
    1745,   167,  1199,   718,   462,  1627,   727,  1751,  1060,  1747,
    1766,  1769,   230,   230,  1775,  1783,   132,  1784,  1880,  1881,
    1888,  1894,  1895,  1899,   377,  1897,    81,    82,    83,    84,
      85,  1901,  1909,  1214,  1898,  1907,  1942,   222,  1910,   687,
    1920,  1921,  1949,    89,    90,   126,   227,  1099,  1946,  1100,
     700,  1950,   412,   700,  1955,   413,  1981,    99,   117,  1977,
    1979,   227,   227,   171,  1985,  1993,  1992,  1994,  1998,  1999,
    2002,  2091,   105,  2011,   605,  1120,  1884,  2007,   171,  2012,
    -386,   117,  2014,  2015,  2019,  2017,   732,  2023,   817,   817,
   -1133, -1133, -1133, -1133, -1133,   494,   495,   496,   497,   498,
     499,   500,   501,  1936,   502,  1592,  1298,  1299,  1300,   213,
    2032,  2034,  2026,   117,  2033,  1592,   503,  2039,  2040,  2045,
    2049,   171,  2052,  2062,  2053,  2064,  2069,  2070,  2075,  1637,
    2080,  1638,    50,  2088,   225,   225,  1730,  2089,  2093,  2097,
     392,  1736,  2098,  2105,   727,   727,  2106,  2108,  1209,  1580,
    1301,  1301,  1084,  2109,  1422,  2048,   227,  1287,  1774,  2063,
     796,  1229,  1523,  1914,   650,  1220,  2061,  1473,  1665,   217,
     218,   219,   220,   221,   230,   230,   935,   791,  1905,  1782,
    1929,   793,   689,  1934,  1712,   848,   171,   848,  2100,  2074,
    1885,    14,   117,  1903,   117,   659,   117,    93,    94,  1693,
      95,   191,    97,  1762,  1375,  1597,  1506,  1445,  1367,   171,
    1303,  1497,  1322,  1498,   652,  1597,   728,  1354,  1092,  1092,
    1092,  1092,  1092,  1092,  1996,  2028,   108,  1092,  2021,  1092,
    1144,  1268,  1683,  1268,  1356,   459,    63,    64,    65,    66,
      67,   171,  1425,   605,   132,  1415,    72,   509,     0,     0,
    1948,     0,     0,   700,     0,     0,  1581,     0,     0,  1896,
       0,     0,  1582,     0,   459,  1583,   188,    65,    66,    67,
    1584,   516,   909,   126,     0,   689,     0,  1752,   225,  1753,
       0,  1754,   687,     0,     0,   132,  1755,   510,     0,   511,
       0,     0,     0,     0,  1715,     0,     0,     0,  1592,     0,
    1912,  1762,  1459,   512,  1592,   513,  1592,     0,   460,   117,
       0,     0,  1585,  1586,   126,  1587,     0,     0,   230,     0,
     171,     0,   171,     0,   171,     0,  1120,  1337,     0,  1592,
       0,   132,   377,   377,   377,   620,   377,   460,     0,     0,
     132,   225,     0,     0,     0,     0,  1588,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   225,   225,     0,     0,
     126,     0,     0,     0,     0,     0,     0,  1943,     0,   126,
       0,     0,     0,     0,   672,   687,     0,     0,     0,     0,
       0,   230,     0,     0,     0,     0,     0,  1908,     0,     0,
    1084,  1084,  1084,  1084,  1084,  1084,   230,   230,  1597,  1084,
       0,  1084,     0,  1092,  1597,  1092,  1597,  2037,     0,   700,
       0,     0,   117,     0,     0,     0,     0,     0,     0,  1951,
       0,     0,     0,  1592,   117,     0,  1889,     0,     0,  1597,
       0,  1943,   132,     0,  1538,     0,     0,   171,   132,     0,
       0,     0,     0,     0,     0,   132,     0,     0,     0,     0,
     462,   225,     0,     0,  1268,     0,     0,     0,     0,     0,
       0,   126,     0,     0,     0,     0,     0,   126,     0,     0,
    1472,     0,     0,     0,   126,     0,   224,   224,     0,     0,
       0,     0,     0,     0,  1580,     0,     0,   247,     0,     0,
       0,   230,     0,   546,   518,   519,   520,   521,   522,   523,
     524,   525,   526,   527,   528,   529,     0,  1575,     0,     0,
       0,     0,     0,   247,     0,     0,     0,     0,     0,  1580,
       0,     0,     0,  1597,     0,     0,    14,     0,     0,     0,
       0,     0,     0,  1515,     0,     0,     0,     0,   530,   531,
     171,     0,     0,     0,     0,  1966,  1967,     0,   650,  1120,
       0,  1092,   171,  1092,     0,  1092,     0,     0,     0,     0,
    1092,    14,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1084,     0,  1084,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   931,   931,   132,
       0,  1581,     0,     0,     0,     0,     0,  1582,     0,   459,
    1583,   188,    65,    66,    67,  1584,     0,     0,     0,     0,
       0,     0,     0,  2099,     0,   532,   533,     0,   126,     0,
       0,     0,  2107,     0,     0,     0,  1581,     0,     0,     0,
    2110,     0,  1582,  2111,   459,  1583,   188,    65,    66,    67,
    1584,     0,   132,   650,     0,     0,     0,  1585,  1586,     0,
    1587,     0,   117,     0,     0,     0,     0,     0,     0,     0,
       0,  1092,     0,   352,  1609,     0,     0,     0,     0,  1710,
       0,   126,   460,     0,     0,   132,     0,     0,     0,   705,
     132,  1602,  1585,  1586,     0,  1587,     0,     0,     0,  2038,
       0,     0,     0,   117,     0,     0,  1580,   224,     0,     0,
       0,     0,     0,     0,   126,   132,     0,   460,     0,   126,
       0,     0,     0,     0,     0,     0,  1742,     0,     0,     0,
       0,     0,     0,  1084,     0,  1084,     0,  1084,     0,     0,
       0,     0,  1084,     0,   126,     0,     0,     0,    14,   117,
       0,     0,     0,     0,   117,     0,     0,   247,   117,   247,
   -1133, -1133, -1133, -1133, -1133,  1181,  1182,  1183,  1184,  1185,
    1186,  1187,  1188,     0,     0,   132,   132,     0,   392,     0,
       0,   605,     0,     0,   352,     0,  1189,     0,     0,     0,
     171,     0,     0,     0,  1869,     0,     0,     0,     0,  1580,
       0,  1876,     0,     0,   126,   126,   700,     0,     0,   352,
       0,   352,     0,  1581,     0,     0,     0,   352,   247,  1582,
       0,   459,  1583,   188,    65,    66,    67,  1584,   700,  1092,
    1092,   171,     0,     0,     0,     0,     0,   700,     0,     0,
       0,    14,     0,  1084,     0,   355,   224,     0,     0,     0,
     117,   117,   117,     0,     0,   224,   117,     0,     0,     0,
       0,     0,   224,   117,     0,     0,     0,     0,     0,  1585,
    1586,     0,  1587,     0,   224,     0,     0,   171,     0,     0,
       0,     0,   171,     0,     0,   224,   171,     0,   931,     0,
     931,     0,   931,     0,   460,     0,   931,     0,   931,   931,
    1203,     0,     0,  1746,     0,     0,  1581,     0,     0,     0,
     247,     0,  1582,   247,   459,  1583,   188,    65,    66,    67,
    1584,     0,     0,     0,     0,     0,     0,     0,  1580,   546,
     518,   519,   520,   521,   522,   523,   524,   525,   526,   527,
     528,   529,     0,   517,   518,   519,   520,   521,   522,   523,
     524,   525,   526,   527,   528,   529,  1580,     0,     0,     0,
       0,     0,  1585,  1586,     0,  1587,     0,     0,     0,   247,
      14,     0,     0,     0,   530,   531,   605,     0,   171,   171,
     171,     0,     0,     0,   171,     0,     0,   460,   530,   531,
       0,   171,     0,     0,   213,     0,  1748,   352,    14,     0,
       0,  1084,  1084,     0,     0,     0,     0,   117,     0,     0,
     224,     0,     0,     0,     0,     0,  1972,    50,     0,     0,
       0,     0,     0,  1869,  1869,     0,     0,  1876,  1876,     0,
       0,     0,     0,     0,     0,  1581,     0,     0,     0,     0,
     605,  1582,     0,   459,  1583,   188,    65,    66,    67,  1584,
       0,   532,   533,     0,   217,   218,   219,   220,   221,     0,
     117,     0,   247,  1581,   247,   532,   533,   896,     0,  1582,
       0,   459,  1583,   188,    65,    66,    67,  1584,     0,   355,
    1874,   355,    93,    94,  1875,    95,   191,    97,     0,     0,
       0,  1585,  1586,   117,  1587,     0,     0,     0,   117,     0,
     896,     0,     0,     0,     0,     0,  2036,     0,     0,     0,
       0,   108,  1709,     0,     0,   844,   460,     0,     0,  1585,
    1586,     0,  1587,   117,     0,  1757,     0,     0,     0,  2050,
       0,     0,     0,     0,     0,   171,     0,     0,     0,     0,
     355,     0,     0,     0,   460,     0,     0,     0,     0,     0,
       0,     0,     0,  1906,     0,     0,     0,     0,     0,   247,
     247,     0,     0,     0,     0,     0,     0,     0,   247,     0,
     474,   475,   476,     0,     0,     0,     0,     0,   931,     0,
       0,     0,     0,   117,   117,     0,     0,     0,   171,   224,
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,     0,     0,
       0,   171,     0,     0,     0,     0,   171,     0,     0,   503,
       0,     0,   355,     0,     0,   355,     0,     0,     0,     0,
       0,     0,     0,   474,   475,   476,     0,     0,     0,     0,
       0,   171,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   477,   478,   224,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,     0,     0,     0,     0,     0,     0,     0,   247,     0,
       0,     0,   503,     0,   224,     0,   224,     0,     0,     0,
       0,   171,   171,     0,     0,   546,   518,   519,   520,   521,
     522,   523,   524,   525,   526,   527,   528,   529,     0,     0,
       0,     0,   224,   896,     0,     0,     0,     0,     0,     0,
     247,     0,     0,     0,     0,     0,     0,   247,   247,   896,
     896,   896,   896,   896,   873,     0,     0,     0,     0,     0,
     530,   531,   896,     0,  1438,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   247,
       0,     0,     0,     0,   355,     0,   876,  1028,   518,   519,
     520,   521,   522,   523,   524,   525,   526,   527,   528,   529,
       0,     0,     0,     0,   213,   224,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   874,     0,     0,     0,   247,
       0,   224,   224,     0,     0,     0,     0,    50,     0,     0,
       0,     0,   530,   531,   504,     0,  1040,   532,   533,     0,
       0,     0,     0,     0,     0,   247,   247,     0,     0,     0,
     226,   226,     0,     0,     0,     0,   224,     0,     0,     0,
       0,   249,     0,   247,   217,   218,   219,   220,   221,     0,
     247,     0,     0,     0,     0,     0,   247,     0,     0,     0,
       0,   355,   355,     0,     0,     0,   190,   896,     0,    91,
     355,     0,    93,    94,     0,    95,   191,    97,     0,  1355,
       0,   943,   247,     0,     0,     0,     0,     0,     0,   532,
     533,     0,     0,     0,     0,   474,   475,   476,     0,     0,
       0,   108,   247,     0,     0,     0,   247,     0,     0,     0,
       0,     0,     0,     0,     0,   477,   478,   247,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1029,   503,     0,     0,     0,     0,     0,
       0,     0,     0,   224,   224,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   247,     0,     0,
       0,   247,     0,   247,     0,     0,   247,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   896,
     896,   896,   896,   896,   896,   224,   896,     0,     0,   896,
     896,   896,   896,   896,   896,   896,   896,   896,   896,   896,
     896,   896,   896,   896,   896,   896,   896,   896,   896,   896,
     896,   896,   896,   896,   896,   896,   896,   474,   475,   476,
       0,     0,  1133,     0,     0,     0,     0,     0,     0,   355,
     355,   226,     0,     0,     0,   896,     0,   477,   478,     0,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,   474,   475,   476,   974,     0,
       0,     0,   247,     0,   247,     0,   503,     0,     0,     0,
       0,     0,     0,     0,     0,   477,   478,   224,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,     0,     0,   247,     0,     0,   247,     0,
       0,     0,     0,     0,   503,     0,     0,   355,     0,     0,
       0,     0,     0,     0,     0,   247,   247,   247,   247,   247,
     247,     0,     0,   224,   247,   355,   247,     0,     0,     0,
     224,     0,   355,     0,     0,     0,     0,     0,   355,     0,
       0,     0,     0,     0,     0,   224,   224,     0,   896,     0,
     226,     0,     0,     0,     0,     0,     0,     0,   247,   226,
       0,     0,     0,     0,     0,   247,   226,     0,     0,     0,
     896,     0,   896,     0,     0,     0,     0,     0,   226,     0,
     474,   475,   476,     0,   355,     0,     0,     0,     0,   226,
    1008,     0,     0,     0,     0,     0,     0,   896,     0,     0,
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,     0,     0,
       0,     0,     0,   247,   247,     0,     0,   247,  1040,   503,
     224,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   474,   475,   476,     0,     0,     0,     0,     0,   355,
       0,     0,     0,   355,     0,   876,     0,     0,   355,   247,
       0,   477,   478,   249,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
     247,     0,   247,     0,     0,     0,     0,     0,     0,     0,
     503,     0,     0,     0,   226,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   359,     0,
       0,     0,  1028,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,     0,   247,   247,     0,     0,
     247,     0,     0,     0,     0,     0,   896,     0,   896,     0,
     896,     0,     0,     0,     0,   896,   224,     0,     0,     0,
     896,   904,   896,  1066,   355,   896,   355,   530,   531,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   247,   247,
       0,     0,   247,     0,     0,     0,     0,     0,     0,   247,
       0,     0,     0,     0,   904,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   355,     0,     0,
     355,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,   228,   228,
       0,     0,     0,     0,  1207,     0,     0,     0,   247,   253,
     247,  1189,   247,     0,   532,   533,     0,   247,     0,   224,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   247,     0,     0,     0,     0,   896,
     355,     0,     0,     0,     0,     0,     0,   355,     0,     0,
       0,   247,   247,   226,     0,     0,     0,     0,     0,   247,
       0,   247,     0,     0,     0,     0,     0,  1694,     0,  1163,
    1164,  1165,     0,     0,     0,     0,     0,     0,   705,     0,
       0,     0,     0,     0,   247,     0,   247,     0,     0,     0,
    1166,     0,   247,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,  1187,  1188,   355,   355,   213,   247,     0,
       0,     0,   593,     0,   594,     0,     0,     0,  1189,   226,
       0,     0,     0,     0,     0,   896,   896,   896,     0,     0,
      50,     0,   896,     0,   247,     0,     0,     0,     0,     0,
     247,   355,   247,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1091,  1695,     0,     0,     0,     0,   226,     0,
     226,     0,     0,     0,     0,     0,  1696,   217,   218,   219,
     220,   221,  1697,   599,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   226,   904,     0,   190,
       0,     0,    91,  1698,     0,    93,    94,     0,    95,  1699,
      97,     0,     0,   904,   904,   904,   904,   904,     0,   228,
       0,     0,     0,     0,     0,     0,   904,     0,   355,   355,
       0,     0,   355,     0,   108,     0,     0,     0,     0,     0,
       0,     0,     0,  1193,     0,     0,     0,     0,   896,     0,
       0,     0,     0,     0,     0,   247,     0,     0,     0,  1381,
       0,     0,     0,     0,     0,     0,     0,     0,   247,   226,
     355,     0,   247,     0,     0,   721,   247,   247,   359,     0,
       0,   355,     0,  1216,     0,   226,   226,     0,     0,     0,
       0,   247,     0,     0,     0,     0,     0,   896,   474,   475,
     476,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1216,     0,     0,   896,     0,     0,     0,   896,   477,   478,
     226,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,   247,   502,     0,     0,     0,     0,
       0,   904,     0,     0,     0,     0,   355,   503,   228,   896,
       0,     0,     0,     0,     0,     0,  1277,   228,     0,   247,
       0,   247,     0,     0,   228,   213,     0,     0,     0,     0,
       0,   355,     0,     0,     0,     0,   228,     0,     0,     0,
     249,     0,     0,     0,     0,     0,     0,   253,    50,     0,
     247,  1091,     0,     0,     0,     0,   355,     0,   355,     0,
       0,     0,     0,     0,   355,     0,   247,     0,     0,     0,
       0,     0,   247,     0,     0,     0,   247,   872,     0,     0,
       0,     0,     0,     0,     0,   217,   218,   219,   220,   221,
     247,   247,     0,     0,     0,     0,     0,   226,   226,     0,
       0,     0,     0,     0,     0,     0,     0,   190,     0,     0,
      91,    92,     0,    93,    94,     0,    95,   191,    97,     0,
       0,     0,   355,     0,     0,     0,     0,     0,     0,     0,
       0,   253,     0,   904,   904,   904,   904,   904,   904,   226,
     904,  1284,   108,   904,   904,   904,   904,   904,   904,   904,
     904,   904,   904,   904,   904,   904,   904,   904,   904,   904,
     904,   904,   904,   904,   904,   904,   904,   904,   904,   904,
     904,     0,   228,     0,   994,   995,   474,   475,   476,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   904,
       0,     0,     0,     0,     0,     0,   477,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,     0,     0,     0,   355,     0,   905,
       0,     0,     0,     0,     0,   503,     0,     0,     0,     0,
     355,   226,     0,     0,   355,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   905,  1973,   213,     0,   214,    40,   546,   518,
     519,   520,   521,   522,   523,   524,   525,   526,   527,   528,
     529,     0,     0,     0,     0,     0,     0,    50,     0,  1091,
    1091,  1091,  1091,  1091,  1091,     0,     0,   226,  1091,     0,
    1091,     0,     0,     0,   226,     0,     0,     0,     0,     0,
       0,     0,     0,   530,   531,     0,   355,     0,     0,   226,
     226,     0,   904,     0,   217,   218,   219,   220,   221,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   355,     0,   355,   904,     0,   904,     0,     0,     0,
     815,   228,    93,    94,     0,    95,   191,    97,     0,     0,
       0,     0,  1142,   721,     0,     0,     0,     0,     0,  1295,
       0,   904,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   108,     0,     0,     0,   816,     0,     0,   112,     0,
     532,   533,     0,   213,   355,   968,     0,   969,   355,     0,
       0,     0,     0,     0,     0,   474,   475,   476,     0,     0,
       0,  1579,   355,   355,   226,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,   477,   478,   228,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,   217,   218,   219,   220,   221,     0,     0,
    1228,     0,     0,     0,   503,     0,   228,     0,   228,     0,
       0,     0,     0,     0,  1091,     0,  1091,     0,  1239,     0,
       0,    93,    94,     0,    95,   191,    97,     0,     0,     0,
       0,  1258,     0,     0,   228,   905,     0,  1028,   518,   519,
     520,   521,   522,   523,   524,   525,   526,   527,   528,   529,
     108,   905,   905,   905,   905,   905,     0,     0,   213,     0,
     214,    40,     0,     0,   905,     0,     0,     0,     0,     0,
     904,     0,   904,     0,   904,     0,     0,  1290,     0,   904,
     226,    50,   530,   531,   904,     0,   904,     0,     0,   904,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1691,     0,     0,  1704,   228,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   217,   218,
     219,   220,   221,   228,   228,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1326,     0,
       0,     0,     0,     0,   815,     0,    93,    94,     0,    95,
     191,    97,  1349,     0,     0,     0,  1353,     0,   253,   532,
     533,  1357,  1091,     0,  1091,     0,  1091,     0,     0,     0,
       0,  1091,     0,   226,     0,   108,     0,     0,     0,   847,
       0,     0,   112,     0,     0,     0,     0,     0,     0,   905,
       0,     0,   906,   904,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1772,  1773,     0,     0,     0,
     474,   475,   476,     0,     0,  1704,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   936,     0,     0,   253,     0,
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1460,     0,   503,
       0,     0,  1091,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   228,   228,     0,     0,   904,
     904,   904,     0,     0,     0,     0,   904,     0,  1923,     0,
       0,     0,     0,     0,     0,     0,  1704,   213,     0,     0,
    1485,     0,     0,  1488,     0,     0,     0,     0,     0,     0,
       0,   905,   905,   905,   905,   905,   905,   253,   905,     0,
      50,   905,   905,   905,   905,   905,   905,   905,   905,   905,
     905,   905,   905,   905,   905,   905,   905,   905,   905,   905,
     905,   905,   905,   905,   905,   905,   905,   905,   905,     0,
       0,     0,     0,     0,     0,     0,     0,   217,   218,   219,
     220,   221,     0,  1537,     0,     0,     0,   905,     0,     0,
    1541,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   410,     0,     0,    93,    94,     0,    95,   191,
      97,     0,   904,  1605,     0,     0,     0,     0,     0,     0,
     290,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   108,     0,     0,     0,   411,   228,
    1091,  1091,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   292,  1576,  1577,
       0,   904,     0,     0,     0,     0,     0,     0,  1121,     0,
     213,     0,     0,     0,     0,     0,     0,   904,     0,     0,
       0,   904,     0,     0,  1145,  1146,  1147,  1148,  1149,     0,
       0,     0,     0,    50,     0,   253,     0,  1160,     0,     0,
       0,  -433,   228,     0,     0,     0,     0,     0,     0,     0,
     459,   187,   188,    65,    66,    67,     0,   228,   228,     0,
     905,     0,     0,   904,     0,     0,     0,     0,     0,   591,
     217,   218,   219,   220,   221,   592,     0,     0,     0,     0,
       0,     0,   905,     0,   905,     0,     0,     0,     0,     0,
       0,     0,   190,     0,     0,    91,   345,     0,    93,    94,
       0,    95,   191,    97,  2047,     0,     0,     0,     0,   905,
       0,  1668,  1669,     0,     0,  1671,   349,     0,     0,     0,
    1691,     0,     0,   460,     0,     0,     0,   108,   351,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   474,   475,   476,     0,     0,
       0,     0,   228,  1689,     0,     0,     0,     0,     0,     0,
       0,     0,  1264,     0,  1716,   477,   478,     0,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,   474,   475,   476,     0,     0,     0,     0,
       0,     0,     0,     0,   503,     0,     0,     0,     0,     0,
       0,     0,     0,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,  1765,
     502,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   503,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1689,     0,     0,     0,   905,     0,
     905,     0,   905,   213,     0,     0,     0,   905,   253,     0,
       0,     0,   905,     0,   905,     0,     0,   905,     0,  1689,
       0,  1689,     0,     0,  1149,  1370,    50,  1689,  1370,     0,
       0,     0,     0,     0,  1384,  1387,  1388,  1389,  1391,  1392,
    1393,  1394,  1395,  1396,  1397,  1398,  1399,  1400,  1401,  1402,
    1403,  1404,  1405,  1406,  1407,  1408,  1409,  1410,  1411,  1412,
    1413,  1414,     0,   217,   218,   219,   220,   221,  1728,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1424,     0,     0,     0,     0,  1925,     0,     0,     0,   456,
       0,    93,    94,     0,    95,   191,    97,     0,     0,     0,
       0,   253,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1729,     0,     0,     0,
     108,   905,   477,   478,  1532,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
       0,     0,     0,     0,     0,     0,   290,     0,     0,   477,
     478,   503,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,   213,   502,     0,     0,     0,
    1947,     0,     0,   292,     0,     0,     0,     0,   503,     0,
       0,     0,     0,  1958,     0,     0,   213,  1689,    50,     0,
       0,     0,  1139,  1527,     0,     0,     0,   905,   905,   905,
       0,     0,     0,     0,   905,     0,     0,     0,     0,    50,
       0,     0,     0,  1928,  1708,  1545,     0,  1546,     0,     0,
       0,     0,     0,     0,     0,   217,   218,   219,   220,   221,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1567,     0,     0,   591,   217,   218,   219,   220,
     221,   592,     0,    93,    94,     0,    95,   191,    97,  2016,
       0,     0,     0,     0,     0,     0,     0,     0,   190,     0,
       0,    91,   345,  1533,    93,    94,     0,    95,   191,    97,
       0, -1132,   108,  1709,  1958,     0,  2029,     0,   474,   475,
     476,     0,   349,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   108,   351,     0,     0,     0,   477,   478,
     905,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,     0,  1068,  1069,
       0,     0,     0,     0,     0,     0,     0,   503,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1070,   905,
       0,     0,     0,     0,     0,     0,  1071,  1072,  1073,   213,
       0,     0,     0,     0,     0,   905,     0,     0,     0,   905,
    1074,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,  1673,     0,  1674,     0,  1675,     0,     0,  2018,     0,
    1676,     0,     0,     0,     0,  1678,     0,  1679,     0,     0,
    1680,   905,     0,     0,     0,     0,     0,     0,  1075,  1076,
    1077,  1078,  1079,  1080,     0,     0,     0,     0,     0,     0,
       5,     6,     7,     8,     9,     0,  1081,     0,     0,     0,
      10,   190,     0,     0,    91,    92,     0,    93,    94,     0,
      95,   191,    97,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,  1082,  1083,     0,     0,   504,
       0,     0,     0,     0,     0,    14,   108,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,  1767,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
      56,    57,    58,     0,    59,  -205,    60,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,   213,    86,     0,     0,    87,     0,     0,     0,     0,
      88,    89,    90,    91,    92,     0,    93,    94,     0,    95,
      96,    97,    98,     0,    50,    99,     0,     0,   100,     0,
    1916,  1917,  1918,     0,   101,   102,   103,  1922,   104,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
       0,     0,   112,   113,   114,   115,     0,     0,     0,     0,
       0,   217,   218,   219,   220,   221,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     5,     6,     7,
       8,     9,     0,   190,     0,     0,    91,    10,     0,    93,
      94,     0,    95,   191,    97,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   108,     0,
       0,     0,    14,  1971,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,  1941,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,     0,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,    56,    57,    58,
       0,    59,  1982,    60,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,    71,    72,    73,  1995,     0,
       0,     0,  1997,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,    88,    89,    90,
      91,    92,     0,    93,    94,     0,    95,    96,    97,    98,
       0,     0,    99,     0,  2020,   100,     0,     0,     0,     0,
       0,   101,   102,   103,     0,   104,     0,   105,   106,   107,
       0,     0,   108,   109,     0,   110,   111,  1226,     0,   112,
     113,   114,   115,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,    56,    57,    58,     0,    59,     0,    60,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,    88,    89,    90,    91,    92,     0,    93,
      94,     0,    95,    96,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,   103,
       0,   104,     0,   105,   106,   107,     0,     0,   108,   109,
       0,   110,   111,  1439,     0,   112,   113,   114,   115,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,     0,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   190,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   191,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   109,     0,   110,   111,   707,
       0,   112,   113,   114,   115,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,     0,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   190,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   191,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   109,     0,   110,   111,  1030,     0,   112,   113,   114,
     115,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,  -205,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   190,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   191,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   109,     0,   110,
     111,     0,     0,   112,   113,   114,   115,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,     0,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   190,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   191,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,     0,     0,     0,   105,   106,   107,
       0,     0,   108,   109,     0,   110,   111,  1192,     0,   112,
     113,   114,   115,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   190,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   191,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
       0,     0,     0,   105,   106,   107,     0,     0,   108,   109,
       0,   110,   111,  1241,     0,   112,   113,   114,   115,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,     0,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   190,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   191,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   109,     0,   110,   111,  1273,
       0,   112,   113,   114,   115,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,     0,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   190,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   191,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   109,     0,   110,   111,  1332,     0,   112,   113,   114,
     115,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,  1334,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   190,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   191,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   109,     0,   110,
     111,     0,     0,   112,   113,   114,   115,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,     0,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,  1528,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   190,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   191,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,     0,     0,     0,   105,   106,   107,
       0,     0,   108,   109,     0,   110,   111,     0,     0,   112,
     113,   114,   115,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   190,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   191,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
       0,     0,     0,   105,   106,   107,     0,     0,   108,   109,
       0,   110,   111,  1682,     0,   112,   113,   114,   115,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,  -297,    34,    35,    36,    37,    38,    39,
      40,     0,    41,     0,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   190,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   191,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   109,     0,   110,   111,     0,
       0,   112,   113,   114,   115,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,     0,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   190,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   191,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   109,     0,   110,   111,  1919,     0,   112,   113,   114,
     115,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,  1969,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   190,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   191,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   109,     0,   110,
     111,     0,     0,   112,   113,   114,   115,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,     0,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,  2010,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   190,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   191,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,     0,     0,     0,   105,   106,   107,
       0,     0,   108,   109,     0,   110,   111,     0,     0,   112,
     113,   114,   115,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   190,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   191,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
       0,     0,     0,   105,   106,   107,     0,     0,   108,   109,
       0,   110,   111,  2027,     0,   112,   113,   114,   115,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,     0,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   190,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   191,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   109,     0,   110,   111,  2030,
       0,   112,   113,   114,   115,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,     0,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   190,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   191,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   109,     0,   110,   111,  2046,     0,   112,   113,   114,
     115,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   190,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   191,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   109,     0,   110,
     111,  2101,     0,   112,   113,   114,   115,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,     0,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   190,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   191,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,     0,     0,     0,   105,   106,   107,
       0,     0,   108,   109,     0,   110,   111,  2102,     0,   112,
     113,   114,   115,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,   574,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,   187,   188,    65,    66,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   190,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   191,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
       0,     0,     0,   105,   106,   107,     0,     0,   108,   109,
       0,   110,   111,     0,     0,   112,   113,   114,   115,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,   860,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,     0,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,     0,    61,    62,   187,   188,
      65,    66,    67,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   190,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   191,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   109,     0,   110,   111,     0,
       0,   112,   113,   114,   115,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,  1123,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,     0,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,     0,    61,    62,   187,   188,    65,    66,    67,     0,
      68,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   190,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   191,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   109,     0,   110,   111,     0,     0,   112,   113,   114,
     115,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
    1761,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
     187,   188,    65,    66,    67,     0,    68,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   190,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   191,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   109,     0,   110,
     111,     0,     0,   112,   113,   114,   115,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,  1911,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,     0,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,     0,    61,    62,   187,   188,    65,    66,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   190,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   191,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,     0,     0,     0,   105,   106,   107,
       0,     0,   108,   109,     0,   110,   111,     0,     0,   112,
     113,   114,   115,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,   187,   188,    65,    66,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   190,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   191,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
       0,     0,     0,   105,   106,   107,     0,     0,   108,   109,
       0,   110,   111,     0,     0,   112,   113,   114,   115,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   360,   426,    13,     0,     0,     0,     0,
       0,     0,     0,     0,   798,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,   186,   187,   188,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   189,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   190,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   191,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   109,     5,     6,     7,     8,
       9,   112,   113,   114,   115,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     360,     0,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,     0,   186,   187,   188,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   189,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   190,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   191,    97,     0,     0,
       0,    99,     0,     0,   100,     5,     6,     7,     8,     9,
     101,   102,     0,     0,     0,    10,   105,   106,   107,     0,
       0,   108,   192,     0,   361,     0,     0,     0,   112,   113,
     114,   115,     0,     0,     0,     0,     0,     0,     0,     0,
     722,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,   723,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   186,   187,   188,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   189,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   190,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   191,    97,     0,   724,     0,
      99,     0,     0,   100,     5,     6,     7,     8,     9,   101,
     102,     0,     0,     0,    10,   105,   106,   107,     0,     0,
     108,   192,     0,     0,     0,     0,     0,   112,   113,   114,
     115,     0,     0,     0,     0,     0,     0,     0,     0,  1260,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,  1261,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,   186,   187,   188,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     189,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   190,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   191,    97,     0,  1262,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     192,     5,     6,     7,     8,     9,   112,   113,   114,   115,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   360,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   186,
     187,   188,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   189,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   190,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   191,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   192,     0,     0,
     855,     0,     0,   112,   113,   114,   115,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   360,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   798,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   186,   187,   188,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   189,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   190,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   191,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,     0,     0,     0,   105,   106,   107,
       0,     0,   108,   192,     5,     6,     7,     8,     9,   112,
     113,   114,   115,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   360,   426,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,   186,   187,   188,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     189,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   190,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   191,    97,     0,     0,     0,    99,
       0,     0,   100,     5,     6,     7,     8,     9,   101,   102,
       0,     0,     0,    10,   105,   106,   107,     0,     0,   108,
     109,     0,     0,     0,     0,     0,   112,   113,   114,   115,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,   204,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,   186,   187,   188,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   189,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   190,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   191,    97,     0,     0,     0,    99,     0,
       0,   100,     5,     6,     7,     8,     9,   101,   102,     0,
       0,     0,    10,   105,   106,   107,     0,     0,   108,   192,
       0,     0,     0,     0,     0,   112,   113,   114,   115,     0,
       0,     0,     0,     0,     0,     0,     0,   240,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,     0,
     186,   187,   188,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   189,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   190,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   191,    97,     0,     0,     0,    99,     0,     0,
     100,     5,     6,     7,     8,     9,   101,   102,     0,     0,
       0,    10,   105,   106,   107,     0,     0,   108,   192,     0,
       0,     0,     0,     0,   112,   113,   114,   115,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   186,
     187,   188,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   189,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   190,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   191,    97,     0,     0,     0,    99,     0,     0,   100,
       5,     6,     7,     8,     9,   101,   102,     0,     0,     0,
      10,   105,   106,   107,     0,     0,   108,   192,     0,   275,
       0,     0,     0,   112,   113,   114,   115,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   186,   187,
     188,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   189,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   192,     0,   278,     0,
       0,     0,   112,   113,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   426,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,     0,   186,   187,   188,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   189,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   190,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   191,    97,     0,     0,
       0,    99,     0,     0,   100,     5,     6,     7,     8,     9,
     101,   102,     0,     0,     0,    10,   105,   106,   107,     0,
       0,   108,   109,     0,     0,     0,     0,     0,   112,   113,
     114,   115,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   186,   187,   188,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   189,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   190,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   191,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   192,   572,     0,     0,     0,     0,   112,   113,   114,
     115,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   360,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   186,
     187,   188,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   189,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   190,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   191,    97,     0,     0,     0,    99,     0,     0,   100,
       5,     6,     7,     8,     9,   101,   102,     0,     0,     0,
      10,   105,   106,   107,     0,     0,   108,   192,     0,     0,
       0,     0,     0,   112,   113,   114,   115,     0,     0,   753,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   186,   187,
     188,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   189,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,     0,     0,     0,    99,     0,     0,   100,     5,
       6,     7,     8,     9,   101,   102,     0,     0,     0,    10,
     105,   106,   107,     0,     0,   108,   192,     0,     0,     0,
       0,     0,   112,   113,   114,   115,     0,     0,     0,     0,
       0,     0,     0,     0,   798,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,   186,   187,   188,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   189,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   190,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   191,
      97,     0,     0,     0,    99,     0,     0,   100,     5,     6,
       7,     8,     9,   101,   102,     0,     0,     0,    10,   105,
     106,   107,     0,     0,   108,   192,     0,     0,     0,     0,
       0,   112,   113,   114,   115,     0,     0,     0,     0,     0,
       0,     0,     0,   837,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,   186,   187,   188,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   189,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   190,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   191,    97,
       0,     0,     0,    99,     0,     0,   100,     5,     6,     7,
       8,     9,   101,   102,     0,     0,     0,    10,   105,   106,
     107,     0,     0,   108,   192,     0,     0,     0,     0,     0,
     112,   113,   114,   115,     0,     0,     0,     0,     0,     0,
       0,     0,   839,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   186,   187,   188,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   189,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   190,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   191,    97,     0,
       0,     0,    99,     0,     0,   100,     5,     6,     7,     8,
       9,   101,   102,     0,     0,     0,    10,   105,   106,   107,
       0,     0,   108,   192,     0,     0,     0,     0,     0,   112,
     113,   114,   115,     0,     0,     0,     0,     0,     0,     0,
       0,  1323,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,     0,   186,   187,   188,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   189,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   190,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   191,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   192,     5,     6,     7,     8,     9,   112,   113,
     114,   115,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   360,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,   186,   187,   188,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   189,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   190,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   191,    97,     0,     0,     0,    99,     0,
       0,   100,     5,     6,     7,     8,     9,   101,   102,     0,
       0,     0,    10,   105,   106,   107,     0,     0,   108,  1454,
       0,     0,     0,     0,     0,   112,   113,   114,   115,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,     0,
     186,   187,   188,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   189,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   190,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   191,    97,     0,     0,     0,    99,     0,     0,
     100,     5,     6,     7,     8,     9,   101,   102,     0,     0,
       0,    10,   105,   106,   107,     0,     0,   108,   192,     0,
       0,     0,     0,     0,   112,   113,   114,   115,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
     666,    39,    40,     0,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   186,
     187,   188,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   189,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   190,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   191,    97,     0,   280,   281,    99,   282,   283,   100,
       0,   284,   285,   286,   287,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   192,     0,   288,
     289,     0,     0,   112,   113,   114,   115,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,   291,   502,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   503,   293,   294,   295,   296,   297,   298,   299,     0,
       0,     0,   213,     0,   214,    40,     0,     0,   300,     0,
       0,     0,     0,     0,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,    50,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     213,   336,     0,   785,   338,   339,   340,     0,     0,     0,
     341,   602,   217,   218,   219,   220,   221,   603,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,   280,   281,
       0,   282,   283,     0,   604,   284,   285,   286,   287,     0,
      93,    94,     0,    95,   191,    97,   346,     0,   347,     0,
       0,   348,     0,   288,   289,     0,     0,     0,     0,   350,
     217,   218,   219,   220,   221,     0,     0,     0,     0,   108,
       0,     0,     0,   786,     0,     0,   112,     0,     0,     0,
       0,     0,   291,     0,     0,   371,     0,     0,    93,    94,
       0,    95,   191,    97,     0,     0,   293,   294,   295,   296,
     297,   298,   299,     0,     0,     0,   213,     0,   214,    40,
       0,     0,   300,     0,     0,     0,     0,   108,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,    50,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,     0,   336,     0,   337,   338,   339,
     340,     0,     0,     0,   341,   602,   217,   218,   219,   220,
     221,   603,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   280,   281,     0,   282,   283,     0,   604,   284,
     285,   286,   287,     0,    93,    94,     0,    95,   191,    97,
     346,     0,   347,     0,     0,   348,     0,   288,   289,     0,
     290,     0,     0,   350,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   108,     0,     0,     0,   786,     0,     0,
     112,     0,     0,     0,     0,     0,   291,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   292,     0,     0,
     293,   294,   295,   296,   297,   298,   299,     0,     0,     0,
     213,     0,     0,     0,     0,     0,   300,     0,     0,     0,
       0,     0,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,    50,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,     0,   336,
       0,     0,   338,   339,   340,     0,     0,     0,   341,   342,
     217,   218,   219,   220,   221,   343,     0,     0,     0,     0,
       0,     0,   213,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   344,     0,     0,    91,   345,     0,    93,    94,
       0,    95,   191,    97,   346,    50,   347,     0,     0,   348,
       0,   280,   281,     0,   282,   283,   349,   350,   284,   285,
     286,   287,     0,     0,     0,     0,     0,   108,   351,     0,
       0,     0,  1890,     0,     0,     0,   288,   289,     0,   290,
       0,     0,   217,   218,   219,   220,   221,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,   291,   502,     0,     0,     0,
      93,    94,     0,    95,   191,    97,   292,     0,   503,   293,
     294,   295,   296,   297,   298,   299,     0,     0,     0,   213,
       0,     0,     0,     0,     0,   300,     0,     0,     0,   108,
     742,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,    50,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,     0,   336,     0,
       0,   338,   339,   340,     0,     0,     0,   341,   342,   217,
     218,   219,   220,   221,   343,     0,     0,     0,     0,     0,
       0,   213,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   344,     0,     0,    91,   345,     0,    93,    94,     0,
      95,   191,    97,   346,    50,   347,     0,     0,   348,     0,
     280,   281,     0,   282,   283,   349,   350,   284,   285,   286,
     287,     0,     0,     0,     0,     0,   108,   351,     0,     0,
       0,  1964,     0,     0,     0,   288,   289,     0,   290,     0,
       0,   217,   218,   219,   220,   221,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,  1187,  1188,   291,     0,     0,     0,     0,    93,
      94,     0,    95,   191,    97,   292,     0,  1189,   293,   294,
     295,   296,   297,   298,   299,     0,     0,     0,   213,     0,
       0,     0,     0,     0,   300,     0,     0,     0,   108,  1037,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,    50,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,     0,   336,     0,   337,
     338,   339,   340,     0,     0,     0,   341,   342,   217,   218,
     219,   220,   221,   343,     0,     0,     0,     0,     0,     0,
     213,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     344,  1132,     0,    91,   345,     0,    93,    94,     0,    95,
     191,    97,   346,    50,   347,     0,     0,   348,     0,   280,
     281,     0,   282,   283,   349,   350,   284,   285,   286,   287,
       0,     0,     0,     0,     0,   108,   351,     0,     0,     0,
       0,     0,     0,     0,   288,   289,     0,   290,     0,     0,
     217,   218,   219,   220,   221,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   190,   291,     0,    91,     0,     0,    93,    94,
       0,    95,   191,    97,   292,     0,     0,   293,   294,   295,
     296,   297,   298,   299,     0,     0,     0,   213,     0,     0,
       0,     0,     0,   300,     0,     0,     0,   108,     0,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
      50,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,     0,   336,     0,     0,   338,
     339,   340,     0,     0,     0,   341,   342,   217,   218,   219,
     220,   221,   343,     0,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   344,
       0,     0,    91,   345,     0,    93,    94,     0,    95,   191,
      97,   346,    50,   347,     0,     0,   348,     0,     0,     0,
     924,   925,     0,   349,   350,  1686,     0,     0,     0,   280,
     281,     0,   282,   283,   108,   351,   284,   285,   286,   287,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   217,
     218,   219,   220,   221,   288,   289,     0,   290,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   926,     0,     0,    93,    94,     0,
      95,   191,    97,   291,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   292,     0,     0,   293,   294,   295,
     296,   297,   298,   299,     0,     0,   108,   213,     0,     0,
       0,     0,     0,   300,     0,     0,     0,     0,     0,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
      50,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,     0,   336,     0,     0,   338,
     339,   340,     0,     0,     0,   341,   342,   217,   218,   219,
     220,   221,   343,     0,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   344,
       0,     0,    91,   345,     0,    93,    94,     0,    95,   191,
      97,   346,    50,   347,     0,     0,   348,     0,  1787,  1788,
    1789,  1790,  1791,   349,   350,  1792,  1793,  1794,  1795,     0,
       0,     0,     0,     0,   108,   351,     0,     0,     0,     0,
       0,     0,  1796,  1797,  1798,     0,     0,     0,     0,   217,
     218,   219,   220,   221,     0,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,  1799,   502,   926,     0,     0,    93,    94,     0,
      95,   191,    97,     0,     0,   503,  1800,  1801,  1802,  1803,
    1804,  1805,  1806,     0,     0,     0,   213,     0,     0,     0,
       0,     0,  1807,     0,     0,     0,   108,     0,  1808,  1809,
    1810,  1811,  1812,  1813,  1814,  1815,  1816,  1817,  1818,    50,
    1819,  1820,  1821,  1822,  1823,  1824,  1825,  1826,  1827,  1828,
    1829,  1830,  1831,  1832,  1833,  1834,  1835,  1836,  1837,  1838,
    1839,  1840,  1841,  1842,  1843,  1844,  1845,  1846,  1847,  1848,
    1849,     0,     0,     0,  1850,  1851,   217,   218,   219,   220,
     221,     0,  1852,  1853,  1854,  1855,  1856,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1857,  1858,
    1859,     0,   213,     0,    93,    94,     0,    95,   191,    97,
    1860,     0,  1861,  1862,     0,  1863,     0,     0,     0,     0,
       0,     0,  1864,     0,  1865,    50,  1866,     0,  1867,  1868,
       0,   280,   281,   108,   282,   283,     0,     0,   284,   285,
     286,   287,     0,     0,     0,     0,     0,     0,  1695,     0,
       0,     0,     0,     0,     0,     0,   288,   289,     0,     0,
       0,  1696,   217,   218,   219,   220,   221,  1697,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   190,   291,     0,    91,    92,     0,
      93,    94,     0,    95,  1699,    97,     0,     0,     0,   293,
     294,   295,   296,   297,   298,   299,     0,     0,     0,   213,
       0,     0,     0,     0,     0,   300,     0,     0,     0,   108,
       0,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,    50,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   213,   336,     0,
     337,   338,   339,   340,     0,     0,     0,   341,   602,   217,
     218,   219,   220,   221,   603,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,   280,   281,     0,   282,   283,
       0,   604,   284,   285,   286,   287,     0,    93,    94,     0,
      95,   191,    97,   346,     0,   347,     0,     0,   348,     0,
     288,   289,     0,     0,     0,     0,   350,   217,   218,   219,
     220,   221,     0,     0,     0,     0,   108,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   291,
       0,     0,     0,     0,     0,    93,    94,     0,    95,   191,
      97,     0,     0,   293,   294,   295,   296,   297,   298,   299,
       0,     0,     0,   213,     0,     0,     0,     0,     0,   300,
       0,     0,     0,     0,   108,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,    50,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,     0,   336,     0,  1382,   338,   339,   340,     0,     0,
       0,   341,   602,   217,   218,   219,   220,   221,   603,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   280,
     281,     0,   282,   283,     0,   604,   284,   285,   286,   287,
       0,    93,    94,     0,    95,   191,    97,   346,     0,   347,
       0,     0,   348,     0,   288,   289,     0,     0,     0,     0,
     350,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     108,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   291,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   293,   294,   295,
     296,   297,   298,   299,     0,     0,     0,   213,     0,     0,
       0,     0,     0,   300,     0,     0,     0,     0,     0,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
      50,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,     0,   336,     0,     0,   338,
     339,   340,     0,     0,     0,   341,   602,   217,   218,   219,
     220,   221,   603,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   604,
       0,     0,     0,     0,     0,    93,    94,     0,    95,   191,
      97,   346,     0,   347,     0,     0,   348,   474,   475,   476,
       0,     0,     0,     0,   350,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   108,     0,     0,   477,   478,     0,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,   474,   475,   476,     0,     0,
       0,     0,     0,     0,     0,     0,   503,     0,     0,     0,
       0,     0,     0,     0,     0,   477,   478,     0,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,   474,   475,   476,     0,     0,     0,     0,
       0,     0,     0,     0,   503,     0,     0,     0,     0,     0,
       0,     0,     0,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,     0,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,   503,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,   588,   502,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,     0,     0,     0,     0,     0,     0,     0,
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,   590,   502,   474,   475,
     476,     0,     0,     0,     0,     0,     0,     0,     0,   503,
     290,     0,     0,     0,     0,     0,     0,     0,   477,   478,
       0,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,   609,   502,     0,   292,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   503,   290,     0,
     213,     0,     0,     0,     0,     0,  1540,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   613,     0,   292,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   290,     0,   213,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   591,
     217,   218,   219,   220,   221,   592,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,   598,
     829,     0,   190,   292,     0,    91,   345,     0,    93,    94,
       0,    95,   191,    97,   290, -1132,   213,     0,     0,     0,
       0,     0,  1003,     0,     0,     0,   349,   591,   217,   218,
     219,   220,   221,   592,     0,     0,     0,   108,   351,    50,
       0,     0,     0,     0,     0,     0,     0,     0,   852,     0,
     190,   292,     0,    91,   345,     0,    93,    94,     0,    95,
     191,    97,   290,     0,   213,     0,     0,     0,     0,     0,
    1462,     0,     0,     0,   349,   591,   217,   218,   219,   220,
     221,   592,     0,     0,     0,   108,   351,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   190,   292,
       0,    91,   345,     0,    93,    94,     0,    95,   191,    97,
       0,     0,   213,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   349,   591,   217,   218,   219,   220,   221,   592,
       0,     0,     0,   108,   351,    50,     0,     0,     0,     0,
       0,     0,  1390,     0,     0,     0,   190,     0,     0,    91,
     345,     0,    93,    94,     0,    95,   191,    97,     0,     0,
     882,   883,     0,     0,     0,     0,   884,     0,   885,     0,
     349,   591,   217,   218,   219,   220,   221,   592,     0,     0,
     886,   108,   351,     0,     0,     0,     0,     0,    34,    35,
      36,   213,     0,     0,   190,     0,     0,    91,   345,     0,
      93,    94,   215,    95,   191,    97,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,   349,     0,
       0,     0,     0,     0,  1117,     0,     0,     0,     0,   108,
     351,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,
     887,   888,   889,   890,   891,   892,    29,    81,    82,    83,
      84,    85,     0,  1189,    34,    35,    36,   213,   222,   214,
      40,     0,     0,   190,    89,    90,    91,    92,   215,    93,
      94,     0,    95,   191,    97,     0,     0,     0,    99,     0,
      50,     0,     0,     0,     0,     0,     0,   893,   894,     0,
       0,     0,     0,   105,     0,     0,     0,   216,   108,   895,
       0,     0,   882,   883,     0,     0,     0,     0,   884,     0,
     885,     0,     0,     0,     0,  1118,    75,   217,   218,   219,
     220,   221,   886,    81,    82,    83,    84,    85,     0,     0,
      34,    35,    36,   213,   222,     0,     0,     0,     0,   190,
      89,    90,    91,    92,   215,    93,    94,     0,    95,   191,
      97,     0,     0,     0,    99,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   105,
       0,     0,     0,     0,   108,   223,     0,     0,     0,     0,
       0,   112,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   887,   888,   889,   890,   891,   892,    29,    81,
      82,    83,    84,    85,     0,     0,    34,    35,    36,   213,
     222,   214,    40,     0,     0,   190,    89,    90,    91,    92,
     215,    93,    94,     0,    95,   191,    97,     0,     0,     0,
      99,     0,    50,     0,     0,     0,     0,     0,     0,   893,
     894,     0,     0,     0,     0,   105,     0,     0,     0,   216,
     108,   895,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,   217,
     218,   219,   220,   221,    29,    81,    82,    83,    84,    85,
       0,     0,    34,    35,    36,   213,   222,   214,    40,     0,
       0,   190,    89,    90,    91,    92,   215,    93,    94,     0,
      95,   191,    97,     0,     0,     0,    99,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   105,     0,     0,     0,   216,   108,   223,     0,     0,
     629,     0,     0,   112,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   649,    75,   217,   218,   219,   220,   221,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,   222,     0,     0,     0,     0,   190,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   191,    97,    29,
    1059,     0,    99,     0,     0,     0,     0,    34,    35,    36,
     213,     0,   214,    40,     0,     0,     0,   105,     0,     0,
       0,   215,   108,   223,     0,     0,     0,     0,     0,   112,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     216, -1133, -1133, -1133, -1133,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,     0,    75,
     217,   218,   219,   220,   221,    29,    81,    82,    83,    84,
      85,  1189,     0,    34,    35,    36,   213,   222,   214,    40,
       0,     0,   190,    89,    90,    91,    92,   215,    93,    94,
       0,    95,   191,    97,     0,     0,     0,    99,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   105,     0,     0,     0,   216,   108,   223,     0,
       0,     0,     0,     0,   112,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1219,    75,   217,   218,   219,   220,
     221,    29,    81,    82,    83,    84,    85,     0,     0,    34,
      35,    36,   213,   222,   214,    40,     0,     0,   190,    89,
      90,    91,    92,   215,    93,    94,     0,    95,   191,    97,
       0,     0,     0,    99,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   105,     0,
       0,     0,   216,   108,   223,     0,     0,     0,     0,     0,
     112,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,   217,   218,   219,   220,   221,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,   222,
       0,     0,     0,     0,   190,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   191,    97,     0,     0,     0,    99,
       0,     0,     0,     0,   474,   475,   476,     0,     0,     0,
       0,     0,     0,     0,   105,     0,     0,     0,     0,   108,
     223,     0,     0,     0,   477,   478,   112,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,     0,     0,     0,     0,
       0,     0,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,   474,   475,   476,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   549,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,   474,   475,   476,     0,     0,     0,     0,     0,     0,
       0,     0,   503,     0,     0,     0,     0,     0,     0,     0,
     558,   477,   478,     0,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     503,     0,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   958,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,     0,     0,     0,     0,     0,     0,  1045,
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   503,
       0,   474,   475,   476,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1101,   477,   478,     0,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,  1163,
    1164,  1165,     0,     0,     0,     0,     0,     0,     0,     0,
     503,     0,     0,     0,     0,     0,     0,     0,  1437,     0,
    1166,     0,     0,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,  1187,  1188,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1189,  1163,
    1164,  1165,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1480,
    1166,     0,     0,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,  1187,  1188,  1163,  1164,  1165,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1189,     0,
       0,     0,     0,     0,     0,     0,  1166,  1363,     0,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,
    1188,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1189,  1163,  1164,  1165,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1166,  1551,     0,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,
    1188,  1163,  1164,  1165,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1189,     0,     0,     0,     0,     0,
       0,     0,  1166,  1563,     0,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,
    1182,  1183,  1184,  1185,  1186,  1187,  1188,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1189,  1163,  1164,  1165,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1166,  1672,     0,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,
    1182,  1183,  1184,  1185,  1186,  1187,  1188,    34,    35,    36,
     213,     0,   214,    40,     0,     0,     0,     0,     0,     0,
    1189,   215,     0,     0,     0,     0,     0,     0,     0,  1768,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     244,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   245,     0,     0,     0,     0,     0,     0,     0,     0,
     217,   218,   219,   220,   221,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,   222,     0,  1770,
       0,     0,   190,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   191,    97,     0,     0,     0,    99,     0,    34,
      35,    36,   213,     0,   214,    40,     0,     0,     0,     0,
       0,     0,   105,   680,     0,     0,     0,   108,   246,     0,
       0,     0,     0,     0,   112,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   216, -1133, -1133, -1133, -1133,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,   217,   218,   219,   220,   221,     0,    81,    82,
      83,    84,    85,   503,     0,    34,    35,    36,   213,   222,
     214,    40,     0,     0,   190,    89,    90,    91,    92,   215,
      93,    94,     0,    95,   191,    97,     0,     0,     0,    99,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   105,     0,     0,     0,   244,   108,
     681,     0,     0,     0,     0,     0,   682,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   217,   218,
     219,   220,   221,     0,    81,    82,    83,    84,    85,   213,
       0,     0,     0,     0,     0,   222,     0,     0,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,    50,     0,     0,    99,     0,     0,     0,     0,
     368,   369,     0,     0,     0,     0,     0,     0,     0,     0,
     105,     0,     0,     0,     0,   108,   246,     0,     0,     0,
       0,     0,   112,     0,     0,     0,     0,     0,     0,   217,
     218,   219,   220,   221,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   370,     0,     0,   371,     0,     0,    93,    94,     0,
      95,   191,    97,     0,   474,   475,   476,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   372,     0,     0,
       0,   864,     0,     0,   477,   478,   108,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   474,   475,
     476,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   865,   477,   478,
    1042,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,   474,   475,   476,     0,
       0,     0,     0,     0,     0,     0,     0,   503,     0,     0,
       0,     0,     0,     0,     0,     0,   477,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,  1163,  1164,  1165,     0,     0,     0,
       0,     0,     0,     0,     0,   503,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1166,  1568,     0,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,
    1163,  1164,  1165,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1189,     0,     0,     0,     0,     0,     0,
       0,  1166,     0,     0,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,  1186,  1187,  1188,   475,   476,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1189,
       0,     0,     0,     0,     0,   477,   478,     0,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,  1164,  1165,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   503,     0,     0,     0,     0,     0,
       0,     0,     0,  1166,     0,     0,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,   476,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1189,     0,     0,     0,     0,   477,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,  1165,   502,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   503,     0,     0,     0,     0,
       0,  1166,     0,     0,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,  1186,  1187,  1188,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   478,  1189,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1166,     0,   503,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1189,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   503,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,
    1187,  1188,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1189,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,
    1182,  1183,  1184,  1185,  1186,  1187,  1188,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1189,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   503,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,  1187,  1188,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1189,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   503
};

static const yytype_int16 yycheck[] =
{
       5,     6,   109,     8,     9,    10,    11,    12,    13,    56,
      15,    16,    17,    18,   132,     4,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   194,    31,    44,   109,   167,
     552,   124,   109,   109,   737,   574,   172,   173,   124,    44,
      98,   124,    33,     4,   562,   103,   104,    52,     4,    54,
     418,   696,    57,   880,    59,    46,   699,   543,  1001,   508,
      51,   418,   418,   502,    30,    30,   855,     4,    30,    86,
     999,   610,   795,   131,   673,   695,   989,   195,   505,   506,
    1429,    86,   863,   726,   448,   192,    57,   870,   841,     9,
    1036,   368,   369,   370,   167,   372,  1021,   247,  1023,  1116,
       9,   537,   538,     9,   109,     9,     9,     9,  1054,   534,
      14,   192,   539,   258,     9,   192,   223,     9,   534,    14,
      32,   623,   624,   259,     9,     9,  1230,    36,     9,   450,
     451,   452,   568,   655,   656,     9,    54,     9,     9,   246,
      14,     9,   223,     9,     9,   570,   223,     9,     4,     9,
       9,    48,    48,    83,   570,    32,  1102,     9,     9,    57,
       9,     9,   684,     9,   107,   108,     9,     9,    83,   246,
      70,   116,    83,    48,   162,    54,  1003,     4,   162,    48,
      38,  1240,    70,  1112,    83,    38,  1904,   192,    38,   136,
     137,     0,    30,    83,   199,   183,    83,   719,    38,    81,
      70,   183,    91,   166,   162,   183,    38,   162,     8,    70,
      57,    48,    19,    20,   202,   199,   162,   199,   223,    50,
      51,   199,    69,   199,    38,    83,  1581,   199,   183,   168,
      83,   199,   162,    83,    70,   180,   136,   137,   201,   202,
      70,   246,    70,    83,   202,  1158,    70,    70,     4,   167,
     132,    83,   199,   199,   271,    70,   261,    70,   403,   264,
     203,   176,   784,   202,   182,   176,   271,   272,    70,    83,
     159,    83,    70,    70,   162,   182,   265,   176,    70,   183,
     269,   201,   202,   390,   202,   928,    70,    70,   167,   176,
     202,   200,   461,    70,    70,   201,   999,   201,   201,   201,
     196,    70,   202,   200,  1057,   132,   201,    70,   200,   390,
      70,   167,   359,   390,   202,   205,   201,   201,   176,  1244,
     201,   860,  1339,   176,   201,   200,   176,   201,  1031,   201,
     201,   200,   202,   201,  1683,   201,   201,  1692,   199,   201,
     200,   448,   201,  1126,   176,  1128,  1292,  2065,   200,   200,
     872,   200,   200,   184,   200,   360,   878,   200,   200,   196,
     162,  2079,  1717,   200,  1719,   162,   202,   448,   195,   199,
     200,   448,   389,  1477,   202,   199,   545,    14,   977,   202,
    1484,   103,  1486,   195,   389,   390,   199,   202,  1033,   202,
     202,   396,   397,   398,   399,   400,   401,   455,   516,   446,
     202,   406,   199,  1316,   202,   202,    70,   199,    31,  1112,
     202,   199,  1516,    31,   419,   199,  1475,   199,   202,   202,
    1247,   426,   924,   925,   199,   202,   202,    50,   950,   434,
      53,    91,    50,   240,   103,    53,   199,    70,   202,   202,
     445,   517,   202,   448,   166,    83,    84,   103,    83,    84,
     199,    83,   123,   511,   512,   513,   514,   199,   463,   464,
      70,   132,   107,   108,   136,   137,   103,   104,   954,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   166,   503,   159,
     505,   506,   507,   136,   137,  1228,   199,    19,    20,    83,
     166,   418,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   199,   136,   137,   160,   161,
    1634,  1358,   537,   538,   539,   540,   502,   502,   199,   103,
     502,   546,   199,   992,   549,    70,   517,    19,    20,   168,
     448,   122,   162,   558,  1335,   560,     4,   702,   203,   704,
      14,   132,   181,   568,  1623,    70,  1625,   205,    70,   202,
     205,   576,   379,   578,  1096,   713,    70,   433,    32,    70,
     183,   388,  1007,   202,  1123,   159,   160,   161,   395,   199,
      83,  1007,   581,    70,   136,   137,   199,    51,    91,   167,
     407,   508,   166,   199,   681,    53,   433,   183,    56,  1958,
     399,   418,   401,    50,    51,  1051,    83,    84,  1140,   517,
    1059,   989,  1144,   199,   629,    73,  1246,   534,   797,  1151,
     419,  1276,   989,   989,  1279,  1462,   199,   914,   788,   916,
     713,   918,   136,   137,   202,   922,  1559,    32,    96,   556,
      98,   166,   124,    31,   162,   103,   104,  1021,   166,  1023,
     567,   203,   199,   570,   502,    83,   183,   160,   161,   425,
     508,   821,  1963,    91,  1965,   183,   681,   199,   576,   118,
    1105,  1740,   199,   131,     4,  1744,    38,   745,   515,  1105,
     835,   836,    53,    54,    55,    70,   534,   842,   843,    19,
      20,  1022,  1204,    81,  1025,  1026,   842,    70,    69,    83,
      70,   112,  1161,   201,    92,   208,   201,    91,   556,   724,
     121,   122,   123,   124,   125,   126,   104,    83,   240,   567,
      83,   201,   570,    75,    76,    91,   543,    83,    91,  1652,
     201,   159,   160,   161,   201,    91,   896,    70,   753,   121,
     122,   123,   124,   125,   126,   201,   106,   107,   108,   909,
     208,    83,  1485,   141,   142,   143,   144,   145,   240,    91,
     121,   122,   123,   124,   125,   126,  1294,   201,  1155,   162,
    1157,   786,  1159,    70,  1306,   163,   160,   161,   166,   109,
    1158,   169,   170,   194,   172,   173,   174,   201,   176,    70,
     183,  1158,  1158,   159,   160,   161,    70,   160,   161,    75,
      76,   816,   103,   104,   160,   161,   199,   265,    70,   202,
     198,   269,   194,   202,    87,   273,   106,   107,   108,   201,
     134,   135,  1259,  1453,  1479,  1311,   162,  1313,   160,   161,
     199,  1317,   847,   194,  1903,   199,   863,  1369,  1907,  1285,
      70,  1373,  1338,   162,   207,    87,  1378,   713,   863,   199,
    1296,   124,   125,   126,  1386,   201,   202,   379,   859,   113,
     114,   115,   192,  1596,   201,   202,   388,   166,  1417,   868,
    1244,   737,   201,   395,    49,   121,   122,   123,   124,   125,
     126,    69,   124,   125,   126,   407,   132,   133,   404,  1935,
    1936,   183,   408,   223,  1931,  1932,   162,   379,  1611,   199,
     737,   359,  1006,  1007,   199,   722,   388,     9,   390,   162,
     240,   204,   162,   395,   199,     8,   246,   201,   162,   435,
     199,   437,   438,   439,   440,   407,    14,   831,   162,   175,
     696,   201,   368,   369,   370,   265,   372,  1433,  1316,   269,
     201,     9,   202,   958,   201,   960,   132,   962,   194,  1316,
    1316,    14,    50,    51,    52,    53,    54,    55,   973,    57,
    1693,   132,    53,    54,    55,   200,    57,   183,  1500,  1518,
      14,    69,  1504,   988,   103,   433,   200,   976,    69,  1511,
    1635,   798,   200,  2052,   442,  1534,   200,   853,   446,   200,
    1058,   206,   199,   202,   112,   199,   199,   455,     9,   159,
    2069,   200,   200,  1018,   200,   976,  1021,   200,  1023,    95,
     976,     9,   201,  1028,    14,   183,   853,   199,     9,   199,
     837,   543,   839,   202,   201,    83,   202,  1042,   201,   976,
    1045,   201,  1047,   202,   201,  1481,  1051,   202,    50,    51,
      52,    53,    54,    55,   200,   911,   200,   200,   865,   379,
     508,   509,   510,   511,   512,   513,   514,    69,   388,   201,
     390,   543,  2001,   134,   199,   395,   200,  2006,   204,  2022,
       9,     9,   989,   204,   911,   992,   534,   407,   204,   204,
    1454,   204,    70,  1059,  1059,    32,  1101,  1059,   135,  1006,
    1007,  2044,  2031,   182,  1254,   162,   138,  1257,   556,     9,
    2053,   200,   162,   433,   200,    14,   196,  1656,  1109,     9,
     976,  1110,   570,  1021,     9,  1023,  1665,   200,   448,   184,
    1028,   938,  1581,   581,     9,    14,  1457,  1458,  1459,     9,
     200,   200,  1681,   999,   200,  1221,   200,   954,   955,   976,
     134,  1244,   204,   601,   992,   204,     9,   204,  1244,   203,
      14,  1244,   200,  2092,   200,   204,  1955,   162,  1006,  1007,
     199,  1960,   999,   200,     9,  1031,  1070,   103,   201,   627,
     628,   201,   989,  1619,  1620,   138,   162,     9,   200,   199,
      70,  1559,    70,    70,    70,   199,  1103,    70,  1105,   199,
       9,    14,  1559,  1559,  1031,   202,     9,   203,   201,   184,
     722,   831,    14,   202,   662,   663,  1221,   204,  2007,   202,
     976,  1059,  1761,   543,   176,    14,   200,  1232,  2055,   201,
     196,    32,  2059,    70,   199,   199,    32,    14,  1227,  1244,
    1689,   199,   199,  1692,    14,  1001,  2073,  2074,    52,   199,
     722,  1158,    70,    70,  1259,    70,  1112,  1262,  1114,    70,
      70,   581,     9,   199,   199,  1103,  1227,  1105,  1717,   162,
    1719,  1227,   138,   200,   199,    14,  1725,  1033,   201,   201,
    1285,   184,   138,     9,  1652,  1112,   798,  1114,   162,   737,
    1227,  1296,  1297,  2082,   200,  1652,  1652,   745,  2001,   204,
      69,     9,    83,  2006,   176,   176,   203,   203,   203,  1116,
    1117,  1954,   203,  1956,   201,     9,   199,   138,  1335,   201,
      83,    14,  1458,  1221,    83,   837,   798,   839,  2031,     9,
    1335,   199,   201,   200,    92,   199,  1327,   202,   199,   202,
    1345,  1330,   200,   202,   138,   159,  1244,  1454,    32,   204,
     202,  1158,    77,   865,   201,   200,   184,   201,   138,    32,
     200,   200,   204,     9,     9,   837,   204,   839,   204,     9,
     204,  1227,  1911,  1454,   204,   200,   138,  1454,     9,   203,
     200,   200,    14,   831,    83,   833,     9,   203,   138,  2092,
     202,   199,     9,   865,   201,   201,   201,   138,     9,  2042,
    1227,   201,   722,  1925,  1926,   853,   201,   199,   199,  1316,
     200,   204,   200,   200,  1308,  1309,   201,   737,  1312,   867,
     868,   204,   200,   202,   200,  1319,   938,    78,    79,    80,
     204,   204,  1437,   204,   200,   200,   204,    32,   138,  1444,
     201,    92,   954,   955,  1449,   200,  1451,   200,   176,  1454,
    1070,   202,   201,  1260,   113,   201,  1905,   171,    83,  1597,
     167,   201,  1467,   911,    83,    14,   938,    83,   200,   119,
     200,  1227,   920,   921,  1592,  1480,  1481,   138,   798,   202,
     200,   138,   954,   955,    14,   183,  1342,   202,   201,    14,
      14,    14,    83,   199,    60,   200,   147,   148,   149,   150,
     151,   198,   138,   951,   200,   200,    83,   158,   138,  1316,
     201,   201,   201,   164,   165,  1342,  1323,   837,    14,   839,
    1276,    14,    88,  1279,   202,    91,   203,   178,   976,     9,
       9,  1338,  1339,   853,    68,   183,    14,   199,    83,     9,
       9,  2080,   193,   201,   992,   865,  1715,   202,   868,   116,
     103,   999,   162,   103,   174,   184,  1454,   199,  1006,  1007,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    36,    57,  1431,    78,    79,    80,    81,
     201,   180,   200,  1031,   199,  1441,    69,   184,   184,    83,
     177,   911,   200,    83,     9,   201,   200,   200,    83,  1493,
     202,  1495,   104,    14,  1116,  1117,  1611,    83,    83,    14,
    1058,  1616,    83,    14,  1619,  1620,    83,    14,   938,     6,
    1068,  1069,  1070,    83,  1204,  2034,  1433,  1052,  1686,  2049,
     514,   979,  1336,  1760,   954,   955,  2044,  1261,  1531,   141,
     142,   143,   144,   145,  1116,  1117,   631,   509,  1747,  1694,
    1785,   511,  1559,  1872,  1587,  1103,   976,  1105,  2090,  2066,
    1716,    48,  1110,  1743,  1112,   400,  1114,   169,   170,  1583,
     172,   173,   174,  1662,  1156,  1431,  1312,  1231,  1152,   999,
    1069,  1307,  1098,  1308,   396,  1441,   446,  1135,  1308,  1309,
    1310,  1311,  1312,  1313,  1948,  2002,   198,  1317,  1993,  1319,
     881,  1021,  1575,  1023,  1136,   121,   122,   123,   124,   125,
     126,  1031,  1212,  1161,  1570,  1190,   132,   133,    -1,    -1,
    1889,    -1,    -1,  1479,    -1,    -1,   113,    -1,    -1,  1734,
      -1,    -1,   119,    -1,   121,   122,   123,   124,   125,   126,
     127,  1597,  1190,  1570,    -1,  1652,    -1,  1641,  1260,  1643,
      -1,  1645,  1559,    -1,    -1,  1611,  1650,   173,    -1,   175,
      -1,    -1,    -1,    -1,  1591,    -1,    -1,    -1,  1624,    -1,
    1759,  1760,  1244,   189,  1630,   191,  1632,    -1,   194,  1227,
      -1,    -1,   169,   170,  1611,   172,    -1,    -1,  1260,    -1,
    1110,    -1,  1112,    -1,  1114,    -1,  1116,  1117,    -1,  1655,
      -1,  1657,   368,   369,   370,   371,   372,   194,    -1,    -1,
    1666,  1323,    -1,    -1,    -1,    -1,   203,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1338,  1339,    -1,    -1,
    1657,    -1,    -1,    -1,    -1,    -1,    -1,  1884,    -1,  1666,
      -1,    -1,    -1,    -1,   410,  1652,    -1,    -1,    -1,    -1,
      -1,  1323,    -1,    -1,    -1,    -1,    -1,  1751,    -1,    -1,
    1308,  1309,  1310,  1311,  1312,  1313,  1338,  1339,  1624,  1317,
      -1,  1319,    -1,  1493,  1630,  1495,  1632,  2015,    -1,  1635,
      -1,    -1,  1330,    -1,    -1,    -1,    -1,    -1,    -1,  1894,
      -1,    -1,    -1,  1749,  1342,    -1,  1723,    -1,    -1,  1655,
      -1,  1948,  1758,    -1,  1352,    -1,    -1,  1227,  1764,    -1,
      -1,    -1,    -1,    -1,    -1,  1771,    -1,    -1,    -1,    -1,
    2038,  1433,    -1,    -1,  1244,    -1,    -1,    -1,    -1,    -1,
      -1,  1758,    -1,    -1,    -1,    -1,    -1,  1764,    -1,    -1,
    1260,    -1,    -1,    -1,  1771,    -1,    19,    20,    -1,    -1,
      -1,    -1,    -1,    -1,     6,    -1,    -1,    30,    -1,    -1,
      -1,  1433,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,  1425,    -1,    -1,
      -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,     6,
      -1,    -1,    -1,  1749,    -1,    -1,    48,    -1,    -1,    -1,
      -1,    -1,    -1,  1323,    -1,    -1,    -1,    -1,    59,    60,
    1330,    -1,    -1,    -1,    -1,  1909,  1910,    -1,  1338,  1339,
      -1,  1641,  1342,  1643,    -1,  1645,    -1,    -1,    -1,    -1,
    1650,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1493,    -1,  1495,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   623,   624,  1915,
      -1,   113,    -1,    -1,    -1,    -1,    -1,   119,    -1,   121,
     122,   123,   124,   125,   126,   127,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  2088,    -1,   136,   137,    -1,  1915,    -1,
      -1,    -1,  2097,    -1,    -1,    -1,   113,    -1,    -1,    -1,
    2105,    -1,   119,  2108,   121,   122,   123,   124,   125,   126,
     127,    -1,  1968,  1433,    -1,    -1,    -1,   169,   170,    -1,
     172,    -1,  1570,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1751,    -1,  1581,  1454,    -1,    -1,    -1,    -1,  1587,
      -1,  1968,   194,    -1,    -1,  2001,    -1,    -1,    -1,   200,
    2006,   203,   169,   170,    -1,   172,    -1,    -1,    -1,  2015,
      -1,    -1,    -1,  1611,    -1,    -1,     6,   240,    -1,    -1,
      -1,    -1,    -1,    -1,  2001,  2031,    -1,   194,    -1,  2006,
      -1,    -1,    -1,    -1,    -1,    -1,   203,    -1,    -1,    -1,
      -1,    -1,    -1,  1641,    -1,  1643,    -1,  1645,    -1,    -1,
      -1,    -1,  1650,    -1,  2031,    -1,    -1,    -1,    48,  1657,
      -1,    -1,    -1,    -1,  1662,    -1,    -1,   290,  1666,   292,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,  2091,  2092,    -1,  1686,    -1,
      -1,  1689,    -1,    -1,  1692,    -1,    69,    -1,    -1,    -1,
    1570,    -1,    -1,    -1,  1702,    -1,    -1,    -1,    -1,     6,
      -1,  1709,    -1,    -1,  2091,  2092,  2022,    -1,    -1,  1717,
      -1,  1719,    -1,   113,    -1,    -1,    -1,  1725,   351,   119,
      -1,   121,   122,   123,   124,   125,   126,   127,  2044,  1909,
    1910,  1611,    -1,    -1,    -1,    -1,    -1,  2053,    -1,    -1,
      -1,    48,    -1,  1751,    -1,    56,   379,    -1,    -1,    -1,
    1758,  1759,  1760,    -1,    -1,   388,  1764,    -1,    -1,    -1,
      -1,    -1,   395,  1771,    -1,    -1,    -1,    -1,    -1,   169,
     170,    -1,   172,    -1,   407,    -1,    -1,  1657,    -1,    -1,
      -1,    -1,  1662,    -1,    -1,   418,  1666,    -1,   914,    -1,
     916,    -1,   918,    -1,   194,    -1,   922,    -1,   924,   925,
     926,    -1,    -1,   203,    -1,    -1,   113,    -1,    -1,    -1,
     443,    -1,   119,   446,   121,   122,   123,   124,   125,   126,
     127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     6,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,     6,    -1,    -1,    -1,
      -1,    -1,   169,   170,    -1,   172,    -1,    -1,    -1,   502,
      48,    -1,    -1,    -1,    59,    60,  1884,    -1,  1758,  1759,
    1760,    -1,    -1,    -1,  1764,    -1,    -1,   194,    59,    60,
      -1,  1771,    -1,    -1,    81,    -1,   203,  1905,    48,    -1,
      -1,  1909,  1910,    -1,    -1,    -1,    -1,  1915,    -1,    -1,
     543,    -1,    -1,    -1,    -1,    -1,  1924,   104,    -1,    -1,
      -1,    -1,    -1,  1931,  1932,    -1,    -1,  1935,  1936,    -1,
      -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,    -1,    -1,
    1948,   119,    -1,   121,   122,   123,   124,   125,   126,   127,
      -1,   136,   137,    -1,   141,   142,   143,   144,   145,    -1,
    1968,    -1,   595,   113,   597,   136,   137,   600,    -1,   119,
      -1,   121,   122,   123,   124,   125,   126,   127,    -1,   290,
     167,   292,   169,   170,   171,   172,   173,   174,    -1,    -1,
      -1,   169,   170,  2001,   172,    -1,    -1,    -1,  2006,    -1,
     633,    -1,    -1,    -1,    -1,    -1,  2014,    -1,    -1,    -1,
      -1,   198,   199,    -1,    -1,   200,   194,    -1,    -1,   169,
     170,    -1,   172,  2031,    -1,   203,    -1,    -1,    -1,  2037,
      -1,    -1,    -1,    -1,    -1,  1915,    -1,    -1,    -1,    -1,
     351,    -1,    -1,    -1,   194,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,   692,
     693,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   701,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,  1204,    -1,
      -1,    -1,    -1,  2091,  2092,    -1,    -1,    -1,  1968,   722,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,  2001,    -1,    -1,    -1,    -1,  2006,    -1,    -1,    69,
      -1,    -1,   443,    -1,    -1,   446,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,  2031,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,   798,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   831,    -1,
      -1,    -1,    69,    -1,   837,    -1,   839,    -1,    -1,    -1,
      -1,  2091,  2092,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,    -1,   865,   866,    -1,    -1,    -1,    -1,    -1,    -1,
     873,    -1,    -1,    -1,    -1,    -1,    -1,   880,   881,   882,
     883,   884,   885,   886,    31,    -1,    -1,    -1,    -1,    -1,
      59,    60,   895,    -1,   204,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   912,
      -1,    -1,    -1,    -1,   595,    -1,   597,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,    -1,    81,   938,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,   952,
      -1,   954,   955,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    59,    60,   201,    -1,   203,   136,   137,    -1,
      -1,    -1,    -1,    -1,    -1,   978,   979,    -1,    -1,    -1,
      19,    20,    -1,    -1,    -1,    -1,   989,    -1,    -1,    -1,
      -1,    30,    -1,   996,   141,   142,   143,   144,   145,    -1,
    1003,    -1,    -1,    -1,    -1,    -1,  1009,    -1,    -1,    -1,
      -1,   692,   693,    -1,    -1,    -1,   163,  1020,    -1,   166,
     701,    -1,   169,   170,    -1,   172,   173,   174,    -1,   176,
      -1,   200,  1035,    -1,    -1,    -1,    -1,    -1,    -1,   136,
     137,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,   198,  1055,    -1,    -1,    -1,  1059,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,  1070,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   200,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1116,  1117,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1130,    -1,    -1,
      -1,  1134,    -1,  1136,    -1,    -1,  1139,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1152,
    1153,  1154,  1155,  1156,  1157,  1158,  1159,    -1,    -1,  1162,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,  1186,  1187,  1188,  1189,    10,    11,    12,
      -1,    -1,   873,    -1,    -1,    -1,    -1,    -1,    -1,   880,
     881,   240,    -1,    -1,    -1,  1208,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,   203,    -1,
      -1,    -1,  1245,    -1,  1247,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,  1260,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,  1288,    -1,    -1,  1291,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,   978,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1308,  1309,  1310,  1311,  1312,
    1313,    -1,    -1,  1316,  1317,   996,  1319,    -1,    -1,    -1,
    1323,    -1,  1003,    -1,    -1,    -1,    -1,    -1,  1009,    -1,
      -1,    -1,    -1,    -1,    -1,  1338,  1339,    -1,  1341,    -1,
     379,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1351,   388,
      -1,    -1,    -1,    -1,    -1,  1358,   395,    -1,    -1,    -1,
    1363,    -1,  1365,    -1,    -1,    -1,    -1,    -1,   407,    -1,
      10,    11,    12,    -1,  1055,    -1,    -1,    -1,    -1,   418,
     203,    -1,    -1,    -1,    -1,    -1,    -1,  1390,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,  1426,  1427,    -1,    -1,  1430,   203,    69,
    1433,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,  1130,
      -1,    -1,    -1,  1134,    -1,  1136,    -1,    -1,  1139,  1462,
      -1,    30,    31,   502,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
    1493,    -1,  1495,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,   543,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,  1539,  1540,    -1,    -1,
    1543,    -1,    -1,    -1,    -1,    -1,  1549,    -1,  1551,    -1,
    1553,    -1,    -1,    -1,    -1,  1558,  1559,    -1,    -1,    -1,
    1563,   600,  1565,   203,  1245,  1568,  1247,    59,    60,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1581,  1582,
      -1,    -1,  1585,    -1,    -1,    -1,    -1,    -1,    -1,  1592,
      -1,    -1,    -1,    -1,   633,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1288,    -1,    -1,
    1291,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    19,    20,
      -1,    -1,    -1,    -1,   203,    -1,    -1,    -1,  1641,    30,
    1643,    69,  1645,    -1,   136,   137,    -1,  1650,    -1,  1652,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1667,    -1,    -1,    -1,    -1,  1672,
    1351,    -1,    -1,    -1,    -1,    -1,    -1,  1358,    -1,    -1,
      -1,  1684,  1685,   722,    -1,    -1,    -1,    -1,    -1,  1692,
      -1,  1694,    -1,    -1,    -1,    -1,    -1,    31,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,
      -1,    -1,    -1,    -1,  1717,    -1,  1719,    -1,    -1,    -1,
      31,    -1,  1725,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,  1426,  1427,    81,  1751,    -1,
      -1,    -1,   290,    -1,   292,    -1,    -1,    -1,    69,   798,
      -1,    -1,    -1,    -1,    -1,  1768,  1769,  1770,    -1,    -1,
     104,    -1,  1775,    -1,  1777,    -1,    -1,    -1,    -1,    -1,
    1783,  1462,  1785,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   831,   127,    -1,    -1,    -1,    -1,   837,    -1,
     839,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,   351,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   865,   866,    -1,   163,
      -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,   882,   883,   884,   885,   886,    -1,   240,
      -1,    -1,    -1,    -1,    -1,    -1,   895,    -1,  1539,  1540,
      -1,    -1,  1543,    -1,   198,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   912,    -1,    -1,    -1,    -1,  1881,    -1,
      -1,    -1,    -1,    -1,    -1,  1888,    -1,    -1,    -1,   200,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1901,   938,
    1581,    -1,  1905,    -1,    -1,   443,  1909,  1910,   446,    -1,
      -1,  1592,    -1,   952,    -1,   954,   955,    -1,    -1,    -1,
      -1,  1924,    -1,    -1,    -1,    -1,    -1,  1930,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     979,    -1,    -1,  1946,    -1,    -1,    -1,  1950,    30,    31,
     989,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,  1977,    57,    -1,    -1,    -1,    -1,
      -1,  1020,    -1,    -1,    -1,    -1,  1667,    69,   379,  1992,
      -1,    -1,    -1,    -1,    -1,    -1,  1035,   388,    -1,  2002,
      -1,  2004,    -1,    -1,   395,    81,    -1,    -1,    -1,    -1,
      -1,  1692,    -1,    -1,    -1,    -1,   407,    -1,    -1,    -1,
    1059,    -1,    -1,    -1,    -1,    -1,    -1,   418,   104,    -1,
    2033,  1070,    -1,    -1,    -1,    -1,  1717,    -1,  1719,    -1,
      -1,    -1,    -1,    -1,  1725,    -1,  2049,    -1,    -1,    -1,
      -1,    -1,  2055,    -1,    -1,    -1,  2059,   595,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
    2073,  2074,    -1,    -1,    -1,    -1,    -1,  1116,  1117,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,    -1,
      -1,    -1,  1783,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   502,    -1,  1152,  1153,  1154,  1155,  1156,  1157,  1158,
    1159,   203,   198,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,
    1189,    -1,   543,    -1,   692,   693,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1208,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,  1888,    -1,   600,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
    1901,  1260,    -1,    -1,  1905,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   633,  1924,    81,    -1,    83,    84,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,  1308,
    1309,  1310,  1311,  1312,  1313,    -1,    -1,  1316,  1317,    -1,
    1319,    -1,    -1,    -1,  1323,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    59,    60,    -1,  1977,    -1,    -1,  1338,
    1339,    -1,  1341,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  2002,    -1,  2004,  1363,    -1,  1365,    -1,    -1,    -1,
     167,   722,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,    -1,   880,   881,    -1,    -1,    -1,    -1,    -1,   203,
      -1,  1390,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   198,    -1,    -1,    -1,   202,    -1,    -1,   205,    -1,
     136,   137,    -1,    81,  2055,    83,    -1,    85,  2059,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,  1430,  2073,  2074,  1433,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,   798,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,   141,   142,   143,   144,   145,    -1,    -1,
     978,    -1,    -1,    -1,    69,    -1,   837,    -1,   839,    -1,
      -1,    -1,    -1,    -1,  1493,    -1,  1495,    -1,   996,    -1,
      -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
      -1,  1009,    -1,    -1,   865,   866,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     198,   882,   883,   884,   885,   886,    -1,    -1,    81,    -1,
      83,    84,    -1,    -1,   895,    -1,    -1,    -1,    -1,    -1,
    1549,    -1,  1551,    -1,  1553,    -1,    -1,  1055,    -1,  1558,
    1559,   104,    59,    60,  1563,    -1,  1565,    -1,    -1,  1568,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1582,    -1,    -1,  1585,   938,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   954,   955,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   203,    -1,
      -1,    -1,    -1,    -1,   167,    -1,   169,   170,    -1,   172,
     173,   174,  1130,    -1,    -1,    -1,  1134,    -1,   989,   136,
     137,  1139,  1641,    -1,  1643,    -1,  1645,    -1,    -1,    -1,
      -1,  1650,    -1,  1652,    -1,   198,    -1,    -1,    -1,   202,
      -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,  1020,
      -1,    -1,   600,  1672,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1684,  1685,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,  1694,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   633,    -1,    -1,  1059,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1245,    -1,    69,
      -1,    -1,  1751,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1116,  1117,    -1,    -1,  1768,
    1769,  1770,    -1,    -1,    -1,    -1,  1775,    -1,  1777,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1785,    81,    -1,    -1,
    1288,    -1,    -1,  1291,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,    -1,
     104,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,  1189,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,    -1,  1351,    -1,    -1,    -1,  1208,    -1,    -1,
    1358,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,
     174,    -1,  1881,   203,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,   202,  1260,
    1909,  1910,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,  1426,  1427,
      -1,  1930,    -1,    -1,    -1,    -1,    -1,    -1,   866,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,  1946,    -1,    -1,
      -1,  1950,    -1,    -1,   882,   883,   884,   885,   886,    -1,
      -1,    -1,    -1,   104,    -1,  1316,    -1,   895,    -1,    -1,
      -1,   112,  1323,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,   122,   123,   124,   125,   126,    -1,  1338,  1339,    -1,
    1341,    -1,    -1,  1992,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,
      -1,    -1,  1363,    -1,  1365,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   163,    -1,    -1,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,  2033,    -1,    -1,    -1,    -1,  1390,
      -1,  1539,  1540,    -1,    -1,  1543,   187,    -1,    -1,    -1,
    2049,    -1,    -1,   194,    -1,    -1,    -1,   198,   199,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,  1433,  1581,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1020,    -1,  1592,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,  1667,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1692,    -1,    -1,    -1,  1549,    -1,
    1551,    -1,  1553,    81,    -1,    -1,    -1,  1558,  1559,    -1,
      -1,    -1,  1563,    -1,  1565,    -1,    -1,  1568,    -1,  1717,
      -1,  1719,    -1,    -1,  1152,  1153,   104,  1725,  1156,    -1,
      -1,    -1,    -1,    -1,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,
    1188,  1189,    -1,   141,   142,   143,   144,   145,   203,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1208,    -1,    -1,    -1,    -1,  1783,    -1,    -1,    -1,   167,
      -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
      -1,  1652,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   203,    -1,    -1,    -1,
     198,  1672,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    30,
      31,    69,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    81,    57,    -1,    -1,    -1,
    1888,    -1,    -1,    68,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,  1901,    -1,    -1,    81,  1905,   104,    -1,
      -1,    -1,    87,  1341,    -1,    -1,    -1,  1768,  1769,  1770,
      -1,    -1,    -1,    -1,  1775,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,  1784,   130,  1363,    -1,  1365,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1390,    -1,    -1,   140,   141,   142,   143,   144,
     145,   146,    -1,   169,   170,    -1,   172,   173,   174,  1977,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,
      -1,   166,   167,   201,   169,   170,    -1,   172,   173,   174,
      -1,   176,   198,   199,  2002,    -1,  2004,    -1,    10,    11,
      12,    -1,   187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    30,    31,
    1881,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,  1930,
      -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,  1946,    -1,    -1,    -1,  1950,
      92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1549,    -1,  1551,    -1,  1553,    -1,    -1,  1979,    -1,
    1558,    -1,    -1,    -1,    -1,  1563,    -1,  1565,    -1,    -1,
    1568,  1992,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,     7,    -1,   158,    -1,    -1,    -1,
      13,   163,    -1,    -1,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,   201,
      -1,    -1,    -1,    -1,    -1,    48,   198,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,  1672,    88,    -1,    -1,    -1,    92,
      93,    94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,
      -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,   112,
     113,   114,   115,    -1,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,   129,   130,   131,   132,
     133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    81,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,   175,    -1,   104,   178,    -1,    -1,   181,    -1,
    1768,  1769,  1770,    -1,   187,   188,   189,  1775,   191,    -1,
     193,   194,   195,    -1,    -1,   198,   199,    -1,   201,   202,
      -1,    -1,   205,   206,   207,   208,    -1,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,     7,    -1,   163,    -1,    -1,   166,    13,    -1,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,
      -1,    -1,    48,   203,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,  1881,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,
      -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,
      -1,    -1,    -1,   109,   110,   111,   112,   113,   114,   115,
      -1,   117,  1930,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,   129,   130,   131,   132,   133,  1946,    -1,
      -1,    -1,  1950,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,   155,
      -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,   175,
      -1,    -1,   178,    -1,  1992,   181,    -1,    -1,    -1,    -1,
      -1,   187,   188,   189,    -1,   191,    -1,   193,   194,   195,
      -1,    -1,   198,   199,    -1,   201,   202,   203,    -1,   205,
     206,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,   113,   114,   115,    -1,   117,    -1,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,   131,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,   189,
      -1,   191,    -1,   193,   194,   195,    -1,    -1,   198,   199,
      -1,   201,   202,   203,    -1,   205,   206,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,
      94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,
     104,   105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,
     114,   115,    -1,   117,    -1,    -1,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,   129,   130,    -1,   132,   133,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,    -1,   147,   148,   149,   150,   151,    -1,    -1,
      -1,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,   175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,
      -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,
     194,   195,    -1,    -1,   198,   199,    -1,   201,   202,   203,
      -1,   205,   206,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    -1,
      88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,
      -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,
      -1,   109,   110,   111,   112,    -1,   114,   115,    -1,   117,
      -1,    -1,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,   129,   130,    -1,   132,   133,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,    -1,
     178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,
     188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,
     198,   199,    -1,   201,   202,   203,    -1,   205,   206,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,   118,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
     132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,
      -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,   201,
     202,    -1,    -1,   205,   206,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,
      -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,
      -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,   115,
      -1,   117,    -1,    -1,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,   129,   130,    -1,   132,   133,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,   155,
      -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,   175,
      -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,
      -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,
      -1,    -1,   198,   199,    -1,   201,   202,   203,    -1,   205,
     206,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,
      -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,
      -1,   201,   202,   203,    -1,   205,   206,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,
      94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,
     104,   105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,
     114,   115,    -1,   117,    -1,    -1,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,   129,   130,    -1,   132,   133,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,    -1,   147,   148,   149,   150,   151,    -1,    -1,
      -1,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,   175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,
      -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,
     194,   195,    -1,    -1,   198,   199,    -1,   201,   202,   203,
      -1,   205,   206,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    -1,
      88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,
      -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,
      -1,   109,   110,   111,   112,    -1,   114,   115,    -1,   117,
      -1,    -1,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,   129,   130,    -1,   132,   133,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,    -1,
     178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,
     188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,
     198,   199,    -1,   201,   202,   203,    -1,   205,   206,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    -1,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
     132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,
      -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,   201,
     202,    -1,    -1,   205,   206,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,
      -1,    97,    -1,    99,    -1,   101,   102,    -1,   104,   105,
      -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,   115,
      -1,   117,    -1,    -1,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,   129,   130,    -1,   132,   133,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,   155,
      -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,   175,
      -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,
      -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,
      -1,    -1,   198,   199,    -1,   201,   202,    -1,    -1,   205,
     206,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,
      -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,
      -1,   201,   202,   203,    -1,   205,   206,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    77,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,
      94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,
     104,   105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,
     114,   115,    -1,   117,    -1,    -1,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,   129,   130,    -1,   132,   133,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,    -1,   147,   148,   149,   150,   151,    -1,    -1,
      -1,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,   175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,
      -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,
     194,   195,    -1,    -1,   198,   199,    -1,   201,   202,    -1,
      -1,   205,   206,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    -1,
      88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,
      -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,
      -1,   109,   110,   111,   112,    -1,   114,   115,    -1,   117,
      -1,    -1,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,   129,   130,    -1,   132,   133,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,    -1,
     178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,
     188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,
     198,   199,    -1,   201,   202,   203,    -1,   205,   206,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,   100,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
     132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,
      -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,   201,
     202,    -1,    -1,   205,   206,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,
      -1,    97,    98,    99,    -1,   101,    -1,    -1,   104,   105,
      -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,   115,
      -1,   117,    -1,    -1,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,   129,   130,    -1,   132,   133,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,   155,
      -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,   175,
      -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,
      -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,
      -1,    -1,   198,   199,    -1,   201,   202,    -1,    -1,   205,
     206,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,
      -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,
      -1,   201,   202,   203,    -1,   205,   206,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,
      94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,
     104,   105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,
     114,   115,    -1,   117,    -1,    -1,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,   129,   130,    -1,   132,   133,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,    -1,   147,   148,   149,   150,   151,    -1,    -1,
      -1,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,   175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,
      -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,
     194,   195,    -1,    -1,   198,   199,    -1,   201,   202,   203,
      -1,   205,   206,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    -1,
      88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,
      -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,
      -1,   109,   110,   111,   112,    -1,   114,   115,    -1,   117,
      -1,    -1,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,   129,   130,    -1,   132,   133,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,    -1,
     178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,
     188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,
     198,   199,    -1,   201,   202,   203,    -1,   205,   206,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
     132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,
      -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,   201,
     202,   203,    -1,   205,   206,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,
      -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,
      -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,   115,
      -1,   117,    -1,    -1,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,   129,   130,    -1,   132,   133,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,   155,
      -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,   175,
      -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,
      -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,
      -1,    -1,   198,   199,    -1,   201,   202,   203,    -1,   205,
     206,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,
      -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,
      -1,   201,   202,    -1,    -1,   205,   206,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,
      94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,
     104,   105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,
     114,   115,    -1,   117,    -1,    -1,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,   129,   130,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,    -1,   147,   148,   149,   150,   151,    -1,    -1,
      -1,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,
      -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,
     194,   195,    -1,    -1,   198,   199,    -1,   201,   202,    -1,
      -1,   205,   206,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    -1,
      88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,
      -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,
      -1,   109,   110,   111,   112,    -1,   114,   115,    -1,   117,
      -1,    -1,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,   129,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
     178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,
     188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,
     198,   199,    -1,   201,   202,    -1,    -1,   205,   206,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,
      -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,   201,
     202,    -1,    -1,   205,   206,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,
      -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,
      -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,   115,
      -1,   117,    -1,    -1,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,   129,   130,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,   155,
      -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,    -1,
      -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,
      -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,
      -1,    -1,   198,   199,    -1,   201,   202,    -1,    -1,   205,
     206,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,
      -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,
      -1,   201,   202,    -1,    -1,   205,   206,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,
     124,   125,   126,    -1,    -1,   129,   130,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,    -1,   147,   148,   149,   150,   151,    -1,    -1,
      -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,
      -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,
     194,   195,    -1,    -1,   198,   199,     3,     4,     5,     6,
       7,   205,   206,   207,   208,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   121,   122,   123,   124,   125,   126,
      -1,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,   178,    -1,    -1,   181,     3,     4,     5,     6,     7,
     187,   188,    -1,    -1,    -1,    13,   193,   194,   195,    -1,
      -1,   198,   199,    -1,   201,    -1,    -1,    -1,   205,   206,
     207,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   121,   122,   123,   124,   125,   126,    -1,
      -1,   129,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,    -1,   176,    -1,
     178,    -1,    -1,   181,     3,     4,     5,     6,     7,   187,
     188,    -1,    -1,    -1,    13,   193,   194,   195,    -1,    -1,
     198,   199,    -1,    -1,    -1,    -1,    -1,   205,   206,   207,
     208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   121,   122,   123,   124,   125,   126,    -1,    -1,
     129,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,   176,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
      -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,
     199,     3,     4,     5,     6,     7,   205,   206,   207,   208,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
     122,   123,   124,   125,   126,    -1,    -1,   129,   130,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,
      -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,    -1,
     202,    -1,    -1,   205,   206,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   121,   122,   123,   124,   125,
     126,    -1,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,
      -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,    -1,
      -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,
      -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,
      -1,    -1,   198,   199,     3,     4,     5,     6,     7,   205,
     206,   207,   208,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   121,   122,   123,   124,   125,   126,    -1,    -1,
     129,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,
      -1,    -1,   181,     3,     4,     5,     6,     7,   187,   188,
      -1,    -1,    -1,    13,   193,   194,   195,    -1,    -1,   198,
     199,    -1,    -1,    -1,    -1,    -1,   205,   206,   207,   208,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,   109,
      -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   121,   122,   123,   124,   125,   126,    -1,    -1,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
      -1,   181,     3,     4,     5,     6,     7,   187,   188,    -1,
      -1,    -1,    13,   193,   194,   195,    -1,    -1,   198,   199,
      -1,    -1,    -1,    -1,    -1,   205,   206,   207,   208,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,   122,   123,   124,   125,   126,    -1,    -1,   129,   130,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,
     181,     3,     4,     5,     6,     7,   187,   188,    -1,    -1,
      -1,    13,   193,   194,   195,    -1,    -1,   198,   199,    -1,
      -1,    -1,    -1,    -1,   205,   206,   207,   208,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
     122,   123,   124,   125,   126,    -1,    -1,   129,   130,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,
       3,     4,     5,     6,     7,   187,   188,    -1,    -1,    -1,
      13,   193,   194,   195,    -1,    -1,   198,   199,    -1,   201,
      -1,    -1,    -1,   205,   206,   207,   208,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,
     123,   124,   125,   126,    -1,    -1,   129,   130,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,    -1,
      -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,
     193,   194,   195,    -1,    -1,   198,   199,    -1,   201,    -1,
      -1,    -1,   205,   206,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   121,   122,   123,   124,   125,   126,
      -1,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,   178,    -1,    -1,   181,     3,     4,     5,     6,     7,
     187,   188,    -1,    -1,    -1,    13,   193,   194,   195,    -1,
      -1,   198,   199,    -1,    -1,    -1,    -1,    -1,   205,   206,
     207,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   121,   122,   123,   124,   125,   126,    -1,
      -1,   129,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
     178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,
     188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,
     198,   199,   200,    -1,    -1,    -1,    -1,   205,   206,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
     122,   123,   124,   125,   126,    -1,    -1,   129,   130,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,
       3,     4,     5,     6,     7,   187,   188,    -1,    -1,    -1,
      13,   193,   194,   195,    -1,    -1,   198,   199,    -1,    -1,
      -1,    -1,    -1,   205,   206,   207,   208,    -1,    -1,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,
     123,   124,   125,   126,    -1,    -1,   129,   130,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,     3,
       4,     5,     6,     7,   187,   188,    -1,    -1,    -1,    13,
     193,   194,   195,    -1,    -1,   198,   199,    -1,    -1,    -1,
      -1,    -1,   205,   206,   207,   208,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,
     124,   125,   126,    -1,    -1,   129,   130,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,    -1,   147,   148,   149,   150,   151,    -1,    -1,
      -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,   178,    -1,    -1,   181,     3,     4,
       5,     6,     7,   187,   188,    -1,    -1,    -1,    13,   193,
     194,   195,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,
      -1,   205,   206,   207,   208,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,   124,
     125,   126,    -1,    -1,   129,   130,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
      -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,   178,    -1,    -1,   181,     3,     4,     5,
       6,     7,   187,   188,    -1,    -1,    -1,    13,   193,   194,
     195,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,
     205,   206,   207,   208,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   121,   122,   123,   124,   125,
     126,    -1,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,
      -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,    -1,
      -1,    -1,   178,    -1,    -1,   181,     3,     4,     5,     6,
       7,   187,   188,    -1,    -1,    -1,    13,   193,   194,   195,
      -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,   205,
     206,   207,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   121,   122,   123,   124,   125,   126,
      -1,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,
      -1,   198,   199,     3,     4,     5,     6,     7,   205,   206,
     207,   208,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   121,   122,   123,   124,   125,   126,    -1,    -1,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
      -1,   181,     3,     4,     5,     6,     7,   187,   188,    -1,
      -1,    -1,    13,   193,   194,   195,    -1,    -1,   198,   199,
      -1,    -1,    -1,    -1,    -1,   205,   206,   207,   208,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,   122,   123,   124,   125,   126,    -1,    -1,   129,   130,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,
     181,     3,     4,     5,     6,     7,   187,   188,    -1,    -1,
      -1,    13,   193,   194,   195,    -1,    -1,   198,   199,    -1,
      -1,    -1,    -1,    -1,   205,   206,   207,   208,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
     122,   123,   124,   125,   126,    -1,    -1,   129,   130,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,     3,     4,   178,     6,     7,   181,
      -1,    10,    11,    12,    13,   187,   188,    -1,    -1,    -1,
      -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,    28,
      29,    -1,    -1,   205,   206,   207,   208,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    57,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    83,    84,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
      81,   130,    -1,   132,   133,   134,   135,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,   146,    -1,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,   163,    10,    11,    12,    13,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,   177,    -1,
      -1,   180,    -1,    28,    29,    -1,    -1,    -1,    -1,   188,
     141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,   198,
      -1,    -1,    -1,   202,    -1,    -1,   205,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,   166,    -1,    -1,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    83,    84,
      -1,    -1,    87,    -1,    -1,    -1,    -1,   198,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    -1,   130,    -1,   132,   133,   134,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,   163,    10,
      11,    12,    13,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,   177,    -1,    -1,   180,    -1,    28,    29,    -1,
      31,    -1,    -1,   188,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   198,    -1,    -1,    -1,   202,    -1,    -1,
     205,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    -1,   130,
      -1,    -1,   133,   134,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   163,    -1,    -1,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,   175,   104,   177,    -1,    -1,   180,
      -1,     3,     4,    -1,     6,     7,   187,   188,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,   198,   199,    -1,
      -1,    -1,   203,    -1,    -1,    -1,    28,    29,    -1,    31,
      -1,    -1,   141,   142,   143,   144,   145,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    57,    57,    -1,    -1,    -1,
     169,   170,    -1,   172,   173,   174,    68,    -1,    69,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,   198,
     199,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,    -1,   130,    -1,
      -1,   133,   134,   135,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   163,    -1,    -1,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,   104,   177,    -1,    -1,   180,    -1,
       3,     4,    -1,     6,     7,   187,   188,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,
      -1,   203,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,
      -1,   141,   142,   143,   144,   145,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    57,    -1,    -1,    -1,    -1,   169,
     170,    -1,   172,   173,   174,    68,    -1,    69,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,   198,   199,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,    -1,   130,    -1,   132,
     133,   134,   135,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     163,    92,    -1,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,   175,   104,   177,    -1,    -1,   180,    -1,     3,
       4,    -1,     6,     7,   187,   188,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,    -1,
     141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   163,    57,    -1,   166,    -1,    -1,   169,   170,
      -1,   172,   173,   174,    68,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    87,    -1,    -1,    -1,   198,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    -1,   130,    -1,    -1,   133,
     134,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,
      -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,   175,   104,   177,    -1,    -1,   180,    -1,    -1,    -1,
     112,   113,    -1,   187,   188,   189,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,   198,   199,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,    28,    29,    -1,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,
     172,   173,   174,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,   198,    81,    -1,    -1,
      -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    -1,   130,    -1,    -1,   133,
     134,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,
      -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,   175,   104,   177,    -1,    -1,   180,    -1,     3,     4,
       5,     6,     7,   187,   188,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,    -1,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    57,   166,    -1,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    69,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,   198,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,   164,
     165,    -1,    81,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,   177,   178,    -1,   180,    -1,    -1,    -1,    -1,
      -1,    -1,   187,    -1,   189,   104,   191,    -1,   193,   194,
      -1,     3,     4,   198,     6,     7,    -1,    -1,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,   127,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    29,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   163,    57,    -1,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,   198,
      -1,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,    81,   130,    -1,
     132,   133,   134,   135,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,   163,    10,    11,    12,    13,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,   177,    -1,    -1,   180,    -1,
      28,    29,    -1,    -1,    -1,    -1,   188,   141,   142,   143,
     144,   145,    -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,    -1,   198,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    -1,   130,    -1,   132,   133,   134,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,    -1,   163,    10,    11,    12,    13,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,   177,
      -1,    -1,   180,    -1,    28,    29,    -1,    -1,    -1,    -1,
     188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    -1,   130,    -1,    -1,   133,
     134,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,
      -1,    -1,    -1,    -1,    -1,   169,   170,    -1,   172,   173,
     174,   175,    -1,   177,    -1,    -1,   180,    10,    11,    12,
      -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   198,    -1,    -1,    30,    31,    -1,
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
      57,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   201,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,   201,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,   201,    57,    -1,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    31,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   201,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
     200,    -1,   163,    68,    -1,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    31,   176,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,   187,   140,   141,   142,
     143,   144,   145,   146,    -1,    -1,    -1,   198,   199,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,
     163,    68,    -1,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    31,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,   187,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,   198,   199,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    68,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   187,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,   198,   199,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    32,    -1,    -1,    -1,   163,    -1,    -1,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    -1,
     187,   140,   141,   142,   143,   144,   145,   146,    -1,    -1,
      70,   198,   199,    -1,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,
     169,   170,    92,   172,   173,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,   187,    -1,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,   198,
     199,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
     140,   141,   142,   143,   144,   145,    70,   147,   148,   149,
     150,   151,    -1,    69,    78,    79,    80,    81,   158,    83,
      84,    -1,    -1,   163,   164,   165,   166,   167,    92,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,
      -1,    -1,    -1,   193,    -1,    -1,    -1,   121,   198,   199,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,    70,   147,   148,   149,   150,   151,    -1,    -1,
      78,    79,    80,    81,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    92,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,   178,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,
      -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,
      -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,   145,    70,   147,
     148,   149,   150,   151,    -1,    -1,    78,    79,    80,    81,
     158,    83,    84,    -1,    -1,   163,   164,   165,   166,   167,
      92,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
     178,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,   187,
     188,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,   121,
     198,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,    70,   147,   148,   149,   150,   151,
      -1,    -1,    78,    79,    80,    81,   158,    83,    84,    -1,
      -1,   163,   164,   165,   166,   167,    92,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,   178,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   193,    -1,    -1,    -1,   121,   198,   199,    -1,    -1,
     202,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,
      -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,    70,
      71,    -1,   178,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,    -1,   193,    -1,    -1,
      -1,    92,   198,   199,    -1,    -1,    -1,    -1,    -1,   205,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,   140,
     141,   142,   143,   144,   145,    70,   147,   148,   149,   150,
     151,    69,    -1,    78,    79,    80,    81,   158,    83,    84,
      -1,    -1,   163,   164,   165,   166,   167,    92,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   193,    -1,    -1,    -1,   121,   198,   199,    -1,
      -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    70,   147,   148,   149,   150,   151,    -1,    -1,    78,
      79,    80,    81,   158,    83,    84,    -1,    -1,   163,   164,
     165,   166,   167,    92,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,   178,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,
      -1,    -1,   121,   198,   199,    -1,    -1,    -1,    -1,    -1,
     205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,   198,
     199,    -1,    -1,    -1,    30,    31,   205,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,
      31,    -1,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
      31,    -1,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,   138,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,   138,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,   138,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,   138,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,   138,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,   193,    92,    -1,    -1,    -1,   198,   199,    -1,
      -1,    -1,    -1,    -1,   205,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   121,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    69,    -1,    78,    79,    80,    81,   158,
      83,    84,    -1,    -1,   163,   164,   165,   166,   167,    92,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,   121,   198,
     199,    -1,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    81,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,   104,    -1,    -1,   178,    -1,    -1,    -1,    -1,
     112,   113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     193,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,
      -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   163,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   189,    -1,    -1,
      -1,    27,    -1,    -1,    30,    31,   198,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    30,    31,
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
      -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    69,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    69,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   210,   211,     0,   212,     3,     4,     5,     6,     7,
      13,    27,    28,    29,    48,    50,    51,    56,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    70,
      71,    72,    73,    74,    78,    79,    80,    81,    82,    83,
      84,    86,    88,    92,    93,    94,    95,    97,    99,   101,
     104,   105,   109,   110,   111,   112,   113,   114,   115,   117,
     119,   120,   121,   122,   123,   124,   125,   126,   128,   129,
     130,   131,   132,   133,   139,   140,   141,   142,   143,   144,
     145,   147,   148,   149,   150,   151,   155,   158,   163,   164,
     165,   166,   167,   169,   170,   172,   173,   174,   175,   178,
     181,   187,   188,   189,   191,   193,   194,   195,   198,   199,
     201,   202,   205,   206,   207,   208,   213,   216,   226,   227,
     228,   229,   230,   236,   245,   246,   257,   258,   262,   265,
     272,   278,   337,   338,   346,   347,   350,   351,   352,   353,
     354,   355,   356,   357,   359,   360,   361,   363,   366,   378,
     379,   386,   389,   392,   395,   398,   401,   407,   409,   410,
     412,   422,   423,   424,   426,   431,   436,   456,   464,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   480,   493,   495,   497,   121,   122,   123,   139,
     163,   173,   199,   216,   257,   337,   359,   468,   359,   199,
     359,   359,   359,   359,   109,   359,   359,   454,   455,   359,
     359,   359,   359,    81,    83,    92,   121,   141,   142,   143,
     144,   145,   158,   199,   227,   379,   423,   426,   431,   468,
     472,   468,   359,   359,   359,   359,   359,   359,   359,   359,
      38,   359,   484,   485,   121,   132,   199,   227,   270,   423,
     424,   425,   427,   431,   465,   466,   467,   476,   481,   482,
     359,   199,   358,   428,   199,   358,   370,   348,   359,   238,
     358,   199,   199,   199,   358,   201,   359,   216,   201,   359,
       3,     4,     6,     7,    10,    11,    12,    13,    28,    29,
      31,    57,    68,    71,    72,    73,    74,    75,    76,    77,
      87,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   130,   132,   133,   134,
     135,   139,   140,   146,   163,   167,   175,   177,   180,   187,
     188,   199,   216,   217,   218,   229,   498,   519,   520,   523,
      27,   201,   353,   355,   359,   202,   250,   359,   112,   113,
     163,   166,   189,   219,   220,   221,   222,   226,    83,   205,
     305,   306,    83,   307,   123,   132,   122,   132,   199,   199,
     199,   199,   216,   276,   501,   199,   199,    70,    70,    70,
      70,    70,   348,    83,    91,   159,   160,   161,   490,   491,
     166,   202,   226,   226,   216,   277,   501,   167,   199,   199,
     501,   501,    83,   195,   202,   371,    28,   347,   350,   359,
     361,   468,   473,   233,   202,    91,   429,   490,    91,   490,
     490,    32,   166,   183,   502,   199,     9,   201,   199,   346,
     360,   469,   472,   118,    38,   256,   167,   275,   501,   121,
     194,   257,   338,    70,   202,   463,   201,   201,   201,   201,
     201,   201,   201,   201,    10,    11,    12,    30,    31,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    69,   201,    70,    70,   202,   162,   133,
     173,   175,   189,   191,   278,   336,   337,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      59,    60,   136,   137,   458,   463,   463,   199,   199,    70,
     202,   205,   477,   199,   256,   257,    14,   359,   201,   138,
      49,   216,   453,    91,   347,   361,   162,   468,   138,   204,
       9,   438,   271,   347,   361,   468,   502,   162,   199,   430,
     458,   463,   200,   359,    32,   236,     8,   372,     9,   201,
     236,   237,   348,   349,   359,   216,   290,   240,   201,   201,
     201,   140,   146,   523,   523,   183,   522,   199,   112,   523,
      14,   162,   140,   146,   163,   216,   218,   201,   201,   201,
     251,   116,   180,   201,   219,   221,   219,   221,   219,   221,
     226,   219,   221,   202,     9,   439,   201,   103,   166,   202,
     468,     9,   201,    14,     9,   201,   132,   132,   468,   494,
     348,   347,   361,   468,   472,   473,   200,   183,   268,   139,
     468,   483,   484,   359,   380,   381,   348,   404,   404,   380,
     404,   201,    70,   458,   159,   491,    82,   359,   468,    91,
     159,   491,   226,   215,   201,   202,   263,   273,   413,   415,
      92,   199,   205,   373,   374,   376,   422,   426,   475,   477,
     495,   404,    14,   103,   496,   367,   368,   369,   300,   301,
     456,   457,   200,   200,   200,   200,   200,   203,   235,   236,
     258,   265,   272,   456,   359,   206,   207,   208,   216,   503,
     504,   523,    38,    87,   176,   303,   304,   359,   498,   247,
     248,   347,   355,   356,   359,   361,   468,   202,   249,   249,
     249,   249,   199,   501,   266,   256,   359,   479,   359,   359,
     359,   359,   359,    32,   359,   359,   359,   359,   359,   359,
     359,   359,   359,   359,   359,   359,   359,   359,   359,   359,
     359,   359,   359,   359,   359,   359,   359,   359,   427,   359,
     479,   479,   359,   486,   487,   132,   202,   217,   218,   476,
     477,   276,   216,   277,   501,   501,   275,   257,    38,   350,
     353,   355,   359,   359,   359,   359,   359,   359,   359,   359,
     359,   359,   359,   359,   359,   167,   202,   216,   459,   460,
     461,   462,   476,   303,   303,   479,   359,   483,   256,   200,
     359,   199,   452,     9,   438,   200,   200,    38,   359,    38,
     359,   430,   200,   200,   200,   476,   303,   202,   216,   459,
     460,   476,   200,   233,   294,   202,   355,   359,   359,    95,
      32,   236,   288,   201,    27,   103,    14,     9,   200,    32,
     202,   291,   523,    31,    92,   176,   229,   516,   517,   518,
     199,     9,    50,    51,    56,    58,    70,   140,   141,   142,
     143,   144,   145,   187,   188,   199,   227,   387,   390,   393,
     396,   399,   402,   408,   423,   431,   432,   434,   435,   216,
     521,   233,   199,   244,   202,   201,   202,   201,   202,   201,
     103,   166,   202,   201,   112,   113,   166,   222,   223,   224,
     225,   226,   222,   216,   359,   306,   432,    83,     9,   200,
     200,   200,   200,   200,   200,   200,   201,    50,    51,   512,
     514,   515,   134,   281,   199,     9,   200,   200,   138,   204,
       9,   438,     9,   438,   204,   204,   204,   204,    83,    85,
     216,   492,   216,    70,   203,   203,   212,   214,    32,   135,
     280,   182,    54,   167,   182,   202,   417,   361,   138,     9,
     438,   200,   162,   200,   523,   523,    14,   372,   300,   231,
     196,     9,   439,    87,   523,   524,   458,   458,   203,     9,
     438,   184,   468,    83,    84,   302,   359,   200,     9,   439,
      14,     9,   200,     9,   200,   200,   200,   200,    14,   200,
     203,   234,   235,   364,   259,   134,   279,   199,   501,   204,
     203,   359,    32,   204,   204,   138,   203,     9,   438,   359,
     502,   199,   269,   264,   274,    14,   496,   267,   256,    71,
     468,   359,   502,   200,   200,   204,   203,   200,    50,    51,
      70,    78,    79,    80,    92,   140,   141,   142,   143,   144,
     145,   158,   187,   188,   216,   388,   391,   394,   397,   400,
     403,   423,   434,   441,   443,   444,   448,   451,   216,   468,
     468,   138,   279,   458,   463,   458,   200,   359,   295,    75,
      76,   296,   231,   358,   233,   349,   103,    38,   139,   285,
     468,   432,   216,    32,   236,   289,   201,   292,   201,   292,
       9,   438,    92,   229,   138,   162,     9,   438,   200,    87,
     505,   506,   523,   524,   503,   432,   432,   432,   432,   432,
     437,   440,   199,    70,    70,    70,    70,    70,   199,   199,
     432,   162,   202,    10,    11,    12,    31,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    69,
     162,   502,   203,   423,   202,   253,   221,   221,   221,   216,
     221,   222,   222,   226,     9,   439,   203,   203,    14,   468,
     201,   184,     9,   438,   216,   282,   423,   202,   483,   139,
     468,    14,   359,   359,   204,   359,   203,   212,   523,   282,
     202,   416,   176,    14,   200,   359,   373,   476,   201,   523,
     196,   203,   232,   235,   245,    32,   510,   457,   524,    38,
      83,   176,   459,   460,   462,   459,   460,   462,   523,    70,
      38,    87,   176,   359,   432,   248,   355,   356,   468,   249,
     248,   249,   249,   203,   235,   300,   199,   423,   280,   365,
     260,   359,   359,   359,   203,   199,   303,   281,    32,   280,
     523,    14,   279,   501,   427,   203,   199,    14,    78,    79,
      80,   216,   442,   442,   444,   446,   447,    52,   199,    70,
      70,    70,    70,    70,    91,   159,   199,   199,   162,     9,
     438,   200,   452,    38,   359,   280,   203,    75,    76,   297,
     358,   236,   203,   201,    96,   201,   285,   468,   199,   138,
     284,    14,   233,   292,   106,   107,   108,   292,   203,   523,
     184,   138,   162,   523,   216,   176,   516,   523,     9,   438,
     200,   176,   438,   138,   204,     9,   438,   437,   382,   383,
     432,   405,   432,   433,   405,   382,   405,   373,   375,   377,
     405,   200,   132,   217,   432,   488,   489,   432,   432,   432,
      32,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   521,    83,   254,   203,   203,
     203,   203,   225,   201,   432,   515,   103,   104,   511,   513,
       9,   311,   200,   199,   350,   355,   359,   138,   204,   203,
     496,   311,   168,   181,   202,   412,   419,   359,   168,   202,
     418,   138,   201,   510,   199,   248,   346,   360,   469,   472,
     523,   372,    87,   524,    83,    83,   176,    14,    83,   502,
     502,   479,   468,   302,   359,   200,   300,   202,   300,   199,
     138,   199,   303,   200,   202,   523,   202,   201,   523,   280,
     261,   430,   303,   138,   204,     9,   438,   443,   446,   384,
     385,   444,   406,   444,   445,   406,   384,   406,   159,   373,
     449,   450,   406,    81,   444,   468,   202,   358,    32,    77,
     236,   201,   349,   284,   483,   285,   200,   432,   102,   106,
     201,   359,    32,   201,   293,   203,   184,   523,   216,   138,
      87,   523,   524,    32,   200,   432,   432,   200,   204,     9,
     438,   138,   204,     9,   438,   204,   204,   204,   138,     9,
     438,   200,   200,   138,   203,     9,   438,   432,    32,   200,
     233,   201,   201,   201,   201,   216,   523,   523,   511,   423,
       6,   113,   119,   122,   127,   169,   170,   172,   203,   312,
     335,   336,   337,   342,   343,   344,   345,   456,   483,   359,
     203,   202,   203,    54,   359,   203,   359,   359,   372,   468,
     201,   202,   524,    38,    83,   176,    14,    83,   359,   199,
     199,   204,   510,   200,   311,   200,   300,   359,   303,   200,
     311,   496,   311,   201,   202,   199,   200,   444,   444,   200,
     204,     9,   438,   138,   204,     9,   438,   204,   204,   204,
     138,   200,     9,   438,   200,   311,    32,   233,   201,   200,
     200,   200,   241,   201,   201,   293,   233,   138,   523,   523,
     176,   523,   138,   432,   432,   432,   432,   373,   432,   432,
     432,   202,   203,   513,   134,   135,   189,   217,   499,   523,
     283,   423,   113,   345,    31,   127,   140,   146,   167,   173,
     319,   320,   321,   322,   423,   171,   327,   328,   130,   199,
     216,   329,   330,    83,   341,   257,   523,     9,   201,     9,
     201,   201,   496,   336,   200,   308,   167,   414,   203,   203,
     359,    83,    83,   176,    14,    83,   359,   303,   303,   119,
     362,   510,   203,   510,   200,   200,   203,   202,   203,   311,
     300,   138,   444,   444,   444,   444,   373,   203,   233,   239,
     242,    32,   236,   287,   233,   523,   200,   432,   138,   138,
     138,   233,   423,   423,   501,    14,   217,     9,   201,   202,
     499,   496,   322,   183,   202,     9,   201,     3,     4,     5,
       6,     7,    10,    11,    12,    13,    27,    28,    29,    57,
      71,    72,    73,    74,    75,    76,    77,    87,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     139,   140,   147,   148,   149,   150,   151,   163,   164,   165,
     175,   177,   178,   180,   187,   189,   191,   193,   194,   216,
     420,   421,     9,   201,   167,   171,   216,   330,   331,   332,
     201,    14,     9,   201,   256,   341,   499,   499,    14,   257,
     203,   309,   310,   499,    14,    83,   359,   200,   200,   199,
     510,   198,   507,   362,   510,   308,   203,   200,   444,   138,
     138,    32,   236,   286,   287,   233,   432,   432,   432,   203,
     201,   201,   432,   423,   315,   523,   323,   324,   431,   320,
      14,    32,    51,   325,   328,     9,    36,   200,    31,    50,
      53,   432,    83,   218,   500,   201,    14,   523,   256,   201,
      14,   359,    38,    83,   411,   202,   508,   509,   523,   201,
     202,   333,   510,   507,   203,   510,   444,   444,   233,   100,
     252,   203,   216,   229,   316,   317,   318,     9,   438,     9,
     438,   203,   432,   421,   421,    68,   326,   331,   331,    31,
      50,    53,    14,   183,   199,   432,   500,   432,    83,     9,
     439,   231,     9,   439,    14,   511,   231,   202,   333,   333,
      98,   201,   116,   243,   162,   103,   523,   184,   431,   174,
     432,   512,   313,   199,    38,    83,   200,   203,   509,   523,
     203,   231,   201,   199,   180,   255,   216,   336,   337,   184,
     184,   298,   299,   457,   314,    83,   203,   423,   253,   177,
     216,   201,   200,     9,   439,    87,   124,   125,   126,   339,
     340,   298,    83,   283,   201,   510,   457,   524,   524,   200,
     200,   201,   507,    87,   339,    83,    38,    83,   176,   510,
     202,   201,   202,   334,   524,   524,    83,   176,    14,    83,
     507,   233,   231,    83,    38,    83,   176,    14,    83,   359,
     334,   203,   203,    83,   176,    14,    83,   359,    14,    83,
     359,   359
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   209,   211,   210,   212,   212,   213,   213,   213,   213,
     213,   213,   213,   213,   214,   213,   215,   213,   213,   213,
     213,   213,   213,   213,   213,   213,   213,   213,   213,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   218,
     218,   219,   219,   220,   220,   221,   222,   222,   222,   222,
     223,   223,   224,   225,   225,   225,   226,   226,   227,   227,
     227,   228,   229,   230,   230,   231,   231,   232,   232,   233,
     233,   234,   234,   235,   235,   235,   235,   236,   236,   236,
     237,   236,   238,   236,   239,   236,   240,   236,   236,   236,
     236,   236,   236,   236,   236,   236,   236,   236,   236,   236,
     236,   236,   236,   241,   236,   242,   236,   236,   243,   236,
     244,   236,   236,   236,   236,   236,   236,   236,   236,   236,
     236,   236,   236,   236,   236,   236,   236,   236,   236,   236,
     236,   236,   236,   245,   246,   246,   247,   247,   248,   248,
     248,   249,   249,   251,   250,   252,   252,   254,   253,   255,
     255,   256,   256,   257,   259,   258,   260,   258,   261,   258,
     263,   262,   264,   262,   266,   265,   267,   265,   268,   265,
     269,   265,   271,   270,   273,   272,   274,   272,   275,   275,
     276,   277,   278,   278,   278,   278,   278,   279,   279,   280,
     280,   281,   281,   282,   282,   283,   283,   284,   284,   285,
     285,   285,   286,   286,   287,   287,   288,   288,   289,   289,
     290,   290,   291,   291,   291,   291,   292,   292,   292,   293,
     293,   294,   294,   295,   295,   296,   296,   297,   297,   298,
     298,   298,   298,   298,   298,   298,   298,   299,   299,   299,
     299,   299,   299,   299,   299,   299,   299,   300,   300,   300,
     300,   300,   300,   300,   300,   301,   301,   301,   301,   301,
     301,   301,   301,   301,   301,   302,   302,   302,   303,   303,
     304,   304,   304,   304,   304,   304,   304,   304,   305,   305,
     306,   306,   306,   307,   307,   307,   307,   308,   308,   309,
     310,   311,   311,   312,   312,   312,   312,   312,   313,   312,
     314,   312,   312,   312,   312,   312,   312,   312,   312,   315,
     315,   315,   316,   317,   317,   318,   318,   319,   319,   320,
     320,   321,   321,   322,   322,   322,   322,   322,   322,   322,
     323,   323,   324,   325,   325,   326,   326,   327,   327,   328,
     329,   329,   329,   330,   330,   330,   330,   331,   331,   331,
     331,   331,   331,   331,   332,   332,   332,   333,   333,   334,
     334,   335,   335,   336,   336,   337,   337,   338,   338,   338,
     338,   338,   338,   338,   339,   339,   340,   340,   340,   341,
     341,   341,   341,   342,   342,   343,   343,   344,   344,   345,
     346,   347,   347,   347,   347,   347,   347,   348,   348,   349,
     349,   350,   350,   350,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   359,   359,   359,   359,   360,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   362,   362,   364,
     363,   365,   363,   367,   366,   368,   366,   369,   366,   370,
     366,   371,   366,   372,   372,   372,   373,   373,   374,   374,
     375,   375,   376,   376,   377,   377,   378,   379,   379,   380,
     380,   381,   381,   382,   382,   383,   383,   384,   384,   385,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     404,   405,   405,   406,   406,   407,   408,   409,   409,   410,
     410,   410,   410,   410,   410,   410,   410,   410,   410,   410,
     410,   411,   411,   411,   411,   412,   413,   413,   414,   414,
     415,   415,   415,   416,   416,   417,   418,   418,   419,   419,
     419,   420,   420,   420,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   422,   423,   423,
     424,   424,   424,   424,   424,   425,   425,   426,   426,   426,
     426,   427,   427,   427,   428,   428,   428,   429,   429,   429,
     430,   430,   431,   431,   431,   431,   431,   431,   431,   431,
     431,   431,   431,   431,   431,   431,   431,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   433,   433,   434,   435,   435,   436,   436,
     436,   436,   436,   436,   436,   437,   437,   438,   438,   439,
     439,   440,   440,   440,   440,   441,   441,   441,   441,   441,
     442,   442,   442,   442,   443,   443,   444,   444,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   445,   445,   446,   446,   447,   447,   447,   447,
     448,   448,   449,   449,   450,   450,   451,   451,   452,   452,
     453,   453,   455,   454,   456,   457,   457,   458,   458,   459,
     459,   459,   460,   460,   461,   461,   462,   462,   463,   463,
     464,   464,   464,   465,   465,   466,   466,   467,   467,   468,
     468,   468,   468,   468,   468,   468,   468,   468,   468,   468,
     469,   470,   470,   470,   470,   470,   470,   470,   471,   471,
     471,   471,   471,   471,   471,   471,   471,   472,   473,   473,
     474,   474,   474,   475,   475,   475,   476,   476,   477,   477,
     477,   478,   478,   478,   479,   479,   480,   480,   481,   481,
     481,   481,   481,   481,   482,   482,   482,   482,   482,   483,
     483,   483,   483,   483,   483,   484,   484,   485,   485,   485,
     485,   485,   485,   485,   485,   486,   486,   487,   487,   487,
     487,   488,   488,   489,   489,   489,   489,   490,   490,   490,
     490,   491,   491,   491,   491,   491,   491,   492,   492,   492,
     493,   493,   493,   493,   493,   493,   493,   493,   493,   493,
     493,   494,   494,   495,   495,   496,   496,   497,   497,   497,
     497,   498,   498,   499,   499,   500,   500,   501,   501,   502,
     502,   503,   503,   504,   505,   505,   505,   505,   505,   505,
     506,   506,   506,   506,   507,   507,   508,   508,   509,   509,
     510,   510,   511,   511,   512,   513,   513,   514,   514,   514,
     514,   515,   515,   515,   516,   516,   516,   516,   517,   517,
     518,   518,   518,   518,   519,   520,   521,   521,   522,   522,
     523,   523,   523,   523,   523,   523,   523,   523,   523,   523,
     523,   524,   524
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
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     3,     3,     1,     2,     1,     2,     3,     4,
       3,     1,     2,     1,     2,     2,     1,     3,     1,     3,
       2,     2,     2,     5,     4,     2,     0,     1,     3,     2,
       0,     2,     1,     1,     1,     1,     1,     3,     5,     8,
       0,     4,     0,     6,     0,    10,     0,     4,     2,     3,
       2,     3,     2,     3,     3,     3,     3,     3,     3,     5,
       1,     1,     1,     0,     9,     0,    10,     5,     0,    13,
       0,     5,     3,     3,     3,     3,     5,     5,     5,     3,
       3,     2,     2,     2,     2,     2,     2,     3,     2,     2,
       3,     2,     2,     2,     1,     0,     3,     3,     1,     1,
       1,     3,     2,     0,     4,     9,     0,     0,     4,     2,
       0,     1,     0,     1,     0,    10,     0,    11,     0,    11,
       0,     9,     0,    10,     0,     8,     0,     9,     0,     7,
       0,     8,     0,     8,     0,     7,     0,     8,     1,     1,
       1,     1,     1,     2,     3,     3,     2,     2,     0,     2,
       0,     2,     0,     1,     3,     1,     3,     2,     0,     1,
       2,     4,     1,     4,     1,     4,     1,     4,     1,     4,
       3,     5,     3,     4,     4,     5,     5,     4,     0,     1,
       1,     4,     0,     5,     0,     2,     0,     3,     0,     7,
       8,     6,     2,     5,     6,     4,     0,     4,     4,     5,
       7,     6,     6,     6,     7,     9,     8,     6,     7,     5,
       2,     4,     5,     3,     0,     3,     4,     4,     6,     5,
       5,     6,     6,     8,     7,     4,     1,     1,     2,     0,
       1,     2,     2,     2,     3,     4,     4,     4,     3,     1,
       1,     2,     4,     3,     5,     1,     3,     2,     0,     2,
       3,     2,     0,     3,     4,     2,     2,     2,     0,    11,
       0,    12,     3,     3,     3,     4,     4,     3,     5,     2,
       2,     0,     6,     5,     4,     3,     1,     1,     3,     4,
       1,     2,     1,     1,     5,     6,     1,     1,     4,     1,
       1,     3,     2,     2,     0,     2,     0,     1,     3,     1,
       1,     1,     1,     3,     4,     4,     4,     1,     1,     2,
       2,     2,     3,     3,     1,     1,     1,     1,     3,     1,
       3,     1,     1,     1,     0,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     1,     1,     1,     3,
       5,     1,     3,     5,     4,     3,     3,     3,     4,     3,
       3,     3,     3,     2,     2,     1,     1,     3,     1,     1,
       0,     1,     2,     4,     3,     3,     6,     2,     3,     2,
       3,     6,     3,     1,     1,     1,     1,     1,     3,     6,
       3,     4,     6,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     5,     4,     3,     1,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     1,     5,     0,     0,
      12,     0,    13,     0,     4,     0,     7,     0,     5,     0,
       3,     0,     6,     2,     2,     4,     1,     1,     5,     3,
       5,     3,     2,     0,     2,     0,     4,     4,     3,     2,
       0,     5,     3,     2,     0,     5,     3,     2,     0,     5,
       3,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     2,
       0,     2,     0,     2,     0,     4,     4,     4,     4,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     3,     4,     1,     2,     4,     2,     6,     0,     1,
       0,     5,     4,     2,     0,     1,     1,     3,     1,     3,
       1,     1,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     1,     1,
       1,     1,     1,     1,     3,     1,     3,     1,     1,     1,
       3,     1,     1,     1,     2,     1,     0,     0,     1,     1,
       3,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     1,     1,     4,
       3,     4,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     5,     4,     3,     1,     3,     3,     1,     1,     1,
       1,     1,     3,     3,     3,     2,     0,     1,     0,     1,
       0,     5,     3,     3,     1,     1,     1,     1,     3,     2,
       1,     1,     1,     1,     1,     3,     1,     1,     1,     3,
       1,     2,     2,     4,     3,     4,     1,     1,     1,     1,
       1,     1,     3,     1,     2,     0,     5,     3,     3,     1,
       3,     1,     2,     0,     5,     3,     2,     0,     3,     0,
       4,     2,     0,     3,     3,     1,     0,     1,     1,     1,
       1,     3,     1,     1,     1,     3,     1,     1,     3,     3,
       2,     2,     2,     2,     4,     5,     5,     5,     5,     1,
       1,     1,     1,     1,     1,     3,     3,     4,     4,     3,
       3,     1,     1,     1,     1,     3,     1,     4,     1,     1,
       1,     1,     1,     3,     3,     4,     4,     3,     1,     1,
       7,     9,     9,     7,     6,     8,     1,     2,     4,     4,
       1,     1,     1,     4,     1,     0,     1,     2,     1,     1,
       1,     3,     3,     3,     0,     1,     1,     3,     3,     2,
       3,     6,     0,     1,     4,     2,     0,     5,     3,     3,
       1,     6,     4,     4,     2,     2,     0,     5,     3,     3,
       1,     2,     0,     5,     3,     3,     1,     2,     2,     1,
       2,     1,     4,     3,     3,     6,     3,     1,     1,     1,
       4,     4,     4,     4,     4,     4,     2,     2,     4,     2,
       2,     1,     3,     3,     3,     0,     2,     5,     6,     6,
       7,     1,     2,     1,     2,     1,     4,     1,     4,     3,
       0,     1,     3,     2,     1,     2,     4,     3,     3,     1,
       4,     2,     2,     0,     0,     3,     1,     3,     3,     2,
       0,     2,     2,     2,     2,     1,     2,     4,     2,     5,
       3,     1,     1,     0,     3,     4,     5,     6,     3,     1,
       3,     2,     1,     0,     4,     1,     3,     2,     4,     5,
       2,     2,     1,     1,     1,     1,     3,     2,     1,     8,
       6,     1,     0
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
#line 760 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
#line 7362 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 763 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 7370 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 770 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 7376 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 771 "hphp.y" /* yacc.c:1646  */
    { }
#line 7382 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 7388 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 775 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7394 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7400 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7406 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7412 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 779 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 7418 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 7426 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 783 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 7433 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 785 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 7439 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 786 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7445 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 787 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 7451 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 788 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7457 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 789 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7465 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 793 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7474 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 798 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7483 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 803 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7490 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 806 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7497 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 809 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7505 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 813 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7513 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 817 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7521 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 821 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7529 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 825 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7537 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 828 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7544 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7550 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7556 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7562 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7568 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7574 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7580 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7586 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 840 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7592 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7598 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 842 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7604 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 843 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7610 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 844 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7616 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 845 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7622 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 927 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7628 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 929 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7634 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 934 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7640 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 935 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7647 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 941 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7653 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 945 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7659 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 946 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7665 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 948 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7671 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 950 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7677 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 955 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7683 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 956 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7690 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 962 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7696 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 966 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7703 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 968 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7710 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 970 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7717 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 975 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7723 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 977 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7729 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 980 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7735 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 982 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7741 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 983 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7747 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 988 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7756 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 995 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7765 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1003 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7772 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1006 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7779 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1012 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 7785 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1013 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval)); }
#line 7791 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1018 "hphp.y" /* yacc.c:1646  */
    {
                                         _p->onUsing((yyval), (yyvsp[-2]), true, (yyvsp[-1]), nullptr);
                                       }
#line 7799 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1025 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7805 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1026 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7811 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1031 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 7817 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1032 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval), (yyval), (yyvsp[0])); }
#line 7824 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1037 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7830 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7836 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1039 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7842 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1040 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7848 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7854 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1047 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7860 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1052 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7866 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1053 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7873 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7881 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1059 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7888 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1062 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7896 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1066 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7903 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1068 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7911 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1071 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7918 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1073 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7926 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7932 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1077 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7938 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1078 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7944 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1079 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7950 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1080 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7956 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1081 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7962 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1082 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7968 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7974 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7980 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1085 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7986 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7992 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1087 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7998 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1088 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 8004 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1089 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 8010 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1090 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 8017 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1094 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 8024 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1096 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 8032 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1101 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 8039 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1103 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 8047 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1107 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 8055 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1116 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 8061 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1117 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8067 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1120 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false); }
#line 8073 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1121 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 8079 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1123 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8087 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1127 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8095 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1131 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8103 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1135 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8111 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1139 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8119 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1143 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8127 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1148 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8135 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1151 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 8141 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 8150 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1156 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8156 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1157 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8162 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1158 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8168 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1159 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8174 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1160 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8180 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1161 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8186 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1162 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 8192 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1163 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8198 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1164 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8204 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1165 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 8210 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1166 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8216 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1167 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 8226 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1189 "hphp.y" /* yacc.c:1646  */
    { _p->pushLabelScope(LS::Using);
                                         _p->onNewLabelScope(false);
                                         (yyval) = (yyvsp[-1]); }
#line 8234 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1195 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1; }
#line 8240 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1196 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8246 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1205 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), nullptr, (yyvsp[-2]));
                                         _p->onExprListElem((yyval), &(yyval), (yyvsp[0])); }
#line 8253 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1207 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0])); }
#line 8259 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1217 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8265 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1218 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8271 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1222 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false); }
#line 8277 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1223 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8283 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1232 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 8289 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1233 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8295 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1237 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::Finally);}
#line 8302 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1239 "hphp.y" /* yacc.c:1646  */
    { _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->popLabelScope();
                                         _p->onCompleteLabelScope(false);}
#line 8310 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1245 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8316 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1246 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8322 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1250 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 8328 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1251 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8334 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1255 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 8340 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1261 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8349 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1268 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8359 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1276 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8368 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1283 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8378 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1291 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8387 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1297 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8397 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1306 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8404 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1310 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 8410 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1314 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8417 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1318 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 8423 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1324 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8430 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1327 "hphp.y" /* yacc.c:1646  */
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
#line 8448 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1342 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8455 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1345 "hphp.y" /* yacc.c:1646  */
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
#line 8473 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1359 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8480 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1362 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8488 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1367 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8495 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1370 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8503 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1376 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 8509 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1379 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 8515 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1383 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8522 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1386 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8533 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1394 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8540 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1397 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8551 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1405 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8557 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1406 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 8564 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1410 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8570 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1413 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8576 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1416 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 8582 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1417 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 8588 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1418 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 8596 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1421 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 8602 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1422 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 8608 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1426 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8614 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1427 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8620 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1430 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8626 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1431 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8632 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1434 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8638 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1435 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8644 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1438 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8650 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1440 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8656 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1443 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8662 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1445 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8668 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8674 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1450 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8680 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8686 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8692 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1455 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8698 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8704 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8710 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8716 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8722 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1469 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8728 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1471 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8734 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1474 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8740 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1476 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8746 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1480 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8752 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1482 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8759 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8765 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8771 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8777 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8783 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1495 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8789 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1497 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8795 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1498 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8801 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1501 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8807 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1502 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8813 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1507 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8819 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1508 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8825 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1513 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8831 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1514 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8837 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8843 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1518 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8849 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1521 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8855 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1522 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8861 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1530 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8868 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1536 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8875 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1542 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8883 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1546 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8889 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1550 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8896 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1555 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8903 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1560 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8911 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1563 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8917 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1569 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8925 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1574 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8933 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1579 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8941 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1585 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8949 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1591 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8957 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1597 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8965 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1603 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8973 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1609 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8981 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1616 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8989 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1623 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8997 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1632 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 9004 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1637 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 9011 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1642 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 9019 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1646 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9025 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1649 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 9032 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1653 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 9039 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1657 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 9047 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1660 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9053 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1665 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9061 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1669 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9069 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1673 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9077 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1678 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9085 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1683 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9093 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1688 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9101 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1693 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9109 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1698 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9117 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1704 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9125 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1710 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9133 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1716 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 9139 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1717 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 9145 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1718 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 9151 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9157 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1724 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9163 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1727 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,false);}
#line 9170 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1729 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::InOut,false);}
#line 9177 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1731 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::Ref,false);}
#line 9184 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1733 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,true);}
#line 9191 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1736 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::In,false);}
#line 9198 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1739 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::In,true);}
#line 9205 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1742 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::Ref,false);}
#line 9212 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1745 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::InOut,false);}
#line 9219 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1750 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9225 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1751 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 9231 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1754 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9237 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1755 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 9243 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1756 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 9249 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1760 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9255 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1762 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9261 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1763 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 9267 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1764 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9273 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1769 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9279 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1770 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9285 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1773 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9292 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1778 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9298 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1784 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9304 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1785 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9310 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1789 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-2]),(yyvsp[-1]),NULL);}
#line 9317 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1793 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-2]));}
#line 9324 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1795 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9331 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1798 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 9338 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1800 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9344 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1803 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9352 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1810 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9362 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1818 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9370 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1825 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9380 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1831 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 9386 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1833 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9392 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1835 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9398 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1837 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 9404 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1839 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 9410 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1840 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 9417 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1843 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 9423 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9429 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1847 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9435 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1848 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 9441 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1854 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 9447 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1859 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 9454 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1862 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 9462 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1869 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 9468 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1870 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 9475 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1875 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 9482 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1878 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 9488 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1885 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 9495 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1887 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 9501 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1891 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9507 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1896 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9513 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1898 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9519 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1900 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9525 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1901 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 9536 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1907 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 9542 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1909 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 9548 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1910 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 9554 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1914 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval),nullptr,(yyvsp[0])); }
#line 9560 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1916 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval),&(yyvsp[-2]),(yyvsp[0])); }
#line 9566 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1921 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9572 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1924 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9578 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1925 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 9584 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1929 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 9590 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1930 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 9596 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1934 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 9603 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1937 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 9610 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1942 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 9617 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1947 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 9623 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1948 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 9630 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 9636 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1954 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 9642 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1955 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 9648 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 9654 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 9660 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1961 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9666 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 9672 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 9678 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 9684 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 9690 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1967 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 9696 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1969 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 9702 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9710 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9716 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1977 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9722 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1981 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9728 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9734 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9740 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1987 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9746 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1990 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9752 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9758 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9764 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1995 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9770 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1998 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9776 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9782 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9788 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9794 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9800 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 2006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9806 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9812 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 2008 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9818 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 2009 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9824 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9830 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9836 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9842 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9848 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 2019 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9854 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 2023 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9860 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 2025 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9866 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 2026 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9872 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9878 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 2031 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9884 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 2033 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 2037 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9896 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 2039 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9902 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 2043 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9910 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 2047 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9917 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 2051 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9923 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 2055 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9929 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 2059 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9935 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 2061 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9941 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 2062 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9947 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 2063 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9953 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 2064 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9959 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 2065 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9965 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 2068 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9971 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 2069 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9977 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 2073 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9983 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 2074 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9989 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 2078 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9995 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 2079 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 10001 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 2080 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 10007 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 2081 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10013 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 2085 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10019 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 2090 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 10025 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 2094 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 10031 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 2098 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10037 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 2102 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 10043 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 2106 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10049 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 2111 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 10055 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 2115 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10061 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 2119 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10067 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 2120 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10073 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 2121 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10079 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 2122 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10085 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 2123 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10091 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 2127 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10097 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 2132 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 10103 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 2133 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 10109 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 2134 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 10115 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 2137 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 10121 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 2138 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 10127 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 2139 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 10133 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 2140 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 10139 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2141 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 10145 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2142 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 10151 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2143 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 10157 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2144 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 10163 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2145 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 10169 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2146 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 10175 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2147 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 10181 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 10187 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2149 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 10193 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2150 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 10199 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2151 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 10205 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2152 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 10211 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2153 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 10217 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2154 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 10223 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2155 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 10229 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2156 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 10235 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2157 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 10241 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2158 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 10247 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2159 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 10253 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2160 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 10259 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2161 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 10265 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2162 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 10271 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2163 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 10277 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2164 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 10283 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2165 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 10289 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2166 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 10295 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2167 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 10301 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2168 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 10307 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2169 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 10313 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2170 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 10319 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2171 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 10325 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2172 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 10331 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2173 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 10337 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2174 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 10343 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2175 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 10349 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2176 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 10355 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2177 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 10361 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2178 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 10367 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2179 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 10373 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2180 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 10379 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2181 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 10386 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2183 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 10392 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2184 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 10399 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2186 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 10405 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2188 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 10411 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2189 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10417 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2190 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 10423 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2191 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 10429 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2192 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 10435 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2193 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10441 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2194 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 10447 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2195 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 10453 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2196 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 10459 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2197 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 10465 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2198 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 10471 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2199 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 10477 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2200 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 10483 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2201 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 10489 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2202 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 10495 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2203 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10501 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2204 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10507 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2205 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10513 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2206 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10519 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2207 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10525 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10531 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2209 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10537 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2210 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10543 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2211 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10549 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2212 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 10555 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2213 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 10561 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2214 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10567 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2221 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 10573 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2222 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10579 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2227 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10588 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2233 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10600 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2242 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10609 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2248 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10621 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2259 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,NULL,NULL);}
#line 10635 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2268 "hphp.y" /* yacc.c:1646  */
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
#line 10650 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2279 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10660 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2287 "hphp.y" /* yacc.c:1646  */
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
#line 10675 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2298 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10685 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2304 "hphp.y" /* yacc.c:1646  */
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
#line 10702 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2316 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,NULL,NULL);}
#line 10716 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2325 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10729 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2333 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10739 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2341 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10752 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2352 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10758 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2353 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10764 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2355 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10770 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2359 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10777 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2361 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10783 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2368 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10789 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2371 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10795 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2378 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10801 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2381 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10807 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2386 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10813 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2387 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10819 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2392 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10825 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2393 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10831 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2397 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval), (yyvsp[-1]));}
#line 10837 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2401 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10843 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2402 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10849 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2407 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10855 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2408 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10861 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2413 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10867 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2414 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10873 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2419 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10879 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2420 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10885 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2426 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10891 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2428 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10897 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2433 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10903 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2434 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10909 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2440 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10915 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2442 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10921 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2446 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10927 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2450 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10933 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2454 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10939 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2458 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10945 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2462 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10951 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2466 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10957 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2470 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10963 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2474 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10969 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2478 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10975 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2482 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10981 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2486 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10987 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2490 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10993 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2494 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10999 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2498 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11005 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2502 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11011 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2506 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11017 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2510 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11023 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2514 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11029 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2519 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11035 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2520 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11041 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2525 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11047 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2526 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11053 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2531 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11059 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2532 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11065 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2537 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11073 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2544 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11081 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2551 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11087 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2553 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11093 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2557 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11099 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2558 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11105 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2559 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11111 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2560 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11117 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2561 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11123 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2562 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11129 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2563 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11135 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2564 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11141 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2565 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11147 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2566 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 11154 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2568 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11160 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2569 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11166 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11172 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2574 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 11178 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2575 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 11184 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2576 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 11190 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2583 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 11196 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2586 "hphp.y" /* yacc.c:1646  */
    { Token t1; _p->onDArray(t1,(yyvsp[-1]));
                                         Token t2; _p->onVArray(t2,(yyvsp[0]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[-1]),NULL,t1,
                                                         ParamMode::In,0);
                                         _p->onCallParam((yyval), &(yyvsp[-1]),t2,
                                                         ParamMode::In,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),file,
                                                         ParamMode::In,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),line,
                                                         ParamMode::In,0);
                                         (yyval).setText("");}
#line 11214 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2601 "hphp.y" /* yacc.c:1646  */
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onDArray((yyvsp[-2]),(yyvsp[-5]));
                                         _p->onVArray((yyvsp[-1]),(yyvsp[-3]));
                                         _p->onCallParam((yyvsp[-4]),NULL,(yyvsp[-2]),
                                                         ParamMode::In,0);
                                         _p->onCallParam((yyval), &(yyvsp[-4]),(yyvsp[-1]),
                                                         ParamMode::In,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),file,
                                                         ParamMode::In,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),line,
                                                         ParamMode::In,0);
                                         (yyval).setText((yyvsp[0]).text());}
#line 11232 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2616 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 11238 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2617 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 11244 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2620 "hphp.y" /* yacc.c:1646  */
    { _p->onXhpAttributesStart(); (yyval).reset();}
#line 11250 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2622 "hphp.y" /* yacc.c:1646  */
    { _p->onXhpAttributeSpread((yyval), &(yyvsp[-4]), (yyvsp[-1]));}
#line 11256 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2625 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11262 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2628 "hphp.y" /* yacc.c:1646  */
    { _p->onOptExprListElem((yyval), &(yyvsp[-1]), (yyvsp[0])); }
#line 11268 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2629 "hphp.y" /* yacc.c:1646  */
    {  (yyval).reset();}
#line 11274 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2632 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11281 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11289 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11295 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 11307 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2649 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11313 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11319 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2654 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11325 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2656 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 11331 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2658 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 11337 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2662 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11343 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11349 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11355 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2665 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11361 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2666 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11367 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2667 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11373 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11379 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2669 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11385 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11391 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2671 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11397 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2672 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11403 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2673 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11409 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2674 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11415 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2675 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11421 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2676 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11427 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2677 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11433 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2678 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11439 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2679 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11445 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2680 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11451 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2681 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11457 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2682 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11463 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2683 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11469 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2684 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11475 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2685 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11481 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11487 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2687 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11493 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2688 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11499 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2689 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11505 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2690 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11511 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2691 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11517 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11523 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2693 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11529 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2694 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11535 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2695 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11541 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2696 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11547 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2697 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11553 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2698 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11559 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2699 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11565 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2700 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11571 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2701 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11577 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2702 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11583 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2703 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11589 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2704 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11595 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2705 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11601 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2706 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11607 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2707 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11613 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2708 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11619 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2709 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11625 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2710 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11631 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2711 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11637 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11643 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2713 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11649 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2714 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11655 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2715 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11661 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2716 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11667 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2717 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11673 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11679 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11685 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11691 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11697 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2722 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11703 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11709 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2724 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11715 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11721 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2726 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11727 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2727 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11733 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2728 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11739 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11745 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11751 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2731 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11757 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2732 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11763 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2733 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11769 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11775 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2735 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11781 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11787 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2737 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11793 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11799 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11805 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11811 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11817 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2742 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11823 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2743 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11829 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2744 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11835 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2749 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11841 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2753 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11847 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2754 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11853 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2758 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11859 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2759 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11865 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2760 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11871 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2761 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11878 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2763 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11885 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2767 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11891 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2776 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11897 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2779 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11903 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2780 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11910 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2782 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11917 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2792 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11923 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2796 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11929 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2797 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11935 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2798 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11941 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2802 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11947 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2803 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11953 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2804 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11959 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2808 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11965 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2809 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11971 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2810 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11977 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2814 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11983 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2815 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11989 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2819 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11995 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2820 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12001 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2821 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12007 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2822 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 12014 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2824 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 12020 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2825 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 12026 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2826 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 12032 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2827 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 12038 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2828 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 12044 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2829 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 12050 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2830 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 12056 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2831 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 12062 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2832 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 12068 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2835 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12074 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2837 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12080 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12086 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2842 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12092 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2844 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12098 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2845 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12104 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2847 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1]));}
#line 12110 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2848 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12116 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12122 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2850 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12128 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2851 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12134 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2852 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12140 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2853 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12146 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2854 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12152 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2855 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12158 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2856 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12164 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2858 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 12170 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2860 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 12176 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2862 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 12182 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2864 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 12188 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2866 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 12194 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 12200 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2868 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 12206 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2869 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 12212 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2870 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 12218 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2871 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 12224 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2872 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 12230 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2873 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 12236 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2874 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 12242 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2875 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 12248 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2876 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 12254 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2877 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 12260 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2878 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 12266 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2879 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 12272 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2880 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 12278 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2881 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12284 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2882 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12290 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2884 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 12296 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2886 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 12302 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2888 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 12308 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2890 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 12314 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2891 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 12320 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2893 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 12327 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2895 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 12333 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2898 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 12340 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2902 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 12346 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2905 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 12352 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2906 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 12358 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2910 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12364 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2911 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12370 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2917 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12376 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2923 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12382 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2924 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12388 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2928 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12394 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2929 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12400 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2930 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12406 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2931 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12412 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2932 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 12418 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2933 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 12424 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2935 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 12431 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2940 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12437 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2941 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12443 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2945 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12449 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2946 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12455 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2949 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 12461 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2950 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12467 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2956 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12473 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2958 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12479 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2960 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12485 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2961 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12491 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2965 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12497 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2966 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12503 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2967 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12509 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2970 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12515 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2972 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12521 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2975 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 12527 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2976 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 12533 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2977 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 12539 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2978 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12545 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2982 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 12552 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2985 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 12560 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2992 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12566 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2993 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12572 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2996 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 12580 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2999 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12586 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 3000 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12592 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 3001 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12598 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 3003 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12604 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 3004 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12610 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 3006 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1]));}
#line 12616 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 3007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12622 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 3008 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12628 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 3009 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12634 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 3010 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12640 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 3011 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12646 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 3012 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12652 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12658 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 3018 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12664 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 3023 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12670 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 3024 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12676 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 3029 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12682 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 3031 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12688 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 3033 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12694 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 3034 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12700 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3038 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12706 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3039 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12712 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3044 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12718 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3045 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12724 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3050 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12730 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3053 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12736 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3058 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12742 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3059 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12748 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3062 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12754 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3063 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 12761 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3070 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 12767 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3072 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 12773 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3075 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 12779 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3077 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12785 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3080 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12791 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3083 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12797 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3084 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12803 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3088 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12809 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3089 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12815 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3093 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12821 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3094 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12827 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3095 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12833 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3099 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12839 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3101 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12845 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3109 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12851 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3110 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12857 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3114 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12863 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3116 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12869 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3124 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12875 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3125 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12881 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3130 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12887 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3131 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12893 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3133 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12899 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3138 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12905 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3140 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12911 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3146 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12925 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3157 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12939 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3172 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12953 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3184 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12967 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3196 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12973 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3197 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12979 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3198 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12985 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3199 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12991 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3200 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12997 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3201 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13003 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3203 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13017 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3220 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13023 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3222 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13029 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3224 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13035 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3225 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13041 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3229 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13047 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3233 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13053 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3234 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13059 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3235 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13065 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3236 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13071 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3244 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13085 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3253 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13091 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3255 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13097 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3264 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13103 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3265 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13109 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3266 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13115 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3267 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13121 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3268 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13127 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3269 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13133 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3270 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13139 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3276 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13145 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3278 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13151 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3282 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13157 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3286 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13163 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3287 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13169 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3293 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13175 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3297 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13181 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3301 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13187 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3308 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 13193 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3317 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 13199 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3321 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 13205 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3325 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13211 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3328 "hphp.y" /* yacc.c:1646  */
    { _p->onIndirectRef((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 13217 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3334 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13223 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3335 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13229 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3336 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13235 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3340 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13241 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3341 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 13247 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3342 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 13253 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3349 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13259 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3350 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13265 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3355 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 13271 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3356 "hphp.y" /* yacc.c:1646  */
    { (yyval)++;}
#line 13277 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3361 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13283 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3362 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13289 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3363 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13295 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3366 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13309 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3377 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13315 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3378 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13321 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3382 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13327 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3383 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13333 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3386 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13347 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3395 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13353 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3399 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 13359 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3400 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 13365 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3402 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 13371 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3403 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 13377 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3404 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 13383 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3405 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 13389 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3410 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13395 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3411 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13401 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3415 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13407 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3416 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13413 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3417 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13419 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3418 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13425 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3421 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13431 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3423 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 13437 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3424 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13443 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3425 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 13449 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3430 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13455 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3431 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13461 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3435 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13467 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3436 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13473 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3437 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13479 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3438 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13485 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3443 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13491 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3444 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13497 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3449 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13503 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3451 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13509 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3453 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13515 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3454 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13521 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3458 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 13527 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3460 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 13533 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3461 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 13539 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1030:
#line 3463 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 13546 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1031:
#line 3468 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13552 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1032:
#line 3470 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13558 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1033:
#line 3472 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 13572 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1034:
#line 3482 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 13578 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1035:
#line 3484 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 13584 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1036:
#line 3485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13590 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 13596 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1038:
#line 3489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 13602 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1039:
#line 3490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 13608 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3494 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 13614 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3495 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 13620 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3496 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13626 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3497 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13632 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3498 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13638 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3499 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13644 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3500 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 13650 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3501 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 13656 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3502 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 13662 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3503 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 13668 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3504 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 13674 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3508 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 13680 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3509 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 13686 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3514 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13692 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3516 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13698 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3530 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13706 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3535 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 13714 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3539 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13722 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3544 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 13730 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3550 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13736 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3551 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13742 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3555 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13748 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3556 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13754 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3562 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13760 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3566 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 13766 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3572 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13772 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3576 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 13779 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3583 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13785 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3584 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13791 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3588 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 13799 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3591 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 13806 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3597 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13812 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3601 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13820 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3604 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13828 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3607 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-3]); }
#line 13835 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1077:
#line 3609 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13842 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1078:
#line 3611 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13849 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1079:
#line 3613 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13855 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1080:
#line 3618 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]); }
#line 13861 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1081:
#line 3619 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13867 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1082:
#line 3620 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13873 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1083:
#line 3621 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13879 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1090:
#line 3642 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13885 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1091:
#line 3643 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13891 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1094:
#line 3652 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13897 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1097:
#line 3663 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13903 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1098:
#line 3665 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13909 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1099:
#line 3669 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13915 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1100:
#line 3672 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13921 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1101:
#line 3676 "hphp.y" /* yacc.c:1646  */
    {}
#line 13927 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1102:
#line 3677 "hphp.y" /* yacc.c:1646  */
    {}
#line 13933 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1103:
#line 3678 "hphp.y" /* yacc.c:1646  */
    {}
#line 13939 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1104:
#line 3684 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13946 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1105:
#line 3689 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13956 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1106:
#line 3698 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13962 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1107:
#line 3704 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13971 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1108:
#line 3712 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13977 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1109:
#line 3713 "hphp.y" /* yacc.c:1646  */
    { }
#line 13983 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1110:
#line 3719 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13989 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1111:
#line 3721 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13995 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1112:
#line 3722 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 14005 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1113:
#line 3727 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 14012 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1114:
#line 3733 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("HH\\darray"); }
#line 14019 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1115:
#line 3738 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14025 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1116:
#line 3743 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 14033 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1117:
#line 3747 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14039 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1118:
#line 3752 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 14045 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1119:
#line 3754 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 14051 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1120:
#line 3760 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 14058 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1121:
#line 3762 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 14066 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1122:
#line 3765 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14072 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1123:
#line 3766 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14080 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1124:
#line 3769 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14088 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1125:
#line 3772 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14094 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1126:
#line 3775 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 14102 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1127:
#line 3778 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14109 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1128:
#line 3780 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 14118 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1129:
#line 3786 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 14127 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1130:
#line 3792 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("HH\\varray");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 14137 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1131:
#line 3800 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14143 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1132:
#line 3801 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 14149 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;


#line 14153 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 3804 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}
