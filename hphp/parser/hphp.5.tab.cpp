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
#define YYLAST   20368

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  209
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  316
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1136
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  2118

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
    1777,  1783,  1785,  1788,  1791,  1795,  1799,  1804,  1806,  1809,
    1812,  1810,  1827,  1824,  1839,  1841,  1843,  1845,  1847,  1849,
    1851,  1855,  1856,  1857,  1860,  1866,  1870,  1876,  1879,  1884,
    1886,  1891,  1896,  1900,  1901,  1905,  1906,  1908,  1910,  1916,
    1917,  1919,  1923,  1924,  1929,  1933,  1934,  1938,  1939,  1943,
    1945,  1951,  1956,  1957,  1959,  1963,  1964,  1965,  1966,  1970,
    1971,  1972,  1973,  1974,  1975,  1977,  1982,  1985,  1986,  1990,
    1991,  1995,  1996,  1999,  2000,  2003,  2004,  2007,  2008,  2012,
    2013,  2014,  2015,  2016,  2017,  2018,  2022,  2023,  2026,  2027,
    2028,  2031,  2033,  2035,  2036,  2039,  2041,  2050,  2052,  2056,
    2060,  2064,  2069,  2073,  2074,  2076,  2077,  2078,  2079,  2082,
    2083,  2087,  2088,  2092,  2093,  2094,  2095,  2099,  2103,  2108,
    2112,  2116,  2120,  2124,  2129,  2133,  2134,  2135,  2136,  2137,
    2141,  2145,  2147,  2148,  2149,  2152,  2153,  2154,  2155,  2156,
    2157,  2158,  2159,  2160,  2161,  2162,  2163,  2164,  2165,  2166,
    2167,  2168,  2169,  2170,  2171,  2172,  2173,  2174,  2175,  2176,
    2177,  2178,  2179,  2180,  2181,  2182,  2183,  2184,  2185,  2186,
    2187,  2188,  2189,  2190,  2191,  2192,  2193,  2194,  2195,  2197,
    2198,  2200,  2201,  2203,  2204,  2205,  2206,  2207,  2208,  2209,
    2210,  2211,  2212,  2213,  2214,  2215,  2216,  2217,  2218,  2219,
    2220,  2221,  2222,  2223,  2224,  2225,  2226,  2227,  2228,  2232,
    2236,  2241,  2240,  2256,  2254,  2273,  2272,  2293,  2292,  2312,
    2311,  2330,  2330,  2347,  2347,  2366,  2367,  2368,  2373,  2375,
    2379,  2383,  2389,  2393,  2399,  2401,  2405,  2407,  2411,  2415,
    2416,  2420,  2422,  2426,  2428,  2432,  2434,  2438,  2441,  2446,
    2448,  2452,  2455,  2460,  2464,  2468,  2472,  2476,  2480,  2484,
    2488,  2492,  2496,  2500,  2504,  2508,  2512,  2516,  2520,  2524,
    2528,  2532,  2534,  2538,  2540,  2544,  2546,  2550,  2557,  2564,
    2566,  2571,  2572,  2573,  2574,  2575,  2576,  2577,  2578,  2579,
    2580,  2582,  2583,  2587,  2588,  2589,  2590,  2594,  2600,  2613,
    2630,  2631,  2634,  2635,  2637,  2642,  2643,  2646,  2650,  2653,
    2656,  2663,  2664,  2668,  2669,  2671,  2676,  2677,  2678,  2679,
    2680,  2681,  2682,  2683,  2684,  2685,  2686,  2687,  2688,  2689,
    2690,  2691,  2692,  2693,  2694,  2695,  2696,  2697,  2698,  2699,
    2700,  2701,  2702,  2703,  2704,  2705,  2706,  2707,  2708,  2709,
    2710,  2711,  2712,  2713,  2714,  2715,  2716,  2717,  2718,  2719,
    2720,  2721,  2722,  2723,  2724,  2725,  2726,  2727,  2728,  2729,
    2730,  2731,  2732,  2733,  2734,  2735,  2736,  2737,  2738,  2739,
    2740,  2741,  2742,  2743,  2744,  2745,  2746,  2747,  2748,  2749,
    2750,  2751,  2752,  2753,  2754,  2755,  2756,  2757,  2758,  2762,
    2767,  2768,  2772,  2773,  2774,  2775,  2777,  2781,  2782,  2793,
    2794,  2796,  2798,  2810,  2811,  2812,  2816,  2817,  2818,  2822,
    2823,  2824,  2827,  2829,  2833,  2834,  2835,  2836,  2838,  2839,
    2840,  2841,  2842,  2843,  2844,  2845,  2846,  2847,  2850,  2855,
    2856,  2857,  2859,  2860,  2862,  2863,  2864,  2865,  2866,  2867,
    2868,  2869,  2870,  2871,  2873,  2875,  2877,  2879,  2881,  2882,
    2883,  2884,  2885,  2886,  2887,  2888,  2889,  2890,  2891,  2892,
    2893,  2894,  2895,  2896,  2897,  2899,  2901,  2903,  2905,  2906,
    2909,  2910,  2914,  2918,  2920,  2924,  2925,  2929,  2935,  2938,
    2942,  2943,  2944,  2945,  2946,  2947,  2948,  2953,  2955,  2959,
    2960,  2963,  2964,  2968,  2971,  2973,  2975,  2979,  2980,  2981,
    2982,  2985,  2989,  2990,  2991,  2992,  2996,  2998,  3005,  3006,
    3007,  3008,  3013,  3014,  3015,  3016,  3018,  3019,  3021,  3022,
    3023,  3024,  3025,  3026,  3030,  3032,  3036,  3038,  3041,  3044,
    3046,  3048,  3051,  3053,  3057,  3059,  3062,  3065,  3071,  3073,
    3076,  3077,  3082,  3085,  3089,  3089,  3094,  3097,  3098,  3102,
    3103,  3107,  3108,  3109,  3113,  3115,  3123,  3124,  3128,  3130,
    3138,  3139,  3143,  3145,  3146,  3151,  3153,  3158,  3169,  3183,
    3195,  3210,  3211,  3212,  3213,  3214,  3215,  3216,  3226,  3235,
    3237,  3239,  3243,  3247,  3248,  3249,  3250,  3251,  3267,  3268,
    3278,  3279,  3280,  3281,  3282,  3283,  3284,  3286,  3287,  3289,
    3291,  3296,  3300,  3301,  3305,  3308,  3312,  3319,  3323,  3332,
    3339,  3341,  3347,  3349,  3350,  3354,  3355,  3356,  3363,  3364,
    3369,  3370,  3375,  3376,  3377,  3378,  3389,  3392,  3395,  3396,
    3397,  3398,  3409,  3413,  3414,  3415,  3417,  3418,  3419,  3423,
    3425,  3428,  3430,  3431,  3432,  3433,  3436,  3438,  3439,  3443,
    3445,  3448,  3450,  3451,  3452,  3456,  3458,  3461,  3464,  3466,
    3468,  3472,  3473,  3475,  3476,  3482,  3483,  3485,  3495,  3497,
    3499,  3502,  3503,  3504,  3508,  3509,  3510,  3511,  3512,  3513,
    3514,  3515,  3516,  3517,  3518,  3522,  3523,  3527,  3529,  3537,
    3539,  3543,  3547,  3552,  3556,  3564,  3565,  3569,  3570,  3576,
    3577,  3586,  3587,  3595,  3598,  3602,  3605,  3610,  3615,  3618,
    3621,  3623,  3625,  3627,  3631,  3633,  3634,  3635,  3638,  3640,
    3646,  3647,  3651,  3652,  3656,  3657,  3661,  3662,  3665,  3670,
    3671,  3675,  3678,  3680,  3684,  3690,  3691,  3692,  3696,  3700,
    3708,  3713,  3725,  3727,  3731,  3734,  3736,  3741,  3746,  3752,
    3755,  3760,  3765,  3767,  3774,  3776,  3779,  3780,  3783,  3786,
    3787,  3792,  3794,  3798,  3804,  3814,  3815
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

#define YYPACT_NINF -1769

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1769)))

