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

#line 662 "hphp.7.tab.cpp" /* yacc.c:339  */

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



int Compiler7parse (HPHP::HPHP_PARSER_NS::Parser *_p);

#endif /* !YY_YY_HPHP_Y_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 906 "hphp.7.tab.cpp" /* yacc.c:358  */

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
#define YYLAST   20435

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  209
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  315
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1136
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  2124

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
    2028,  2031,  2033,  2035,  2036,  2039,  2041,  2044,  2050,  2052,
    2056,  2060,  2064,  2069,  2073,  2074,  2076,  2077,  2078,  2079,
    2082,  2083,  2087,  2088,  2092,  2093,  2094,  2095,  2099,  2103,
    2108,  2112,  2116,  2120,  2124,  2129,  2133,  2134,  2135,  2136,
    2137,  2141,  2145,  2147,  2148,  2149,  2152,  2153,  2154,  2155,
    2156,  2157,  2158,  2159,  2160,  2161,  2162,  2163,  2164,  2165,
    2166,  2167,  2168,  2169,  2170,  2171,  2172,  2173,  2174,  2175,
    2176,  2177,  2178,  2179,  2180,  2181,  2182,  2183,  2184,  2185,
    2186,  2187,  2188,  2189,  2190,  2191,  2192,  2193,  2194,  2195,
    2197,  2198,  2200,  2201,  2203,  2204,  2205,  2206,  2207,  2208,
    2209,  2210,  2211,  2212,  2213,  2214,  2215,  2216,  2217,  2218,
    2219,  2220,  2221,  2222,  2223,  2224,  2225,  2226,  2227,  2228,
    2232,  2236,  2241,  2240,  2256,  2254,  2273,  2272,  2293,  2292,
    2312,  2311,  2330,  2330,  2347,  2347,  2366,  2367,  2368,  2373,
    2375,  2379,  2383,  2389,  2393,  2399,  2401,  2405,  2407,  2411,
    2415,  2416,  2420,  2422,  2426,  2428,  2432,  2434,  2438,  2441,
    2446,  2448,  2452,  2455,  2460,  2464,  2468,  2472,  2476,  2480,
    2484,  2488,  2492,  2496,  2500,  2504,  2508,  2512,  2516,  2520,
    2524,  2528,  2532,  2534,  2538,  2540,  2544,  2546,  2550,  2557,
    2564,  2566,  2571,  2572,  2573,  2574,  2575,  2576,  2577,  2578,
    2579,  2580,  2582,  2583,  2587,  2588,  2589,  2590,  2594,  2600,
    2613,  2630,  2631,  2634,  2635,  2637,  2642,  2643,  2646,  2650,
    2653,  2656,  2663,  2664,  2668,  2669,  2671,  2676,  2677,  2678,
    2679,  2680,  2681,  2682,  2683,  2684,  2685,  2686,  2687,  2688,
    2689,  2690,  2691,  2692,  2693,  2694,  2695,  2696,  2697,  2698,
    2699,  2700,  2701,  2702,  2703,  2704,  2705,  2706,  2707,  2708,
    2709,  2710,  2711,  2712,  2713,  2714,  2715,  2716,  2717,  2718,
    2719,  2720,  2721,  2722,  2723,  2724,  2725,  2726,  2727,  2728,
    2729,  2730,  2731,  2732,  2733,  2734,  2735,  2736,  2737,  2738,
    2739,  2740,  2741,  2742,  2743,  2744,  2745,  2746,  2747,  2748,
    2749,  2750,  2751,  2752,  2753,  2754,  2755,  2756,  2757,  2758,
    2762,  2767,  2768,  2772,  2773,  2774,  2775,  2777,  2781,  2782,
    2793,  2794,  2796,  2798,  2810,  2811,  2812,  2816,  2817,  2818,
    2822,  2823,  2824,  2827,  2829,  2833,  2834,  2835,  2836,  2838,
    2839,  2840,  2841,  2842,  2843,  2844,  2845,  2846,  2847,  2850,
    2855,  2856,  2857,  2859,  2860,  2862,  2863,  2864,  2865,  2866,
    2867,  2868,  2869,  2870,  2871,  2873,  2875,  2877,  2879,  2881,
    2882,  2883,  2884,  2885,  2886,  2887,  2888,  2889,  2890,  2891,
    2892,  2893,  2894,  2895,  2896,  2897,  2899,  2901,  2903,  2905,
    2906,  2909,  2910,  2914,  2918,  2920,  2924,  2925,  2929,  2935,
    2938,  2942,  2943,  2944,  2945,  2946,  2947,  2948,  2953,  2955,
    2959,  2960,  2963,  2964,  2968,  2971,  2973,  2975,  2979,  2980,
    2981,  2982,  2985,  2989,  2990,  2991,  2992,  2996,  2998,  3005,
    3006,  3007,  3008,  3013,  3014,  3015,  3016,  3018,  3019,  3021,
    3022,  3023,  3024,  3025,  3026,  3030,  3032,  3036,  3038,  3041,
    3044,  3046,  3048,  3051,  3053,  3057,  3059,  3062,  3065,  3071,
    3073,  3076,  3077,  3082,  3085,  3089,  3089,  3094,  3097,  3098,
    3102,  3103,  3107,  3108,  3109,  3113,  3118,  3123,  3124,  3128,
    3133,  3138,  3139,  3143,  3145,  3146,  3151,  3153,  3158,  3169,
    3183,  3195,  3210,  3211,  3212,  3213,  3214,  3215,  3216,  3226,
    3235,  3237,  3239,  3243,  3247,  3248,  3249,  3250,  3251,  3267,
    3268,  3271,  3278,  3279,  3280,  3281,  3282,  3283,  3284,  3286,
    3287,  3289,  3291,  3296,  3300,  3301,  3305,  3308,  3312,  3319,
    3323,  3332,  3339,  3347,  3349,  3350,  3354,  3355,  3356,  3358,
    3363,  3364,  3375,  3376,  3377,  3378,  3389,  3392,  3395,  3396,
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
  "dim_offset", "variable_no_calls", "dimmable_variable_no_calls",
  "assignment_list", "array_pair_list", "non_empty_array_pair_list",
  "collection_init", "non_empty_collection_init", "static_collection_init",
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

#define YYPACT_NINF -1833

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1833)))

