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
#define YYLAST   20393

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  209
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  315
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1134
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
    1777,  1783,  1785,  1788,  1791,  1795,  1797,  1800,  1803,  1801,
    1818,  1815,  1830,  1832,  1834,  1836,  1838,  1840,  1842,  1846,
    1847,  1848,  1851,  1857,  1861,  1867,  1870,  1875,  1877,  1882,
    1887,  1891,  1892,  1896,  1897,  1899,  1901,  1907,  1908,  1910,
    1914,  1915,  1920,  1924,  1925,  1929,  1930,  1934,  1936,  1942,
    1947,  1948,  1950,  1954,  1955,  1956,  1957,  1961,  1962,  1963,
    1964,  1965,  1966,  1968,  1973,  1976,  1977,  1981,  1982,  1986,
    1987,  1990,  1991,  1994,  1995,  1998,  1999,  2003,  2004,  2005,
    2006,  2007,  2008,  2009,  2013,  2014,  2017,  2018,  2019,  2022,
    2024,  2026,  2027,  2030,  2032,  2035,  2041,  2043,  2047,  2051,
    2055,  2060,  2064,  2065,  2067,  2068,  2069,  2070,  2073,  2074,
    2078,  2079,  2083,  2084,  2085,  2086,  2090,  2094,  2099,  2103,
    2107,  2111,  2115,  2120,  2124,  2125,  2126,  2127,  2128,  2132,
    2136,  2138,  2139,  2140,  2143,  2144,  2145,  2146,  2147,  2148,
    2149,  2150,  2151,  2152,  2153,  2154,  2155,  2156,  2157,  2158,
    2159,  2160,  2161,  2162,  2163,  2164,  2165,  2166,  2167,  2168,
    2169,  2170,  2171,  2172,  2173,  2174,  2175,  2176,  2177,  2178,
    2179,  2180,  2181,  2182,  2183,  2184,  2185,  2186,  2188,  2189,
    2191,  2192,  2194,  2195,  2196,  2197,  2198,  2199,  2200,  2201,
    2202,  2203,  2204,  2205,  2206,  2207,  2208,  2209,  2210,  2211,
    2212,  2213,  2214,  2215,  2216,  2217,  2218,  2219,  2223,  2227,
    2232,  2231,  2247,  2245,  2264,  2263,  2284,  2283,  2303,  2302,
    2321,  2321,  2338,  2338,  2357,  2358,  2359,  2364,  2366,  2370,
    2374,  2380,  2384,  2390,  2392,  2396,  2398,  2402,  2406,  2407,
    2411,  2413,  2417,  2419,  2423,  2425,  2429,  2432,  2437,  2439,
    2443,  2446,  2451,  2455,  2459,  2463,  2467,  2471,  2475,  2479,
    2483,  2487,  2491,  2495,  2499,  2503,  2507,  2511,  2515,  2519,
    2523,  2525,  2529,  2531,  2535,  2537,  2541,  2548,  2555,  2557,
    2562,  2563,  2564,  2565,  2566,  2567,  2568,  2569,  2570,  2571,
    2573,  2574,  2578,  2579,  2580,  2581,  2585,  2591,  2604,  2621,
    2622,  2625,  2626,  2628,  2633,  2634,  2637,  2641,  2644,  2647,
    2654,  2655,  2659,  2660,  2662,  2667,  2668,  2669,  2670,  2671,
    2672,  2673,  2674,  2675,  2676,  2677,  2678,  2679,  2680,  2681,
    2682,  2683,  2684,  2685,  2686,  2687,  2688,  2689,  2690,  2691,
    2692,  2693,  2694,  2695,  2696,  2697,  2698,  2699,  2700,  2701,
    2702,  2703,  2704,  2705,  2706,  2707,  2708,  2709,  2710,  2711,
    2712,  2713,  2714,  2715,  2716,  2717,  2718,  2719,  2720,  2721,
    2722,  2723,  2724,  2725,  2726,  2727,  2728,  2729,  2730,  2731,
    2732,  2733,  2734,  2735,  2736,  2737,  2738,  2739,  2740,  2741,
    2742,  2743,  2744,  2745,  2746,  2747,  2748,  2749,  2753,  2758,
    2759,  2763,  2764,  2765,  2766,  2768,  2772,  2773,  2784,  2785,
    2787,  2789,  2801,  2802,  2803,  2807,  2808,  2809,  2813,  2814,
    2815,  2818,  2820,  2824,  2825,  2826,  2827,  2829,  2830,  2831,
    2832,  2833,  2834,  2835,  2836,  2837,  2838,  2841,  2846,  2847,
    2848,  2850,  2851,  2853,  2854,  2855,  2856,  2857,  2858,  2859,
    2860,  2861,  2862,  2864,  2866,  2868,  2870,  2872,  2873,  2874,
    2875,  2876,  2877,  2878,  2879,  2880,  2881,  2882,  2883,  2884,
    2885,  2886,  2887,  2888,  2890,  2892,  2894,  2896,  2897,  2900,
    2901,  2905,  2909,  2911,  2915,  2916,  2920,  2926,  2929,  2933,
    2934,  2935,  2936,  2937,  2938,  2939,  2944,  2946,  2950,  2951,
    2954,  2955,  2959,  2962,  2964,  2966,  2970,  2971,  2972,  2973,
    2976,  2980,  2981,  2982,  2983,  2987,  2989,  2996,  2997,  2998,
    2999,  3004,  3005,  3006,  3007,  3009,  3010,  3012,  3013,  3014,
    3015,  3016,  3017,  3021,  3023,  3027,  3029,  3032,  3035,  3037,
    3039,  3042,  3044,  3048,  3050,  3053,  3056,  3062,  3064,  3067,
    3068,  3073,  3076,  3080,  3080,  3085,  3088,  3089,  3093,  3094,
    3098,  3099,  3100,  3104,  3109,  3114,  3115,  3119,  3124,  3129,
    3130,  3134,  3136,  3137,  3142,  3144,  3149,  3160,  3174,  3186,
    3201,  3202,  3203,  3204,  3205,  3206,  3207,  3217,  3226,  3228,
    3230,  3234,  3238,  3239,  3240,  3241,  3242,  3258,  3259,  3262,
    3269,  3270,  3271,  3272,  3273,  3274,  3275,  3277,  3278,  3280,
    3282,  3287,  3291,  3292,  3296,  3299,  3303,  3310,  3314,  3323,
    3330,  3338,  3340,  3341,  3345,  3346,  3347,  3349,  3354,  3355,
    3366,  3367,  3368,  3369,  3380,  3383,  3386,  3387,  3388,  3389,
    3400,  3404,  3405,  3406,  3408,  3409,  3410,  3414,  3416,  3419,
    3421,  3422,  3423,  3424,  3427,  3429,  3430,  3434,  3436,  3439,
    3441,  3442,  3443,  3447,  3449,  3452,  3455,  3457,  3459,  3463,
    3464,  3466,  3467,  3473,  3474,  3476,  3486,  3488,  3490,  3493,
    3494,  3495,  3499,  3500,  3501,  3502,  3503,  3504,  3505,  3506,
    3507,  3508,  3509,  3513,  3514,  3518,  3520,  3528,  3530,  3534,
    3538,  3543,  3547,  3555,  3556,  3560,  3561,  3567,  3568,  3577,
    3578,  3586,  3589,  3593,  3596,  3601,  3606,  3609,  3612,  3614,
    3616,  3618,  3622,  3624,  3625,  3626,  3629,  3631,  3637,  3638,
    3642,  3643,  3647,  3648,  3652,  3653,  3656,  3661,  3662,  3666,
    3669,  3671,  3675,  3681,  3682,  3683,  3687,  3691,  3699,  3704,
    3716,  3718,  3722,  3725,  3727,  3732,  3737,  3743,  3746,  3751,
    3756,  3758,  3765,  3767,  3770,  3771,  3774,  3777,  3778,  3783,
    3785,  3789,  3795,  3805,  3806
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

#define YYPACT_NINF -1867

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1867)))