#define YYTABLE_NINF -1137

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1137)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1769,   212, -1769, -1769,  5520, 15046, 15046,     3, 15046, 15046,
   15046, 15046, 12647, 15046, -1769, 15046, 15046, 15046, 15046, 18449,
   18449, 15046, 15046, 15046, 15046, 15046, 15046, 15046, 15046, 12826,
   19247, 15046,    16,    33, -1769, -1769, -1769,   364, -1769,   446,
   -1769, -1769, -1769,   390, 15046, -1769,    33,   223,   307,   343,
   -1769,    33, 13005,  4183, 13184, -1769, 16064, 11510,   375, 15046,
   19496,   365,    87,   330,   529, -1769, -1769, -1769,   387,   398,
     410,   430, -1769,  4183,   433,   474,   556,   618,   620,   625,
     632, -1769, -1769, -1769, -1769, -1769, 15046,   550,   765, -1769,
   -1769,  4183, -1769, -1769, -1769, -1769,  4183, -1769,  4183, -1769,
     526,   513,   517,  4183,  4183, -1769,   344, -1769, -1769, 13390,
   -1769, -1769,   518,   511,   606,   606, -1769,   701,   602,   570,
     574, -1769,   109, -1769,   577,   679,   777, -1769, -1769, -1769,
   -1769,  2079,    83, -1769,   177, -1769,   621,   640,   642,   649,
     652,   655,   676,   678, 17380, -1769, -1769, -1769, -1769, -1769,
      93,   754,   763,   792,   813,   816,   818, -1769,   828,   848,
   -1769,   348,   717, -1769,   760,    52, -1769,  3082,   184, -1769,
   -1769,  2498,   177,   177,   722,   360, -1769,   209,   178,   724,
     381, -1769,    95, -1769,   854, -1769,   766, -1769, -1769,   726,
     761, -1769, 15046, -1769,   777,    83, 19784,  3611, 19784, 15046,
   19784, 19784, 20051, 20051,   728, 18622, 19784,   881,  4183,   863,
     863,   188,   863, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769, -1769,   102, 15046,   757, -1769, -1769,   779,   743,    89,
     749,    89,   863,   863,   863,   863,   863,   863,   863,   863,
   18449, 18670,   748,   945,   766, -1769, 15046,   757, -1769,   796,
   -1769,   797,   767, -1769,   195, -1769, -1769, -1769,    89,   177,
   -1769, 13569, -1769, -1769, 15046, 10077,   952,   112, 19784, 11107,
   -1769, 15046, 15046,  4183, -1769, -1769, 17428,   764, -1769, 17501,
   -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
    5240, -1769,  5240, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769, -1769,   105,   113,   761, -1769, -1769, -1769, -1769,   770,
   -1769,  3831,   115, -1769, -1769,   800,   958, -1769,   812, 16805,
   15046, -1769,   774,   775, 17549, -1769,   448, 17597, 15446, 15446,
   15446,  4183, 15446,   776,   975,   788, -1769,    80, -1769, 18116,
     119, -1769,   972,   128,   858, -1769,   860, -1769, 18449, 15046,
   15046,   793,   814, -1769, -1769, 18192, 12826, 15046, 15046, 15046,
   15046, 15046,   131,    99,   505, -1769, 15225, 18449,   623, -1769,
    4183, -1769,   562,   602, -1769, -1769, -1769, -1769, 19349, 15046,
     981,   893, -1769, -1769, -1769,   123, 15046,   799,   801, 19784,
     803,  2356,   805,  6163, 15046,   531,   804,   630,   531,   520,
     366, -1769,  4183,  5240,   807, 11689, 16064, -1769, 13775,   798,
     798,   798,   798, -1769, -1769,  4175, -1769, -1769, -1769, -1769,
   -1769,   777, -1769, 15046, 15046, -1769, -1769, -1769, -1769, -1769,
   -1769, -1769, -1769, -1769, 15046, 15046, 15046, 15046, 13954, 15046,
   15046, 15046, 15046, 15046, 15046, 15046, 15046, 15046, 15046, 15046,
   15046, 15046, 15046, 15046, 15046, 15046, 15046, 15046, 15046, 15046,
   15046, 15046, 19425, 15046, -1769, 15046, 15046, 15046, 15398,  4183,
    4183,  4183,  4183,  4183,  2079,   897,   742, 11313, 15046, 15046,
   15046, 15046, 15046, 15046, 15046, 15046, 15046, 15046, 15046, 15046,
   -1769, -1769, -1769, -1769,  1310, -1769, -1769, 11689, 11689, 15046,
   15046,   518,   201, 18192,   815,   777, 14133, 17646, -1769, 15046,
   -1769,   821,  1002,   862,   817,   822, 15552,    89, 14312, -1769,
   14491, -1769,   767,   824,   825,  2515, -1769,   106, 11689, -1769,
    2559, -1769, -1769, 17694, -1769, -1769, 12065, -1769, 15046, -1769,
     933, 10283,  1020,   832, 19662,  1021,   107,    73, -1769, -1769,
   -1769,   851, -1769, -1769, -1769,  5240, -1769,  3570,   837,  1029,
   17964,  4183, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769,   840, -1769, -1769,   842,   844,   845,   849,   847,   850,
     469,   853,   852, 16315, 16505, -1769, -1769,  4183,  4183, 15046,
      89,   365, -1769, 17964,   969, -1769, -1769, -1769,    89,   126,
     135,   856,   857,  2650,   242,   869,   866,   415,   938,   878,
      89,   150,   879, 18731,   876,  1072,  1075,   886,   887,   889,
     890, -1769, 16136,  4183, -1769, -1769,  1015,  2885,   327, -1769,
   -1769, -1769,   602, -1769, -1769, -1769,  1067,   970,   922,    96,
     948, 15046,   518,   973,  1098,   912, -1769,   955, -1769,   201,
   -1769,   918,  5240,  5240,  1107,   952,   123, -1769,   927,  1115,
   -1769, 17685,   367, -1769,   506,   213, -1769, -1769, -1769, -1769,
   -1769, -1769, -1769,  1196,  3059, -1769, -1769, -1769, -1769,  1118,
     946, -1769, 18449,   661, 15046,   931,  1123, 19784,  1120,   153,
    1128,   942,   944,   954, 19784,   956,  2839,  6369, -1769, -1769,
   -1769, -1769, -1769, -1769,  1012,  4535, 19784,   959,  3409, 19923,
   20014, 20051, 20088, 15046, 19736, 20161, 15399, 20266, 20299, 16244,
    5307, 19432, 19432, 19432, 19432,  3353,  3353,  3353,  3353,  3353,
    1149,  1149,   759,   759,   759,   188,   188,   188, -1769,   863,
     960,   962, 18779,   964,  1150,    30, 15046,   256,   757,    59,
     201, -1769, -1769, -1769,  1155,   893, -1769,   777, 18297, -1769,
   -1769, -1769, 20051, 20051, 20051, 20051, 20051, 20051, 20051, 20051,
   20051, 20051, 20051, 20051, 20051, -1769, 15046,   476,   211, -1769,
   -1769,   757,   502,   971,   974,   966,  3483,   163,   976, -1769,
   19784, 18040, -1769,  4183, -1769,    89,   431, 18449, 19784, 18449,
   18840,  1012,    91,    89,   334,  1010,   977, 15046, -1769,   347,
   -1769, -1769, -1769,  6575,   681, -1769, -1769, 19784, 19784,    33,
   -1769, -1769, -1769, 15046,  1078, 17888, 17964,  4183, 10489,   982,
     984, -1769,  1173, 15778,  1049, -1769,  1026, -1769,  1181,   992,
    3166,  5240, 17964, 17964, 17964, 17964, 17964,   995,  1125,  1127,
    1135,  1138,  1140,   999,  1013, 17964,    37, -1769, -1769, -1769,
   -1769, -1769, -1769, -1769,    48, -1769, 19878, -1769, -1769,    41,
   -1769,  6781, 15957,  1009, 16505, -1769, 16505, -1769, 16505, -1769,
    4183,  4183, 16505, -1769, 16505, 16505,  4183, -1769,  1204,  1016,
   -1769,   479, -1769, -1769,  4063, -1769, 19878,  1206, 18449,  1023,
   -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769,  1037,
    1217,  4183, 15957,  1027, 18192, 18373,  1214, -1769, 15046, -1769,
   15046, -1769, 15046, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769,  1030, -1769, 15046, -1769, -1769,  5740, -1769,  5240, 15957,
    1031, -1769, -1769, -1769, -1769,  1054,  1222,  1038, 15046, 19349,
   -1769, -1769, 15398, -1769,  1036, -1769,  5240, -1769,  1035,  6987,
    1208,   174, -1769,  5240, -1769,   148,  1310,  1310, -1769,  5240,
   -1769, -1769,    89, -1769, -1769,  1172, 19784, -1769, 11868, -1769,
   17964, 13775,   798, 13775, -1769,   798,   798, -1769, 12271, -1769,
   -1769,  7193, -1769,    90,  1044, 15957,   970, -1769, -1769, -1769,
   -1769, 20161, 15046, -1769, -1769, 15046, -1769, 15046, -1769,  4402,
    1045, 11689,   938,  1213,   970,  5240,  1233,  1012,  4183, 19425,
      89,  5054,  1050,   229,  1056, -1769, -1769,  1242,   657,   657,
   18040, -1769, -1769, -1769,  1215,  1061,  1198,  1200,  1203,  1205,
    1216,   125,  1085,  1088,    34, -1769, -1769, -1769, -1769, -1769,
   -1769,  1104, -1769, -1769, -1769, -1769,  1279,  1093,   821,    89,
      89, 14670,   970,  2559, -1769,  2559, -1769,  5184,   704,    33,
   11107, -1769,  7399,  1094,  7605,  1095, 17888, 18449,  1099,  1156,
      89, 19878,  1287, -1769, -1769, -1769, -1769,   741, -1769,    77,
    5240,  1126,  1166,  1151,  5240,  4183,  4133, -1769, -1769,  5240,
    1302,  1112,  1139,  1158,  1118,   827,   827,  1245,  1245, 18997,
    1122,  1307, 17964, 17964, 17964, 17964, 17964, 17964, 19349, 17964,
    4725, 16959, 17964, 17964, 17964, 17964, 17812, 17964, 17964, 17964,
   17964, 17964, 17964, 17964, 17964, 17964, 17964, 17964, 17964, 17964,
   17964, 17964, 17964, 17964, 17964, 17964, 17964, 17964, 17964, 17964,
    4183, -1769, -1769,  1248, -1769, -1769,  1129,  1132,  1133, -1769,
    1134, -1769, -1769,   488, 16315, -1769,  1137, -1769, 17964,    89,
   -1769, -1769,   141, -1769,   691,  1330, -1769, -1769,   164,  1144,
      89, 12468, 19784, 18888, -1769,  2576, -1769,  5957,   893,  1330,
   -1769,   549, 15046,   413, -1769, 19784,  1207,  1145, -1769,  1146,
    1208, -1769, -1769, -1769, 14867,  5240,   952, 17733,  1263,    84,
    1334,  1266,   349, -1769,   757,   350, -1769,   757, -1769, 15046,
   18449,   661, 15046, 19784, 19878, -1769, -1769, -1769,  3809, -1769,
   -1769, -1769, -1769, -1769, -1769,  1152,    90, -1769,  1148,    90,
    1154, 20161, 19784, 18949,  1160, 11689,  1157,  1153,  5240,  1162,
    1159,  5240,   970, -1769,   767,   551, 11689, 15046, -1769, -1769,
   -1769, -1769, -1769, -1769,  1224,  1147,  1345,  1273, 18040, 18040,
   18040, 18040, 18040, 18040,  1209, -1769, 19349, 18040,    79, 18040,
   -1769, -1769, -1769, 18449, 19784,  1164, -1769,    33,  1335,  1293,
   11107, -1769, -1769, -1769,  1175, 15046,  1156,    89, 18192, 17888,
    1174, 17964,  7811,   794,  1176, 15046,   104,   166, -1769,  1194,
   -1769,  5240,  4183, -1769,  1243, -1769, -1769, -1769,  3673, -1769,
    1348, -1769,  1182, 17964, -1769, 17964, -1769,  1186,  1183,  1386,
   19057,  1199, 19878,  1395,  1202,  1211,  1212,  1269,  1399,  1210,
    1218, -1769, -1769, -1769, 19103,  1219,  1400, 19970, 16801, 20124,
   17964, 19832, 20232,  5467, 17896, 16613,  3177, 18200, 18200, 18200,
   18200,  4429,  4429,  4429,  4429,  4429,   785,   785,   827,   827,
     827,  1245,  1245,  1245,  1245, -1769,  1221, -1769,  1225,  1227,
    1228,  1229, -1769, -1769, 19878,  4183,  5240,  5240, -1769,   691,
   15957,  1597, -1769, 18192, -1769, -1769, 20051, 15046,  1230, -1769,
    1234,  1686, -1769,   103, 15046, -1769, -1769,  5250, -1769, 15046,
   -1769, 15046, -1769,   952, 13775,  1239,   356,   798,   356,   234,
   -1769, -1769,  5240,   199, -1769,  1398,  1352, 15046, -1769,  1232,
    1246,  1238,    89,  1172, 19784,  1208,  1247, -1769,  1250,    90,
   15046, 11689,  1256, -1769, -1769,   893, -1769, -1769,  1257,  1255,
    1249, -1769,  1260, 18040, -1769, 18040, -1769, -1769,  1261,  1267,
    1404,  1326,  1277, -1769,  1456,  1284,  1291,  1292, -1769,  1329,
    1270,  1490,  1301, -1769, -1769,    89, -1769,  1470, -1769,  1304,
   -1769, -1769,  1309,  1311,   165, -1769, -1769, 19878,  1313,  1317,
   -1769, 17332, -1769, -1769, -1769, -1769, -1769, -1769,  1383,  5240,
    5240,  1139,  1346,  5240, -1769, 19878, 19163, -1769, -1769, 17964,
   -1769, 17964, -1769, 17964, -1769, -1769, -1769, -1769, 17964, 19349,
   -1769, -1769, -1769, 17964, -1769, 17964, -1769, 20197, 17964,  1323,
    8017, -1769, -1769, -1769, -1769,   691, -1769, -1769, -1769, -1769,
     669, 16243, 15957,  1414, -1769,  1435,  1358,  4420, -1769, -1769,
    1448,   897,  4529,   136,   137,  1331,   893,   742,   168, 19784,
   -1769, -1769, -1769,  1367,  5433, -1769, 17284, 19784, -1769,  2887,
   -1769,  6369,  1452,    92,  1522,  1454, 15046, -1769, 19784, 11689,
   11689, -1769,  1419,  1208,  1836,  1208,  1340, 19784,  1341, -1769,
    1871,  1344,  1933, -1769, -1769,    90, -1769, -1769,  1405, -1769,
   -1769, 18040, -1769, 18040, -1769, 18040, -1769, -1769, -1769, -1769,
   18040, -1769, 19349, -1769, -1769,  2031, -1769,  8223, -1769, -1769,
   -1769, -1769, 10695, -1769, -1769, -1769,  6575,  5240, -1769, -1769,
   -1769,  1342, 17964, 19209, 19878, 19878, 19878,  1409, 19878, 19269,
   20197, -1769, -1769,   691, 15957, 15957,  4183, -1769,  1535, 17113,
     100, -1769, 16243,   893,  4705, -1769,  1370, -1769,   138,  1353,
     140, -1769, 16612, -1769, -1769, -1769,   142, -1769, -1769,  2577,
   -1769,  1349, -1769,  1540,   143,   777,  1448, 16433, -1769, 16433,
   -1769, -1769,  1543,   897,  4862, -1769, 15706, -1769, -1769, -1769,
   -1769,  2771, -1769,  1546,  1480, 15046, -1769, 19784,  1364,  1365,
    1368,  1208,  1371, -1769,  1419,  1208, -1769, -1769, -1769, -1769,
    2241,  1366, 18040,  1432, -1769, -1769, -1769,  1433, -1769,  6575,
   10901, 10695, -1769, -1769, -1769,  6575, -1769, -1769, 19878, 17964,
   17964, 17964,  8429,  1372,  1373, -1769, 17964, -1769, 15957, -1769,
   -1769, -1769, -1769, -1769,  5240,  2151,  1435, -1769, -1769, -1769,
   -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769,   187, -1769,  1358, -1769, -1769, -1769, -1769, -1769,   122,
     534, -1769, 17964,  1489, -1769, 16805,   144,  1568, -1769,  5240,
     777,   145,  1448, -1769, -1769,  1384,  1570, 15046, -1769, 19784,
   -1769, -1769,   159,  1385,  5240,   617,  1208,  1371, 15885, -1769,
    1208, -1769, 18040, 18040, -1769, -1769, -1769, -1769,  8635, 19878,
   19878, 19878, -1769, -1769, -1769, 19878, -1769,  2875,  1577,  1579,
    1387, -1769, -1769, 17964, 16612, 16612,  1521, -1769,  2577,  2577,
     721, -1769, -1769, -1769, 19878,  1578,  1410,  1396, -1769, 17964,
   -1769, 16805, -1769,   146, -1769, 17964, 19784,  1511, -1769,  1587,
   -1769,  1588, -1769,   543, -1769, -1769, -1769,  1397,   617, -1769,
     617, -1769, -1769,  8841,  1412,  1484, -1769,  1513,  1461, -1769,
   -1769,  1523,  5240,  1440,  2151, -1769, -1769, 19878, -1769, -1769,
    1451, -1769,  1591, -1769, -1769, -1769, -1769, 17964,   415, -1769,
   19878,  1429, -1769, 19878, -1769,   160,  1436,  9047,  5240, -1769,
    5240, -1769,  9253, -1769, -1769, -1769,  1428, -1769,  1438,  1458,
    4183,   742,  1457, -1769, -1769, -1769, 19878,  1459,   133, -1769,
    1549, -1769, -1769, -1769, -1769, -1769, -1769,  9459, -1769, 15957,
    1009, -1769,  1463,  4183,   684, -1769, -1769,  1442,  1635,   493,
     133, -1769, -1769,  1565, -1769, 15957,  1449, -1769,  1208,   134,
   -1769,  5240, -1769, -1769, -1769,  5240, -1769,  1453,  1460,   147,
   -1769,  1371,   599,  1566,   202,  1208,  1465, -1769,   703,  5240,
    5240, -1769,   276,  1640,  1575,  1371, -1769, -1769, -1769, -1769,
    1576,   206,  1647,  1580, 15046, -1769,   703,  9665,  9871, -1769,
     414,  1651,  1586, 15046, -1769, 19784, -1769, -1769, -1769,  1656,
    1589, 15046, -1769, 19784, 15046, -1769, 19784, 19784
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   204,   473,     0,   914,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1010,
     998,     0,   778,     0,   784,   785,   786,    29,   851,   985,
     986,   171,   172,   787,     0,   152,     0,     0,     0,     0,
      30,     0,     0,     0,     0,   223,     0,     0,     0,     0,
       0,     0,   442,   443,   444,   441,   440,   439,     0,     0,
       0,     0,   252,     0,     0,     0,    37,    38,    40,    41,
      39,   791,   793,   794,   788,   789,     0,     0,     0,   795,
     790,     0,   761,    32,    33,    34,    36,    35,     0,   792,
       0,     0,     0,     0,     0,   796,   445,   583,    31,     0,
     170,   140,   990,   779,     0,     0,     4,   126,   128,   850,
       0,   760,     0,     6,     0,     0,   222,     7,     9,     8,
      10,     0,     0,   437,   967,   487,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   543,   485,   972,   973,   565,
     558,   559,   560,   561,   564,   562,   563,   468,   568,     0,
     467,   942,   762,   769,     0,   853,   557,   436,   945,   946,
     958,   486,     0,     0,     0,   489,   488,   943,   944,   941,
     980,   984,     0,   547,   852,    11,   442,   443,   444,     0,
       0,    36,     0,   126,   222,     0,  1050,   486,  1051,     0,
    1053,  1054,   567,   481,     0,   474,   479,     0,     0,   529,
     530,   531,   532,    29,   985,   787,   764,    37,    38,    40,
      41,    39,     0,     0,  1074,   963,   762,     0,   763,   508,
       0,   510,   548,   549,   550,   551,   552,   553,   554,   556,
       0,  1014,     0,   860,   774,   242,     0,  1074,   465,   773,
     767,     0,   783,   763,   993,   994,  1000,   992,   775,     0,
     466,     0,   777,   555,     0,   205,     0,     0,   470,   205,
     150,   472,     0,     0,   156,   158,     0,     0,   160,     0,
      75,    76,    82,    83,    67,    68,    59,    80,    91,    92,
       0,    62,     0,    66,    74,    72,    94,    86,    85,    57,
     108,    81,   101,   102,    58,    97,    55,    98,    56,    99,
      54,   103,    90,    95,   100,    87,    88,    61,    89,    93,
      53,    84,    69,   104,    77,   106,    70,    60,    47,    48,
      49,    50,    51,    52,    71,   107,   105,   110,    64,    45,
      46,    73,  1127,  1128,    65,  1132,    44,    63,    96,     0,
      79,     0,   126,   109,  1065,  1126,     0,  1129,     0,     0,
       0,   162,     0,     0,     0,   213,     0,     0,     0,     0,
       0,     0,     0,     0,   862,     0,   114,   116,   350,     0,
       0,   349,   355,     0,     0,   253,     0,   256,     0,     0,
       0,     0,  1071,   238,   250,  1006,  1010,   602,   632,   632,
     602,   632,     0,  1035,     0,   798,     0,     0,     0,  1033,
       0,    16,     0,   130,   230,   244,   251,   662,   595,   632,
       0,  1059,   575,   577,   579,   918,   473,   487,     0,     0,
     485,   486,   488,   205,     0,   780,     0,   781,     0,     0,
       0,   202,     0,     0,   132,   339,     0,    28,     0,     0,
       0,     0,     0,   203,   221,     0,   249,   234,   248,   442,
     445,   222,   438,   989,     0,   934,   192,   193,   194,   195,
     196,   198,   199,   201,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   998,     0,   191,   989,   989,  1020,     0,     0,
       0,     0,     0,     0,     0,     0,   435,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     507,   509,   919,   920,     0,   933,   932,   339,   339,   989,
       0,   991,   981,  1006,     0,   222,     0,     0,   164,     0,
     916,   911,   860,     0,   487,   485,     0,  1018,     0,   600,
     859,  1009,   783,   487,   485,   486,   132,     0,   339,   464,
       0,   935,   776,     0,   140,   292,     0,   582,     0,   167,
       0,   205,   471,     0,     0,     0,     0,     0,   159,   190,
     161,  1127,  1128,  1124,  1125,     0,  1131,  1117,     0,     0,
       0,     0,    78,    43,    65,    42,  1066,   197,   200,   163,
     140,     0,   180,   189,     0,     0,     0,     0,     0,     0,
     117,     0,     0,     0,   861,   115,    18,     0,   111,     0,
     351,     0,   165,     0,     0,   166,   254,   255,  1055,     0,
       0,   487,   485,   486,   489,   488,     0,  1107,   262,     0,
    1007,     0,     0,     0,     0,   860,   860,     0,     0,     0,
       0,   168,     0,     0,   797,  1034,   851,     0,     0,  1032,
     856,  1031,   129,     5,    13,    14,     0,   260,     0,     0,
     588,     0,     0,     0,   860,     0,   771,     0,   770,   765,
     589,     0,     0,     0,     0,     0,   918,   136,     0,   862,
     917,  1136,   463,   476,   490,   951,   971,   147,   139,   143,
     144,   145,   146,   436,     0,   566,   854,   855,   127,   860,
       0,  1075,     0,     0,     0,     0,   862,   340,     0,     0,
       0,   487,   209,   210,   208,   485,   486,   205,   184,   182,
     183,   185,   571,   224,   258,     0,   988,     0,     0,   513,
     515,   514,   526,     0,     0,   546,   511,   512,   516,   518,
     517,   535,   536,   533,   534,   537,   538,   539,   540,   541,
     527,   528,   520,   521,   519,   522,   523,   525,   542,   524,
       0,     0,  1024,     0,   860,  1058,     0,  1057,  1074,   948,
     980,   240,   232,   246,     0,  1059,   236,   222,     0,   477,
     480,   482,   492,   495,   496,   497,   498,   499,   500,   501,
     502,   503,   504,   505,   506,   922,     0,   921,   924,   947,
     928,  1074,   925,     0,     0,     0,     0,     0,     0,  1052,
     475,   909,   913,   859,   915,   462,   766,     0,  1013,     0,
    1012,   258,     0,   766,   997,   996,     0,     0,   921,   924,
     995,   925,   484,   294,   296,   136,   586,   585,   469,     0,
     140,   276,   151,   472,     0,     0,     0,     0,   205,   288,
     288,   157,   860,     0,     0,  1116,     0,  1113,   860,     0,
    1087,     0,     0,     0,     0,     0,   858,     0,    37,    38,
      40,    41,    39,     0,     0,     0,   800,   804,   805,   806,
     809,   807,   808,   811,     0,   799,   134,   849,   810,  1074,
    1130,   205,     0,     0,     0,    21,     0,    22,     0,    19,
       0,   112,     0,    20,     0,     0,     0,   123,   862,     0,
     121,   116,   113,   118,     0,   348,   356,   353,     0,     0,
    1044,  1049,  1046,  1045,  1048,  1047,    12,  1105,  1106,     0,
     860,     0,     0,     0,  1006,  1003,     0,   599,     0,   613,
     859,   601,   859,   631,   616,   625,   628,   619,  1043,  1042,
    1041,     0,  1037,     0,  1038,  1040,   205,     5,     0,     0,
       0,   657,   658,   667,   666,     0,     0,   485,     0,   859,
     594,   598,     0,   622,     0,  1060,     0,   576,     0,   205,
    1094,   918,   320,  1136,  1135,     0,     0,     0,   987,   859,
    1077,  1073,   342,   336,   337,   341,   343,   759,   861,   338,
       0,     0,     0,     0,   462,     0,     0,   490,     0,   952,
     212,   205,   142,   918,     0,     0,   260,   573,   226,   930,
     931,   545,     0,   639,   640,     0,   637,   859,  1019,     0,
       0,   339,   262,     0,   260,     0,     0,   258,     0,   998,
     493,     0,     0,   949,   950,   982,   983,     0,     0,     0,
     897,   867,   868,   869,   876,     0,    37,    38,    40,    41,
      39,     0,     0,     0,   882,   888,   889,   890,   893,   891,
     892,     0,   880,   878,   879,   903,   860,     0,   911,  1017,
    1016,     0,   260,     0,   936,     0,   782,     0,   298,     0,
     205,   148,   205,     0,   205,     0,     0,     0,     0,   268,
     269,   280,     0,   140,   278,   177,   288,     0,   288,     0,
     859,     0,     0,     0,     0,     0,   859,  1115,  1118,  1083,
     860,     0,  1078,     0,   860,   832,   833,   830,   831,   866,
       0,   860,   858,   606,   634,   634,   606,   634,   597,   634,
       0,     0,  1026,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1121,   214,     0,   217,   181,     0,     0,     0,   119,
       0,   124,   125,   117,   861,   122,     0,   352,     0,  1056,
     169,  1072,  1107,  1098,  1102,   261,   263,   362,     0,     0,
    1004,     0,   604,     0,  1036,     0,    17,   205,  1059,   259,
     362,     0,     0,     0,   766,   591,     0,   772,  1061,     0,
    1094,   580,   135,   137,     0,     0,     0,  1136,     0,     0,
     325,   323,   924,   937,  1074,   924,   938,  1074,  1076,   989,
       0,     0,     0,   344,   133,   207,   209,   210,   486,   188,
     206,   186,   187,   211,   141,     0,   918,   257,     0,   918,
       0,   544,  1023,  1022,     0,   339,     0,     0,     0,     0,
       0,     0,   260,   228,   783,   923,   339,     0,   872,   873,
     874,   875,   883,   884,   901,     0,   860,     0,   897,   610,
     636,   636,   610,   636,     0,   871,   905,   636,     0,   859,
     908,   910,   912,     0,  1011,     0,   923,     0,     0,     0,
     205,   295,   587,   153,     0,   472,   268,   270,  1006,     0,
       0,     0,   205,     0,     0,     0,     0,     0,   282,     0,
    1122,     0,     0,  1108,     0,  1114,  1112,  1079,   859,  1085,
       0,  1086,     0,     0,   802,   859,   857,     0,     0,   860,
       0,     0,   846,   860,     0,     0,     0,     0,   860,     0,
       0,   812,   847,   848,  1030,     0,   860,   815,   817,   816,
       0,     0,   813,   814,   818,   820,   819,   836,   837,   834,
     835,   838,   839,   840,   841,   842,   827,   828,   822,   823,
     821,   824,   825,   826,   829,  1120,     0,   140,     0,     0,
       0,     0,   120,    23,   354,     0,     0,     0,  1099,  1104,
       0,   436,  1008,  1006,   478,   483,   491,     0,     0,    15,
       0,   436,   670,     0,     0,   672,   665,     0,   668,     0,
     664,     0,  1063,     0,     0,     0,   967,   543,     0,   489,
    1095,   584,  1136,     0,   326,   327,     0,     0,   321,     0,
       0,     0,   346,   347,   345,  1094,     0,   362,     0,   918,
       0,   339,     0,   978,   362,  1059,   362,  1062,     0,     0,
       0,   494,     0,     0,   886,   859,   896,   877,     0,     0,
     860,     0,     0,   895,   860,     0,     0,     0,   870,     0,
       0,   860,     0,   881,   902,  1015,   362,     0,   140,     0,
     291,   277,     0,     0,     0,   267,   173,   281,     0,     0,
     284,     0,   289,   290,   140,   283,  1123,  1109,     0,     0,
    1082,  1081,     0,     0,  1134,   865,   864,   801,   614,   859,
     605,     0,   617,   859,   633,   626,   629,   620,     0,   859,
     596,   803,   623,     0,   638,   859,  1025,   844,     0,     0,
     205,    24,    25,    26,    27,  1101,  1096,  1097,  1100,   264,
       0,     0,     0,   443,   434,     0,     0,     0,   239,   361,
       0,     0,   433,     0,     0,     0,  1059,   436,     0,   603,
    1039,   358,   245,   660,     0,   663,     0,   590,   578,   486,
     138,   205,     0,     0,   330,   319,     0,   322,   329,   339,
     339,   335,   570,  1094,   436,  1094,     0,  1021,     0,   977,
     436,     0,   436,  1064,   362,   918,   974,   900,   899,   885,
     615,   859,   609,     0,   618,   859,   635,   627,   630,   621,
       0,   887,   859,   904,   624,   436,   140,   205,   149,   154,
     175,   271,   205,   279,   285,   140,   287,     0,  1110,  1080,
    1084,     0,     0,     0,   608,   845,   593,     0,  1029,  1028,
     843,   140,   218,  1103,     0,     0,     0,  1067,     0,     0,
       0,   265,     0,  1059,     0,   399,   395,   401,   761,    36,
       0,   389,     0,   394,   398,   411,     0,   409,   414,     0,
     413,     0,   412,   453,     0,   222,     0,     0,   367,     0,
     368,   369,     0,     0,   435,  1005,     0,   661,   659,   671,
     669,     0,   331,   332,     0,     0,   317,   328,     0,     0,
       0,  1094,  1088,   235,   570,  1094,   979,   241,   358,   247,
     436,     0,     0,     0,   612,   894,   907,     0,   243,   293,
     205,   205,   140,   274,   174,   286,  1111,  1133,   863,     0,
       0,     0,   205,     0,     0,   461,     0,  1068,     0,   379,
     383,   458,   459,   393,     0,     0,     0,   374,   720,   721,
     719,   722,   723,   740,   742,   741,   711,   683,   681,   682,
     701,   716,   717,   677,   688,   689,   691,   690,   758,   710,
     694,   692,   693,   695,   696,   697,   698,   699,   700,   702,
     703,   704,   705,   706,   707,   709,   708,   678,   679,   680,
     684,   685,   687,   757,   725,   726,   730,   731,   732,   733,
     734,   735,   718,   737,   727,   728,   729,   712,   713,   714,
     715,   738,   739,   743,   745,   744,   746,   747,   724,   749,
     748,   751,   753,   752,   686,   756,   754,   755,   750,   736,
     676,   406,   673,     0,   375,   427,   428,   426,   419,     0,
     420,   376,     0,     0,   363,     0,     0,     0,   457,     0,
     222,     0,     0,   231,   357,     0,     0,     0,   318,   334,
     975,   976,     0,     0,     0,     0,  1094,  1088,     0,   237,
    1094,   898,     0,     0,   140,   272,   155,   176,   205,   607,
     592,  1027,   216,   377,   378,   456,   266,     0,   860,   860,
       0,   402,   390,     0,     0,     0,   408,   410,     0,     0,
     415,   422,   423,   421,   454,   451,  1069,     0,   364,     0,
     460,     0,   365,     0,   359,     0,   333,     0,   655,   862,
     136,   862,  1090,     0,   429,   136,   225,     0,     0,   233,
       0,   611,   906,   205,     0,   178,   380,   126,     0,   381,
     382,     0,   859,     0,   859,   404,   400,   405,   674,   675,
       0,   391,   424,   425,   417,   418,   416,     0,  1107,   370,
     455,     0,   366,   360,   656,   861,     0,   205,   861,  1089,
       0,  1093,   205,   136,   227,   229,     0,   275,     0,   220,
       0,   436,     0,   396,   403,   407,   452,     0,   918,   372,
       0,   653,   569,   572,  1091,  1092,   430,   205,   273,     0,
       0,   179,   387,     0,   435,   397,  1070,     0,   862,   447,
     918,   654,   574,     0,   219,     0,     0,   386,  1094,   918,
     302,  1136,   450,   449,   448,  1136,   446,     0,     0,     0,
     385,  1088,   447,     0,     0,  1094,     0,   384,     0,  1136,
    1136,   308,     0,   307,   305,  1088,   140,   431,   136,   371,
       0,     0,   309,     0,     0,   303,     0,   205,   205,   313,
       0,   312,   301,     0,   304,   311,   373,   215,   432,   314,
       0,     0,   299,   310,     0,   300,   316,   315
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1769, -1769, -1769,  -570, -1769, -1769, -1769,   552,  -467,   -41,
     521, -1769,  -278,  -525, -1769, -1769,   467,  1028,  2005, -1769,
    3355, -1769,  -817, -1769,  -530, -1769,  -735,    26, -1769, -1769,
   -1769, -1769, -1769, -1769, -1769, -1769,  -936, -1769, -1769,  -908,
    -318, -1769, -1769, -1769,  -367, -1769, -1769,  -185,   162,    51,
   -1769, -1769, -1769, -1769, -1769, -1769,    55, -1769, -1769, -1769,
   -1769, -1769, -1769,    56, -1769, -1769,  1163,  1169,  1170,   -93,
    -734,  -934,   628,   705,  -372,   346, -1008, -1769,   -76, -1769,
   -1769, -1769, -1769,  -774,   155, -1769, -1769, -1769, -1769,  -361,
   -1769,  -627, -1769,   434,  -462, -1769, -1769,  1063, -1769,   -52,
   -1769, -1769, -1135, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769,   -89, -1769,     4, -1769, -1769, -1769, -1769, -1769,  -174,
   -1769,   117, -1068, -1769, -1715,  -396, -1769,  -138,   412,  -107,
    -370, -1769, -1536, -1769, -1769, -1769,   124,   -92,   -81,    -2,
    -781,   -56, -1769, -1769,    44, -1769,    20,  -400, -1769,     0,
      -5,   -84,   -82,   -29, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769, -1769, -1769,  -633,  -906, -1769, -1769, -1769, -1769, -1769,
     370,  1325, -1769,   571, -1769,   416, -1769, -1769, -1769, -1769,
   -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769, -1769, -1769, -1769, -1769,   238,  -416,  -713, -1769, -1769,
   -1769, -1769, -1769,   495, -1769, -1769, -1769, -1769, -1769, -1769,
   -1769, -1769, -1020,  -404,  2972,    36, -1769,   352,  -437, -1769,
   -1769,  -501,  4004,  3019, -1769,   181, -1769, -1769,   579,  -422,
    -656, -1769, -1769,   660,   426,   427, -1769,   432, -1769, -1769,
   -1769, -1769, -1769,   645, -1769, -1769, -1769,    82,  -930,  -173,
    -447,  -445, -1769,   -90,  -116, -1769, -1769,    38,    43,   680,
     -66, -1769, -1769,   308,   -75, -1769,  -383,   867,  -132, -1769,
    -427, -1769, -1769, -1769,  -446,  1351, -1769, -1769, -1769, -1769,
   -1769,   673,   542, -1769, -1769, -1769,  -373,  -723, -1769,  1298,
   -1294,  -213,   -67,  -154,   864, -1769, -1769, -1769, -1768, -1769,
    -260, -1114, -1362,  -249,   175, -1769,   539,   616, -1769, -1769,
   -1769, -1769,   566, -1769,  1699,  -780
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   116,   977,   673,   193,   353,   788,
     373,   374,   375,   376,   928,   929,   930,   118,   119,   120,
     121,   122,   999,  1242,   433,  1031,   708,   709,   581,   269,
    1760,   587,  1662,  1761,  2019,   913,   124,   125,   729,   730,
     738,   366,   610,  1975,  1195,  1417,  2041,   455,   194,   710,
    1034,  1280,  1490,   128,   676,  1053,   711,   744,  1057,   648,
    1052,   248,   562,   712,   677,  1054,   457,   393,   415,   131,
    1036,   980,   953,  1215,  1690,  1340,  1119,  1916,  1764,   862,
    1125,   586,   871,  1127,  1534,   854,  1108,  1111,  1329,  2047,
    2048,   698,   699,  1015,   725,   726,   380,   381,   383,  1726,
    1894,  1895,  1431,  1589,  2028,  2050,  1927,  1979,  1980,  1981,
    1700,  1701,  1702,  1703,  1929,  1930,  1936,  1991,  1706,  1707,
    1711,  1878,  1879,  1880,  1966,  2089,  1590,  1591,   195,   133,
    2065,  2066,  1714,  1593,  1594,  1595,  1596,   134,   135,   656,
     583,   136,   137,   138,   139,   140,   141,   142,   143,   262,
     144,   145,   146,  1741,   147,  1033,  1279,   148,   695,   696,
     697,   266,   425,   577,   683,   684,  1378,   685,  1379,   149,
     150,   654,   655,  1368,  1369,  1499,  1500,   151,   897,  1085,
     152,   898,  1086,   153,   899,  1087,   154,   900,  1088,   155,
     901,  1089,   156,   902,  1090,   657,  1371,  1502,   157,   903,
     158,   159,  1959,   160,   678,  1728,   679,  1231,   986,  1450,
    1446,  1871,  1872,   161,   162,   163,   251,   164,   252,   263,
     436,   569,   165,  1372,  1373,   907,   908,   166,  1150,   561,
     625,  1151,  1093,  1302,  1094,  1503,  1504,  1305,  1306,  1096,
    1510,  1511,  1097,   832,   552,   207,   208,   713,   701,   534,
    1252,  1253,   820,   821,   465,   168,   254,   169,   170,   197,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     747,   182,   258,   259,   651,   242,   243,   783,   784,  1385,
    1386,   408,   409,   971,   183,   639,   184,   694,   185,   356,
    1896,  1947,   394,   444,   719,   720,  1140,  1141,  1905,  1961,
    1962,  1246,  1428,   949,  1429,   950,   951,   877,   878,   879,
     357,   358,   910,   596,  1004,  1005
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     196,   198,  1032,   200,   201,   202,   203,   205,   206,   544,
     209,   210,   211,   212,   686,   354,   232,   233,   234,   235,
     236,   237,   238,   239,   241,   462,   260,   430,   427,   515,
     123,   416,   449,   265,   432,   688,   420,   421,  1112,   268,
     450,   787,   267,  1002,   853,   690,   270,   276,   733,   279,
     542,   274,   364,   428,   367,   127,   535,   536,   451,   129,
     130,   841,   997,  1244,   458,   778,   250,  1578,   255,   998,
    1019,  1247,  1056,   256,   514,   823,   824,   363,   780,   781,
     911,   268,  1115,  1236,   402,   570,   167,   818,   462,   819,
     615,   617,   619,   566,   622,  1441,  1129,   827,   927,   932,
    1143,   362,  1278,   976,   429,   869,   846,  1102,  1336,  1778,
     430,   427,   825,  1265,   -78,  1270,   867,   432,   446,   -78,
    1289,   578,   -43,   849,   -42,   850,  1453,   -43,   631,   -42,
     834,  1938,   739,   740,   741,   938,  1532,   634,    14,  1968,
     578,   555,   554,   571,   578,  1717,  1719,  -392,   432,  1786,
     982,  1873,  1883,  1883,  1883,  1883,  1778,  1603,  1939,   955,
    1513,   463,  1021,  -641,   564,   563,   126,  1465,  1325,   662,
     382,    14,   955,   955,   955,  1733,  1244,   955,   214,    40,
    1886,    14,    14,   627,  1345,  1346,  1249,   429,  1891,   214,
      40,   947,   948,   553,   547,    55,  -126,  1957,  2030, -1074,
    -126,  1933,   199,  1190,   459,   187,   188,    65,    66,    67,
    1161,  1382,     3,  -110,  -763,   261,  1314,  -126,   429,  1934,
     443,  -772,    14,  1248,   443,   532,   533,   532,   533,  -110,
     663,  1250,   264,   961,   963,   532,   533,  1613,  1935, -1074,
    2082,   429,  1958,  2031,  2100,   502,   628,   463,  -955,  -764,
    1162,  -964,  1377,  2014,  -956,  2015,   573,   503,  1051,   573,
    1466,   405,   990,   983,  1243,  -999,   268,   584,  1734,   582,
     417,   539,  1205,  1345,  1346,   870,   745,   460,   984,  -954,
    1348,  -957,  1614,  -952,  1315,  2083,   689,  1688,   595,  2101,
    -324,   575,  -963,   464,   461,   580,  1274,  1010,   985,  -959,
     541,  1779,  1780,  2078,  -649,  1533,   -78,   868,   642,   641,
     447,   112,  -649,   579,   -43,   645,   -42,  2096,   606,  -324,
     632,  1578,  1940,  1292,  1251,  -859,   939,   230,   230,   635,
    1114,  1525,   661,  -306,  -861,   940,  1455,  1718,  1720,  -392,
    -770,  1787,  1624,  1874,  1884,  1948,  1952,  2002,  2077,  1630,
     956,  1632,  1343,  1022,  1347,   203,  1953,   545,  1489,  2092,
     828,  1622,  1048,  1067,  1432,  1661,   735,   731,  1725,  1535,
    -861,   227,   227,   432,  -861,  1615,   790,  1243,  2084,   464,
    -955,  1655,  2102,  -961,   268,   429,  -956,   640,   743,   225,
     225,   241,   653,   268,   268,   653,   268,  -999,  1781,  1201,
    1202,   667,   790,   540, -1002,   354,  1275,  1227,  -968,   462,
    1509,  -954,  -965,  -957,   268,  -952,   132, -1001,  -953,  -939,
    -940,   205,   271,  1887,   790,  1888,   463,   422,  -970,   714,
    -649,  -959,   452,   538,  -652,   790,   737,  -462,   790,  -109,
     727,   538,   944,   734,   416,   794,   795,   458,   378,   403,
    1131,   539,  2093,   384,  -581,  -109,  1137,   669,   746,   748,
    -650,   799,   385,   532,   533,   947,   948,  1463,   732,   749,
     750,   751,   752,   754,   755,   756,   757,   758,   759,   760,
     761,   762,   763,   764,   765,   766,   767,   768,   769,   770,
     771,   772,   773,   774,   775,   776,   777,  2109,   779,  1750,
     746,   746,   782,   532,   533,  1440,   272,   700,  1218,  1742,
    -771,  1744,   802,   803,   804,   805,   806,   807,   808,   809,
     810,   811,   812,   813,   814,   787,   406,   407,  1213,  1006,
     975,  1007,   727,   727,   746,   826, -1002,   801,   250,   423,
     255,   802,   273,  -765,   830,   256,   424,  -962,   230, -1001,
    -953,  -939,  -940,   838,  1522,   840,   117,  2010,  1611,   538,
    1255,   800,  1256,   727,   611,  1941,  -462,   532,   533,  -462,
     379,   857,   920,   858,   717,   515,  -651,   365,  1542,   516,
    2061,  1448,   627,   540,  1942,   686,   388,  1943,   403,  1286,
    2110,   920,   227,  1342,   403,   126,   856,   389,  1505,   987,
    1507,  2011,   435,   403,  1512,   277,   688,   861,   352,   390,
     225,   669,  1058,  1461,   403,  1449,   690,  2062,  2063,  2064,
     514,  1267,  1294,  1267,   934,   392,   397,  1903,   612,   391,
    -966,  1907,   395,   403,  1050,   921,  1196,   658,  1197,   660,
    1198,   404,   532,   533,  1200,   442,  1426,  1427,   414,  1476,
     392,   386,  1478,  1677,   442,   392,   392,   691,  1255,  -926,
    1256,   387,  1006,  1007,   664,   406,   407,  1062,  -766,  1103,
    1105,   406,   407,   396,  1320,  -926,   429,   797,  1038,   927,
     406,   407,  1612,   392,   171,  -929,  2079,   230,   398,   403,
     399,   406,   407,   417,  1383,   400,   230,   438,   644,   229,
     231,  -929,   401,   230,  1269,  -966,   403,  1271,  1272,   405,
     406,   407,   418,   403,   669,   230,   419,  1442,  1359,  1016,
     434,   669,  1362,  2062,  2063,  2064,  1104,   716,   442,  1366,
    1443,   227, -1074,   441,  -927,  1298,  1299,  1300,   213,  1374,
     227,  1376,   566,  1380,  1013,  1014,  1757,   227,  1041,   225,
    -927,  1444,  1994,   443,   686,  1191,  1109,  1110,   225,   227,
     551,    50,  1631,   674,   675,   225,   406,   407,   442, -1074,
     687,  1995, -1074,   445,  1996,   688,   448,   225,   700,  1327,
    1328,  1049,   670,   406,   407,   690,   437,   439,   440,   431,
     406,   407,  1967,  1491,  1426,  1427,  1970,   453,   217,   218,
     219,   220,   221,  1684,  1685,   459,   187,   188,    65,    66,
      67,  1061,   499,   500,   501,   454,   502,   117,  1964,  1965,
    1608,   117,   466,  1482,  -642,   585,    93,    94,   503,    95,
     191,    97,  1471,  -643,  1492,  1183,  1184,  1185,  1186,  1187,
    1188,   467,  1107,   468,  1267,   132,   213,  1344,  1345,  1346,
     469,   230,  1626,   470,  1189,   108,   471,   689,   268,  1113,
     790,   582,  -644,   459,   187,   188,    65,    66,    67,    50,
    1992,  1993,   431,  1722,   790,   790,  1032,   472,   460,   473,
    1186,  1187,  1188,  -647,  1496,  2057,  -645,  1570,  -646,   614,
     616,   618,  1524,   621,  1124,   227,  1189,   257,   505,   126,
    1529,  1345,  1346,   431,  2087,  2088,   217,   218,   219,   220,
     221,   605,   686,   225,  1988,  1989,  1254,  1257,   506,   507,
     557,   537,   508,  -960,  -648,   543,   565,   410,  -764,   548,
     550,   410,   503,   688,    93,    94,   460,    95,   191,    97,
     443,   556,  -964,   690,  2071,   171,   665,  1550,   538,   171,
     671,  1554,   559,  1222,   560,  1223,  1560,   858,  -762,   567,
     576,  2085, -1119,   108,  1566,   589,   568,   411,  1225,   597,
    1782,   790,   600,   790,   601,   607,   608,   665,   623,   671,
     665,   671,   671,  1235,   624,   117,   633,  1598,  1657,   626,
     636,  1293,   637,   646,   718,   692,   693,   647,   352,   702,
     737,   703,   123,   704,  1666,   706,  -131,   392,  1751,    55,
     715,   833,  1092,  1263,   742,   126,   734,   835,   734,  1628,
     831,   664,   836,   802,   842,   843,   689,   127,   859,   578,
     230,   129,   130,   863,   595,   866,   880,  1281,   881,   912,
    1282,  1266,  1283,  1266,   914,   915,   727,   916,   801,   918,
     917,   919,   937,   923,   733,   922,   941,   942,   167,   630,
     605,   392,   792,   392,   392,   392,   392,   946,   638,   945,
     643,  1244,   952,   126,   227,   650,  1244,   954,  1642,   957,
     959,   960,  1646,   700,   962,   973,   817,   668,   377,  1653,
     964,   965,   225,   966,   967,   250,  1324,   255,  2049,   978,
    1469,  1244,   256,  1470,   981,   979,   230,   989,   605,  1330,
    -787,   988,   991,   171,  1687,   700,   412,   992,   993,   413,
    2049,   996,   848,  1000,  1001,   516,  1759,  1009,   736,  2072,
    1011,  1017,  1018,   117,  1020,  1765,  1331,  1023,   126,   739,
     740,   741,  1024,  2007,  1025,   230,  1035,   230,  2012,   132,
     227,  1772,  1456,   909,  1026,   686,  1027,  1738,  1739,  1047,
    1457,   126,  1244,  1039,  1043,  1434,  1044,  1046,   225,  1055,
    1065,  1063,  -768,   230,  1064,  1037,   688,  1106,  1458,   933,
     718,  1116,  1130,  1126,   689,  1128,   690,  1134,  1135,   227,
    1136,   227,  1138,   126,  1152,  1153,  2037,  1154,  1158,   496,
     497,   498,   499,   500,   501,  1155,   502,   225,  1156,   225,
    1157,  1194,  1159,  1204,   970,   972,  1436,   227,   503,  1206,
    1208,  1211,  1777,   650,  1210,  1687,  1212,  1447,  1221,  1217,
    1232,  1240,  1918,  1230,  1224,   225,  1233,  1238,  1234,   734,
    1245,  1435,  1259,  1276,  1285,  1288,   230,  1291,   686,  1296,
    1687,  1092,  1687,   123,   746,  -969,  1297,  1474,  1095,  1687,
    1308,   171,   230,   230,  1266,   132,  1318,  1307,  1309,   688,
    1310,  2098,  1243,  1311,   126,  1312,   126,  1243,   127,   690,
     727,  2073,   129,   130,  1316,  2074,  1313,  1317,  1319,   117,
     227,   727,  1436,  1321,  1339,  1333,  1335,   392,  1338,  2090,
    2091,  1341,  1243,  2006,  1351,  2009,   227,   227,   225,   167,
    1350,  1358,  1360,  1352,  1189, -1135,  1365,   459,    63,    64,
      65,    66,    67,   132,   225,   225,  1364,  1517,    72,   509,
     268,  1416,  1418,   582,  1361,  1419,  1420,  1421,  1423,  1430,
    1531,   687,   535,  1433,  1051,  1451,  1464,  1452,  1467,  1468,
    1477,  1494,  1475,  1479,  1495,  1484,  1520,  1483,   700,  1481,
    1487,   700,  1493,  1243,  1486,  1074,  1516,  1518,  1508,   257,
    1519,   511,   430,   427,  1526,   789,  1521,  1530,  1536,   432,
    1543,  1539,  1544,  1084,  1973,  1098,  1547,  1548,   132,   126,
     460,   213,  2060,   214,    40,  1549,   377,   377,   377,   620,
     377,   822,  1012,  1552,  1553,   117,  1555,  1558,  1559,  1565,
    1561,   132,  1616,  1641,    50,  1556,  1557,   171,  1562,  1122,
     117,  1569,  1564,   789,   230,   230,  1571,   689,  1572,  1573,
    1574,  1619,  1599,  1600,   845,  1617,  1601,   851,   672,  1604,
    1610,  1687,  1621,   132,  1606,  1620,  1607,  1623,  1635,   734,
    1625,   217,   218,   219,   220,   221,  1629,  1634,  1633,  1723,
    1636,  1639,  1618,   117,  1643,  1645,  1694,  1650,   227,   227,
    1651,  1640,  1199,   718,   732,  1627,   727,   815,  1060,    93,
      94,  1644,    95,   191,    97,   462,   225,   225,  1647,  1092,
    1092,  1092,  1092,  1092,  1092,  1648,  1649,  1304,  1092,  1652,
    1092,  1654,  1656,  1214,   126,  1658,  1983,  1985,   108,  1659,
     687,  1660,   816,  1597,  1663,   112,   213,  1099,  1664,  1100,
     689,  1667,  1670,  1597,   132,  1681,   132,  1692,   117,  1705,
    1885,  1713,  1721,   171,  1727,  1732,  1735,  1736,  1740,    50,
    1745,  1746,  1767,  1752,   605,  1120,  1748,  1770,   171,  1776,
    1881,   117,  1459,  1784,  1882,  1785,  2097,  1889,   817,   817,
    1897,   700,  1695,  1898,  1900,  1901,  1910,  1902,   230,  1904,
    1912,  1913,  1945,  1923,  1924,  1696,   217,   218,   219,   220,
     221,  1697,  1949,   117,  1955,  1954,  1982,  1960,  1984,  1990,
    1986,   171,  1997,  1998,  2004,  1999,  2005,  2008,   190,  2013,
    2018,    91,  1698,  1580,    93,    94,  1731,    95,  1699,    97,
     392,  1737,   227,  2017,   727,   727,  -388,   462,  1209,  1775,
    1301,  1301,  1084,  2020,  2023,  2025,  2021,  1939,  2029,  2038,
     225,   230,  2051,   108,   650,  1220,  2032,  2039,  2040,   132,
    2055,  2045,  2058,  2046,  2059,    14,   230,   230,  2068,  2081,
    2070,   931,   931,  2075,  2094,   848,   171,   848,  2095,  2099,
    2076,  2103,   117,  2104,   117,  2111,   117,  2086,   687,  2112,
    2114,  1422,  2115,  2054,  1092,   227,  1092,   796,   791,   171,
    1287,   793,  1523,  2069,  1229,  1917,  1665,  1354,  1763,  2067,
     227,   227,  1580,   225,   935,  1473,  1908,  1932,  1783,  1937,
    2106,  1268,  2080,  1268,  1712,  1951,  1597,  1693,   225,   225,
    1581,   171,  1597,   605,  1597,  1906,  1582,   700,   459,  1583,
     188,    65,    66,    67,  1584,   659,  1445,  1375,  1506,  1303,
    1899,  1367,   126,  1497,    14,  1304,  1501,  1597,  2001,  1501,
    1498,   230,   909,  1322,   728,  1144,  1514,   652,  2034,  2027,
    1683,  1425,  1356,  1715,   132,   359,  1415,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1585,  1586,     0,  1587,
       0,     0,     0,   126,     0,     0,     0,     0,     0,   117,
       0,     0,     0,     0,     0,   227,  1915,  1763,     0,     0,
     171,   460,   171,     0,   171,     0,  1120,  1337,     0,  1581,
    1588,     0,     0,   225,     0,  1582,     0,   459,  1583,   188,
      65,    66,    67,  1584,     0,     0,     0,     0,     0,   126,
       0,     0,  1092,     0,  1092,     0,  1092,     0,   126,     0,
       0,  1092,  1597,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1580,  1592,  1946,     0,     0,     0,     0,     0,
       0,     0,     0,  1592,     0,  1585,  1586,     0,  1587,  1237,
    1084,  1084,  1084,  1084,  1084,  1084,     0,     0,     0,  1084,
       0,  1084,     0,   822,   822,     0,     0,  1580,     0,     0,
     460,     0,   117,  2043,    14,  1890,     0,     0,     0,  1602,
       0,     0,  1956,     0,   117,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1538,     0,     0,   171,     0,     0,
    1946,   687,     0,     0,     0,     0,     0,     0,     0,    14,
    1637,   126,  1638,     0,  1268,     0,   257,   126,     0,     0,
       0,     0,     0,  1092,   126,     0,     0,   462,     0,  1580,
    1472,     0,   931,     0,   931,     0,   931,     0,     0,  1581,
     931,     0,   931,   931,  1203,  1582,     0,   459,  1583,   188,
      65,    66,    67,  1584,     0,     0,     0,     0,     0,     0,
     851,     0,   851,     0,     0,     0,     0,  1575,     0,     0,
       0,    14,   132,     0,  1581,     0,     0,     0,     0,   593,
    1582,   594,   459,  1583,   188,    65,    66,    67,  1584,     0,
       0,     0,     0,  1515,   687,  1585,  1586,     0,  1587,  1724,
     171,     0,     0,     0,     0,     0,     0,     0,   650,  1120,
       0,     0,   171,   132,   224,   224,     0,     0,     0,     0,
     460,     0,     0,     0,     0,   247,  1592,  1580,     0,  1743,
    1585,  1586,  1592,  1587,  1592,  1084,  1581,  1084,     0,     0,
     599,     0,  1582,     0,   459,  1583,   188,    65,    66,    67,
    1584,   247,     0,     0,     0,   460,     0,  1592,  1753,   132,
    1754,     0,  1755,     0,  1747,     0,     0,  1756,   132,    14,
     126,     0,     0,     0,     0,     0,     0,     0,     0,  2105,
       0,     0,     0,  1092,  1092,     0,     0,     0,  2113,     0,
       0,     0,  1585,  1586,     0,  1587,  2116,     0,     0,  2117,
     700,     0,     0,   650,     0,     0,     0,     0,     0,     0,
       0,     0,   117,     0,     0,     0,     0,   460,     0,     0,
       0,     0,   700,   352,  1609,   126,  1749,     0,     0,  1710,
       0,   700,   721,     0,  1581,   359,     0,     0,     0,     0,
    1582,     0,   459,  1583,   188,    65,    66,    67,  1584,     0,
     213,     0,  1592,   117,     0,     0,     0,     0,     0,   126,
       0,   132,     0,     0,   126,     0,     0,   132,     0,  1911,
       0,     0,     0,    50,   132,     0,     0,     0,     0,     0,
       0,     0,     0,  1084,     0,  1084,     0,  1084,     0,   126,
    1585,  1586,  1084,  1587,     0,     0,     0,     0,     0,   117,
       0,     0,     0,     0,   117,     0,     0,     0,   117,     0,
     217,   218,   219,   220,   221,   460,     0,     0,     0,    34,
      35,    36,   931,     0,  1758,     0,     0,     0,   392,     0,
       0,   605,     0,   215,   352,   224,   456,  1580,    93,    94,
     171,    95,   191,    97,  1870,     0,     0,     0,     0,   126,
     126,  1877,     0,     0,     0,     0,     0,     0,     0,   352,
       0,   352,     0,     0,     0,     0,     0,   108,   352,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
       0,   171,     0,     0,   872,   247,     0,   247,    81,    82,
      83,    84,    85,     0,  1084,     0,     0,     0,     0,   222,
       0,   117,   117,   117,     0,    89,    90,   117,     0,     0,
       0,     0,     0,     0,   117,     0,     0,     0,     0,    99,
     132,     0,     0,     0,     0,     0,     0,   171,     0,  1971,
    1972,     0,   171,     0,   105,     0,   171,     0,     0,     0,
       0,     0,     0,     0,  1581,     0,   247,     0,     0,     0,
    1582,     0,   459,  1583,   188,    65,    66,    67,  1584,     0,
     546,   518,   519,   520,   521,   522,   523,   524,   525,   526,
     527,   528,   529,     0,   224,   132,     0,     0,     0,     0,
       0,   994,   995,   224,     0,     0,     0,     0,     0,     0,
     224,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1585,  1586,   224,  1587,     0,   530,   531,     0,     0,   132,
       0,     0,     0,   224,   132,     0,     0,     0,     0,     0,
       0,     0,     0,  2044,     0,   460,     0,   605,     0,   171,
     171,   171,     0,     0,  1909,   171,     0,     0,   247,   132,
       0,   247,   171,     0,     0,     0,     0,     0,     0,     0,
     352,     0,     0,     0,  1084,  1084,     0,     0,     0,     0,
     117,     0,     0,     0,     0,     0,     0,     0,     0,  1977,
       0,     0,     0,     0,     0,     0,  1870,  1870,     0,     0,
    1877,  1877,   532,   533,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   605,     0,     0,     0,   247,     0,   132,
     132,     0,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   117,     0,     0,     0,   546,
     518,   519,   520,   521,   522,   523,   524,   525,   526,   527,
     528,   529,     0,     0,     0,     0,     0,     0,   224,     0,
       0,     0,     0,     0,     0,     0,   705,   530,   531,   117,
       0,     0,     0,     0,   117,     0,     0,     0,     0,     0,
       0,     0,  2042,     0,   530,   531,     0,     0,     0,  1142,
     721,     0,     0,     0,     0,     0,   474,   475,   476,   117,
       0,     0,     0,     0,     0,  2056,     0,     0,   171,     0,
     247,     0,   247,     0,     0,   896,   477,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,   532,   533,     0,     0,   896,     0,
     213,     0,   214,    40,     0,   503,     0,     0,     0,   117,
     117,   532,   533,   171,     0,     0,     0,     0,   213,     0,
       0,     0,     0,    50,   546,   518,   519,   520,   521,   522,
     523,   524,   525,   526,   527,   528,   529,  1228,     0,     0,
       0,    50,     0,     0,     0,     0,     0,   171,     0,     0,
       0,     0,   171,     0,     0,  1239,     0,   247,   247,     0,
     217,   218,   219,   220,   221,     0,   247,     0,  1258,   530,
     531,     0,     0,     0,     0,   844,     0,   171,   217,   218,
     219,   220,   221,     0,     0,     0,   815,   224,    93,    94,
       0,    95,   191,    97,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1875,     0,    93,    94,  1876,    95,
     191,    97,     0,     0,  1290,     0,     0,   108,     0,     0,
       0,   847,     0,     0,   112,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   108,  1709,   171,   171,     0,
    1438,   474,   475,   476,     0,     0,   532,   533,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   477,   478,   224,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,  1349,
       0,     0,     0,  1353,     0,     0,   247,     0,  1357,     0,
     503,     0,   224,     0,   224,     0,     0,     0,     0,     0,
     943,     0,     0,  1028,   518,   519,   520,   521,   522,   523,
     524,   525,   526,   527,   528,   529,     0,     0,     0,     0,
     224,   896,     0,     0,     0,     0,     0,     0,   247,     0,
       0,     0,     0,     0,     0,   247,   247,   896,   896,   896,
     896,   896,     0,     0,     0,   474,   475,   476,   530,   531,
     896,  1028,   518,   519,   520,   521,   522,   523,   524,   525,
     526,   527,   528,   529,     0,   477,   478,   247,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,   224,  1460,     0,   530,   531,     0,     0,
       0,     0,     0,     0,   503,     0,   213,   247,     0,   224,
     224,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   504,     0,  1040,   532,   533,     0,     0,    50,
       0,     0,     0,   247,   247,     0,     0,  1485,     0,     0,
    1488,   226,   226,     0,   224,     0,     0,     0,     0,     0,
       0,   247,   249,     0,     0,     0,     0,     0,   247,     0,
       0,     0,     0,     0,   247,     0,   217,   218,   219,   220,
     221,     0,     0,   532,   533,   896,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   190,  1029,
     247,    91,     0,     0,    93,    94,     0,    95,   191,    97,
    1537,     0,     0,     0,     0,     0,     0,  1541,     0,     0,
     247,     0,     0,     0,   247,     0,     0,     0,     0,   474,
     475,   476,     0,   108,     0,   247,     0,     0,  1976,     0,
       0,     0,     0,     0,     0,     0,     0,   705,   974,   477,
     478,     0,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,     0,     0,     0,
       0,   224,   224,     0,     0,  1576,  1577,     0,   503,     0,
       0,     0,     0,     0,     0,   247,     0,     0,     0,   247,
       0,   247,     0,     0,   247,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   896,   896,   896,
     896,   896,   896,   224,   896,     0,     0,   896,   896,   896,
     896,   896,   896,   896,   896,   896,   896,   896,   896,   896,
     896,   896,   896,   896,   896,   896,   896,   896,   896,   896,
     896,   896,   896,   896,   896,     0,     0,   290,     0,     0,
       0,     0,     0,   459,    63,    64,    65,    66,    67,     0,
       0,     0,   226,   896,    72,   509,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
    1186,  1187,  1188,     0,   292,     0,     0,     0,  1668,  1669,
       0,     0,  1671,     0,     0,     0,  1189,   213,     0,     0,
     247,     0,   247,  1139,     0,   510,     0,   511,     0,     0,
       0,     0,  1008,     0,     0,   224,     0,     0,     0,     0,
      50,   512,     0,   513,     0,     0,   460,     0,     0,     0,
    1689,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1716,     0,   247,     0,     0,   247,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   591,   217,   218,   219,
     220,   221,   592,   247,   247,   247,   247,   247,   247,     0,
       0,   224,   247,     0,   247,     0,     0,     0,   224,   190,
       0,     0,    91,   345,     0,    93,    94,     0,    95,   191,
      97,     0, -1136,   224,   224,     0,   896,     0,     0,     0,
       0,   226,     0,   349,     0,     0,   247,     0,     0,     0,
     226,     0,     0,   247,   108,   351,  1766,   226,   896,     0,
     896,     0,     0,     0,     0,     0,     0,     0,     0,   226,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     226,  1689,     0,     0,     0,   896, -1137, -1137, -1137, -1137,
   -1137,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,   355,     0,     0,     0,     0,  1689,     0,  1689,   474,
     475,   476,   503,  1892,     0,  1689,     0,     0,     0,     0,
       0,   247,   247,     0,     0,   247,     0,     0,   224,   477,
     478,     0,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,   247,     0,     0,
       0,     0,     0,     0,   249,     0,     0,     0,   503,     0,
       0,     0,     0,  1928,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   474,   475,   476,     0,     0,   247,     0,
     247,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   477,   478,   226,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,     0,     0,     0,   247,   247,     0,     0,   247,     0,
       0,     0,   503,     0,   896,     0,   896,     0,   896,     0,
       0,     0,     0,   896,   224,     0,     0,     0,   896,     0,
     896,     0,   904,   896,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   247,   247,  1950,     0,
     247,     0,     0,     0,     0,     0,     0,   247,     0,     0,
       0,   873,     0,  1963,     0,   904,     0,  1689,     0,     0,
       0,     0,  1040,     0,     0,     0,     0,     0,     0,   906,
       0,     0,     0,     0,     0,   546,   518,   519,   520,   521,
     522,   523,   524,   525,   526,   527,   528,   529,     0,     0,
       0,     0,     0,     0,     0,   355,   247,   355,   247,     0,
     247,   213,   936,     0,     0,   247,     0,   224,     0,     0,
       0,     0,   874,     0,     0,     0,     0,     0,     0,     0,
     530,   531,   247,     0,    50,     0,     0,   896,     0,     0,
       0,  2022,     0,     0,     0,     0,  1066,     0,     0,   247,
     247,     0,     0,     0,   226,     0,     0,   247,     0,   247,
       0,     0,     0,     0,   290,     0,   355,  1963,     0,  2035,
       0,   217,   218,   219,   220,   221,     0,     0,     0,     0,
       0,     0,   247,     0,   247,     0,     0,     0,     0,   247,
       0,   247,     0,   190,     0,     0,    91,     0,     0,    93,
      94,   292,    95,   191,    97,     0,   875,   532,   533,     0,
       0,     0,     0,     0,   213,     0,     0,   247,     0,     0,
    1540,     0,     0,     0,     0,     0,     0,     0,   108,     0,
     226,     0,     0,     0,   896,   896,   896,    50,     0,     0,
       0,   896,     0,   247,     0,     0,     0,     0,     0,   247,
       0,   247,     0,     0,     0,     0,     0,     0,   355,     0,
       0,   355,     0,  1091,     0,     0,     0,     0,     0,   226,
       0,   226,     0,   591,   217,   218,   219,   220,   221,   592,
       0,     0,     0,  1028,   518,   519,   520,   521,   522,   523,
     524,   525,   526,   527,   528,   529,   190,   226,   904,    91,
     345,     0,    93,    94,     0,    95,   191,    97,     0, -1136,
       0,     0,     0,     0,   904,   904,   904,   904,   904,     0,
     349,     0,   290,     0,     0,     0,     0,   904,   530,   531,
       0,   108,   351,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1193,  1121,     0,   896,     0,     0,
       0,     0,     0,     0,   247,     0,     0,     0,     0,   292,
       0,  1145,  1146,  1147,  1148,  1149,     0,     0,     0,   247,
     226,     0,   213,   247,  1160,     0,     0,   247,   247,     0,
       0,     0,     0,     0,  1216,     0,   226,   226,     0,     0,
       0,     0,   247,     0,     0,    50,     0,     0,   896,     0,
       0,     0,     0,   598,     0,   532,   533,     0,     0,     0,
     355,  1216,   876,     0,   896,     0,     0,     0,     0,     0,
     896,   226,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   591,   217,   218,   219,   220,   221,   592,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   247,     0,     0,
       0,     0,   904,     0,   190,     0,     0,    91,   345,     0,
      93,    94,   896,    95,   191,    97,     0,  1277,     0,     0,
       0,     0,     0,   247,     0,   247,     0,     0,   349,     0,
       0,     0,     0,   228,   228,     0,     0,     0,     0,   108,
     351,   249,     0,     0,   253,     0,     0,     0,     0,  1264,
       0,     0,  1091,     0,   247,     0,     0,   355,   355,     0,
       0,     0,     0,     0,     0,     0,   355,     0,     0,     0,
     247,     0,     0,     0,     0,     0,   247,     0,     0,     0,
     247,     0,     0,   474,   475,   476,     0,     0,     0,     0,
       0,     0,     0,     0,   247,   247,     0,     0,   226,   226,
       0,     0,     0,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,     0,     0,     0,   904,   904,   904,   904,   904,   904,
     226,   904,   503,     0,   904,   904,   904,   904,   904,   904,
     904,   904,   904,   904,   904,   904,   904,   904,   904,   904,
     904,   904,   904,   904,   904,   904,   904,   904,   904,   904,
     904,   904,     0,     0,   873,     0,     0,     0,     0,     0,
       0,  1149,  1370,     0,     0,  1370,     0,     0,     0,     0,
     904,  1384,  1387,  1388,  1389,  1391,  1392,  1393,  1394,  1395,
    1396,  1397,  1398,  1399,  1400,  1401,  1402,  1403,  1404,  1405,
    1406,  1407,  1408,  1409,  1410,  1411,  1412,  1413,  1414,     0,
       0,     0,     0,     0,   213,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   874,     0,  1424,  1133,     0,
       0,     0,   226,     0,     0,   355,   355,    50,     0,     0,
       0,     0,     0,     0,   228,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   213,     0,     0,     0,
       0,     0,     0,     0,   213,     0,  1207,     0,     0,     0,
       0,     0,     0,     0,   217,   218,   219,   220,   221,    50,
    1091,  1091,  1091,  1091,  1091,  1091,     0,    50,   226,  1091,
       0,  1091,     0,     0,     0,   226,   190,     0,     0,    91,
       0,     0,    93,    94,     0,    95,   191,    97,     0,  1355,
     226,   226,     0,   904,     0,     0,   217,   218,   219,   220,
     221,     0,     0,     0,   217,   218,   219,   220,   221,     0,
       0,   108,     0,   355,     0,   904,     0,   904,     0,     0,
       0,     0,     0,     0,    93,    94,     0,    95,   191,    97,
       0,   355,    93,    94,     0,    95,   191,    97,   355,     0,
    1527,     0,   904,     0,   355,     0,     0,     0,     0,     0,
       0,     0,     0,   108,   742,     0,     0,     0,     0,     0,
       0,   108,  1545,   228,  1546,     0,     0,     0,     0,     0,
       0,     0,   228,     0,     0,     0,     0,     0,     0,   228,
       0,     0,  1579,     0,     0,   226,     0,     0,     0,  1567,
     355,   228,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,   253,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
       0,     0,     0,     0,     0,  1091,     0,  1091,     0,     0,
       0,   503, -1137, -1137, -1137, -1137, -1137,  1181,  1182,  1183,
    1184,  1185,  1186,  1187,  1188,   355,     0,     0,     0,   355,
       0,   876,     0,     0,   355,     0,     0,     0,  1189,     0,
       0,   213,     0,     0,     0,     0,   253,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   904,     0,   904,    50,   904,     0,     0,     0,     0,
     904,   226,     0,     0,     0,   904,     0,   904,     0,     0,
     904,     0,     0,     0,     0,     0,     0,   228,     0,     0,
    1708,     0,     0,     0,  1691,     0,     0,  1704,     0,     0,
     290,   217,   218,   219,   220,   221,     0,     0,  1673,     0,
    1674,     0,  1675,     0,     0,     0,     0,  1676,     0,     0,
       0,     0,  1678,     0,  1679,     0,     0,  1680,     0,    93,
      94,     0,    95,   191,    97,     0,     0,   292,     0,     0,
     355,     0,   355,     0,   905,  1284,     0,     0,     0,     0,
     213,     0,     0,  1091,     0,  1091,   213,  1091,   108,  1709,
       0,     0,  1091,     0,   226,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,   905,     0,    50,
       0,  -435,     0,   355,   904,     0,   355,     0,     0,     0,
     459,   187,   188,    65,    66,    67,  1773,  1774,     0,     0,
       0,     0,     0,     0,     0,     0,  1704,     0,     0,   591,
     217,   218,   219,   220,   221,   592,   217,   218,   219,   220,
     221,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1768,   190,     0,     0,    91,   345,     0,    93,    94,
       0,    95,   191,    97,    93,    94,   355,    95,   191,    97,
       0,     0,     0,   355,     0,     0,   349,     0,     0,     0,
       0,     0,     0,   460,  1091,     0,   228,   108,   351,     0,
       0,     0,     0,   108,  1037,  1163,  1164,  1165,     0,     0,
       0,   904,   904,   904,     0,     0,     0,     0,   904,     0,
    1926,     0,     0,     0,     0,     0,  1166,     0,  1704,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,
    1188,   355,   355,     0,     0,     0,   213,     0,  1919,  1920,
    1921,     0,     0,     0,  1189,  1925,     0,     0,     0,     0,
       0,     0,   228,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,   355,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1695,     0,     0,     0,     0,     0,     0,     0,
       0,   228,     0,   228,     0,  1696,   217,   218,   219,   220,
     221,  1697,     0,     0,   904,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   190,   228,
     905,    91,    92,     0,    93,    94,     0,    95,  1699,    97,
       0,     0,     0,     0,  1091,  1091,   905,   905,   905,   905,
     905,     0,     0,   290,   355,   355,     0,     0,   355,   905,
       0,  1944,     0,   108,     0,   904,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   904,     0,     0,     0,  1381,     0,   904,     0,     0,
     292,     0,     0,     0,     0,     0,   355,     0,     0,     0,
       0,     0,   228,   213,     0,  1713,     0,   355,     0,     0,
       0,     0,  1987,     0,     0,     0,     0,     0,   228,   228,
       0,     0,     0,     0,     0,     0,    50,     0,  2000,   904,
       0,     0,     0,     0,  2003,     0,     0,     0,     0,     0,
       0,     0,     0,   459,   187,   188,    65,    66,    67,     0,
       0,     0,     0,   253,     0,     0,     0,     0,     0,     0,
       0,     0,   591,   217,   218,   219,   220,   221,   592,     0,
       0,  2053,     0,     0,     0,     0,  2026,     0,     0,     0,
       0,     0,   355,     0,   905,   190,     0,  1691,    91,   345,
       0,    93,    94,     0,    95,   191,    97,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   355,     0,   349,
       0,     0,     0,     0,     0,     0,   460,     0,     0,     0,
     108,   351,     0,   253,   474,   475,   476,     0,     0,     0,
       0,     0,   355,     0,   355,     0,     0,     0,     0,   355,
       0,   355,     0,     0,   477,   478,     0,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,     0,     0,     0,     0,     0,     0,     0,     0,
     228,   228,     0,   503,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   355,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   905,   905,   905,   905,
     905,   905,   253,   905,     0,     0,   905,   905,   905,   905,
     905,   905,   905,   905,   905,   905,   905,   905,   905,   905,
     905,   905,   905,   905,   905,   905,   905,   905,   905,   905,
     905,   905,   905,   905,   474,   475,   476,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   905,     0,   477,   478,     0,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,     0,     0,   355,     0,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,     0,  1295,     0,   355,
     474,   475,   476,   355,   228,     0,     0,     0,     0,     0,
       0,   290,     0,     0,     0,     0,     0,     0,     0,     0,
     477,   478,  1978,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,   292,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   503,
     253,   213,     0,     0,     0,     0,     0,   228,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   355,     0,     0,
       0,     0,   228,   228,    50,   905,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,   355,   502,   355,     0,   905,     0,   905,
       0,     0,     0,     0,     0,     0,   503,     0,     0,     0,
     591,   217,   218,   219,   220,   221,   592,  1326,     0,     0,
       0,     0,     0,     0,   905,     0,     0,     0,     0,     0,
       0,     0,     0,   190,     0,     0,    91,   345,     0,    93,
      94,     0,    95,   191,    97,     0,   355,     0,     0,     0,
     355,     0,     0,     0,     0,     0,     0,   349,     0,     0,
       0,     0,     0,     0,   355,   355,     0,   228,   108,   351,
       0,     0,     0,   474,   475,   476,     0,     0,     0,     0,
       0,     0,     0,  1605,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   503,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
    1186,  1187,  1188,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,  1189,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,   905,     0,   905,     0,   905,     0,     0,
       0,     0,   905,   253,     0,     0,     0,   905,    14,   905,
      15,    16,   905,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,    56,    57,    58,  1729,    59,  -205,    60,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,    71,    72,    73,     0,     0,   253,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,   905,     0,    87,     0,
       0,     0,     0,    88,    89,    90,    91,    92,     0,    93,
      94,     0,    95,    96,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,   103,
       0,   104,     0,   105,   106,   107,     0,     0,   108,   109,
       0,   110,   111,     0,     0,   112,   113,   114,   115,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,   905,   905,   905,     0,     0,     0,     0,
     905,     0,     0,     0,     0,     0,     0,     0,    14,  1931,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,    56,    57,    58,     0,    59,     0,    60,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,   905,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,    88,    89,    90,    91,    92,     0,    93,
      94,     0,    95,    96,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,   103,
       0,   104,     0,   105,   106,   107,     0,   905,   108,   109,
       0,   110,   111,  1226,     0,   112,   113,   114,   115,     0,
       0,     0,     0,   905,     0,     0,     0,     0,     0,   905,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,  2024,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   905,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
      56,    57,    58,     0,    59,     0,    60,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
      88,    89,    90,    91,    92,     0,    93,    94,     0,    95,
      96,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,   103,     0,   104,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
    1439,     0,   112,   113,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   190,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   191,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,   707,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,     0,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   190,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   191,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,  1030,     0,   112,   113,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,  -205,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   190,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   191,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,     0,     0,
     112,   113,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,     0,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   190,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   191,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,  1192,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
    1241,     0,   112,   113,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   190,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   191,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,  1273,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,     0,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   190,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   191,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,  1332,     0,   112,   113,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,  1334,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   190,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   191,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,     0,     0,
     112,   113,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,     0,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,  1528,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   190,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   191,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,     0,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
    1682,     0,   112,   113,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
    -297,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   190,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   191,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,     0,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,     0,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   190,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   191,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,  1922,     0,   112,   113,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,  1974,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   190,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   191,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,     0,     0,
     112,   113,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,     0,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,  2016,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   190,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   191,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,     0,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
    2033,     0,   112,   113,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   190,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   191,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,  2036,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,     0,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   190,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   191,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,  2052,     0,   112,   113,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   190,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   191,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,  2107,     0,
     112,   113,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,     0,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   190,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   191,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,  2108,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,   574,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,     0,    61,    62,   187,
     188,    65,    66,    67,     0,    68,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
       0,     0,   112,   113,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,   860,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,   187,   188,    65,    66,    67,
       0,    68,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   190,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   191,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,     0,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,  1123,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,     0,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,   187,   188,    65,    66,    67,     0,    68,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   190,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   191,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,     0,     0,   112,   113,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,  1762,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,   187,   188,    65,
      66,    67,     0,    68,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   190,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   191,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,     0,     0,
     112,   113,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,  1914,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,     0,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
       0,    61,    62,   187,   188,    65,    66,    67,     0,    68,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   190,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   191,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,     0,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,     0,    61,    62,   187,
     188,    65,    66,    67,     0,    68,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
       0,     0,   112,   113,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     360,   426,    13,     0,     0,     0,     0,     0,     0,     0,
       0,   798,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,   108,   109,     5,     6,     7,     8,     9,   112,   113,
     114,   115,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   360,     0,    13,
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
       0,     0,    10,   105,   106,   107,     0,     0,   108,   192,
       0,   361,     0,     0,     0,   112,   113,   114,   115,     0,
       0,     0,     0,     0,     0,     0,     0,   722,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,   723,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,     0,
     186,   187,   188,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   189,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   190,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   191,    97,     0,   724,     0,    99,     0,     0,
     100,     5,     6,     7,     8,     9,   101,   102,     0,     0,
       0,    10,   105,   106,   107,     0,     0,   108,   192,     0,
       0,     0,     0,     0,   112,   113,   114,   115,     0,     0,
       0,     0,     0,     0,     0,     0,  1260,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,  1261,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   186,
     187,   188,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   189,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   190,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   191,    97,     0,  1262,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   192,     5,     6,
       7,     8,     9,   112,   113,   114,   115,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   360,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   192,     0,     0,   855,     0,     0,
     112,   113,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   360,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   798,
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
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     192,     5,     6,     7,     8,     9,   112,   113,   114,   115,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   360,   426,     0,     0,     0,
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
      10,   105,   106,   107,     0,     0,   108,   109,     0,     0,
       0,     0,     0,   112,   113,   114,   115,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,   204,     0,     0,    55,
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
       0,     0,     0,     0,   240,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     107,     0,     0,   108,   192,     0,   275,     0,     0,     0,
     112,   113,   114,   115,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,   108,   192,     0,   278,     0,     0,     0,   112,
     113,   114,   115,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   426,     0,
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
       0,     0,    10,   105,   106,   107,     0,     0,   108,   109,
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
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   192,   572,
       0,     0,     0,     0,   112,   113,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   360,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     112,   113,   114,   115,     0,     0,   753,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,   798,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,   108,   192,     0,     0,     0,     0,     0,   112,   113,
     114,   115,     0,     0,     0,     0,     0,     0,     0,     0,
     837,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
      99,     0,     0,   100,     5,     6,     7,     8,     9,   101,
     102,     0,     0,     0,    10,   105,   106,   107,     0,     0,
     108,   192,     0,     0,     0,     0,     0,   112,   113,   114,
     115,     0,     0,     0,     0,     0,     0,     0,     0,   839,
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
     192,     0,     0,     0,     0,     0,   112,   113,   114,   115,
       0,     0,     0,     0,     0,     0,     0,     0,  1323,     0,
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
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
       0,     0,     0,   105,   106,   107,     0,     0,   108,   192,
       5,     6,     7,     8,     9,   112,   113,   114,   115,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   360,     0,     0,     0,     0,     0,
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
     105,   106,   107,     0,     0,   108,  1454,     0,     0,     0,
       0,     0,   112,   113,   114,   115,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,   666,    39,    40,
       0,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,   186,   187,   188,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   189,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   190,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   191,    97,
       0,   280,   281,    99,   282,   283,   100,     0,   284,   285,
     286,   287,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   192,     0,   288,   289,     0,     0,
     112,   113,   114,   115,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,   291,   502,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   503,   293,
     294,   295,   296,   297,   298,   299,     0,     0,     0,   213,
       0,   214,    40,     0,     0,   300,     0,     0,     0,     0,
       0,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,    50,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   213,   336,     0,
     785,   338,   339,   340,     0,     0,     0,   341,   602,   217,
     218,   219,   220,   221,   603,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,   280,   281,     0,   282,   283,
       0,   604,   284,   285,   286,   287,     0,    93,    94,     0,
      95,   191,    97,   346,     0,   347,     0,     0,   348,     0,
     288,   289,     0,     0,     0,     0,   350,   217,   218,   219,
     220,   221,     0,     0,     0,     0,   108,     0,     0,     0,
     786,     0,     0,   112,     0,     0,     0,     0,     0,   291,
       0,     0,   371,     0,     0,    93,    94,     0,    95,   191,
      97,     0,     0,   293,   294,   295,   296,   297,   298,   299,
       0,     0,     0,   213,     0,   214,    40,     0,     0,   300,
       0,     0,     0,     0,   108,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,    50,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,     0,   336,     0,   337,   338,   339,   340,     0,     0,
       0,   341,   602,   217,   218,   219,   220,   221,   603,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   280,
     281,     0,   282,   283,     0,   604,   284,   285,   286,   287,
       0,    93,    94,     0,    95,   191,    97,   346,     0,   347,
       0,     0,   348,     0,   288,   289,     0,   290,     0,     0,
     350,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     108,     0,     0,     0,   786,     0,     0,   112,     0,     0,
       0,     0,     0,   291,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   292,     0,     0,   293,   294,   295,
     296,   297,   298,   299,     0,     0,     0,   213,     0,     0,
       0,     0,     0,   300,     0,     0,     0,     0,     0,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
      50,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,     0,   336,     0,     0,   338,
     339,   340,     0,     0,     0,   341,   342,   217,   218,   219,
     220,   221,   343,     0,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   344,
    1132,     0,    91,   345,     0,    93,    94,     0,    95,   191,
      97,   346,    50,   347,     0,     0,   348,     0,   280,   281,
       0,   282,   283,   349,   350,   284,   285,   286,   287,     0,
       0,     0,     0,     0,   108,   351,     0,     0,     0,  1893,
       0,     0,     0,   288,   289,     0,   290,     0,     0,   217,
     218,   219,   220,   221,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   190,   291,     0,    91,     0,     0,    93,    94,     0,
      95,   191,    97,   292,     0,     0,   293,   294,   295,   296,
     297,   298,   299,     0,     0,     0,   213,     0,     0,     0,
       0,     0,   300,     0,     0,     0,   108,     0,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,    50,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,     0,   336,     0,     0,   338,   339,
     340,     0,     0,     0,   341,   342,   217,   218,   219,   220,
     221,   343,     0,     0,     0,     0,     0,     0,   213,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   344,     0,
       0,    91,   345,     0,    93,    94,     0,    95,   191,    97,
     346,    50,   347,     0,     0,   348,     0,   280,   281,     0,
     282,   283,   349,   350,   284,   285,   286,   287,     0,     0,
       0,     0,     0,   108,   351,     0,     0,     0,  1969,     0,
       0,     0,   288,   289,     0,   290,     0,     0,   217,   218,
     219,   220,   221,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     190,   291,     0,    91,    92,     0,    93,    94,     0,    95,
     191,    97,   292,     0,     0,   293,   294,   295,   296,   297,
     298,   299,     0,     0,     0,   213,     0,     0,     0,     0,
       0,   300,     0,     0,     0,   108,     0,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,    50,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,     0,   336,     0,   337,   338,   339,   340,
       0,     0,     0,   341,   342,   217,   218,   219,   220,   221,
     343,     0,     0,     0,     0,     0,     0,   213,     0,   968,
       0,   969,     0,     0,     0,     0,     0,   344,     0,     0,
      91,   345,     0,    93,    94,     0,    95,   191,    97,   346,
      50,   347,     0,     0,   348,     0,   280,   281,     0,   282,
     283,   349,   350,   284,   285,   286,   287,     0,     0,     0,
       0,     0,   108,   351,     0,     0,     0,     0,     0,     0,
       0,   288,   289,     0,   290,     0,     0,   217,   218,   219,
     220,   221,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
     291,   502,     0,     0,     0,    93,    94,     0,    95,   191,
      97,   292,     0,   503,   293,   294,   295,   296,   297,   298,
     299,     0,     0,     0,   213,     0,     0,     0,     0,     0,
     300,     0,     0,     0,   108,     0,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,    50,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,     0,   336,     0,     0,   338,   339,   340,     0,
       0,     0,   341,   342,   217,   218,   219,   220,   221,   343,
       0,     0,     0,     0,     0,     0,   213,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   344,     0,     0,    91,
     345,     0,    93,    94,     0,    95,   191,    97,   346,    50,
     347,     0,     0,   348,     0,     0,     0,   924,   925,     0,
     349,   350,  1686,     0,     0,     0,   280,   281,     0,   282,
     283,   108,   351,   284,   285,   286,   287,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   217,   218,   219,   220,
     221,   288,   289,     0,   290,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   926,     0,     0,    93,    94,     0,    95,   191,    97,
     291,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   292,     0,     0,   293,   294,   295,   296,   297,   298,
     299,     0,     0,   108,   213,     0,     0,     0,     0,     0,
     300,     0,     0,     0,     0,     0,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,    50,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,     0,   336,     0,     0,   338,   339,   340,     0,
       0,     0,   341,   342,   217,   218,   219,   220,   221,   343,
       0,     0,     0,     0,     0,     0,   213,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   344,     0,     0,    91,
     345,     0,    93,    94,     0,    95,   191,    97,   346,    50,
     347,     0,     0,   348,     0,  1788,  1789,  1790,  1791,  1792,
     349,   350,  1793,  1794,  1795,  1796,     0,     0,     0,     0,
       0,   108,   351,     0,     0,     0,     0,     0,     0,  1797,
    1798,  1799,     0,     0,     0,     0,   217,   218,   219,   220,
     221,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,  1800,
       0,   926,     0,     0,    93,    94,     0,    95,   191,    97,
       0,     0,  1189,  1801,  1802,  1803,  1804,  1805,  1806,  1807,
       0,     0,     0,   213,     0,     0,     0,     0,     0,  1808,
       0,     0,     0,   108,     0,  1809,  1810,  1811,  1812,  1813,
    1814,  1815,  1816,  1817,  1818,  1819,    50,  1820,  1821,  1822,
    1823,  1824,  1825,  1826,  1827,  1828,  1829,  1830,  1831,  1832,
    1833,  1834,  1835,  1836,  1837,  1838,  1839,  1840,  1841,  1842,
    1843,  1844,  1845,  1846,  1847,  1848,  1849,  1850,     0,     0,
       0,  1851,  1852,   217,   218,   219,   220,   221,     0,  1853,
    1854,  1855,  1856,  1857,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1858,  1859,  1860,     0,     0,
       0,    93,    94,     0,    95,   191,    97,  1861,     0,  1862,
    1863,     0,  1864,     0,     0,     0,     0,     0,     0,  1865,
       0,  1866,     0,  1867,     0,  1868,  1869,     0,   280,   281,
     108,   282,   283,  1165,     0,   284,   285,   286,   287,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1166,   288,   289,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,
    1182,  1183,  1184,  1185,  1186,  1187,  1188,     0,     0,     0,
       0,     0,   291,     0,     0,     0,     0,     0,     0,     0,
    1189,     0,     0,     0,     0,     0,   293,   294,   295,   296,
     297,   298,   299,     0,     0,     0,   213,     0,     0,     0,
       0,     0,   300,     0,     0,     0,     0,     0,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,    50,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,     0,   336,     0,   337,   338,   339,
     340,     0,     0,     0,   341,   602,   217,   218,   219,   220,
     221,   603,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   280,   281,     0,   282,   283,     0,   604,   284,
     285,   286,   287,     0,    93,    94,     0,    95,   191,    97,
     346,     0,   347,     0,     0,   348,     0,   288,   289,     0,
       0,     0,     0,   350,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   108,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   291,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     293,   294,   295,   296,   297,   298,   299,     0,     0,     0,
     213,     0,     0,     0,     0,     0,   300,     0,     0,     0,
       0,     0,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,    50,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,     0,   336,
       0,  1382,   338,   339,   340,     0,     0,     0,   341,   602,
     217,   218,   219,   220,   221,   603,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   280,   281,     0,   282,
     283,     0,   604,   284,   285,   286,   287,     0,    93,    94,
       0,    95,   191,    97,   346,     0,   347,     0,     0,   348,
       0,   288,   289,     0,     0,     0,     0,   350,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   108,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     291,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   293,   294,   295,   296,   297,   298,
     299,     0,     0,     0,   213,     0,     0,     0,     0,     0,
     300,     0,     0,     0,     0,     0,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,    50,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,     0,   336,     0,     0,   338,   339,   340,     0,
       0,     0,   341,   602,   217,   218,   219,   220,   221,   603,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   604,     0,     0,     0,
       0,     0,    93,    94,     0,    95,   191,    97,   346,     0,
     347,     0,     0,   348,   474,   475,   476,     0,     0,     0,
       0,   350,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   108,     0,     0,   477,   478,     0,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,     0,     0,     0,     0,
       0,     0,   477,   478,  1532,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,     0,     0,     0,     0,     0,     0,     0,
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,   474,   475,
     476,     0,     0,     0,     0,     0,     0,     0,     0,   503,
       0,     0,     0,     0,     0,     0,     0,     0,   477,   478,
       0,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,  1730,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   503,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   474,   475,   476,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   477,   478,  1533,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,   474,
     475,   476,     0,     0,     0,     0,     0,     0,     0,     0,
     503,     0,     0,     0,     0,     0,     0,     0,     0,   477,
     478,   504,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,   474,   475,   476,
       0,     0,     0,     0,     0,     0,     0,     0,   503,     0,
       0,     0,     0,     0,     0,     0,     0,   477,   478,   588,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,     0,   474,   475,   476,     0,
       0,     0,     0,     0,     0,     0,   503,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   477,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,   590,   502,   474,   475,   476,     0,     0,     0,
       0,     0,     0,     0,     0,   503,   290,     0,     0,     0,
       0,     0,     0,     0,   477,   478,     0,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
     609,   502,     0,   292,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   503,   290,     0,   213,     0,     0,     0,
       0,     0,  1003,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,   613,     0,
       0,   292,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   213,     0,     0,     0,     0,     0,
    1462,     0,     0,     0,     0,   591,   217,   218,   219,   220,
     221,   592,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,  1390,     0,   829,     0,   190,     0,
       0,    91,   345,     0,    93,    94,     0,    95,   191,    97,
       0,     0,   882,   883,     0,     0,     0,     0,   884,     0,
     885,     0,   349,   591,   217,   218,   219,   220,   221,   592,
       0,     0,   886,   108,   351,     0,     0,     0,     0,     0,
      34,    35,    36,   213,   852,     0,   190,     0,     0,    91,
     345,     0,    93,    94,   215,    95,   191,    97,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
     349,     0,     0,     0,     0,     0,  1117,     0,     0,     0,
       0,   108,   351,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,
    1187,  1188,   887,   888,   889,   890,   891,   892,    29,    81,
      82,    83,    84,    85,     0,  1189,    34,    35,    36,   213,
     222,   214,    40,     0,     0,   190,    89,    90,    91,    92,
     215,    93,    94,     0,    95,   191,    97,     0,     0,     0,
      99,     0,    50,     0,     0,     0,     0,     0,     0,   893,
     894,     0,     0,     0,     0,   105,     0,     0,     0,   216,
     108,   895,     0,     0,   882,   883,     0,     0,     0,     0,
     884,     0,   885,     0,     0,     0,     0,  1118,    75,   217,
     218,   219,   220,   221,   886,    81,    82,    83,    84,    85,
       0,     0,    34,    35,    36,   213,   222,     0,     0,     0,
       0,   190,    89,    90,    91,    92,   215,    93,    94,     0,
      95,   191,    97,     0,     0,     0,    99,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   105,     0,     0,     0,     0,   108,   223,     0,     0,
    1068,  1069,     0,   112,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   887,   888,   889,   890,   891,   892,
    1070,    81,    82,    83,    84,    85,     0,     0,  1071,  1072,
    1073,   213,   222,     0,     0,     0,     0,   190,    89,    90,
      91,    92,  1074,    93,    94,     0,    95,   191,    97,     0,
       0,     0,    99,     0,    50,     0,     0,     0,     0,     0,
       0,   893,   894,     0,     0,     0,     0,   105,     0,     0,
       0,     0,   108,   895,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1075,  1076,  1077,  1078,  1079,  1080,    29,     0,     0,     0,
       0,     0,     0,     0,    34,    35,    36,   213,  1081,   214,
      40,     0,     0,   190,     0,     0,    91,    92,   215,    93,
      94,     0,    95,   191,    97,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,  1082,  1083,     0,
       0,     0,     0,     0,     0,     0,     0,   216,   108, -1137,
   -1137, -1137, -1137,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,  1186,  1187,  1188,    75,   217,   218,   219,
     220,   221,    29,    81,    82,    83,    84,    85,     0,  1189,
      34,    35,    36,   213,   222,   214,    40,     0,     0,   190,
      89,    90,    91,    92,   215,    93,    94,     0,    95,   191,
      97,     0,     0,     0,    99,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   105,
       0,     0,     0,   216,   108,   223,     0,     0,   629,     0,
       0,   112,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   649,    75,   217,   218,   219,   220,   221,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
     222,     0,     0,     0,     0,   190,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   191,    97,    29,  1059,     0,
      99,     0,     0,     0,     0,    34,    35,    36,   213,     0,
     214,    40,     0,     0,     0,   105,     0,     0,     0,   215,
     108,   223,     0,     0,     0,     0,     0,   112,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   216,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,   217,   218,
     219,   220,   221,    29,    81,    82,    83,    84,    85,     0,
       0,    34,    35,    36,   213,   222,   214,    40,     0,     0,
     190,    89,    90,    91,    92,   215,    93,    94,     0,    95,
     191,    97,     0,     0,     0,    99,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     105,     0,     0,     0,   216,   108,   223,     0,     0,     0,
       0,     0,   112,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1219,    75,   217,   218,   219,   220,   221,    29,
      81,    82,    83,    84,    85,     0,     0,    34,    35,    36,
     213,   222,   214,    40,     0,     0,   190,    89,    90,    91,
      92,   215,    93,    94,     0,    95,   191,    97,     0,     0,
       0,    99,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   105,     0,     0,     0,
     216,   108,   223,     0,     0,     0,     0,     0,   112,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
     217,   218,   219,   220,   221,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,   222,     0,     0,
       0,     0,   190,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   191,    97,     0,     0,     0,    99,     0,     0,
       0,     0,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,   105,     0,     0,     0,     0,   108,   223,     0,
       0,     0,   477,   478,   112,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,     0,     0,     0,     0,     0,     0,     0,
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   503,
       0,   474,   475,   476,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     549,   477,   478,     0,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,   474,
     475,   476,     0,     0,     0,     0,     0,     0,     0,     0,
     503,     0,     0,     0,     0,     0,     0,     0,   558,   477,
     478,     0,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   503,     0,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   958,
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,   474,   475,
     476,     0,     0,     0,     0,     0,     0,     0,     0,   503,
       0,     0,     0,     0,     0,     0,     0,  1045,   477,   478,
       0,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   503,     0,   474,
     475,   476,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1101,   477,
     478,     0,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,  1163,  1164,  1165,
       0,     0,     0,     0,     0,     0,     0,     0,   503,     0,
       0,     0,     0,     0,     0,     0,  1437,     0,  1166,     0,
       0,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
    1186,  1187,  1188,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1189,  1163,  1164,  1165,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1480,  1166,     0,
       0,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
    1186,  1187,  1188,  1163,  1164,  1165,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1189,     0,     0,     0,
       0,     0,     0,     0,  1166,  1363,     0,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1189,  1163,  1164,  1165,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1166,  1551,     0,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,  1163,
    1164,  1165,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1189,     0,     0,     0,     0,     0,     0,     0,
    1166,  1563,     0,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,  1187,  1188,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1189,  1163,
    1164,  1165,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1166,  1672,     0,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,  1187,  1188,    34,    35,    36,   213,     0,
     214,    40,     0,     0,     0,     0,     0,     0,  1189,   215,
       0,     0,     0,     0,     0,     0,     0,  1769,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   244,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   245,
       0,     0,     0,     0,     0,     0,     0,     0,   217,   218,
     219,   220,   221,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   222,     0,  1771,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,     0,     0,     0,    99,     0,    34,    35,    36,
     213,     0,   214,    40,     0,     0,     0,     0,     0,     0,
     105,   680,     0,     0,     0,   108,   246,     0,     0,     0,
       0,     0,   112,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     216, -1137, -1137, -1137, -1137,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
     217,   218,   219,   220,   221,     0,    81,    82,    83,    84,
      85,   503,     0,    34,    35,    36,   213,   222,   214,    40,
       0,     0,   190,    89,    90,    91,    92,   215,    93,    94,
       0,    95,   191,    97,     0,     0,     0,    99,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   105,     0,     0,     0,   244,   108,   681,     0,
       0,     0,     0,     0,   682,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   217,   218,   219,   220,
     221,     0,    81,    82,    83,    84,    85,   213,     0,     0,
       0,     0,     0,   222,     0,     0,     0,     0,   190,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   191,    97,
      50,     0,     0,    99,     0,     0,     0,     0,   368,   369,
       0,     0,     0,     0,     0,     0,     0,     0,   105,     0,
       0,     0,     0,   108,   246,     0,     0,     0,     0,     0,
     112,     0,     0,     0,     0,     0,     0,   217,   218,   219,
     220,   221,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   370,
       0,     0,   371,     0,     0,    93,    94,     0,    95,   191,
      97,     0,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   372,     0,     0,     0,   864,
       0,     0,   477,   478,   108,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   474,   475,   476,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   865,   477,   478,  1042,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,   474,   475,   476,     0,     0,     0,
       0,     0,     0,     0,     0,   503,     0,     0,     0,     0,
       0,     0,     0,     0,   477,   478,     0,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,  1163,  1164,  1165,     0,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1166,  1568,     0,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,  1163,  1164,
    1165,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1189,     0,     0,     0,     0,     0,     0,     0,  1166,
       0,     0,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,  1187,  1188,   475,   476,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1189,     0,     0,
       0,     0,     0,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,  1164,  1165,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   503,     0,     0,     0,     0,     0,     0,     0,
       0,  1166,     0,     0,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,  1186,  1187,  1188,   476,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1189,
       0,     0,     0,     0,   477,   478,     0,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   477,   478,   503,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   478,
     503,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1166,     0,   503,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1189,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     503,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
    1186,  1187,  1188,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1189,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1189,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   503,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   503
};