#define YYTABLE_NINF -1137

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1137)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1833,   221, -1833, -1833,  5792, 15293, 15293,    19, 15293, 15293,
   15293, 15293, 12894, 15293, -1833, 15293, 15293, 15293, 15293, 18512,
   18512, 15293, 15293, 15293, 15293, 15293, 15293, 15293, 15293, 13073,
   19247, 15293,   173,   223, -1833, -1833, -1833,   178, -1833,   333,
   -1833, -1833, -1833,   181, 15293, -1833,   223,   238,   261,   265,
   -1833,   223, 13252, 16417, 13431, -1833, 16157, 11757,   303, 15293,
   19496,   104,   106,   334,   462, -1833, -1833, -1833,   330,   359,
     398,   407, -1833, 16417,   429,   432,   583,   590,   606,   622,
     626, -1833, -1833, -1833, -1833, -1833, 15293,   604,  3055, -1833,
   -1833, 16417, -1833, -1833, -1833, -1833, 16417, -1833, 16417, -1833,
     533,   504,   513, 16417, 16417, -1833,   448, -1833, -1833, 13637,
   -1833, -1833,   325,   642,   764,   764, -1833,   686,   556,   536,
     530, -1833,   122, -1833,   544,   629,   729, -1833, -1833, -1833,
   -1833,  1189,   698, -1833,   174, -1833,   561,   574,   580,   587,
     595,   603,   624,   645, 17572, -1833, -1833, -1833, -1833, -1833,
     182,   714,   779,   782,   787,   803,   805, -1833,   813,   815,
   -1833,   236,   589, -1833,   726,   252, -1833,  2238,   243, -1833,
   -1833,  4504,   174,   174,   692,   363, -1833,   248,    92,   694,
     172, -1833, -1833,   832, -1833,   742, -1833, -1833,   707,   744,
   -1833, 15293, -1833,   729,   698, 19784,  4712, 19784, 15293, 19784,
   19784, 16706, 16706,   712, 17982, 19784,   859, 16417,   854,   854,
     153,   854, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833,   101, 15293,   749, -1833, -1833,   772,   765,    70,   768,
      70,   854,   854,   854,   854,   854,   854,   854,   854, 18512,
   18685,   737,   954,   742, -1833, 15293,   749, -1833,   806, -1833,
     810,   778, -1833,   180, -1833, -1833, -1833,    70,   174, -1833,
   13816, -1833, -1833, 15293, 10324,   972,   124, 19784, 11354, -1833,
   15293, 15293, 16417, -1833, -1833, 17620,   781, -1833, 17690, -1833,
   -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,  4256,
   -1833,  4256, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833,   103,   113,   744, -1833, -1833, -1833, -1833,   788, -1833,
    4476,   116, -1833, -1833,   824,   976, -1833,   834, 16898, 15293,
   -1833,   798,   800, 17741, -1833,    57, 17789,  3596,  3596,  3596,
   16417,  3596,   802,   997,   807, -1833,    73, -1833, 18179,   125,
   -1833,   995,   136,   881, -1833,   886, -1833, 18512, 15293, 15293,
     819,   839, -1833, -1833, 18255, 13073, 15293, 15293, 15293, 15293,
   15293,   137,   475,    69, -1833, 15472, 18512,   677, -1833, 16417,
   -1833,   540,   556, -1833, -1833, -1833, -1833, 19349, 15293,  1010,
     922, -1833, -1833, -1833,    87, 15293,   828,   829, 19784,   831,
    2550,   833,  6410, 15293, -1833,   632,   830,   775,   632,   541,
     491, -1833, 16417,  4256,   835, 11936, 16157, -1833, 14022,   852,
     852,   852,   852, -1833, -1833,  2768, -1833, -1833, -1833, -1833,
   -1833,   729, -1833, 15293, 15293, -1833, -1833, -1833, -1833, -1833,
   -1833, -1833, -1833, -1833, 15293, 15293, 15293, 15293, 14201, 15293,
   15293, 15293, 15293, 15293, 15293, 15293, 15293, 15293, 15293, 15293,
   15293, 15293, 15293, 15293, 15293, 15293, 15293, 15293, 15293, 15293,
   15293, 15293, 19425, 15293, -1833, 15293, 15293, 15293,  5384, 16417,
   16417, 16417, 16417, 16417,  1189,   925,   795, 11560, 15293, 15293,
   15293, 15293, 15293, 15293, 15293, 15293, 15293, 15293, 15293, 15293,
   -1833, -1833, -1833, -1833,  2600, -1833, -1833, 11936, 11936, 15293,
   15293, 18255,   856,   729, 14380,  5700, -1833, 15293, -1833,   862,
    1049,   903,   863,   866, 15645,    70, 14559, -1833, 14738, -1833,
     778,   869,   873,  3105, -1833,   384, 11936, -1833,  2616, -1833,
   -1833, 17859, -1833, -1833, 12312, -1833, 15293, -1833,   973, 10530,
    1065,   879, 19662,  1068,   110,    88, -1833, -1833, -1833,   913,
   -1833, -1833, -1833,  4256, -1833,   613,   901,  1093, 18103, 16417,
   -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,   911,
   -1833, -1833,   909,   912,   917,   914,   921,   926,   109,   932,
     929,  1770, 16408, -1833, -1833, 16417, 16417, 15293,    70,   104,
   -1833, 18103,  1052, -1833, -1833, -1833,    70,   115,   119,   938,
     939,  3188,   218,   941,   945,   722,  1013,   949,    70,   135,
     950, 18733,   952,  1144,  1145,   953,   955,   957,   960, -1833,
    1387, 16417, -1833, -1833,  1088,  3328,    67, -1833, -1833, -1833,
     556, -1833, -1833, -1833,  1133,  1031,   985,    82,  1009, 15293,
    1034,  1165,   975, -1833,  1016, -1833,   186, -1833,   983,  4256,
    4256,  1170,   972,    87, -1833,   998,  1184, -1833,  4635,   242,
   -1833,   439,   263, -1833, -1833, -1833, -1833, -1833, -1833, -1833,
    1635,  4338, -1833, -1833, -1833, -1833,  1187,  1019, -1833, 18512,
     503, 15293,  1000,  1196, 19784,  1193,   149,  1202,  1012,  1015,
    1018, 19784,  1020,  3384,  6616, -1833, -1833, -1833, -1833, -1833,
   -1833,  1079,  5057, 19784,  1022,  4969, 19923, 20014, 16706, 20095,
   15293, 19736, 20168,  5077, 20238, 20271, 15979, 19432, 20335, 20335,
   20335, 20335,  2926,  2926,  2926,  2926,  2926,  1135,  1135,   825,
     825,   825,   153,   153,   153, -1833,   854,  1024,  1025, 18794,
    1021,  1210,    14, 15293,    33,   749,   486, -1833, -1833, -1833,
    1208,   922, -1833,   729, 18360, -1833, -1833, -1833, 16706, 16706,
   16706, 16706, 16706, 16706, 16706, 16706, 16706, 16706, 16706, 16706,
   16706, -1833, 15293,   364, -1833,   199, -1833,   749,   365,  1032,
    1035,  1027,  5140,   156,  1037, -1833, 19784,  3970, -1833, 16417,
   -1833,    70,    54, 18512, 19784, 18512, 18842,  1079,    83,    70,
     204, -1833,   186,  1072,  1038, 15293, -1833,   225, -1833, -1833,
   -1833,  6822,   724, -1833, -1833, 19784, 19784,   223, -1833, -1833,
   -1833, 15293,  1136, 18027, 18103, 16417, 10736,  1039,  1040, -1833,
    1233, 16050,  1105, -1833,  1082, -1833,  1236,  1046,  3819,  4256,
   18103, 18103, 18103, 18103, 18103,  1048,  1179,  1180,  1181,  1186,
    1188,  1055,  1058, 18103,    32, -1833, -1833, -1833, -1833, -1833,
   -1833, -1833,    36, -1833, 19878, -1833, -1833,    43, -1833,  7028,
   16229,  1057, 16408, -1833, 16408, -1833, 16408, -1833, 16417, 16417,
   16408, -1833, 16408, 16408, 16417, -1833,  1253,  1060, -1833,   433,
   -1833, -1833,  5237, -1833, 19878,  1252, 18512,  1074, -1833, -1833,
   -1833, -1833, -1833, -1833, -1833, -1833, -1833,  1085,  1264, 16417,
   16229,  1078, 18255, 18436,  1268, -1833, 15293, -1833, 15293, -1833,
   15293, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,  1087,
   -1833, 15293, -1833, -1833,  5998, -1833,  4256, 16229,  1092, -1833,
   -1833, -1833, -1833,  1119,  1283,  1100, 15293, 19349, -1833, -1833,
    5384, -1833,  1097, -1833,  4256, -1833,  1107,  7234,  1269,    81,
   -1833,  4256, -1833,   193,  2600,  2600, -1833,  4256, -1833, -1833,
      70, -1833, -1833,  1234, 19784, -1833, 12115, -1833, 18103, 14022,
     852, 14022, -1833,   852,   852, -1833, 12518, -1833, -1833,  7440,
   -1833,   140,  1108, 16229,  1031, -1833, -1833, -1833, -1833, 20168,
   15293, -1833, -1833, 15293, -1833, 15293, -1833,  5329,  1109, 11936,
    1013,  1274,  1031,  4256,  1295,  1079, 16417, 19425,    70,  5557,
    1111,   353,  1112, -1833, -1833,  1298,  5063,  5063,  3970, -1833,
   -1833, -1833,  1265,  1117,  1248,  1250,  1254,  1255,  1256,   108,
    1128,  1130,   451, -1833, -1833, -1833, -1833, -1833, -1833,  1173,
   -1833, -1833, -1833, -1833,  1334,  1146,   862,    70,    70, 14917,
    1031,  2616, -1833,  2616, -1833,  5628,   738,   223, 11354, -1833,
    7646,  1147,  7852,  1148, 18027, 18512,  1151,  1207,    70, 19878,
    1333, -1833, -1833, -1833, -1833,   838, -1833,    93,  4256,  1168,
    1215,  1195,  4256, 16417,  3583, -1833, -1833,  4256,  1351,  1167,
    1192,  1194,  1187,   763,   763,  1296,  1296, 19011,  1171,  1360,
   18103, 18103, 18103, 18103, 18103, 18103, 19349, 18103,  2849, 17052,
   18103, 18103, 18103, 18103, 17905, 18103, 18103, 18103, 18103, 18103,
   18103, 18103, 18103, 18103, 18103, 18103, 18103, 18103, 18103, 18103,
   18103, 18103, 18103, 18103, 18103, 18103, 18103, 18103, 16417, -1833,
   -1833,  1289, -1833, -1833,  1174,  1175,  1178, -1833,  1182, -1833,
   -1833,   470,  1770, -1833,  1185, -1833, 18103,    70, -1833, -1833,
     145, -1833,   121,  1365, -1833, -1833,   162,  1177,    70, 12715,
   19784, 18903, -1833,  2623, -1833,  6204,   922,  1365, -1833,   568,
   15293,   219, -1833, 19784,  1244,  1205, -1833,  1206,  1269, -1833,
   -1833, -1833, 15114,  4256,   972, 17780,  1300,    97,  1392,  1326,
     232, -1833,   749,   366, -1833,   749, -1833, 15293, 18512,   503,
   15293, 19784, 19878, -1833, -1833, -1833,  5080, -1833, -1833, -1833,
   -1833, -1833, -1833,  1211,   140, -1833,  1216,   140,  1214, 20168,
   19784, 18951,  1218, 11936,  1219,  1223,  4256,  1224,  1209,  4256,
    1031, -1833,   778,   421, 11936, 15293, -1833, -1833, -1833, -1833,
   -1833, -1833,  1290,  1217,  1418,  1337,  3970,  3970,  3970,  3970,
    3970,  3970,  1272, -1833, 19349,  3970,   498,  3970, -1833, -1833,
   -1833, 18512, 19784,  1230, -1833,   223,  1401,  1357, 11354, -1833,
   -1833, -1833,  1237, 15293,  1207,    70, 18255, 18027,  1235, 18103,
    8058,   868,  1239, 15293,   100,   488, -1833,  1257, -1833,  4256,
   16417, -1833,  1307, -1833, -1833, -1833,  4405, -1833,  1416, -1833,
    1258, 18103, -1833, 18103, -1833,  1259,  1246,  1443, 19057,  1249,
   19878,  1453,  1260,  1261,  1262,  1331,  1462,  1275,  1277, -1833,
   -1833, -1833, 19117,  1276,  1469, 19970, 20058, 20131, 18103, 19832,
   15646,  5739, 20304, 18444,  4861, 20366, 20366, 20366, 20366,  1346,
    1346,  1346,  1346,  1346,   846,   846,   763,   763,   763,  1296,
    1296,  1296,  1296, -1833,  1280, -1833,  1281,  1288,  1292,  1293,
   -1833, -1833, 19878, 16417,  4256,  4256, -1833,   121, 16229,  1621,
   -1833, 18255, -1833, -1833, 16706, 15293,  1278, -1833,  1294,  1850,
   -1833,   130, 15293, -1833, -1833, 17377, -1833, 15293, -1833, 15293,
   -1833,   972, 14022,  1297,   378,   852,   378,   379, -1833, -1833,
    4256,   195, -1833,  1476,  1412, 15293, -1833,  1301,  1302,  1299,
      70,  1234, 19784,  1269,  1306, -1833,  1308,   140, 15293, 11936,
    1309, -1833, -1833,   922, -1833, -1833,  1311,  1314,  1303, -1833,
    1310,  3970, -1833,  3970, -1833, -1833,  1313,  1315,  1488,  1361,
    1316, -1833,  1508,  1317,  1319,  1322, -1833,  1380,  1336,  1524,
    1339, -1833, -1833,    70, -1833,  1505, -1833,  1342, -1833, -1833,
    1340,  1349,   165, -1833, -1833, 19878,  1350,  1362, -1833, 17521,
   -1833, -1833, -1833, -1833, -1833, -1833,  1420,  4256,  4256,  1192,
    1374,  4256, -1833, 19878, 19163, -1833, -1833, 18103, -1833, 18103,
   -1833, 18103, -1833, -1833, -1833, -1833, 18103, 19349, -1833, -1833,
   -1833, 18103, -1833, 18103, -1833, 20204, 18103,  1363,  8264, -1833,
   -1833, -1833, -1833,   121, -1833, -1833, -1833, -1833,   693, 16336,
   16229,  1449,  1451,  1454,  1455, -1833,  4042,  1398, 15871, -1833,
   -1833,  1495,   925,  3496,  1466,   138,   141,  1379,   922,   795,
     168, 19784, -1833, -1833, -1833,  1414, 17425, -1833, 17473, 19784,
   -1833,  3536, -1833,  6616,  1499,   287,  1570,  1504, 15293, -1833,
   19784, 11936, 11936, -1833,  1470,  1269,  1870,  1269,  1388, 19784,
    1390, -1833,  1975,  1389,  2149, -1833, -1833,   140, -1833, -1833,
    1456, -1833, -1833,  3970, -1833,  3970, -1833,  3970, -1833, -1833,
   -1833, -1833,  3970, -1833, 19349, -1833, -1833,  2278, -1833,  8470,
   -1833, -1833, -1833, -1833, 10942, -1833, -1833, -1833,  6822,  4256,
   -1833, -1833, -1833,  1395, 18103, 19223, 19878, 19878, 19878,  1458,
   19878, 19269, 20204, -1833, -1833,   121, 16229, 16229, 16417, -1833,
    1578, 17206,    98, -1833, 16336,   922, 16791, -1833,  1415, -1833,
     142,  1397,   150, -1833, 16705, -1833, -1833, -1833,   151, -1833,
   -1833,  1403, -1833,  1399, -1833,  1592,   152,   729,  1495, 16526,
   16526, -1833, 16526, -1833, -1833,  1593,   925,  4123, -1833, 15799,
   -1833, -1833, -1833, -1833,  2978, -1833,  1595,  1527, 15293, -1833,
   19784,  1411,  1419,  1413,  1269,  1417, -1833,  1470,  1269, -1833,
   -1833, -1833, -1833,  2339,  1423,  3970,  1480, -1833, -1833, -1833,
    1486, -1833,  6822, 11148, 10942, -1833, -1833, -1833,  6822, -1833,
   -1833, 19878, 18103, 18103, 18103,  8676,  1425,  1427, -1833, 18103,
   -1833, 16229, -1833, -1833, -1833, -1833, -1833,  4256,  1542,  4042,
   -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833, -1833, -1833, -1833,   699, -1833,  1398, -1833, -1833, -1833,
   -1833, -1833,   131,   636, -1833, 18103,  1531, -1833, 16898,   154,
    1617,  1618, -1833,  4256,   729,   157,  1495, -1833, -1833,  1432,
    1622, 15293, -1833, 19784, -1833, -1833,   189,  1433,  4256,   728,
    1269,  1417, 15978, -1833,  1269, -1833,  3970,  3970, -1833, -1833,
   -1833, -1833,  8882, 19878, 19878, 19878, -1833, -1833, -1833, 19878,
   -1833,  3752,  1628,  1634,  1445, -1833, -1833, 18103, 16705, 16705,
    1582, -1833,  1403,  1403,   721, -1833, -1833, -1833, 19878,  1632,
    1468,  1459, -1833, 18103, 18103, -1833, 16898, -1833,   159, -1833,
   18103, 19784,  1569, -1833,  1644, -1833,  1646, -1833,   512, -1833,
   -1833, -1833,  1457,   728, -1833,   728, -1833, -1833,  9088,  1460,
    1541, -1833,  1560,  1506, -1833, -1833,  1563,  4256,  1483,  1542,
   -1833, -1833, 19878, -1833, -1833,  1497, -1833,  1638, -1833, -1833,
   -1833, -1833, 18103,   722, -1833, 19878, 19878,  1477, -1833, 19878,
   -1833,   224,  1475,  9294,  4256, -1833,  4256, -1833,  9500, -1833,
   -1833, -1833,  1479, -1833,  1478,  1498, 16417,   795,  1500, -1833,
   -1833, -1833, 19878,  1502,   163, -1833,  1598, -1833, -1833, -1833,
   -1833, -1833, -1833,  9706, -1833, 16229,  1057, -1833,  1511, 16417,
     739, -1833, -1833,  1482,  1674,   521,   163, -1833, -1833,  1614,
   -1833, 16229,  1503, -1833,  1269,   166, -1833,  4256, -1833, -1833,
   -1833,  4256, -1833,  1501,  1510,   160, -1833,  1417,   555,  1615,
     197,  1269,  1509, -1833,   736,  4256,  4256, -1833,   427,  1685,
    1625,  1417, -1833, -1833, -1833, -1833,  1629,   209,  1691,  1630,
   15293, -1833,   736,  9912, 10118, -1833,   442,  1700,  1633, 15293,
   -1833, 19784, -1833, -1833, -1833,  1704,  1636, 15293, -1833, 19784,
   15293, -1833, 19784, 19784
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   204,   474,     0,   915,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1010,
     998,     0,   779,     0,   785,   786,   787,    29,   852,   986,
     987,   171,   172,   788,     0,   152,     0,     0,     0,     0,
      30,     0,     0,     0,     0,   223,     0,     0,     0,     0,
       0,     0,   442,   443,   444,   441,   440,   439,     0,     0,
       0,     0,   252,     0,     0,     0,    37,    38,    40,    41,
      39,   792,   794,   795,   789,   790,     0,     0,     0,   796,
     791,     0,   762,    32,    33,    34,    36,    35,     0,   793,
       0,     0,     0,     0,     0,   797,   445,   584,    31,     0,
     170,   140,     0,   780,     0,     0,     4,   126,   128,   851,
       0,   761,     0,     6,     0,     0,   222,     7,     9,     8,
      10,     0,     0,   437,   969,   488,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   544,   486,   974,   975,   566,
     559,   560,   561,   562,   565,   563,   564,   469,   569,     0,
     468,   943,   763,   770,     0,   854,   558,   436,   946,   947,
     959,   487,     0,     0,     0,   490,   489,   944,   945,   942,
     982,   985,   548,   853,    11,   442,   443,   444,     0,     0,
      36,     0,   126,   222,     0,  1050,   487,  1051,     0,  1053,
    1054,   568,   482,     0,   475,   480,     0,     0,   530,   531,
     532,   533,    29,   986,   788,   765,    37,    38,    40,    41,
      39,     0,     0,  1074,   965,   763,     0,   764,   509,     0,
     511,   549,   550,   551,   552,   553,   554,   555,   557,     0,
    1014,     0,   861,   775,   242,     0,  1074,   466,   774,   768,
       0,   784,   764,   993,   994,  1000,   992,   776,     0,   467,
       0,   778,   556,     0,   205,     0,     0,   471,   205,   150,
     473,     0,     0,   156,   158,     0,     0,   160,     0,    75,
      76,    82,    83,    67,    68,    59,    80,    91,    92,     0,
      62,     0,    66,    74,    72,    94,    86,    85,    57,   108,
      81,   101,   102,    58,    97,    55,    98,    56,    99,    54,
     103,    90,    95,   100,    87,    88,    61,    89,    93,    53,
      84,    69,   104,    77,   106,    70,    60,    47,    48,    49,
      50,    51,    52,    71,   107,   105,   110,    64,    45,    46,
      73,  1127,  1128,    65,  1132,    44,    63,    96,     0,    79,
       0,   126,   109,  1065,  1126,     0,  1129,     0,     0,     0,
     162,     0,     0,     0,   213,     0,     0,     0,     0,     0,
       0,     0,     0,   863,     0,   114,   116,   350,     0,     0,
     349,   355,     0,     0,   253,     0,   256,     0,     0,     0,
       0,  1071,   238,   250,  1006,  1010,   603,   633,   633,   603,
     633,     0,  1035,     0,   799,     0,     0,     0,  1033,     0,
      16,     0,   130,   230,   244,   251,   663,   596,   633,     0,
    1059,   576,   578,   580,   919,   474,   488,     0,     0,   486,
     487,   489,   205,     0,   989,   781,     0,   782,     0,     0,
       0,   202,     0,     0,   132,   339,     0,    28,     0,     0,
       0,     0,     0,   203,   221,     0,   249,   234,   248,   442,
     445,   222,   438,   991,     0,   935,   192,   193,   194,   195,
     196,   198,   199,   201,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   998,     0,   191,   991,   991,  1020,     0,     0,
       0,     0,     0,     0,     0,     0,   435,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     508,   510,   920,   921,     0,   934,   933,   339,   339,   991,
       0,  1006,     0,   222,     0,     0,   164,     0,   917,   912,
     861,     0,   488,   486,     0,  1018,     0,   601,   860,  1009,
     784,   488,   486,   487,   132,     0,   339,   465,     0,   936,
     777,     0,   140,   292,     0,   583,     0,   167,     0,   205,
     472,     0,     0,     0,     0,     0,   159,   190,   161,  1127,
    1128,  1124,  1125,     0,  1131,  1117,     0,     0,     0,     0,
      78,    43,    65,    42,  1066,   197,   200,   163,   140,     0,
     180,   189,     0,     0,     0,     0,     0,     0,   117,     0,
       0,     0,   862,   115,    18,     0,   111,     0,   351,     0,
     165,     0,     0,   166,   254,   255,  1055,     0,     0,   488,
     486,   487,   490,   489,     0,  1107,   262,     0,  1007,     0,
       0,     0,     0,   861,   861,     0,     0,     0,     0,   168,
       0,     0,   798,  1034,   852,     0,     0,  1032,   857,  1031,
     129,     5,    13,    14,     0,   260,     0,     0,   589,     0,
       0,   861,     0,   772,     0,   771,   766,   590,     0,     0,
       0,     0,     0,   919,   136,     0,   863,   918,  1136,   464,
     477,   491,   952,   973,   147,   139,   143,   144,   145,   146,
     436,     0,   567,   855,   856,   127,   861,     0,  1075,     0,
       0,     0,     0,   863,   340,     0,     0,     0,   488,   209,
     210,   208,   486,   487,   205,   184,   182,   183,   185,   572,
     224,   258,     0,   990,     0,     0,   514,   516,   515,   527,
       0,     0,   547,   512,   513,   517,   519,   518,   536,   537,
     534,   535,   538,   539,   540,   541,   542,   528,   529,   521,
     522,   520,   523,   524,   526,   543,   525,     0,     0,  1024,
       0,   861,  1058,     0,  1057,  1074,   949,   240,   232,   246,
       0,  1059,   236,   222,     0,   478,   481,   483,   493,   496,
     497,   498,   499,   500,   501,   502,   503,   504,   505,   506,
     507,   923,     0,   922,   925,   948,   929,  1074,   926,     0,
       0,     0,     0,     0,     0,  1052,   476,   910,   914,   860,
     916,   463,   767,     0,  1013,     0,  1012,   258,     0,   767,
     997,   996,   982,   985,     0,     0,   922,   925,   995,   926,
     485,   294,   296,   136,   587,   586,   470,     0,   140,   276,
     151,   473,     0,     0,     0,     0,   205,   288,   288,   157,
     861,     0,     0,  1116,     0,  1113,   861,     0,  1087,     0,
       0,     0,     0,     0,   859,     0,    37,    38,    40,    41,
      39,     0,     0,     0,   801,   805,   806,   807,   810,   808,
     809,   812,     0,   800,   134,   850,   811,  1074,  1130,   205,
       0,     0,     0,    21,     0,    22,     0,    19,     0,   112,
       0,    20,     0,     0,     0,   123,   863,     0,   121,   116,
     113,   118,     0,   348,   356,   353,     0,     0,  1044,  1049,
    1046,  1045,  1048,  1047,    12,  1105,  1106,     0,   861,     0,
       0,     0,  1006,  1003,     0,   600,     0,   614,   860,   602,
     860,   632,   617,   626,   629,   620,  1043,  1042,  1041,     0,
    1037,     0,  1038,  1040,   205,     5,     0,     0,     0,   658,
     659,   668,   667,     0,     0,   486,     0,   860,   595,   599,
       0,   623,     0,  1060,     0,   577,     0,   205,  1094,   919,
     320,  1136,  1135,     0,     0,     0,   988,   860,  1077,  1073,
     342,   336,   337,   341,   343,   760,   862,   338,     0,     0,
       0,     0,   463,     0,     0,   491,     0,   953,   212,   205,
     142,   919,     0,     0,   260,   574,   226,   931,   932,   546,
       0,   640,   641,     0,   638,   860,  1019,     0,     0,   339,
     262,     0,   260,     0,     0,   258,     0,   998,   494,     0,
       0,   950,   951,   983,   984,     0,     0,     0,   898,   868,
     869,   870,   877,     0,    37,    38,    40,    41,    39,     0,
       0,     0,   883,   889,   890,   891,   894,   892,   893,     0,
     881,   879,   880,   904,   861,     0,   912,  1017,  1016,     0,
     260,     0,   937,     0,   783,     0,   298,     0,   205,   148,
     205,     0,   205,     0,     0,     0,     0,   268,   269,   280,
       0,   140,   278,   177,   288,     0,   288,     0,   860,     0,
       0,     0,     0,     0,   860,  1115,  1118,  1083,   861,     0,
    1078,     0,   861,   833,   834,   831,   832,   867,     0,   861,
     859,   607,   635,   635,   607,   635,   598,   635,     0,     0,
    1026,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1121,
     214,     0,   217,   181,     0,     0,     0,   119,     0,   124,
     125,   117,   862,   122,     0,   352,     0,  1056,   169,  1072,
    1107,  1098,  1102,   261,   263,   362,     0,     0,  1004,     0,
     605,     0,  1036,     0,    17,   205,  1059,   259,   362,     0,
       0,     0,   767,   592,     0,   773,  1061,     0,  1094,   581,
     135,   137,     0,     0,     0,  1136,     0,     0,   325,   323,
     925,   938,  1074,   925,   939,  1074,  1076,   991,     0,     0,
       0,   344,   133,   207,   209,   210,   487,   188,   206,   186,
     187,   211,   141,     0,   919,   257,     0,   919,     0,   545,
    1023,  1022,     0,   339,     0,     0,     0,     0,     0,     0,
     260,   228,   784,   924,   339,     0,   873,   874,   875,   876,
     884,   885,   902,     0,   861,     0,   898,   611,   637,   637,
     611,   637,     0,   872,   906,   637,     0,   860,   909,   911,
     913,     0,  1011,     0,   924,     0,     0,     0,   205,   295,
     588,   153,     0,   473,   268,   270,  1006,     0,     0,     0,
     205,     0,     0,     0,     0,     0,   282,     0,  1122,     0,
       0,  1108,     0,  1114,  1112,  1079,   860,  1085,     0,  1086,
       0,     0,   803,   860,   858,     0,     0,   861,     0,     0,
     847,   861,     0,     0,     0,     0,   861,     0,     0,   813,
     848,   849,  1030,     0,   861,   816,   818,   817,     0,     0,
     814,   815,   819,   821,   820,   837,   838,   835,   836,   839,
     840,   841,   842,   843,   828,   829,   823,   824,   822,   825,
     826,   827,   830,  1120,     0,   140,     0,     0,     0,     0,
     120,    23,   354,     0,     0,     0,  1099,  1104,     0,   436,
    1008,  1006,   479,   484,   492,     0,     0,    15,     0,   436,
     671,     0,     0,   673,   666,     0,   669,     0,   665,     0,
    1063,     0,     0,     0,   969,   544,     0,   490,  1095,   585,
    1136,     0,   326,   327,     0,     0,   321,     0,     0,     0,
     346,   347,   345,  1094,     0,   362,     0,   919,     0,   339,
       0,   980,   362,  1059,   362,  1062,     0,     0,     0,   495,
       0,     0,   887,   860,   897,   878,     0,     0,   861,     0,
       0,   896,   861,     0,     0,     0,   871,     0,     0,   861,
       0,   882,   903,  1015,   362,     0,   140,     0,   291,   277,
       0,     0,     0,   267,   173,   281,     0,     0,   284,     0,
     289,   290,   140,   283,  1123,  1109,     0,     0,  1082,  1081,
       0,     0,  1134,   866,   865,   802,   615,   860,   606,     0,
     618,   860,   634,   627,   630,   621,     0,   860,   597,   804,
     624,     0,   639,   860,  1025,   845,     0,     0,   205,    24,
      25,    26,    27,  1101,  1096,  1097,  1100,   264,     0,     0,
       0,   443,   441,   440,   439,   434,     0,     0,     0,   239,
     361,     0,     0,   433,     0,     0,     0,     0,  1059,   436,
       0,   604,  1039,   358,   245,   661,     0,   664,     0,   591,
     579,   487,   138,   205,     0,     0,   330,   319,     0,   322,
     329,   339,   339,   335,   571,  1094,   436,  1094,     0,  1021,
       0,   979,   436,     0,   436,  1064,   362,   919,   976,   901,
     900,   886,   616,   860,   610,     0,   619,   860,   636,   628,
     631,   622,     0,   888,   860,   905,   625,   436,   140,   205,
     149,   154,   175,   271,   205,   279,   285,   140,   287,     0,
    1110,  1080,  1084,     0,     0,     0,   609,   846,   594,     0,
    1029,  1028,   844,   140,   218,  1103,     0,     0,     0,  1067,
       0,     0,     0,   265,     0,  1059,     0,   399,   395,   401,
     762,    36,     0,   389,     0,   394,   398,   411,     0,   409,
     414,     0,   413,     0,   412,   453,     0,   222,     0,     0,
       0,   367,     0,   368,   369,     0,     0,   435,  1005,     0,
     662,   660,   672,   670,     0,   331,   332,     0,     0,   317,
     328,     0,     0,     0,  1094,  1088,   235,   571,  1094,   981,
     241,   358,   247,   436,     0,     0,     0,   613,   895,   908,
       0,   243,   293,   205,   205,   140,   274,   174,   286,  1111,
    1133,   864,     0,     0,     0,   205,     0,     0,   462,     0,
    1068,     0,   379,   383,   459,   460,   393,     0,     0,     0,
     374,   721,   722,   720,   723,   724,   741,   743,   742,   712,
     684,   682,   683,   702,   717,   718,   678,   689,   690,   692,
     691,   759,   711,   695,   693,   694,   696,   697,   698,   699,
     700,   701,   703,   704,   705,   706,   707,   708,   710,   709,
     679,   680,   681,   685,   686,   688,   758,   726,   727,   731,
     732,   733,   734,   735,   736,   719,   738,   728,   729,   730,
     713,   714,   715,   716,   739,   740,   744,   746,   745,   747,
     748,   725,   750,   749,   752,   754,   753,   687,   757,   755,
     756,   751,   737,   677,   406,   674,     0,   375,   427,   428,
     426,   419,     0,   420,   376,     0,     0,   363,     0,     0,
       0,     0,   458,     0,   222,     0,     0,   231,   357,     0,
       0,     0,   318,   334,   977,   978,     0,     0,     0,     0,
    1094,  1088,     0,   237,  1094,   899,     0,     0,   140,   272,
     155,   176,   205,   608,   593,  1027,   216,   377,   378,   456,
     266,     0,   861,   861,     0,   402,   390,     0,     0,     0,
     408,   410,     0,     0,   415,   422,   423,   421,   454,   451,
    1069,     0,   364,     0,     0,   461,     0,   365,     0,   359,
       0,   333,     0,   656,   863,   136,   863,  1090,     0,   429,
     136,   225,     0,     0,   233,     0,   612,   907,   205,     0,
     178,   380,   126,     0,   381,   382,     0,   860,     0,   860,
     404,   400,   405,   675,   676,     0,   391,   424,   425,   417,
     418,   416,     0,  1107,   370,   457,   455,     0,   366,   360,
     657,   862,     0,   205,   862,  1089,     0,  1093,   205,   136,
     227,   229,     0,   275,     0,   220,     0,   436,     0,   396,
     403,   407,   452,     0,   919,   372,     0,   654,   570,   573,
    1091,  1092,   430,   205,   273,     0,     0,   179,   387,     0,
     435,   397,  1070,     0,   863,   447,   919,   655,   575,     0,
     219,     0,     0,   386,  1094,   919,   302,  1136,   450,   449,
     448,  1136,   446,     0,     0,     0,   385,  1088,   447,     0,
       0,  1094,     0,   384,     0,  1136,  1136,   308,     0,   307,
     305,  1088,   140,   431,   136,   371,     0,     0,   309,     0,
       0,   303,     0,   205,   205,   313,     0,   312,   301,     0,
     304,   311,   373,   215,   432,   314,     0,     0,   299,   310,
       0,   300,   316,   315
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1833, -1833, -1833,  -565, -1833, -1833, -1833,   537,   -47,   -31,
     710, -1833,  -186,  -520, -1833, -1833,   519,   500,  1929, -1833,
    2904, -1833,  -810, -1833,  -538, -1833,  -699,    23, -1833, -1833,
   -1833, -1833, -1833, -1833, -1833, -1833,  -953, -1833, -1833,  -910,
    -313, -1833, -1833, -1833,  -323, -1833, -1833,  -179,   111,    49,
   -1833, -1833, -1833, -1833, -1833, -1833,    52, -1833, -1833, -1833,
   -1833, -1833, -1833,    56, -1833, -1833,  1212,  1220,  1221,   -87,
    -752,  -944,   681,   750,  -325,   403, -1015, -1833,   -26, -1833,
   -1833, -1833, -1833,  -779,   222, -1833, -1833, -1833, -1833,  -306,
   -1833,  -605, -1833,   493,  -440, -1833, -1833,  1125, -1833,    12,
   -1833, -1833, -1125, -1833, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833,   -24, -1833,    75, -1833, -1833, -1833, -1833, -1833,  -110,
   -1833,   184,  -993, -1833, -1300,  -333, -1833,  -138,    96,  -130,
    -305, -1832, -1548, -1833, -1833, -1833,   194,   -93,   -73,   -11,
    -783,   -18, -1833, -1833,    37, -1833,    -9,  -355, -1833,    17,
      -5,   -63,   -81,    29, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833, -1833, -1833,  -625,  -895, -1833, -1833, -1833, -1833, -1833,
     689,  1378, -1833,   628, -1833,   468, -1833, -1833, -1833, -1833,
   -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833, -1833, -1833, -1833, -1833,   397,  -322,  -467, -1833, -1833,
   -1833, -1833, -1833,   551, -1833, -1833, -1833, -1833, -1833, -1833,
   -1833, -1833,  -981,  -366,  2915,    35, -1833,  1098,  -418, -1833,
   -1833,  -484,  3772,  3881, -1833,  -652, -1833, -1833,   633,  -445,
    -681, -1833, -1833,   717,   480,  -740, -1833,   482, -1833, -1833,
   -1833, -1833, -1833,   696, -1833, -1833, -1833,    28,  -928,  -161,
    -420,  -413, -1833,   -44,  -115, -1833, -1833,    44,    47,   720,
     -62, -1833, -1833,    18,   -68, -1833,  -362,    42,  -387,   212,
    -423, -1833, -1833,  -446,  1394, -1833, -1833, -1833, -1833, -1833,
     928,   649, -1833, -1833, -1833,  -358,  -722, -1833,  1348, -1290,
    -160,   -58,  -180,   916, -1833, -1833, -1833, -1798, -1833,  -217,
    -741, -1341,  -205,   226, -1833,   591,   666, -1833, -1833, -1833,
   -1833,   614, -1833,  1718,  -799
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   116,   975,   671,   192,  1689,   785,
     372,   373,   374,   375,   926,   927,   928,   118,   119,   120,
     121,   122,   997,  1240,   432,  1029,   705,   706,   579,   268,
    1763,   585,  1664,  1764,  2025,   911,   124,   125,   726,   727,
     735,   365,   608,  1980,  1193,  1415,  2047,   455,   193,   707,
    1032,  1278,  1488,   128,   674,  1051,   708,   741,  1055,   646,
    1050,   247,   560,   709,   675,  1052,   457,   392,   414,   131,
    1034,   978,   951,  1213,  1692,  1338,  1117,  1920,  1767,   860,
    1123,   584,   869,  1125,  1532,   852,  1106,  1109,  1327,  2053,
    2054,   695,   696,  1013,   722,   723,   379,   380,   382,  1729,
    1898,  1899,  1429,  1590,  2034,  2056,  1931,  1984,  1985,  1986,
    1702,  1703,  1704,  1705,  1933,  1934,  1940,  1996,  1708,  1709,
    1713,  1881,  1882,  1883,  1971,  2095,  1591,  1592,   194,   133,
    2071,  1594,  1716,  1595,  1596,  1597,  1598,   134,   135,   654,
     581,   136,   137,   138,   139,   140,   141,   142,   143,   261,
     144,   145,   146,  1744,   147,  1031,  1277,   148,   692,   693,
     694,   265,   424,   575,   680,   681,  1376,   682,  1377,   149,
     150,   652,   653,  1366,  1367,  1497,  1498,   151,   895,  1083,
     152,   896,  1084,   153,   897,  1085,   154,   898,  1086,   155,
     899,  1087,   156,   900,  1088,   655,  1369,  1500,   157,   901,
     158,   159,  1964,   160,   676,  1731,   677,  1229,   984,  1448,
    1444,  1874,  1875,   161,   162,   163,   250,   164,   251,   262,
     436,   567,   165,  1370,  1371,   905,   906,   166,  1148,   559,
     623,  1149,  1091,  1300,  1092,  1501,  1502,  1303,  1304,  1094,
    1508,  1509,  1095,   828,   550,   206,   207,   710,   698,   534,
    1250,  1251,   816,   817,   465,   168,   253,   169,   170,   196,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     744,   257,   258,   649,   241,   242,   780,   781,  1383,  1384,
     407,   408,   969,   182,   637,   183,   691,   184,   355,  1900,
    1951,   393,   444,   716,   717,  1138,  1139,  1909,  1966,  1967,
    1244,  1426,   947,  1427,   948,   949,   875,   876,   877,   356,
     357,   908,   594,  1002,  1003
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     195,   197,   462,   199,   200,   201,   202,   204,   205,   352,
     208,   209,   210,   211,   542,  1000,   231,   232,   233,   234,
     235,   236,   237,   238,   240,   353,   259,   123,   429,   515,
     686,   449,   167,   266,   851,  1030,   426,   229,   229,   267,
     415,   431,  1017,  1110,  1242,   419,   420,   275,   362,   278,
     264,   683,   363,   127,   366,   685,   129,   535,   536,   687,
     130,   450,   451,   269,   462,   249,   564,   995,   273,  1054,
     909,  1245,   256,   458,   254,   401,   837,   255,  1113,  1141,
     514,   267,   777,   778,   775,  1100,  1576,  1093,   996,  1127,
    1276,   427,  1234,   730,   361,   823,   568,   819,   820,  1334,
     132,   925,   930,  1439,   428,   830,   974,  1781,  1287,  1263,
     429,  1268,   -78,  1973,   814,   126,   821,   -78,   426,   865,
     867,   815,   -43,   431,   936,   -42,   844,   -43,   576,    14,
     -42,   446,  1530,   576,   629,    14,   980,   736,   737,   738,
    1942,   553,   452,   569,   953,   632,   576,  1720,   847,   552,
    1722,  -392,   402,   463,   431,   848,  1323,  1242,  1019,  1789,
    1876,  1886,  -956,  1886,   562,   953,  1886,  1943,  1886,  1781,
    1889,   953,   561,   609,   953,  1090,   625,   953,   842,  1895,
    1463,   613,   615,   617,  1605,   620,   428,   377,    14,   381,
     532,   533,   551,   545, -1074,   945,   946,  -110,  1159,  1312,
    1343,  1344,  1246,   532,   533,  1188,   532,   533,   959,   961,
     502,    14,   918,  -110,    14,   443,  -109,   428,   198,   532,
     533,     3,   503,  2072,  1424,  1425,   443,  1962,   662,   405,
     406,  1247,  -109,  1615, -1074,  2088,   988,   610,  1160,   626,
     428,   661,   539,   461,   463,  1203,  2072,  2106,  -653,   981,
    -999,  -651,  -642,  -968,  -771,   571,   539,   229,   571,   580,
     404,  1375,  2036,   516,   982,   267,   582,  1313,  -765,  -958,
     973,  1008,  1963,  1464, -1002,   919,  1248,  -862,  1616,  2084,
    2089,  -862,   742,  -324,   983,   464,   593,   573,  -650,  1690,
     868,   578,  2107,  2102,  -956, -1001,  1346,   416,  1241,  1782,
    1783,  1531,  -940,  1290,   -78,   543,  -954,  2037,   640,   378,
     866,   352,  -463,  -957,   -43,   937,   639,   -42,  -955,   938,
    1112,   643,  1523,   447,   434,   577,   630,   604,  1302,  -860,
    1272,  1944,  1453,  -953,  -766,   954,  1046,   633,   659,  1721,
    -324,  -582,  1723,  -392,  1576,  1341,  1487,  1345,  1958,  1020,
    1626,  1790,  1877,  1887,   202,  1952,  1065,  1632,  1957,  1634,
    2008,  2083,  1430,  -306,   824,  1663,  -862,   732,  1728,  1249,
    1736,  1617,   260,  2090,   540,   728,   464,   638,   532,   533,
     431,  -965,  -999,   267,   428,  2108,   462,  1446,   540,  1657,
     240,   651,   267,   267,   651,   267,   229,   740,  -772,   352,
     665,  -958,  1199,  1200,  1784,   229, -1002,   642,   213,    40,
    1225,  1241,   229,   267,  -764,   353,  1090,   538,   942,  1507,
     204,  1447,   263,  -960,   229,  1129,  1273, -1001,   711,  1890,
    1891,  1135,  1892,  -650,  -940,  -964,  -941,   270,  -954,   729,
     724,  -463,  -963,   731,  -463,  -957,  1461,  -970,   463,  -650,
    -955,  -966,   697,   415,   790,   791,   458,   383,   743,   745,
     271,   784,  -967,  1737,   272,  -953,   384,   213,    40,   746,
     747,   748,   749,   751,   752,   753,   754,   755,   756,   757,
     758,   759,   760,   761,   762,   763,   764,   765,   766,   767,
     768,   769,   770,   771,   772,   773,   774,  1451,   776,   795,
     743,   743,   779,  1211,  1438,   364,  1216,   352,   797,  -652,
    2098,  1753,   798,   799,   800,   801,   802,   803,   804,   805,
     806,   807,   808,   809,   810,  2115,  2016,   433,   132,   387,
     112,   421,   724,   724,   743,   822,   625,   249,  1004,   798,
    1005,   117,   826,   126,   256,   660,   254,  -927,  -930,   255,
    1520,   834,  -972,   836,   796,  -960,  -961,  1540,   388,   229,
     376,   724,   538,  -927,  -930,   854,  1302,  1499,  -941,   855,
    1499,   856,   515,   918,   402,   532,   533,  1512,   538,  1511,
    1613,   734,   667,  1340,   385,  1253,  1011,  1012,   411,   112,
     276,   412,  1254,   351,   386,  1343,  1344,   389,   985,   442,
     686,  -767,   859,  2099,  -928,  1048,   390,   841,  2067,  1284,
     391,   532,   533,  -126,  1056,  1424,  1425,  -126,  2116,  1459,
    -928,   683,   932,   514,   402,   685,   793,  2017,   394,   687,
    1380,   395,   667,   413,  -126,   391,   442,  1060,  -968,  1292,
     391,   391,  2085,   422,   871,  2068,  2069,  2070,  -773,  1318,
     423,   405,   406,   396,  1090,  1090,  1090,  1090,  1090,  1090,
     397,  1614,  1679,  1090,  1265,  1090,  1265,  1945,   391,  1474,
    1004,  1005,  1476,  2020,   428,  2021,   398,  1101,  1103,  2068,
    2069,  2070,   925,  1253,  1036,  1049,  1946,   402,  -961,  1947,
    1254,  1533,   399,  1357,   212,   403,   400,  1360, -1074,   714,
     416,   405,   406,   417,  1364,   872,   442,  1267,   224,   224,
    1269,  1270,   418,  1937,   564,   402,  1014,    50,   441,   443,
     786,   697,   442,  1102,   171,   402,  1194,  1189,  1195,   445,
    1196,  1938,  1624,   435,  1198, -1074,  1440,   229, -1074,   228,
     230,   672,   673,   448,   549,  1039,   818,   453,   713,  1441,
    1939,  1639,  1999,  1640,   216,   217,   218,   219,   220,  1760,
     402,  1633,   466,   404,   405,   406,   786,   454,   667,   686,
    1442,  2000,   945,   946,  2001,   467,   189,   843,  1047,    91,
     849,   468,    93,    94,  -643,    95,   190,    97,   469,   873,
     683,   507,   405,   406,   685,   656,   470,   658,   687,  1107,
    1108,   117,   405,   406,   471,   117,   516,  1059,  1489,   583,
      55,   108,   229,  1325,  1326,   688,  1184,  1185,  1186,   459,
     186,   187,    65,    66,    67,   472,  1610,  1686,  1687,   430,
     132,  1372,  1187,  1374,  1469,  1378,   668,   405,   406,  1090,
    1105,  1090,  1503,  1480,  1505,   126,   473,   402,  1510,  -644,
     580,   229,  -645,   229,  1490,   438,   267,  -648,   402,  1494,
     459,   186,   187,    65,    66,    67,   667,   376,   376,   376,
     618,   376,  1628,  -646,  1111,  -647,  1725,  1568,   499,   500,
     501,   229,   502,   505,  1745,   506,  1747,  1265,   508,  1122,
    1522,   537,   460,  -962,   503,   603,  1181,  1182,  1183,  1184,
    1185,  1186,  -649,  1756,  -765,  1757,   541,  1758,   548,   670,
     409,   430,  1759,   546,  1030,  1187,   459,   186,   187,    65,
      66,    67,  1548,   503,   405,   406,  1552,   686,   224,  1969,
    1970,  1558,   443,   460,   554,   405,   406,  2093,  2094,  1564,
    2063,   557,   430,   784,  1342,  1343,  1344,   132,   683,  1997,
    1998,  1220,   685,  1221,   229,   856,   687,  1993,  1994,   555,
    1252,  1255,   126,   558,  -966,   563,  1223,   538,  -763,   117,
     229,   229,   565,  1785,  1527,  1343,  1344,   566,  1659,   715,
     574,  1233,   587,   351,   171,  1600, -1119,   595,   171,   460,
     598,  1090,   391,  1090,  1668,  1090,   599,   123,  1291,   605,
    1090,   606,   167,  1907,   621,   132,   622,  1911,   624,   631,
    1264,  1261,  1264,   634,   731,  1915,   731,   797,   635,   644,
     126,   798,   645,   127,   689,   690,   129,   697,   699,   700,
     130,   701,  1754,   703,  -131,  1279,   712,    55,  1280,  1630,
    1281,   437,   439,   440,   724,   603,   391,   788,   391,   391,
     391,   391,   663,  1644,   734,   739,   669,  1648,   829,   697,
    1242,   827,   662,   831,  1655,  1242,   832,   224,   857,   838,
     132,   813,  1467,   839,   576,  1468,   224,   612,   614,   616,
     861,   619,   864,   224,   663,   126,   669,   663,   669,   669,
    1242,   603,   249,   132,  1322,   224,   593,   730,   628,   256,
     878,   254,   879,  1090,   255,   846,  2055,   636,   126,   641,
     910,   912,  1381,   913,   648,   915,   117,   226,   226,   914,
    1762,   929,   929,   916,  1328,   132,   666,   917,  2055,  1768,
     921,  1329,   229,   229,   920,   935,   907,  2078,   939,   940,
     126,   943,   736,   737,   738,  1775,   944,   950,   952,  1454,
     955,  1242,   171,   958,   960,  2013,   957,   962,   971,   963,
    2018,   964,   931,   715,   965,   976,   977,   979,   733,  1972,
     686,  -788,   986,  1975,   987,   989,  1976,  1977,   990,  1455,
    1456,  1741,  1742,   991,   994,   496,   497,   498,   499,   500,
     501,   683,   502,   999,   998,   685,  1007,   968,   970,   687,
    1015,  1432,  1235,  1009,   503,  1016,   132,  1018,   132,  2043,
    1433,  1021,  1022,  1033,  1434,  1023,   818,   818,  1024,  1045,
    1025,   126,  1053,   126,  1044,  1445,  1037,  1922,  1041,  1042,
     224,  1063,  1061,  1264,  -769,  1062,  1035,   731,  1104,  1114,
    1124,  1126,  1128,  1132,  1133,  1134,  1136,  1150,   123,  1151,
    1152,  1153,   743,   167,  1156,  1472,  1154,  1157,  1155,  1192,
    1457,   648,  1202,  1204,  1090,  1090,  1206,   686,  2079,  1209,
     212,   117,  2080,  1210,   127,  1208,   229,   129,   724,   391,
    1215,   130,  1219,  2012,  2104,  2015,  2096,  2097,   683,   724,
    1434,  1222,   685,    50,  1228,  1230,   687,  1231,  1236,   171,
    1232,  1243,   697,  1238,  1257,   697,  1286,  1274,  1283,  1289,
    1294,  -971,  1295,   849,  1241,   849,  1306,  1305,  1307,  1241,
    1308,   132,   580,  2077,  1309,  1310,  1311,  1314,   267,  1315,
     216,   217,   218,   219,   220,  1316,   126,   226,  1529,   229,
    2091,   535,  1515,  1317,  1241,  1337,  1319,  1339,  1331,  1333,
    1336,  1518,  1348,  1349,   229,   229,   456,  1350,    93,    94,
    1356,    95,   190,    97,  1082,  1187,  1096,  1358, -1135,  1363,
    1359,   429,  1414,  2066,  1428,  1362,  1431,  1416,  1417,   426,
    1978,  1418,  1449,  1462,   431,  1419,  1421,   108,   117, -1137,
   -1137, -1137, -1137, -1137,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,  1120,   117,  1049,  1241,  1465,  1450,   224,  1466,
    1485,  1473,   929,  1477,   929,  1187,   929,  1479,  1475,  1481,
     929,  1492,   929,   929,  1201,  1482,  1484,  1493,  1491,  1072,
    1601,  1506,  1514,  1516,  1517,  1524,   132,  1606,  1519,  1010,
    1528,  1534,  1608,   729,  1609,  1537,   117,   731,  1541,   229,
    1546,   126,  1547,  1550,   171,  1197,   715,  1599,  1542,  1545,
    1620,  1726,  1551,   462,  1553,  1554,  1555,  1599,   212,  1556,
     966,  1557,   967,  1629,   724,  1559,   226,  1560,  1563,  1562,
    1567,  1602,  1569,   224,   212,   226,  1212,  1988,  1990,  1570,
    1618,    50,   226,  1571,  1572,  1619,  1603,  1643,  1612,  1645,
    1621,  1622,  1637,  1623,   226,   697,  1625,    50,  1627,  1631,
    1638,   117,  1635,  1641,  1058,   684,  1636,  1647,  1652,  1642,
    1646,  1649,   224,  1650,   224,  1593,  1651,   603,   216,   217,
     218,   219,   220,  1654,   117,  1593,  1653,  1658,  1888,  1656,
    1661,   813,   813,  1660,   216,   217,   218,   219,   220,  1662,
    1672,  1665,   224,  1097,  2103,  1098,    93,    94,  1669,    95,
     190,    97,  1694,  1666,  -450,  1683,   117,  -449,  -448,  1707,
    1878,   171,    93,    94,  1879,    95,   190,    97,  1715,  1719,
    1724,  1730,  1735,  1118,  1738,   108,   171,  1739,  1748,  1743,
    1749,  1751,  1779,   391,  1755,  1770,  1773,   462,  1787,  1788,
    1884,   108,  1711,  1299,  1299,  1082,  1885,  1893,  1734,  1901,
    1902,  1904,  1906,  1740,  1949,  1908,   724,   724,  1916,  1905,
      34,    35,    36,  1914,  1917,   224,  1927,  1578,  1928,   171,
    1778,  1953,  1954,  1959,   214,  1965,  1960,  1987,   846,   226,
     846,   224,   224,  1989,  1780,   117,  2002,   117,  1991,   117,
    1995,  2003,  2010,  2011,  1599,  2014,  1207,  2024,  2004,  2019,
    1599,  2023,  1599,  -388,   132,   697,  2027,  2029,  2026,    14,
    1352,  2031,   648,  1218,  1943,  2038,  2035,  2045,  2046,   126,
    2044,  2057,  2064,  2065,  2051,  1599,  2052,  1766,  2061,    81,
      82,    83,    84,    85,   171,  1727,   603,  2074,  2087,  2100,
     221,  2081,   929,  1717,  2076,  2109,    89,    90,  2101,   132,
    2082,  2092,  2105,  2110,  2117,  1956,  2118,   171,  2120,  2121,
      99,  1420,  1593,  2060,   126,   907,   792,  1227,  1593,   787,
    1593,  1285,   789,  1903,  1579,   105,  2075,  1521,  1921,  1266,
    1580,  1266,   459,  1581,   187,  1582,  1583,  1584,  1585,   171,
    2073,  1667,  1471,  1593,   933,   132,   459,    63,    64,    65,
      66,    67,   117,  1912,   132,  1936,  1941,    72,   509,  2112,
     126,  1786,  1714,  2086,   358,  1695,  1910,   657,  1504,   126,
    1443,  1599,  1373,  1365,  1301,  1495,  1919,  1766,  1496,   650,
    1586,  1587,  1320,  1588,   725,  1142,  2007,  2040,  2033,  1685,
    1354,  1423,  1413,   224,   224,     0,     0,     0,     0,     0,
     511,     0,     0,     0,     0,   460,     0,   226,     0,     0,
       0,     0,     0,     0,  1589,     0,     0,     0,   171,   460,
     171,     0,   171,     0,  1118,  1335,     0,  1894,     0,     0,
       0,   352,     0,  1082,  1082,  1082,  1082,  1082,  1082,  1593,
       0,   212,  1082,     0,  1082,     0,  1578,  1950,   132,     0,
       0,     0,     0,     0,   132,   117,     0,     0,     0,     0,
       0,   132,     0,   126,    50,     0,  1578,   117,     0,   126,
       0,     0,   922,   923,     0,     0,   126,  1536,     0,  2049,
       0,     0,   226,     0,     0,     0,  1961,     0,    14,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   352,
       0,   216,   217,   218,   219,   220,     0,     0,    14,     0,
     462,     0,     0,     0,     0,  1950,     0,     0,     0,     0,
       0,   226,     0,   226,     0,     0,   924,     0,     0,    93,
      94,     0,    95,   190,    97,   171,     0,   224,   223,   223,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   246,
    1573,   226,  1266,  1579,     0,     0,     0,     0,   108,  1580,
       0,   459,  1581,   187,  1582,  1583,  1584,  1585,  1470,     0,
       0,  1578,     0,  1579,     0,   246,     0,     0,     0,  1580,
       0,   459,  1581,   187,  1582,  1583,  1584,  1585,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   591,     0,   592,
     224,     0,     0,     0,     0,     0,     0,     0,   132,  1586,
    1587,     0,  1588,    14,     0,   224,   224,     0,  1082,     0,
    1082,     0,     0,   126,   226,     0,     0,     0,     0,  1586,
    1587,  1513,  1588,     0,   460,     0,     0,     0,   171,     0,
     226,   226,     0,  1604,     0,     0,   648,  1118,     0,     0,
     171,     0,   697,     0,   460,     0,     0,     0,   597,     0,
       0,     0,     0,  1746,   132,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   697,   684,     0,     0,  1579,   126,
       0,     0,     0,   697,  1580,  2111,   459,  1581,   187,  1582,
    1583,  1584,  1585,     0,  2119,   117,     0,     0,     0,   132,
       0,     0,  2122,     0,   132,  2123,   351,     0,     0,     0,
     224,     0,     0,  2050,   126,  1712,     0,     0,     0,   126,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   132,
       0,     0,     0,     0,  1586,  1587,     0,  1588,     0,     0,
     117,   648,     0,     0,   126,  1578,     0,     0,     0,     0,
       0,   718,     0,     0,   358,     0,     0,     0,   223,   460,
       0,     0,  1611,     0,     0,     0,     0,     0,  1750,     0,
    1082,     0,  1082,     0,  1082,     0,     0,     0,     0,  1082,
       0,     0,     0,     0,     0,     0,   117,    14,     0,   132,
     132,   117,     0,     0,     0,   117,     0,     0,     0,     0,
       0,     0,   226,   226,   126,   126,     0,     0,   246,     0,
     246,     0,     0,     0,     0,   391,     0,     0,   603,     0,
       0,   351,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1873,     0,     0,     0,     0,     0,     0,  1880,     0,
       0,     0,     0,     0,   684,     0,   351,   351,     0,   351,
       0,     0,  1579,     0,     0,     0,   351,     0,  1580,     0,
     459,  1581,   187,  1582,  1583,  1584,  1585,     0,     0,   246,
       0,     0,     0,     0,  1578,     0,     0,     0,   171,     0,
       0,     0,  1082,     0,     0,     0,     0,     0,     0,   117,
     117,   117,     0,     0,     0,   117,     0,   223,     0,     0,
       0,   870,   117,     0,     0,     0,   223,     0,  1586,  1587,
       0,  1588,     0,   223,     0,     0,    14,     0,     0,     0,
       0,     0,     0,   171,     0,   223,     0,     0,     0,     0,
       0,     0,     0,   460,     0,  1578,   223,     0,     0,     0,
       0,     0,  1752,     0,     0,     0,   226,     0,     0,   459,
      63,    64,    65,    66,    67,     0,     0,     0,     0,     0,
      72,   509,   246,     0,     0,   246,     0,     0,     0,   171,
       0,     0,     0,     0,   171,     0,     0,    14,   171,     0,
       0,  1579,     0,     0,     0,     0,     0,  1580,     0,   459,
    1581,   187,  1582,  1583,  1584,  1585,     0,   992,   993,     0,
       0,   510,   684,   511,     0,     0,     0,     0,     0,   226,
       0,     0,     0,     0,     0,   603,     0,   512,     0,   513,
       0,   246,   460,     0,   226,   226,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1586,  1587,   351,
    1588,     0,  1579,  1082,  1082,     0,     0,     0,  1580,   117,
     459,  1581,   187,  1582,  1583,  1584,  1585,     0,  1982,     0,
     223,     0,   460,     0,     0,  1873,  1873,     0,     0,  1880,
    1880,  1761,   171,   171,   171,     0,     0,     0,   171,     0,
       0,     0,     0,   603,     0,   171,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1586,  1587,
       0,  1588,     0,     0,     0,   117,     0,     0,     0,     0,
       0,     0,   246,     0,   246,     0,     0,   894,     0,   226,
       0,     0,     0,   460,     0,     0,     0,     0,     0,     0,
       0,     0,  1913,     0,     0,     0,     0,     0,     0,     0,
     117,     0,     0,     0,     0,   117,     0,     0,     0,     0,
     894,     0,     0,  2048,   544,   518,   519,   520,   521,   522,
     523,   524,   525,   526,   527,   528,   529,     0,     0,     0,
     117,     0,     0,     0,     0,     0,  2062,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1140,   718,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   530,
     531,     0,     0,     0,     0,     0,     0,     0,   246,   246,
       0,     0,     0,     0,     0,     0,     0,   246,     0,     0,
       0,     0,     0,   474,   475,   476,     0,     0,     0,     0,
     117,   117,   171,     0,     0,     0,     0,     0,   223,     0,
       0,     0,     0,   477,   478,   684,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,   212,     0,   213,    40,     0,   532,   533,     0,     0,
       0,     0,   503,     0,  1226,     0,     0,   212,   171,   213,
      40,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,  1237,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,   223,     0,  1256,     0,     0,     0,     0,
       0,     0,     0,   171,     0,     0,     0,     0,   171,     0,
       0,   216,   217,   218,   219,   220,     0,     0,     0,     0,
     702,     0,   684,     0,     0,     0,   246,   216,   217,   218,
     219,   220,   223,   171,   223,     0,     0,   811,     0,    93,
      94,  1288,    95,   190,    97,     0,     0,     0,     0,     0,
       0,     0,     0,   811,     0,    93,    94,     0,    95,   190,
      97,     0,   223,   894,     0,     0,     0,     0,   108,     0,
     246,     0,   812,     0,     0,   112,     0,   246,   246,   894,
     894,   894,   894,   894,   108,     0,     0,     0,   845,     0,
       0,   112,   894,   171,   171,     0,     0,  1436,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   246,
       0,     0,     0,     0,     0,     0,  1347,     0,     0,   212,
    1351,     0,     0,     0,     0,  1355,     0,     0,     0,  1161,
    1162,  1163,     0,     0,     0,   223,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,   246,
    1164,   223,   223,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,
    1182,  1183,  1184,  1185,  1186,   246,   246,     0,     0,   216,
     217,   218,   219,   220,     0,     0,   223,     0,  1187,     0,
       0,     0,     0,   246,     0,     0,     0,     0,     0,     0,
     246,     0,     0,     0,   225,   225,   246,    93,    94,     0,
      95,   190,    97,     0,     0,   248,     0,   894,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     354,  1458,   246,     0,     0,     0,   108,   739,     0, -1137,
   -1137, -1137, -1137, -1137,   494,   495,   496,   497,   498,   499,
     500,   501,   246,   502,     0,     0,   246,     0,   474,   475,
     476,     0,     0,     0,     0,   503,     0,   246,     0,     0,
       0,     0,     0,     0,  1483,     0,     0,  1486,   477,   478,
       0,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,     0,     0,     0,
       0,     0,     0,   223,   223,     0,     0,   503,     0,  1379,
       0,     0,     0,     0,     0,     0,     0,   246,     0,     0,
       0,   246,     0,   246,     0,     0,   246,  1535,     0,     0,
       0,     0,     0,     0,  1539,     0,     0,     0,     0,   894,
     894,   894,   894,   894,   894,   223,   894,     0,     0,   894,
     894,   894,   894,   894,   894,   894,   894,   894,   894,   894,
     894,   894,   894,   894,   894,   894,   894,   894,   894,   894,
     894,   894,   894,   894,   894,   894,   894,     0,     0,   544,
     518,   519,   520,   521,   522,   523,   524,   525,   526,   527,
     528,   529,     0,     0,     0,   894,   212,     0,     0,     0,
       0,     0,  1574,  1575,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   225,     0,     0,     0,     0,    50,
       0,     0,     0,     0,   530,   531,     0,     0,     0,     0,
       0,     0,   246,     0,   246,     0,     0,     0,     0,   504,
       0,  1038,     0,     0,     0,     0,     0,   223,     0,     0,
       0,     0,     0,   354,     0,   354,   216,   217,   218,   219,
     220,     0,   544,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   246,     0,     0,   246,     0,
       0,   409,     0,     0,    93,    94,     0,    95,   190,    97,
       0,     0,     0,     0,     0,   246,   246,   246,   246,   246,
     246,   532,   533,   223,   246,     0,   246,   530,   531,     0,
     223,     0,     0,   108,   354,  1670,  1671,   410,     0,  1673,
       0,     0,     0,     0,     0,   223,   223,     0,   894,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   246,     0,
       0,     0,     0,     0,     0,   246,     0,     0,     0,     0,
     894,     0,   894,   225,     0,     0,     0,  1691,     0,     0,
       0,     0,   225,     0,     0,   840,     0,     0,     0,   225,
       0,  1718,     0,     0,     0,     0,     0,   894,     0,     0,
       0,   225,     0,     0,   532,   533,     0,     0,     0,     0,
       0,     0,   225,     0,     0,     0,     0,     0,   474,   475,
     476,     0,     0,     0,     0,     0,     0,   354,     0,     0,
     354,     0,     0,   246,   246,     0,     0,   246,   477,   478,
     223,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,  1769,   941,   246,
       0,     0,     0,     0,     0,     0,     0,   503,  1026,   518,
     519,   520,   521,   522,   523,   524,   525,   526,   527,   528,
     529,     0,  1691,     0,     0,     0,     0,   248,     0,     0,
     246,     0,   246,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1691,  1691,     0,
    1691,     0,     0,   530,   531,  1896,     0,  1691,     0,     0,
       0,     0,     0,     0,     0,     0,   225,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   246,   246,     0,     0,
     246,     0,     0,     0,     0,     0,   894,     0,   894,     0,
     894,     0,     0,     0,     0,   894,   223,     0,     0,     0,
     894,     0,   894,     0,     0,   894,     0,   354,     0,   874,
       0,     0,     0,     0,     0,  1932,     0,     0,   246,   246,
       0,     0,     0,   902,     0,   246,     0,     0,     0,     0,
     532,   533,   246,     0,     0,     0,     0,   289,     0,     0,
       0,   972,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   902,     0,     0,     0,
    1026,   518,   519,   520,   521,   522,   523,   524,   525,   526,
     527,   528,   529,     0,   291,     0,     0,     0,     0,     0,
       0,     0,   246,     0,   246,     0,   246,   212,     0,     0,
       0,   246,     0,   223,  1027,     0,     0,     0,     0,     0,
       0,     0,     0,   354,   354,   530,   531,     0,   246,     0,
      50,     0,   354,   894,     0,     0,     0,     0,  -435,     0,
       0,  1955,     0,     0,   871,   246,   246,   459,   186,   187,
      65,    66,    67,   246,     0,   246,  1968,     0,     0,     0,
    1691,     0,     0,     0,   225,     0,   589,   216,   217,   218,
     219,   220,   590,     0,     0,     0,     0,     0,   246,   246,
       0,   246,     0,     0,     0,     0,   246,     0,   246,   189,
       0,     0,    91,   344,   212,    93,    94,     0,    95,   190,
      97,     0,   532,   533,     0,   872,     0,   212,     0,     0,
       0,     0,     0,   348,   246,     0,     0,    50,     0,     0,
     460,     0,     0,     0,   108,   350,     0,     0,     0,     0,
      50,   894,   894,   894,     0,  2028,     0,     0,   894,   225,
     246,     0,     0,     0,     0,     0,   246,     0,   246,     0,
       0,     0,     0,     0,   216,   217,   218,   219,   220,     0,
       0,     0,  1968,     0,  2041,     0,   702,   216,   217,   218,
     219,   220,  1089,     0,     0,     0,   189,     0,   225,    91,
     225,     0,    93,    94,     0,    95,   190,    97,     0,  1353,
       0,     0,   370,     0,     0,    93,    94,     0,    95,   190,
      97,     0,     0,     0,     0,  1131,     0,     0,   225,   902,
       0,   108,   354,   354,     0,     0,     0,     0,     0,     0,
       0,   227,   227,     0,   108,   902,   902,   902,   902,   902,
       0,     0,   252,     0,     0,     0,     0,     0,   902,     0,
       0,     0,     0,     0,   894,     0,     0,     0,     0,     0,
       0,     0,   246,     0,     0,  1191,     0,     0,     0,     0,
       0,     0,     0,   212,     0,     0,     0,   246,     0,     0,
       0,   246,     0,     0,     0,   246,   246,     0,     0,     0,
     289,   225,     0,     0,     0,     0,    50,     0,     0,     0,
     246,     0,     0,     0,     0,  1214,   894,   225,   225,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     354,     0,   894,   894,     0,     0,     0,   291,     0,   894,
       0,     0,  1214,   216,   217,   218,   219,   220,   354,     0,
     212,     0,   225,     0,     0,   354,  1137,     0,     0,     0,
       0,   354,     0,     0,     0,   189,   246,     0,    91,     0,
       0,    93,    94,    50,    95,   190,    97,     0,     0,     0,
       0,   894,     0,   902,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   246,     0,   246,     0,     0,  1275,     0,
     108,     0,     0,     0,     0,  1981,     0,   354,     0,   589,
     216,   217,   218,   219,   220,   590,     0,     0,     0,     0,
       0,     0,   248,     0,   246,     0,     0,     0,     0,     0,
       0,     0,   189,  1089,     0,    91,   344,     0,    93,    94,
     246,    95,   190,    97,     0, -1136,   246,     0,     0,     0,
     246,     0,     0,     0,     0,     0,   348,     0,     0,     0,
       0,   227,     0,     0,   246,   246,     0,   108,   350,     0,
    1066,  1067,     0,     0,     0,     0,     0,     0,     0,   225,
     225,     0,   354,     0,     0,     0,   354,     0,   874,     0,
    1068,   354,     0,     0,     0,     0,     0,     0,  1069,  1070,
    1071,   212,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1072,     0,     0,   902,   902,   902,   902,   902,
     902,   225,   902,  1696,    50,   902,   902,   902,   902,   902,
     902,   902,   902,   902,   902,   902,   902,   902,   902,   902,
     902,   902,   902,   902,   902,   902,   902,   902,   902,   902,
     902,   902,   902,     0,     0,     0,     0,     0,     0,     0,
    1073,  1074,  1075,  1076,  1077,  1078,     0,     0,     0,     0,
       0,   902,     0,   212,     0,     0,     0,     0,  1079,     0,
       0,     0,     0,   189,     0,     0,    91,    92,     0,    93,
      94,     0,    95,   190,    97,     0,    50,   354,     0,   354,
     227,     0,     0,     0,   289,     0,     0,  1080,  1081,   227,
       0,     0,     0,     0,     0,     0,   227,     0,   108,  1697,
       0,     0,     0,   225,     0,     0,     0,     0,   227,     0,
       0,     0,  1698,   216,   217,   218,   219,   220,  1699,   252,
     354,   291,     0,   354,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   212,   189,  1715,     0,    91,  1700,
       0,    93,    94,     0,    95,  1701,    97,     0,     0,     0,
       0,  1089,  1089,  1089,  1089,  1089,  1089,    50,     0,   225,
    1089,     0,  1089,     0,     0,     0,   225,     0,     0,     0,
     108,     0,     0,     0,   459,   186,   187,    65,    66,    67,
       0,   225,   225,   354,   902,     0,     0,     0,     0,     0,
     354,     0,     0,   589,   216,   217,   218,   219,   220,   590,
       0,     0,     0,     0,   252,     0,   902,     0,   902,     0,
       0,     0,     0,     0,     0,     0,   189,   289,     0,    91,
     344,     0,    93,    94,     0,    95,   190,    97,     0,     0,
       0,     0,     0,   902,     0,     0,     0,     0,     0,     0,
     348,     0,     0,   227,     0,     0,     0,   460,     0,     0,
       0,   108,   350,     0,   291,     0,     0,     0,   354,   354,
       0,     0,     0,     0,     0,     0,     0,   212,     0,     0,
       0,     0,     0,  1577,     0,     0,   225,     0,   474,   475,
     476,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,   354,     0,     0,     0,   477,   478,
     903,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,   589,   216,   217,   218,
     219,   220,   590,   903,     0,     0,  1089,   503,  1089,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   189,
       0,     0,    91,   344,     0,    93,    94,     0,    95,   190,
      97,     0,     0,     0,     0,     0,   289,     0,     0,     0,
       0,   354,   354,   348,     0,   354,     0,     0,     0,     0,
       0,     0,     0,     0,   108,   350,     0,     0,     0,     0,
       0,     0,   902,     0,   902,     0,   902,     0,     0,     0,
       0,   902,   225,   291,     0,     0,   902,     0,   902,   904,
       0,   902,     0,   354,     0,     0,   212,     0,     0,     0,
       0,   227,  1538,     0,     0,  1693,     0,   354,     0,     0,
       0,  1706,     0,     0,     0,     0,     0,   289,     0,    50,
       0,     0,   934,     0,     0,     0,     0,     0,   517,   518,
     519,   520,   521,   522,   523,   524,   525,   526,   527,   528,
     529,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1006,     0,     0,   291,   589,   216,   217,   218,   219,
     220,   590,     0,     0,     0,     0,     0,   212,  1089,     0,
    1089,     0,  1089,   530,   531,     0,   227,  1089,   189,   225,
       0,    91,   344,   354,    93,    94,     0,    95,   190,    97,
      50, -1136,     0,     0,     0,     0,     0,     0,   596,   902,
       0,     0,   348,     0,     0,     0,     0,     0,   354,     0,
       0,  1776,  1777,   108,   350,   227,     0,   227,     0,     0,
       0,  1706,     0,     0,     0,     0,   589,   216,   217,   218,
     219,   220,   590,   354,   354,     0,   354,     0,     0,     0,
       0,   354,     0,   354,     0,   227,   903,     0,     0,   189,
     532,   533,    91,   344,     0,    93,    94,     0,    95,   190,
      97,     0,   903,   903,   903,   903,   903,     0,     0,     0,
       0,     0,     0,   348,     0,   903,   289,     0,     0,     0,
    1089,     0,     0,     0,   108,   350,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   902,   902,   902,
       0,   354,     0,     0,   902,     0,  1930,     0,     0,     0,
       0,     0,     0,   291,  1706,     0,     0,     0,   227,     0,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     0,
       0,     0,  1001,     0,   227,   227,   544,   518,   519,   520,
     521,   522,   523,   524,   525,   526,   527,   528,   529,    50,
       0,     0,     0,     0,     0,  1119,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   252,
       0,  1143,  1144,  1145,  1146,  1147,     0,     0,     0,     0,
       0,   530,   531,     0,  1158,   589,   216,   217,   218,   219,
     220,   590,     0,     0,     0,     0,     0,     0,     0,     0,
     903,     0,     0,     0,     0,     0,     0,   354,   189,     0,
     902,    91,   344,     0,    93,    94,     0,    95,   190,    97,
       0,     0,   354,     0,     0,     0,   354,     0,     0,     0,
       0,     0,   348,     0,     0,     0,     0,     0,     0,   252,
       0,  1089,  1089,   108,   350,  1983,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   532,   533,
       0,     0,   902,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   902,   902,
       0,     0,     0,     0,     0,   902,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   227,   227,     0,     0,
       0,   354,     0,     0,     0,     0,     0,     0,     0,  1262,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,   902,   354,     0,
     354,     0,   903,   903,   903,   903,   903,   903,   252,   903,
    1187,     0,   903,   903,   903,   903,   903,   903,   903,   903,
     903,   903,   903,   903,   903,   903,   903,   903,   903,   903,
     903,   903,   903,   903,   903,   903,   903,   903,   903,   903,
    2059,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   354,     0,     0,     0,   354,  1693,     0,   903,   474,
     475,   476,     0,     0,     0,     0,     0,     0,     0,   354,
     354,     0,     0,     0,     0,     0,     0,     0,     0,   477,
     478,     0,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,     0,     0,     0,
     227,  1147,  1368,     0,     0,  1368,     0,     0,   503,     0,
       0,  1382,  1385,  1386,  1387,  1389,  1390,  1391,  1392,  1393,
    1394,  1395,  1396,  1397,  1398,  1399,  1400,  1401,  1402,  1403,
    1404,  1405,  1406,  1407,  1408,  1409,  1410,  1411,  1412,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   252,  1422,     0,     0,
       0,     0,     0,   227,  1026,   518,   519,   520,   521,   522,
     523,   524,   525,   526,   527,   528,   529,     0,   227,   227,
       0,   903,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,   903,   502,   903,     0,     0,   212,   530,
     531,  1296,  1297,  1298,   212,     0,   503,     0,     0,     0,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
     903,    50,     0,     0,     0,     0,     0,    50,     0,     0,
     477,   478,  1038,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,   216,   217,
     218,   219,   220,   227,   216,   217,   218,   219,   220,   503,
       0,     0,     0,     0,     0,     0,   532,   533,     0,     0,
    1525,     0,     0,     0,     0,     0,    93,    94,     0,    95,
     190,    97,    93,    94,     0,    95,   190,    97,     0,     0,
       0,     0,  1543,     0,  1544,     0,     0,   474,   475,   476,
       0,     0,     0,     0,     0,   108,  1035,     0,     0,     0,
       0,   108,     0,     0,     0,     0,     0,   477,   478,  1565,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   503,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   903,
       0,   903,     0,   903,     0,     0,     0,     0,   903,   252,
       0,     0,     0,   903,     0,   903,     0,     0,   903,   474,
     475,   476,     0,  1064,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   477,
     478,     0,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,   279,   280,     0,
     281,   282,     0,     0,   283,   284,   285,   286,   503,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   287,   288,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   252,     0,  1675,     0,
    1676,     0,  1677,     0,     0,     0,     0,  1678,     0,     0,
    1205,   290,  1680,     0,  1681,     0,   903,  1682,     0,     0,
       0,     0,     0,     0,     0,   292,   293,   294,   295,   296,
     297,   298,     0,     0,     0,   212,     0,   213,    40,     0,
       0,   299,     0,     0,     0,     0,     0,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    50,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,     0,   335,     0,   782,   337,   338,   339,
       0,     0,     0,   340,   600,   216,   217,   218,   219,   220,
     601,     0,  1282,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   903,   903,   903,   602,     0,     0,
       0,   903,     0,    93,    94,  1771,    95,   190,    97,   345,
    1935,   346,     0,     0,   347,     0,     0,   474,   475,   476,
       0,     0,   349,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   108,     0,     0,     0,   783,   477,   478,   112,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   503,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   474,   475,
     476,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1923,  1924,  1925,     0,   903,   477,   478,
    1929,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   503,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   903,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   903,   903,     0,     0,     0,
     477,   478,   903,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,     0,     0,
    1293,  2030,     0,     0,     0,     0,  1948,     0,     0,   503,
       0,     0,     0,     0,   903,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,
    1182,  1183,  1184,  1185,  1186,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,  1187,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1992,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1324,     0,     0,  2005,  2006,     0,     0,     0,     0,
      14,  2009,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,     0,
      42,     0,     0,  2032,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
     825,    52,    53,    54,    55,    56,    57,    58,     0,    59,
    -205,    60,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,    88,    89,    90,    91,    92,
       0,    93,    94,     0,    95,    96,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,   103,     0,   104,     0,   105,   106,   107,     0,     0,
     108,   109,     0,   110,   111,     0,     0,   112,   113,   114,
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
      55,    56,    57,    58,     0,    59,     0,    60,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,    88,    89,    90,    91,    92,     0,    93,    94,     0,
      95,    96,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,   103,     0,   104,
       0,   105,   106,   107,     0,     0,   108,   109,     0,   110,
     111,  1224,     0,   112,   113,   114,   115,     5,     6,     7,
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
       0,     0,     0,    52,    53,    54,    55,    56,    57,    58,
       0,    59,     0,    60,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,    88,    89,    90,
      91,    92,     0,    93,    94,     0,    95,    96,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,   103,     0,   104,     0,   105,   106,   107,
       0,     0,   108,   109,     0,   110,   111,  1437,     0,   112,
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
       0,     0,     0,   189,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   190,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
       0,     0,     0,   105,   106,   107,     0,     0,   108,   109,
       0,   110,   111,   704,     0,   112,   113,   114,   115,     5,
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
       0,    86,     0,     0,    87,     0,     0,     0,     0,   189,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   190,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   109,     0,   110,   111,  1028,
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
    -205,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   189,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   190,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   109,     0,   110,   111,     0,     0,   112,   113,   114,
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
       0,   189,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   190,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   109,     0,   110,
     111,  1190,     0,   112,   113,   114,   115,     5,     6,     7,
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
       0,     0,    87,     0,     0,     0,     0,   189,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   190,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,     0,     0,     0,   105,   106,   107,
       0,     0,   108,   109,     0,   110,   111,  1239,     0,   112,
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
       0,     0,     0,   189,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   190,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
       0,     0,     0,   105,   106,   107,     0,     0,   108,   109,
       0,   110,   111,  1271,     0,   112,   113,   114,   115,     5,
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
       0,    86,     0,     0,    87,     0,     0,     0,     0,   189,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   190,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   109,     0,   110,   111,  1330,
       0,   112,   113,   114,   115,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,     0,
      42,     0,     0,     0,    43,    44,    45,    46,  1332,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   189,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   190,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   109,     0,   110,   111,     0,     0,   112,   113,   114,
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
    1526,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   189,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   190,    97,    98,     0,     0,    99,     0,     0,   100,
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
       0,     0,    87,     0,     0,     0,     0,   189,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   190,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,     0,     0,     0,   105,   106,   107,
       0,     0,   108,   109,     0,   110,   111,  1684,     0,   112,
     113,   114,   115,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,  -297,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   189,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   190,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
       0,     0,     0,   105,   106,   107,     0,     0,   108,   109,
       0,   110,   111,     0,     0,   112,   113,   114,   115,     5,
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
       0,    86,     0,     0,    87,     0,     0,     0,     0,   189,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   190,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   109,     0,   110,   111,  1926,
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
       0,    48,  1979,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   189,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   190,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   109,     0,   110,   111,     0,     0,   112,   113,   114,
     115,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,  2022,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   189,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   190,    97,    98,     0,     0,    99,     0,     0,   100,
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
       0,     0,    87,     0,     0,     0,     0,   189,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   190,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,     0,     0,     0,   105,   106,   107,
       0,     0,   108,   109,     0,   110,   111,  2039,     0,   112,
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
       0,     0,     0,   189,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   190,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
       0,     0,     0,   105,   106,   107,     0,     0,   108,   109,
       0,   110,   111,  2042,     0,   112,   113,   114,   115,     5,
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
       0,    86,     0,     0,    87,     0,     0,     0,     0,   189,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   190,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   109,     0,   110,   111,  2058,
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
      87,     0,     0,     0,     0,   189,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   190,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   109,     0,   110,   111,  2113,     0,   112,   113,   114,
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
       0,   189,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   190,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   109,     0,   110,
     111,  2114,     0,   112,   113,   114,   115,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,   572,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,     0,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,     0,    61,    62,   186,   187,    65,    66,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   189,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   190,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,     0,     0,     0,   105,   106,   107,
       0,     0,   108,   109,     0,   110,   111,     0,     0,   112,
     113,   114,   115,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,   858,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,   186,   187,    65,    66,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   189,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   190,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
       0,     0,     0,   105,   106,   107,     0,     0,   108,   109,
       0,   110,   111,     0,     0,   112,   113,   114,   115,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,  1121,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,     0,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,     0,    61,    62,   186,   187,
      65,    66,    67,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   189,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   190,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   109,     0,   110,   111,     0,
       0,   112,   113,   114,   115,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,  1765,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,     0,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,     0,    61,    62,   186,   187,    65,    66,    67,     0,
      68,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   189,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   190,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   109,     0,   110,   111,     0,     0,   112,   113,   114,
     115,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
    1918,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
     186,   187,    65,    66,    67,     0,    68,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   189,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   190,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   109,     0,   110,
     111,     0,     0,   112,   113,   114,   115,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,     0,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,     0,    61,    62,   186,   187,    65,    66,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   189,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   190,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,     0,     0,     0,   105,   106,   107,
       0,     0,   108,   109,     0,   110,   111,     0,     0,   112,
     113,   114,   115,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   359,   425,    13,
       0,     0,     0,     0,     0,     0,     0,     0,   794,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,   185,   186,   187,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   188,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   189,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   190,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
       0,     0,     0,   105,   106,   107,     0,     0,   108,   109,
       5,     6,     7,     8,     9,   112,   113,   114,   115,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   359,     0,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   185,   186,
     187,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   188,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     189,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     190,    97,     0,     0,     0,    99,     0,     0,   100,     5,
       6,     7,     8,     9,   101,   102,     0,     0,     0,    10,
     105,   106,   107,     0,     0,   108,   191,     0,   360,     0,
       0,     0,   112,   113,   114,   115,     0,     0,     0,     0,
       0,     0,     0,     0,   719,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,   720,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,   185,   186,   187,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   188,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   189,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   190,
      97,     0,   721,     0,    99,     0,     0,   100,     5,     6,
       7,     8,     9,   101,   102,     0,     0,     0,    10,   105,
     106,   107,     0,     0,   108,   191,     0,     0,     0,     0,
       0,   112,   113,   114,   115,     0,     0,     0,     0,     0,
       0,     0,     0,  1258,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,  1259,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,   185,   186,   187,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   188,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   189,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   190,    97,
       0,  1260,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   191,     5,     6,     7,     8,     9,
     112,   113,   114,   115,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   359,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   185,   186,   187,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   188,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   189,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   190,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   191,     0,     0,   853,     0,     0,   112,   113,   114,
     115,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   359,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   794,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   185,
     186,   187,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   188,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   189,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   190,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   191,     5,     6,
       7,     8,     9,   112,   113,   114,   115,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   359,   425,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,   185,   186,   187,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   188,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   189,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   190,    97,
       0,     0,     0,    99,     0,     0,   100,     5,     6,     7,
       8,     9,   101,   102,     0,     0,     0,    10,   105,   106,
     107,     0,     0,   108,   109,     0,     0,     0,     0,     0,
     112,   113,   114,   115,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,   203,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   185,   186,   187,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   188,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   189,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   190,    97,     0,
       0,     0,    99,     0,     0,   100,     5,     6,     7,     8,
       9,   101,   102,     0,     0,     0,    10,   105,   106,   107,
       0,     0,   108,   191,     0,     0,     0,     0,     0,   112,
     113,   114,   115,     0,     0,     0,     0,     0,     0,     0,
       0,   239,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,     0,   185,   186,   187,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   188,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   189,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   190,    97,     0,     0,
       0,    99,     0,     0,   100,     5,     6,     7,     8,     9,
     101,   102,     0,     0,     0,    10,   105,   106,   107,     0,
       0,   108,   191,     0,     0,     0,     0,     0,   112,   113,
     114,   115,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   185,   186,   187,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   188,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   189,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   190,    97,     0,     0,     0,
      99,     0,     0,   100,     5,     6,     7,     8,     9,   101,
     102,     0,     0,     0,    10,   105,   106,   107,     0,     0,
     108,   191,     0,   274,     0,     0,     0,   112,   113,   114,
     115,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,   185,   186,   187,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     188,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   189,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   190,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     191,     0,   277,     0,     0,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   425,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   185,   186,
     187,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   188,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     189,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     190,    97,     0,     0,     0,    99,     0,     0,   100,     5,
       6,     7,     8,     9,   101,   102,     0,     0,     0,    10,
     105,   106,   107,     0,     0,   108,   109,     0,     0,     0,
       0,     0,   112,   113,   114,   115,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,   185,   186,   187,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   188,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   189,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   190,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   191,   570,     0,     0,     0,
       0,   112,   113,   114,   115,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   359,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   185,   186,   187,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   188,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   189,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   190,    97,     0,     0,     0,
      99,     0,     0,   100,     5,     6,     7,     8,     9,   101,
     102,     0,     0,     0,    10,   105,   106,   107,     0,     0,
     108,   191,     0,     0,     0,     0,     0,   112,   113,   114,
     115,     0,     0,   750,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,   185,   186,   187,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     188,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   189,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   190,    97,     0,     0,     0,    99,
       0,     0,   100,     5,     6,     7,     8,     9,   101,   102,
       0,     0,     0,    10,   105,   106,   107,     0,     0,   108,
     191,     0,     0,     0,     0,     0,   112,   113,   114,   115,
       0,     0,     0,     0,     0,     0,     0,     0,   794,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,   185,   186,   187,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   188,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   189,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   190,    97,     0,     0,     0,    99,     0,
       0,   100,     5,     6,     7,     8,     9,   101,   102,     0,
       0,     0,    10,   105,   106,   107,     0,     0,   108,   191,
       0,     0,     0,     0,     0,   112,   113,   114,   115,     0,
       0,     0,     0,     0,     0,     0,     0,   833,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,     0,
     185,   186,   187,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   188,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   189,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   190,    97,     0,     0,     0,    99,     0,     0,
     100,     5,     6,     7,     8,     9,   101,   102,     0,     0,
       0,    10,   105,   106,   107,     0,     0,   108,   191,     0,
       0,     0,     0,     0,   112,   113,   114,   115,     0,     0,
       0,     0,     0,     0,     0,     0,   835,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   185,
     186,   187,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   188,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   189,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   190,    97,     0,     0,     0,    99,     0,     0,   100,
       5,     6,     7,     8,     9,   101,   102,     0,     0,     0,
      10,   105,   106,   107,     0,     0,   108,   191,     0,     0,
       0,     0,     0,   112,   113,   114,   115,     0,     0,     0,
       0,     0,     0,     0,     0,  1321,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   185,   186,
     187,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   188,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     189,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     190,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   191,     5,     6,     7,
       8,     9,   112,   113,   114,   115,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   359,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   185,   186,   187,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   188,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   189,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   190,    97,     0,
       0,     0,    99,     0,     0,   100,     5,     6,     7,     8,
       9,   101,   102,     0,     0,     0,    10,   105,   106,   107,
       0,     0,   108,  1452,     0,     0,     0,     0,     0,   112,
     113,   114,   115,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,     0,   185,   186,   187,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   188,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   189,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   190,    97,     0,     0,
       0,    99,     0,     0,   100,     5,     6,     7,     8,     9,
     101,   102,     0,     0,     0,    10,   105,   106,   107,     0,
       0,   108,   191,     0,     0,     0,     0,     0,   112,   113,
     114,   115,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,   664,    39,    40,     0,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   185,   186,   187,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   188,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   189,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   190,    97,     0,   279,   280,
      99,   281,   282,   100,     0,   283,   284,   285,   286,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   191,     0,   287,   288,     0,     0,   112,   113,   114,
     115,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,   290,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1187,   292,   293,   294,   295,
     296,   297,   298,     0,     0,     0,   212,     0,   213,    40,
       0,     0,   299,     0,     0,     0,     0,     0,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,    50,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,     0,   335,     0,   336,   337,   338,
     339,     0,     0,     0,   340,   600,   216,   217,   218,   219,
     220,   601,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   279,   280,     0,   281,   282,     0,   602,   283,
     284,   285,   286,     0,    93,    94,     0,    95,   190,    97,
     345,     0,   346,     0,     0,   347,     0,   287,   288,     0,
     289,     0,     0,   349,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   108,     0,     0,     0,   783,     0,     0,
     112,     0,     0,     0,     0,     0,   290,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   291,     0,     0,
     292,   293,   294,   295,   296,   297,   298,     0,     0,     0,
     212,     0,     0,     0,     0,     0,   299,     0,     0,     0,
       0,     0,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,    50,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,     0,   335,
       0,     0,   337,   338,   339,     0,     0,     0,   340,   341,
     216,   217,   218,   219,   220,   342,     0,     0,     0,     0,
       0,     0,   212,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   343,     0,     0,    91,   344,     0,    93,    94,
       0,    95,   190,    97,   345,    50,   346,     0,     0,   347,
       0,   279,   280,     0,   281,   282,   348,   349,   283,   284,
     285,   286,     0,     0,     0,     0,     0,   108,   350,     0,
       0,  1710,  1897,     0,     0,     0,   287,   288,     0,   289,
       0,     0,   216,   217,   218,   219,   220,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,   290,   502,     0,     0,     0,
      93,    94,     0,    95,   190,    97,   291,     0,   503,   292,
     293,   294,   295,   296,   297,   298,     0,     0,     0,   212,
       0,     0,     0,     0,     0,   299,     0,     0,     0,   108,
    1711,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,    50,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,     0,   335,     0,
       0,   337,   338,   339,     0,     0,     0,   340,   341,   216,
     217,   218,   219,   220,   342,     0,     0,     0,     0,     0,
       0,   212,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   343,  1130,     0,    91,   344,     0,    93,    94,     0,
      95,   190,    97,   345,    50,   346,     0,     0,   347,     0,
     279,   280,     0,   281,   282,   348,   349,   283,   284,   285,
     286,     0,     0,     0,     0,     0,   108,   350,     0,     0,
       0,  1974,     0,     0,     0,   287,   288,     0,   289,     0,
       0,   216,   217,   218,   219,   220,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   189,   290,     0,    91,     0,     0,    93,
      94,     0,    95,   190,    97,   291,     0,     0,   292,   293,
     294,   295,   296,   297,   298,     0,     0,     0,   212,     0,
       0,     0,     0,     0,   299,     0,     0,     0,   108,     0,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,    50,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,     0,   335,     0,   336,
     337,   338,   339,     0,     0,     0,   340,   341,   216,   217,
     218,   219,   220,   342,     0,     0,     0,     0,     0,     0,
     212,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     343,     0,     0,    91,   344,     0,    93,    94,     0,    95,
     190,    97,   345,    50,   346,     0,     0,   347,     0,   279,
     280,     0,   281,   282,   348,   349,   283,   284,   285,   286,
       0,     0,     0,     0,     0,   108,   350,     0,     0,     0,
       0,     0,     0,     0,   287,   288,     0,   289,     0,     0,
     216,   217,   218,   219,   220,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   189,   290,     0,    91,    92,     0,    93,    94,
       0,    95,   190,    97,   291,     0,     0,   292,   293,   294,
     295,   296,   297,   298,     0,     0,     0,   212,     0,     0,
       0,     0,     0,   299,     0,     0,     0,   108,     0,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
      50,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,     0,   335,     0,     0,   337,
     338,   339,     0,     0,     0,   340,   341,   216,   217,   218,
     219,   220,   342,     0,     0,     0,     0,     0,     0,   212,
       0,     0,     0,     0,     0,     0,     0,     0,   212,   343,
       0,     0,    91,   344,     0,    93,    94,     0,    95,   190,
      97,   345,    50,   346,     0,     0,   347,     0,     0,     0,
       0,    50,     0,   348,   349,  1688,     0,     0,     0,   279,
     280,     0,   281,   282,   108,   350,   283,   284,   285,   286,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   216,
     217,   218,   219,   220,   287,   288,     0,   289,   216,   217,
     218,   219,   220,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   924,     0,     0,    93,    94,     0,
      95,   190,    97,   290,     0,     0,    93,    94,     0,    95,
     190,    97,     0,     0,   291,     0,     0,   292,   293,   294,
     295,   296,   297,   298,     0,     0,   108,   212,     0,     0,
       0,     0,     0,   299,     0,   108,     0,     0,     0,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
      50,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,     0,   335,     0,     0,   337,
     338,   339,     0,     0,     0,   340,   341,   216,   217,   218,
     219,   220,   342,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   343,
       0,     0,    91,   344,     0,    93,    94,     0,    95,   190,
      97,   345,     0,   346,     0,     0,   347,     0,  1791,  1792,
    1793,  1794,  1795,   348,   349,  1796,  1797,  1798,  1799,     0,
       0,     0,     0,     0,   108,   350,     0,     0,     0,     0,
       0,     0,  1800,  1801,  1802,     0,   477,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,  1803,   502,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   503,  1804,  1805,  1806,  1807,
    1808,  1809,  1810,     0,     0,     0,   212,     0,     0,     0,
       0,     0,  1811,     0,     0,     0,     0,     0,  1812,  1813,
    1814,  1815,  1816,  1817,  1818,  1819,  1820,  1821,  1822,    50,
    1823,  1824,  1825,  1826,  1827,  1828,  1829,  1830,  1831,  1832,
    1833,  1834,  1835,  1836,  1837,  1838,  1839,  1840,  1841,  1842,
    1843,  1844,  1845,  1846,  1847,  1848,  1849,  1850,  1851,  1852,
    1853,     0,     0,     0,  1854,  1855,   216,   217,   218,   219,
     220,     0,  1856,  1857,  1858,  1859,  1860,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1861,  1862,
    1863,     0,   212,     0,    93,    94,     0,    95,   190,    97,
    1864,     0,  1865,  1866,     0,  1867,     0,     0,     0,     0,
       0,     0,  1868,     0,  1869,    50,  1870,     0,  1871,  1872,
       0,   279,   280,   108,   281,   282,     0,     0,   283,   284,
     285,   286,     0,     0,     0,     0,     0,     0,  1697,     0,
       0,     0,     0,     0,     0,     0,   287,   288,     0,     0,
       0,  1698,   216,   217,   218,   219,   220,  1699,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   189,   290,     0,    91,    92,     0,
      93,    94,     0,    95,  1701,    97,     0,     0,     0,   292,
     293,   294,   295,   296,   297,   298,     0,     0,     0,   212,
       0,     0,     0,     0,     0,   299,     0,     0,     0,   108,
       0,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,    50,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,     0,   335,     0,
     336,   337,   338,   339,     0,     0,     0,   340,   600,   216,
     217,   218,   219,   220,   601,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   279,   280,     0,   281,   282,
       0,   602,   283,   284,   285,   286,     0,    93,    94,     0,
      95,   190,    97,   345,     0,   346,     0,     0,   347,     0,
     287,   288,     0,     0,     0,     0,   349,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   108,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   290,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   292,   293,   294,   295,   296,   297,   298,
       0,     0,     0,   212,     0,     0,     0,     0,     0,   299,
       0,     0,     0,     0,     0,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,    50,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,     0,   335,     0,  1380,   337,   338,   339,     0,     0,
       0,   340,   600,   216,   217,   218,   219,   220,   601,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   279,
     280,     0,   281,   282,     0,   602,   283,   284,   285,   286,
       0,    93,    94,     0,    95,   190,    97,   345,     0,   346,
       0,     0,   347,     0,   287,   288,     0,     0,     0,     0,
     349,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     108,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   290,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   292,   293,   294,
     295,   296,   297,   298,     0,     0,     0,   212,     0,     0,
       0,     0,     0,   299,     0,     0,     0,     0,     0,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
      50,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,     0,   335,     0,     0,   337,
     338,   339,     0,     0,     0,   340,   600,   216,   217,   218,
     219,   220,   601,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   602,
       0,     0,     0,     0,     0,    93,    94,     0,    95,   190,
      97,   345,     0,   346,     0,     0,   347,   474,   475,   476,
       0,     0,     0,     0,   349,     0,     0,     0,     0,     0,
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
     502,   474,   475,   476,     0,     0,     0,     0,     0,     0,
       0,     0,   503,     0,     0,     0,     0,     0,     0,     0,
       0,   477,   478,  1530,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
    1607,     0,   474,   475,   476,     0,     0,     0,     0,     0,
     503,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,  1732,   502,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,     0,     0,     0,     0,     0,     0,     0,
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,  1733,   502,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   503,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     477,   478,  1531,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,     0,     0,
       0,   474,   475,   476,     0,     0,     0,     0,     0,   503,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   477,   478,   504,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,   474,
     475,   476,     0,     0,     0,     0,     0,     0,     0,     0,
     503,   289,     0,     0,     0,     0,     0,     0,     0,   477,
     478,   586,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,     0,   291,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   503,     0,
       0,   212,     0,     0,     0,     0,     0,  1460,     0,   474,
     475,   476,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,   477,
     478,   588,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,     0,     0,     0,
     589,   216,   217,   218,   219,   220,   590,     0,   503,     0,
       0,     0,     0,     0,     0,     0,     0,  1388,     0,     0,
       0,     0,   607,   189,     0,     0,    91,   344,     0,    93,
      94,     0,    95,   190,    97,   880,   881,     0,     0,     0,
       0,   882,     0,   883,     0,     0,     0,   348,     0,     0,
       0,     0,     0,     0,     0,   884,     0,     0,   108,   350,
       0,     0,     0,    34,    35,    36,   212,     0,     0,     0,
     611,     0,   474,   475,   476,     0,     0,   214,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
       0,     0,     0,     0,     0,   885,   886,   887,   888,   889,
     890,   503,    81,    82,    83,    84,    85,     0,     0,   850,
       0,     0,     0,   221,     0,  1115,     0,     0,   189,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   190,    97,
       0,     0,     0,    99,     0,     0,     0,     0,     0,     0,
       0,     0,   891,   892,     0,     0,     0,    29,   105,     0,
       0,     0,     0,   108,   893,    34,    35,    36,   212,     0,
     213,    40,     0,     0,     0,     0,     0,     0,     0,   214,
     547,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   215,     0,
       0,     0,     0,   880,   881,     0,     0,     0,     0,   882,
       0,   883,     0,     0,     0,     0,  1116,    75,   216,   217,
     218,   219,   220,   884,    81,    82,    83,    84,    85,     0,
       0,    34,    35,    36,   212,   221,     0,     0,     0,     0,
     189,    89,    90,    91,    92,   214,    93,    94,     0,    95,
     190,    97,     0,     0,     0,    99,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     105,     0,     0,     0,     0,   108,   222,     0,     0,     0,
       0,     0,   112,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   885,   886,   887,   888,   889,   890,    29,
      81,    82,    83,    84,    85,     0,     0,    34,    35,    36,
     212,   221,   213,    40,     0,     0,   189,    89,    90,    91,
      92,   214,    93,    94,     0,    95,   190,    97,     0,     0,
       0,    99,     0,    50,     0,     0,     0,     0,     0,     0,
     891,   892,     0,     0,     0,     0,   105,     0,     0,     0,
     215,   108,   893,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
     216,   217,   218,   219,   220,    29,    81,    82,    83,    84,
      85,     0,     0,    34,    35,    36,   212,   221,   213,    40,
       0,     0,   189,    89,    90,    91,    92,   214,    93,    94,
       0,    95,   190,    97,     0,     0,     0,    99,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   105,     0,     0,     0,   215,   108,   222,     0,
       0,   627,     0,     0,   112,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   647,    75,   216,   217,   218,   219,
     220,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,   221,     0,     0,     0,     0,   189,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   190,    97,
      29,  1057,     0,    99,     0,     0,     0,     0,    34,    35,
      36,   212,     0,   213,    40,     0,     0,     0,   105,     0,
       0,     0,   214,   108,   222,     0,     0,     0,     0,     0,
     112,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   215,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,
      75,   216,   217,   218,   219,   220,    29,    81,    82,    83,
      84,    85,     0,  1187,    34,    35,    36,   212,   221,   213,
      40,     0,     0,   189,    89,    90,    91,    92,   214,    93,
      94,     0,    95,   190,    97,     0,     0,     0,    99,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   105,     0,     0,     0,   215,   108,   222,
       0,     0,     0,     0,     0,   112,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1217,    75,   216,   217,   218,
     219,   220,    29,    81,    82,    83,    84,    85,     0,     0,
      34,    35,    36,   212,   221,   213,    40,     0,     0,   189,
      89,    90,    91,    92,   214,    93,    94,     0,    95,   190,
      97,     0,     0,     0,    99,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   105,
       0,     0,     0,   215,   108,   222,     0,     0,     0,     0,
       0,   112,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,   216,   217,   218,   219,   220,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
     221,     0,     0,     0,     0,   189,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   190,    97,     0,     0,     0,
      99,     0,     0,     0,     0,   474,   475,   476,     0,     0,
       0,     0,     0,     0,     0,   105,     0,     0,     0,     0,
     108,   222,     0,     0,     0,   477,   478,   112,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,   474,   475,   476,     0,     0,     0,     0,
       0,     0,     0,     0,   503,     0,     0,     0,     0,     0,
       0,     0,     0,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   503,     0,   474,   475,   476,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   556,   477,   478,     0,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,     0,     0,     0,     0,
       0,   956,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,   474,   475,   476,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1043,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,   474,   475,   476,     0,     0,     0,     0,     0,     0,
       0,     0,   503,     0,     0,     0,     0,     0,     0,     0,
    1099,   477,   478,     0,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     503,  1161,  1162,  1163,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1435,  1164,     0,     0,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,  1161,  1162,  1163,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1187,     0,     0,     0,     0,     0,     0,     0,  1164,  1478,
       0,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1187,  1161,  1162,  1163,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1164,  1361,
       0,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,  1161,  1162,  1163,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1187,     0,     0,     0,
       0,     0,     0,     0,  1164,  1549,     0,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1187,  1161,  1162,  1163,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1164,  1561,     0,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1161,
    1162,  1163,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1187,     0,     0,     0,     0,     0,     0,     0,
    1164,  1674,     0,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,
    1182,  1183,  1184,  1185,  1186,    34,    35,    36,   212,     0,
     213,    40,     0,     0,     0,     0,     0,     0,  1187,   214,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1772,     0,     0,     0,     0,     0,     0,   243,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   244,
       0,     0,     0,     0,     0,     0,     0,     0,   216,   217,
     218,   219,   220,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   221,     0,  1774,     0,     0,
     189,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     190,    97,     0,     0,     0,    99,     0,    34,    35,    36,
     212,     0,   213,    40,     0,     0,     0,     0,     0,     0,
     105,   678,     0,     0,     0,   108,   245,     0,     0,     0,
       0,     0,   112,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     215,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
     216,   217,   218,   219,   220,     0,    81,    82,    83,    84,
      85,   503,     0,    34,    35,    36,   212,   221,   213,    40,
       0,     0,   189,    89,    90,    91,    92,   214,    93,    94,
       0,    95,   190,    97,     0,     0,     0,    99,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   105,     0,     0,     0,   243,   108,   679,     0,
       0,     0,     0,     0,   112,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   216,   217,   218,   219,
     220,     0,    81,    82,    83,    84,    85,   212,     0,     0,
       0,     0,     0,   221,     0,     0,     0,     0,   189,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   190,    97,
      50,     0,     0,    99,     0,     0,     0,     0,   367,   368,
       0,     0,     0,     0,     0,     0,     0,     0,   105,     0,
       0,     0,     0,   108,   245,     0,     0,     0,     0,     0,
     112,     0,     0,     0,     0,     0,     0,   216,   217,   218,
     219,   220,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   369,
       0,     0,   370,     0,     0,    93,    94,     0,    95,   190,
      97,     0,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   371,     0,     0,     0,   862,
       0,     0,   477,   478,   108,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   474,   475,   476,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   863,   477,   478,  1040,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,   474,   475,   476,     0,     0,     0,
       0,     0,     0,     0,     0,   503,     0,     0,     0,     0,
       0,     0,     0,     0,   477,   478,     0,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,  1161,  1162,  1163,     0,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1164,  1566,     0,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1161,  1162,
    1163,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1187,     0,     0,     0,     0,     0,     0,     0,  1164,
       0,     0,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,  1186,   475,   476,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1187,     0,     0,
       0,     0,     0,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,  1162,  1163,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   503,     0,     0,     0,     0,     0,     0,     0,
       0,  1164,     0,     0,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,   476,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1187,
       0,     0,     0,     0,   477,   478,     0,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
    1163,   502,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,     0,     0,     0,  1164,
       0,     0,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,  1186,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   478,  1187,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1164,     0,   503,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1187,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   503,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1187,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   503,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     503,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1187, -1137, -1137, -1137, -1137,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   503, -1137, -1137, -1137, -1137,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1187
};