#define YYTABLE_NINF -1135

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1135)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1867,   203, -1867, -1867,  5831, 15332, 15332,    -3, 15332, 15332,
   15332, 15332, 12933, 15332, -1867, 15332, 15332, 15332, 15332, 18447,
   18447, 15332, 15332, 15332, 15332, 15332, 15332, 15332, 15332, 13112,
   19245, 15332,    11,    40, -1867, -1867, -1867,   180, -1867,   263,
   -1867, -1867, -1867,   211, 15332, -1867,    40,   228,   232,   251,
   -1867,    40, 13291,   578, 13470, -1867, 16196, 11796,   122, 15332,
   19494,   257,    83,   606,   313, -1867, -1867, -1867,   300,   363,
     384,   395, -1867,   578,   397,   407,   215,   511,   549,   605,
     610, -1867, -1867, -1867, -1867, -1867, 15332,   614,  4217, -1867,
   -1867,   578, -1867, -1867, -1867, -1867,   578, -1867,   578, -1867,
     523,   493,   496,   578,   578, -1867,   350, -1867, -1867, 13676,
   -1867, -1867,   443,   107,   678,   678, -1867,   698,   571,   672,
     550, -1867,   108, -1867,   557,   628,   722, -1867, -1867, -1867,
   -1867,  5456,   588, -1867,   161, -1867,   583,   586,   591,   620,
     624,   631,   640,   647, 17358, -1867, -1867, -1867, -1867, -1867,
     190,   702,   795,   803,   805,   808,   811, -1867,   816,   824,
   -1867,    74,   652, -1867,   734,   226, -1867,  1612,   177, -1867,
   -1867,  4233,   161,   161,   685,   230, -1867,   183,   135,   709,
     216, -1867, -1867,   831, -1867,   756, -1867, -1867,   711,   754,
   -1867, 15332, -1867,   722,   588, 19782,  5070, 19782, 15332, 19782,
   19782, 16745, 16745,   725, 18620, 19782,   883,   578,   870,   870,
     491,   870, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867,    73, 15332,   761, -1867, -1867,   792,   764,   562,   783,
     562,   870,   870,   870,   870,   870,   870,   870,   870, 18447,
   18668,   796,   971,   756, -1867, 15332,   761, -1867,   823, -1867,
     825,   802, -1867,   172, -1867, -1867, -1867,   562,   161, -1867,
   13855, -1867, -1867, 15332, 10363,   996,   109, 19782, 11393, -1867,
   15332, 15332,   578, -1867, -1867, 17406,   818, -1867, 17479, -1867,
   -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, 17807,
   -1867, 17807, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867,   116,   115,   754, -1867, -1867, -1867, -1867,   807, -1867,
    2819,   117, -1867, -1867,   847,   998, -1867,   848, 16937, 15332,
   -1867,   819,   821, 17527, -1867,    66, 17575,  5501,  5501,  5501,
     578,  5501,   813,  1015,   826, -1867,    54, -1867, 18114,   130,
   -1867,  1012,   132,   893, -1867,   897, -1867, 18447, 15332, 15332,
     832,   850, -1867, -1867, 18190, 13112, 15332, 15332, 15332, 15332,
   15332,   133,   171,   566, -1867, 15511, 18447,   703, -1867,   578,
   -1867,    28,   571, -1867, -1867, -1867, -1867, 19347, 15332,  1017,
     934, -1867, -1867, -1867,   127, 15332,   839,   841, 19782,   842,
    2384,   843,  6449, 15332, -1867,   517,   846,   719,   517,   547,
     377, -1867,   578, 17807,   854, 11975, 16196, -1867, 14061,   853,
     853,   853,   853, -1867, -1867,  4660, -1867, -1867, -1867, -1867,
   -1867,   722, -1867, 15332, 15332, -1867, -1867, -1867, -1867, -1867,
   -1867, -1867, -1867, -1867, 15332, 15332, 15332, 15332, 14240, 15332,
   15332, 15332, 15332, 15332, 15332, 15332, 15332, 15332, 15332, 15332,
   15332, 15332, 15332, 15332, 15332, 15332, 15332, 15332, 15332, 15332,
   15332, 15332, 19423, 15332, -1867, 15332, 15332, 15332,  5389,   578,
     578,   578,   578,   578,  5456,   944,   781, 11599, 15332, 15332,
   15332, 15332, 15332, 15332, 15332, 15332, 15332, 15332, 15332, 15332,
   -1867, -1867, -1867, -1867,  2692, -1867, -1867, 11975, 11975, 15332,
   15332, 18190,   860,   722, 14419, 17624, -1867, 15332, -1867,   873,
    1064,   916,   876,   878, 15684,   562, 14598, -1867, 14777, -1867,
     802,   881,   882,  2483, -1867,   382, 11975, -1867,  3792, -1867,
   -1867, 17672, -1867, -1867, 12351, -1867, 15332, -1867,   988, 10569,
    1077,   887, 19660,  1075,   153,   102, -1867, -1867, -1867,   907,
   -1867, -1867, -1867, 17807, -1867,  1866,   898,  1083, 18038,   578,
   -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867,   901,
   -1867, -1867,   894,   905,   906,   914,   917,   919,   256,   925,
     920,  2457, 16089, -1867, -1867,   578,   578, 15332,   562,   257,
   -1867, 18038,  1046, -1867, -1867, -1867,   562,   156,   158,   930,
     932,  2859,   379,   939,   933,   692,  1006,   952,   562,   160,
     943, 18729,   942,  1146,  1147,   955,   957,   961,   962, -1867,
    3834,   578, -1867, -1867,  1097,  3283,   514, -1867, -1867, -1867,
     571, -1867, -1867, -1867,  1138,  1044,   999,   179,  1021, 15332,
    1048,  1175,   987, -1867,  1030, -1867,   208, -1867,   993, 17807,
   17807,  1180,   996,   127, -1867,  1000,  1186, -1867, 17711,   393,
   -1867,   461,   222, -1867, -1867, -1867, -1867, -1867, -1867, -1867,
     766,  3386, -1867, -1867, -1867, -1867,  1190,  1018, -1867, 18447,
     730, 15332,  1003,  1195, 19782,  1191,   162,  1200,  1010,  1019,
    1022, 19782,  1023,  2924,  6655, -1867, -1867, -1867, -1867, -1867,
   -1867,  1078,  5237, 19782,  1007,  3483, 19921, 20012, 16745, 16376,
   15332, 19734, 20093, 15685, 20198, 20231, 16197, 19430, 20262, 20262,
   20262, 20262,  2169,  2169,  2169,  2169,  2169,  1011,  1011,   862,
     862,   862,   491,   491,   491, -1867,   870,  1014,  1020, 18777,
    1026,  1206,     2, 15332,    55,   761,   205, -1867, -1867, -1867,
    1207,   934, -1867,   722, 18295, -1867, -1867, -1867, 16745, 16745,
   16745, 16745, 16745, 16745, 16745, 16745, 16745, 16745, 16745, 16745,
   16745, -1867, 15332,   258, -1867,   210, -1867,   761,   389,  1031,
    1032,  1029,  4064,   165,  1027, -1867, 19782,  4562, -1867,   578,
   -1867,   562,   480, 18447, 19782, 18447, 18838,  1078,   372,   562,
     214, -1867,   208,  1068,  1035, 15332, -1867,   221, -1867, -1867,
   -1867,  6861,   741, -1867, -1867, 19782, 19782,    40, -1867, -1867,
   -1867, 15332,  1136, 17962, 18038,   578, 10775,  1039,  1040, -1867,
    1233, 15910,  1105, -1867,  1082, -1867,  1236,  1047,  4731, 17807,
   18038, 18038, 18038, 18038, 18038,  1049,  1176,  1179,  1183,  1184,
    1187,  1051,  1057, 18038,    33, -1867, -1867, -1867, -1867, -1867,
   -1867, -1867,    42, -1867, 19876, -1867, -1867,    29, -1867,  7067,
    5657,  1061, 16089, -1867, 16089, -1867, 16089, -1867,   578,   578,
   16089, -1867, 16089, 16089,   578, -1867,  1259,  1067, -1867,   441,
   -1867, -1867,  4116, -1867, 19876,  1258, 18447,  1072, -1867, -1867,
   -1867, -1867, -1867, -1867, -1867, -1867, -1867,  1091,  1267,   578,
    5657,  1080, 18190, 18371,  1269, -1867, 15332, -1867, 15332, -1867,
   15332, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867,  1087,
   -1867, 15332, -1867, -1867,  6037, -1867, 17807,  5657,  1084, -1867,
   -1867, -1867, -1867,  1108,  1273,  1093, 15332, 19347, -1867, -1867,
    5389, -1867,  1094, -1867, 17807, -1867,  1106,  7273,  1274,    87,
   -1867, 17807, -1867,   125,  2692,  2692, -1867, 17807, -1867, -1867,
     562, -1867, -1867,  1231, 19782, -1867, 12154, -1867, 18038, 14061,
     853, 14061, -1867,   853,   853, -1867, 12557, -1867, -1867,  7479,
   -1867,    95,  1113,  5657,  1044, -1867, -1867, -1867, -1867, 20093,
   15332, -1867, -1867, 15332, -1867, 15332, -1867,  4299,  1115, 11975,
    1006,  1275,  1044, 17807,  1301,  1078,   578, 19423,   562,  4430,
    1117,   249,  1118, -1867, -1867,  1304,   654,   654,  4562, -1867,
   -1867, -1867,  1268,  1124,  1255,  1257,  1261,  1262,  1264,    90,
    1137,  1140,   481, -1867, -1867, -1867, -1867, -1867, -1867,  1173,
   -1867, -1867, -1867, -1867,  1328,  1141,   873,   562,   562, 14956,
    1044,  3792, -1867,  3792, -1867,  4540,   784,    40, 11393, -1867,
    7685,  1142,  7891,  1143, 17962, 18447,  1149,  1204,   562, 19876,
    1335, -1867, -1867, -1867, -1867,   815, -1867,   396, 17807,  1166,
    1214,  1192, 17807,   578,  3714, -1867, -1867, 17807,  1344,  1155,
    1181,  1182,  1190,   814,   814,  1287,  1287, 18995,  1157,  1350,
   18038, 18038, 18038, 18038, 18038, 18038, 19347, 18038, 17720, 17091,
   18038, 18038, 18038, 18038, 17886, 18038, 18038, 18038, 18038, 18038,
   18038, 18038, 18038, 18038, 18038, 18038, 18038, 18038, 18038, 18038,
   18038, 18038, 18038, 18038, 18038, 18038, 18038, 18038,   578, -1867,
   -1867,  1282, -1867, -1867,  1164,  1165,  1167, -1867,  1168, -1867,
   -1867,   525,  2457, -1867,  1171, -1867, 18038,   562, -1867, -1867,
     138, -1867,   809,  1360, -1867, -1867,   168,  1177,   562, 12754,
   19782, 18886, -1867,  3097, -1867,  6243,   934,  1360, -1867,   602,
   15332,    50, -1867, 19782,  1240,  1188, -1867,  1185,  1274, -1867,
   -1867, -1867, 15153, 17807,   996, 17759,  1306,   101,  1368,  1308,
     356, -1867,   761,   368, -1867,   761, -1867, 15332, 18447,   730,
   15332, 19782, 19876, -1867, -1867, -1867,  5139, -1867, -1867, -1867,
   -1867, -1867, -1867,  1196,    95, -1867,  1193,    95,  1198, 20093,
   19782, 18947,  1199, 11975,  1201,  1197, 17807,  1202,  1205, 17807,
    1044, -1867,   802,   473, 11975, 15332, -1867, -1867, -1867, -1867,
   -1867, -1867,  1254,  1203,  1391,  1313,  4562,  4562,  4562,  4562,
    4562,  4562,  1249, -1867, 19347,  4562,   119,  4562, -1867, -1867,
   -1867, 18447, 19782,  1208, -1867,    40,  1379,  1336, 11393, -1867,
   -1867, -1867,  1213, 15332,  1204,   562, 18190, 17962,  1215, 18038,
    8097,   864,  1216, 15332,    88,   501, -1867,  1232, -1867, 17807,
     578, -1867,  1281, -1867, -1867, -1867,  5166, -1867,  1395, -1867,
    1228, 18038, -1867, 18038, -1867,  1229,  1227,  1423, 19055,  1230,
   19876,  1424,  1234,  1239,  1241,  1297,  1439,  1250,  1251, -1867,
   -1867, -1867, 19101,  1246,  1449, 19968, 20056, 16566, 18038, 19830,
   20164,  4177, 17970, 18379, 20293, 20324, 20324, 20324, 20324,  2671,
    2671,  2671,  2671,  2671,  1121,  1121,   814,   814,   814,  1287,
    1287,  1287,  1287, -1867,  1265, -1867,  1260,  1263,  1266,  1271,
   -1867, -1867, 19876,   578, 17807, 17807, -1867,   809,  5657,   100,
   -1867, 18190, -1867, -1867, 16745, 15332,  1256, -1867,  1276,  1299,
   -1867,    97, 15332, -1867, -1867,  5069, -1867, 15332, -1867, 15332,
   -1867,   996, 14061,  1279,   374,   853,   374,   344, -1867, -1867,
   17807,   154, -1867,  1448,  1383, 15332, -1867,  1277,  1278,  1280,
     562,  1231, 19782,  1274,  1270, -1867,  1285,    95, 15332, 11975,
    1292, -1867, -1867,   934, -1867, -1867,  1293,  1294,  1296, -1867,
    1300,  4562, -1867,  4562, -1867, -1867,  1303,  1302,  1466,  1359,
    1309, -1867,  1495,  1314,  1315,  1316, -1867,  1367,  1307,  1499,
    1312, -1867, -1867,   562, -1867,  1485, -1867,  1320, -1867, -1867,
    1322,  1323,   169, -1867, -1867, 19876,  1325,  1327, -1867, 17310,
   -1867, -1867, -1867, -1867, -1867, -1867,  1386, 17807, 17807,  1181,
    1353, 17807, -1867, 19876, 19161, -1867, -1867, 18038, -1867, 18038,
   -1867, 18038, -1867, -1867, -1867, -1867, 18038, 19347, -1867, -1867,
   -1867, 18038, -1867, 18038, -1867, 20129, 18038,  1331,  8303, -1867,
   -1867, -1867, -1867,   809, -1867, -1867, -1867, -1867,   822, 16375,
    5657,  1417,  1422,  1426,  1430, -1867,  3515,  1365,  1950, -1867,
   -1867,  1454,   944,  3598,  1431,   137,   140,  1345,   934,   781,
     170, 19782, -1867, -1867, -1867,  1378,  5247, -1867, 17262, 19782,
   -1867,  3341, -1867,  6655,  1464,   428,  1534,  1467, 15332, -1867,
   19782, 11975, 11975, -1867,  1432,  1274,  2158,  1274,  1349, 19782,
    1352, -1867,  2224,  1355,  2409, -1867, -1867,    95, -1867, -1867,
    1416, -1867, -1867,  4562, -1867,  4562, -1867,  4562, -1867, -1867,
   -1867, -1867,  4562, -1867, 19347, -1867, -1867,  2535, -1867,  8509,
   -1867, -1867, -1867, -1867, 10981, -1867, -1867, -1867,  6861, 17807,
   -1867, -1867, -1867,  1358, 18038, 19207, 19876, 19876, 19876,  1421,
   19876, 19267, 20129, -1867, -1867,   809,  5657,  5657,   578, -1867,
    1546,  5609,   104, -1867, 16375,   934, 16830, -1867,  1381, -1867,
     141,  1363,   146, -1867, 16744, -1867, -1867, -1867,   147, -1867,
   -1867,  5044, -1867,  1361, -1867,  1547,   149,   722,  1454, 16565,
   16565, -1867, 16565, -1867, -1867,  1553,   944, -1867, 15838, -1867,
   -1867, -1867, -1867,  3145, -1867,  1554,  1486, 15332, -1867, 19782,
    1370,  1372,  1374,  1274,  1376, -1867,  1432,  1274, -1867, -1867,
   -1867, -1867,  2565,  1375,  4562,  1438, -1867, -1867, -1867,  1440,
   -1867,  6861, 11187, 10981, -1867, -1867, -1867,  6861, -1867, -1867,
   19876, 18038, 18038, 18038,  8715,  1380,  1384, -1867, 18038, -1867,
    5657, -1867, -1867, -1867, -1867, -1867, 17807,  2616,  3515, -1867,
   -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867, -1867, -1867,   757, -1867,  1365, -1867, -1867, -1867, -1867,
   -1867,   144,   589, -1867, 18038,  1496, -1867, 16937,   150,  1566,
    1568, -1867, 17807,   722, -1867, -1867,  1387,  1570, 15332, -1867,
   19782, -1867, -1867,   364,  1388, 17807,   748,  1274,  1376, 16017,
   -1867,  1274, -1867,  4562,  4562, -1867, -1867, -1867, -1867,  8921,
   19876, 19876, 19876, -1867, -1867, -1867, 19876, -1867,  1824,  1580,
    1582,  1389, -1867, -1867, 18038, 16744, 16744,  1526, -1867,  5044,
    5044,   665, -1867, -1867, -1867, 19876,  1581,  1413,  1398, -1867,
   18038, 18038, -1867, 16937, -1867, 18038, 19782,  1516, -1867,  1591,
   -1867,  1592, -1867,   159, -1867, -1867, -1867,  1400,   748, -1867,
     748, -1867, -1867,  9127,  1405,  1491, -1867,  1506,  1450, -1867,
   -1867,  1507, 17807,  1427,  2616, -1867, -1867, 19876, -1867, -1867,
    1441, -1867,  1578, -1867, -1867, -1867, -1867, 18038,   692, -1867,
   19876, 19876,  1419, 19876, -1867,   381,  1420,  9333, 17807, -1867,
   17807, -1867,  9539, -1867, -1867, -1867,  1418, -1867,  1425,  1442,
     578,   781,  1437, -1867, -1867, -1867, 19876,  1443,   113, -1867,
    1540, -1867, -1867, -1867, -1867, -1867, -1867,  9745, -1867,  5657,
    1061, -1867,  1451,   578,   642, -1867, -1867,  1433,  1616,   528,
     113, -1867, -1867,  1548, -1867,  5657,  1434, -1867,  1274,   139,
   -1867, 17807, -1867, -1867, -1867, 17807, -1867,  1452,  1455,   151,
   -1867,  1376,   694,  1549,   173,  1274,  1446, -1867,   776, 17807,
   17807, -1867,   442,  1620,  1556,  1376, -1867, -1867, -1867, -1867,
    1558,   196,  1629,  1567, 15332, -1867,   776,  9951, 10157, -1867,
     453,  1637,  1571, 15332, -1867, 19782, -1867, -1867, -1867,  1643,
    1575, 15332, -1867, 19782, 15332, -1867, 19782, 19782
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   204,   472,     0,   913,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1008,
     996,     0,   777,     0,   783,   784,   785,    29,   850,   984,
     985,   171,   172,   786,     0,   152,     0,     0,     0,     0,
      30,     0,     0,     0,     0,   223,     0,     0,     0,     0,
       0,     0,   440,   441,   442,   439,   438,   437,     0,     0,
       0,     0,   252,     0,     0,     0,    37,    38,    40,    41,
      39,   790,   792,   793,   787,   788,     0,     0,     0,   794,
     789,     0,   760,    32,    33,    34,    36,    35,     0,   791,
       0,     0,     0,     0,     0,   795,   443,   582,    31,     0,
     170,   140,     0,   778,     0,     0,     4,   126,   128,   849,
       0,   759,     0,     6,     0,     0,   222,     7,     9,     8,
      10,     0,     0,   435,   967,   486,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   542,   484,   972,   973,   564,
     557,   558,   559,   560,   563,   561,   562,   467,   567,     0,
     466,   941,   761,   768,     0,   852,   556,   434,   944,   945,
     957,   485,     0,     0,     0,   488,   487,   942,   943,   940,
     980,   983,   546,   851,    11,   440,   441,   442,     0,     0,
      36,     0,   126,   222,     0,  1048,   485,  1049,     0,  1051,
    1052,   566,   480,     0,   473,   478,     0,     0,   528,   529,
     530,   531,    29,   984,   786,   763,    37,    38,    40,    41,
      39,     0,     0,  1072,   963,   761,     0,   762,   507,     0,
     509,   547,   548,   549,   550,   551,   552,   553,   555,     0,
    1012,     0,   859,   773,   242,     0,  1072,   464,   772,   766,
       0,   782,   762,   991,   992,   998,   990,   774,     0,   465,
       0,   776,   554,     0,   205,     0,     0,   469,   205,   150,
     471,     0,     0,   156,   158,     0,     0,   160,     0,    75,
      76,    82,    83,    67,    68,    59,    80,    91,    92,     0,
      62,     0,    66,    74,    72,    94,    86,    85,    57,   108,
      81,   101,   102,    58,    97,    55,    98,    56,    99,    54,
     103,    90,    95,   100,    87,    88,    61,    89,    93,    53,
      84,    69,   104,    77,   106,    70,    60,    47,    48,    49,
      50,    51,    52,    71,   107,   105,   110,    64,    45,    46,
      73,  1125,  1126,    65,  1130,    44,    63,    96,     0,    79,
       0,   126,   109,  1063,  1124,     0,  1127,     0,     0,     0,
     162,     0,     0,     0,   213,     0,     0,     0,     0,     0,
       0,     0,     0,   861,     0,   114,   116,   350,     0,     0,
     349,   355,     0,     0,   253,     0,   256,     0,     0,     0,
       0,  1069,   238,   250,  1004,  1008,   601,   631,   631,   601,
     631,     0,  1033,     0,   797,     0,     0,     0,  1031,     0,
      16,     0,   130,   230,   244,   251,   661,   594,   631,     0,
    1057,   574,   576,   578,   917,   472,   486,     0,     0,   484,
     485,   487,   205,     0,   987,   779,     0,   780,     0,     0,
       0,   202,     0,     0,   132,   339,     0,    28,     0,     0,
       0,     0,     0,   203,   221,     0,   249,   234,   248,   440,
     443,   222,   436,   989,     0,   933,   192,   193,   194,   195,
     196,   198,   199,   201,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   996,     0,   191,   989,   989,  1018,     0,     0,
       0,     0,     0,     0,     0,     0,   433,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     506,   508,   918,   919,     0,   932,   931,   339,   339,   989,
       0,  1004,     0,   222,     0,     0,   164,     0,   915,   910,
     859,     0,   486,   484,     0,  1016,     0,   599,   858,  1007,
     782,   486,   484,   485,   132,     0,   339,   463,     0,   934,
     775,     0,   140,   292,     0,   581,     0,   167,     0,   205,
     470,     0,     0,     0,     0,     0,   159,   190,   161,  1125,
    1126,  1122,  1123,     0,  1129,  1115,     0,     0,     0,     0,
      78,    43,    65,    42,  1064,   197,   200,   163,   140,     0,
     180,   189,     0,     0,     0,     0,     0,     0,   117,     0,
       0,     0,   860,   115,    18,     0,   111,     0,   351,     0,
     165,     0,     0,   166,   254,   255,  1053,     0,     0,   486,
     484,   485,   488,   487,     0,  1105,   262,     0,  1005,     0,
       0,     0,     0,   859,   859,     0,     0,     0,     0,   168,
       0,     0,   796,  1032,   850,     0,     0,  1030,   855,  1029,
     129,     5,    13,    14,     0,   260,     0,     0,   587,     0,
       0,   859,     0,   770,     0,   769,   764,   588,     0,     0,
       0,     0,     0,   917,   136,     0,   861,   916,  1134,   462,
     475,   489,   950,   971,   147,   139,   143,   144,   145,   146,
     434,     0,   565,   853,   854,   127,   859,     0,  1073,     0,
       0,     0,     0,   861,   340,     0,     0,     0,   486,   209,
     210,   208,   484,   485,   205,   184,   182,   183,   185,   570,
     224,   258,     0,   988,     0,     0,   512,   514,   513,   525,
       0,     0,   545,   510,   511,   515,   517,   516,   534,   535,
     532,   533,   536,   537,   538,   539,   540,   526,   527,   519,
     520,   518,   521,   522,   524,   541,   523,     0,     0,  1022,
       0,   859,  1056,     0,  1055,  1072,   947,   240,   232,   246,
       0,  1057,   236,   222,     0,   476,   479,   481,   491,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   921,     0,   920,   923,   946,   927,  1072,   924,     0,
       0,     0,     0,     0,     0,  1050,   474,   908,   912,   858,
     914,   461,   765,     0,  1011,     0,  1010,   258,     0,   765,
     995,   994,   980,   983,     0,     0,   920,   923,   993,   924,
     483,   294,   296,   136,   585,   584,   468,     0,   140,   276,
     151,   471,     0,     0,     0,     0,   205,   288,   288,   157,
     859,     0,     0,  1114,     0,  1111,   859,     0,  1085,     0,
       0,     0,     0,     0,   857,     0,    37,    38,    40,    41,
      39,     0,     0,     0,   799,   803,   804,   805,   808,   806,
     807,   810,     0,   798,   134,   848,   809,  1072,  1128,   205,
       0,     0,     0,    21,     0,    22,     0,    19,     0,   112,
       0,    20,     0,     0,     0,   123,   861,     0,   121,   116,
     113,   118,     0,   348,   356,   353,     0,     0,  1042,  1047,
    1044,  1043,  1046,  1045,    12,  1103,  1104,     0,   859,     0,
       0,     0,  1004,  1001,     0,   598,     0,   612,   858,   600,
     858,   630,   615,   624,   627,   618,  1041,  1040,  1039,     0,
    1035,     0,  1036,  1038,   205,     5,     0,     0,     0,   656,
     657,   666,   665,     0,     0,   484,     0,   858,   593,   597,
       0,   621,     0,  1058,     0,   575,     0,   205,  1092,   917,
     320,  1134,  1133,     0,     0,     0,   986,   858,  1075,  1071,
     342,   336,   337,   341,   343,   758,   860,   338,     0,     0,
       0,     0,   461,     0,     0,   489,     0,   951,   212,   205,
     142,   917,     0,     0,   260,   572,   226,   929,   930,   544,
       0,   638,   639,     0,   636,   858,  1017,     0,     0,   339,
     262,     0,   260,     0,     0,   258,     0,   996,   492,     0,
       0,   948,   949,   981,   982,     0,     0,     0,   896,   866,
     867,   868,   875,     0,    37,    38,    40,    41,    39,     0,
       0,     0,   881,   887,   888,   889,   892,   890,   891,     0,
     879,   877,   878,   902,   859,     0,   910,  1015,  1014,     0,
     260,     0,   935,     0,   781,     0,   298,     0,   205,   148,
     205,     0,   205,     0,     0,     0,     0,   268,   269,   280,
       0,   140,   278,   177,   288,     0,   288,     0,   858,     0,
       0,     0,     0,     0,   858,  1113,  1116,  1081,   859,     0,
    1076,     0,   859,   831,   832,   829,   830,   865,     0,   859,
     857,   605,   633,   633,   605,   633,   596,   633,     0,     0,
    1024,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1119,
     214,     0,   217,   181,     0,     0,     0,   119,     0,   124,
     125,   117,   860,   122,     0,   352,     0,  1054,   169,  1070,
    1105,  1096,  1100,   261,   263,   362,     0,     0,  1002,     0,
     603,     0,  1034,     0,    17,   205,  1057,   259,   362,     0,
       0,     0,   765,   590,     0,   771,  1059,     0,  1092,   579,
     135,   137,     0,     0,     0,  1134,     0,     0,   325,   323,
     923,   936,  1072,   923,   937,  1072,  1074,   989,     0,     0,
       0,   344,   133,   207,   209,   210,   485,   188,   206,   186,
     187,   211,   141,     0,   917,   257,     0,   917,     0,   543,
    1021,  1020,     0,   339,     0,     0,     0,     0,     0,     0,
     260,   228,   782,   922,   339,     0,   871,   872,   873,   874,
     882,   883,   900,     0,   859,     0,   896,   609,   635,   635,
     609,   635,     0,   870,   904,   635,     0,   858,   907,   909,
     911,     0,  1009,     0,   922,     0,     0,     0,   205,   295,
     586,   153,     0,   471,   268,   270,  1004,     0,     0,     0,
     205,     0,     0,     0,     0,     0,   282,     0,  1120,     0,
       0,  1106,     0,  1112,  1110,  1077,   858,  1083,     0,  1084,
       0,     0,   801,   858,   856,     0,     0,   859,     0,     0,
     845,   859,     0,     0,     0,     0,   859,     0,     0,   811,
     846,   847,  1028,     0,   859,   814,   816,   815,     0,     0,
     812,   813,   817,   819,   818,   835,   836,   833,   834,   837,
     838,   839,   840,   841,   826,   827,   821,   822,   820,   823,
     824,   825,   828,  1118,     0,   140,     0,     0,     0,     0,
     120,    23,   354,     0,     0,     0,  1097,  1102,     0,   434,
    1006,  1004,   477,   482,   490,     0,     0,    15,     0,   434,
     669,     0,     0,   671,   664,     0,   667,     0,   663,     0,
    1061,     0,     0,     0,   967,   542,     0,   488,  1093,   583,
    1134,     0,   326,   327,     0,     0,   321,     0,     0,     0,
     346,   347,   345,  1092,     0,   362,     0,   917,     0,   339,
       0,   978,   362,  1057,   362,  1060,     0,     0,     0,   493,
       0,     0,   885,   858,   895,   876,     0,     0,   859,     0,
       0,   894,   859,     0,     0,     0,   869,     0,     0,   859,
       0,   880,   901,  1013,   362,     0,   140,     0,   291,   277,
       0,     0,     0,   267,   173,   281,     0,     0,   284,     0,
     289,   290,   140,   283,  1121,  1107,     0,     0,  1080,  1079,
       0,     0,  1132,   864,   863,   800,   613,   858,   604,     0,
     616,   858,   632,   625,   628,   619,     0,   858,   595,   802,
     622,     0,   637,   858,  1023,   843,     0,     0,   205,    24,
      25,    26,    27,  1099,  1094,  1095,  1098,   264,     0,     0,
       0,   441,   439,   438,   437,   432,     0,     0,     0,   239,
     361,     0,     0,   431,     0,     0,     0,     0,  1057,   434,
       0,   602,  1037,   358,   245,   659,     0,   662,     0,   589,
     577,   485,   138,   205,     0,     0,   330,   319,     0,   322,
     329,   339,   339,   335,   569,  1092,   434,  1092,     0,  1019,
       0,   977,   434,     0,   434,  1062,   362,   917,   974,   899,
     898,   884,   614,   858,   608,     0,   617,   858,   634,   626,
     629,   620,     0,   886,   858,   903,   623,   434,   140,   205,
     149,   154,   175,   271,   205,   279,   285,   140,   287,     0,
    1108,  1078,  1082,     0,     0,     0,   607,   844,   592,     0,
    1027,  1026,   842,   140,   218,  1101,     0,     0,     0,  1065,
       0,     0,     0,   265,     0,  1057,     0,   397,   393,   399,
     760,    36,     0,   387,     0,   392,   396,   409,     0,   407,
     412,     0,   411,     0,   410,   451,     0,   222,     0,     0,
       0,   365,     0,   366,   367,     0,     0,  1003,     0,   660,
     658,   670,   668,     0,   331,   332,     0,     0,   317,   328,
       0,     0,     0,  1092,  1086,   235,   569,  1092,   979,   241,
     358,   247,   434,     0,     0,     0,   611,   893,   906,     0,
     243,   293,   205,   205,   140,   274,   174,   286,  1109,  1131,
     862,     0,     0,     0,   205,     0,     0,   460,     0,  1066,
       0,   377,   381,   457,   458,   391,     0,     0,     0,   372,
     719,   720,   718,   721,   722,   739,   741,   740,   710,   682,
     680,   681,   700,   715,   716,   676,   687,   688,   690,   689,
     757,   709,   693,   691,   692,   694,   695,   696,   697,   698,
     699,   701,   702,   703,   704,   705,   706,   708,   707,   677,
     678,   679,   683,   684,   686,   756,   724,   725,   729,   730,
     731,   732,   733,   734,   717,   736,   726,   727,   728,   711,
     712,   713,   714,   737,   738,   742,   744,   743,   745,   746,
     723,   748,   747,   750,   752,   751,   685,   755,   753,   754,
     749,   735,   675,   404,   672,     0,   373,   425,   426,   424,
     417,     0,   418,   374,     0,     0,   363,     0,     0,     0,
       0,   456,     0,   222,   231,   357,     0,     0,     0,   318,
     334,   975,   976,     0,     0,     0,     0,  1092,  1086,     0,
     237,  1092,   897,     0,     0,   140,   272,   155,   176,   205,
     606,   591,  1025,   216,   375,   376,   454,   266,     0,   859,
     859,     0,   400,   388,     0,     0,     0,   406,   408,     0,
       0,   413,   420,   421,   419,   452,   449,  1067,     0,   364,
       0,     0,   459,     0,   359,     0,   333,     0,   654,   861,
     136,   861,  1088,     0,   427,   136,   225,     0,     0,   233,
       0,   610,   905,   205,     0,   178,   378,   126,     0,   379,
     380,     0,   858,     0,   858,   402,   398,   403,   673,   674,
       0,   389,   422,   423,   415,   416,   414,     0,  1105,   368,
     455,   453,     0,   360,   655,   860,     0,   205,   860,  1087,
       0,  1091,   205,   136,   227,   229,     0,   275,     0,   220,
       0,   434,     0,   394,   401,   405,   450,     0,   917,   370,
       0,   652,   568,   571,  1089,  1090,   428,   205,   273,     0,
       0,   179,   385,     0,   433,   395,  1068,     0,   861,   445,
     917,   653,   573,     0,   219,     0,     0,   384,  1092,   917,
     302,  1134,   448,   447,   446,  1134,   444,     0,     0,     0,
     383,  1086,   445,     0,     0,  1092,     0,   382,     0,  1134,
    1134,   308,     0,   307,   305,  1086,   140,   429,   136,   369,
       0,     0,   309,     0,     0,   303,     0,   205,   205,   313,
       0,   312,   301,     0,   304,   311,   371,   215,   430,   314,
       0,     0,   299,   310,     0,   300,   316,   315
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1867, -1867, -1867,  -570, -1867, -1867, -1867,   537,   -47,   -29,
     478, -1867,  -231,  -514, -1867, -1867,   457,   567,  1919, -1867,
    2956, -1867,  -796, -1867,  -530, -1867,  -704,    34, -1867, -1867,
   -1867, -1867, -1867, -1867, -1867, -1867,  -938, -1867, -1867,  -916,
    -354, -1867, -1867, -1867,  -378, -1867, -1867,  -179,    96,    39,
   -1867, -1867, -1867, -1867, -1867, -1867,    44, -1867, -1867, -1867,
   -1867, -1867, -1867,    49, -1867, -1867,  1151,  1154,  1156,   -85,
    -735,  -924,   616,   691,  -386,   337, -1005, -1867,   -89, -1867,
   -1867, -1867, -1867,  -769,   152, -1867, -1867, -1867, -1867,  -375,
   -1867,  -601, -1867,   417,  -472, -1867, -1867,  1053, -1867,   -73,
   -1867, -1867, -1139, -1867, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867,  -110, -1867,   -12, -1867, -1867, -1867, -1867, -1867,  -190,
   -1867,    98,  -972, -1867, -1761,  -409, -1867,  -142,   148,  -130,
    -383, -1866,   -27, -1867, -1867, -1867,   111,  -109,   -75,   -11,
    -766,   -65, -1867, -1867,    30, -1867,    58,  -385, -1867,     4,
      -5,   -64,   -77,   -53, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867, -1867, -1867,  -620,  -913, -1867, -1867, -1867, -1867, -1867,
     546,  1298, -1867,   541, -1867,   390, -1867, -1867, -1867, -1867,
   -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867, -1867, -1867, -1867, -1867,   435,  -490,  -556, -1867, -1867,
   -1867, -1867, -1867,   470, -1867, -1867, -1867, -1867, -1867, -1867,
   -1867, -1867,  -945,  -356,  2873,    47, -1867,   195,  -431, -1867,
   -1867,  -480,  3891,  3807, -1867,   674, -1867, -1867,   551,   504,
    -665, -1867, -1867,   635,   398,   373, -1867,   399, -1867, -1867,
   -1867, -1867, -1867,   608, -1867, -1867, -1867,    86,  -915,  -145,
    -446,  -441, -1867,    -8,  -137, -1867, -1867,    53,    61,   720,
     -62, -1867, -1867,     9,   -68, -1867,  -350,    38,  -366,   223,
    -420, -1867, -1867,  -485,  1311, -1867, -1867, -1867, -1867, -1867,
     880,   490, -1867, -1867, -1867,  -341,  -721, -1867,  1283, -1283,
    -246,   -58,  -114,   829, -1867, -1867, -1867, -1797, -1867,  -298,
   -1041, -1358,  -287,   142, -1867,   503,   585, -1867, -1867, -1867,
   -1867,   533, -1867,   947,  -799
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   116,   975,   671,   192,  1689,   785,
     372,   373,   374,   375,   926,   927,   928,   118,   119,   120,
     121,   122,   997,  1240,   432,  1029,   705,   706,   579,   268,
    1762,   585,  1664,  1763,  2019,   911,   124,   125,   726,   727,
     735,   365,   608,  1975,  1193,  1415,  2041,   455,   193,   707,
    1032,  1278,  1488,   128,   674,  1051,   708,   741,  1055,   646,
    1050,   247,   560,   709,   675,  1052,   457,   392,   414,   131,
    1034,   978,   951,  1213,  1692,  1338,  1117,  1917,  1766,   860,
    1123,   584,   869,  1125,  1532,   852,  1106,  1109,  1327,  2047,
    2048,   695,   696,  1013,   722,   723,   379,   380,   382,  1728,
    1895,  1896,  1429,  1590,  2028,  2050,  1928,  1979,  1980,  1981,
    1702,  1703,  1704,  1705,  1930,  1931,  1937,  1991,  1708,  1709,
    1713,  1880,  1881,  1882,  1966,  2089,  1591,  1592,   194,   133,
    2065,  1594,  1716,  1595,  1596,  1597,  1598,   134,   135,   654,
     581,   136,   137,   138,   139,   140,   141,   142,   143,   261,
     144,   145,   146,  1743,   147,  1031,  1277,   148,   692,   693,
     694,   265,   424,   575,   680,   681,  1376,   682,  1377,   149,
     150,   652,   653,  1366,  1367,  1497,  1498,   151,   895,  1083,
     152,   896,  1084,   153,   897,  1085,   154,   898,  1086,   155,
     899,  1087,   156,   900,  1088,   655,  1369,  1500,   157,   901,
     158,   159,  1959,   160,   676,  1730,   677,  1229,   984,  1448,
    1444,  1873,  1874,   161,   162,   163,   250,   164,   251,   262,
     436,   567,   165,  1370,  1371,   905,   906,   166,  1148,   559,
     623,  1149,  1091,  1300,  1092,  1501,  1502,  1303,  1304,  1094,
    1508,  1509,  1095,   828,   550,   206,   207,   710,   698,   534,
    1250,  1251,   816,   817,   465,   168,   253,   169,   170,   196,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     744,   257,   258,   649,   241,   242,   780,   781,  1383,  1384,
     407,   408,   969,   182,   637,   183,   691,   184,   355,  1897,
    1948,   393,   444,   716,   717,  1138,  1139,  1906,  1961,  1962,
    1244,  1426,   947,  1427,   948,   949,   875,   876,   877,   356,
     357,   908,   594,  1002,  1003
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     195,   197,   462,   199,   200,   201,   202,   204,   205,   352,
     208,   209,   210,   211,   542,   449,   231,   232,   233,   234,
     235,   236,   237,   238,   240,   515,   259,   353,   229,   229,
    1030,  1000,   429,   266,   426,   535,   536,   264,   123,   267,
     415,   431,   851,   127,   427,   419,   420,   275,   129,   278,
     269,   686,   363,   130,   366,   273,   823,  1110,  1017,  1242,
     450,   683,   451,   730,   462,   819,   820,   685,   256,  1576,
    1054,   775,   995,   458,  1234,   401,   687,   249,   909,  1141,
     837,   267,   514,   254,  1245,   777,   778,   361,   814,  1439,
     167,   255,   996,   815,   844,  1113,   736,   737,   738,  1127,
     126,   974,  1100,  1263,   428,  1268,  1578,   925,   930,  1334,
    1276,  1968,   568,  1780,   429,   362,   426,   446,   576,   821,
    1530,   569,   847,   431,   -43,   -78,   -42,   848,  1287,   -43,
     -78,   -42,   564,   452,   867,    14,   613,   615,   617,   629,
     620,   632,   576,    14,  -952,   553,  1720,   552,    14,  1722,
    -390,  1605,   132,  1939,   431,  1788,  1875,   625,  1885,  1885,
    1780,    14,   865,  1247,   551,   936,   381,   576,   562,   953,
     561,  1019,  1242,  2010,   953,    14,  1323,   953,   953,   953,
    1940,  1312,   609,  2066,  1463,  -110,   428,    14,   945,   946,
     402,  1188,  1615,   545,   442, -1072,   198,  1451,   435,   842,
    1511,  -110,  1246,     3,  1159,  -954,  2066,  2014,  1248,  2015,
     260,  2082,   443,  1579,   226,   226,   443,   428,  1446,  1580,
     626,   459,  1581,   187,  1582,  1583,  1584,  1585,   461,   672,
     673,   463,   404,   980,  2100, -1072,  -770,  1616,  -109,   263,
     428,   660,  -997,  1375,  1160,  -763,   610,  -955,   229,  1313,
    -651,  1380,  1447,  -953,  -109,   571,  2083,   661,   571,   580,
    -640,  1203,  1424,  1425,   416,   267,   582,   405,   406,  1586,
    1587,  -580,  1588,  -962,  2078,  -959,  -952,  1464,   539,  2101,
    -956,  -649,   742,  -860, -1000,   396,   539,  -860,  2096,  1531,
     543,  -999,  -951,  1241,   460,  -324,  1690,  -769,   573,   593,
    -648,  1249,   578,  1589,   868,  1781,  1782,   532,   533,   447,
     577,   352,   640,  -306,   639,   516,   -43,   -78,   -42,  -958,
    1290,   643,  -858,  -324,   364,  1272,  1453,  1576,  1112,   604,
    1617,   630,  1523,   633,   659,   434,  1626,  -954,  1721,  -860,
     377,  1723,  -390,  1632,  1941,  1634,   981,  1789,  1876,  2084,
    1886,  1949,  2077,   866,   202,  1341,   937,  1345,   938,   918,
     954,   982,  1020,   464,   824,  1065,  1487,  -771,  1430,  1663,
    1727,   732,  2102,   728,  -997,  1657,  -961,   638,  -764,  -955,
     431,   983,  -968,   267,   428,  -953,   462,   229,  -762,  -963,
     240,   651,   267,   267,   651,   267,   229,   740,   642,   352,
     665,  1507,  1957,   229,  1049,  1225,  1241,  -959,  1199,  1200,
     540,  1783,  -956,   267,  -648,   229, -1000,   353,   540,  2030,
     204,  -965,   919,  -999,  -951,  -964,  -938,   270,   711,   538,
    1273,   271,  1624,   421,   226,   385,  1889,  1890,  -939,  1891,
     724,  -925,   463,   731,   463,   386,  1461,  1958,  -970,  -648,
     272,  -958,   795,   415,   790,   791,   458,  -925,   743,   745,
     402,   784,   378,  -461,  2031,   213,    40,  1216,   667,   746,
     747,   748,   749,   751,   752,   753,   754,   755,   756,   757,
     758,   759,   760,   761,   762,   763,   764,   765,   766,   767,
     768,   769,   770,   771,   772,   773,   774,  1752,   776,   387,
     743,   743,   779,  1343,  1344,  1438,   729,   352,   532,   533,
     697,  1735,   798,   799,   800,   801,   802,   803,   804,   805,
     806,   807,   808,   809,   810,  2092,   213,    40,   126,   532,
     533,  -650,   724,   724,   743,   822,  2109,   405,   406,   798,
     256,   117,   826,   538,   625,   422,   734,   796,   502,   249,
     229,   834,   423,   836,  1004,   254,  1005,  1540,  -938,  1253,
     503,   724,   388,   255,  1254,   224,   224,  1520,   515,   855,
    -939,   856,  -928,   226,   464,   797,  1613,  1284,   538,   942,
     132,   397,   226,   389,  1744,   714,  1746,   112,  -928,   226,
     276,  1340,  -461,   351,   390,  -461,   394,   532,   533,  1346,
     402,   226,   985,   841,  1736,  2011,   395,   442,  1343,  1344,
     391,   793,   684,   859,  1056,  2061,   532,   533,  2093,   398,
    1942,   686,   932,  -765,  1459,   514,  1292,   376,   918,  2110,
     402,   683,   854,   413,  1265,   391,  1265,   685,   667,  1943,
     391,   391,  1944,  -126,  1679,   433,   687,  -126,   112,   402,
     532,   533,  2062,  2063,  2064,   411,  -926,  1253,   412,   212,
    -966,  1614,  1254,  1372,  -126,  1374,  1267,  1378,   391,  1269,
    1270,  1048,  -926,  1474,   428,   399,  1476,   405,   406,  -966,
     400,  1194,    50,  1195,  1036,  1196,  1004,  1005,   925,  1198,
     416,   442,   417,  1101,  1103,   418,  1994,   402,   532,   533,
      55,  1102,  1904,  1060,  1533,   403,  1908,   405,   406,   459,
     186,   187,    65,    66,    67,  1995,  1014,   973,  1996,   216,
     217,   218,   219,   220,   171,   662,   405,   406,   229,   383,
     441,   786,  1296,  1297,  1298,   212,   226,   442,   384,   228,
     230,  1759,   945,   946,   549,  1039,   453,    93,    94,   445,
      95,   190,    97,  1503,   713,  1505,   448,   818,    50,  1510,
     454,   402,  1633,   459,   186,   187,    65,    66,    67,   438,
    1440,  1934,  -641,   404,   405,   406,   108,   786,  1047,   697,
     564,  2079,   460,  1441,   466,   224,   402,   467,   843,  1935,
     686,   849,   468,  1189,   667,   216,   217,   218,   219,   220,
     683,   117,   402,   229,  1442,   117,   685,  1059,  1936,   583,
     667,  1480,  1489,  1011,  1012,   687,  1107,  1108,  2062,  2063,
    2064,   469,  1490,    93,    94,   470,    95,   190,    97,   430,
     126,  1610,   471,   656, -1072,   658,   460,  1469,   405,   406,
    1105,   472,   229,  2057,   229,   612,   614,   616,   473,   619,
     580,  1522,   108,   688,   507,   443,   267,  1265,   516,  1325,
    1326,  1111,   668,   405,   406,  -642,  1967,  1184,  1185,  1186,
    1970, -1072,   229,  -643, -1072,  -646,  1628,  1725,  -644,   405,
     406,  -645,   132,  1187,   537,  1568,   505,   459,    63,    64,
      65,    66,    67,   663,   506,   603,   508,   669,    72,   509,
    1122,  -647,   459,   186,   187,    65,    66,    67,  -960,  1030,
     541,   430,  1424,  1425,   226,   499,   500,   501,  -763,   502,
     409,  1342,  1343,  1344,   224,   663,   546,   669,   663,   669,
     669,   503,   548,   224,   376,   376,   376,   618,   376,   503,
     224,   511,   430,   784,   443,   229,  1600,   126,   686,  1964,
    1965,  1220,   224,  1221,   554,   856,  1686,  1687,   683,   555,
     460,   229,   229,  -964,   685,   563,  1223,  1992,  1993,   117,
    1527,  1343,  1344,   687,  1784,   460,   670,  2087,  2088,   715,
     558,  1233,   538,   351,   171,  -761,  1659,   565,   171,   226,
    1988,  1989,   391,   437,   439,   440,  1252,  1255,  1291,   132,
     557,   566,  1668,   358,   574,   126,   595,  1630,   123, -1117,
     599,  1261,   598,   127,   731,   621,   731,  2071,   129,   587,
     605,   798,   606,   130,   622,   634,   631,   624,   226,   635,
     226,   689,   644,   645,  2085,  1279,  1753,   690,  1280,   699,
    1281,   700,   701,   703,   724,   603,   391,   788,   391,   391,
     391,   391,   712,  -131,   830,   734,    55,   132,   226,   739,
     167,   496,   497,   498,   499,   500,   501,   730,   502,  1242,
     126,   813,   827,   829,  1242,   662,   831,  1264,   832,  1264,
     503,   838,   839,   857,   797,   697,   576,   224,   861,   864,
     593,   603,   879,   126,  1322,   256,   912,   878,   628,  1242,
     910,   736,   737,   738,   249,   846,   913,   636,   914,   641,
     254,  1328,  1381,  2049,   648,   915,   117,   697,   255,   916,
     917,   921,   132,   229,   229,   126,   666,   920,  1761,   935,
     939,   226,   940,  1454,   944,  2049,   907,  1767,  1467,   943,
     950,  1468,  1329,   955,  2072,   132,   957,   226,   226,  1740,
    1741,   952,   171,  1774,  1432,   958,   960,   959,   961,   962,
    1242,   963,   931,   715,  2007,   964,   965,   971,   733,  2012,
     976,  1181,  1182,  1183,  1184,  1185,  1186,   132,  1455,   977,
    1456,   979,   684,  -786,   987,   988,   986,   989,   929,   929,
    1187,   686,   990,   991,   994,   999,   998,   968,   970,  1007,
    1093,   683,  1009,  1015,  1016,  1018,   126,   685,   126,  1021,
    1022,  1037,  1033,  1235,  1434,  1045,   687,  2037,  1041,  1023,
    1008,  1053,  1024,  1025,  1042,  1445,  1035,   818,   818,  1044,
    -767,  1061,  1062,  1063,  1919,  1104,   591,   731,   592,  1114,
    1124,  1126,  1128,  1132,  1133,  1134,  1151,  1136,  1150,  1152,
    1156,  1457,   743,  1153,  1154,  1472,  1157,  1155,   132,   123,
     132,   648,  2073,  1192,   127,   224,  2074,   229,  1202,   129,
    1204,   117,  1206,  1208,   130,  1209,  1210,  1433,   724,   391,
    2090,  2091,  1215,  1219,  1230,  1046,  1228,  1231,   686,   724,
    1434,  1222,  2098,  1232,  2006,  1236,  2009,   597,   683,   171,
    1264,  1257,  1238,  1241,   685,  1578,  1243,  1286,  1241,   226,
     226,   167,  1274,   687,  1283,  1289,  1294,  -969,  1295,   535,
    1305,   126,   580,  1306,   849,  1307,   849,  1308,   267,  1515,
     229,  1309,  1310,  1241,  1311,  1316,  1314,  1317,  1529,  1315,
     224,  1319,  1337,  1331,  1333,   229,   229,    14,  1336,  1339,
    1348,   684,  1349,  1356,  1350,  1358,  1187, -1133,  1359,  1363,
     697,  1362,  1518,   697,  1082,  1414,  1096,  1416,  1417,  1428,
    1418,  1419,  1421,   132,  1129,   429,  1431,   426,  1449,   224,
    1135,   224,  1465,  2060,   431,  1973,  1450,  1049,   117,  1462,
     718,  1466,  1491,   358,  1241,  1475,  1473,  1477,  1479,  1482,
    1493,  1481,  1120,   117,  1484,  1072,  1485,  1492,  1506,   224,
    1514,  1516,  1579,  1517,  1519,  1524,  1534,  1528,  1580,  1537,
     459,  1581,   187,  1582,  1583,  1584,  1585,  1541,  1542,  1545,
    1601,  1546,  1547,  1551,  1550,  1556,   126,  1606,  1553,  1010,
     229,  1302,  1608,  1554,  1609,  1555,   117,   731,  1557,  1562,
    1559,  1560,  1211,   226,   171,  1197,   715,  1726,  1563,  1602,
    1620,  1569,  1618,   462,  1570,  1567,  1619,  1571,  1586,  1587,
    1625,  1588,  1572,  1629,   724,  1643,  1621,  1622,  1603,   929,
    1612,   929,   224,   929,  1623,  1627,  1212,   929,   132,   929,
     929,  1201,  1631,   460,  1635,  1637,  1636,  1645,   224,   224,
    1638,  1090,  1604,  1641,  1647,  1652,  1642,  1653,  1654,   684,
     729,   117,  1656,  1646,  1058,  1599,   226,  1658,  1649,  1650,
    1651,  1660,  1661,  1662,  1669,  1599,  1665,   603,  1666,  1672,
    1694,   226,   226,  1683,   117,  -448,  1707,  1715,  1887,  -447,
     870,   813,   813,  -446,  1719,  1729,  1724,  1734,  1737,  1747,
    1738,  1742,  1748,  1097,  1754,  1098,  2097,  1750,  1769,  1772,
    1778,  1884,  1883,   697,  1786,  1787,   117,  1892,  1898,  1899,
    1901,   171,  1902,  1903,  1905,  1911,  1913,  1593,  1914,  1946,
    1950,  1924,  1951,  1118,  1955,  1925,   171,  1593,  1954,  1982,
    1960,  1984,  1986,   391,  1990,  1997,  1998,  1999,  1318,  2004,
    2005,  2008,  2013,  1299,  1299,  1082,  2017,  2018,  1733,  -386,
    2021,  2023,  2020,  1739,  1940,  2025,   724,   724,  2029,  2038,
    2032,  2045,  2040,  2051,  2039,  2059,   226,  2046,  2055,   171,
    1777,  2068,  2081,  2058,  2094,  2070,   992,   993,   846,  2095,
     846,  2099,  1357,  2103,  1779,   117,  1360,   117,  2086,   117,
    2104,  2111,  2075,  1364,  2112,  2076,  1207,  2114,  2115,  1420,
     224,   224,  2054,   787,   126,   792,  1285,   789,  1227,  2069,
    1352,  1521,   648,  1218,  1918,  2067,  1471,  1909,  1933,  1302,
    1499,  1667,   933,  1499,  1785,  1938,  1714,  2106,  1717,  2080,
    1512,  1888,  1695,  1907,   171,  1373,   603,   657,  1765,  1443,
    1504,  1365,  1301,  1495,  1320,  1496,   650,  2002,  1142,   126,
    2034,  2027,  1599,  1423,  1953,  1685,   132,   171,  1599,  1354,
    1599,  1413,     0,   697,     0,   907,     0,     0,     0,   725,
       0,     0,  1900,   459,    63,    64,    65,    66,    67,  1266,
       0,  1266,  1090,  1599,    72,   509,     0,   516,     0,   171,
       0,     0,   684,     0,     0,   126,     0,     0,     0,     0,
       0,   132,   117,     0,   126,     0,     0,     0,     0,   929,
       0,     0,     0,     0,  1593,     0,     0,     0,     0,     0,
    1593,     0,  1593,     0,     0,   510,     0,   511,     0,     0,
       0,     0,     0,     0,     0,     0,  1916,  1765,     0,     0,
       0,   512,     0,   513,   224,  1593,   460,   132,  1494,     0,
       0,     0,     0,     0,     0,     0,   132,     0,     0,     0,
       0,     0,  1893,     0,     0,  1140,   718,     0,   171,     0,
     171,     0,   171,     0,  1118,  1335,     0,     0,  1599,     0,
     352,     0,     0,  1082,  1082,  1082,  1082,  1082,  1082,   684,
       0,     0,  1082,     0,  1082,     0,     0,   126,  1947,     0,
       0,     0,     0,   126,  1639,   117,  1640,   224,     0,     0,
     126,  1548,     0,     0,     0,  1552,     0,   117,     0,  2043,
    1558,     0,   224,   224,     0,     0,     0,  1536,  1564,     0,
       0,     0,     0,  1956,     0,     0,     0,   871,     0,     0,
    1593,     0,     0,     0,     0,   212,   352,     0,     0,   132,
       0,     0,     0,     0,   462,   132,     0,     0,     0,     0,
       0,     0,   132,  1226,  1947,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   223,   223,
       0,  1237,     0,     0,     0,   171,     0,   212,     0,   246,
       0,     0,     0,     0,  1256,     0,     0,     0,   872,     0,
    1573,     0,  1266,     0,     0,   216,   217,   218,   219,   220,
      50,     0,     0,     0,     0,   246,     0,   224,  1470,     0,
    1090,  1090,  1090,  1090,  1090,  1090,     0,   189,     0,  1090,
      91,  1090,     0,    93,    94,     0,    95,   190,    97,     0,
    1288,     0,  1644,     0,     0,     0,  1648,   216,   217,   218,
     219,   220,     0,  1655,     0,   126,  1755,     0,  1756,     0,
    1757,     0,   108,     0,     0,  1758,     0,  1976,  1082,   189,
    1082,   212,    91,     0,     0,    93,    94,     0,    95,   190,
      97,  1513,   873,     0,     0,     0,     0,     0,   171,     0,
       0,     0,     0,     0,    50,     0,   648,  1118,     0,     0,
     171,     0,     0,     0,   108,     0,     0,   132,     0,   126,
       0,     0,     0,     0,     0,  1347,     0,     0,     0,  1351,
    1710,     0,     0,     0,  1355,     0,     0,     0,     0,  2105,
       0,   216,   217,   218,   219,   220,     0,     0,  2113,     0,
       0,     0,     0,   126,     0,   117,  2116,     0,   126,  2117,
       0,     0,     0,     0,   697,     0,   351,     0,     0,    93,
      94,   132,    95,   190,    97,  1712,     0,  1912,     0,     0,
       0,     0,     0,   126,     0,     0,   697,     0,     0,     0,
       0,     0,     0,     0,     0,   697,     0,     0,   108,  1711,
     117,   648,     0,     0,     0,   132,     0,     0,   223,     0,
     132,     0,     0,     0,  1578,  1090,     0,  1090,     0,  2044,
       0,     0,  1611,     0,     0,     0,     0,     0,     0,     0,
    1082,     0,  1082,     0,  1082,   132,     0,     0,     0,  1082,
    1458,     0,     0,   126,   126,     0,   117,     0,     0,     0,
       0,   117,     0,     0,     0,   117,    14,     0,   246,     0,
     246,     0, -1135, -1135, -1135, -1135, -1135,   494,   495,   496,
     497,   498,   499,   500,   501,   391,   502,     0,   603,     0,
    1578,   351,     0,  1483,     0,     0,  1486,     0,   503,     0,
       0,  1872,     0,     0,     0,   132,   132,     0,  1879,     0,
       0,     0,     0,     0,     0,     0,   351,   351,     0,   351,
       0,     0,     0,     0,     0,   351,     0,     0,     0,   246,
       0,  1579,    14,     0,     0,     0,     0,  1580,     0,   459,
    1581,   187,  1582,  1583,  1584,  1585,  1971,  1972,   171,     0,
       0,  1082,     0,     0,     0,     0,  1535,   223,   117,   117,
     117,     0,     0,  1539,   117,     0,   223,     0,     0,     0,
       0,   117,     0,   223,     0,     0,     0,  1090,     0,  1090,
       0,  1090,     0,     0,     0,   223,  1090,  1586,  1587,     0,
    1588,     0,     0,   171,     0,     0,   223,  1579,     0,     0,
       0,     0,     0,  1580,     0,   459,  1581,   187,  1582,  1583,
    1584,  1585,   460,     0,     0,     0,     0,     0,     0,     0,
       0,  1745,   246,     0,     0,   246,     0,     0,     0,     0,
       0,  1574,  1575,     0,     0,     0,     0,     0,     0,   171,
       0,     0,     0,     0,   171,     0,     0,     0,   171,     0,
       0,     0,     0,  1586,  1587,     0,  1588,     0,   544,   518,
     519,   520,   521,   522,   523,   524,   525,   526,   527,   528,
     529,     0,     0,     0,     0,  1578,     0,     0,   460,     0,
       0,   246,     0,     0,   603,     0,     0,  1749,  1090,     0,
       0,     0,     0,  1983,  1985,     0,     0,     0,     0,     0,
       0,     0,     0,   530,   531,     0,   351,     0,     0,     0,
    1082,  1082,     0,     0,     0,     0,   117,    14,     0,     0,
     223,     0,     0,     0,     0,  1977,     0,     0,     0,     0,
       0,     0,  1872,  1872,     0,     0,  1879,  1879,     0,     0,
       0,   171,   171,   171,  1670,  1671,     0,   171,  1673,     0,
     603,     0,     0,     0,   171,     0,     0,   544,   518,   519,
     520,   521,   522,   523,   524,   525,   526,   527,   528,   529,
     117,     0,   246,     0,   246,     0,     0,   894,     0,     0,
     532,   533,  1579,     0,     0,     0,  1691,     0,  1580,     0,
     459,  1581,   187,  1582,  1583,  1584,  1585,     0,   212,     0,
    1718,  1578,   530,   531,   117,     0,     0,     0,     0,   117,
     894,     0,     0,     0,     0,     0,     0,  2042,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,   922,
     923,  1578,     0,     0,   117,     0,     0,     0,  1586,  1587,
    2056,  1588,     0,    14,   702,     0,     0,  1090,  1090,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   216,   217,
     218,   219,   220,   460,     0,     0,     0,     0,   246,   246,
       0,     0,  1751,    14,     0,     0,  1768,   246,     0,   532,
     533,     0,     0,   924,     0,     0,    93,    94,     0,    95,
     190,    97,     0,     0,   117,   117,     0,     0,   223,   171,
       0,  1691,     0,     0,     0,     0,     0,     0,  1579,     0,
       0,     0,     0,     0,  1580,   108,   459,  1581,   187,  1582,
    1583,  1584,  1585,     0,     0,     0,  1691,  1691,     0,  1691,
       0,     0,     0,     0,     0,  1691,     0,     0,  1579,     0,
       0,     0,     0,   840,  1580,     0,   459,  1581,   187,  1582,
    1583,  1584,  1585,   171,    34,    35,    36,     0,     0,     0,
       0,     0,     0,     0,  1586,  1587,     0,  1588,   214,     0,
       0,     0,     0,   223, -1135, -1135, -1135, -1135, -1135,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,   171,     0,   460,
       0,     0,   171,  1929,  1586,  1587,     0,  1588,  1760,     0,
    1187,     0,     0,     0,     0,     0,   246,     0,     0,     0,
       0,     0,   223,     0,   223,     0,     0,   171,     0,   460,
       0,     0,     0,    81,    82,    83,    84,    85,  1910,     0,
       0,     0,     0,   212,   221,   213,    40,     0,     0,     0,
      89,    90,   223,   894,     0,     0,     0,     0,     0,     0,
     246,     0,     0,     0,    99,     0,    50,   246,   246,   894,
     894,   894,   894,   894,     0,     0,     0,     0,     0,   105,
       0,     0,   894,     0,     0,     0,     0,   171,   171,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   246,
       0,     0,     0,   216,   217,   218,   219,   220,     0,  1952,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     289,     0,  1963,     0,     0,   223,  1691,     0,     0,   811,
       0,    93,    94,     0,    95,   190,    97,     0,     0,   246,
       0,   223,   223,   544,   518,   519,   520,   521,   522,   523,
     524,   525,   526,   527,   528,   529,     0,   291,     0,     0,
     108,     0,   225,   225,   812,   246,   246,   112,     0,     0,
     212,     0,     0,   248,     0,     0,   223,     0,     0,     0,
       0,     0,     0,   246,     0,     0,     0,     0,   530,   531,
     246,     0,     0,    50,     0,     0,   246,     0,     0,  2022,
       0,   596,     0,     0,     0,     0,     0,   894,  1026,   518,
     519,   520,   521,   522,   523,   524,   525,   526,   527,   528,
     529,     0,   246,     0,     0,  1963,     0,  2035,     0,   589,
     216,   217,   218,   219,   220,   590,     0,     0,     0,     0,
       0,     0,   246,     0,     0,     0,   246,     0,     0,     0,
       0,     0,   189,   530,   531,    91,   344,   246,    93,    94,
       0,    95,   190,    97,     0,   532,   533,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   348,     0,     0,     0,
       0,     0,   354,     0,     0,     0,     0,   108,   350,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   223,   223,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   246,     0,     0,
       0,   246,     0,   246,     0,     0,   246,     0,     0,   941,
     532,   533,     0,     0,     0,     0,     0,     0,     0,   894,
     894,   894,   894,   894,   894,   223,   894,     0,     0,   894,
     894,   894,   894,   894,   894,   894,   894,   894,   894,   894,
     894,   894,   894,   894,   894,   894,   894,   894,   894,   894,
     894,   894,   894,   894,   894,   894,   894,   474,   475,   476,
       0,     0,   225,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1027,   894,     0,   477,   478,     0,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,   474,   475,   476,     0,     0,
       0,     0,   246,     0,   246,     0,   503,     0,     0,     0,
       0,     0,     0,     0,     0,   477,   478,   223,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,     0,     0,   246,     0,     0,   246,     0,
       0,     0,     0,     0,   503,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   246,   246,   246,   246,   246,
     246,     0,     0,   223,   246,     0,   246,     0,     0,     0,
     223,     0,     0,     0,     0,   354,     0,   354,     0,     0,
       0,   225,     0,     0,     0,   223,   223,     0,   894,     0,
     225,     0,     0,     0,     0,     0,     0,   225,   246,     0,
       0,     0,     0,     0,     0,   246,     0,     0,     0,   225,
     894,     0,   894,     0,     0,     0,     0,     0,     0,     0,
     225,     0,     0,   474,   475,   476,     0,     0,     0,     0,
       0,  1436,     0,     0,     0,     0,   354,   894,     0,     0,
       0,     0,     0,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,     0,     0,   246,   246,     0,   504,   246,  1038,     0,
     223,     0,   503,     0,     0,  1026,   518,   519,   520,   521,
     522,   523,   524,   525,   526,   527,   528,   529,     0,     0,
       0,     0,     0,     0,     0,   248,     0,     0,     0,   246,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   474,   475,   476,   354,
     530,   531,   354,     0,     0,     0,     0,     0,     0,     0,
     246,     0,   246,     0,   225,     0,   477,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   503,   246,   246,     0,     0,
     246,     0,     0,     0,     0,     0,   894,     0,   894,     0,
     894,   902,     0,     0,     0,   894,   223,   532,   533,     0,
     894,     0,   894,     0,     0,   894,   972,     0,     0,     0,
       0,     0,     0,   474,   475,   476,     0,     0,   246,   246,
       0,     0,     0,     0,   902,   246,     0,     0,     0,     0,
       0,     0,   246,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,   702,     0,     0,     0,     0,  1696,     0,     0,   354,
       0,   874,   503,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   246,     0,   246,     0,   246,     0,     0,     0,
       0,   246,     0,   223,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   246,  1006,
       0,     0,   225,   894,     0,     0,   212,     0,     0,     0,
       0,     0,     0,     0,     0,   246,   246,     0,     0,     0,
       0,     0,     0,   246,     0,   246,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   289,
       0,     0,     0,     0,     0,     0,     0,     0,   246,   246,
       0,   246,  1697,     0,     0,   354,   354,   246,     0,     0,
       0,     0,     0,     0,   354,  1698,   216,   217,   218,   219,
     220,  1699,     0,     0,     0,     0,   291,   225,     0,     0,
       0,     0,     0,   246,     0,     0,     0,     0,   189,   212,
       0,    91,  1700,     0,    93,    94,  1038,    95,  1701,    97,
     894,   894,   894,     0,     0,     0,     0,   894,     0,   246,
    1089,     0,    50,     0,     0,   246,   225,   246,   225,     0,
    -433,     0,     0,   108,     0,     0,     0,     0,     0,   459,
     186,   187,    65,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   225,   902,   589,   216,
     217,   218,   219,   220,   590,   871,     0,     0,     0,     0,
       0,     0,     0,   902,   902,   902,   902,   902,     0,     0,
       0,   189,     0,     0,    91,   344,   902,    93,    94,     0,
      95,   190,    97,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1191,     0,   348,     0,     0,     0,     0,
       0,     0,   460,     0,     0,   212,   108,   350,     0,     0,
       0,     0,     0,   894,     0,     0,   872,     0,     0,   225,
       0,   246,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,  1214,   246,   225,   225,  1131,   246,     0,
       0,     0,   246,   246,   354,   354,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   246,     0,     0,
    1214,     0,     0,   894,     0,   216,   217,   218,   219,   220,
     225,     0,     0,     0,     0,     0,     0,     0,     0,   894,
     894,     0,     0,   212,   894,   213,    40,   189,     0,     0,
      91,     0,     0,    93,    94,     0,    95,   190,    97,     0,
    1353,   902,     0,     0,     0,     0,    50,     0,     0,     0,
       0,   246,     0,     0,     0,     0,  1275,     0,     0,     0,
     227,   227,   108,     0,     0,   212,   894,   966,     0,   967,
       0,   252,     0,     0,     0,     0,     0,   246,     0,   246,
     248,     0,   354,   216,   217,   218,   219,   220,    50,     0,
       0,  1089,     0,     0,     0,     0,     0,     0,     0,     0,
     354,     0,     0,     0,     0,     0,     0,   354,   246,   811,
       0,    93,    94,   354,    95,   190,    97,     0,     0,     0,
       0,     0,     0,     0,   246,   216,   217,   218,   219,   220,
     246,     0,     0,     0,   246,     0,     0,   225,   225,     0,
     108,     0,     0,     0,   845,     0,     0,   112,   246,   246,
       0,     0,     0,    93,    94,     0,    95,   190,    97,   354,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   902,   902,   902,   902,   902,   902,   225,
     902,     0,   108,   902,   902,   902,   902,   902,   902,   902,
     902,   902,   902,   902,   902,   902,   902,   902,   902,   902,
     902,   902,   902,   902,   902,   902,   902,   902,   902,   902,
     902,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   474,   475,   476,     0,     0,   902,
       0,     0,     0,     0,   354,     0,     0,     0,   354,     0,
     874,     0,     0,   354,   477,   478,     0,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,     0,     0,     0,     0,   474,   475,   476,     0,
     227,   225,     0,   503,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   477,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,     0,     0,     0,     0,     0,  1089,
    1089,  1089,  1089,  1089,  1089,   503,     0,   225,  1089,     0,
    1089,     0,     0,     0,   225,     0,     0,     0,     0,   354,
       0,   354,     0,     0,     0,     0,     0,     0,     0,   225,
     225,     0,   902,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,     0,   902,     0,   902,     0,     0,     0,
       0,     0,   354,     0,     0,   354,  1187,   517,   518,   519,
     520,   521,   522,   523,   524,   525,   526,   527,   528,   529,
       0,   902,     0,     0,     0,     0,     0,  1064,     0,   227,
       0,     0,     0,     0,     0,     0,     0,     0,   227,     0,
       0,     0,     0,     0,     0,   227,     0,     0,     0,     0,
       0,     0,   530,   531,     0,     0,     0,   227,   212,     0,
       0,  1577,     0,     0,   225,   354,     0,     0,   252,   474,
     475,   476,   354,     0,     0,     0,     0,     0,     0,  1205,
       0,    50,     0,     0,     0,     0,     0,     0,     0,   477,
     478,     0,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,     0,   216,   217,
     218,   219,   220,     0,  1089,     0,  1089,     0,   503,   532,
     533,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     354,   354,     0,   409,     0,     0,    93,    94,     0,    95,
     190,    97,     0,   252,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   904,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   108,   354,     0,     0,   410,
     902,     0,   902,     0,   902,     0,     0,     0,     0,   902,
     225,     0,   227,     0,   902,     0,   902,     0,   934,   902,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1693,     0,     0,     0,     0,     0,  1706,
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,     0,   903,
       0,     0,     0,   354,   354,     0,     0,   354,     0,   503,
       0,     0,  1282,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1089,     0,  1089,     0,
    1089,     0,   903,     0,     0,  1089,     0,   225,     0,     0,
       0,     0,     0,     0,     0,   354,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   902,     0,   354,
     474,   475,   476,     0,     0,     0,     0,     0,     0,  1775,
    1776,     0,     0,     0,     0,     0,     0,     0,     0,  1706,
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   503,
     227,     0,  1066,  1067,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   354,     0,  1089,     0,     0,
       0,     0,  1068,  1293,     0,     0,     0,     0,     0,     0,
    1069,  1070,  1071,   212,   902,   902,   902,     0,     0,     0,
     354,   902,     0,  1927,  1072,     0,     0,     0,     0,     0,
       0,  1706,     0,     0,     0,     0,    50,     0,     0,     0,
       0,  1119,     0,     0,     0,   354,   354,     0,   354,     0,
       0,     0,     0,     0,   354,   227,     0,  1143,  1144,  1145,
    1146,  1147,     0,     0,     0,     0,     0,     0,     0,     0,
    1158,     0,  1073,  1074,  1075,  1076,  1077,  1078,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1079,     0,     0,     0,   227,   189,   227,     0,    91,    92,
       0,    93,    94,     0,    95,   190,    97,     0,     0,     0,
       0,   212,   354,  1324,     0,     0,     0,     0,     0,  1080,
    1081,     0,     0,     0,   227,   903,     0,   902,     0,     0,
     108,     0,   289,     0,    50,     0,     0,     0,     0,     0,
       0,   903,   903,   903,   903,   903,     0,     0,     0,     0,
       0,     0,     0,     0,   903,     0,  1089,  1089,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   291,
       0,   216,   217,   218,   219,   220,     0,   902,     0,     0,
       0,     0,   212,     0,     0,     0,     0,     0,  1137,     0,
       0,     0,     0,   902,   902,  1262,     0,   227,   902,    93,
      94,     0,    95,   190,    97,    50,     0,     0,     0,     0,
       0,     0,     0,   227,   227,     0,     0,     0,   354,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   108,   739,
       0,   354,     0,     0,     0,   354,     0,     0,     0,     0,
     902,   589,   216,   217,   218,   219,   220,   590,   252,     0,
       0,     0,     0,     0,  1978,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   189,     0,     0,    91,   344,     0,
      93,    94,     0,    95,   190,    97,     0, -1134,     0,   903,
       0,     0,  2053,     0,     0,     0,     0,     0,   348,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1693,   108,
     350,     0,     0,     0,     0,     0,     0,     0,   354,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   252,     0,
       0,     0,     0,     0,     0,     0,     0,  1147,  1368,     0,
       0,  1368,     0,     0,   354,     0,   354,  1382,  1385,  1386,
    1387,  1389,  1390,  1391,  1392,  1393,  1394,  1395,  1396,  1397,
    1398,  1399,  1400,  1401,  1402,  1403,  1404,  1405,  1406,  1407,
    1408,  1409,  1410,  1411,  1412,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   227,   227,     0,     0,     0,
       0,     0,     0,  1422,     0,     0,     0,   354,     0,     0,
       0,   354,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   354,   354,     0,     0,     0,
       0,   903,   903,   903,   903,   903,   903,   252,   903,     0,
       0,   903,   903,   903,   903,   903,   903,   903,   903,   903,
     903,   903,   903,   903,   903,   903,   903,   903,   903,   903,
     903,   903,   903,   903,   903,   903,   903,   903,   903,   474,
     475,   476,     0,     0,   544,   518,   519,   520,   521,   522,
     523,   524,   525,   526,   527,   528,   529,   903,     0,   477,
     478,     0,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,   212,   502,     0,     0,   530,
     531,     0,     0,     0,     0,     0,     0,     0,   503,     0,
       0,     0,     0,     0,     0,     0,  1525,     0,    50,   227,
       0,     0,     0,  1026,   518,   519,   520,   521,   522,   523,
     524,   525,   526,   527,   528,   529,     0,     0,  1543,     0,
    1544,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   216,   217,   218,   219,   220,
       0,     0,     0,     0,     0,  1565,     0,   289,   530,   531,
       0,     0,     0,     0,     0,   252,   532,   533,     0,     0,
       0,  1877,   227,    93,    94,  1878,    95,   190,    97,     0,
       0,     0,     0,     0,     0,     0,     0,   227,   227,     0,
     903,     0,     0,     0,   291,     0,     0,     0,     0,     0,
       0,     0,   108,  1711,     0,     0,     0,   212,     0,     0,
       0,     0,   903,  1538,   903,     0,     0,   474,   475,   476,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,  1607,     0,     0,   532,   533,   477,   478,   903,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,     0,   589,   216,   217,   218,
     219,   220,   590,     0,     0,     0,   503,     0,   212,     0,
       0,     0,   227,     0,     0,     0,     0,     0,     0,   189,
       0,     0,    91,   344,     0,    93,    94,     0,    95,   190,
      97,    50, -1134,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   348,  1675,     0,  1676,     0,  1677,     0,
       0,     0,     0,  1678,   108,   350,     0,     0,  1680,     0,
    1681,     0,     0,  1682,     0,     0,     0,     0,   216,   217,
     218,   219,   220,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   279,   280,     0,   281,   282,     0,     0,   283,
     284,   285,   286,     0,     0,     0,    93,    94,     0,    95,
     190,    97,     0,     0,     0,     0,     0,   287,   288,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   108,  1035,     0,   903,     0,
     903,     0,   903,     0,     0,     0,   290,   903,   252,     0,
    1731,     0,   903,     0,   903,     0,     0,   903,     0,     0,
     292,   293,   294,   295,   296,   297,   298,     0,     0,     0,
     212,     0,   213,    40,     0,     0,   299,     0,     0,     0,
       0,  1770,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,    50,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,     0,   335,
       0,   782,   337,   338,   339,     0,     0,     0,   340,   600,
     216,   217,   218,   219,   220,   601,     0,   212,     0,     0,
       0,     0,     0,     0,     0,   252,     0,     0,     0,     0,
       0,     0,   602,     0,     0,     0,     0,     0,    93,    94,
      50,    95,   190,    97,   345,   903,   346,     0,     0,   347,
       0,     0,     0,     0,     0,     0,     0,   349,  1920,  1921,
    1922,     0,   212,     0,     0,  1926,     0,   108,     0,     0,
       0,   783,     0,     0,   112,     0,     0,   216,   217,   218,
     219,   220,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,   279,   280,     0,   281,   282,     0,     0,   283,
     284,   285,   286,   456,     0,    93,    94,     0,    95,   190,
      97,     0,     0,     0,     0,     0,     0,   287,   288,     0,
       0,     0,   216,   217,   218,   219,   220,     0,     0,     0,
       0,     0,     0,     0,   108,     0,     0,     0,     0,     0,
       0,     0,   903,   903,   903,     0,   290,   370,     0,   903,
      93,    94,     0,    95,   190,    97,     0,     0,  1932,     0,
     292,   293,   294,   295,   296,   297,   298,     0,     0,     0,
     212,  1945,     0,     0,     0,     0,   299,     0,     0,   108,
       0,     0,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,    50,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   212,   335,
       0,  1987,   337,   338,   339,     0,     0,     0,   340,   600,
     216,   217,   218,   219,   220,   601,     0,  2000,  2001,     0,
       0,    50,  2003,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   602,     0,     0,   903,     0,     0,    93,    94,
       0,    95,   190,    97,   345,     0,   346,     0,     0,   347,
       0,     0,     0,     0,     0,     0,     0,   349,   216,   217,
     218,   219,   220,     0,  2026,     0,     0,   108,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     189,     0,     0,    91,    92,   903,    93,    94,     0,    95,
     190,    97,     0,     0,     5,     6,     7,     8,     9,     0,
       0,   903,   903,     0,    10,     0,   903,     0,     0,     0,
       0,     0,     0,     0,     0,   108,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  2024,     0,     0,     0,    14,
       0,    15,    16,     0,     0,     0,     0,    17,   903,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,     0,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,    56,    57,    58,     0,    59,  -205,
      60,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,    88,    89,    90,    91,    92,     0,
      93,    94,     0,    95,    96,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
     103,     0,   104,     0,   105,   106,   107,     0,     0,   108,
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
      56,    57,    58,     0,    59,     0,    60,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
      88,    89,    90,    91,    92,     0,    93,    94,     0,    95,
      96,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,   103,     0,   104,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
    1224,     0,   112,   113,   114,   115,     5,     6,     7,     8,
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
       0,     0,    52,    53,    54,    55,    56,    57,    58,     0,
      59,     0,    60,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,    88,    89,    90,    91,
      92,     0,    93,    94,     0,    95,    96,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,   103,     0,   104,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,  1437,     0,   112,   113,
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
       0,     0,   189,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   190,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,   704,     0,   112,   113,   114,   115,     5,     6,
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
      86,     0,     0,    87,     0,     0,     0,     0,   189,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   190,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,  1028,     0,
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
      52,    53,    54,    55,     0,    57,    58,     0,    59,  -205,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   189,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   190,    97,    98,     0,     0,    99,
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
     189,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     190,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
    1190,     0,   112,   113,   114,   115,     5,     6,     7,     8,
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
       0,    87,     0,     0,     0,     0,   189,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   190,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,  1239,     0,   112,   113,
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
       0,     0,   189,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   190,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,  1271,     0,   112,   113,   114,   115,     5,     6,
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
      86,     0,     0,    87,     0,     0,     0,     0,   189,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   190,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,  1330,     0,
     112,   113,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,     0,    42,
       0,     0,     0,    43,    44,    45,    46,  1332,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   189,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   190,    97,    98,     0,     0,    99,
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
      44,    45,    46,     0,    47,     0,    48,     0,    49,  1526,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     189,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     190,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
       0,     0,   112,   113,   114,   115,     5,     6,     7,     8,
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
       0,    87,     0,     0,     0,     0,   189,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   190,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,  1684,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,  -297,    34,    35,    36,
      37,    38,    39,    40,     0,    41,     0,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   189,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   190,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,     0,     0,   112,   113,   114,   115,     5,     6,
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
      86,     0,     0,    87,     0,     0,     0,     0,   189,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   190,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,  1923,     0,
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
      48,  1974,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   189,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   190,    97,    98,     0,     0,    99,
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
      44,    45,    46,     0,    47,  2016,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     189,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     190,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
       0,     0,   112,   113,   114,   115,     5,     6,     7,     8,
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
       0,    87,     0,     0,     0,     0,   189,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   190,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,  2033,     0,   112,   113,
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
       0,     0,   189,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   190,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,  2036,     0,   112,   113,   114,   115,     5,     6,
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
      86,     0,     0,    87,     0,     0,     0,     0,   189,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   190,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,  2052,     0,
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
       0,     0,     0,     0,   189,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   190,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,  2107,     0,   112,   113,   114,   115,
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
     189,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     190,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
    2108,     0,   112,   113,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,   572,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,   186,   187,    65,    66,    67,
       0,    68,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   189,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   190,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,     0,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,   858,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,     0,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,   186,   187,    65,    66,    67,     0,    68,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   189,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   190,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,     0,     0,   112,   113,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,  1121,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,   186,   187,    65,
      66,    67,     0,    68,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   189,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   190,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,     0,     0,
     112,   113,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,  1764,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,     0,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
       0,    61,    62,   186,   187,    65,    66,    67,     0,    68,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   189,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   190,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,     0,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,  1915,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,     0,    61,    62,   186,
     187,    65,    66,    67,     0,    68,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     189,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     190,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
       0,     0,   112,   113,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,   186,   187,    65,    66,    67,
       0,    68,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   189,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   190,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,     0,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   359,   425,    13,     0,
       0,     0,     0,     0,     0,     0,     0,   794,     0,     0,
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
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     5,
       6,     7,     8,     9,   112,   113,   114,   115,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   359,     0,    13,     0,     0,     0,     0,
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
      97,     0,     0,     0,    99,     0,     0,   100,     5,     6,
       7,     8,     9,   101,   102,     0,     0,     0,    10,   105,
     106,   107,     0,     0,   108,   191,     0,   360,     0,     0,
       0,   112,   113,   114,   115,     0,     0,     0,     0,     0,
       0,     0,     0,   719,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,   720,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,   185,   186,   187,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   188,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   189,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   190,    97,
       0,   721,     0,    99,     0,     0,   100,     5,     6,     7,
       8,     9,   101,   102,     0,     0,     0,    10,   105,   106,
     107,     0,     0,   108,   191,     0,     0,     0,     0,     0,
     112,   113,   114,   115,     0,     0,     0,     0,     0,     0,
       0,     0,  1258,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,  1259,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   185,   186,   187,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   188,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   189,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   190,    97,     0,
    1260,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,     0,     0,     0,   105,   106,   107,
       0,     0,   108,   191,     5,     6,     7,     8,     9,   112,
     113,   114,   115,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   359,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     191,     0,     0,   853,     0,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   359,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   794,     0,     0,     0,     0,
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
       0,   359,   425,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,   108,   109,     0,     0,     0,     0,     0,   112,
     113,   114,   115,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,   203,     0,     0,    55,     0,     0,     0,     0,
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
     239,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,   100,     5,     6,     7,     8,     9,   101,   102,
       0,     0,     0,    10,   105,   106,   107,     0,     0,   108,
     191,     0,   274,     0,     0,     0,   112,   113,   114,   115,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,   105,   106,   107,     0,     0,   108,   191,
       0,   277,     0,     0,     0,   112,   113,   114,   115,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   425,     0,     0,     0,     0,     0,
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
      97,     0,     0,     0,    99,     0,     0,   100,     5,     6,
       7,     8,     9,   101,   102,     0,     0,     0,    10,   105,
     106,   107,     0,     0,   108,   109,     0,     0,     0,     0,
       0,   112,   113,   114,   115,     0,     0,     0,     0,     0,
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
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   191,   570,     0,     0,     0,     0,
     112,   113,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   359,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,   750,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,   794,     0,     0,
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
       0,     0,     0,     0,     0,     0,   833,     0,     0,     0,
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
       0,     0,     0,     0,     0,   835,     0,     0,     0,     0,
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
     105,   106,   107,     0,     0,   108,   191,     0,     0,     0,
       0,     0,   112,   113,   114,   115,     0,     0,     0,     0,
       0,     0,     0,     0,  1321,     0,     0,     0,     0,     0,
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
     106,   107,     0,     0,   108,   191,     5,     6,     7,     8,
       9,   112,   113,   114,   115,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     359,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,   108,  1452,     0,     0,     0,     0,     0,   112,   113,
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
     108,   191,     0,     0,     0,     0,     0,   112,   113,   114,
     115,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,   664,    39,    40,     0,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,   185,   186,   187,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     188,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   189,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   190,    97,     0,   279,   280,    99,
     281,   282,   100,     0,   283,   284,   285,   286,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     191,     0,   287,   288,     0,     0,   112,   113,   114,   115,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,   290,   502,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   503,   292,   293,   294,   295,   296,
     297,   298,     0,     0,     0,   212,     0,   213,    40,     0,
       0,   299,     0,     0,     0,     0,     0,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    50,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,     0,   335,     0,   336,   337,   338,   339,
       0,     0,     0,   340,   600,   216,   217,   218,   219,   220,
     601,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   279,   280,     0,   281,   282,     0,   602,   283,   284,
     285,   286,     0,    93,    94,     0,    95,   190,    97,   345,
       0,   346,     0,     0,   347,     0,   287,   288,     0,   289,
       0,     0,   349,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   108,     0,     0,     0,   783,     0,     0,   112,
       0,     0,     0,     0,     0,   290,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   291,     0,     0,   292,
     293,   294,   295,   296,   297,   298,     0,     0,     0,   212,
       0,     0,     0,     0,     0,   299,     0,     0,     0,     0,
       0,   300,   301,   302,   303,   304,   305,   306,   307,   308,
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
       0,  1894,     0,     0,     0,   287,   288,     0,   289,     0,
       0,   216,   217,   218,   219,   220,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   189,   290,     0,    91,     0,     0,    93,
      94,     0,    95,   190,    97,   291,     0,     0,   292,   293,
     294,   295,   296,   297,   298,     0,     0,     0,   212,     0,
       0,     0,     0,     0,   299,     0,     0,     0,   108,     0,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,    50,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,     0,   335,     0,     0,
     337,   338,   339,     0,     0,     0,   340,   341,   216,   217,
     218,   219,   220,   342,     0,     0,     0,     0,     0,     0,
     212,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     343,     0,     0,    91,   344,     0,    93,    94,     0,    95,
     190,    97,   345,    50,   346,     0,     0,   347,     0,   279,
     280,     0,   281,   282,   348,   349,   283,   284,   285,   286,
       0,     0,     0,     0,     0,   108,   350,     0,     0,     0,
    1969,     0,     0,     0,   287,   288,     0,   289,     0,     0,
     216,   217,   218,   219,   220,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,   290,   502,   924,     0,     0,    93,    94,
       0,    95,   190,    97,   291,     0,   503,   292,   293,   294,
     295,   296,   297,   298,     0,     0,     0,   212,     0,     0,
       0,     0,     0,   299,     0,     0,     0,   108,     0,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
      50,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,     0,   335,     0,   336,   337,
     338,   339,     0,     0,     0,   340,   341,   216,   217,   218,
     219,   220,   342,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   343,
       0,     0,    91,   344,     0,    93,    94,     0,    95,   190,
      97,   345,     0,   346,     0,     0,   347,     0,   279,   280,
       0,   281,   282,   348,   349,   283,   284,   285,   286,     0,
       0,     0,     0,     0,   108,   350,     0,     0,     0,     0,
       0,     0,     0,   287,   288,     0,   289,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,   290,   502,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   291,     0,   503,   292,   293,   294,   295,
     296,   297,   298,     0,     0,     0,   212,     0,     0,     0,
       0,     0,   299,     0,     0,     0,     0,     0,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,    50,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,     0,   335,     0,     0,   337,   338,
     339,     0,     0,     0,   340,   341,   216,   217,   218,   219,
     220,   342,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   343,     0,
       0,    91,   344,     0,    93,    94,     0,    95,   190,    97,
     345,     0,   346,     0,     0,   347,     0,     0,     0,     0,
       0,     0,   348,   349,  1688,     0,     0,     0,   279,   280,
       0,   281,   282,   108,   350,   283,   284,   285,   286,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   287,   288,     0,   289,  1164,     0,     0,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,   290,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   291,     0,  1187,   292,   293,   294,   295,
     296,   297,   298,     0,     0,     0,   212,     0,     0,     0,
       0,     0,   299,     0,     0,     0,     0,     0,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,    50,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,     0,   335,     0,     0,   337,   338,
     339,     0,     0,     0,   340,   341,   216,   217,   218,   219,
     220,   342,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   343,     0,
       0,    91,   344,     0,    93,    94,     0,    95,   190,    97,
     345,     0,   346,     0,     0,   347,     0,  1790,  1791,  1792,
    1793,  1794,   348,   349,  1795,  1796,  1797,  1798,     0,     0,
       0,     0,     0,   108,   350,     0,     0,     0,     0,     0,
       0,  1799,  1800,  1801,     0,   477,   478,     0,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,  1802,   502,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   503,  1803,  1804,  1805,  1806,  1807,
    1808,  1809,     0,     0,     0,   212,     0,     0,     0,     0,
       0,  1810,     0,     0,     0,     0,     0,  1811,  1812,  1813,
    1814,  1815,  1816,  1817,  1818,  1819,  1820,  1821,    50,  1822,
    1823,  1824,  1825,  1826,  1827,  1828,  1829,  1830,  1831,  1832,
    1833,  1834,  1835,  1836,  1837,  1838,  1839,  1840,  1841,  1842,
    1843,  1844,  1845,  1846,  1847,  1848,  1849,  1850,  1851,  1852,
       0,     0,     0,  1853,  1854,   216,   217,   218,   219,   220,
       0,  1855,  1856,  1857,  1858,  1859,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1860,  1861,  1862,
       0,   212,     0,    93,    94,     0,    95,   190,    97,  1863,
       0,  1864,  1865,     0,  1866,     0,     0,     0,     0,     0,
       0,  1867,     0,  1868,    50,  1869,     0,  1870,  1871,     0,
     279,   280,   108,   281,   282,     0,     0,   283,   284,   285,
     286,     0,     0,     0,     0,     0,     0,  1697,     0,     0,
       0,     0,     0,     0,     0,   287,   288,     0,     0,     0,
    1698,   216,   217,   218,   219,   220,  1699,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   189,   290,     0,    91,    92,     0,    93,
      94,     0,    95,  1701,    97,     0,     0,     0,   292,   293,
     294,   295,   296,   297,   298,     0,     0,     0,   212,     0,
       0,     0,     0,     0,   299,     0,     0,     0,   108,     0,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,    50,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,     0,   335,     0,   336,
     337,   338,   339,     0,     0,     0,   340,   600,   216,   217,
     218,   219,   220,   601,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   279,   280,     0,   281,   282,     0,
     602,   283,   284,   285,   286,     0,    93,    94,     0,    95,
     190,    97,   345,     0,   346,     0,     0,   347,     0,   287,
     288,     0,     0,     0,     0,   349,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   108,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   290,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   292,   293,   294,   295,   296,   297,   298,     0,
       0,     0,   212,     0,     0,     0,     0,     0,   299,     0,
       0,     0,     0,     0,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,    50,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
       0,   335,     0,  1380,   337,   338,   339,     0,     0,     0,
     340,   600,   216,   217,   218,   219,   220,   601,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   602,     0,     0,     0,     0,     0,
      93,    94,     0,    95,   190,    97,   345,     0,   346,     0,
       0,   347,   474,   475,   476,     0,     0,     0,     0,   349,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   108,
       0,     0,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,     0,     0,     0,     0,     0,     0,     0,
     477,   478,  1530,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,   474,   475,
     476,     0,     0,     0,     0,     0,     0,     0,     0,   503,
       0,     0,     0,     0,     0,     0,     0,     0,   477,   478,
       0,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,   474,   475,   476,     0,
       0,     0,     0,     0,     0,     0,     0,   503,     0,     0,
       0,     0,     0,     0,     0,     0,   477,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,     0,  1732,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   503,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   474,
     475,   476,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   477,
     478,  1531,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,   474,   475,   476,
       0,     0,     0,     0,     0,     0,     0,     0,   503,     0,
       0,     0,     0,     0,     0,     0,     0,   477,   478,   504,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,   474,   475,   476,     0,     0,
       0,     0,     0,     0,     0,     0,   503,     0,     0,     0,
       0,     0,     0,     0,     0,   477,   478,   586,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,     0,   474,   475,   476,     0,     0,     0,
       0,     0,     0,     0,   503,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   477,   478,     0,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
     588,   502,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,     0,     0,     0,     0,
       0,     0,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,   607,   502,
    1161,  1162,  1163,     0,     0,     0,     0,     0,     0,     0,
       0,   503,   289,     0,     0,     0,     0,     0,     0,     0,
       0,  1164,     0,     0,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,   611,     0,     0,   291,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1187,
     289,     0,   212,     0,     0,     0,     0,     0,  1001,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,   825,     0,     0,   291,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   289,     0,
     212,     0,     0,     0,     0,     0,  1460,     0,     0,     0,
       0,   589,   216,   217,   218,   219,   220,   590,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,   850,     0,   189,   291,     0,    91,   344,     0,
      93,    94,     0,    95,   190,    97,     0,     0,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   348,   589,
     216,   217,   218,   219,   220,   590,     0,     0,     0,   108,
     350,    50,     0,     0,     0,     0,     0,     0,  1388,     0,
    1379,     0,   189,     0,     0,    91,   344,     0,    93,    94,
       0,    95,   190,    97,     0,     0,   880,   881,     0,     0,
       0,     0,   882,     0,   883,     0,   348,   589,   216,   217,
     218,   219,   220,   590,     0,     0,   884,   108,   350,     0,
       0,     0,     0,     0,    34,    35,    36,   212,     0,     0,
     189,     0,     0,    91,   344,     0,    93,    94,   214,    95,
     190,    97,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,   348,     0,     0,     0,     0,     0,
    1115,     0,     0,     0,     0,   108,   350,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,   885,   886,   887,   888,
     889,   890,    29,    81,    82,    83,    84,    85,     0,  1187,
      34,    35,    36,   212,   221,   213,    40,     0,     0,   189,
      89,    90,    91,    92,   214,    93,    94,     0,    95,   190,
      97,     0,     0,     0,    99,     0,    50,     0,     0,     0,
       0,     0,     0,   891,   892,     0,     0,     0,     0,   105,
       0,     0,     0,   215,   108,   893,     0,     0,   880,   881,
       0,     0,     0,     0,   882,     0,   883,     0,     0,     0,
       0,  1116,    75,   216,   217,   218,   219,   220,   884,    81,
      82,    83,    84,    85,     0,     0,    34,    35,    36,   212,
     221,     0,     0,     0,     0,   189,    89,    90,    91,    92,
     214,    93,    94,     0,    95,   190,    97,     0,     0,     0,
      99,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   105,     0,     0,     0,     0,
     108,   222,     0,     0,     0,     0,     0,   112,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   885,   886,
     887,   888,   889,   890,    29,    81,    82,    83,    84,    85,
       0,     0,    34,    35,    36,   212,   221,   213,    40,     0,
       0,   189,    89,    90,    91,    92,   214,    93,    94,     0,
      95,   190,    97,     0,     0,     0,    99,     0,    50,     0,
       0,     0,     0,     0,     0,   891,   892,     0,     0,     0,
       0,   105,     0,     0,     0,   215,   108,   893,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,   216,   217,   218,   219,   220,
      29,    81,    82,    83,    84,    85,     0,     0,    34,    35,
      36,   212,   221,   213,    40,     0,     0,   189,    89,    90,
      91,    92,   214,    93,    94,     0,    95,   190,    97,     0,
       0,     0,    99,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   105,     0,     0,
       0,   215,   108,   222,     0,     0,   627,     0,     0,   112,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   647,
      75,   216,   217,   218,   219,   220,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   221,     0,
       0,     0,     0,   189,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   190,    97,    29,  1057,     0,    99,     0,
       0,     0,     0,    34,    35,    36,   212,     0,   213,    40,
       0,     0,     0,   105,     0,     0,     0,   214,   108,   222,
       0,     0,     0,     0,     0,   112,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   215,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,
    1182,  1183,  1184,  1185,  1186,    75,   216,   217,   218,   219,
     220,    29,    81,    82,    83,    84,    85,     0,  1187,    34,
      35,    36,   212,   221,   213,    40,     0,     0,   189,    89,
      90,    91,    92,   214,    93,    94,     0,    95,   190,    97,
       0,     0,     0,    99,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   105,     0,
       0,     0,   215,   108,   222,     0,     0,     0,     0,     0,
     112,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1217,    75,   216,   217,   218,   219,   220,    29,    81,    82,
      83,    84,    85,     0,     0,    34,    35,    36,   212,   221,
     213,    40,     0,     0,   189,    89,    90,    91,    92,   214,
      93,    94,     0,    95,   190,    97,     0,     0,     0,    99,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   105,     0,     0,     0,   215,   108,
     222,     0,     0,     0,     0,     0,   112,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,   216,   217,
     218,   219,   220,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   221,     0,     0,     0,     0,
     189,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     190,    97,     0,     0,     0,    99,     0,     0,     0,     0,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
     105,     0,     0,     0,     0,   108,   222,     0,     0,     0,
     477,   478,   112,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,   474,   475,
     476,     0,     0,     0,     0,     0,     0,     0,     0,   503,
       0,     0,     0,     0,     0,     0,     0,     0,   477,   478,
       0,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   503,     0,   474,
     475,   476,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   547,   477,
     478,     0,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,   474,   475,   476,
       0,     0,     0,     0,     0,     0,     0,     0,   503,     0,
       0,     0,     0,     0,     0,     0,   556,   477,   478,     0,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   503,     0,   474,   475,
     476,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   956,   477,   478,
       0,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,   474,   475,   476,     0,
       0,     0,     0,     0,     0,     0,     0,   503,     0,     0,
       0,     0,     0,     0,     0,  1043,   477,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   503,     0,   474,   475,   476,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1099,   477,   478,     0,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,  1161,  1162,  1163,     0,     0,
       0,     0,     0,     0,     0,     0,   503,     0,     0,     0,
       0,     0,     0,     0,  1435,     0,  1164,     0,     0,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
    1186,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1187,  1161,  1162,  1163,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1478,  1164,     0,     0,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
    1186,  1161,  1162,  1163,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1187,     0,     0,     0,     0,     0,
       0,     0,  1164,  1361,     0,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1187,  1161,  1162,  1163,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1164,  1549,     0,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,  1161,  1162,  1163,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1187,     0,     0,     0,     0,     0,     0,     0,  1164,  1561,
       0,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1187,  1161,  1162,  1163,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1164,  1674,
       0,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,    34,    35,    36,   212,     0,   213,    40,
       0,     0,     0,     0,     0,     0,  1187,   214,     0,     0,
       0,     0,     0,     0,     0,  1771,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   243,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   244,     0,     0,
       0,     0,     0,     0,     0,     0,   216,   217,   218,   219,
     220,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,   221,     0,  1773,     0,     0,   189,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   190,    97,
       0,     0,     0,    99,     0,    34,    35,    36,   212,     0,
     213,    40,     0,     0,     0,     0,     0,     0,   105,   678,
       0,     0,     0,   108,   245,     0,     0,     0,     0,     0,
     112,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   215,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,   216,   217,
     218,   219,   220,     0,    81,    82,    83,    84,    85,   503,
       0,    34,    35,    36,   212,   221,   213,    40,     0,     0,
     189,    89,    90,    91,    92,   214,    93,    94,     0,    95,
     190,    97,     0,     0,     0,    99,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     105,     0,     0,     0,   243,   108,   679,     0,     0,     0,
       0,     0,   112,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   216,   217,   218,   219,   220,     0,
      81,    82,    83,    84,    85,   212,     0,     0,     0,     0,
       0,   221,     0,     0,     0,     0,   189,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   190,    97,    50,     0,
       0,    99,     0,     0,     0,     0,   367,   368,     0,     0,
       0,     0,     0,     0,     0,     0,   105,     0,     0,     0,
       0,   108,   245,     0,     0,     0,     0,     0,   112,     0,
       0,     0,     0,     0,     0,   216,   217,   218,   219,   220,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   369,     0,     0,
     370,     0,     0,    93,    94,     0,    95,   190,    97,     0,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   371,     0,     0,     0,   862,     0,     0,
     477,   478,   108,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   503,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   474,   475,   476,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   863,   477,   478,  1040,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,     0,     0,     0,     0,
       0,     0,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
    1161,  1162,  1163,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1164,  1566,     0,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1161,  1162,  1163,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1187,
       0,     0,     0,     0,     0,     0,     0,  1164,     0,     0,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,   475,   476,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1187,     0,     0,     0,     0,
       0,   477,   478,     0,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,  1162,
    1163,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     503,     0,     0,     0,     0,     0,     0,     0,     0,  1164,
       0,     0,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,  1186,   476,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1187,     0,     0,
       0,     0,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,  1163,   502,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,     0,     0,     0,     0,  1164,     0,     0,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1187,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   503,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,
    1182,  1183,  1184,  1185,  1186,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1187,  1166,
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
     503, -1135, -1135, -1135, -1135,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   503,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1187, -1135, -1135, -1135, -1135,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1187
};