static const yytype_int16 yycheck[] =
{
       5,     6,   737,     8,     9,    10,    11,    12,    13,   194,
      15,    16,    17,    18,   418,    56,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   132,    31,   109,   109,   167,
       4,    98,   124,    33,   109,   418,   103,   104,   855,    44,
     124,   508,    44,   699,   574,   418,    46,    52,   448,    54,
     182,    51,    57,   109,    59,     4,   172,   173,   124,     4,
       4,   562,   695,   999,   131,   502,    30,  1429,    30,   696,
     726,  1001,   795,    30,   167,   537,   538,    57,   505,   506,
     610,    86,   863,   989,    86,   258,     4,   534,   195,   534,
     368,   369,   370,   247,   372,  1230,   870,   543,   623,   624,
     880,    57,  1036,   673,   109,    32,   568,   841,  1116,     9,
     192,   192,   539,  1021,     9,  1023,     9,   192,     9,    14,
    1054,     9,     9,   570,     9,   570,  1240,    14,     9,    14,
     552,     9,   450,   451,   452,     9,    32,     9,    48,  1907,
       9,   223,   223,   259,     9,     9,     9,     9,   223,     9,
      54,     9,     9,     9,     9,     9,     9,    54,    36,     9,
      81,    70,     9,    70,   246,   246,     4,    83,  1102,    70,
      83,    48,     9,     9,     9,    83,  1112,     9,    83,    84,
    1716,    48,    48,   103,   107,   108,    38,   192,  1724,    83,
      84,    50,    51,    91,   199,   112,   162,    38,    38,   162,
     166,    14,   199,   162,   121,   122,   123,   124,   125,   126,
     162,   132,     0,   183,   162,   199,    91,   183,   223,    32,
     183,   162,    48,  1003,   183,   136,   137,   136,   137,   199,
     403,    83,   199,   655,   656,   136,   137,    38,    51,   202,
      38,   246,    83,    83,    38,    57,   166,    70,    70,   162,
     202,   199,  1158,  1968,    70,  1970,   261,    69,   199,   264,
     176,   159,   684,   167,   999,    70,   271,   272,   176,   271,
     167,    70,   928,   107,   108,   202,   461,   194,   182,    70,
     203,    70,    83,    70,   159,    83,   418,  1581,   183,    83,
     200,   265,   199,   202,   132,   269,  1031,   719,   202,    70,
     205,   201,   202,  2071,    70,   201,   201,   200,   390,   390,
     201,   205,    70,   201,   201,   390,   201,  2085,   359,   196,
     201,  1683,   200,  1057,   176,   184,   200,    19,    20,   201,
     860,  1339,   201,   200,   200,   200,  1244,   201,   201,   201,
     162,   201,  1477,   201,   201,   201,   201,   201,   201,  1484,
     200,  1486,  1126,   200,  1128,   360,  1892,   195,  1292,    83,
     545,  1475,   784,   200,   200,   200,   448,   448,   200,   203,
     196,    19,    20,   448,   200,   176,   508,  1112,   176,   202,
     202,  1516,   176,   199,   389,   390,   202,   389,   455,    19,
      20,   396,   397,   398,   399,   400,   401,   202,  1692,   924,
     925,   406,   534,   202,    70,   446,  1033,   977,   199,   516,
    1316,   202,   199,   202,   419,   202,     4,    70,    70,    70,
      70,   426,   199,  1717,   556,  1719,    70,    83,   199,   434,
      70,   202,   124,   199,    70,   567,   202,    70,   570,   183,
     445,   199,   200,   448,   511,   512,   513,   514,    83,    83,
     872,    70,   176,   123,     8,   199,   878,    91,   463,   464,
      70,   517,   132,   136,   137,    50,    51,  1247,   448,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,    83,   503,  1634,
     505,   506,   507,   136,   137,  1228,   199,   425,   954,  1623,
     162,  1625,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   992,   160,   161,   950,   702,
     203,   704,   537,   538,   539,   540,   202,   517,   502,   195,
     502,   546,   199,   162,   549,   502,   202,   199,   240,   202,
     202,   202,   202,   558,  1335,   560,     4,    14,   202,   199,
    1007,   517,  1007,   568,   116,    31,   199,   136,   137,   202,
     205,   576,   103,   578,   208,   713,    70,   202,  1358,   167,
      87,   168,   103,   202,    50,   989,   199,    53,    83,  1051,
     176,   103,   240,  1123,    83,   433,   576,   199,  1311,   681,
    1313,  1963,    91,    83,  1317,    53,   989,   581,    56,   199,
     240,    91,   797,  1246,    83,   202,   989,   124,   125,   126,
     713,  1021,  1059,  1023,   629,    73,    70,  1741,   180,   199,
     199,  1745,   199,    83,   788,   166,   914,   399,   916,   401,
     918,    91,   136,   137,   922,   166,   103,   104,    96,  1276,
      98,   122,  1279,  1559,   166,   103,   104,   419,  1105,   183,
    1105,   132,   835,   836,   159,   160,   161,   821,   162,   842,
     843,   160,   161,   199,  1096,   199,   681,   515,   745,  1204,
     160,   161,  1462,   131,     4,   183,    87,   379,    70,    83,
      70,   160,   161,   167,  1161,    70,   388,    91,   390,    19,
      20,   199,    70,   395,  1022,   199,    83,  1025,  1026,   159,
     160,   161,   199,    83,    91,   407,   199,   168,  1140,   724,
     202,    91,  1144,   124,   125,   126,   842,   207,   166,  1151,
     181,   379,   162,    32,   183,    78,    79,    80,    81,  1155,
     388,  1157,   896,  1159,    83,    84,  1652,   395,   753,   379,
     199,   202,    31,   183,  1158,   909,    75,    76,   388,   407,
     208,   104,  1485,   201,   202,   395,   160,   161,   166,   199,
     418,    50,   202,   199,    53,  1158,   199,   407,   696,    75,
      76,   786,   159,   160,   161,  1158,   113,   114,   115,   109,
     160,   161,  1906,  1294,   103,   104,  1910,   118,   141,   142,
     143,   144,   145,   134,   135,   121,   122,   123,   124,   125,
     126,   816,    53,    54,    55,    38,    57,   265,   201,   202,
    1453,   269,   201,  1285,    70,   273,   169,   170,    69,   172,
     173,   174,  1259,    70,  1296,    50,    51,    52,    53,    54,
      55,   201,   847,   201,  1244,   433,    81,   106,   107,   108,
     201,   543,  1479,   201,    69,   198,   201,   989,   863,   859,
     992,   863,    70,   121,   122,   123,   124,   125,   126,   104,
    1938,  1939,   192,  1596,  1006,  1007,  1611,   201,   194,   201,
      53,    54,    55,    70,  1306,   201,    70,  1417,    70,   368,
     369,   370,  1338,   372,   868,   543,    69,    30,    70,   737,
     106,   107,   108,   223,   201,   202,   141,   142,   143,   144,
     145,   359,  1316,   543,  1934,  1935,  1006,  1007,    70,   202,
     240,   199,   162,   199,    70,   199,   246,   166,   162,   201,
      49,   166,    69,  1316,   169,   170,   194,   172,   173,   174,
     183,   162,   199,  1316,  2058,   265,   404,  1369,   199,   269,
     408,  1373,   204,   958,     9,   960,  1378,   962,   162,   162,
       8,  2075,   162,   198,  1386,   201,   199,   202,   973,   199,
    1693,  1103,    14,  1105,   162,   201,   201,   435,   202,   437,
     438,   439,   440,   988,     9,   433,    14,  1433,  1518,   201,
     132,  1058,   132,   200,   442,    14,   103,   183,   446,   200,
     202,   200,   976,   200,  1534,   200,   199,   455,  1635,   112,
     206,     9,   831,  1018,   199,   853,  1021,   200,  1023,  1481,
     199,   159,   200,  1028,   200,   200,  1158,   976,    95,     9,
     722,   976,   976,   201,   183,    14,   199,  1042,     9,   199,
    1045,  1021,  1047,  1023,   202,   201,  1051,   202,  1028,   202,
     201,   201,    83,   201,  1454,   202,   200,   200,   976,   379,
     508,   509,   510,   511,   512,   513,   514,   201,   388,   200,
     390,  2007,   134,   911,   722,   395,  2012,   199,  1500,   200,
     204,     9,  1504,  1001,     9,    70,   534,   407,    60,  1511,
     204,   204,   722,   204,   204,  1059,  1101,  1059,  2028,    32,
    1254,  2037,  1059,  1257,   182,   135,   798,     9,   556,  1109,
     162,   138,   200,   433,  1581,  1033,    88,   162,   200,    91,
    2050,    14,   570,   196,     9,   713,  1656,     9,   448,  2059,
     184,   200,     9,   581,    14,  1665,  1110,     9,   976,  1457,
    1458,  1459,   200,  1960,   200,   837,   134,   839,  1965,   737,
     798,  1681,  1244,   601,   200,  1559,   200,  1619,  1620,     9,
    1244,   999,  2098,   204,   204,  1221,   204,   203,   798,    14,
     204,   200,   162,   865,   200,   199,  1559,   200,  1244,   627,
     628,   103,     9,   201,  1316,   201,  1559,   138,   162,   837,
       9,   839,   200,  1031,   199,    70,  2013,    70,   199,    50,
      51,    52,    53,    54,    55,    70,    57,   837,    70,   839,
      70,   202,   199,     9,   662,   663,  1221,   865,    69,   203,
      14,   184,  1689,   543,   201,  1692,     9,  1232,    14,   202,
     176,   196,  1762,   202,   204,   865,    14,   201,   200,  1244,
      32,  1221,    70,   199,   199,    32,   938,    14,  1652,   199,
    1717,  1070,  1719,  1227,  1259,   199,    14,  1262,   831,  1726,
     199,   581,   954,   955,  1244,   853,   162,    52,    70,  1652,
      70,  2088,  2007,    70,  1112,    70,  1114,  2012,  1227,  1652,
    1285,  2061,  1227,  1227,   199,  2065,    70,   199,     9,   737,
     938,  1296,  1297,   200,   138,   201,   201,   745,   199,  2079,
    2080,    14,  2037,  1959,   138,  1961,   954,   955,   938,  1227,
     184,     9,   200,   162,    69,   176,     9,   121,   122,   123,
     124,   125,   126,   911,   954,   955,   204,  1327,   132,   133,
    1335,    83,   203,  1335,   176,   203,   203,   203,   201,     9,
    1345,   989,  1458,   199,   199,   138,    83,   201,    14,    83,
     202,   204,   200,   199,     9,   202,  1330,   200,  1276,   199,
     201,  1279,   138,  2098,   202,    92,   202,    32,   159,   502,
      77,   175,  1454,  1454,   200,   508,   201,   201,   184,  1454,
      32,   138,   200,   831,  1914,   833,   200,   204,   976,  1227,
     194,    81,  2048,    83,    84,     9,   368,   369,   370,   371,
     372,   534,   722,   204,     9,   853,   204,   138,     9,     9,
     200,   999,    14,     9,   104,   204,   204,   737,   200,   867,
     868,   200,   203,   556,  1116,  1117,   201,  1559,   201,   201,
     201,   199,  1437,   203,   567,    83,   202,   570,   410,  1444,
     201,  1908,   204,  1031,  1449,   199,  1451,   200,   199,  1454,
     200,   141,   142,   143,   144,   145,   200,   202,   201,  1597,
     200,   200,  1467,   911,   138,     9,    31,   138,  1116,  1117,
     200,   204,   920,   921,  1454,  1480,  1481,   167,   798,   169,
     170,   204,   172,   173,   174,  1592,  1116,  1117,   204,  1308,
    1309,  1310,  1311,  1312,  1313,   204,   204,  1070,  1317,     9,
    1319,   200,    32,   951,  1342,   201,  1928,  1929,   198,   200,
    1158,   200,   202,  1431,   201,   205,    81,   837,   201,   839,
    1652,   138,   176,  1441,  1112,   202,  1114,   113,   976,   171,
    1715,    83,   201,   853,   167,    83,    14,    83,   119,   104,
     200,   200,   200,   138,   992,   865,   202,   138,   868,    14,
     201,   999,  1244,   183,    14,   202,  2086,    14,  1006,  1007,
      14,  1479,   127,    83,   200,   200,   200,   199,  1260,   198,
     138,   138,    83,   201,   201,   140,   141,   142,   143,   144,
     145,   146,    14,  1031,    14,   201,     9,   202,     9,    68,
     203,   911,    14,   183,    83,   199,     9,     9,   163,   202,
     116,   166,   167,     6,   169,   170,  1611,   172,   173,   174,
    1058,  1616,  1260,   201,  1619,  1620,   103,  1724,   938,  1686,
    1068,  1069,  1070,   162,   184,   174,   103,    36,   199,   201,
    1260,  1323,    83,   198,   954,   955,   200,   199,   180,  1227,
     177,   184,   200,   184,     9,    48,  1338,  1339,    83,    83,
     201,   623,   624,   200,    14,  1103,   976,  1105,    83,    83,
     200,    14,  1110,    83,  1112,    14,  1114,   202,  1316,    83,
      14,  1204,    83,  2040,  1493,  1323,  1495,   514,   509,   999,
    1052,   511,  1336,  2055,   979,  1761,  1531,  1135,  1662,  2050,
    1338,  1339,     6,  1323,   631,  1261,  1748,  1786,  1694,  1873,
    2096,  1021,  2072,  1023,  1587,  1890,  1624,  1583,  1338,  1339,
     113,  1031,  1630,  1161,  1632,  1744,   119,  1635,   121,   122,
     123,   124,   125,   126,   127,   400,  1231,  1156,  1312,  1069,
    1735,  1152,  1570,  1307,    48,  1308,  1309,  1655,  1951,  1312,
    1308,  1433,  1190,  1098,   446,   881,  1319,   396,  2008,  1998,
    1575,  1212,  1136,  1591,  1342,    56,  1190,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   169,   170,    -1,   172,
      -1,    -1,    -1,  1611,    -1,    -1,    -1,    -1,    -1,  1227,
      -1,    -1,    -1,    -1,    -1,  1433,  1760,  1761,    -1,    -1,
    1110,   194,  1112,    -1,  1114,    -1,  1116,  1117,    -1,   113,
     203,    -1,    -1,  1433,    -1,   119,    -1,   121,   122,   123,
     124,   125,   126,   127,    -1,    -1,    -1,    -1,    -1,  1657,
      -1,    -1,  1641,    -1,  1643,    -1,  1645,    -1,  1666,    -1,
      -1,  1650,  1750,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     6,  1431,  1885,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1441,    -1,   169,   170,    -1,   172,   992,
    1308,  1309,  1310,  1311,  1312,  1313,    -1,    -1,    -1,  1317,
      -1,  1319,    -1,  1006,  1007,    -1,    -1,     6,    -1,    -1,
     194,    -1,  1330,  2021,    48,  1723,    -1,    -1,    -1,   203,
      -1,    -1,  1897,    -1,  1342,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1352,    -1,    -1,  1227,    -1,    -1,
    1951,  1559,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
    1493,  1759,  1495,    -1,  1244,    -1,  1059,  1765,    -1,    -1,
      -1,    -1,    -1,  1752,  1772,    -1,    -1,  2044,    -1,     6,
    1260,    -1,   914,    -1,   916,    -1,   918,    -1,    -1,   113,
     922,    -1,   924,   925,   926,   119,    -1,   121,   122,   123,
     124,   125,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,
    1103,    -1,  1105,    -1,    -1,    -1,    -1,  1425,    -1,    -1,
      -1,    48,  1570,    -1,   113,    -1,    -1,    -1,    -1,   290,
     119,   292,   121,   122,   123,   124,   125,   126,   127,    -1,
      -1,    -1,    -1,  1323,  1652,   169,   170,    -1,   172,  1597,
    1330,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1338,  1339,
      -1,    -1,  1342,  1611,    19,    20,    -1,    -1,    -1,    -1,
     194,    -1,    -1,    -1,    -1,    30,  1624,     6,    -1,   203,
     169,   170,  1630,   172,  1632,  1493,   113,  1495,    -1,    -1,
     351,    -1,   119,    -1,   121,   122,   123,   124,   125,   126,
     127,    56,    -1,    -1,    -1,   194,    -1,  1655,  1641,  1657,
    1643,    -1,  1645,    -1,   203,    -1,    -1,  1650,  1666,    48,
    1918,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2094,
      -1,    -1,    -1,  1912,  1913,    -1,    -1,    -1,  2103,    -1,
      -1,    -1,   169,   170,    -1,   172,  2111,    -1,    -1,  2114,
    2028,    -1,    -1,  1433,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1570,    -1,    -1,    -1,    -1,   194,    -1,    -1,
      -1,    -1,  2050,  1581,  1454,  1973,   203,    -1,    -1,  1587,
      -1,  2059,   443,    -1,   113,   446,    -1,    -1,    -1,    -1,
     119,    -1,   121,   122,   123,   124,   125,   126,   127,    -1,
      81,    -1,  1750,  1611,    -1,    -1,    -1,    -1,    -1,  2007,
      -1,  1759,    -1,    -1,  2012,    -1,    -1,  1765,    -1,  1752,
      -1,    -1,    -1,   104,  1772,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1641,    -1,  1643,    -1,  1645,    -1,  2037,
     169,   170,  1650,   172,    -1,    -1,    -1,    -1,    -1,  1657,
      -1,    -1,    -1,    -1,  1662,    -1,    -1,    -1,  1666,    -1,
     141,   142,   143,   144,   145,   194,    -1,    -1,    -1,    78,
      79,    80,  1204,    -1,   203,    -1,    -1,    -1,  1686,    -1,
      -1,  1689,    -1,    92,  1692,   240,   167,     6,   169,   170,
    1570,   172,   173,   174,  1702,    -1,    -1,    -1,    -1,  2097,
    2098,  1709,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1717,
      -1,  1719,    -1,    -1,    -1,    -1,    -1,   198,  1726,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,  1611,    -1,    -1,   595,   290,    -1,   292,   147,   148,
     149,   150,   151,    -1,  1752,    -1,    -1,    -1,    -1,   158,
      -1,  1759,  1760,  1761,    -1,   164,   165,  1765,    -1,    -1,
      -1,    -1,    -1,    -1,  1772,    -1,    -1,    -1,    -1,   178,
    1918,    -1,    -1,    -1,    -1,    -1,    -1,  1657,    -1,  1912,
    1913,    -1,  1662,    -1,   193,    -1,  1666,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   113,    -1,   351,    -1,    -1,    -1,
     119,    -1,   121,   122,   123,   124,   125,   126,   127,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,   379,  1973,    -1,    -1,    -1,    -1,
      -1,   692,   693,   388,    -1,    -1,    -1,    -1,    -1,    -1,
     395,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     169,   170,   407,   172,    -1,    59,    60,    -1,    -1,  2007,
      -1,    -1,    -1,   418,  2012,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  2021,    -1,   194,    -1,  1885,    -1,  1759,
    1760,  1761,    -1,    -1,   203,  1765,    -1,    -1,   443,  2037,
      -1,   446,  1772,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1908,    -1,    -1,    -1,  1912,  1913,    -1,    -1,    -1,    -1,
    1918,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1927,
      -1,    -1,    -1,    -1,    -1,    -1,  1934,  1935,    -1,    -1,
    1938,  1939,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1951,    -1,    -1,    -1,   502,    -1,  2097,
    2098,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,  1973,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    -1,    -1,    -1,   543,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   200,    59,    60,  2007,
      -1,    -1,    -1,    -1,  2012,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  2020,    -1,    59,    60,    -1,    -1,    -1,   880,
     881,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,  2037,
      -1,    -1,    -1,    -1,    -1,  2043,    -1,    -1,  1918,    -1,
     595,    -1,   597,    -1,    -1,   600,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,   136,   137,    -1,    -1,   633,    -1,
      81,    -1,    83,    84,    -1,    69,    -1,    -1,    -1,  2097,
    2098,   136,   137,  1973,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,   104,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,   978,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,  2007,    -1,    -1,
      -1,    -1,  2012,    -1,    -1,   996,    -1,   692,   693,    -1,
     141,   142,   143,   144,   145,    -1,   701,    -1,  1009,    59,
      60,    -1,    -1,    -1,    -1,   200,    -1,  2037,   141,   142,
     143,   144,   145,    -1,    -1,    -1,   167,   722,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   167,    -1,   169,   170,   171,   172,
     173,   174,    -1,    -1,  1055,    -1,    -1,   198,    -1,    -1,
      -1,   202,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   198,   199,  2097,  2098,    -1,
     204,    10,    11,    12,    -1,    -1,   136,   137,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,   798,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,  1130,
      -1,    -1,    -1,  1134,    -1,    -1,   831,    -1,  1139,    -1,
      69,    -1,   837,    -1,   839,    -1,    -1,    -1,    -1,    -1,
     200,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    -1,    -1,    -1,
     865,   866,    -1,    -1,    -1,    -1,    -1,    -1,   873,    -1,
      -1,    -1,    -1,    -1,    -1,   880,   881,   882,   883,   884,
     885,   886,    -1,    -1,    -1,    10,    11,    12,    59,    60,
     895,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    30,    31,   912,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,   938,  1245,    -1,    59,    60,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    81,   952,    -1,   954,
     955,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   201,    -1,   203,   136,   137,    -1,    -1,   104,
      -1,    -1,    -1,   978,   979,    -1,    -1,  1288,    -1,    -1,
    1291,    19,    20,    -1,   989,    -1,    -1,    -1,    -1,    -1,
      -1,   996,    30,    -1,    -1,    -1,    -1,    -1,  1003,    -1,
      -1,    -1,    -1,    -1,  1009,    -1,   141,   142,   143,   144,
     145,    -1,    -1,   136,   137,  1020,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,   200,
    1035,   166,    -1,    -1,   169,   170,    -1,   172,   173,   174,
    1351,    -1,    -1,    -1,    -1,    -1,    -1,  1358,    -1,    -1,
    1055,    -1,    -1,    -1,  1059,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,   198,    -1,  1070,    -1,    -1,   203,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   200,   203,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,  1116,  1117,    -1,    -1,  1426,  1427,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,  1130,    -1,    -1,    -1,  1134,
      -1,  1136,    -1,    -1,  1139,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1152,  1153,  1154,
    1155,  1156,  1157,  1158,  1159,    -1,    -1,  1162,  1163,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,  1187,  1188,  1189,    -1,    -1,    31,    -1,    -1,
      -1,    -1,    -1,   121,   122,   123,   124,   125,   126,    -1,
      -1,    -1,   240,  1208,   132,   133,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    68,    -1,    -1,    -1,  1539,  1540,
      -1,    -1,  1543,    -1,    -1,    -1,    69,    81,    -1,    -1,
    1245,    -1,  1247,    87,    -1,   173,    -1,   175,    -1,    -1,
      -1,    -1,   203,    -1,    -1,  1260,    -1,    -1,    -1,    -1,
     104,   189,    -1,   191,    -1,    -1,   194,    -1,    -1,    -1,
    1581,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1592,    -1,  1288,    -1,    -1,  1291,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,  1308,  1309,  1310,  1311,  1312,  1313,    -1,
      -1,  1316,  1317,    -1,  1319,    -1,    -1,    -1,  1323,   163,
      -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,   176,  1338,  1339,    -1,  1341,    -1,    -1,    -1,
      -1,   379,    -1,   187,    -1,    -1,  1351,    -1,    -1,    -1,
     388,    -1,    -1,  1358,   198,   199,  1667,   395,  1363,    -1,
    1365,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   407,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     418,  1692,    -1,    -1,    -1,  1390,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    56,    -1,    -1,    -1,    -1,  1717,    -1,  1719,    10,
      11,    12,    69,  1724,    -1,  1726,    -1,    -1,    -1,    -1,
      -1,  1426,  1427,    -1,    -1,  1430,    -1,    -1,  1433,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,  1462,    -1,    -1,
      -1,    -1,    -1,    -1,   502,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,  1784,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,  1493,    -1,
    1495,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,   543,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,  1539,  1540,    -1,    -1,  1543,    -1,
      -1,    -1,    69,    -1,  1549,    -1,  1551,    -1,  1553,    -1,
      -1,    -1,    -1,  1558,  1559,    -1,    -1,    -1,  1563,    -1,
    1565,    -1,   600,  1568,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1581,  1582,  1889,    -1,
    1585,    -1,    -1,    -1,    -1,    -1,    -1,  1592,    -1,    -1,
      -1,    31,    -1,  1904,    -1,   633,    -1,  1908,    -1,    -1,
      -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,   600,
      -1,    -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   290,  1641,   292,  1643,    -1,
    1645,    81,   633,    -1,    -1,  1650,    -1,  1652,    -1,    -1,
      -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      59,    60,  1667,    -1,   104,    -1,    -1,  1672,    -1,    -1,
      -1,  1982,    -1,    -1,    -1,    -1,   203,    -1,    -1,  1684,
    1685,    -1,    -1,    -1,   722,    -1,    -1,  1692,    -1,  1694,
      -1,    -1,    -1,    -1,    31,    -1,   351,  2008,    -1,  2010,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,
      -1,    -1,  1717,    -1,  1719,    -1,    -1,    -1,    -1,  1724,
      -1,  1726,    -1,   163,    -1,    -1,   166,    -1,    -1,   169,
     170,    68,   172,   173,   174,    -1,   176,   136,   137,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    -1,  1752,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,
     798,    -1,    -1,    -1,  1769,  1770,  1771,   104,    -1,    -1,
      -1,  1776,    -1,  1778,    -1,    -1,    -1,    -1,    -1,  1784,
      -1,  1786,    -1,    -1,    -1,    -1,    -1,    -1,   443,    -1,
      -1,   446,    -1,   831,    -1,    -1,    -1,    -1,    -1,   837,
      -1,   839,    -1,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,   163,   865,   866,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,    -1,   176,
      -1,    -1,    -1,    -1,   882,   883,   884,   885,   886,    -1,
     187,    -1,    31,    -1,    -1,    -1,    -1,   895,    59,    60,
      -1,   198,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   912,   866,    -1,  1882,    -1,    -1,
      -1,    -1,    -1,    -1,  1889,    -1,    -1,    -1,    -1,    68,
      -1,   882,   883,   884,   885,   886,    -1,    -1,    -1,  1904,
     938,    -1,    81,  1908,   895,    -1,    -1,  1912,  1913,    -1,
      -1,    -1,    -1,    -1,   952,    -1,   954,   955,    -1,    -1,
      -1,    -1,  1927,    -1,    -1,   104,    -1,    -1,  1933,    -1,
      -1,    -1,    -1,   112,    -1,   136,   137,    -1,    -1,    -1,
     595,   979,   597,    -1,  1949,    -1,    -1,    -1,    -1,    -1,
    1955,   989,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1982,    -1,    -1,
      -1,    -1,  1020,    -1,   163,    -1,    -1,   166,   167,    -1,
     169,   170,  1997,   172,   173,   174,    -1,  1035,    -1,    -1,
      -1,    -1,    -1,  2008,    -1,  2010,    -1,    -1,   187,    -1,
      -1,    -1,    -1,    19,    20,    -1,    -1,    -1,    -1,   198,
     199,  1059,    -1,    -1,    30,    -1,    -1,    -1,    -1,  1020,
      -1,    -1,  1070,    -1,  2039,    -1,    -1,   692,   693,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   701,    -1,    -1,    -1,
    2055,    -1,    -1,    -1,    -1,    -1,  2061,    -1,    -1,    -1,
    2065,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  2079,  2080,    -1,    -1,  1116,  1117,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,  1152,  1153,  1154,  1155,  1156,  1157,
    1158,  1159,    69,    -1,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,
    1188,  1189,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,  1152,  1153,    -1,    -1,  1156,    -1,    -1,    -1,    -1,
    1208,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,  1189,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    -1,  1208,   873,    -1,
      -1,    -1,  1260,    -1,    -1,   880,   881,   104,    -1,    -1,
      -1,    -1,    -1,    -1,   240,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    -1,   203,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   104,
    1308,  1309,  1310,  1311,  1312,  1313,    -1,   104,  1316,  1317,
      -1,  1319,    -1,    -1,    -1,  1323,   163,    -1,    -1,   166,
      -1,    -1,   169,   170,    -1,   172,   173,   174,    -1,   176,
    1338,  1339,    -1,  1341,    -1,    -1,   141,   142,   143,   144,
     145,    -1,    -1,    -1,   141,   142,   143,   144,   145,    -1,
      -1,   198,    -1,   978,    -1,  1363,    -1,  1365,    -1,    -1,
      -1,    -1,    -1,    -1,   169,   170,    -1,   172,   173,   174,
      -1,   996,   169,   170,    -1,   172,   173,   174,  1003,    -1,
    1341,    -1,  1390,    -1,  1009,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,
      -1,   198,  1363,   379,  1365,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   388,    -1,    -1,    -1,    -1,    -1,    -1,   395,
      -1,    -1,  1430,    -1,    -1,  1433,    -1,    -1,    -1,  1390,
    1055,   407,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   418,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,  1493,    -1,  1495,    -1,    -1,
      -1,    69,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,  1130,    -1,    -1,    -1,  1134,
      -1,  1136,    -1,    -1,  1139,    -1,    -1,    -1,    69,    -1,
      -1,    81,    -1,    -1,    -1,    -1,   502,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1549,    -1,  1551,   104,  1553,    -1,    -1,    -1,    -1,
    1558,  1559,    -1,    -1,    -1,  1563,    -1,  1565,    -1,    -1,
    1568,    -1,    -1,    -1,    -1,    -1,    -1,   543,    -1,    -1,
     130,    -1,    -1,    -1,  1582,    -1,    -1,  1585,    -1,    -1,
      31,   141,   142,   143,   144,   145,    -1,    -1,  1549,    -1,
    1551,    -1,  1553,    -1,    -1,    -1,    -1,  1558,    -1,    -1,
      -1,    -1,  1563,    -1,  1565,    -1,    -1,  1568,    -1,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    68,    -1,    -1,
    1245,    -1,  1247,    -1,   600,   203,    -1,    -1,    -1,    -1,
      81,    -1,    -1,  1641,    -1,  1643,    81,  1645,   198,   199,
      -1,    -1,  1650,    -1,  1652,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,   633,    -1,   104,
      -1,   112,    -1,  1288,  1672,    -1,  1291,    -1,    -1,    -1,
     121,   122,   123,   124,   125,   126,  1684,  1685,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1694,    -1,    -1,   140,
     141,   142,   143,   144,   145,   146,   141,   142,   143,   144,
     145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1672,   163,    -1,    -1,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,   169,   170,  1351,   172,   173,   174,
      -1,    -1,    -1,  1358,    -1,    -1,   187,    -1,    -1,    -1,
      -1,    -1,    -1,   194,  1752,    -1,   722,   198,   199,    -1,
      -1,    -1,    -1,   198,   199,    10,    11,    12,    -1,    -1,
      -1,  1769,  1770,  1771,    -1,    -1,    -1,    -1,  1776,    -1,
    1778,    -1,    -1,    -1,    -1,    -1,    31,    -1,  1786,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,  1426,  1427,    -1,    -1,    -1,    81,    -1,  1769,  1770,
    1771,    -1,    -1,    -1,    69,  1776,    -1,    -1,    -1,    -1,
      -1,    -1,   798,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1462,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   837,    -1,   839,    -1,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,  1882,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,   865,
     866,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,    -1,  1912,  1913,   882,   883,   884,   885,
     886,    -1,    -1,    31,  1539,  1540,    -1,    -1,  1543,   895,
      -1,  1882,    -1,   198,    -1,  1933,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1949,    -1,    -1,    -1,   200,    -1,  1955,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,  1581,    -1,    -1,    -1,
      -1,    -1,   938,    81,    -1,    83,    -1,  1592,    -1,    -1,
      -1,    -1,  1933,    -1,    -1,    -1,    -1,    -1,   954,   955,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,  1949,  1997,
      -1,    -1,    -1,    -1,  1955,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   121,   122,   123,   124,   125,   126,    -1,
      -1,    -1,    -1,   989,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,  2039,    -1,    -1,    -1,    -1,  1997,    -1,    -1,    -1,
      -1,    -1,  1667,    -1,  1020,   163,    -1,  2055,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1692,    -1,   187,
      -1,    -1,    -1,    -1,    -1,    -1,   194,    -1,    -1,    -1,
     198,   199,    -1,  1059,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,  1717,    -1,  1719,    -1,    -1,    -1,    -1,  1724,
      -1,  1726,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1116,  1117,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1784,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1152,  1153,  1154,  1155,
    1156,  1157,  1158,  1159,    -1,    -1,  1162,  1163,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
    1186,  1187,  1188,  1189,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1208,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,  1889,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,   203,    -1,  1904,
      10,    11,    12,  1908,  1260,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,  1927,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
    1316,    81,    -1,    -1,    -1,    -1,    -1,  1323,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1982,    -1,    -1,
      -1,    -1,  1338,  1339,   104,  1341,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,  2008,    57,  2010,    -1,  1363,    -1,  1365,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,   146,   203,    -1,    -1,
      -1,    -1,    -1,    -1,  1390,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,  2061,    -1,    -1,    -1,
    2065,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,    -1,    -1,  2079,  2080,    -1,  1433,   198,   199,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,  1549,    -1,  1551,    -1,  1553,    -1,    -1,
      -1,    -1,  1558,  1559,    -1,    -1,    -1,  1563,    48,  1565,
      50,    51,  1568,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,   113,   114,   115,   203,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,   131,   132,   133,    -1,    -1,  1652,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,  1672,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,   189,
      -1,   191,    -1,   193,   194,   195,    -1,    -1,   198,   199,
      -1,   201,   202,    -1,    -1,   205,   206,   207,   208,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,  1769,  1770,  1771,    -1,    -1,    -1,    -1,
    1776,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,  1785,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,   113,   114,   115,    -1,   117,    -1,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,   131,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,  1882,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,   189,
      -1,   191,    -1,   193,   194,   195,    -1,  1933,   198,   199,
      -1,   201,   202,   203,    -1,   205,   206,   207,   208,    -1,
      -1,    -1,    -1,  1949,    -1,    -1,    -1,    -1,    -1,  1955,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,  1984,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1997,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,
      93,    94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,
      -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,   112,
     113,   114,   115,    -1,   117,    -1,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,   129,   130,   131,   132,
     133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,    -1,
      -1,    -1,    -1,    -1,   187,   188,   189,    -1,   191,    -1,
     193,   194,   195,    -1,    -1,   198,   199,    -1,   201,   202,
     203,    -1,   205,   206,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,
      97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,    -1,   109,   110,   111,   112,    -1,   114,   115,    -1,
     117,    -1,    -1,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,   129,   130,    -1,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,
      -1,   198,   199,    -1,   201,   202,   203,    -1,   205,   206,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,
      -1,    92,    93,    94,    95,    -1,    97,    -1,    99,    -1,
     101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,
     111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,   129,   130,
      -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,
     181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,
      -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,
     201,   202,   203,    -1,   205,   206,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,   118,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,
     195,    -1,    -1,   198,   199,    -1,   201,   202,    -1,    -1,
     205,   206,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,
      99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,
     109,   110,   111,   112,    -1,   114,   115,    -1,   117,    -1,
      -1,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
     129,   130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
      -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,
     199,    -1,   201,   202,   203,    -1,   205,   206,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,
      93,    94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,
      -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,   112,
      -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,   129,   130,    -1,   132,
     133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,    -1,
      -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,
     193,   194,   195,    -1,    -1,   198,   199,    -1,   201,   202,
     203,    -1,   205,   206,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,
      97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,    -1,   109,   110,   111,   112,    -1,   114,   115,    -1,
     117,    -1,    -1,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,   129,   130,    -1,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,
      -1,   198,   199,    -1,   201,   202,   203,    -1,   205,   206,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,
      -1,    92,    93,    94,    95,    -1,    97,    -1,    99,    -1,
     101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,
     111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,   129,   130,
      -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,
     181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,
      -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,
     201,   202,   203,    -1,   205,   206,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,
     195,    -1,    -1,   198,   199,    -1,   201,   202,    -1,    -1,
     205,   206,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,
      99,    -1,   101,   102,    -1,   104,   105,    -1,    -1,    -1,
     109,   110,   111,   112,    -1,   114,   115,    -1,   117,    -1,
      -1,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
     129,   130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
      -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,
     199,    -1,   201,   202,    -1,    -1,   205,   206,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,
      93,    94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,
      -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,   112,
      -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,   129,   130,    -1,   132,
     133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,    -1,
      -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,
     193,   194,   195,    -1,    -1,   198,   199,    -1,   201,   202,
     203,    -1,   205,   206,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      77,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,
      97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,    -1,   109,   110,   111,   112,    -1,   114,   115,    -1,
     117,    -1,    -1,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,   129,   130,    -1,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,
      -1,   198,   199,    -1,   201,   202,    -1,    -1,   205,   206,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,
      -1,    92,    93,    94,    95,    -1,    97,    -1,    99,    -1,
     101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,
     111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,   129,   130,
      -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,
     181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,
      -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,
     201,   202,   203,    -1,   205,   206,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,   100,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,
     195,    -1,    -1,   198,   199,    -1,   201,   202,    -1,    -1,
     205,   206,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,    98,
      99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,
     109,   110,   111,   112,    -1,   114,   115,    -1,   117,    -1,
      -1,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
     129,   130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
      -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,
     199,    -1,   201,   202,    -1,    -1,   205,   206,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,
      93,    94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,
      -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,   112,
      -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,   129,   130,    -1,   132,
     133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,    -1,
      -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,
     193,   194,   195,    -1,    -1,   198,   199,    -1,   201,   202,
     203,    -1,   205,   206,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,
      97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,    -1,   109,   110,   111,   112,    -1,   114,   115,    -1,
     117,    -1,    -1,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,   129,   130,    -1,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,
      -1,   198,   199,    -1,   201,   202,   203,    -1,   205,   206,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,
      -1,    92,    93,    94,    95,    -1,    97,    -1,    99,    -1,
     101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,
     111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,   129,   130,
      -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,
     181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,
      -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,
     201,   202,   203,    -1,   205,   206,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,
     195,    -1,    -1,   198,   199,    -1,   201,   202,   203,    -1,
     205,   206,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,
      99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,
     109,   110,   111,   112,    -1,   114,   115,    -1,   117,    -1,
      -1,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
     129,   130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
      -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,
     199,    -1,   201,   202,   203,    -1,   205,   206,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,
      93,    94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,
      -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,   112,
      -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,   129,   130,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,    -1,
      -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,
     193,   194,   195,    -1,    -1,   198,   199,    -1,   201,   202,
      -1,    -1,   205,   206,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,
      97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,    -1,   109,   110,   111,   112,    -1,   114,   115,    -1,
     117,    -1,    -1,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,   129,   130,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,
      -1,   198,   199,    -1,   201,   202,    -1,    -1,   205,   206,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,
      -1,    92,    93,    94,    95,    -1,    97,    -1,    99,    -1,
     101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,
     111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,   129,   130,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,
     181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,
      -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,
     201,   202,    -1,    -1,   205,   206,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,
     195,    -1,    -1,   198,   199,    -1,   201,   202,    -1,    -1,
     205,   206,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,
      99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,
     109,   110,   111,   112,    -1,   114,   115,    -1,   117,    -1,
      -1,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
     129,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
      -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,
     199,    -1,   201,   202,    -1,    -1,   205,   206,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,
      93,    94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,
      -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,   112,
      -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,   129,   130,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,    -1,
      -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,
     193,   194,   195,    -1,    -1,   198,   199,    -1,   201,   202,
      -1,    -1,   205,   206,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,
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
      -1,   201,    -1,    -1,    -1,   205,   206,   207,   208,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,   122,   123,   124,   125,   126,    -1,    -1,   129,   130,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    -1,   176,    -1,   178,    -1,    -1,
     181,     3,     4,     5,     6,     7,   187,   188,    -1,    -1,
      -1,    13,   193,   194,   195,    -1,    -1,   198,   199,    -1,
      -1,    -1,    -1,    -1,   205,   206,   207,   208,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
     122,   123,   124,   125,   126,    -1,    -1,   129,   130,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,   176,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,
      -1,   193,   194,   195,    -1,    -1,   198,   199,     3,     4,
       5,     6,     7,   205,   206,   207,   208,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,
     195,    -1,    -1,   198,   199,    -1,    -1,   202,    -1,    -1,
     205,   206,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,
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
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
      -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,
     199,     3,     4,     5,     6,     7,   205,   206,   207,   208,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
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
      -1,    -1,    -1,   205,   206,   207,   208,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,   109,    -1,    -1,   112,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     195,    -1,    -1,   198,   199,    -1,   201,    -1,    -1,    -1,
     205,   206,   207,   208,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   198,   199,    -1,   201,    -1,    -1,    -1,   205,
     206,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,
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
     181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,
      -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,   200,
      -1,    -1,    -1,    -1,   205,   206,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     205,   206,   207,   208,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   178,    -1,    -1,   181,     3,     4,     5,     6,     7,
     187,   188,    -1,    -1,    -1,    13,   193,   194,   195,    -1,
      -1,   198,   199,    -1,    -1,    -1,    -1,    -1,   205,   206,
     207,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     178,    -1,    -1,   181,     3,     4,     5,     6,     7,   187,
     188,    -1,    -1,    -1,    13,   193,   194,   195,    -1,    -1,
     198,   199,    -1,    -1,    -1,    -1,    -1,   205,   206,   207,
     208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,
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
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,
      -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,
       3,     4,     5,     6,     7,   205,   206,   207,   208,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,     3,     4,   178,     6,     7,   181,    -1,    10,    11,
      12,    13,   187,   188,    -1,    -1,    -1,    -1,   193,   194,
     195,    -1,    -1,   198,   199,    -1,    28,    29,    -1,    -1,
     205,   206,   207,   208,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    57,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    83,    84,    -1,    -1,    87,    -1,    -1,    -1,    -1,
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
     202,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    83,    84,    -1,    -1,    87,
      -1,    -1,    -1,    -1,   198,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    -1,   130,    -1,   132,   133,   134,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,    -1,   163,    10,    11,    12,    13,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,   177,
      -1,    -1,   180,    -1,    28,    29,    -1,    31,    -1,    -1,
     188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     198,    -1,    -1,    -1,   202,    -1,    -1,   205,    -1,    -1,
      -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    -1,   130,    -1,    -1,   133,
     134,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,
      92,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,   175,   104,   177,    -1,    -1,   180,    -1,     3,     4,
      -1,     6,     7,   187,   188,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,   203,
      -1,    -1,    -1,    28,    29,    -1,    31,    -1,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   163,    57,    -1,   166,    -1,    -1,   169,   170,    -1,
     172,   173,   174,    68,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,   198,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    -1,   130,    -1,    -1,   133,   134,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,   104,   177,    -1,    -1,   180,    -1,     3,     4,    -1,
       6,     7,   187,   188,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,   198,   199,    -1,    -1,    -1,   203,    -1,
      -1,    -1,    28,    29,    -1,    31,    -1,    -1,   141,   142,
     143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     163,    57,    -1,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    68,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    87,    -1,    -1,    -1,   198,    -1,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    -1,   130,    -1,   132,   133,   134,   135,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
     146,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    83,
      -1,    85,    -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,   175,
     104,   177,    -1,    -1,   180,    -1,     3,     4,    -1,     6,
       7,   187,   188,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    29,    -1,    31,    -1,    -1,   141,   142,   143,
     144,   145,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      57,    57,    -1,    -1,    -1,   169,   170,    -1,   172,   173,
     174,    68,    -1,    69,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,   198,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,    -1,   130,    -1,    -1,   133,   134,   135,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,   104,
     177,    -1,    -1,   180,    -1,    -1,    -1,   112,   113,    -1,
     187,   188,   189,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,   198,   199,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,    28,    29,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,   174,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,   198,    81,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,    -1,   130,    -1,    -1,   133,   134,   135,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,   104,
     177,    -1,    -1,   180,    -1,     3,     4,     5,     6,     7,
     187,   188,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   198,   199,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    57,
      -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    69,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,   198,    -1,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   163,   164,   165,    -1,    -1,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,   177,
     178,    -1,   180,    -1,    -1,    -1,    -1,    -1,    -1,   187,
      -1,   189,    -1,   191,    -1,   193,   194,    -1,     3,     4,
     198,     6,     7,    12,    -1,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    28,    29,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    -1,   130,    -1,   132,   133,   134,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,   163,    10,
      11,    12,    13,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,   177,    -1,    -1,   180,    -1,    28,    29,    -1,
      -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   198,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    -1,   130,
      -1,   132,   133,   134,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,   163,    10,    11,    12,    13,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,   177,    -1,    -1,   180,
      -1,    28,    29,    -1,    -1,    -1,    -1,   188,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,    -1,   130,    -1,    -1,   133,   134,   135,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,    -1,
      -1,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
     177,    -1,    -1,   180,    10,    11,    12,    -1,    -1,    -1,
      -1,   188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   198,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
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
      52,    53,    54,    55,    -1,    57,    -1,   203,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,   201,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,   201,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   201,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   201,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
     201,    57,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    31,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   201,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    32,    -1,   200,    -1,   163,    -1,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    -1,   187,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,    70,   198,   199,    -1,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,   200,    -1,   163,    -1,    -1,   166,
     167,    -1,   169,   170,    92,   172,   173,   174,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
     187,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      -1,   198,   199,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   140,   141,   142,   143,   144,   145,    70,   147,
     148,   149,   150,   151,    -1,    69,    78,    79,    80,    81,
     158,    83,    84,    -1,    -1,   163,   164,   165,   166,   167,
      92,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
     178,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,   187,
     188,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,   121,
     198,   199,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    70,   147,   148,   149,   150,   151,
      -1,    -1,    78,    79,    80,    81,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    92,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,   178,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   193,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,
      50,    51,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,   145,
      70,   147,   148,   149,   150,   151,    -1,    -1,    78,    79,
      80,    81,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    92,   169,   170,    -1,   172,   173,   174,    -1,
      -1,    -1,   178,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,   187,   188,    -1,    -1,    -1,    -1,   193,    -1,    -1,
      -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,   158,    83,
      84,    -1,    -1,   163,    -1,    -1,   166,   167,    92,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   198,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,   140,   141,   142,   143,
     144,   145,    70,   147,   148,   149,   150,   151,    -1,    69,
      78,    79,    80,    81,   158,    83,    84,    -1,    -1,   163,
     164,   165,   166,   167,    92,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,   178,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,
      -1,    -1,    -1,   121,   198,   199,    -1,    -1,   202,    -1,
      -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,    70,    71,    -1,
     178,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,   193,    -1,    -1,    -1,    92,
     198,   199,    -1,    -1,    -1,    -1,    -1,   205,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,    70,   147,   148,   149,   150,   151,    -1,
      -1,    78,    79,    80,    81,   158,    83,    84,    -1,    -1,
     163,   164,   165,   166,   167,    92,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     193,    -1,    -1,    -1,   121,   198,   199,    -1,    -1,    -1,
      -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    70,
     147,   148,   149,   150,   151,    -1,    -1,    78,    79,    80,
      81,   158,    83,    84,    -1,    -1,   163,   164,   165,   166,
     167,    92,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,   178,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,
     121,   198,   199,    -1,    -1,    -1,    -1,    -1,   205,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   193,    -1,    -1,    -1,    -1,   198,   199,    -1,
      -1,    -1,    30,    31,   205,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    31,    -1,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    31,    -1,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,   138,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,   138,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,   138,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,   138,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    69,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,   138,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
     193,    92,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,
      -1,    -1,   205,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    69,    -1,    78,    79,    80,    81,   158,    83,    84,
      -1,    -1,   163,   164,   165,   166,   167,    92,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   193,    -1,    -1,    -1,   121,   198,   199,    -1,
      -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    81,    -1,    -1,
      -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     104,    -1,    -1,   178,    -1,    -1,    -1,    -1,   112,   113,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,
      -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,
     205,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,
      -1,    -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   189,    -1,    -1,    -1,    27,
      -1,    -1,    30,    31,   198,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    30,    31,    32,    33,
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
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,    69,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      69,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    -1,    69,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69
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
     201,   201,   496,   336,   337,   200,   308,   167,   414,   203,
     203,   359,    83,    83,   176,    14,    83,   359,   303,   303,
     119,   362,   510,   203,   510,   200,   200,   203,   202,   203,
     311,   300,   138,   444,   444,   444,   444,   373,   203,   233,
     239,   242,    32,   236,   287,   233,   523,   200,   432,   138,
     138,   138,   233,   423,   423,   501,    14,   217,     9,   201,
     202,   499,   496,   322,   183,   202,     9,   201,     3,     4,
       5,     6,     7,    10,    11,    12,    13,    27,    28,    29,
      57,    71,    72,    73,    74,    75,    76,    77,    87,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   139,   140,   147,   148,   149,   150,   151,   163,   164,
     165,   175,   177,   178,   180,   187,   189,   191,   193,   194,
     216,   420,   421,     9,   201,   167,   171,   216,   330,   331,
     332,   201,    14,     9,   201,   256,   341,   499,   499,    14,
     257,   341,   523,   203,   309,   310,   499,    14,    83,   359,
     200,   200,   199,   510,   198,   507,   362,   510,   308,   203,
     200,   444,   138,   138,    32,   236,   286,   287,   233,   432,
     432,   432,   203,   201,   201,   432,   423,   315,   523,   323,
     324,   431,   320,    14,    32,    51,   325,   328,     9,    36,
     200,    31,    50,    53,   432,    83,   218,   500,   201,    14,
     523,   256,   201,   341,   201,    14,   359,    38,    83,   411,
     202,   508,   509,   523,   201,   202,   333,   510,   507,   203,
     510,   444,   444,   233,   100,   252,   203,   216,   229,   316,
     317,   318,     9,   438,     9,   438,   203,   432,   421,   421,
      68,   326,   331,   331,    31,    50,    53,    14,   183,   199,
     432,   500,   201,   432,    83,     9,   439,   231,     9,   439,
      14,   511,   231,   202,   333,   333,    98,   201,   116,   243,
     162,   103,   523,   184,   431,   174,   432,   512,   313,   199,
      38,    83,   200,   203,   509,   523,   203,   231,   201,   199,
     180,   255,   216,   336,   337,   184,   184,   298,   299,   457,
     314,    83,   203,   423,   253,   177,   216,   201,   200,     9,
     439,    87,   124,   125,   126,   339,   340,   298,    83,   283,
     201,   510,   457,   524,   524,   200,   200,   201,   507,    87,
     339,    83,    38,    83,   176,   510,   202,   201,   202,   334,
     524,   524,    83,   176,    14,    83,   507,   233,   231,    83,
      38,    83,   176,    14,    83,   359,   334,   203,   203,    83,
     176,    14,    83,   359,    14,    83,   359,   359
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
     310,   311,   311,   312,   312,   312,   312,   312,   312,   312,
     313,   312,   314,   312,   312,   312,   312,   312,   312,   312,
     312,   315,   315,   315,   316,   317,   317,   318,   318,   319,
     319,   320,   320,   321,   321,   322,   322,   322,   322,   322,
     322,   322,   323,   323,   324,   325,   325,   326,   326,   327,
     327,   328,   329,   329,   329,   330,   330,   330,   330,   331,
     331,   331,   331,   331,   331,   331,   332,   332,   332,   333,
     333,   334,   334,   335,   335,   336,   336,   337,   337,   338,
     338,   338,   338,   338,   338,   338,   339,   339,   340,   340,
     340,   341,   341,   341,   341,   342,   342,   343,   343,   344,
     344,   345,   346,   347,   347,   347,   347,   347,   347,   348,
     348,   349,   349,   350,   350,   350,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   359,   359,   359,   359,
     360,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   362,
     362,   364,   363,   365,   363,   367,   366,   368,   366,   369,
     366,   370,   366,   371,   366,   372,   372,   372,   373,   373,
     374,   374,   375,   375,   376,   376,   377,   377,   378,   379,
     379,   380,   380,   381,   381,   382,   382,   383,   383,   384,
     384,   385,   385,   386,   387,   388,   389,   390,   391,   392,
     393,   394,   395,   396,   397,   398,   399,   400,   401,   402,
     403,   404,   404,   405,   405,   406,   406,   407,   408,   409,
     409,   410,   410,   410,   410,   410,   410,   410,   410,   410,
     410,   410,   410,   411,   411,   411,   411,   412,   413,   413,
     414,   414,   415,   415,   415,   416,   416,   417,   418,   418,
     419,   419,   419,   420,   420,   420,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   422,
     423,   423,   424,   424,   424,   424,   424,   425,   425,   426,
     426,   426,   426,   427,   427,   427,   428,   428,   428,   429,
     429,   429,   430,   430,   431,   431,   431,   431,   431,   431,
     431,   431,   431,   431,   431,   431,   431,   431,   431,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   433,   433,   434,   435,   435,
     436,   436,   436,   436,   436,   436,   436,   437,   437,   438,
     438,   439,   439,   440,   440,   440,   440,   441,   441,   441,
     441,   441,   442,   442,   442,   442,   443,   443,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   445,   445,   446,   446,   447,   447,
     447,   447,   448,   448,   449,   449,   450,   450,   451,   451,
     452,   452,   453,   453,   455,   454,   456,   457,   457,   458,
     458,   459,   459,   459,   460,   460,   461,   461,   462,   462,
     463,   463,   464,   464,   464,   465,   465,   466,   466,   467,
     467,   468,   468,   468,   468,   468,   468,   468,   468,   468,
     468,   468,   469,   470,   470,   470,   470,   470,   470,   470,
     471,   471,   471,   471,   471,   471,   471,   471,   471,   471,
     471,   472,   473,   473,   474,   474,   474,   475,   475,   475,
     476,   476,   477,   477,   477,   478,   478,   478,   479,   479,
     480,   480,   481,   481,   481,   481,   481,   481,   482,   482,
     482,   482,   482,   483,   483,   483,   483,   483,   483,   484,
     484,   485,   485,   485,   485,   485,   485,   485,   485,   486,
     486,   487,   487,   487,   487,   488,   488,   489,   489,   489,
     489,   490,   490,   490,   490,   491,   491,   491,   491,   491,
     491,   492,   492,   492,   493,   493,   493,   493,   493,   493,
     493,   493,   493,   493,   493,   494,   494,   495,   495,   496,
     496,   497,   497,   497,   497,   498,   498,   499,   499,   500,
     500,   501,   501,   502,   502,   503,   503,   504,   505,   505,
     505,   505,   505,   505,   506,   506,   506,   506,   507,   507,
     508,   508,   509,   509,   510,   510,   511,   511,   512,   513,
     513,   514,   514,   514,   514,   515,   515,   515,   516,   516,
     516,   516,   517,   517,   518,   518,   518,   518,   519,   520,
     521,   521,   522,   522,   523,   523,   523,   523,   523,   523,
     523,   523,   523,   523,   523,   524,   524
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
       3,     2,     0,     3,     4,     4,     5,     2,     2,     2,
       0,    11,     0,    12,     3,     3,     3,     4,     4,     3,
       5,     2,     2,     0,     6,     5,     4,     3,     1,     1,
       3,     4,     1,     2,     1,     1,     5,     6,     1,     1,
       4,     1,     1,     3,     2,     2,     0,     2,     0,     1,
       3,     1,     1,     1,     1,     3,     4,     4,     4,     1,
       1,     2,     2,     2,     3,     3,     1,     1,     1,     1,
       3,     1,     3,     1,     1,     1,     0,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     1,     1,
       1,     3,     5,     1,     3,     5,     4,     3,     3,     3,
       4,     3,     3,     3,     3,     2,     2,     1,     1,     3,
       1,     1,     0,     1,     2,     4,     3,     3,     6,     2,
       3,     2,     3,     6,     3,     1,     1,     1,     1,     1,
       3,     6,     3,     4,     6,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     5,     4,     3,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     1,     5,
       0,     0,    12,     0,    13,     0,     4,     0,     7,     0,
       5,     0,     3,     0,     6,     2,     2,     4,     1,     1,
       5,     3,     5,     3,     2,     0,     2,     0,     4,     4,
       3,     2,     0,     5,     3,     2,     0,     5,     3,     2,
       0,     5,     3,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     2,     0,     2,     0,     2,     0,     4,     4,     4,
       4,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     3,     4,     1,     2,     4,     2,     6,
       0,     1,     0,     5,     4,     2,     0,     1,     1,     3,
       1,     3,     1,     1,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       1,     1,     1,     1,     1,     1,     3,     1,     3,     1,
       1,     1,     3,     1,     1,     1,     2,     1,     0,     0,
       1,     1,     3,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     2,     1,
       1,     4,     3,     4,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     4,     3,     1,     3,     3,     1,
       1,     1,     1,     1,     3,     3,     3,     2,     0,     1,
       0,     1,     0,     5,     3,     3,     1,     1,     1,     1,
       3,     2,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     3,     1,     2,     2,     4,     3,     4,     1,     1,
       1,     1,     1,     1,     3,     1,     2,     0,     5,     3,
       3,     1,     3,     1,     2,     0,     5,     3,     2,     0,
       3,     0,     4,     2,     0,     3,     3,     1,     0,     1,
       1,     1,     1,     3,     1,     1,     1,     3,     1,     1,
       3,     3,     2,     2,     2,     2,     4,     5,     5,     5,
       5,     1,     1,     1,     1,     1,     1,     3,     3,     4,
       4,     3,     3,     1,     1,     1,     1,     3,     1,     4,
       1,     1,     1,     1,     1,     3,     3,     1,     1,     4,
       4,     3,     1,     1,     7,     9,     9,     7,     6,     8,
       1,     2,     4,     4,     1,     1,     1,     4,     1,     0,
       1,     2,     1,     1,     1,     3,     3,     3,     0,     1,
       1,     3,     3,     2,     3,     6,     0,     1,     4,     2,
       0,     5,     3,     3,     1,     6,     4,     4,     2,     2,
       0,     5,     3,     3,     1,     2,     0,     5,     3,     3,
       1,     2,     2,     1,     2,     1,     4,     3,     3,     6,
       3,     1,     1,     1,     4,     4,     4,     4,     4,     4,
       2,     2,     4,     2,     2,     1,     3,     3,     3,     0,
       2,     5,     6,     6,     7,     1,     2,     1,     2,     1,
       4,     1,     4,     3,     0,     1,     3,     2,     1,     2,
       4,     3,     3,     1,     4,     2,     2,     0,     0,     3,
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
#line 760 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
#line 7368 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 763 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 7376 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 770 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 7382 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 771 "hphp.y" /* yacc.c:1646  */
    { }
#line 7388 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 7394 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 775 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7400 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7406 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7412 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7418 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 779 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 7424 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 7432 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 783 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 7439 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 785 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 7445 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 786 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7451 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 787 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 7457 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 788 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7463 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 789 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7471 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 793 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7480 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 798 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7489 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 803 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7496 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 806 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7503 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 809 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7511 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 813 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7519 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 817 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7527 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 821 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7535 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 825 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7543 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 828 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7550 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7556 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7562 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7568 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7574 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7580 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7586 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7592 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 840 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7598 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7604 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 842 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7610 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 843 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7616 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 844 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7622 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 845 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7628 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 927 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7634 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 929 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7640 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 934 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7646 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 935 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7653 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 941 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7659 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 945 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7665 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 946 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7671 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 948 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7677 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 950 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7683 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 955 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7689 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 956 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7696 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 962 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7702 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 966 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7709 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 968 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7716 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 970 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7723 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 975 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7729 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 977 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7735 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 980 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7741 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 982 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7747 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 983 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7753 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 988 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7762 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 995 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7771 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1003 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7778 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1006 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7785 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1012 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 7791 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1013 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval)); }
#line 7797 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1018 "hphp.y" /* yacc.c:1646  */
    {
                                         _p->onUsing((yyval), (yyvsp[-2]), true, (yyvsp[-1]), nullptr);
                                       }
#line 7805 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1025 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7811 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1026 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7817 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1031 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 7823 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1032 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval), (yyval), (yyvsp[0])); }
#line 7830 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1037 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7836 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7842 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1039 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7848 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1040 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7854 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7860 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1047 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7866 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1052 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7872 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1053 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7879 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7887 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1059 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7894 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1062 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7902 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1066 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7909 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1068 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7917 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1071 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7924 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1073 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7932 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7938 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1077 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7944 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1078 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7950 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1079 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7956 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1080 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7962 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1081 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7968 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1082 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7974 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7980 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7986 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1085 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7992 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7998 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1087 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 8004 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1088 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 8010 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1089 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 8016 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1090 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 8023 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1094 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 8030 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1096 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 8038 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1101 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 8045 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1103 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 8053 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1107 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 8061 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1116 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 8067 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1117 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8073 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1120 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false); }
#line 8079 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1121 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 8085 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1123 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8093 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1127 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8101 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1131 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8109 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1135 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8117 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1139 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8125 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1143 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8133 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1148 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8141 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1151 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 8147 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 8156 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1156 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8162 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1157 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8168 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1158 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8174 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1159 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8180 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1160 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8186 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1161 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8192 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1162 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 8198 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1163 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8204 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1164 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8210 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1165 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 8216 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1166 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8222 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1167 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 8232 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1189 "hphp.y" /* yacc.c:1646  */
    { _p->pushLabelScope(LS::Using);
                                         _p->onNewLabelScope(false);
                                         (yyval) = (yyvsp[-1]); }
#line 8240 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1195 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1; }
#line 8246 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1196 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8252 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1205 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), nullptr, (yyvsp[-2]));
                                         _p->onExprListElem((yyval), &(yyval), (yyvsp[0])); }