static const yytype_int16 yycheck[] =
{
       5,     6,   132,     8,     9,    10,    11,    12,    13,    56,
      15,    16,    17,    18,   193,   696,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    56,    31,     4,   109,   167,
     417,   124,     4,    44,   572,   734,   109,    19,    20,    44,
      98,   109,   723,   853,   997,   103,   104,    52,    57,    54,
      33,   417,    57,     4,    59,   417,     4,   172,   173,   417,
       4,   124,   124,    46,   194,    30,   246,   692,    51,   791,
     608,   999,    30,   131,    30,    86,   560,    30,   861,   878,
     167,    86,   505,   506,   502,   837,  1427,   827,   693,   868,
    1034,   109,   987,   448,    57,   541,   257,   537,   538,  1114,
       4,   621,   622,  1228,   109,   550,   671,     9,  1052,  1019,
     191,  1021,     9,  1911,   534,     4,   539,    14,   191,     9,
      32,   534,     9,   191,     9,     9,   566,    14,     9,    48,
      14,     9,    32,     9,     9,    48,    54,   450,   451,   452,
       9,   222,   124,   258,     9,     9,     9,     9,   568,   222,
       9,     9,    83,    70,   222,   568,  1100,  1110,     9,     9,
       9,     9,    70,     9,   245,     9,     9,    36,     9,     9,
    1718,     9,   245,   116,     9,   827,   103,     9,   565,  1727,
      83,   367,   368,   369,    54,   371,   191,    83,    48,    83,
     136,   137,    91,   198,   162,    50,    51,   183,   162,    91,
     107,   108,  1001,   136,   137,   162,   136,   137,   653,   654,
      57,    48,   103,   199,    48,   183,   183,   222,   199,   136,
     137,     0,    69,  2055,   103,   104,   183,    38,   159,   160,
     161,    38,   199,    38,   202,    38,   681,   180,   202,   166,
     245,   402,    70,   132,    70,   926,  2078,    38,    70,   167,
      70,    70,    70,   199,   162,   260,    70,   239,   263,   270,
     159,  1156,    38,   167,   182,   270,   271,   159,   162,    70,
     203,   716,    83,   176,    70,   166,    83,   196,    83,  2077,
      83,   200,   461,   196,   202,   202,   183,   264,    70,  1579,
     202,   268,    83,  2091,   202,    70,   203,   167,   997,   201,
     202,   201,    70,  1055,   201,   194,    70,    83,   389,   205,
     200,   358,    70,    70,   201,   200,   389,   201,    70,   200,
     858,   389,  1337,   201,   112,   201,   201,   358,  1068,   184,
    1029,   200,  1242,    70,   162,   200,   781,   201,   201,   201,
     200,     8,   201,   201,  1685,  1124,  1290,  1126,  1896,   200,
    1475,   201,   201,   201,   359,   201,   200,  1482,   201,  1484,
     201,   201,   200,   200,   543,   200,   200,   448,   200,   176,
      83,   176,   199,   176,   202,   448,   202,   388,   136,   137,
     448,   199,   202,   388,   389,   176,   516,   168,   202,  1514,
     395,   396,   397,   398,   399,   400,   378,   455,   162,   446,
     405,   202,   922,   923,  1694,   387,   202,   389,    83,    84,
     975,  1110,   394,   418,   162,   446,  1068,   199,   200,  1314,
     425,   202,   199,    70,   406,   870,  1031,   202,   433,  1719,
    1720,   876,  1722,    70,   202,   199,    70,   199,   202,   448,
     445,   199,   199,   448,   202,   202,  1245,   199,    70,    70,
     202,   199,   424,   511,   512,   513,   514,   123,   463,   464,
     199,   508,   199,   176,   199,   202,   132,    83,    84,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,  1238,   503,   517,
     505,   506,   507,   948,  1226,   202,   952,   554,   517,    70,
      83,  1636,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,    83,    14,   202,   432,   199,
     205,    83,   537,   538,   539,   540,   103,   502,   699,   544,
     701,     4,   547,   432,   502,    70,   502,   183,   183,   502,
    1333,   556,   199,   558,   517,   202,    70,  1356,   199,   541,
      60,   566,   199,   199,   199,   574,  1306,  1307,   202,   574,
    1310,   576,   710,   103,    83,   136,   137,  1317,   199,    81,
     202,   202,    91,  1121,   122,  1005,    83,    84,    88,   205,
      53,    91,  1005,    56,   132,   107,   108,   199,   679,   166,
     987,   162,   579,   176,   183,   785,   199,   565,    87,  1049,
      73,   136,   137,   162,   793,   103,   104,   166,   176,  1244,
     199,   987,   627,   710,    83,   987,   515,  1968,   199,   987,
     132,   199,    91,    96,   183,    98,   166,   817,   199,  1057,
     103,   104,    87,   195,    31,   124,   125,   126,   162,  1094,
     202,   160,   161,    70,  1306,  1307,  1308,  1309,  1310,  1311,
      70,  1460,  1557,  1315,  1019,  1317,  1021,    31,   131,  1274,
     831,   832,  1277,  1973,   679,  1975,    70,   838,   839,   124,
     125,   126,  1202,  1103,   742,   199,    50,    83,   202,    53,
    1103,   203,    70,  1138,    81,    91,    70,  1142,   162,   208,
     167,   160,   161,   199,  1149,    92,   166,  1020,    19,    20,
    1023,  1024,   199,    14,   894,    83,   721,   104,    32,   183,
     508,   693,   166,   838,     4,    83,   912,   907,   914,   199,
     916,    32,  1473,    91,   920,   199,   168,   719,   202,    19,
      20,   201,   202,   199,   207,   750,   534,   118,   207,   181,
      51,  1491,    31,  1493,   141,   142,   143,   144,   145,  1654,
      83,  1483,   201,   159,   160,   161,   554,    38,    91,  1156,
     202,    50,    50,    51,    53,   201,   163,   565,   783,   166,
     568,   201,   169,   170,    70,   172,   173,   174,   201,   176,
    1156,   202,   160,   161,  1156,   398,   201,   400,  1156,    75,
      76,   264,   160,   161,   201,   268,   710,   812,  1292,   272,
     112,   198,   794,    75,    76,   418,    53,    54,    55,   121,
     122,   123,   124,   125,   126,   201,  1451,   134,   135,   109,
     734,  1153,    69,  1155,  1257,  1157,   159,   160,   161,  1491,
     845,  1493,  1309,  1283,  1311,   734,   201,    83,  1315,    70,
     861,   833,    70,   835,  1294,    91,   861,    70,    83,  1304,
     121,   122,   123,   124,   125,   126,    91,   367,   368,   369,
     370,   371,  1477,    70,   857,    70,  1598,  1415,    53,    54,
      55,   863,    57,    70,  1625,    70,  1627,  1242,   162,   866,
    1336,   199,   194,   199,    69,   358,    50,    51,    52,    53,
      54,    55,    70,  1643,   162,  1645,   199,  1647,    49,   409,
     166,   191,  1652,   201,  1613,    69,   121,   122,   123,   124,
     125,   126,  1367,    69,   160,   161,  1371,  1314,   239,   201,
     202,  1376,   183,   194,   162,   160,   161,   201,   202,  1384,
     201,   204,   222,   990,   106,   107,   108,   851,  1314,  1942,
    1943,   956,  1314,   958,   936,   960,  1314,  1938,  1939,   239,
    1004,  1005,   851,     9,   199,   245,   971,   199,   162,   432,
     952,   953,   162,  1695,   106,   107,   108,   199,  1516,   442,
       8,   986,   201,   446,   264,  1431,   162,   199,   268,   194,
      14,  1643,   455,  1645,  1532,  1647,   162,   974,  1056,   201,
    1652,   201,   974,  1744,   202,   909,     9,  1748,   201,    14,
    1019,  1016,  1021,   132,  1019,  1755,  1021,  1026,   132,   200,
     909,  1026,   183,   974,    14,   103,   974,   999,   200,   200,
     974,   200,  1637,   200,   199,  1040,   206,   112,  1043,  1479,
    1045,   113,   114,   115,  1049,   508,   509,   510,   511,   512,
     513,   514,   403,  1498,   202,   199,   407,  1502,     9,  1031,
    2013,   199,   159,   200,  1509,  2018,   200,   378,    95,   200,
     974,   534,  1252,   200,     9,  1255,   387,   367,   368,   369,
     201,   371,    14,   394,   435,   974,   437,   438,   439,   440,
    2043,   554,  1057,   997,  1099,   406,   183,  1452,   378,  1057,
     199,  1057,     9,  1755,  1057,   568,  2034,   387,   997,   389,
     199,   202,  1159,   201,   394,   201,   579,    19,    20,   202,
    1658,   621,   622,   202,  1107,  1029,   406,   201,  2056,  1667,
     201,  1108,  1114,  1115,   202,    83,   599,  2065,   200,   200,
    1029,   200,  1455,  1456,  1457,  1683,   201,   134,   199,  1242,
     200,  2104,   432,     9,     9,  1965,   204,   204,    70,   204,
    1970,   204,   625,   626,   204,    32,   135,   182,   448,  1910,
    1557,   162,   138,  1914,     9,   200,  1916,  1917,   162,  1242,
    1242,  1621,  1622,   200,    14,    50,    51,    52,    53,    54,
      55,  1557,    57,     9,   196,  1557,     9,   660,   661,  1557,
     200,  1219,   990,   184,    69,     9,  1110,    14,  1112,  2019,
    1219,     9,   200,   134,  1219,   200,  1004,  1005,   200,     9,
     200,  1110,    14,  1112,   203,  1230,   204,  1765,   204,   204,
     541,   204,   200,  1242,   162,   200,   199,  1242,   200,   103,
     201,   201,     9,   138,   162,     9,   200,   199,  1225,    70,
      70,    70,  1257,  1225,   199,  1260,    70,   199,    70,   202,
    1242,   541,     9,   203,  1916,  1917,    14,  1654,  2067,   184,
      81,   734,  2071,     9,  1225,   201,  1258,  1225,  1283,   742,
     202,  1225,    14,  1964,  2094,  1966,  2085,  2086,  1654,  1294,
    1295,   204,  1654,   104,   202,   176,  1654,    14,   201,   579,
     200,    32,  1274,   196,    70,  1277,    32,   199,   199,    14,
     199,   199,    14,  1101,  2013,  1103,   199,    52,    70,  2018,
      70,  1225,  1333,  2064,    70,    70,    70,   199,  1333,   199,
     141,   142,   143,   144,   145,   162,  1225,   239,  1343,  1321,
    2081,  1456,  1325,     9,  2043,   138,   200,    14,   201,   201,
     199,  1328,   184,   138,  1336,  1337,   167,   162,   169,   170,
       9,   172,   173,   174,   827,    69,   829,   200,   176,     9,
     176,  1452,    83,  2054,     9,   204,   199,   203,   203,  1452,
    1918,   203,   138,    83,  1452,   203,   201,   198,   851,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   865,   866,   199,  2104,    14,   201,   719,    83,
     201,   200,   912,   199,   914,    69,   916,   199,   202,   200,
     920,   204,   922,   923,   924,   202,   202,     9,   138,    92,
    1435,   159,   202,    32,    77,   200,  1340,  1442,   201,   719,
     201,   184,  1447,  1452,  1449,   138,   909,  1452,    32,  1431,
     204,  1340,     9,   204,   734,   918,   919,  1429,   200,   200,
    1465,  1599,     9,  1593,   204,   204,   204,  1439,    81,   138,
      83,     9,    85,  1478,  1479,   200,   378,   200,     9,   203,
     200,   203,   201,   794,    81,   387,   949,  1932,  1933,   201,
      14,   104,   394,   201,   201,    83,   202,     9,   201,   138,
     199,   199,   199,   204,   406,  1477,   200,   104,   200,   200,
     200,   974,   201,   200,   794,   417,   202,     9,   138,   204,
     204,   204,   833,   204,   835,  1429,   204,   990,   141,   142,
     143,   144,   145,     9,   997,  1439,   200,    32,  1717,   200,
     200,  1004,  1005,   201,   141,   142,   143,   144,   145,   200,
     176,   201,   863,   833,  2092,   835,   169,   170,   138,   172,
     173,   174,   113,   201,   113,   202,  1029,   113,   113,   171,
     167,   851,   169,   170,   171,   172,   173,   174,    83,   113,
     201,   167,    83,   863,    14,   198,   866,    83,   200,   119,
     200,   202,    14,  1056,   138,   200,   138,  1727,   183,   202,
     201,   198,   199,  1066,  1067,  1068,    14,    14,  1613,    14,
      83,   200,   199,  1618,    83,   198,  1621,  1622,   138,   200,
      78,    79,    80,   200,   138,   936,   201,     6,   201,   909,
    1688,    14,    14,   201,    92,   202,    14,     9,  1101,   541,
    1103,   952,   953,     9,  1691,  1108,    14,  1110,   203,  1112,
      68,   183,    83,     9,  1626,     9,   936,   116,   199,   202,
    1632,   201,  1634,   103,  1568,  1637,   103,   184,   162,    48,
    1133,   174,   952,   953,    36,   200,   199,   199,   180,  1568,
     201,    83,   200,     9,   184,  1657,   184,  1664,   177,   147,
     148,   149,   150,   151,   974,  1599,  1159,    83,    83,    14,
     158,   200,  1202,  1592,   201,    14,   164,   165,    83,  1613,
     200,   202,    83,    83,    14,  1894,    83,   997,    14,    83,
     178,  1202,  1626,  2046,  1613,  1188,   514,   977,  1632,   509,
    1634,  1050,   511,  1738,   113,   193,  2061,  1334,  1764,  1019,
     119,  1021,   121,   122,   123,   124,   125,   126,   127,  1029,
    2056,  1529,  1259,  1657,   629,  1659,   121,   122,   123,   124,
     125,   126,  1225,  1751,  1668,  1789,  1876,   132,   133,  2102,
    1659,  1696,  1588,  2078,    56,  1581,  1747,   399,  1310,  1668,
    1229,  1753,  1154,  1150,  1067,  1305,  1763,  1764,  1306,   395,
     169,   170,  1096,   172,   446,   879,  1956,  2014,  2003,  1573,
    1134,  1210,  1188,  1114,  1115,    -1,    -1,    -1,    -1,    -1,
     175,    -1,    -1,    -1,    -1,   194,    -1,   719,    -1,    -1,
      -1,    -1,    -1,    -1,   203,    -1,    -1,    -1,  1108,   194,
    1110,    -1,  1112,    -1,  1114,  1115,    -1,  1726,    -1,    -1,
      -1,  1888,    -1,  1306,  1307,  1308,  1309,  1310,  1311,  1753,
      -1,    81,  1315,    -1,  1317,    -1,     6,  1888,  1762,    -1,
      -1,    -1,    -1,    -1,  1768,  1328,    -1,    -1,    -1,    -1,
      -1,  1775,    -1,  1762,   104,    -1,     6,  1340,    -1,  1768,
      -1,    -1,   112,   113,    -1,    -1,  1775,  1350,    -1,  2027,
      -1,    -1,   794,    -1,    -1,    -1,  1901,    -1,    48,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1956,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    48,    -1,
    2050,    -1,    -1,    -1,    -1,  1956,    -1,    -1,    -1,    -1,
      -1,   833,    -1,   835,    -1,    -1,   166,    -1,    -1,   169,
     170,    -1,   172,   173,   174,  1225,    -1,  1258,    19,    20,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
    1423,   863,  1242,   113,    -1,    -1,    -1,    -1,   198,   119,
      -1,   121,   122,   123,   124,   125,   126,   127,  1258,    -1,
      -1,     6,    -1,   113,    -1,    56,    -1,    -1,    -1,   119,
      -1,   121,   122,   123,   124,   125,   126,   127,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   289,    -1,   291,
    1321,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1922,   169,
     170,    -1,   172,    48,    -1,  1336,  1337,    -1,  1491,    -1,
    1493,    -1,    -1,  1922,   936,    -1,    -1,    -1,    -1,   169,
     170,  1321,   172,    -1,   194,    -1,    -1,    -1,  1328,    -1,
     952,   953,    -1,   203,    -1,    -1,  1336,  1337,    -1,    -1,
    1340,    -1,  2034,    -1,   194,    -1,    -1,    -1,   350,    -1,
      -1,    -1,    -1,   203,  1978,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  2056,   987,    -1,    -1,   113,  1978,
      -1,    -1,    -1,  2065,   119,  2100,   121,   122,   123,   124,
     125,   126,   127,    -1,  2109,  1568,    -1,    -1,    -1,  2013,
      -1,    -1,  2117,    -1,  2018,  2120,  1579,    -1,    -1,    -1,
    1431,    -1,    -1,  2027,  2013,  1588,    -1,    -1,    -1,  2018,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2043,
      -1,    -1,    -1,    -1,   169,   170,    -1,   172,    -1,    -1,
    1613,  1431,    -1,    -1,  2043,     6,    -1,    -1,    -1,    -1,
      -1,   443,    -1,    -1,   446,    -1,    -1,    -1,   239,   194,
      -1,    -1,  1452,    -1,    -1,    -1,    -1,    -1,   203,    -1,
    1643,    -1,  1645,    -1,  1647,    -1,    -1,    -1,    -1,  1652,
      -1,    -1,    -1,    -1,    -1,    -1,  1659,    48,    -1,  2103,
    2104,  1664,    -1,    -1,    -1,  1668,    -1,    -1,    -1,    -1,
      -1,    -1,  1114,  1115,  2103,  2104,    -1,    -1,   289,    -1,
     291,    -1,    -1,    -1,    -1,  1688,    -1,    -1,  1691,    -1,
      -1,  1694,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1704,    -1,    -1,    -1,    -1,    -1,    -1,  1711,    -1,
      -1,    -1,    -1,    -1,  1156,    -1,  1719,  1720,    -1,  1722,
      -1,    -1,   113,    -1,    -1,    -1,  1729,    -1,   119,    -1,
     121,   122,   123,   124,   125,   126,   127,    -1,    -1,   350,
      -1,    -1,    -1,    -1,     6,    -1,    -1,    -1,  1568,    -1,
      -1,    -1,  1755,    -1,    -1,    -1,    -1,    -1,    -1,  1762,
    1763,  1764,    -1,    -1,    -1,  1768,    -1,   378,    -1,    -1,
      -1,   593,  1775,    -1,    -1,    -1,   387,    -1,   169,   170,
      -1,   172,    -1,   394,    -1,    -1,    48,    -1,    -1,    -1,
      -1,    -1,    -1,  1613,    -1,   406,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   194,    -1,     6,   417,    -1,    -1,    -1,
      -1,    -1,   203,    -1,    -1,    -1,  1258,    -1,    -1,   121,
     122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,    -1,
     132,   133,   443,    -1,    -1,   446,    -1,    -1,    -1,  1659,
      -1,    -1,    -1,    -1,  1664,    -1,    -1,    48,  1668,    -1,
      -1,   113,    -1,    -1,    -1,    -1,    -1,   119,    -1,   121,
     122,   123,   124,   125,   126,   127,    -1,   689,   690,    -1,
      -1,   173,  1314,   175,    -1,    -1,    -1,    -1,    -1,  1321,
      -1,    -1,    -1,    -1,    -1,  1888,    -1,   189,    -1,   191,
      -1,   502,   194,    -1,  1336,  1337,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,   170,  1912,
     172,    -1,   113,  1916,  1917,    -1,    -1,    -1,   119,  1922,
     121,   122,   123,   124,   125,   126,   127,    -1,  1931,    -1,
     541,    -1,   194,    -1,    -1,  1938,  1939,    -1,    -1,  1942,
    1943,   203,  1762,  1763,  1764,    -1,    -1,    -1,  1768,    -1,
      -1,    -1,    -1,  1956,    -1,  1775,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,   170,
      -1,   172,    -1,    -1,    -1,  1978,    -1,    -1,    -1,    -1,
      -1,    -1,   593,    -1,   595,    -1,    -1,   598,    -1,  1431,
      -1,    -1,    -1,   194,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    2013,    -1,    -1,    -1,    -1,  2018,    -1,    -1,    -1,    -1,
     631,    -1,    -1,  2026,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
    2043,    -1,    -1,    -1,    -1,    -1,  2049,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   878,   879,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,
      60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   689,   690,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   698,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
    2103,  2104,  1922,    -1,    -1,    -1,    -1,    -1,   719,    -1,
      -1,    -1,    -1,    30,    31,  1557,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    81,    -1,    83,    84,    -1,   136,   137,    -1,    -1,
      -1,    -1,    69,    -1,   976,    -1,    -1,    81,  1978,    83,
      84,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   994,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,   794,    -1,  1007,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  2013,    -1,    -1,    -1,    -1,  2018,    -1,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,
     200,    -1,  1654,    -1,    -1,    -1,   827,   141,   142,   143,
     144,   145,   833,  2043,   835,    -1,    -1,   167,    -1,   169,
     170,  1053,   172,   173,   174,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,   863,   864,    -1,    -1,    -1,    -1,   198,    -1,
     871,    -1,   202,    -1,    -1,   205,    -1,   878,   879,   880,
     881,   882,   883,   884,   198,    -1,    -1,    -1,   202,    -1,
      -1,   205,   893,  2103,  2104,    -1,    -1,   204,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   910,
      -1,    -1,    -1,    -1,    -1,    -1,  1128,    -1,    -1,    81,
    1132,    -1,    -1,    -1,    -1,  1137,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,   936,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,   950,
      31,   952,   953,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,   976,   977,    -1,    -1,   141,
     142,   143,   144,   145,    -1,    -1,   987,    -1,    69,    -1,
      -1,    -1,    -1,   994,    -1,    -1,    -1,    -1,    -1,    -1,
    1001,    -1,    -1,    -1,    19,    20,  1007,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    30,    -1,  1018,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      56,  1243,  1033,    -1,    -1,    -1,   198,   199,    -1,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,  1053,    57,    -1,    -1,  1057,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    69,    -1,  1068,    -1,    -1,
      -1,    -1,    -1,    -1,  1286,    -1,    -1,  1289,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1114,  1115,    -1,    -1,    69,    -1,   200,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1128,    -1,    -1,
      -1,  1132,    -1,  1134,    -1,    -1,  1137,  1349,    -1,    -1,
      -1,    -1,    -1,    -1,  1356,    -1,    -1,    -1,    -1,  1150,
    1151,  1152,  1153,  1154,  1155,  1156,  1157,    -1,    -1,  1160,
    1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,  1206,    81,    -1,    -1,    -1,
      -1,    -1,  1424,  1425,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   239,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    59,    60,    -1,    -1,    -1,    -1,
      -1,    -1,  1243,    -1,  1245,    -1,    -1,    -1,    -1,   201,
      -1,   203,    -1,    -1,    -1,    -1,    -1,  1258,    -1,    -1,
      -1,    -1,    -1,   289,    -1,   291,   141,   142,   143,   144,
     145,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,  1286,    -1,    -1,  1289,    -1,
      -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,    -1,    -1,  1306,  1307,  1308,  1309,  1310,
    1311,   136,   137,  1314,  1315,    -1,  1317,    59,    60,    -1,
    1321,    -1,    -1,   198,   350,  1537,  1538,   202,    -1,  1541,
      -1,    -1,    -1,    -1,    -1,  1336,  1337,    -1,  1339,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1349,    -1,
      -1,    -1,    -1,    -1,    -1,  1356,    -1,    -1,    -1,    -1,
    1361,    -1,  1363,   378,    -1,    -1,    -1,  1579,    -1,    -1,
      -1,    -1,   387,    -1,    -1,   200,    -1,    -1,    -1,   394,
      -1,  1593,    -1,    -1,    -1,    -1,    -1,  1388,    -1,    -1,
      -1,   406,    -1,    -1,   136,   137,    -1,    -1,    -1,    -1,
      -1,    -1,   417,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,   443,    -1,    -1,
     446,    -1,    -1,  1424,  1425,    -1,    -1,  1428,    30,    31,
    1431,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,  1669,   200,  1460,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,  1694,    -1,    -1,    -1,    -1,   502,    -1,    -1,
    1491,    -1,  1493,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1719,  1720,    -1,
    1722,    -1,    -1,    59,    60,  1727,    -1,  1729,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   541,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1537,  1538,    -1,    -1,
    1541,    -1,    -1,    -1,    -1,    -1,  1547,    -1,  1549,    -1,
    1551,    -1,    -1,    -1,    -1,  1556,  1557,    -1,    -1,    -1,
    1561,    -1,  1563,    -1,    -1,  1566,    -1,   593,    -1,   595,
      -1,    -1,    -1,    -1,    -1,  1787,    -1,    -1,  1579,  1580,
      -1,    -1,    -1,   598,    -1,  1586,    -1,    -1,    -1,    -1,
     136,   137,  1593,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   631,    -1,    -1,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1643,    -1,  1645,    -1,  1647,    81,    -1,    -1,
      -1,  1652,    -1,  1654,   200,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   689,   690,    59,    60,    -1,  1669,    -1,
     104,    -1,   698,  1674,    -1,    -1,    -1,    -1,   112,    -1,
      -1,  1893,    -1,    -1,    31,  1686,  1687,   121,   122,   123,
     124,   125,   126,  1694,    -1,  1696,  1908,    -1,    -1,    -1,
    1912,    -1,    -1,    -1,   719,    -1,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,  1719,  1720,
      -1,  1722,    -1,    -1,    -1,    -1,  1727,    -1,  1729,   163,
      -1,    -1,   166,   167,    81,   169,   170,    -1,   172,   173,
     174,    -1,   136,   137,    -1,    92,    -1,    81,    -1,    -1,
      -1,    -1,    -1,   187,  1755,    -1,    -1,   104,    -1,    -1,
     194,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,
     104,  1772,  1773,  1774,    -1,  1987,    -1,    -1,  1779,   794,
    1781,    -1,    -1,    -1,    -1,    -1,  1787,    -1,  1789,    -1,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,  2014,    -1,  2016,    -1,   200,   141,   142,   143,
     144,   145,   827,    -1,    -1,    -1,   163,    -1,   833,   166,
     835,    -1,   169,   170,    -1,   172,   173,   174,    -1,   176,
      -1,    -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,    -1,   871,    -1,    -1,   863,   864,
      -1,   198,   878,   879,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    20,    -1,   198,   880,   881,   882,   883,   884,
      -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,   893,    -1,
      -1,    -1,    -1,    -1,  1885,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1893,    -1,    -1,   910,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,  1908,    -1,    -1,
      -1,  1912,    -1,    -1,    -1,  1916,  1917,    -1,    -1,    -1,
      31,   936,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
    1931,    -1,    -1,    -1,    -1,   950,  1937,   952,   953,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     976,    -1,  1953,  1954,    -1,    -1,    -1,    68,    -1,  1960,
      -1,    -1,   977,   141,   142,   143,   144,   145,   994,    -1,
      81,    -1,   987,    -1,    -1,  1001,    87,    -1,    -1,    -1,
      -1,  1007,    -1,    -1,    -1,   163,  1987,    -1,   166,    -1,
      -1,   169,   170,   104,   172,   173,   174,    -1,    -1,    -1,
      -1,  2002,    -1,  1018,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  2014,    -1,  2016,    -1,    -1,  1033,    -1,
     198,    -1,    -1,    -1,    -1,   203,    -1,  1053,    -1,   140,
     141,   142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,
      -1,    -1,  1057,    -1,  2045,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   163,  1068,    -1,   166,   167,    -1,   169,   170,
    2061,   172,   173,   174,    -1,   176,  2067,    -1,    -1,    -1,
    2071,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,
      -1,   239,    -1,    -1,  2085,  2086,    -1,   198,   199,    -1,
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1114,
    1115,    -1,  1128,    -1,    -1,    -1,  1132,    -1,  1134,    -1,
      70,  1137,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    -1,    -1,  1150,  1151,  1152,  1153,  1154,
    1155,  1156,  1157,    31,   104,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,  1187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,
      -1,  1206,    -1,    81,    -1,    -1,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,   104,  1243,    -1,  1245,
     378,    -1,    -1,    -1,    31,    -1,    -1,   187,   188,   387,
      -1,    -1,    -1,    -1,    -1,    -1,   394,    -1,   198,   127,
      -1,    -1,    -1,  1258,    -1,    -1,    -1,    -1,   406,    -1,
      -1,    -1,   140,   141,   142,   143,   144,   145,   146,   417,
    1286,    68,    -1,  1289,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,   163,    83,    -1,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
      -1,  1306,  1307,  1308,  1309,  1310,  1311,   104,    -1,  1314,
    1315,    -1,  1317,    -1,    -1,    -1,  1321,    -1,    -1,    -1,
     198,    -1,    -1,    -1,   121,   122,   123,   124,   125,   126,
      -1,  1336,  1337,  1349,  1339,    -1,    -1,    -1,    -1,    -1,
    1356,    -1,    -1,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,   502,    -1,  1361,    -1,  1363,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   163,    31,    -1,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,    -1,    -1,  1388,    -1,    -1,    -1,    -1,    -1,    -1,
     187,    -1,    -1,   541,    -1,    -1,    -1,   194,    -1,    -1,
      -1,   198,   199,    -1,    68,    -1,    -1,    -1,  1424,  1425,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,  1428,    -1,    -1,  1431,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,  1460,    -1,    -1,    -1,    30,    31,
     598,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,   140,   141,   142,   143,
     144,   145,   146,   631,    -1,    -1,  1491,    69,  1493,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,
      -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      -1,  1537,  1538,   187,    -1,  1541,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,
      -1,    -1,  1547,    -1,  1549,    -1,  1551,    -1,    -1,    -1,
      -1,  1556,  1557,    68,    -1,    -1,  1561,    -1,  1563,   598,
      -1,  1566,    -1,  1579,    -1,    -1,    81,    -1,    -1,    -1,
      -1,   719,    87,    -1,    -1,  1580,    -1,  1593,    -1,    -1,
      -1,  1586,    -1,    -1,    -1,    -1,    -1,    31,    -1,   104,
      -1,    -1,   631,    -1,    -1,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   203,    -1,    -1,    68,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    81,  1643,    -1,
    1645,    -1,  1647,    59,    60,    -1,   794,  1652,   163,  1654,
      -1,   166,   167,  1669,   169,   170,    -1,   172,   173,   174,
     104,   176,    -1,    -1,    -1,    -1,    -1,    -1,   112,  1674,
      -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,  1694,    -1,
      -1,  1686,  1687,   198,   199,   833,    -1,   835,    -1,    -1,
      -1,  1696,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,  1719,  1720,    -1,  1722,    -1,    -1,    -1,
      -1,  1727,    -1,  1729,    -1,   863,   864,    -1,    -1,   163,
     136,   137,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,   880,   881,   882,   883,   884,    -1,    -1,    -1,
      -1,    -1,    -1,   187,    -1,   893,    31,    -1,    -1,    -1,
    1755,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1772,  1773,  1774,
      -1,  1787,    -1,    -1,  1779,    -1,  1781,    -1,    -1,    -1,
      -1,    -1,    -1,    68,  1789,    -1,    -1,    -1,   936,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,   952,   953,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,   104,
      -1,    -1,    -1,    -1,    -1,   864,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   987,
      -1,   880,   881,   882,   883,   884,    -1,    -1,    -1,    -1,
      -1,    59,    60,    -1,   893,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1018,    -1,    -1,    -1,    -1,    -1,    -1,  1893,   163,    -1,
    1885,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,  1908,    -1,    -1,    -1,  1912,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,    -1,  1057,
      -1,  1916,  1917,   198,   199,  1931,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,   137,
      -1,    -1,  1937,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1953,  1954,
      -1,    -1,    -1,    -1,    -1,  1960,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1114,  1115,    -1,    -1,
      -1,  1987,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1018,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,  2002,  2014,    -1,
    2016,    -1,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,
      69,    -1,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,
    2045,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  2067,    -1,    -1,    -1,  2071,  2061,    -1,  1206,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2085,
    2086,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
    1258,  1150,  1151,    -1,    -1,  1154,    -1,    -1,    69,    -1,
      -1,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1314,  1206,    -1,    -1,
      -1,    -1,    -1,  1321,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,  1336,  1337,
      -1,  1339,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,  1361,    57,  1363,    -1,    -1,    81,    59,
      60,    78,    79,    80,    81,    -1,    69,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1388,   104,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      30,    31,   203,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,   141,   142,
     143,   144,   145,  1431,   141,   142,   143,   144,   145,    69,
      -1,    -1,    -1,    -1,    -1,    -1,   136,   137,    -1,    -1,
    1339,    -1,    -1,    -1,    -1,    -1,   169,   170,    -1,   172,
     173,   174,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,    -1,  1361,    -1,  1363,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,
      -1,   198,    -1,    -1,    -1,    -1,    -1,    30,    31,  1388,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1547,
      -1,  1549,    -1,  1551,    -1,    -1,    -1,    -1,  1556,  1557,
      -1,    -1,    -1,  1561,    -1,  1563,    -1,    -1,  1566,    10,
      11,    12,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1654,    -1,  1547,    -1,
    1549,    -1,  1551,    -1,    -1,    -1,    -1,  1556,    -1,    -1,
     203,    57,  1561,    -1,  1563,    -1,  1674,  1566,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    83,    84,    -1,
      -1,    87,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    -1,   130,    -1,   132,   133,   134,   135,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
     146,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1772,  1773,  1774,   163,    -1,    -1,
      -1,  1779,    -1,   169,   170,  1674,   172,   173,   174,   175,
    1788,   177,    -1,    -1,   180,    -1,    -1,    10,    11,    12,
      -1,    -1,   188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   198,    -1,    -1,    -1,   202,    30,    31,   205,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1772,  1773,  1774,    -1,  1885,    30,    31,
    1779,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1937,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1953,  1954,    -1,    -1,    -1,
      30,    31,  1960,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
     203,  1989,    -1,    -1,    -1,    -1,  1885,    -1,    -1,    69,
      -1,    -1,    -1,    -1,  2002,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1937,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   203,    -1,    -1,  1953,  1954,    -1,    -1,    -1,    -1,
      48,  1960,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    -1,
      88,    -1,    -1,  2002,    92,    93,    94,    95,    -1,    97,
      -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,
     200,   109,   110,   111,   112,   113,   114,   115,    -1,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,   129,   130,   131,   132,   133,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,    -1,
     178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,
     188,   189,    -1,   191,    -1,   193,   194,   195,    -1,    -1,
     198,   199,    -1,   201,   202,    -1,    -1,   205,   206,   207,
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
     112,   113,   114,   115,    -1,   117,    -1,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,   131,
     132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,   188,   189,    -1,   191,
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
      -1,    -1,    -1,   109,   110,   111,   112,   113,   114,   115,
      -1,   117,    -1,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,   129,   130,   131,   132,   133,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,   155,
      -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,   175,
      -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,
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
     118,    -1,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,   129,   130,    -1,   132,   133,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,    -1,
     178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,
     188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,
     198,   199,    -1,   201,   202,    -1,    -1,   205,   206,   207,
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
      88,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
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
     198,   199,    -1,   201,   202,    -1,    -1,   205,   206,   207,
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
     102,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
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
      70,    71,    72,    73,    74,    -1,    -1,    77,    78,    79,
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
      -1,   201,   202,    -1,    -1,   205,   206,   207,   208,     3,
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
      -1,    99,   100,   101,    -1,    -1,   104,   105,    -1,    -1,
      -1,   109,   110,   111,   112,    -1,   114,   115,    -1,   117,
      -1,    -1,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,   129,   130,    -1,   132,   133,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,    -1,
     178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,
     188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,
     198,   199,    -1,   201,   202,    -1,    -1,   205,   206,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    98,    99,    -1,   101,
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
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    27,    -1,    29,    -1,    -1,    -1,
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
     193,   194,   195,    -1,    -1,   198,   199,    -1,   201,    -1,
      -1,    -1,   205,   206,   207,   208,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    87,    -1,    -1,    -1,    -1,    92,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,
     124,   125,   126,    -1,    -1,   129,   130,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,    -1,   147,   148,   149,   150,   151,    -1,    -1,
      -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,   176,    -1,   178,    -1,    -1,   181,     3,     4,
       5,     6,     7,   187,   188,    -1,    -1,    -1,    13,   193,
     194,   195,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,
      -1,   205,   206,   207,   208,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    92,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,   124,
     125,   126,    -1,    -1,   129,   130,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
      -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,   176,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,
     195,    -1,    -1,   198,   199,     3,     4,     5,     6,     7,
     205,   206,   207,   208,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     198,   199,    -1,    -1,   202,    -1,    -1,   205,   206,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,
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
      -1,   193,   194,   195,    -1,    -1,   198,   199,     3,     4,
       5,     6,     7,   205,   206,   207,   208,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
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
     205,   206,   207,   208,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,   109,    -1,    -1,   112,    -1,    -1,    -1,
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
     178,    -1,    -1,   181,     3,     4,     5,     6,     7,   187,
     188,    -1,    -1,    -1,    13,   193,   194,   195,    -1,    -1,
     198,   199,    -1,   201,    -1,    -1,    -1,   205,   206,   207,
     208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     199,    -1,   201,    -1,    -1,    -1,   205,   206,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,
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
     174,    -1,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,
      -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,
     194,   195,    -1,    -1,   198,   199,   200,    -1,    -1,    -1,
      -1,   205,   206,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     178,    -1,    -1,   181,     3,     4,     5,     6,     7,   187,
     188,    -1,    -1,    -1,    13,   193,   194,   195,    -1,    -1,
     198,   199,    -1,    -1,    -1,    -1,    -1,   205,   206,   207,
     208,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
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
     193,   194,   195,    -1,    -1,   198,   199,     3,     4,     5,
       6,     7,   205,   206,   207,   208,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   169,   170,    -1,   172,   173,   174,    -1,     3,     4,
     178,     6,     7,   181,    -1,    10,    11,    12,    13,   187,
     188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,
     198,   199,    -1,    28,    29,    -1,    -1,   205,   206,   207,
     208,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    83,    84,
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
      -1,   130,   203,    -1,    -1,    -1,    28,    29,    -1,    31,
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
      -1,   163,    92,    -1,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,   104,   177,    -1,    -1,   180,    -1,
       3,     4,    -1,     6,     7,   187,   188,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,
      -1,   203,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   163,    57,    -1,   166,    -1,    -1,   169,
     170,    -1,   172,   173,   174,    68,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,   198,    -1,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,    -1,   130,    -1,   132,
     133,   134,   135,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     163,    -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,   175,   104,   177,    -1,    -1,   180,    -1,     3,
       4,    -1,     6,     7,   187,   188,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,    -1,
     141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   163,    57,    -1,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    68,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    87,    -1,    -1,    -1,   198,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    -1,   130,    -1,    -1,   133,
     134,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,   163,
      -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,   175,   104,   177,    -1,    -1,   180,    -1,    -1,    -1,
      -1,   104,    -1,   187,   188,   189,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,   198,   199,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,    28,    29,    -1,    31,   141,   142,
     143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,
     172,   173,   174,    57,    -1,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    68,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,   198,    81,    -1,    -1,
      -1,    -1,    -1,    87,    -1,   198,    -1,    -1,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    -1,   130,    -1,    -1,   133,
     134,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,
      -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,   175,    -1,   177,    -1,    -1,   180,    -1,     3,     4,
       5,     6,     7,   187,   188,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    93,    94,
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
     122,   123,   124,   125,   126,   127,   128,    -1,   130,    -1,
     132,   133,   134,   135,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,   163,    10,    11,    12,    13,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,   177,    -1,    -1,   180,    -1,
      28,    29,    -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,
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
      57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
     203,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   203,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,   203,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,   201,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,   201,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,   201,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    30,
      31,   201,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,   146,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,
      -1,    -1,   201,   163,    -1,    -1,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,   198,   199,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,    -1,    -1,
     201,    -1,    10,    11,    12,    -1,    -1,    92,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,
     145,    69,   147,   148,   149,   150,   151,    -1,    -1,   200,
      -1,    -1,    -1,   158,    -1,    38,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,   178,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   187,   188,    -1,    -1,    -1,    70,   193,    -1,
      -1,    -1,    -1,   198,   199,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
     138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    70,   147,   148,   149,   150,   151,    -1,
      -1,    78,    79,    80,    81,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    92,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     193,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,
      -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   140,   141,   142,   143,   144,   145,    70,
     147,   148,   149,   150,   151,    -1,    -1,    78,    79,    80,
      81,   158,    83,    84,    -1,    -1,   163,   164,   165,   166,
     167,    92,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,   178,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
     187,   188,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,
     121,   198,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,   145,    70,   147,   148,   149,   150,
     151,    -1,    -1,    78,    79,    80,    81,   158,    83,    84,
      -1,    -1,   163,   164,   165,   166,   167,    92,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   193,    -1,    -1,    -1,   121,   198,   199,    -1,
      -1,   202,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
      -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      70,    71,    -1,   178,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,   193,    -1,
      -1,    -1,    92,   198,   199,    -1,    -1,    -1,    -1,    -1,
     205,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   121,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
     140,   141,   142,   143,   144,   145,    70,   147,   148,   149,
     150,   151,    -1,    69,    78,    79,    80,    81,   158,    83,
      84,    -1,    -1,   163,   164,   165,   166,   167,    92,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   193,    -1,    -1,    -1,   121,   198,   199,
      -1,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,    70,   147,   148,   149,   150,   151,    -1,    -1,
      78,    79,    80,    81,   158,    83,    84,    -1,    -1,   163,
     164,   165,   166,   167,    92,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,   178,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,
      -1,    -1,    -1,   121,   198,   199,    -1,    -1,    -1,    -1,
      -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
     178,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,
     198,   199,    -1,    -1,    -1,    30,    31,   205,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,    30,    31,    -1,    33,    34,    35,    36,    37,
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
      69,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,    31,    -1,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,   138,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,   138,
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
      51,    52,    53,    54,    55,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    69,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,    -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,
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
      12,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    69,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    69,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69
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
     477,   478,   492,   494,   496,   121,   122,   123,   139,   163,
     173,   199,   216,   257,   337,   359,   468,   359,   199,   359,
     359,   359,   359,   109,   359,   359,   454,   455,   359,   359,
     359,   359,    81,    83,    92,   121,   141,   142,   143,   144,
     145,   158,   199,   227,   379,   423,   426,   431,   468,   472,
     468,   359,   359,   359,   359,   359,   359,   359,   359,    38,
     359,   483,   484,   121,   132,   199,   227,   270,   423,   424,
     425,   427,   431,   465,   466,   467,   476,   480,   481,   359,
     199,   358,   428,   199,   358,   370,   348,   359,   238,   358,
     199,   199,   199,   358,   201,   359,   216,   201,   359,     3,
       4,     6,     7,    10,    11,    12,    13,    28,    29,    31,
      57,    68,    71,    72,    73,    74,    75,    76,    77,    87,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   130,   132,   133,   134,   135,
     139,   140,   146,   163,   167,   175,   177,   180,   187,   188,
     199,   216,   217,   218,   229,   497,   518,   519,   522,    27,
     201,   353,   355,   359,   202,   250,   359,   112,   113,   163,
     166,   189,   219,   220,   221,   222,   226,    83,   205,   305,
     306,    83,   307,   123,   132,   122,   132,   199,   199,   199,
     199,   216,   276,   500,   199,   199,    70,    70,    70,    70,
      70,   348,    83,    91,   159,   160,   161,   489,   490,   166,
     202,   226,   226,   216,   277,   500,   167,   199,   199,   500,
     500,    83,   195,   202,   371,    28,   347,   350,   359,   361,
     468,   473,   233,   202,   478,    91,   429,   489,    91,   489,
     489,    32,   166,   183,   501,   199,     9,   201,   199,   346,
     360,   469,   472,   118,    38,   256,   167,   275,   500,   121,
     194,   257,   338,    70,   202,   463,   201,   201,   201,   201,
     201,   201,   201,   201,    10,    11,    12,    30,    31,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    69,   201,    70,    70,   202,   162,   133,
     173,   175,   189,   191,   278,   336,   337,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      59,    60,   136,   137,   458,   463,   463,   199,   199,    70,
     202,   199,   256,   257,    14,   359,   201,   138,    49,   216,
     453,    91,   347,   361,   162,   468,   138,   204,     9,   438,
     271,   347,   361,   468,   501,   162,   199,   430,   458,   463,
     200,   359,    32,   236,     8,   372,     9,   201,   236,   237,
     348,   349,   359,   216,   290,   240,   201,   201,   201,   140,
     146,   522,   522,   183,   521,   199,   112,   522,    14,   162,
     140,   146,   163,   216,   218,   201,   201,   201,   251,   116,
     180,   201,   219,   221,   219,   221,   219,   221,   226,   219,
     221,   202,     9,   439,   201,   103,   166,   202,   468,     9,
     201,    14,     9,   201,   132,   132,   468,   493,   348,   347,
     361,   468,   472,   473,   200,   183,   268,   139,   468,   482,
     483,   359,   380,   381,   348,   404,   404,   380,   404,   201,
      70,   458,   159,   490,    82,   359,   468,    91,   159,   490,
     226,   215,   201,   202,   263,   273,   413,   415,    92,   199,
     373,   374,   376,   422,   426,   475,   477,   494,   404,    14,
     103,   495,   367,   368,   369,   300,   301,   456,   457,   200,
     200,   200,   200,   200,   203,   235,   236,   258,   265,   272,
     456,   359,   206,   207,   208,   216,   502,   503,   522,    38,
      87,   176,   303,   304,   359,   497,   247,   248,   347,   355,
     356,   359,   361,   468,   202,   249,   249,   249,   249,   199,
     500,   266,   256,   359,   479,   359,   359,   359,   359,   359,
      32,   359,   359,   359,   359,   359,   359,   359,   359,   359,
     359,   359,   359,   359,   359,   359,   359,   359,   359,   359,
     359,   359,   359,   359,   359,   427,   359,   479,   479,   359,
     485,   486,   132,   202,   217,   218,   478,   276,   216,   277,
     500,   500,   275,   257,    38,   350,   353,   355,   359,   359,
     359,   359,   359,   359,   359,   359,   359,   359,   359,   359,
     359,   167,   202,   216,   459,   460,   461,   462,   478,   303,
     303,   479,   359,   482,   256,   200,   359,   199,   452,     9,
     438,   200,   200,    38,   359,    38,   359,   430,   200,   200,
     200,   476,   477,   478,   303,   202,   216,   459,   460,   478,
     200,   233,   294,   202,   355,   359,   359,    95,    32,   236,
     288,   201,    27,   103,    14,     9,   200,    32,   202,   291,
     522,    31,    92,   176,   229,   515,   516,   517,   199,     9,
      50,    51,    56,    58,    70,   140,   141,   142,   143,   144,
     145,   187,   188,   199,   227,   387,   390,   393,   396,   399,
     402,   408,   423,   431,   432,   434,   435,   216,   520,   233,
     199,   244,   202,   201,   202,   201,   202,   201,   103,   166,
     202,   201,   112,   113,   166,   222,   223,   224,   225,   226,
     222,   216,   359,   306,   432,    83,     9,   200,   200,   200,
     200,   200,   200,   200,   201,    50,    51,   511,   513,   514,
     134,   281,   199,     9,   200,   200,   138,   204,     9,   438,
       9,   438,   204,   204,   204,   204,    83,    85,   216,   491,
     216,    70,   203,   203,   212,   214,    32,   135,   280,   182,
      54,   167,   182,   202,   417,   361,   138,     9,   438,   200,
     162,   200,   522,   522,    14,   372,   300,   231,   196,     9,
     439,    87,   522,   523,   458,   458,   203,     9,   438,   184,
     468,    83,    84,   302,   359,   200,     9,   439,    14,     9,
     200,     9,   200,   200,   200,   200,    14,   200,   203,   234,
     235,   364,   259,   134,   279,   199,   500,   204,   203,   359,
      32,   204,   204,   138,   203,     9,   438,   359,   501,   199,
     269,   264,   274,    14,   495,   267,   256,    71,   468,   359,
     501,   200,   200,   204,   203,   200,    50,    51,    70,    78,
      79,    80,    92,   140,   141,   142,   143,   144,   145,   158,
     187,   188,   216,   388,   391,   394,   397,   400,   403,   423,
     434,   441,   443,   444,   448,   451,   216,   468,   468,   138,
     279,   458,   463,   458,   200,   359,   295,    75,    76,   296,
     231,   358,   233,   349,   103,    38,   139,   285,   468,   432,
     216,    32,   236,   289,   201,   292,   201,   292,     9,   438,
      92,   229,   138,   162,     9,   438,   200,    87,   504,   505,
     522,   523,   502,   432,   432,   432,   432,   432,   437,   440,
     199,    70,    70,    70,    70,    70,   199,   199,   432,   162,
     202,    10,    11,    12,    31,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    69,   162,   501,
     203,   423,   202,   253,   221,   221,   221,   216,   221,   222,
     222,   226,     9,   439,   203,   203,    14,   468,   201,   184,
       9,   438,   216,   282,   423,   202,   482,   139,   468,    14,
     359,   359,   204,   359,   203,   212,   522,   282,   202,   416,
     176,    14,   200,   359,   373,   478,   201,   522,   196,   203,
     232,   235,   245,    32,   509,   457,   523,    38,    83,   176,
     459,   460,   462,   459,   460,   462,   522,    70,    38,    87,
     176,   359,   432,   248,   355,   356,   468,   249,   248,   249,
     249,   203,   235,   300,   199,   423,   280,   365,   260,   359,
     359,   359,   203,   199,   303,   281,    32,   280,   522,    14,
     279,   500,   427,   203,   199,    14,    78,    79,    80,   216,
     442,   442,   444,   446,   447,    52,   199,    70,    70,    70,
      70,    70,    91,   159,   199,   199,   162,     9,   438,   200,
     452,    38,   359,   280,   203,    75,    76,   297,   358,   236,
     203,   201,    96,   201,   285,   468,   199,   138,   284,    14,
     233,   292,   106,   107,   108,   292,   203,   522,   184,   138,
     162,   522,   216,   176,   515,   522,     9,   438,   200,   176,
     438,   138,   204,     9,   438,   437,   382,   383,   432,   405,
     432,   433,   405,   382,   405,   373,   375,   377,   405,   200,
     132,   217,   432,   487,   488,   432,   432,   432,    32,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   520,    83,   254,   203,   203,   203,   203,
     225,   201,   432,   514,   103,   104,   510,   512,     9,   311,
     200,   199,   350,   355,   359,   138,   204,   203,   495,   311,
     168,   181,   202,   412,   419,   359,   168,   202,   418,   138,
     201,   509,   199,   248,   346,   360,   469,   472,   522,   372,
      87,   523,    83,    83,   176,    14,    83,   501,   501,   479,
     468,   302,   359,   200,   300,   202,   300,   199,   138,   199,
     303,   200,   202,   522,   202,   201,   522,   280,   261,   430,
     303,   138,   204,     9,   438,   443,   446,   384,   385,   444,
     406,   444,   445,   406,   384,   406,   159,   373,   449,   450,
     406,    81,   444,   468,   202,   358,    32,    77,   236,   201,
     349,   284,   482,   285,   200,   432,   102,   106,   201,   359,
      32,   201,   293,   203,   184,   522,   216,   138,    87,   522,
     523,    32,   200,   432,   432,   200,   204,     9,   438,   138,
     204,     9,   438,   204,   204,   204,   138,     9,   438,   200,
     200,   138,   203,     9,   438,   432,    32,   200,   233,   201,
     201,   201,   201,   216,   522,   522,   510,   423,     6,   113,
     119,   122,   124,   125,   126,   127,   169,   170,   172,   203,
     312,   335,   336,   337,   340,   342,   343,   344,   345,   456,
     482,   359,   203,   202,   203,    54,   359,   203,   359,   359,
     372,   468,   201,   202,   523,    38,    83,   176,    14,    83,
     359,   199,   199,   204,   509,   200,   311,   200,   300,   359,
     303,   200,   311,   495,   311,   201,   202,   199,   200,   444,
     444,   200,   204,     9,   438,   138,   204,     9,   438,   204,
     204,   204,   138,   200,     9,   438,   200,   311,    32,   233,
     201,   200,   200,   200,   241,   201,   201,   293,   233,   138,
     522,   522,   176,   522,   138,   432,   432,   432,   432,   373,
     432,   432,   432,   202,   203,   512,   134,   135,   189,   217,
     498,   522,   283,   423,   113,   345,    31,   127,   140,   146,
     167,   173,   319,   320,   321,   322,   423,   171,   327,   328,
     130,   199,   216,   329,   330,    83,   341,   257,   522,   113,
       9,   201,     9,   201,   201,   495,   336,   337,   200,   308,
     167,   414,   203,   203,   359,    83,    83,   176,    14,    83,
     359,   303,   303,   119,   362,   509,   203,   509,   200,   200,
     203,   202,   203,   311,   300,   138,   444,   444,   444,   444,
     373,   203,   233,   239,   242,    32,   236,   287,   233,   522,
     200,   432,   138,   138,   138,   233,   423,   423,   500,    14,
     217,     9,   201,   202,   498,   495,   322,   183,   202,     9,
     201,     3,     4,     5,     6,     7,    10,    11,    12,    13,
      27,    28,    29,    57,    71,    72,    73,    74,    75,    76,
      77,    87,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   139,   140,   147,   148,   149,   150,
     151,   163,   164,   165,   175,   177,   178,   180,   187,   189,
     191,   193,   194,   216,   420,   421,     9,   201,   167,   171,
     216,   330,   331,   332,   201,    14,     9,   201,   256,   341,
     498,   498,   498,    14,   257,   341,   522,   203,   309,   310,
     498,    14,    83,   359,   200,   200,   199,   509,   198,   506,
     362,   509,   308,   203,   200,   444,   138,   138,    32,   236,
     286,   287,   233,   432,   432,   432,   203,   201,   201,   432,
     423,   315,   522,   323,   324,   431,   320,    14,    32,    51,
     325,   328,     9,    36,   200,    31,    50,    53,   432,    83,
     218,   499,   201,    14,    14,   522,   256,   201,   341,   201,
      14,   359,    38,    83,   411,   202,   507,   508,   522,   201,
     202,   333,   509,   506,   203,   509,   444,   444,   233,   100,
     252,   203,   216,   229,   316,   317,   318,     9,   438,     9,
     438,   203,   432,   421,   421,    68,   326,   331,   331,    31,
      50,    53,    14,   183,   199,   432,   432,   499,   201,   432,
      83,     9,   439,   231,     9,   439,    14,   510,   231,   202,
     333,   333,    98,   201,   116,   243,   162,   103,   522,   184,
     431,   174,   432,   511,   313,   199,    38,    83,   200,   203,
     508,   522,   203,   231,   201,   199,   180,   255,   216,   336,
     337,   184,   184,   298,   299,   457,   314,    83,   203,   423,
     253,   177,   216,   201,   200,     9,   439,    87,   124,   125,
     126,   339,   340,   298,    83,   283,   201,   509,   457,   523,
     523,   200,   200,   201,   506,    87,   339,    83,    38,    83,
     176,   509,   202,   201,   202,   334,   523,   523,    83,   176,
      14,    83,   506,   233,   231,    83,    38,    83,   176,    14,
      83,   359,   334,   203,   203,    83,   176,    14,    83,   359,
      14,    83,   359,   359
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
     340,   341,   341,   341,   341,   342,   342,   342,   343,   343,
     344,   344,   345,   346,   347,   347,   347,   347,   347,   347,
     348,   348,   349,   349,   350,   350,   350,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   359,   359,   359,
     359,   360,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     362,   362,   364,   363,   365,   363,   367,   366,   368,   366,
     369,   366,   370,   366,   371,   366,   372,   372,   372,   373,
     373,   374,   374,   375,   375,   376,   376,   377,   377,   378,
     379,   379,   380,   380,   381,   381,   382,   382,   383,   383,
     384,   384,   385,   385,   386,   387,   388,   389,   390,   391,
     392,   393,   394,   395,   396,   397,   398,   399,   400,   401,
     402,   403,   404,   404,   405,   405,   406,   406,   407,   408,
     409,   409,   410,   410,   410,   410,   410,   410,   410,   410,
     410,   410,   410,   410,   411,   411,   411,   411,   412,   413,
     413,   414,   414,   415,   415,   415,   416,   416,   417,   418,
     418,   419,   419,   419,   420,   420,   420,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     422,   423,   423,   424,   424,   424,   424,   424,   425,   425,
     426,   426,   426,   426,   427,   427,   427,   428,   428,   428,
     429,   429,   429,   430,   430,   431,   431,   431,   431,   431,
     431,   431,   431,   431,   431,   431,   431,   431,   431,   431,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   433,   433,   434,   435,
     435,   436,   436,   436,   436,   436,   436,   436,   437,   437,
     438,   438,   439,   439,   440,   440,   440,   440,   441,   441,
     441,   441,   441,   442,   442,   442,   442,   443,   443,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   444,   445,   445,   446,   446,   447,
     447,   447,   447,   448,   448,   449,   449,   450,   450,   451,
     451,   452,   452,   453,   453,   455,   454,   456,   457,   457,
     458,   458,   459,   459,   459,   460,   460,   461,   461,   462,
     462,   463,   463,   464,   464,   464,   465,   465,   466,   466,
     467,   467,   468,   468,   468,   468,   468,   468,   468,   468,
     468,   468,   468,   469,   470,   470,   470,   470,   470,   470,
     470,   470,   471,   471,   471,   471,   471,   471,   471,   471,
     471,   471,   471,   472,   473,   473,   474,   474,   474,   475,
     475,   475,   476,   477,   477,   477,   478,   478,   478,   478,
     479,   479,   480,   480,   480,   480,   480,   480,   481,   481,
     481,   481,   481,   482,   482,   482,   482,   482,   482,   483,
     483,   484,   484,   484,   484,   484,   484,   484,   484,   485,
     485,   486,   486,   486,   486,   487,   487,   488,   488,   488,
     488,   489,   489,   489,   489,   490,   490,   490,   490,   490,
     490,   491,   491,   491,   492,   492,   492,   492,   492,   492,
     492,   492,   492,   492,   492,   493,   493,   494,   494,   495,
     495,   496,   496,   496,   496,   497,   497,   498,   498,   499,
     499,   500,   500,   501,   501,   502,   502,   503,   504,   504,
     504,   504,   504,   504,   505,   505,   505,   505,   506,   506,
     507,   507,   508,   508,   509,   509,   510,   510,   511,   512,
     512,   513,   513,   513,   513,   514,   514,   514,   515,   515,
     515,   515,   516,   516,   517,   517,   517,   517,   518,   519,
     520,   520,   521,   521,   522,   522,   522,   522,   522,   522,
     522,   522,   522,   522,   522,   523,   523
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
       1,     3,     5,     1,     3,     5,     4,     5,     3,     3,
       3,     4,     3,     3,     3,     3,     2,     2,     1,     1,
       3,     1,     1,     0,     1,     2,     4,     3,     3,     6,
       2,     3,     2,     3,     6,     3,     1,     1,     1,     1,
       1,     3,     6,     3,     4,     6,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     5,     4,     3,     1,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     2,     1,
       5,     0,     0,    12,     0,    13,     0,     4,     0,     7,
       0,     5,     0,     3,     0,     6,     2,     2,     4,     1,
       1,     5,     3,     5,     3,     2,     0,     2,     0,     4,
       4,     3,     2,     0,     5,     3,     2,     0,     5,     3,
       2,     0,     5,     3,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     2,     0,     2,     0,     2,     0,     4,     4,
       4,     4,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     3,     4,     1,     2,     4,     2,
       6,     0,     1,     0,     5,     4,     2,     0,     1,     1,
       3,     1,     3,     1,     1,     3,     3,     1,     1,     1,
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
       1,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     5,     4,     3,     1,     3,     3,
       1,     1,     1,     1,     1,     3,     3,     3,     2,     0,
       1,     0,     1,     0,     5,     3,     3,     1,     1,     1,
       1,     3,     2,     1,     1,     1,     1,     1,     3,     1,
       1,     1,     3,     1,     2,     2,     4,     3,     4,     1,
       1,     1,     1,     1,     1,     3,     1,     2,     0,     5,
       3,     3,     1,     3,     1,     2,     0,     5,     3,     2,
       0,     3,     0,     4,     2,     0,     3,     3,     1,     0,
       1,     1,     1,     1,     3,     1,     1,     1,     3,     1,
       1,     3,     3,     2,     2,     2,     2,     4,     5,     5,
       5,     5,     1,     1,     1,     1,     1,     1,     3,     3,
       4,     4,     3,     3,     1,     1,     1,     1,     3,     1,
       4,     3,     1,     1,     1,     1,     1,     3,     3,     1,
       1,     4,     4,     3,     1,     1,     7,     9,     9,     7,
       6,     8,     1,     4,     4,     1,     1,     1,     4,     2,
       1,     0,     1,     1,     1,     3,     3,     3,     0,     1,
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
#line 7384 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 763 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 7392 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 770 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 7398 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 771 "hphp.y" /* yacc.c:1646  */
    { }
#line 7404 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 7410 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 775 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7416 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7422 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7428 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 779 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 7440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 7448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 783 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 7455 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 785 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 7461 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 786 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7467 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 787 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 7473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 788 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 789 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7487 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 793 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7496 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 798 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 803 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7512 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 806 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7519 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 809 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 813 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 817 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7543 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 821 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 825 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 828 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7566 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7572 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7578 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7584 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7590 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7596 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7602 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7608 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 840 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7614 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7620 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 842 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7626 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 843 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7632 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 844 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7638 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 845 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 927 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 929 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 934 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 935 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7669 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 941 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7675 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 945 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7681 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 946 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7687 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 948 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7693 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 950 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7699 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 955 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7705 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 956 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7712 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 962 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7718 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 966 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7725 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 968 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7732 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 970 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 975 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7745 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 977 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7751 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 980 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7757 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 982 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7763 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 983 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7769 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 988 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7778 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 995 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7787 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1003 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7794 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1006 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7801 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1012 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 7807 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1013 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval)); }
#line 7813 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1018 "hphp.y" /* yacc.c:1646  */
    {
                                         _p->onUsing((yyval), (yyvsp[-2]), true, (yyvsp[-1]), nullptr);
                                       }
#line 7821 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1025 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7827 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1026 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1031 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 7839 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1032 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval), (yyval), (yyvsp[0])); }
#line 7846 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1037 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7852 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7858 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1039 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7864 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1040 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7870 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7876 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1047 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7882 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1052 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7888 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1053 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7903 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1059 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7910 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1062 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7918 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1066 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7925 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1068 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7933 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1071 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7940 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1073 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7948 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7954 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1077 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7960 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1078 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7966 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1079 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7972 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1080 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7978 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1081 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7984 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1082 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7990 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7996 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 8002 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1085 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 8008 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 8014 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1087 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 8020 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1088 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 8026 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1089 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 8032 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1090 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 8039 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1094 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 8046 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1096 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 8054 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1101 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 8061 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1103 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 8069 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1107 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 8077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1116 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 8083 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1117 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8089 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1120 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false); }
#line 8095 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1121 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 8101 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1123 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1127 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1131 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8125 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1135 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8133 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1139 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1143 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1148 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1151 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 8163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 8172 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1156 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8178 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1157 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8184 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1158 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8190 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1159 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8196 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1160 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8202 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1161 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8208 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1162 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 8214 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1163 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1164 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1165 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 8232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1166 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1167 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 8248 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1189 "hphp.y" /* yacc.c:1646  */
    { _p->pushLabelScope(LS::Using);
                                         _p->onNewLabelScope(false);
                                         (yyval) = (yyvsp[-1]); }
#line 8256 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1195 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1; }
#line 8262 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1196 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8268 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1205 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), nullptr, (yyvsp[-2]));
                                         _p->onExprListElem((yyval), &(yyval), (yyvsp[0])); }