static const yytype_int16 yycheck[] =
{
       5,     6,   132,     8,     9,    10,    11,    12,    13,    56,
      15,    16,    17,    18,   193,   124,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   167,    31,    56,    19,    20,
     734,   696,   109,    44,   109,   172,   173,    33,     4,    44,
      98,   109,   572,     4,   109,   103,   104,    52,     4,    54,
      46,   417,    57,     4,    59,    51,   541,   853,   723,   997,
     124,   417,   124,   448,   194,   537,   538,   417,    30,  1427,
     791,   502,   692,   131,   987,    86,   417,    30,   608,   878,
     560,    86,   167,    30,   999,   505,   506,    57,   534,  1228,
       4,    30,   693,   534,   566,   861,   450,   451,   452,   868,
       4,   671,   837,  1019,   109,  1021,     6,   621,   622,  1114,
    1034,  1908,   257,     9,   191,    57,   191,     9,     9,   539,
      32,   258,   568,   191,     9,     9,     9,   568,  1052,    14,
      14,    14,   246,   124,    32,    48,   367,   368,   369,     9,
     371,     9,     9,    48,    70,   222,     9,   222,    48,     9,
       9,    54,     4,     9,   222,     9,     9,   103,     9,     9,
       9,    48,     9,    38,    91,     9,    83,     9,   245,     9,
     245,     9,  1110,    14,     9,    48,  1100,     9,     9,     9,
      36,    91,   116,  2049,    83,   183,   191,    48,    50,    51,
      83,   162,    38,   198,   166,   162,   199,  1238,    91,   565,
      81,   199,  1001,     0,   162,    70,  2072,  1968,    83,  1970,
     199,    38,   183,   113,    19,    20,   183,   222,   168,   119,
     166,   121,   122,   123,   124,   125,   126,   127,   132,   201,
     202,    70,   159,    54,    38,   202,   162,    83,   183,   199,
     245,    70,    70,  1156,   202,   162,   180,    70,   239,   159,
      70,   132,   202,    70,   199,   260,    83,   402,   263,   270,
      70,   926,   103,   104,   167,   270,   271,   160,   161,   169,
     170,     8,   172,   199,  2071,    70,   202,   176,    70,    83,
      70,    70,   461,   196,    70,    70,    70,   200,  2085,   201,
     194,    70,    70,   997,   194,   200,  1579,   162,   264,   183,
      70,   176,   268,   203,   202,   201,   202,   136,   137,   201,
     201,   358,   389,   200,   389,   167,   201,   201,   201,    70,
    1055,   389,   184,   196,   202,  1029,  1242,  1685,   858,   358,
     176,   201,  1337,   201,   201,   112,  1475,   202,   201,   200,
      83,   201,   201,  1482,   200,  1484,   167,   201,   201,   176,
     201,   201,   201,   200,   359,  1124,   200,  1126,   200,   103,
     200,   182,   200,   202,   543,   200,  1290,   162,   200,   200,
     200,   448,   176,   448,   202,  1514,   199,   388,   162,   202,
     448,   202,   199,   388,   389,   202,   516,   378,   162,   199,
     395,   396,   397,   398,   399,   400,   387,   455,   389,   446,
     405,  1314,    38,   394,   199,   975,  1110,   202,   922,   923,
     202,  1694,   202,   418,    70,   406,   202,   446,   202,    38,
     425,   199,   166,   202,   202,   199,    70,   199,   433,   199,
    1031,   199,  1473,    83,   239,   122,  1719,  1720,    70,  1722,
     445,   183,    70,   448,    70,   132,  1245,    83,   199,    70,
     199,   202,   517,   511,   512,   513,   514,   199,   463,   464,
      83,   508,   205,    70,    83,    83,    84,   952,    91,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,  1636,   503,   199,
     505,   506,   507,   107,   108,  1226,   448,   554,   136,   137,
     424,    83,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,    83,    83,    84,   432,   136,
     137,    70,   537,   538,   539,   540,    83,   160,   161,   544,
     502,     4,   547,   199,   103,   195,   202,   517,    57,   502,
     541,   556,   202,   558,   699,   502,   701,  1356,   202,  1005,
      69,   566,   199,   502,  1005,    19,    20,  1333,   710,   574,
     202,   576,   183,   378,   202,   517,   202,  1049,   199,   200,
     432,    70,   387,   199,  1625,   208,  1627,   205,   199,   394,
      53,  1121,   199,    56,   199,   202,   199,   136,   137,   203,
      83,   406,   679,   565,   176,  1963,   199,   166,   107,   108,
      73,   515,   417,   579,   793,    87,   136,   137,   176,    70,
      31,   987,   627,   162,  1244,   710,  1057,    60,   103,   176,
      83,   987,   574,    96,  1019,    98,  1021,   987,    91,    50,
     103,   104,    53,   162,  1557,   202,   987,   166,   205,    83,
     136,   137,   124,   125,   126,    88,   183,  1103,    91,    81,
     199,  1460,  1103,  1153,   183,  1155,  1020,  1157,   131,  1023,
    1024,   785,   199,  1274,   679,    70,  1277,   160,   161,   199,
      70,   912,   104,   914,   742,   916,   831,   832,  1202,   920,
     167,   166,   199,   838,   839,   199,    31,    83,   136,   137,
     112,   838,  1743,   817,   203,    91,  1747,   160,   161,   121,
     122,   123,   124,   125,   126,    50,   721,   203,    53,   141,
     142,   143,   144,   145,     4,   159,   160,   161,   719,   123,
      32,   508,    78,    79,    80,    81,   541,   166,   132,    19,
      20,  1654,    50,    51,   207,   750,   118,   169,   170,   199,
     172,   173,   174,  1309,   207,  1311,   199,   534,   104,  1315,
      38,    83,  1483,   121,   122,   123,   124,   125,   126,    91,
     168,    14,    70,   159,   160,   161,   198,   554,   783,   693,
     894,    87,   194,   181,   201,   239,    83,   201,   565,    32,
    1156,   568,   201,   907,    91,   141,   142,   143,   144,   145,
    1156,   264,    83,   794,   202,   268,  1156,   812,    51,   272,
      91,  1283,  1292,    83,    84,  1156,    75,    76,   124,   125,
     126,   201,  1294,   169,   170,   201,   172,   173,   174,   109,
     734,  1451,   201,   398,   162,   400,   194,  1257,   160,   161,
     845,   201,   833,   201,   835,   367,   368,   369,   201,   371,
     861,  1336,   198,   418,   202,   183,   861,  1242,   710,    75,
      76,   857,   159,   160,   161,    70,  1907,    53,    54,    55,
    1911,   199,   863,    70,   202,    70,  1477,  1598,    70,   160,
     161,    70,   734,    69,   199,  1415,    70,   121,   122,   123,
     124,   125,   126,   403,    70,   358,   162,   407,   132,   133,
     866,    70,   121,   122,   123,   124,   125,   126,   199,  1613,
     199,   191,   103,   104,   719,    53,    54,    55,   162,    57,
     166,   106,   107,   108,   378,   435,   201,   437,   438,   439,
     440,    69,    49,   387,   367,   368,   369,   370,   371,    69,
     394,   175,   222,   990,   183,   936,  1431,   851,  1314,   201,
     202,   956,   406,   958,   162,   960,   134,   135,  1314,   239,
     194,   952,   953,   199,  1314,   245,   971,  1939,  1940,   432,
     106,   107,   108,  1314,  1695,   194,   409,   201,   202,   442,
       9,   986,   199,   446,   264,   162,  1516,   162,   268,   794,
    1935,  1936,   455,   113,   114,   115,  1004,  1005,  1056,   851,
     204,   199,  1532,    56,     8,   909,   199,  1479,   974,   162,
     162,  1016,    14,   974,  1019,   202,  1021,  2058,   974,   201,
     201,  1026,   201,   974,     9,   132,    14,   201,   833,   132,
     835,    14,   200,   183,  2075,  1040,  1637,   103,  1043,   200,
    1045,   200,   200,   200,  1049,   508,   509,   510,   511,   512,
     513,   514,   206,   199,   550,   202,   112,   909,   863,   199,
     974,    50,    51,    52,    53,    54,    55,  1452,    57,  2007,
     974,   534,   199,     9,  2012,   159,   200,  1019,   200,  1021,
      69,   200,   200,    95,  1026,   999,     9,   541,   201,    14,
     183,   554,     9,   997,  1099,  1057,   202,   199,   378,  2037,
     199,  1455,  1456,  1457,  1057,   568,   201,   387,   202,   389,
    1057,  1107,  1159,  2028,   394,   201,   579,  1031,  1057,   202,
     201,   201,   974,  1114,  1115,  1029,   406,   202,  1658,    83,
     200,   936,   200,  1242,   201,  2050,   599,  1667,  1252,   200,
     134,  1255,  1108,   200,  2059,   997,   204,   952,   953,  1621,
    1622,   199,   432,  1683,  1219,     9,     9,   653,   654,   204,
    2098,   204,   625,   626,  1960,   204,   204,    70,   448,  1965,
      32,    50,    51,    52,    53,    54,    55,  1029,  1242,   135,
    1242,   182,   987,   162,     9,   681,   138,   200,   621,   622,
      69,  1557,   162,   200,    14,     9,   196,   660,   661,     9,
     827,  1557,   184,   200,     9,    14,  1110,  1557,  1112,     9,
     200,   204,   134,   990,  1219,     9,  1557,  2013,   204,   200,
     716,    14,   200,   200,   204,  1230,   199,  1004,  1005,   203,
     162,   200,   200,   204,  1764,   200,   289,  1242,   291,   103,
     201,   201,     9,   138,   162,     9,    70,   200,   199,    70,
     199,  1242,  1257,    70,    70,  1260,   199,    70,  1110,  1225,
    1112,   541,  2061,   202,  1225,   719,  2065,  1258,     9,  1225,
     203,   734,    14,   201,  1225,   184,     9,  1219,  1283,   742,
    2079,  2080,   202,    14,   176,   781,   202,    14,  1654,  1294,
    1295,   204,  2088,   200,  1959,   201,  1961,   350,  1654,   579,
    1242,    70,   196,  2007,  1654,     6,    32,    32,  2012,  1114,
    1115,  1225,   199,  1654,   199,    14,   199,   199,    14,  1456,
      52,  1225,  1333,   199,  1101,    70,  1103,    70,  1333,  1325,
    1321,    70,    70,  2037,    70,   162,   199,     9,  1343,   199,
     794,   200,   138,   201,   201,  1336,  1337,    48,   199,    14,
     184,  1156,   138,     9,   162,   200,    69,   176,   176,     9,
    1274,   204,  1328,  1277,   827,    83,   829,   203,   203,     9,
     203,   203,   201,  1225,   870,  1452,   199,  1452,   138,   833,
     876,   835,    14,  2048,  1452,  1915,   201,   199,   851,    83,
     443,    83,   138,   446,  2098,   202,   200,   199,   199,   202,
       9,   200,   865,   866,   202,    92,   201,   204,   159,   863,
     202,    32,   113,    77,   201,   200,   184,   201,   119,   138,
     121,   122,   123,   124,   125,   126,   127,    32,   200,   200,
    1435,   204,     9,     9,   204,   138,  1340,  1442,   204,   719,
    1431,  1068,  1447,   204,  1449,   204,   909,  1452,     9,   203,
     200,   200,   948,  1258,   734,   918,   919,  1599,     9,   203,
    1465,   201,    14,  1593,   201,   200,    83,   201,   169,   170,
     200,   172,   201,  1478,  1479,     9,   199,   199,   202,   912,
     201,   914,   936,   916,   204,   200,   949,   920,  1340,   922,
     923,   924,   200,   194,   201,   199,   202,   138,   952,   953,
     200,   827,   203,   200,     9,   138,   204,   200,     9,  1314,
    1452,   974,   200,   204,   794,  1429,  1321,    32,   204,   204,
     204,   201,   200,   200,   138,  1439,   201,   990,   201,   176,
     113,  1336,  1337,   202,   997,   113,   171,    83,  1717,   113,
     593,  1004,  1005,   113,   113,   167,   201,    83,    14,   200,
      83,   119,   200,   833,   138,   835,  2086,   202,   200,   138,
      14,    14,   201,  1477,   183,   202,  1029,    14,    14,    83,
     200,   851,   200,   199,   198,   200,   138,  1429,   138,    83,
      14,   201,    14,   863,    14,   201,   866,  1439,   201,     9,
     202,     9,   203,  1056,    68,    14,   183,   199,  1094,    83,
       9,     9,   202,  1066,  1067,  1068,   201,   116,  1613,   103,
     103,   184,   162,  1618,    36,   174,  1621,  1622,   199,   201,
     200,   184,   180,    83,   199,     9,  1431,   184,   177,   909,
    1688,    83,    83,   200,    14,   201,   689,   690,  1101,    83,
    1103,    83,  1138,    14,  1691,  1108,  1142,  1110,   202,  1112,
      83,    14,   200,  1149,    83,   200,   936,    14,    83,  1202,
    1114,  1115,  2040,   509,  1568,   514,  1050,   511,   977,  2055,
    1133,  1334,   952,   953,  1763,  2050,  1259,  1750,  1788,  1306,
    1307,  1529,   629,  1310,  1696,  1875,  1588,  2096,  1592,  2072,
    1317,  1718,  1581,  1746,   974,  1154,  1159,   399,  1664,  1229,
    1310,  1150,  1067,  1305,  1096,  1306,   395,  1953,   879,  1613,
    2008,  1998,  1626,  1210,  1893,  1573,  1568,   997,  1632,  1134,
    1634,  1188,    -1,  1637,    -1,  1188,    -1,    -1,    -1,   446,
      -1,    -1,  1737,   121,   122,   123,   124,   125,   126,  1019,
      -1,  1021,  1068,  1657,   132,   133,    -1,  1599,    -1,  1029,
      -1,    -1,  1557,    -1,    -1,  1659,    -1,    -1,    -1,    -1,
      -1,  1613,  1225,    -1,  1668,    -1,    -1,    -1,    -1,  1202,
      -1,    -1,    -1,    -1,  1626,    -1,    -1,    -1,    -1,    -1,
    1632,    -1,  1634,    -1,    -1,   173,    -1,   175,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1762,  1763,    -1,    -1,
      -1,   189,    -1,   191,  1258,  1657,   194,  1659,  1304,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1668,    -1,    -1,    -1,
      -1,    -1,  1726,    -1,    -1,   878,   879,    -1,  1108,    -1,
    1110,    -1,  1112,    -1,  1114,  1115,    -1,    -1,  1752,    -1,
    1887,    -1,    -1,  1306,  1307,  1308,  1309,  1310,  1311,  1654,
      -1,    -1,  1315,    -1,  1317,    -1,    -1,  1761,  1887,    -1,
      -1,    -1,    -1,  1767,  1491,  1328,  1493,  1321,    -1,    -1,
    1774,  1367,    -1,    -1,    -1,  1371,    -1,  1340,    -1,  2021,
    1376,    -1,  1336,  1337,    -1,    -1,    -1,  1350,  1384,    -1,
      -1,    -1,    -1,  1898,    -1,    -1,    -1,    31,    -1,    -1,
    1752,    -1,    -1,    -1,    -1,    81,  1953,    -1,    -1,  1761,
      -1,    -1,    -1,    -1,  2044,  1767,    -1,    -1,    -1,    -1,
      -1,    -1,  1774,   976,  1953,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,
      -1,   994,    -1,    -1,    -1,  1225,    -1,    81,    -1,    30,
      -1,    -1,    -1,    -1,  1007,    -1,    -1,    -1,    92,    -1,
    1423,    -1,  1242,    -1,    -1,   141,   142,   143,   144,   145,
     104,    -1,    -1,    -1,    -1,    56,    -1,  1431,  1258,    -1,
    1306,  1307,  1308,  1309,  1310,  1311,    -1,   163,    -1,  1315,
     166,  1317,    -1,   169,   170,    -1,   172,   173,   174,    -1,
    1053,    -1,  1498,    -1,    -1,    -1,  1502,   141,   142,   143,
     144,   145,    -1,  1509,    -1,  1919,  1643,    -1,  1645,    -1,
    1647,    -1,   198,    -1,    -1,  1652,    -1,   203,  1491,   163,
    1493,    81,   166,    -1,    -1,   169,   170,    -1,   172,   173,
     174,  1321,   176,    -1,    -1,    -1,    -1,    -1,  1328,    -1,
      -1,    -1,    -1,    -1,   104,    -1,  1336,  1337,    -1,    -1,
    1340,    -1,    -1,    -1,   198,    -1,    -1,  1919,    -1,  1973,
      -1,    -1,    -1,    -1,    -1,  1128,    -1,    -1,    -1,  1132,
     130,    -1,    -1,    -1,  1137,    -1,    -1,    -1,    -1,  2094,
      -1,   141,   142,   143,   144,   145,    -1,    -1,  2103,    -1,
      -1,    -1,    -1,  2007,    -1,  1568,  2111,    -1,  2012,  2114,
      -1,    -1,    -1,    -1,  2028,    -1,  1579,    -1,    -1,   169,
     170,  1973,   172,   173,   174,  1588,    -1,  1754,    -1,    -1,
      -1,    -1,    -1,  2037,    -1,    -1,  2050,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  2059,    -1,    -1,   198,   199,
    1613,  1431,    -1,    -1,    -1,  2007,    -1,    -1,   239,    -1,
    2012,    -1,    -1,    -1,     6,  1491,    -1,  1493,    -1,  2021,
      -1,    -1,  1452,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1643,    -1,  1645,    -1,  1647,  2037,    -1,    -1,    -1,  1652,
    1243,    -1,    -1,  2097,  2098,    -1,  1659,    -1,    -1,    -1,
      -1,  1664,    -1,    -1,    -1,  1668,    48,    -1,   289,    -1,
     291,    -1,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,  1688,    57,    -1,  1691,    -1,
       6,  1694,    -1,  1286,    -1,    -1,  1289,    -1,    69,    -1,
      -1,  1704,    -1,    -1,    -1,  2097,  2098,    -1,  1711,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1719,  1720,    -1,  1722,
      -1,    -1,    -1,    -1,    -1,  1728,    -1,    -1,    -1,   350,
      -1,   113,    48,    -1,    -1,    -1,    -1,   119,    -1,   121,
     122,   123,   124,   125,   126,   127,  1913,  1914,  1568,    -1,
      -1,  1754,    -1,    -1,    -1,    -1,  1349,   378,  1761,  1762,
    1763,    -1,    -1,  1356,  1767,    -1,   387,    -1,    -1,    -1,
      -1,  1774,    -1,   394,    -1,    -1,    -1,  1643,    -1,  1645,
      -1,  1647,    -1,    -1,    -1,   406,  1652,   169,   170,    -1,
     172,    -1,    -1,  1613,    -1,    -1,   417,   113,    -1,    -1,
      -1,    -1,    -1,   119,    -1,   121,   122,   123,   124,   125,
     126,   127,   194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   203,   443,    -1,    -1,   446,    -1,    -1,    -1,    -1,
      -1,  1424,  1425,    -1,    -1,    -1,    -1,    -1,    -1,  1659,
      -1,    -1,    -1,    -1,  1664,    -1,    -1,    -1,  1668,    -1,
      -1,    -1,    -1,   169,   170,    -1,   172,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,    -1,     6,    -1,    -1,   194,    -1,
      -1,   502,    -1,    -1,  1887,    -1,    -1,   203,  1754,    -1,
      -1,    -1,    -1,  1929,  1930,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    59,    60,    -1,  1909,    -1,    -1,    -1,
    1913,  1914,    -1,    -1,    -1,    -1,  1919,    48,    -1,    -1,
     541,    -1,    -1,    -1,    -1,  1928,    -1,    -1,    -1,    -1,
      -1,    -1,  1935,  1936,    -1,    -1,  1939,  1940,    -1,    -1,
      -1,  1761,  1762,  1763,  1537,  1538,    -1,  1767,  1541,    -1,
    1953,    -1,    -1,    -1,  1774,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    1973,    -1,   593,    -1,   595,    -1,    -1,   598,    -1,    -1,
     136,   137,   113,    -1,    -1,    -1,  1579,    -1,   119,    -1,
     121,   122,   123,   124,   125,   126,   127,    -1,    81,    -1,
    1593,     6,    59,    60,  2007,    -1,    -1,    -1,    -1,  2012,
     631,    -1,    -1,    -1,    -1,    -1,    -1,  2020,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
     113,     6,    -1,    -1,  2037,    -1,    -1,    -1,   169,   170,
    2043,   172,    -1,    48,   200,    -1,    -1,  1913,  1914,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   194,    -1,    -1,    -1,    -1,   689,   690,
      -1,    -1,   203,    48,    -1,    -1,  1669,   698,    -1,   136,
     137,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,  2097,  2098,    -1,    -1,   719,  1919,
      -1,  1694,    -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,
      -1,    -1,    -1,    -1,   119,   198,   121,   122,   123,   124,
     125,   126,   127,    -1,    -1,    -1,  1719,  1720,    -1,  1722,
      -1,    -1,    -1,    -1,    -1,  1728,    -1,    -1,   113,    -1,
      -1,    -1,    -1,   200,   119,    -1,   121,   122,   123,   124,
     125,   126,   127,  1973,    78,    79,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   169,   170,    -1,   172,    92,    -1,
      -1,    -1,    -1,   794,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,  2007,    -1,   194,
      -1,    -1,  2012,  1786,   169,   170,    -1,   172,   203,    -1,
      69,    -1,    -1,    -1,    -1,    -1,   827,    -1,    -1,    -1,
      -1,    -1,   833,    -1,   835,    -1,    -1,  2037,    -1,   194,
      -1,    -1,    -1,   147,   148,   149,   150,   151,   203,    -1,
      -1,    -1,    -1,    81,   158,    83,    84,    -1,    -1,    -1,
     164,   165,   863,   864,    -1,    -1,    -1,    -1,    -1,    -1,
     871,    -1,    -1,    -1,   178,    -1,   104,   878,   879,   880,
     881,   882,   883,   884,    -1,    -1,    -1,    -1,    -1,   193,
      -1,    -1,   893,    -1,    -1,    -1,    -1,  2097,  2098,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   910,
      -1,    -1,    -1,   141,   142,   143,   144,   145,    -1,  1892,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,  1905,    -1,    -1,   936,  1909,    -1,    -1,   167,
      -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,   950,
      -1,   952,   953,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    68,    -1,    -1,
     198,    -1,    19,    20,   202,   976,   977,   205,    -1,    -1,
      81,    -1,    -1,    30,    -1,    -1,   987,    -1,    -1,    -1,
      -1,    -1,    -1,   994,    -1,    -1,    -1,    -1,    59,    60,
    1001,    -1,    -1,   104,    -1,    -1,  1007,    -1,    -1,  1982,
      -1,   112,    -1,    -1,    -1,    -1,    -1,  1018,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,  1033,    -1,    -1,  2008,    -1,  2010,    -1,   140,
     141,   142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,
      -1,    -1,  1053,    -1,    -1,    -1,  1057,    -1,    -1,    -1,
      -1,    -1,   163,    59,    60,   166,   167,  1068,   169,   170,
      -1,   172,   173,   174,    -1,   136,   137,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,
      -1,    -1,    56,    -1,    -1,    -1,    -1,   198,   199,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1114,  1115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1128,    -1,    -1,
      -1,  1132,    -1,  1134,    -1,    -1,  1137,    -1,    -1,   200,
     136,   137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1150,
    1151,  1152,  1153,  1154,  1155,  1156,  1157,    -1,    -1,  1160,
    1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,    10,    11,    12,
      -1,    -1,   239,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   200,  1206,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,  1243,    -1,  1245,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,  1258,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,  1286,    -1,    -1,  1289,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1306,  1307,  1308,  1309,  1310,
    1311,    -1,    -1,  1314,  1315,    -1,  1317,    -1,    -1,    -1,
    1321,    -1,    -1,    -1,    -1,   289,    -1,   291,    -1,    -1,
      -1,   378,    -1,    -1,    -1,  1336,  1337,    -1,  1339,    -1,
     387,    -1,    -1,    -1,    -1,    -1,    -1,   394,  1349,    -1,
      -1,    -1,    -1,    -1,    -1,  1356,    -1,    -1,    -1,   406,
    1361,    -1,  1363,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     417,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,   204,    -1,    -1,    -1,    -1,   350,  1388,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,  1424,  1425,    -1,   201,  1428,   203,    -1,
    1431,    -1,    69,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   502,    -1,    -1,    -1,  1460,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,   443,
      59,    60,   446,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1491,    -1,  1493,    -1,   541,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,  1537,  1538,    -1,    -1,
    1541,    -1,    -1,    -1,    -1,    -1,  1547,    -1,  1549,    -1,
    1551,   598,    -1,    -1,    -1,  1556,  1557,   136,   137,    -1,
    1561,    -1,  1563,    -1,    -1,  1566,   203,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,  1579,  1580,
      -1,    -1,    -1,    -1,   631,  1586,    -1,    -1,    -1,    -1,
      -1,    -1,  1593,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,   200,    -1,    -1,    -1,    -1,    31,    -1,    -1,   593,
      -1,   595,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1643,    -1,  1645,    -1,  1647,    -1,    -1,    -1,
      -1,  1652,    -1,  1654,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1669,   203,
      -1,    -1,   719,  1674,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1686,  1687,    -1,    -1,    -1,
      -1,    -1,    -1,  1694,    -1,  1696,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1719,  1720,
      -1,  1722,   127,    -1,    -1,   689,   690,  1728,    -1,    -1,
      -1,    -1,    -1,    -1,   698,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    68,   794,    -1,    -1,
      -1,    -1,    -1,  1754,    -1,    -1,    -1,    -1,   163,    81,
      -1,   166,   167,    -1,   169,   170,   203,   172,   173,   174,
    1771,  1772,  1773,    -1,    -1,    -1,    -1,  1778,    -1,  1780,
     827,    -1,   104,    -1,    -1,  1786,   833,  1788,   835,    -1,
     112,    -1,    -1,   198,    -1,    -1,    -1,    -1,    -1,   121,
     122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   863,   864,   140,   141,
     142,   143,   144,   145,   146,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   880,   881,   882,   883,   884,    -1,    -1,
      -1,   163,    -1,    -1,   166,   167,   893,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   910,    -1,   187,    -1,    -1,    -1,    -1,
      -1,    -1,   194,    -1,    -1,    81,   198,   199,    -1,    -1,
      -1,    -1,    -1,  1884,    -1,    -1,    92,    -1,    -1,   936,
      -1,  1892,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,   950,  1905,   952,   953,   871,  1909,    -1,
      -1,    -1,  1913,  1914,   878,   879,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1928,    -1,    -1,
     977,    -1,    -1,  1934,    -1,   141,   142,   143,   144,   145,
     987,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1950,
    1951,    -1,    -1,    81,  1955,    83,    84,   163,    -1,    -1,
     166,    -1,    -1,   169,   170,    -1,   172,   173,   174,    -1,
     176,  1018,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
      -1,  1982,    -1,    -1,    -1,    -1,  1033,    -1,    -1,    -1,
      19,    20,   198,    -1,    -1,    81,  1997,    83,    -1,    85,
      -1,    30,    -1,    -1,    -1,    -1,    -1,  2008,    -1,  2010,
    1057,    -1,   976,   141,   142,   143,   144,   145,   104,    -1,
      -1,  1068,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     994,    -1,    -1,    -1,    -1,    -1,    -1,  1001,  2039,   167,
      -1,   169,   170,  1007,   172,   173,   174,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  2055,   141,   142,   143,   144,   145,
    2061,    -1,    -1,    -1,  2065,    -1,    -1,  1114,  1115,    -1,
     198,    -1,    -1,    -1,   202,    -1,    -1,   205,  2079,  2080,
      -1,    -1,    -1,   169,   170,    -1,   172,   173,   174,  1053,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1150,  1151,  1152,  1153,  1154,  1155,  1156,
    1157,    -1,   198,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,
    1187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,  1206,
      -1,    -1,    -1,    -1,  1128,    -1,    -1,    -1,  1132,    -1,
    1134,    -1,    -1,  1137,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
     239,  1258,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,  1306,
    1307,  1308,  1309,  1310,  1311,    69,    -1,  1314,  1315,    -1,
    1317,    -1,    -1,    -1,  1321,    -1,    -1,    -1,    -1,  1243,
      -1,  1245,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1336,
    1337,    -1,  1339,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,  1361,    -1,  1363,    -1,    -1,    -1,
      -1,    -1,  1286,    -1,    -1,  1289,    69,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,  1388,    -1,    -1,    -1,    -1,    -1,   203,    -1,   378,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   387,    -1,
      -1,    -1,    -1,    -1,    -1,   394,    -1,    -1,    -1,    -1,
      -1,    -1,    59,    60,    -1,    -1,    -1,   406,    81,    -1,
      -1,  1428,    -1,    -1,  1431,  1349,    -1,    -1,   417,    10,
      11,    12,  1356,    -1,    -1,    -1,    -1,    -1,    -1,   203,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,   141,   142,
     143,   144,   145,    -1,  1491,    -1,  1493,    -1,    69,   136,
     137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1424,  1425,    -1,   166,    -1,    -1,   169,   170,    -1,   172,
     173,   174,    -1,   502,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   598,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   198,  1460,    -1,    -1,   202,
    1547,    -1,  1549,    -1,  1551,    -1,    -1,    -1,    -1,  1556,
    1557,    -1,   541,    -1,  1561,    -1,  1563,    -1,   631,  1566,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1580,    -1,    -1,    -1,    -1,    -1,  1586,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,   598,
      -1,    -1,    -1,  1537,  1538,    -1,    -1,  1541,    -1,    69,
      -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1643,    -1,  1645,    -1,
    1647,    -1,   631,    -1,    -1,  1652,    -1,  1654,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1579,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1674,    -1,  1593,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,  1686,
    1687,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1696,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
     719,    -1,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1669,    -1,  1754,    -1,    -1,
      -1,    -1,    70,   203,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,  1771,  1772,  1773,    -1,    -1,    -1,
    1694,  1778,    -1,  1780,    92,    -1,    -1,    -1,    -1,    -1,
      -1,  1788,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
      -1,   864,    -1,    -1,    -1,  1719,  1720,    -1,  1722,    -1,
      -1,    -1,    -1,    -1,  1728,   794,    -1,   880,   881,   882,
     883,   884,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     893,    -1,   140,   141,   142,   143,   144,   145,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     158,    -1,    -1,    -1,   833,   163,   835,    -1,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
      -1,    81,  1786,   203,    -1,    -1,    -1,    -1,    -1,   187,
     188,    -1,    -1,    -1,   863,   864,    -1,  1884,    -1,    -1,
     198,    -1,    31,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,   880,   881,   882,   883,   884,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   893,    -1,  1913,  1914,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,   141,   142,   143,   144,   145,    -1,  1934,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,    -1,  1950,  1951,  1018,    -1,   936,  1955,   169,
     170,    -1,   172,   173,   174,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   952,   953,    -1,    -1,    -1,  1892,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,   199,
      -1,  1905,    -1,    -1,    -1,  1909,    -1,    -1,    -1,    -1,
    1997,   140,   141,   142,   143,   144,   145,   146,   987,    -1,
      -1,    -1,    -1,    -1,  1928,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,   176,    -1,  1018,
      -1,    -1,  2039,    -1,    -1,    -1,    -1,    -1,   187,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2055,   198,
     199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1982,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1057,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1150,  1151,    -1,
      -1,  1154,    -1,    -1,  2008,    -1,  2010,  1160,  1161,  1162,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,  1186,  1187,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1114,  1115,    -1,    -1,    -1,
      -1,    -1,    -1,  1206,    -1,    -1,    -1,  2061,    -1,    -1,
      -1,  2065,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  2079,  2080,    -1,    -1,    -1,
      -1,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,    -1,
      -1,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,    10,
      11,    12,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,  1206,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    81,    57,    -1,    -1,    59,
      60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1339,    -1,   104,  1258,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    -1,  1361,    -1,
    1363,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,    -1,  1388,    -1,    31,    59,    60,
      -1,    -1,    -1,    -1,    -1,  1314,   136,   137,    -1,    -1,
      -1,   167,  1321,   169,   170,   171,   172,   173,   174,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1336,  1337,    -1,
    1339,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   198,   199,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,  1361,    87,  1363,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,   203,    -1,    -1,   136,   137,    30,    31,  1388,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    69,    -1,    81,    -1,
      -1,    -1,  1431,    -1,    -1,    -1,    -1,    -1,    -1,   163,
      -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,   104,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   187,  1547,    -1,  1549,    -1,  1551,    -1,
      -1,    -1,    -1,  1556,   198,   199,    -1,    -1,  1561,    -1,
    1563,    -1,    -1,  1566,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,
      11,    12,    13,    -1,    -1,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,    -1,    -1,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   198,   199,    -1,  1547,    -1,
    1549,    -1,  1551,    -1,    -1,    -1,    57,  1556,  1557,    -1,
     203,    -1,  1561,    -1,  1563,    -1,    -1,  1566,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    83,    84,    -1,    -1,    87,    -1,    -1,    -1,
      -1,  1674,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    -1,   130,
      -1,   132,   133,   134,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,   146,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1654,    -1,    -1,    -1,    -1,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,   170,
     104,   172,   173,   174,   175,  1674,   177,    -1,    -1,   180,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   188,  1771,  1772,
    1773,    -1,    81,    -1,    -1,  1778,    -1,   198,    -1,    -1,
      -1,   202,    -1,    -1,   205,    -1,    -1,   141,   142,   143,
     144,   145,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,
      11,    12,    13,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,    -1,
      -1,    -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1771,  1772,  1773,    -1,    57,   166,    -1,  1778,
     169,   170,    -1,   172,   173,   174,    -1,    -1,  1787,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,  1884,    -1,    -1,    -1,    -1,    87,    -1,    -1,   198,
      -1,    -1,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    81,   130,
      -1,  1934,   133,   134,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,   146,    -1,  1950,  1951,    -1,
      -1,   104,  1955,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   163,    -1,    -1,  1884,    -1,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,   177,    -1,    -1,   180,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   188,   141,   142,
     143,   144,   145,    -1,  1997,    -1,    -1,   198,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     163,    -1,    -1,   166,   167,  1934,   169,   170,    -1,   172,
     173,   174,    -1,    -1,     3,     4,     5,     6,     7,    -1,
      -1,  1950,  1951,    -1,    13,    -1,  1955,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   198,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1984,    -1,    -1,    -1,    48,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,  1997,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,
      99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,
     109,   110,   111,   112,   113,   114,   115,    -1,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
     129,   130,   131,   132,   133,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
     189,    -1,   191,    -1,   193,   194,   195,    -1,    -1,   198,
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
      -1,    -1,   109,   110,   111,   112,   113,   114,   115,    -1,
     117,    -1,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,   129,   130,   131,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,   188,   189,    -1,   191,    -1,   193,   194,   195,    -1,
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
     109,   110,   111,   112,    -1,   114,   115,    -1,   117,   118,
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
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    -1,
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
      93,    94,    95,    -1,    97,    -1,    99,    -1,   101,   102,
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
      -1,    -1,   205,   206,   207,   208,     3,     4,     5,     6,
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
      71,    72,    73,    74,    -1,    -1,    77,    78,    79,    80,
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
     201,   202,    -1,    -1,   205,   206,   207,   208,     3,     4,
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
      99,   100,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,
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
      93,    94,    95,    -1,    97,    98,    99,    -1,   101,    -1,
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
      -1,    -1,   205,   206,   207,   208,     3,     4,     5,     6,
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
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,
      -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,     3,
       4,     5,     6,     7,   205,   206,   207,   208,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    29,    -1,    -1,    -1,    -1,
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
     194,   195,    -1,    -1,   198,   199,    -1,   201,    -1,    -1,
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
      -1,   176,    -1,   178,    -1,    -1,   181,     3,     4,     5,
       6,     7,   187,   188,    -1,    -1,    -1,    13,   193,   194,
     195,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,
     205,   206,   207,   208,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    87,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   121,   122,   123,   124,   125,
     126,    -1,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,
      -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,    -1,
     176,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,
      -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,
      -1,    -1,   198,   199,     3,     4,     5,     6,     7,   205,
     206,   207,   208,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
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
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
      -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,
     199,    -1,    -1,   202,    -1,    -1,   205,   206,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,
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
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   109,    -1,    -1,   112,    -1,    -1,    -1,    -1,
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
      -1,    -1,   181,     3,     4,     5,     6,     7,   187,   188,
      -1,    -1,    -1,    13,   193,   194,   195,    -1,    -1,   198,
     199,    -1,   201,    -1,    -1,    -1,   205,   206,   207,   208,
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
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,
      -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,
      -1,   201,    -1,    -1,    -1,   205,   206,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,
     195,    -1,    -1,   198,   199,   200,    -1,    -1,    -1,    -1,
     205,   206,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
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
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     174,    -1,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,
      -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,
     194,   195,    -1,    -1,   198,   199,     3,     4,     5,     6,
       7,   205,   206,   207,   208,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     178,    -1,    -1,   181,     3,     4,     5,     6,     7,   187,
     188,    -1,    -1,    -1,    13,   193,   194,   195,    -1,    -1,
     198,   199,    -1,    -1,    -1,    -1,    -1,   205,   206,   207,
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
     169,   170,    -1,   172,   173,   174,    -1,     3,     4,   178,
       6,     7,   181,    -1,    10,    11,    12,    13,   187,   188,
      -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,
     199,    -1,    28,    29,    -1,    -1,   205,   206,   207,   208,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    57,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    83,    84,    -1,
      -1,    87,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    -1,   130,    -1,   132,   133,   134,   135,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
     146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,   163,    10,    11,
      12,    13,    -1,   169,   170,    -1,   172,   173,   174,   175,
      -1,   177,    -1,    -1,   180,    -1,    28,    29,    -1,    31,
      -1,    -1,   188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   198,    -1,    -1,    -1,   202,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      -1,    93,    94,    95,    96,    97,    98,    99,   100,   101,
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
     123,   124,   125,   126,   127,   128,    -1,   130,    -1,    -1,
     133,   134,   135,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     163,    -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,   175,   104,   177,    -1,    -1,   180,    -1,     3,
       4,    -1,     6,     7,   187,   188,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,
     203,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,    -1,
     141,   142,   143,   144,   145,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    57,    57,   166,    -1,    -1,   169,   170,
      -1,   172,   173,   174,    68,    -1,    69,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    87,    -1,    -1,    -1,   198,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    -1,   130,    -1,   132,   133,
     134,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,
      -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,   175,    -1,   177,    -1,    -1,   180,    -1,     3,     4,
      -1,     6,     7,   187,   188,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    29,    -1,    31,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    69,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    -1,   130,    -1,    -1,   133,   134,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,
      -1,    -1,   187,   188,   189,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,   198,   199,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    29,    -1,    31,    31,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    69,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    -1,   130,    -1,    -1,   133,   134,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,   177,    -1,    -1,   180,    -1,     3,     4,     5,
       6,     7,   187,   188,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    57,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    87,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,   164,   165,
      -1,    81,    -1,   169,   170,    -1,   172,   173,   174,   175,
      -1,   177,   178,    -1,   180,    -1,    -1,    -1,    -1,    -1,
      -1,   187,    -1,   189,   104,   191,    -1,   193,   194,    -1,
       3,     4,   198,     6,     7,    -1,    -1,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,    -1,   127,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    29,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   163,    57,    -1,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,   198,    -1,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,    -1,   130,    -1,   132,
     133,   134,   135,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
     163,    10,    11,    12,    13,    -1,   169,   170,    -1,   172,
     173,   174,   175,    -1,   177,    -1,    -1,   180,    -1,    28,
      29,    -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
      -1,   130,    -1,   132,   133,   134,   135,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,   146,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,   177,    -1,
      -1,   180,    10,    11,    12,    -1,    -1,    -1,    -1,   188,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,   203,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,   201,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   201,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,   201,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
     201,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   201,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,   201,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      31,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   200,    -1,    -1,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,    -1,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   200,    -1,   163,    68,    -1,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,   140,
     141,   142,   143,   144,   145,   146,    -1,    -1,    -1,   198,
     199,   104,    -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,
     200,    -1,   163,    -1,    -1,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    -1,   187,   140,   141,   142,
     143,   144,   145,   146,    -1,    -1,    70,   198,   199,    -1,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,    -1,
     163,    -1,    -1,   166,   167,    -1,   169,   170,    92,   172,
     173,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,   198,   199,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,   140,   141,   142,   143,
     144,   145,    70,   147,   148,   149,   150,   151,    -1,    69,
      78,    79,    80,    81,   158,    83,    84,    -1,    -1,   163,
     164,   165,   166,   167,    92,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,   178,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,
      -1,    -1,    -1,   121,   198,   199,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    70,   147,
     148,   149,   150,   151,    -1,    -1,    78,    79,    80,    81,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      92,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
     178,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,
     198,   199,    -1,    -1,    -1,    -1,    -1,   205,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,    70,   147,   148,   149,   150,   151,
      -1,    -1,    78,    79,    80,    81,   158,    83,    84,    -1,
      -1,   163,   164,   165,   166,   167,    92,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,   178,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,
      -1,   193,    -1,    -1,    -1,   121,   198,   199,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,   145,
      70,   147,   148,   149,   150,   151,    -1,    -1,    78,    79,
      80,    81,   158,    83,    84,    -1,    -1,   163,   164,   165,
     166,   167,    92,   169,   170,    -1,   172,   173,   174,    -1,
      -1,    -1,   178,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,
      -1,   121,   198,   199,    -1,    -1,   202,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    70,    71,    -1,   178,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,   193,    -1,    -1,    -1,    92,   198,   199,
      -1,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   121,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,   140,   141,   142,   143,   144,
     145,    70,   147,   148,   149,   150,   151,    -1,    69,    78,
      79,    80,    81,   158,    83,    84,    -1,    -1,   163,   164,
     165,   166,   167,    92,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,   178,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,
      -1,    -1,   121,   198,   199,    -1,    -1,    -1,    -1,    -1,
     205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    70,   147,   148,
     149,   150,   151,    -1,    -1,    78,    79,    80,    81,   158,
      83,    84,    -1,    -1,   163,   164,   165,   166,   167,    92,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,   121,   198,
     199,    -1,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     193,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,
      30,    31,   205,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
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
      -1,    -1,    -1,    -1,    -1,    -1,   138,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,    -1,    31,    -1,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,    31,    -1,    -1,    34,
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
      53,    54,    55,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    92,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
      -1,    -1,    -1,   158,    -1,   138,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,   178,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,   193,    92,
      -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,
     205,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    69,
      -1,    78,    79,    80,    81,   158,    83,    84,    -1,    -1,
     163,   164,   165,   166,   167,    92,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     193,    -1,    -1,    -1,   121,   198,   199,    -1,    -1,    -1,
      -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    81,    -1,    -1,    -1,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   104,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   112,   113,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,
      -1,   198,   199,    -1,    -1,    -1,    -1,    -1,   205,    -1,
      -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,
     166,    -1,    -1,   169,   170,    -1,   172,   173,   174,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   189,    -1,    -1,    -1,    27,    -1,    -1,
      30,    31,   198,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    32,    -1,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
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
      -1,    -1,    -1,    -1,    -1,    69,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    35,
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
      69,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69
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
       9,   201,     9,   201,   201,   495,   336,   200,   308,   167,
     414,   203,   203,   359,    83,    83,   176,    14,    83,   359,
     303,   303,   119,   362,   509,   203,   509,   200,   200,   203,
     202,   203,   311,   300,   138,   444,   444,   444,   444,   373,
     203,   233,   239,   242,    32,   236,   287,   233,   522,   200,
     432,   138,   138,   138,   233,   423,   423,   500,    14,   217,
       9,   201,   202,   498,   495,   322,   183,   202,     9,   201,
       3,     4,     5,     6,     7,    10,    11,    12,    13,    27,
      28,    29,    57,    71,    72,    73,    74,    75,    76,    77,
      87,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   139,   140,   147,   148,   149,   150,   151,
     163,   164,   165,   175,   177,   178,   180,   187,   189,   191,
     193,   194,   216,   420,   421,     9,   201,   167,   171,   216,
     330,   331,   332,   201,    14,     9,   201,   256,   341,   498,
     498,   498,    14,   257,   203,   309,   310,   498,    14,    83,
     359,   200,   200,   199,   509,   198,   506,   362,   509,   308,
     203,   200,   444,   138,   138,    32,   236,   286,   287,   233,
     432,   432,   432,   203,   201,   201,   432,   423,   315,   522,
     323,   324,   431,   320,    14,    32,    51,   325,   328,     9,
      36,   200,    31,    50,    53,   432,    83,   218,   499,   201,
      14,    14,   522,   256,   201,    14,   359,    38,    83,   411,
     202,   507,   508,   522,   201,   202,   333,   509,   506,   203,
     509,   444,   444,   233,   100,   252,   203,   216,   229,   316,
     317,   318,     9,   438,     9,   438,   203,   432,   421,   421,
      68,   326,   331,   331,    31,    50,    53,    14,   183,   199,
     432,   432,   499,   432,    83,     9,   439,   231,     9,   439,
      14,   510,   231,   202,   333,   333,    98,   201,   116,   243,
     162,   103,   522,   184,   431,   174,   432,   511,   313,   199,
      38,    83,   200,   203,   508,   522,   203,   231,   201,   199,
     180,   255,   216,   336,   337,   184,   184,   298,   299,   457,
     314,    83,   203,   423,   253,   177,   216,   201,   200,     9,
     439,    87,   124,   125,   126,   339,   340,   298,    83,   283,
     201,   509,   457,   523,   523,   200,   200,   201,   506,    87,
     339,    83,    38,    83,   176,   509,   202,   201,   202,   334,
     523,   523,    83,   176,    14,    83,   506,   233,   231,    83,
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
     310,   311,   311,   312,   312,   312,   312,   312,   313,   312,
     314,   312,   312,   312,   312,   312,   312,   312,   312,   315,
     315,   315,   316,   317,   317,   318,   318,   319,   319,   320,
     320,   321,   321,   322,   322,   322,   322,   322,   322,   322,
     323,   323,   324,   325,   325,   326,   326,   327,   327,   328,
     329,   329,   329,   330,   330,   330,   330,   331,   331,   331,
     331,   331,   331,   331,   332,   332,   332,   333,   333,   334,
     334,   335,   335,   336,   336,   337,   337,   338,   338,   338,
     338,   338,   338,   338,   339,   339,   340,   340,   340,   341,
     341,   341,   341,   342,   342,   342,   343,   343,   344,   344,
     345,   346,   347,   347,   347,   347,   347,   347,   348,   348,
     349,   349,   350,   350,   350,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   359,   359,   359,   359,   360,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   362,   362,
     364,   363,   365,   363,   367,   366,   368,   366,   369,   366,
     370,   366,   371,   366,   372,   372,   372,   373,   373,   374,
     374,   375,   375,   376,   376,   377,   377,   378,   379,   379,
     380,   380,   381,   381,   382,   382,   383,   383,   384,   384,
     385,   385,   386,   387,   388,   389,   390,   391,   392,   393,
     394,   395,   396,   397,   398,   399,   400,   401,   402,   403,
     404,   404,   405,   405,   406,   406,   407,   408,   409,   409,
     410,   410,   410,   410,   410,   410,   410,   410,   410,   410,
     410,   410,   411,   411,   411,   411,   412,   413,   413,   414,
     414,   415,   415,   415,   416,   416,   417,   418,   418,   419,
     419,   419,   420,   420,   420,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   422,   423,
     423,   424,   424,   424,   424,   424,   425,   425,   426,   426,
     426,   426,   427,   427,   427,   428,   428,   428,   429,   429,
     429,   430,   430,   431,   431,   431,   431,   431,   431,   431,
     431,   431,   431,   431,   431,   431,   431,   431,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   433,   433,   434,   435,   435,   436,
     436,   436,   436,   436,   436,   436,   437,   437,   438,   438,
     439,   439,   440,   440,   440,   440,   441,   441,   441,   441,
     441,   442,   442,   442,   442,   443,   443,   444,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   445,   445,   446,   446,   447,   447,   447,
     447,   448,   448,   449,   449,   450,   450,   451,   451,   452,
     452,   453,   453,   455,   454,   456,   457,   457,   458,   458,
     459,   459,   459,   460,   460,   461,   461,   462,   462,   463,
     463,   464,   464,   464,   465,   465,   466,   466,   467,   467,
     468,   468,   468,   468,   468,   468,   468,   468,   468,   468,
     468,   469,   470,   470,   470,   470,   470,   470,   470,   470,
     471,   471,   471,   471,   471,   471,   471,   471,   471,   471,
     471,   472,   473,   473,   474,   474,   474,   475,   475,   475,
     476,   477,   477,   477,   478,   478,   478,   478,   479,   479,
     480,   480,   480,   480,   480,   480,   481,   481,   481,   481,
     481,   482,   482,   482,   482,   482,   482,   483,   483,   484,
     484,   484,   484,   484,   484,   484,   484,   485,   485,   486,
     486,   486,   486,   487,   487,   488,   488,   488,   488,   489,
     489,   489,   489,   490,   490,   490,   490,   490,   490,   491,
     491,   491,   492,   492,   492,   492,   492,   492,   492,   492,
     492,   492,   492,   493,   493,   494,   494,   495,   495,   496,
     496,   496,   496,   497,   497,   498,   498,   499,   499,   500,
     500,   501,   501,   502,   502,   503,   504,   504,   504,   504,
     504,   504,   505,   505,   505,   505,   506,   506,   507,   507,
     508,   508,   509,   509,   510,   510,   511,   512,   512,   513,
     513,   513,   513,   514,   514,   514,   515,   515,   515,   515,
     516,   516,   517,   517,   517,   517,   518,   519,   520,   520,
     521,   521,   522,   522,   522,   522,   522,   522,   522,   522,
     522,   522,   522,   523,   523
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
       5,     1,     3,     5,     4,     5,     3,     3,     3,     4,
       3,     3,     3,     3,     2,     2,     1,     1,     3,     1,
       1,     0,     1,     2,     4,     3,     3,     6,     2,     3,
       2,     3,     6,     3,     1,     1,     1,     1,     1,     3,
       6,     3,     4,     6,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     5,     4,     3,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     1,     5,     0,
       0,    12,     0,    13,     0,     4,     0,     7,     0,     5,
       0,     3,     0,     6,     2,     2,     4,     1,     1,     5,
       3,     5,     3,     2,     0,     2,     0,     4,     4,     3,
       2,     0,     5,     3,     2,     0,     5,     3,     2,     0,
       5,     3,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       2,     0,     2,     0,     2,     0,     4,     4,     4,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     3,     4,     1,     2,     4,     2,     6,     0,
       1,     0,     5,     4,     2,     0,     1,     1,     3,     1,
       3,     1,     1,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     4,     1,
       1,     1,     1,     1,     1,     3,     1,     3,     1,     1,
       1,     3,     1,     1,     1,     2,     1,     0,     0,     1,
       1,     3,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     1,     1,
       4,     3,     4,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     4,     3,     1,     3,     3,     1,     1,
       1,     1,     1,     3,     3,     3,     2,     0,     1,     0,
       1,     0,     5,     3,     3,     1,     1,     1,     1,     3,
       2,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       3,     1,     2,     2,     4,     3,     4,     1,     1,     1,
       1,     1,     1,     3,     1,     2,     0,     5,     3,     3,
       1,     3,     1,     2,     0,     5,     3,     2,     0,     3,
       0,     4,     2,     0,     3,     3,     1,     0,     1,     1,
       1,     1,     3,     1,     1,     1,     3,     1,     1,     3,
       3,     2,     2,     2,     2,     4,     5,     5,     5,     5,
       1,     1,     1,     1,     1,     1,     3,     3,     4,     4,
       3,     3,     1,     1,     1,     1,     3,     1,     4,     3,
       1,     1,     1,     1,     1,     3,     3,     1,     1,     4,
       4,     3,     1,     1,     7,     9,     9,     7,     6,     8,
       1,     4,     4,     1,     1,     1,     4,     2,     1,     0,
       1,     1,     1,     3,     3,     3,     0,     1,     1,     3,
       3,     2,     3,     6,     0,     1,     4,     2,     0,     5,
       3,     3,     1,     6,     4,     4,     2,     2,     0,     5,
       3,     3,     1,     2,     0,     5,     3,     3,     1,     2,
       2,     1,     2,     1,     4,     3,     3,     6,     3,     1,
       1,     1,     4,     4,     4,     4,     4,     4,     2,     2,
       4,     2,     2,     1,     3,     3,     3,     0,     2,     5,
       6,     6,     7,     1,     2,     1,     2,     1,     4,     1,
       4,     3,     0,     1,     3,     2,     1,     2,     4,     3,
       3,     1,     4,     2,     2,     0,     0,     3,     1,     3,
       3,     2,     0,     2,     2,     2,     2,     1,     2,     4,
       2,     5,     3,     1,     1,     0,     3,     4,     5,     6,
       3,     1,     3,     2,     1,     0,     4,     1,     3,     2,
       4,     5,     2,     2,     1,     1,     1,     1,     3,     2,
       1,     8,     6,     1,     0
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
#line 7373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 763 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 7381 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 770 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 7387 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 771 "hphp.y" /* yacc.c:1646  */
    { }
#line 7393 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 7399 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 775 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7405 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7411 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7417 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7423 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 779 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 7429 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 7437 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 783 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 7444 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 785 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 7450 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 786 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7456 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 787 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 7462 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 788 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7468 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 789 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7476 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 793 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 798 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7494 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 803 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7501 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 806 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7508 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 809 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7516 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 813 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7524 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 817 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7532 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 821 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7540 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 825 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7548 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 828 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7555 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7561 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7567 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7573 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7579 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7585 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7591 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7597 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 840 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7603 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7609 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 842 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7615 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 843 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7621 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 844 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7627 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 845 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7633 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 927 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7639 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 929 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7645 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 934 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7651 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 935 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7658 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 941 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7664 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 945 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7670 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 946 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7676 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 948 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7682 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 950 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7688 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 955 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7694 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 956 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7701 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 962 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7707 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 966 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7714 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 968 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7721 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 970 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 975 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7734 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 977 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 980 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 982 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 983 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 988 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7767 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 995 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1003 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7783 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1006 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7790 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1012 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 7796 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1013 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval)); }
#line 7802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1018 "hphp.y" /* yacc.c:1646  */
    {
                                         _p->onUsing((yyval), (yyvsp[-2]), true, (yyvsp[-1]), nullptr);
                                       }
#line 7810 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1025 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7816 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1026 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7822 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1031 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 7828 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1032 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval), (yyval), (yyvsp[0])); }
#line 7835 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1037 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7841 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7847 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1039 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7853 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1040 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7859 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7865 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1047 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7871 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1052 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7877 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1053 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7884 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1059 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1062 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7907 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1066 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7914 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1068 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7922 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1071 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1073 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7937 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7943 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1077 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7949 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1078 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7955 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1079 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7961 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1080 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7967 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1081 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7973 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1082 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7979 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7985 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7991 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1085 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7997 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 8003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1087 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 8009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1088 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 8015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1089 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 8021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1090 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 8028 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1094 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 8035 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1096 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 8043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1101 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 8050 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1103 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 8058 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1107 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 8066 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1116 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 8072 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1117 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8078 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1120 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false); }
#line 8084 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1121 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 8090 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1123 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8098 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1127 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8106 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1131 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8114 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1135 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8122 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1139 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8130 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1143 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8138 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1148 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8146 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1151 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 8152 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 8161 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1156 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8167 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1157 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8173 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1158 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8179 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1159 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8185 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1160 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8191 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1161 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8197 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1162 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 8203 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1163 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8209 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1164 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8215 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1165 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 8221 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1166 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8227 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1167 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 8237 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1189 "hphp.y" /* yacc.c:1646  */
    { _p->pushLabelScope(LS::Using);
                                         _p->onNewLabelScope(false);
                                         (yyval) = (yyvsp[-1]); }
#line 8245 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1195 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1; }
#line 8251 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1196 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8257 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1205 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), nullptr, (yyvsp[-2]));
                                         _p->onExprListElem((yyval), &(yyval), (yyvsp[0])); }