#line 8259 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1207 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0])); }
#line 8265 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1217 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8271 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1218 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8277 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1222 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false); }
#line 8283 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1223 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8289 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1232 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 8295 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1233 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8301 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1237 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::Finally);}
#line 8308 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1239 "hphp.y" /* yacc.c:1646  */
    { _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->popLabelScope();
                                         _p->onCompleteLabelScope(false);}
#line 8316 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1245 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8322 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1246 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8328 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1250 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 8334 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1251 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8340 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1255 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 8346 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1261 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8355 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1268 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8365 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1276 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8374 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1283 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8384 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1291 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8393 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1297 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8403 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1306 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8410 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1310 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 8416 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1314 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8423 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1318 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 8429 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1324 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8436 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 8454 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1342 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8461 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 8479 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1359 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8486 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1362 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8494 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1367 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8501 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1370 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8509 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1376 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 8515 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1379 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 8521 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1383 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8528 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1386 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8539 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1394 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8546 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1397 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8557 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1405 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8563 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1406 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 8570 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1410 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8576 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1413 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8582 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1416 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 8588 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1417 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 8594 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1418 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 8602 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1421 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 8608 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1422 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 8614 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1426 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8620 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1427 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8626 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1430 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8632 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1431 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8638 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1434 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8644 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1435 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8650 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1438 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8656 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1440 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8662 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1443 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8668 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1445 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8674 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8680 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1450 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8686 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8692 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8698 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1455 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8704 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8710 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8716 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8722 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8728 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1469 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8734 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1471 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8740 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1474 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8746 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1476 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8752 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1480 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8758 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1482 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8765 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8771 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8777 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8783 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8789 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1495 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8795 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1497 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8801 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1498 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8807 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1501 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8813 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1502 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8819 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1507 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8825 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1508 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8831 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1513 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8837 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1514 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8843 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8849 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1518 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8855 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1521 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8861 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1522 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8867 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1530 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8874 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1536 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8881 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1542 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8889 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1546 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8895 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1550 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8902 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1555 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8909 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1560 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8917 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1563 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8923 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1569 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8931 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1574 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8939 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1579 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8947 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1585 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8955 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1591 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8963 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1597 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8971 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1603 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8979 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1609 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8987 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1616 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8995 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1623 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 9003 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1632 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 9010 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1637 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 9017 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1642 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 9025 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1646 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9031 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1649 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 9038 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1653 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 9045 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1657 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 9053 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1660 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9059 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1665 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9067 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1669 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9075 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1673 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9083 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1678 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9091 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1683 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9099 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1688 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9107 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1693 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9115 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1698 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9123 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1704 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9131 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1710 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9139 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1716 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 9145 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1717 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 9151 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1718 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 9157 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9163 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1724 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9169 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1727 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,false);}
#line 9176 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1729 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::InOut,false);}
#line 9183 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1731 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::Ref,false);}
#line 9190 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1733 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,true);}
#line 9197 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1736 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::In,false);}
#line 9204 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1739 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::In,true);}
#line 9211 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1742 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::Ref,false);}
#line 9218 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1745 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::InOut,false);}
#line 9225 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1750 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9231 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1751 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 9237 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1754 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9243 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1755 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 9249 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1756 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 9255 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1760 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9261 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1762 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9267 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1763 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 9273 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1764 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9279 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1769 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9285 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1770 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9291 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1773 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL,NULL);}
#line 9298 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1778 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9304 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1784 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9310 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1785 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9316 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1789 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-2]),(yyvsp[-1]),NULL,NULL);}
#line 9323 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1793 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-2]),NULL);}
#line 9330 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1797 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-2]),(yyvsp[-1]),NULL,&(yyvsp[-3]));}
#line 9337 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1802 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-2]),&(yyvsp[-4]));}
#line 9344 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1804 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL,NULL);}
#line 9351 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1807 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL,NULL,true);}
#line 9358 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1809 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9364 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1812 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9372 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1819 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9382 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1827 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9390 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1834 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9400 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1840 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 9406 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1842 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9412 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1844 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9418 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 9424 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1848 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 9430 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1849 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 9437 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1852 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 9443 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1855 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9449 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1856 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9455 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1857 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 9461 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1863 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 9467 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1868 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 9474 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1871 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 9482 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1878 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 9488 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1879 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 9495 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1884 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 9502 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1887 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 9508 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1894 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 9515 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1896 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 9521 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1900 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9527 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1905 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9533 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1907 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9539 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1909 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9545 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1910 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 9556 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1916 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 9562 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1918 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 9568 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1919 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 9574 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1923 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval),nullptr,(yyvsp[0])); }
#line 9580 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1925 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval),&(yyvsp[-2]),(yyvsp[0])); }
#line 9586 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1930 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9592 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1933 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9598 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1934 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 9604 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1938 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 9610 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1939 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 9616 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1943 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 9623 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1946 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 9630 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1951 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 9637 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 9643 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 9650 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1959 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 9656 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 9662 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 9668 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 9674 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1966 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 9680 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1970 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9686 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1971 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 9692 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1972 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 9698 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 9704 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1974 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 9710 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 9716 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1978 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 9722 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9730 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1985 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9736 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9742 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1990 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9748 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9754 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1995 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9760 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1996 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9766 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9772 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9778 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9784 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9790 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9796 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 2009 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9802 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 2012 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9808 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9814 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9820 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9826 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 2016 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9832 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9838 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9844 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 2022 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9850 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 2023 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9856 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 2026 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9862 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9868 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 2028 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9874 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 2032 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9880 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 2034 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9886 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 2035 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9892 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 2036 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9898 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 2040 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9904 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 2042 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9910 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 2051 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9916 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 2053 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9922 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 2057 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9930 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 2061 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9937 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 2065 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9943 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 2069 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9949 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 2073 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9955 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 2075 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9961 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 2076 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9967 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 2077 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9973 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 2078 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9979 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 2079 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9985 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 2082 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9991 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 2083 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9997 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 2087 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10003 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 2088 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10009 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 2092 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 10015 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 2093 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 10021 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 2094 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 10027 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 2095 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10033 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 2099 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10039 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 2104 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 10045 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 2108 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 10051 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 2112 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10057 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 2116 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 10063 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 2120 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10069 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 2125 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 10075 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 2129 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10081 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 2133 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10087 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 2134 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10093 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 2135 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10099 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 2136 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10105 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 2137 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10111 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 2141 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10117 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 2146 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 10123 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 2147 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 10129 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 10135 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 2151 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 10141 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 2152 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 10147 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2153 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 10153 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2154 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 10159 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2155 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 10165 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2156 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 10171 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2157 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 10177 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2158 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 10183 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2159 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 10189 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2160 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 10195 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2161 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 10201 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2162 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 10207 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2163 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 10213 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2164 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 10219 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2165 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 10225 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2166 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 10231 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2167 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 10237 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2168 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 10243 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2169 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 10249 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2170 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 10255 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2171 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 10261 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2172 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 10267 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2173 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 10273 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2174 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 10279 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2175 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 10285 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2176 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 10291 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2177 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 10297 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2178 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 10303 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2179 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 10309 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2180 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 10315 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2181 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 10321 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2182 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 10327 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2183 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 10333 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2184 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 10339 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2185 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 10345 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2186 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 10351 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2187 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 10357 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2188 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 10363 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2189 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 10369 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2190 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 10375 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2191 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 10381 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2192 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 10387 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2193 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 10393 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2194 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 10399 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2195 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 10406 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2197 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 10412 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2198 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 10419 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2200 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 10425 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2202 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 10431 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2203 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10437 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2204 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 10443 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2205 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 10449 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2206 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 10455 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2207 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10461 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 10467 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2209 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 10473 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2210 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 10479 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2211 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 10485 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2212 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 10491 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2213 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 10497 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2214 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 10503 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2215 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 10509 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2216 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 10515 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2217 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10521 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2218 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10527 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2219 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10533 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2220 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10539 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2221 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10545 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2222 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10551 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2223 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10557 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2224 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10563 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2225 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10569 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2226 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 10575 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2227 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 10581 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2228 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10587 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2235 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 10593 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2236 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10599 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2241 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10608 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2247 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10620 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2256 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10629 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2262 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10641 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2273 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,NULL,NULL);}
#line 10655 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2282 "hphp.y" /* yacc.c:1646  */
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
#line 10670 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2293 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10680 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2301 "hphp.y" /* yacc.c:1646  */
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
#line 10695 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2312 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10705 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2318 "hphp.y" /* yacc.c:1646  */
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
#line 10722 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2330 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,NULL,NULL);}
#line 10736 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2339 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10749 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2347 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10759 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2355 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10772 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2366 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10778 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2367 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10784 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2369 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10790 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2373 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10797 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2375 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10803 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2382 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10809 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2385 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10815 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2392 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10821 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2395 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10827 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2400 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10833 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2401 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10839 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2406 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10845 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2407 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10851 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2411 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval), (yyvsp[-1]));}
#line 10857 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2415 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10863 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2416 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10869 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2421 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10875 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2422 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10881 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2427 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10887 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2428 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10893 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2433 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10899 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2434 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10905 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2440 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10911 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2442 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10917 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2447 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10923 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2448 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10929 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2454 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10935 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2456 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10941 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2460 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10947 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2464 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10953 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2468 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10959 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2472 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10965 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2476 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10971 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2480 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10977 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2484 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10983 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2488 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10989 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2492 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10995 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2496 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11001 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2500 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11007 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2504 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11013 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2508 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11019 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2512 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11025 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2516 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11031 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2520 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11037 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2524 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11043 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2528 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11049 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2533 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11055 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2534 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11061 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2539 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11067 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2540 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11073 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2545 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11079 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2546 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11085 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2551 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11093 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2558 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11101 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2565 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11107 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2567 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11113 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2571 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11119 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2572 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11125 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11131 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2574 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11137 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2575 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11143 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2576 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11149 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2577 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11155 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2578 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11161 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2579 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11167 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2580 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 11174 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2582 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11180 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2583 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11186 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2587 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11192 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2588 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 11198 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2589 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 11204 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2590 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 11210 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2597 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 11216 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2600 "hphp.y" /* yacc.c:1646  */
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
#line 11234 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2615 "hphp.y" /* yacc.c:1646  */
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
#line 11252 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2630 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 11258 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2631 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 11264 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2634 "hphp.y" /* yacc.c:1646  */
    { _p->onXhpAttributesStart(); (yyval).reset();}