#line 8275 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1207 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0])); }
#line 8281 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1217 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8287 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1218 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8293 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1222 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false); }
#line 8299 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1223 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8305 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1232 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 8311 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1233 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8317 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1237 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::Finally);}
#line 8324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1239 "hphp.y" /* yacc.c:1646  */
    { _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->popLabelScope();
                                         _p->onCompleteLabelScope(false);}
#line 8332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1245 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8338 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1246 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8344 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1250 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 8350 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1251 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8356 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1255 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 8362 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1261 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8371 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1268 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8381 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1276 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8390 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1283 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1291 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8409 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1297 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8419 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1306 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8426 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1310 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 8432 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1314 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8439 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1318 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 8445 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1324 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8452 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 8470 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1342 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8477 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 8495 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1359 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8502 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1362 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8510 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1367 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8517 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1370 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8525 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1376 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 8531 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1379 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 8537 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1383 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8544 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1386 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8555 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1394 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8562 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1397 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8573 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1405 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8579 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1406 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 8586 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1410 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8592 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1413 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8598 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1416 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 8604 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1417 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 8610 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1418 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 8618 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1421 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 8624 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1422 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 8630 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1426 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8636 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1427 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8642 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1430 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8648 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1431 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8654 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1434 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8660 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1435 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8666 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1438 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8672 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1440 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8678 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1443 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8684 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1445 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8690 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8696 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1450 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8702 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8708 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8714 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1455 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8720 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8726 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8732 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8738 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8744 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1469 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8750 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1471 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8756 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1474 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8762 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1476 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8768 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1480 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8774 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1482 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8781 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8787 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8793 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8799 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8805 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1495 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8811 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1497 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8817 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1498 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8823 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1501 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8829 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1502 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8835 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1507 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8841 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1508 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8847 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1513 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8853 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1514 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8859 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8865 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1518 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8871 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1521 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8877 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1522 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8883 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1530 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8890 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1536 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8897 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1542 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1546 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1550 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8918 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1555 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8925 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1560 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8933 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1563 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8939 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1569 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1574 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8955 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1579 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8963 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1585 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1591 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8979 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1597 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8987 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1603 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1609 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 9003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1616 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 9011 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1623 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 9019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1632 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 9026 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1637 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 9033 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1642 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 9041 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1646 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9047 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1649 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 9054 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1653 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 9061 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1657 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 9069 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1660 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9075 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1665 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9083 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1669 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1673 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1678 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9107 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1683 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1688 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1693 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9131 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1698 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1704 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9147 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1710 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9155 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1716 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 9161 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1717 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 9167 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1718 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 9173 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9179 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1724 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9185 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1727 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,false);}
#line 9192 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1729 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::InOut,false);}
#line 9199 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1731 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::Ref,false);}
#line 9206 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1733 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,true);}
#line 9213 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1736 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::In,false);}
#line 9220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1739 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::In,true);}
#line 9227 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1742 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::Ref,false);}
#line 9234 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1745 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::InOut,false);}
#line 9241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1750 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1751 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 9253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1754 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9259 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1755 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 9265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1756 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 9271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1760 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1762 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1763 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 9289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1764 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1769 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1770 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9307 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1773 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL,NULL);}
#line 9314 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1778 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9320 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1784 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9326 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1785 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1789 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-2]),(yyvsp[-1]),NULL,NULL);}
#line 9339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1793 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-2]),NULL);}
#line 9346 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1797 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-2]),(yyvsp[-1]),NULL,&(yyvsp[-3]));}
#line 9353 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1802 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-2]),&(yyvsp[-4]));}
#line 9360 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1804 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL,NULL);}
#line 9367 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1807 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL,NULL,true);}
#line 9374 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1809 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9380 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1812 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9388 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1819 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9398 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1827 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1834 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9416 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1840 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 9422 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1842 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9428 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1844 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 9440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1848 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 9446 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1849 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 9453 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1852 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 9459 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1855 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9465 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1856 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9471 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1857 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 9477 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1863 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 9483 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1868 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 9490 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1871 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 9498 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1878 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 9504 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1879 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 9511 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1884 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 9518 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1887 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 9524 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1894 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 9531 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1896 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 9537 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1900 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9543 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1905 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9549 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1907 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9555 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1909 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9561 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1910 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 9572 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1916 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 9578 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1918 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 9584 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1919 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 9590 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1923 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval),nullptr,(yyvsp[0])); }
#line 9596 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1925 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval),&(yyvsp[-2]),(yyvsp[0])); }
#line 9602 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1930 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9608 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1933 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9614 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1934 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 9620 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1938 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 9626 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1939 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 9632 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1943 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 9639 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1946 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 9646 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1951 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 9653 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 9659 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 9666 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1959 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 9672 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 9678 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 9684 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 9690 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1966 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 9696 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1970 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9702 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1971 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 9708 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1972 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 9714 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 9720 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1974 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 9726 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 9732 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1978 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 9738 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1985 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1990 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9764 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1995 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1996 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9788 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9794 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9800 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9806 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9812 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 2009 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 2012 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9824 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9830 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9836 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9842 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 2016 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9848 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9854 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9860 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 2022 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9866 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 2023 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9872 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 2026 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9878 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9884 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 2028 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9890 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 2032 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9896 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 2034 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9902 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 2035 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9908 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 2036 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9914 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 2040 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9920 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 2042 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9926 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 2045 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 2051 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9938 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 2053 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9944 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 2057 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9952 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 2061 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 2065 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9965 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 2069 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 2073 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 2075 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 2076 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 2077 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 2078 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 2079 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 2082 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 10013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 2083 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 10019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 2087 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 2088 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 2092 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 10037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 2093 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 10043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 2094 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 10049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 2095 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10055 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 2099 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10061 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 2104 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 10067 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 2108 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 10073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 2112 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10079 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 2116 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 10085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 2120 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 2125 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 10097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 2129 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 2133 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 2134 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 2135 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10121 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 2136 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 2137 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10133 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 2141 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 2146 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 10145 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 2147 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 10151 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 10157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 2151 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 10163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2152 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 10169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2153 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 10175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2154 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 10181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2155 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 10187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2156 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 10193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2157 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 10199 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2158 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 10205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2159 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 10211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2160 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 10217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2161 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 10223 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2162 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 10229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2163 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 10235 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2164 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 10241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2165 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 10247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2166 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 10253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2167 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 10259 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2168 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 10265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2169 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 10271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2170 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 10277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2171 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 10283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2172 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 10289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2173 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 10295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2174 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 10301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2175 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 10307 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2176 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 10313 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2177 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 10319 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2178 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 10325 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2179 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 10331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2180 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 10337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2181 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 10343 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2182 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 10349 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2183 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 10355 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2184 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 10361 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2185 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 10367 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2186 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 10373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2187 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 10379 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2188 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 10385 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2189 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 10391 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2190 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 10397 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2191 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 10403 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2192 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 10409 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2193 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 10415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2194 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 10421 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2195 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 10428 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2197 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 10434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2198 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 10441 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2200 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 10447 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2202 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 10453 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2203 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10459 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2204 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 10465 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2205 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 10471 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2206 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 10477 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2207 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10483 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 10489 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2209 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 10495 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2210 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 10501 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2211 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 10507 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2212 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 10513 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2213 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 10519 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2214 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 10525 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2215 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 10531 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2216 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 10537 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2217 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10543 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2218 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10549 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2219 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10555 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2220 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10561 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2221 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10567 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2222 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10573 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2223 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10579 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2224 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10585 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2225 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10591 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2226 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 10597 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2227 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 10603 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2228 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10609 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2235 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 10615 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2236 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10621 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2241 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10630 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2247 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10642 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2256 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10651 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2262 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10663 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
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
#line 10677 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
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
#line 10692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2293 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10702 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
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
#line 10717 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2312 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10727 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
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
#line 10744 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
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
#line 10758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2339 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2347 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10781 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2355 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10794 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2366 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10800 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2367 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10806 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2369 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10812 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2373 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10819 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2375 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2382 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10831 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2385 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10837 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2392 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10843 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2395 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10849 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2400 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10855 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2401 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10861 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2406 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10867 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2407 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10873 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2411 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval), (yyvsp[-1]));}
#line 10879 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2415 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10885 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2416 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10891 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2421 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10897 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2422 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10903 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2427 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10909 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2428 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10915 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2433 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10921 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2434 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10927 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2440 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10933 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2442 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10939 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2447 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10945 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2448 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10951 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2454 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10957 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2456 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10963 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2460 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10969 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2464 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10975 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2468 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10981 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2472 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10987 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2476 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10993 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2480 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10999 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2484 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 11005 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2488 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 11011 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2492 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 11017 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2496 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11023 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2500 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11029 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2504 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11035 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2508 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11041 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2512 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11047 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2516 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11053 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2520 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11059 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2524 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11065 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2528 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11071 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2533 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2534 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11083 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2539 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11089 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2540 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11095 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2545 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11101 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2546 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11107 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2551 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2558 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2565 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2567 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2571 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2572 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11147 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11153 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2574 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11159 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2575 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11165 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2576 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11171 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2577 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11177 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2578 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11183 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2579 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11189 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2580 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 11196 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2582 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11202 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2583 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11208 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2587 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11214 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2588 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 11220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2589 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 11226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2590 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 11232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2597 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 11238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
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
#line 11256 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
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
#line 11274 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2630 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 11280 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2631 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 11286 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2634 "hphp.y" /* yacc.c:1646  */
    { _p->onXhpAttributesStart(); (yyval).reset();}