#line 8264 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1207 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0])); }
#line 8270 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1217 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8276 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1218 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8282 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1222 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false); }
#line 8288 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1223 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8294 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1232 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 8300 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1233 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8306 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1237 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::Finally);}
#line 8313 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1239 "hphp.y" /* yacc.c:1646  */
    { _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->popLabelScope();
                                         _p->onCompleteLabelScope(false);}
#line 8321 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1245 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8327 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1246 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8333 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1250 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 8339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1251 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1255 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 8351 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1261 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8360 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1268 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8370 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1276 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8379 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1283 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8389 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1291 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8398 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1297 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8408 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1306 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1310 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 8421 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1314 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8428 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1318 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 8434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1324 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8441 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 8459 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1342 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8466 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 8484 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1359 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1362 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1367 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8506 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1370 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8514 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1376 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 8520 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1379 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 8526 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1383 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8533 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1386 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8544 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1394 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1397 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8562 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1405 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8568 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1406 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 8575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1410 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1413 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1416 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 8593 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1417 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 8599 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1418 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 8607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1421 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 8613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1422 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 8619 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1426 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8625 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1427 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8631 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1430 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8637 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1431 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8643 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1434 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8649 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1435 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8655 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1438 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8661 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1440 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8667 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1443 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8673 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1445 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8679 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8685 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1450 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8691 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8697 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8703 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1455 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8709 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8715 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8721 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8727 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1469 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1471 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8745 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1474 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8751 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1476 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8757 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1480 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8763 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1482 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8788 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8794 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1495 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8800 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1497 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8806 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1498 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8812 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1501 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1502 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8824 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1507 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8830 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1508 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8836 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1513 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8842 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1514 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8848 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8854 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1518 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8860 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1521 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8866 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1522 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8872 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1530 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8879 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1536 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8886 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1542 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8894 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1546 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8900 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1550 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8907 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1555 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8914 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1560 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8922 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1563 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8928 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1569 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8936 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1574 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8944 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1579 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8952 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1585 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8960 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1591 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1597 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8976 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1603 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8984 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1609 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8992 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1616 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 9000 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1623 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 9008 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1632 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 9015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1637 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 9022 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1642 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 9030 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1646 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9036 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1649 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 9043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1653 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 9050 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1657 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 9058 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1660 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9064 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1665 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9072 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1669 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9080 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1673 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9088 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1678 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9096 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1683 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9104 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1688 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9112 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1693 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9120 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1698 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9128 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1704 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9136 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1710 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9144 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1716 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 9150 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1717 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 9156 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1718 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 9162 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9168 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1724 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9174 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1727 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,false);}
#line 9181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1729 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::InOut,false);}
#line 9188 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1731 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::Ref,false);}
#line 9195 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1733 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,true);}
#line 9202 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1736 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::In,false);}
#line 9209 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1739 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::In,true);}
#line 9216 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1742 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::Ref,false);}
#line 9223 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1745 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::InOut,false);}
#line 9230 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1750 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9236 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1751 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 9242 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1754 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9248 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1755 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 9254 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1756 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 9260 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1760 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9266 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1762 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9272 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1763 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 9278 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1764 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9284 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1769 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9290 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1770 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9296 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1773 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9303 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1778 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9309 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1784 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9315 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1785 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9321 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1789 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-2]),(yyvsp[-1]),NULL);}
#line 9328 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1793 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-2]));}
#line 9335 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1795 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9342 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1798 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 9349 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1800 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9355 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1803 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9363 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1810 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1818 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9381 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1825 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9391 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1831 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 9397 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1833 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9403 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1835 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9409 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1837 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 9415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1839 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 9421 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1840 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 9428 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1843 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 9434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1847 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9446 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1848 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 9452 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1854 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 9458 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1859 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 9465 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1862 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 9473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1869 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 9479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1870 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 9486 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1875 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 9493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1878 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 9499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1885 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 9506 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1887 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 9512 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1891 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9518 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1896 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9524 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1898 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9530 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1900 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9536 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1901 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 9547 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1907 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 9553 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1909 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 9559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1910 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 9565 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1914 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval),nullptr,(yyvsp[0])); }
#line 9571 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1916 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval),&(yyvsp[-2]),(yyvsp[0])); }
#line 9577 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1921 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9583 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1924 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1925 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 9595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1929 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 9601 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1930 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 9607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1934 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 9614 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1937 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 9621 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1942 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 9628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1947 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 9634 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1948 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 9641 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 9647 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1954 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 9653 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1955 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 9659 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 9665 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 9671 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1961 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9677 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 9683 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 9689 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 9695 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 9701 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1967 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 9707 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1969 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 9713 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9721 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9727 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1977 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1981 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9745 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9751 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1987 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9757 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1990 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9763 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9769 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9775 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1995 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9781 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1998 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9787 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9793 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9799 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9805 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9811 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 2006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9817 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9823 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 2008 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9829 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 2009 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9835 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9841 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9847 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9853 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9859 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 2019 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9865 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 2023 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9871 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 2025 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9877 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 2026 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9883 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 2031 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 2033 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9901 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 2036 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9907 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 2042 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9913 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 2044 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 2048 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9927 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 2052 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9934 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 2056 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9940 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 2060 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9946 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 2064 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9952 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 2066 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9958 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 2067 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9964 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 2068 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9970 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 2069 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9976 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 2070 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9982 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 2073 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9988 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 2074 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9994 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 2078 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10000 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 2079 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10006 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 2083 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 10012 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 2084 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 10018 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 2085 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 10024 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 2086 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10030 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 2090 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10036 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 2095 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 10042 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 2099 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 10048 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 2103 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10054 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 2107 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 10060 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 2111 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10066 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 2116 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 10072 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 2120 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10078 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 2124 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10084 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 2125 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10090 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 2126 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10096 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 2127 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10102 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 2128 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10108 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 2132 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10114 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 2137 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 10120 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 2138 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 10126 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 2139 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 10132 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 2142 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 10138 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 2143 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 10144 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 2144 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 10150 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2145 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 10156 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2146 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 10162 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2147 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 10168 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 10174 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2149 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 10180 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2150 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 10186 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2151 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 10192 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2152 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 10198 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2153 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 10204 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2154 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 10210 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2155 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 10216 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2156 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 10222 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2157 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 10228 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2158 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 10234 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2159 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 10240 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2160 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 10246 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2161 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 10252 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2162 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 10258 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2163 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 10264 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2164 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 10270 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2165 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 10276 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2166 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 10282 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2167 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 10288 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2168 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 10294 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2169 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 10300 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2170 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 10306 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2171 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 10312 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2172 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 10318 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2173 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 10324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2174 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 10330 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2175 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 10336 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2176 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 10342 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2177 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 10348 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2178 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 10354 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2179 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 10360 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2180 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 10366 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2181 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 10372 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2182 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 10378 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2183 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 10384 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2184 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 10390 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2185 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 10396 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2186 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 10403 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2188 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 10409 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2189 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 10416 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2191 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 10422 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2193 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 10428 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2194 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2195 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 10440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2196 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 10446 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2197 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 10452 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2198 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10458 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2199 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 10464 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2200 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 10470 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2201 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 10476 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2202 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 10482 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2203 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 10488 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2204 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 10494 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2205 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 10500 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2206 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 10506 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2207 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 10512 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10518 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2209 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10524 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2210 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10530 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2211 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10536 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2212 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10542 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2213 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10548 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2214 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10554 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2215 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10560 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2216 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10566 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2217 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 10572 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2218 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 10578 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2219 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10584 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2226 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 10590 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2227 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10596 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2232 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10605 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2238 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10617 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2247 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10626 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2253 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10638 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2264 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,NULL,NULL);}
#line 10652 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2273 "hphp.y" /* yacc.c:1646  */
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
#line 10667 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2284 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10677 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2292 "hphp.y" /* yacc.c:1646  */
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
#line 10692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2303 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10702 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2309 "hphp.y" /* yacc.c:1646  */
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
#line 10719 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2321 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,NULL,NULL);}
#line 10733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2330 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2338 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10756 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2346 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10769 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10775 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2358 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10781 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2360 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10787 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2364 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10794 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2366 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10800 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2373 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10806 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2376 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10812 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2383 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2386 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10824 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2391 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10830 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2392 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10836 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2397 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10842 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2398 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10848 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2402 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval), (yyvsp[-1]));}
#line 10854 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2406 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10860 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2407 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10866 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2412 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10872 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2413 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10878 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2418 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10884 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2419 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10890 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2424 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10896 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2425 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10902 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2431 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10908 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2433 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10914 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2438 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10920 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2439 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10926 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2445 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2447 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10938 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2451 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10944 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2455 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10950 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2459 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10956 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2463 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10962 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2467 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2471 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2475 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10980 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2479 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10986 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2483 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10992 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2487 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10998 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2491 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11004 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2495 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11010 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2499 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11016 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2503 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11022 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2507 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11028 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2511 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11034 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2515 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11040 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2519 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11046 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2524 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11052 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2525 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11058 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2530 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11064 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2531 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11070 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2536 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11076 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2537 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11082 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2542 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11090 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2549 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11098 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2556 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11104 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2558 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11110 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2562 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11116 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2563 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11122 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2564 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11128 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2565 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11134 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2566 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11140 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2567 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11146 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2568 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11152 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2569 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11158 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2570 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11164 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2571 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 11171 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11177 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2574 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11183 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2578 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11189 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2579 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 11195 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2580 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 11201 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2581 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 11207 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2588 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 11213 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2591 "hphp.y" /* yacc.c:1646  */
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
#line 11231 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2606 "hphp.y" /* yacc.c:1646  */
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
#line 11249 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2621 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 11255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2622 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 11261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2625 "hphp.y" /* yacc.c:1646  */
    { _p->onXhpAttributesStart(); (yyval).reset();}