#line 11270 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { _p->onXhpAttributeSpread((yyval), &(yyvsp[-4]), (yyvsp[-1]));}
#line 11276 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11282 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { _p->onOptExprListElem((yyval), &(yyvsp[-1]), (yyvsp[0])); }
#line 11288 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2643 "hphp.y" /* yacc.c:1646  */
    {  (yyval).reset();}
#line 11294 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11301 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11309 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2653 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11315 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2656 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 11327 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11333 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11339 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11345 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 11351 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2672 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 11357 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2676 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11363 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2677 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11369 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2678 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11375 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2679 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11381 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2680 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11387 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2681 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11393 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2682 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11399 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2683 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11405 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2684 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11411 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2685 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11417 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11423 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2687 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11429 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2688 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11435 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2689 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11441 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2690 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11447 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2691 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11453 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11459 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2693 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11465 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2694 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11471 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2695 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11477 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2696 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11483 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2697 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11489 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2698 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11495 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2699 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11501 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2700 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11507 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2701 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11513 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2702 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11519 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2703 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11525 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2704 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11531 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2705 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11537 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2706 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11543 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2707 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11549 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2708 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11555 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2709 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11561 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2710 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11567 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2711 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11573 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11579 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2713 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11585 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2714 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11591 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2715 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11597 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2716 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11603 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2717 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11609 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11615 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11621 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11627 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11633 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2722 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11639 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11645 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2724 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11651 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11657 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2726 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11663 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2727 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11669 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2728 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11675 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11681 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11687 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2731 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11693 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2732 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11699 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2733 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11705 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11711 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2735 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11717 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11723 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2737 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11729 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11735 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11741 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11747 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11753 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2742 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11759 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2743 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11765 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2744 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11771 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2745 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11777 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2746 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11783 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2747 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11789 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2748 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11795 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2749 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11801 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2750 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11807 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2751 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11813 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2752 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11819 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2753 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11825 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2754 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11831 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2755 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11837 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2756 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11843 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2757 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11849 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2758 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11855 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2763 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11861 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2767 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11867 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2768 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11873 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2772 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11879 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2773 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11885 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2774 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11891 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2775 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11898 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2777 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11905 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2781 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11911 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2790 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11917 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2793 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11923 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2794 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11930 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2796 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11937 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2806 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11943 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2810 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11949 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2811 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11955 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2812 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11961 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2816 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11967 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2817 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11973 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2818 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11979 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2822 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11985 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2823 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11991 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2824 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11997 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12003 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2829 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12009 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2833 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12015 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2834 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12021 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2835 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12027 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2836 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 12034 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2838 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 12040 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2839 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 12046 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2840 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 12052 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2841 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 12058 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2842 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 12064 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2843 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 12070 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2844 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 12076 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2845 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 12082 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2846 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 12088 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12094 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2851 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12100 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2855 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12106 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2856 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12112 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2858 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12118 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2859 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12124 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2861 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1]));}
#line 12130 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2862 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12136 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2863 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12142 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2864 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12148 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2865 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12154 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2866 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12160 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12166 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2868 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12172 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2869 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12178 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2870 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12184 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2872 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 12190 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2874 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 12196 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2876 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 12202 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2878 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 12208 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2880 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 12214 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2881 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 12220 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2882 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 12226 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2883 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 12232 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2884 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 12238 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2885 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 12244 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2886 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 12250 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2887 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 12256 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2888 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 12262 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2889 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 12268 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2890 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 12274 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2891 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 12280 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2892 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 12286 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2893 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 12292 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2894 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 12298 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2895 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12304 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2896 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12310 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2898 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 12316 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2900 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 12322 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2902 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 12328 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2904 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 12334 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2905 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 12340 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2907 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 12347 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2909 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 12353 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2912 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 12360 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2916 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 12366 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2919 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 12372 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2920 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 12378 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2924 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12384 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2925 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12390 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2931 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12396 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2937 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12402 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2938 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12408 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2942 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12414 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2943 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12420 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2944 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12426 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2945 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12432 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2946 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 12438 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2947 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 12444 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2949 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 12451 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2954 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12457 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2955 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12463 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2959 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12469 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2960 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12475 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2963 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 12481 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2964 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12487 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2970 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12493 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2972 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12499 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2974 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12505 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2975 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12511 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2979 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12517 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2980 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12523 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2981 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12529 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2984 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12535 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2986 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12541 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2989 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 12547 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2990 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 12553 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2991 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 12559 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2992 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12565 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2996 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 12572 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2999 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 12580 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 3006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12586 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 3007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12592 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 3010 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 12600 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 3013 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12606 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 3014 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12612 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 3015 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12618 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12624 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 3018 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12630 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 3020 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1]));}
#line 12636 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 3021 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12642 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 3022 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12648 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 3023 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12654 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 3024 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12660 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 3025 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12666 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 3026 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12672 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 3031 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12678 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 3032 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12684 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 3037 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12690 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 3038 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12696 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 3043 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12702 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 3045 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12708 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3047 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12714 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3048 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12720 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3052 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12726 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3053 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12732 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3058 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12738 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3059 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12744 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3064 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12750 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3067 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12756 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3072 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12762 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3073 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12768 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3076 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12774 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3077 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 12781 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3084 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 12787 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3086 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 12793 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3089 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 12799 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3091 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12805 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3094 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12811 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3097 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12817 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3098 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12823 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3102 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12829 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3103 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12835 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3107 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12841 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3108 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12847 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3109 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12853 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3113 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12859 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3115 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12865 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3123 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12871 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3124 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12877 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3128 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12883 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3130 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12889 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3138 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12895 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3139 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12901 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3144 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12907 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3145 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12913 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3147 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12919 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3152 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12925 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3154 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12931 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3160 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12945 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3171 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12959 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3186 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12973 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3198 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12987 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3210 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12993 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3211 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12999 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3212 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13005 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3213 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13011 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3214 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13017 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3215 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13023 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3217 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13037 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3234 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13043 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3236 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13049 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3238 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13055 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3239 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13061 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3243 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13067 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3247 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13073 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3248 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13079 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3249 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13085 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3250 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13091 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3258 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13105 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3267 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13111 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3269 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13117 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3278 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13123 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3279 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13129 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3280 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13135 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3281 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13141 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3282 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13147 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3283 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13153 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3284 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13159 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3286 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13165 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3287 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13171 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3290 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13177 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3292 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13183 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3296 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13189 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3300 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13195 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3301 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13201 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3307 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13207 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3311 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13213 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3315 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13219 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3322 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 13225 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3331 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 13231 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3335 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 13237 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3339 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13243 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3342 "hphp.y" /* yacc.c:1646  */
    { _p->onIndirectRef((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 13249 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3348 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13255 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3349 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13261 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3350 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13267 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3354 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13273 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3355 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 13279 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3356 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 13285 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3363 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13291 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3364 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13297 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3369 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 13303 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3370 "hphp.y" /* yacc.c:1646  */
    { (yyval)++;}
#line 13309 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3375 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13315 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3376 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13321 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3377 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13327 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3380 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13341 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3391 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13347 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3392 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13353 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3396 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13359 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3397 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13365 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3400 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13379 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3409 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13385 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3413 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 13391 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3414 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 13397 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3416 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 13403 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3417 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 13409 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3418 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 13415 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3419 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 13421 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3424 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13427 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3425 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13433 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3429 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13439 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3430 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13445 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3431 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13451 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3432 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13457 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3435 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13463 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3437 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 13469 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3438 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13475 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3439 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 13481 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3444 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13487 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3445 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13493 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3449 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13499 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3450 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13505 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3451 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13511 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3452 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13517 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3457 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13523 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3458 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13529 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3463 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13535 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3465 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13541 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3467 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13547 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1030:
#line 3468 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13553 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1031:
#line 3472 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 13559 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1032:
#line 3474 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 13565 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1033:
#line 3475 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 13571 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1034:
#line 3477 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 13578 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1035:
#line 3482 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13584 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1036:
#line 3484 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13590 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3486 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 13604 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1038:
#line 3496 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 13610 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1039:
#line 3498 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 13616 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3499 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13622 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 13628 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3503 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 13634 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3504 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 13640 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3508 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 13646 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3509 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 13652 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3510 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13658 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3511 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13664 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3512 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13670 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3513 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13676 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3514 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 13682 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3515 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 13688 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3516 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 13694 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3517 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 13700 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3518 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 13706 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3522 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 13712 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3523 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 13718 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3528 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13724 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3530 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13730 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3544 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13738 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3549 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 13746 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3553 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13754 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3558 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 13762 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3564 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13768 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3565 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13774 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3569 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13780 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3570 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13786 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3576 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13792 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3580 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 13798 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3586 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13804 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3590 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 13811 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3597 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13817 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3598 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13823 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3602 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 13831 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3605 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 13838 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1077:
#line 3611 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13844 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1078:
#line 3615 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13852 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1079:
#line 3618 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13860 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1080:
#line 3621 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-3]); }
#line 13867 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1081:
#line 3623 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13874 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1082:
#line 3625 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13881 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1083:
#line 3627 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13887 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1084:
#line 3632 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]); }
#line 13893 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1085:
#line 3633 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13899 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1086:
#line 3634 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13905 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1087:
#line 3635 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13911 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1094:
#line 3656 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13917 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1095:
#line 3657 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13923 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1098:
#line 3666 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13929 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1101:
#line 3677 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13935 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1102:
#line 3679 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13941 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1103:
#line 3683 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13947 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1104:
#line 3686 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13953 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1105:
#line 3690 "hphp.y" /* yacc.c:1646  */
    {}
#line 13959 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1106:
#line 3691 "hphp.y" /* yacc.c:1646  */
    {}
#line 13965 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1107:
#line 3692 "hphp.y" /* yacc.c:1646  */
    {}
#line 13971 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1108:
#line 3698 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13978 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1109:
#line 3703 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13988 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1110:
#line 3712 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13994 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1111:
#line 3718 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 14003 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1112:
#line 3726 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 14009 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1113:
#line 3727 "hphp.y" /* yacc.c:1646  */
    { }
#line 14015 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1114:
#line 3733 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 14021 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1115:
#line 3735 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 14027 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1116:
#line 3736 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 14037 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1117:
#line 3741 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 14044 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1118:
#line 3747 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("HH\\darray"); }
#line 14051 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1119:
#line 3752 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14057 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1120:
#line 3757 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 14065 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1121:
#line 3761 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14071 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1122:
#line 3766 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 14077 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1123:
#line 3768 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 14083 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1124:
#line 3774 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 14090 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1125:
#line 3776 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 14098 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1126:
#line 3779 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14104 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1127:
#line 3780 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14112 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1128:
#line 3783 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14120 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1129:
#line 3786 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14126 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1130:
#line 3789 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 14134 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1131:
#line 3792 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14141 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1132:
#line 3794 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 14150 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1133:
#line 3800 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 14159 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1134:
#line 3806 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("HH\\varray");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 14169 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1135:
#line 3814 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14175 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1136:
#line 3815 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 14181 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;


#line 14185 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 3818 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}