#line 11292 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { _p->onXhpAttributeSpread((yyval), &(yyvsp[-4]), (yyvsp[-1]));}
#line 11298 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11304 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { _p->onOptExprListElem((yyval), &(yyvsp[-1]), (yyvsp[0])); }
#line 11310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2643 "hphp.y" /* yacc.c:1646  */
    {  (yyval).reset();}
#line 11316 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11323 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2653 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2656 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 11349 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11355 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11361 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11367 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 11373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2672 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 11379 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2676 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11385 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2677 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11391 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2678 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11397 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2679 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11403 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2680 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11409 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2681 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2682 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11421 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2683 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11427 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2684 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11433 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2685 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11439 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11445 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2687 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11451 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2688 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11457 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2689 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11463 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2690 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11469 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2691 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11475 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11481 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2693 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11487 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2694 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2695 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2696 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2697 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11511 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2698 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11517 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2699 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11523 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2700 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11529 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2701 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2702 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11541 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2703 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11547 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2704 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11553 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2705 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2706 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11565 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2707 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11571 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2708 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11577 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2709 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11583 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2710 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2711 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11601 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2713 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2714 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2715 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11619 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2716 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11625 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2717 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11631 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11637 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11643 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11649 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11655 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2722 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11661 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11667 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2724 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11673 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11679 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2726 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11685 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2727 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11691 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2728 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11697 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11703 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11709 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2731 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11715 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2732 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11721 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2733 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11727 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2735 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11745 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2737 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11751 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11757 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11763 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11769 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11775 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2742 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11781 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2743 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11787 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2744 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11793 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2745 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11799 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2746 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11805 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2747 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11811 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2748 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11817 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2749 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11823 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2750 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11829 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2751 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11835 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2752 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11841 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2753 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11847 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2754 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11853 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2755 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11859 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2756 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11865 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2757 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11871 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2758 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11877 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2763 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11883 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2767 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2768 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2772 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11901 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2773 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11907 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2774 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11913 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2775 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11920 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2777 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11927 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2781 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11933 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2790 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11939 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2793 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11945 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2794 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11952 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2796 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2806 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11965 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2810 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2811 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2812 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2816 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2817 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2818 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2822 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2823 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 12013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2824 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2829 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2833 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2834 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2835 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2836 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 12056 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2838 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 12062 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2839 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 12068 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2840 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 12074 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2841 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 12080 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2842 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 12086 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2843 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 12092 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2844 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 12098 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2845 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 12104 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2846 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 12110 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12116 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2851 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12122 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2855 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12128 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2856 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12134 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2858 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12140 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2859 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12146 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2861 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1]));}
#line 12152 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2862 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12158 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2863 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12164 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2864 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12170 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2865 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12176 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2866 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12182 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12188 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2868 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12194 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2869 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12200 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2870 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12206 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2872 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 12212 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2874 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 12218 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2876 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 12224 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2878 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 12230 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2880 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 12236 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2881 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 12242 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2882 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 12248 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2883 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 12254 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2884 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 12260 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2885 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 12266 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2886 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 12272 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2887 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 12278 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2888 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 12284 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2889 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 12290 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2890 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 12296 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2891 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 12302 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2892 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 12308 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2893 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 12314 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2894 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 12320 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2895 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12326 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2896 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2898 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 12338 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2900 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 12344 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2902 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 12350 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2904 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 12356 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2905 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 12362 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2907 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 12369 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2909 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 12375 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2912 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 12382 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2916 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 12388 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2919 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 12394 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2920 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 12400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2924 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2925 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2931 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2937 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2938 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2942 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2943 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2944 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2945 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2946 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 12460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2947 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 12466 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2949 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 12473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2954 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2955 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2959 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2960 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12497 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2963 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 12503 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2964 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12509 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2970 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2972 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12521 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2974 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2975 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12533 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2979 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2980 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12545 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2981 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2984 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12557 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2986 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12563 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2989 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 12569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2990 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 12575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2991 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 12581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2992 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2996 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 12594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2999 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 12602 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 3006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12608 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 3007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12614 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 3010 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 12622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 3013 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 3014 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12634 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 3015 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12640 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12646 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 3018 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12652 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 3020 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1]));}
#line 12658 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 3021 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12664 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 3022 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12670 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 3023 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12676 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 3024 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12682 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 3025 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12688 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 3026 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12694 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 3031 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12700 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 3032 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12706 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 3037 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12712 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 3038 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12718 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 3043 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12724 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3045 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12730 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3047 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12736 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3048 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12742 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3052 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12748 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3053 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12754 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3058 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12760 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3059 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12766 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3064 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12772 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3067 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12778 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3072 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12784 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3073 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12790 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3076 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12796 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3077 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 12803 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3084 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 12809 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3086 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 12815 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3089 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 12821 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3091 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12827 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3094 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3097 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12839 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3098 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12845 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3102 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12851 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3103 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12857 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3107 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3108 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12869 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3109 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3113 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12881 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3118 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3123 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12893 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3124 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3128 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3133 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3138 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3139 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3144 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3145 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3147 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3152 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3154 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
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
#line 12967 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
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
#line 12981 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
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
#line 12995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
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
#line 13009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3210 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3211 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3212 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13027 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3213 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13033 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3214 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13039 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3215 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13045 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
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
#line 13059 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3234 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13065 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3236 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13071 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3238 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3239 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13083 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3243 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13089 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3247 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13095 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3248 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13101 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3249 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13107 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3250 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13113 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
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
#line 13127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3267 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13133 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3269 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3273 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13145 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3278 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13151 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3279 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3280 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3281 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3282 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3283 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3284 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3286 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3287 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13199 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3290 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3292 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3296 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3300 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13223 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3301 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3307 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13235 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3311 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3315 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3322 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 13253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3331 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 13259 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3335 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 13265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3339 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3348 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3349 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3350 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3354 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3355 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 13301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3356 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 13307 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3358 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]) = 1; _p->onIndirectRef((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 13313 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3363 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13319 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3364 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13325 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3375 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3376 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3377 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13343 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 13357 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3391 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13363 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3392 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13369 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3396 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13375 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3397 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13381 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 13395 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3409 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13401 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3413 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 13407 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3414 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 13413 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3416 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 13419 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3417 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 13425 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3418 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 13431 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3419 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 13437 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3424 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13443 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3425 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13449 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3429 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13455 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3430 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13461 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3431 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13467 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3432 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3435 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3437 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 13485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3438 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3439 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 13497 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3444 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13503 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3445 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13509 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3449 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3450 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13521 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3451 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3452 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13533 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3457 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3458 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13545 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3463 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3465 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13557 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3467 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13563 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1030:
#line 3468 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1031:
#line 3472 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 13575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1032:
#line 3474 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 13581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1033:
#line 3475 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 13587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1034:
#line 3477 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 13594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1035:
#line 3482 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13600 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1036:
#line 3484 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13606 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 13620 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1038:
#line 3496 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 13626 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1039:
#line 3498 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 13632 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3499 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13638 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 13644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3503 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 13650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3504 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 13656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3508 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 13662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3509 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 13668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3510 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3511 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13680 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3512 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13686 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3513 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3514 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 13698 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3515 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 13704 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3516 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 13710 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3517 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 13716 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3518 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 13722 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3522 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 13728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3523 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 13734 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3528 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3530 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3544 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13754 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3549 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 13762 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3553 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3558 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 13778 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3564 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13784 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3565 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13790 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3569 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13796 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3570 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3576 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3580 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 13814 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3586 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13820 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3590 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 13827 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3597 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3598 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13839 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3602 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 13847 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3605 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 13854 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1077:
#line 3611 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13860 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1078:
#line 3615 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1079:
#line 3618 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13876 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1080:
#line 3621 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-3]); }
#line 13883 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1081:
#line 3623 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13890 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1082:
#line 3625 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13897 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1083:
#line 3627 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13903 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1084:
#line 3632 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]); }
#line 13909 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1085:
#line 3633 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13915 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1086:
#line 3634 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13921 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1087:
#line 3635 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13927 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1094:
#line 3656 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13933 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1095:
#line 3657 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13939 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1098:
#line 3666 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13945 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1101:
#line 3677 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13951 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1102:
#line 3679 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13957 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1103:
#line 3683 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13963 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1104:
#line 3686 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13969 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1105:
#line 3690 "hphp.y" /* yacc.c:1646  */
    {}