#line 11267 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2627 "hphp.y" /* yacc.c:1646  */
    { _p->onXhpAttributeSpread((yyval), &(yyvsp[-4]), (yyvsp[-1]));}
#line 11273 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2630 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11279 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2633 "hphp.y" /* yacc.c:1646  */
    { _p->onOptExprListElem((yyval), &(yyvsp[-1]), (yyvsp[0])); }
#line 11285 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2634 "hphp.y" /* yacc.c:1646  */
    {  (yyval).reset();}
#line 11291 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2637 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11298 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2641 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11306 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2644 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11312 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2647 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 11324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2654 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11330 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2655 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11336 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11342 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2661 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 11348 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 11354 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2667 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11360 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11366 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2669 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11372 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11378 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2671 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11384 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2672 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11390 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2673 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11396 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2674 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11402 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2675 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11408 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2676 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11414 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2677 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11420 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2678 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11426 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2679 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11432 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2680 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11438 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2681 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11444 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2682 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11450 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2683 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11456 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2684 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11462 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2685 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11468 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11474 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2687 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11480 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2688 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11486 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2689 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11492 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2690 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11498 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2691 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11504 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11510 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2693 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11516 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2694 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11522 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2695 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11528 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2696 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11534 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2697 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11540 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2698 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11546 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2699 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11552 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2700 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11558 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2701 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11564 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2702 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11570 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2703 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11576 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2704 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11582 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2705 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11588 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2706 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2707 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11600 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2708 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11606 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2709 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11612 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2710 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11618 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2711 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11624 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11630 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2713 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11636 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2714 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11642 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2715 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11648 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2716 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11654 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2717 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11660 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11666 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11672 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11678 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11684 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2722 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11690 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11696 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2724 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11702 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11708 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2726 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11714 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2727 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11720 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2728 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11726 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11732 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11738 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2731 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11744 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2732 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11750 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2733 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11756 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11762 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2735 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11768 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11774 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2737 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11780 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11786 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11792 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11798 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11804 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2742 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11810 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2743 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11816 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2744 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11822 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2745 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11828 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2746 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11834 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2747 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11840 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2748 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11846 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2749 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11852 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2754 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11858 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2758 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11864 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2759 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11870 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2763 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11876 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2764 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11882 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2765 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11888 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2766 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2768 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11902 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2772 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11908 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2781 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11914 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2784 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11920 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2785 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11927 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2787 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11934 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2797 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11940 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2801 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11946 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2802 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11952 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2803 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11958 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2807 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11964 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2808 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11970 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2809 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11976 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2813 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11982 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2814 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11988 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2815 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11994 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2819 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12000 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2820 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12006 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2824 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12012 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2825 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12018 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2826 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12024 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2827 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 12031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2829 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 12037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2830 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 12043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2831 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 12049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2832 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 12055 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2833 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 12061 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2834 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 12067 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2835 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 12073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2836 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 12079 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2837 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 12085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2840 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2842 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2846 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2847 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2850 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12121 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2852 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1]));}
#line 12127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2853 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12133 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2854 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2855 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12145 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2856 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12151 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2857 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2858 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2859 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2860 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2861 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2863 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 12187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2865 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 12193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 12199 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2869 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 12205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2871 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 12211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2872 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 12217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2873 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 12223 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2874 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 12229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2875 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 12235 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2876 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 12241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2877 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 12247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2878 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 12253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2879 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 12259 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2880 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 12265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2881 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 12271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2882 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 12277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2883 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 12283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2884 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 12289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2885 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 12295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2886 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2887 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12307 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2889 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 12313 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2891 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 12319 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2893 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 12325 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2895 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 12331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2896 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 12337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2898 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 12344 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2900 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 12350 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2903 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 12357 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2907 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 12363 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2910 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 12369 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2911 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 12375 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2915 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12381 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2916 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12387 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2922 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12393 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2928 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12399 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2929 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12405 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2933 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12411 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2934 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12417 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2935 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12423 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2936 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12429 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2937 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 12435 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2938 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 12441 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2940 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 12448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2945 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2946 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2950 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12466 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2951 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12472 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2954 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 12478 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2955 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12484 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2961 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12490 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2963 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12496 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2965 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12502 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2966 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12508 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2970 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12514 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2971 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12520 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2972 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12526 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2975 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12532 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2977 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12538 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2980 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 12544 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2981 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 12550 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2982 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 12556 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2983 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12562 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2987 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 12569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2990 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 12577 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2997 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12583 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2998 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 3001 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 12597 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 3004 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12603 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 3005 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12609 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 3006 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12615 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 3008 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12621 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 3009 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12627 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 3011 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1]));}
#line 12633 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 3012 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12639 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 3013 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12645 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 3014 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12651 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 3015 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12657 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 3016 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12663 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12669 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 3022 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12675 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 3023 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12681 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 3028 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12687 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 3029 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12693 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 3034 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12699 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 3036 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12705 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 3038 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12711 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3039 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12717 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3043 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12723 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3044 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12729 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3049 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3050 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12741 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3055 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12747 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3058 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12753 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3063 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12759 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3064 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12765 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3067 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3068 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 12778 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3075 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 12784 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3077 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 12790 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3080 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 12796 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3082 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3085 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3088 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12814 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3089 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12820 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3093 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12826 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3094 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12832 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3098 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12838 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3099 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12844 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3100 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3104 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12856 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3109 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12862 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3114 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3115 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12874 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3119 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12880 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3124 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12886 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3129 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3130 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12898 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3135 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12904 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3136 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12910 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3138 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12916 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3143 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12922 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3145 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12928 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3151 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12942 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3162 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12956 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3177 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12970 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3189 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12984 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3201 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12990 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3202 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12996 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3203 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13002 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3204 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13008 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3205 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13014 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3206 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13020 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3208 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13034 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3225 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13040 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3227 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13046 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3229 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13052 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3230 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13058 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3234 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13064 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3238 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13070 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3239 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13076 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3240 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13082 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3241 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13088 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3249 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13102 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3258 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13108 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3260 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13114 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3264 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13120 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3269 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13126 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3270 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13132 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3271 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13138 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3272 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13144 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3273 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13150 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3274 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13156 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3275 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13162 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3277 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13168 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3278 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13174 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3281 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13180 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3283 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13186 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3287 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13192 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3291 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13198 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3292 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13204 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3298 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13210 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3302 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13216 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3306 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13222 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3313 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 13228 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3322 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 13234 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3326 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 13240 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3330 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13246 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3339 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13252 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3340 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13258 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3341 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13264 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3345 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13270 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3346 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 13276 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3347 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 13282 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3349 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]) = 1; _p->onIndirectRef((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 13288 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3354 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13294 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3355 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13300 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3366 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13306 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3367 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13312 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3368 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13318 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3371 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3382 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13338 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3383 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13344 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3387 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13350 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3388 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13356 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3391 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13370 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3400 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13376 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3404 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 13382 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3405 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 13388 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3407 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 13394 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3408 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 13400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3409 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 13406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3410 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 13412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3415 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3416 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3420 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3421 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3422 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3423 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3426 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3428 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 13460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3429 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13466 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3430 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 13472 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3435 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13478 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3436 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13484 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3440 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13490 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3441 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13496 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3442 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13502 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3443 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13508 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3448 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13514 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3449 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13520 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3454 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13526 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3456 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13532 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3458 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13538 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3459 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13544 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3463 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 13550 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1030:
#line 3465 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 13556 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1031:
#line 3466 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 13562 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1032:
#line 3468 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 13569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1033:
#line 3473 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1034:
#line 3475 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1035:
#line 3477 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 13595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1036:
#line 3487 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 13601 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3489 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 13607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1038:
#line 3490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1039:
#line 3493 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 13619 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3494 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 13625 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3495 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 13631 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3499 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 13637 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3500 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 13643 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3501 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13649 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3502 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13655 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3503 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13661 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3504 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13667 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3505 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 13673 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3506 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 13679 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3507 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 13685 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3508 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 13691 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3509 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 13697 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3513 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 13703 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3514 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 13709 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3519 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13715 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3521 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13721 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3535 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13729 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3540 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 13737 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3544 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13745 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3549 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 13753 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3555 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13759 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3556 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13765 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3560 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3561 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13777 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3567 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13783 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3571 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 13789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3577 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3581 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 13802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3588 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3589 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13814 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3593 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 13822 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3596 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 13829 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3602 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13835 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3606 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13843 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1077:
#line 3609 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13851 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1078:
#line 3612 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-3]); }
#line 13858 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1079:
#line 3614 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13865 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1080:
#line 3616 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13872 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1081:
#line 3618 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13878 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1082:
#line 3623 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]); }
#line 13884 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1083:
#line 3624 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13890 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1084:
#line 3625 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13896 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1085:
#line 3626 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13902 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1092:
#line 3647 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13908 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1093:
#line 3648 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13914 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1096:
#line 3657 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13920 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1099:
#line 3668 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13926 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1100:
#line 3670 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1101:
#line 3674 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13938 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1102:
#line 3677 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13944 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1103:
#line 3681 "hphp.y" /* yacc.c:1646  */
    {}