#line 13975 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1106:
#line 3691 "hphp.y" /* yacc.c:1646  */
    {}
#line 13981 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1107:
#line 3692 "hphp.y" /* yacc.c:1646  */
    {}
#line 13987 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1108:
#line 3698 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13994 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1109:
#line 3703 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 14004 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1110:
#line 3712 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 14010 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1111:
#line 3718 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 14019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1112:
#line 3726 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 14025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1113:
#line 3727 "hphp.y" /* yacc.c:1646  */
    { }
#line 14031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1114:
#line 3733 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 14037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1115:
#line 3735 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 14043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1116:
#line 3736 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 14053 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1117:
#line 3741 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 14060 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1118:
#line 3747 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("HH\\darray"); }
#line 14067 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1119:
#line 3752 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1120:
#line 3757 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 14081 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1121:
#line 3761 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14087 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1122:
#line 3766 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 14093 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1123:
#line 3768 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 14099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1124:
#line 3774 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 14106 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1125:
#line 3776 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 14114 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1126:
#line 3779 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14120 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1127:
#line 3780 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14128 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1128:
#line 3783 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14136 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1129:
#line 3786 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14142 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1130:
#line 3789 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 14150 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1131:
#line 3792 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1132:
#line 3794 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 14166 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1133:
#line 3800 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 14175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1134:
#line 3806 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("HH\\varray");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 14185 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1135:
#line 3814 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14191 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1136:
#line 3815 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 14197 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;


#line 14201 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}