#line 13950 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1104:
#line 3682 "hphp.y" /* yacc.c:1646  */
    {}
#line 13956 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1105:
#line 3683 "hphp.y" /* yacc.c:1646  */
    {}
#line 13962 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1106:
#line 3689 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13969 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1107:
#line 3694 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13979 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1108:
#line 3703 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13985 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1109:
#line 3709 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13994 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1110:
#line 3717 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 14000 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1111:
#line 3718 "hphp.y" /* yacc.c:1646  */
    { }
#line 14006 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1112:
#line 3724 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 14012 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1113:
#line 3726 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 14018 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1114:
#line 3727 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 14028 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1115:
#line 3732 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 14035 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1116:
#line 3738 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("HH\\darray"); }
#line 14042 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1117:
#line 3743 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14048 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1118:
#line 3748 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 14056 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1119:
#line 3752 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14062 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1120:
#line 3757 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 14068 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1121:
#line 3759 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 14074 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1122:
#line 3765 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 14081 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1123:
#line 3767 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 14089 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1124:
#line 3770 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14095 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1125:
#line 3771 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1126:
#line 3774 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14111 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1127:
#line 3777 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1128:
#line 3780 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 14125 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1129:
#line 3783 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14132 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1130:
#line 3785 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 14141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1131:
#line 3791 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 14150 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1132:
#line 3797 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("HH\\varray");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 14160 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1133:
#line 3805 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14166 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1134:
#line 3806 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 14172 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;


#line 14176 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 3809 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}
