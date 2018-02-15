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
#define YYLAST   20210

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  209
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  315
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1131
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  2110

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
    3094,  3095,  3099,  3104,  3109,  3110,  3114,  3119,  3124,  3125,
    3129,  3131,  3132,  3137,  3139,  3144,  3155,  3169,  3181,  3196,
    3197,  3198,  3199,  3200,  3201,  3202,  3212,  3221,  3223,  3225,
    3229,  3233,  3234,  3235,  3236,  3237,  3253,  3254,  3257,  3264,
    3265,  3266,  3267,  3268,  3269,  3270,  3271,  3273,  3278,  3282,
    3283,  3287,  3290,  3294,  3301,  3305,  3314,  3321,  3329,  3331,
    3332,  3336,  3337,  3338,  3340,  3345,  3346,  3357,  3358,  3359,
    3360,  3371,  3374,  3377,  3378,  3379,  3380,  3391,  3395,  3396,
    3397,  3399,  3400,  3401,  3405,  3407,  3410,  3412,  3413,  3414,
    3415,  3418,  3420,  3421,  3425,  3427,  3430,  3432,  3433,  3434,
    3438,  3440,  3443,  3446,  3448,  3450,  3454,  3455,  3457,  3458,
    3464,  3465,  3467,  3477,  3479,  3481,  3484,  3485,  3486,  3490,
    3491,  3492,  3493,  3494,  3495,  3496,  3497,  3498,  3499,  3500,
    3504,  3505,  3509,  3511,  3519,  3521,  3525,  3529,  3534,  3538,
    3546,  3547,  3551,  3552,  3558,  3559,  3568,  3569,  3577,  3580,
    3584,  3587,  3592,  3597,  3600,  3603,  3605,  3607,  3609,  3613,
    3615,  3616,  3617,  3620,  3622,  3628,  3629,  3633,  3634,  3638,
    3639,  3643,  3644,  3647,  3652,  3653,  3657,  3660,  3662,  3666,
    3672,  3673,  3674,  3678,  3682,  3690,  3695,  3707,  3709,  3713,
    3716,  3718,  3723,  3728,  3734,  3737,  3742,  3747,  3749,  3756,
    3758,  3761,  3762,  3765,  3768,  3769,  3774,  3776,  3780,  3786,
    3796,  3797
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

#define YYPACT_NINF -1800

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1800)))

#define YYTABLE_NINF -1132

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1132)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1800,   213, -1800, -1800,  5442, 14758, 14758,    33, 14758, 14758,
   14758, 14758, 12359, 14758, -1800, 14758, 14758, 14758, 14758, 18257,
   18257, 14758, 14758, 14758, 14758, 14758, 14758, 14758, 14758, 12538,
   19055, 14758,    48,   198, -1800, -1800, -1800,   173, -1800,   259,
   -1800, -1800, -1800,   344, 14758, -1800,   198,   253,   258,   386,
   -1800,   198, 12717, 16217, 12896, -1800, 15776, 11222,   408, 14758,
   19304,   304,    86,   573,   284, -1800, -1800, -1800,   389,   428,
     435,   451, -1800, 16217,   453,   463,   602,   605,   608,   613,
     629, -1800, -1800, -1800, -1800, -1800, 14758,   541,  1998, -1800,
   -1800, 16217, -1800, -1800, -1800, -1800, 16217, -1800, 16217, -1800,
     539,   512,   528, 16217, 16217, -1800,   384, -1800, -1800, 13102,
   -1800, -1800,   372,   509,   582,   582, -1800,   706,   584,   575,
     555, -1800,   103, -1800,   557,   645,   733, -1800, -1800, -1800,
   -1800,  4617,    82, -1800,   156, -1800,   585,   600,   618,   622,
     636,   638,   640,   653, 17092, -1800, -1800, -1800, -1800, -1800,
     132,   736,   739,   798,   806,   813,   817, -1800,   846,   859,
   -1800,   220,   580, -1800,   641,    22, -1800,  2674,   203, -1800,
   -1800,   810,   156,   156,   681,   175, -1800,   158,   364,   714,
     365, -1800, -1800,   869, -1800,   778, -1800, -1800,   743,   783,
   -1800, 14758, -1800,   733,    82, 19592,  4271, 19592, 14758, 19592,
   19592, 19859, 19859,   753, 18430, 19592,   895, 16217,   889,   889,
     143,   889, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800,    74, 14758,   776, -1800, -1800,   800,   761,   581,   765,
     581,   889,   889,   889,   889,   889,   889,   889,   889, 18257,
   18478,   763,   956,   778, -1800, 14758,   776, -1800,   807, -1800,
     808,   769, -1800,   167, -1800, -1800, -1800,   581,   156, -1800,
   13281, -1800, -1800, 14758,  9789,   965,   113, 19592, 10819, -1800,
   14758, 14758, 16217, -1800, -1800, 17140,   773, -1800, 17213, -1800,
   -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,  4911,
   -1800,  4911, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800,   125,   104,   783, -1800, -1800, -1800, -1800,   779, -1800,
   17445,   127, -1800, -1800,   815,   969, -1800,   828, 16517, 14758,
   -1800,   792,   796, 17261, -1800,    80, 17309, 15158, 15158, 15158,
   16217, 15158,   777,   989,   801, -1800,    68, -1800, 17924,   119,
   -1800,   985,   121,   872, -1800,   875, -1800, 18257, 14758, 14758,
     809,   818, -1800, -1800, 18000, 12538, 14758, 14758, 14758, 14758,
   14758,   128,   115,   532, -1800, 14937, 18257,   548, -1800, 16217,
   -1800,   519,   584, -1800, -1800, -1800, -1800, 19157, 14758,   998,
     912, -1800, -1800, -1800,    81, 14758,   820,   823, 19592,   824,
    1327,   826,  5190, 14758, -1800,   588,   811,   632,   588,   529,
     506, -1800, 16217,  4911,   829, 11401, 15776, -1800, 13487,   825,
     825,   825,   825, -1800, -1800,  4248, -1800, -1800, -1800, -1800,
   -1800,   733, -1800, 14758, 14758, -1800, -1800, -1800, -1800, -1800,
   -1800, -1800, -1800, -1800, 14758, 14758, 14758, 14758, 13666, 14758,
   14758, 14758, 14758, 14758, 14758, 14758, 14758, 14758, 14758, 14758,
   14758, 14758, 14758, 14758, 14758, 14758, 14758, 14758, 14758, 14758,
   14758, 14758, 19233, 14758, -1800, 14758, 14758, 14758, 15110, 16217,
   16217, 16217, 16217, 16217,  4617,   917,   930, 11025, 14758, 14758,
   14758, 14758, 14758, 14758, 14758, 14758, 14758, 14758, 14758, 14758,
   -1800, -1800, -1800, -1800,  2232, -1800, -1800, 11401, 11401, 14758,
   14758, 18000,   831,   733, 13845, 17358, -1800, 14758, -1800,   835,
    1027,   878,   842,   847, 15264,   581, 14024, -1800, 14203, -1800,
     769,   848,   849,  1976, -1800,   347, 11401, -1800,  3945, -1800,
   -1800, 17406, -1800, -1800, 11777, -1800, 14758, -1800,   948,  9995,
    1057,   866, 19470,  1055,   107,    79, -1800, -1800, -1800,   898,
   -1800, -1800, -1800,  4911, -1800,   716,   879,  1074, 17772, 16217,
   -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,   893,
   -1800, -1800,   894,   896,   900,   904,   905,   907,   120,   908,
     910, 16027, 15848, -1800, -1800, 16217, 16217, 14758,   581,   304,
   -1800, 17772,  1023, -1800, -1800, -1800,   581,   110,   142,   909,
     915,  2271,   189,   916,   922,   169,   992,   928,   581,   145,
     932, 18539,   924,  1127,  1128,   935,   936,   939,   942, -1800,
   15669, 16217, -1800, -1800,  1078,  3051,    51, -1800, -1800, -1800,
     584, -1800, -1800, -1800,  1118,  1016,   970,   199,   993, 14758,
    1020,  1145,   959, -1800,  1000, -1800,   245, -1800,   964,  4911,
    4911,  1155,   965,    81, -1800,   974,  1162, -1800, 17493,   239,
   -1800,   300,   222, -1800, -1800, -1800, -1800, -1800, -1800, -1800,
    1296,  3227, -1800, -1800, -1800, -1800,  1164,   995, -1800, 18257,
     649, 14758,   977,  1171, 19592,  1168,   148,  1181,   991,  1002,
    1003, 19592,  1004,  2725,  6081, -1800, -1800, -1800, -1800, -1800,
   -1800,  1058,  4727, 19592,   994,  3298, 19731, 19822, 19859, 19896,
   14758, 19544, 19969, 15111, 20074, 20141, 15777, 16325, 19240, 19240,
   19240, 19240,  4007,  4007,  4007,  4007,  4007,  1022,  1022,   843,
     843,   843,   143,   143,   143, -1800,   889,   996,  1001, 18587,
    1005,  1197,    42, 14758,   240,   776,   363, -1800, -1800, -1800,
    1195,   912, -1800,   733, 18105, -1800, -1800, -1800, 19859, 19859,
   19859, 19859, 19859, 19859, 19859, 19859, 19859, 19859, 19859, 19859,
   19859, -1800, 14758,   255, -1800,   326, -1800,   776,   392,  1012,
    1013,  1011,  3402,   149,  1017, -1800, 19592, 17848, -1800, 16217,
   -1800,   581,    73, 18257, 19592, 18257, 18648,  1058,   308,   581,
     355, -1800,   245,  1059,  1019, 14758, -1800,   356, -1800, -1800,
   -1800,  6287,   700, -1800, -1800, 19592, 19592,   198, -1800, -1800,
   -1800, 14758,  1114, 17696, 17772, 16217, 10201,  1021,  1026, -1800,
    1211,  3616,  1085, -1800,  1062, -1800,  1219,  1032,  4338,  4911,
   17772, 17772, 17772, 17772, 17772,  1036,  1170,  1172,  1173,  1177,
    1178,  1050,  1051, 17772,    28, -1800, -1800, -1800, -1800, -1800,
   -1800, -1800,    54, -1800, 19686, -1800, -1800,    56, -1800,  6493,
   15490,  1052, 15848, -1800, 15848, -1800, 15848, -1800, 16217, 16217,
   15848, -1800, 15848, 15848, 16217, -1800,  1244,  1053, -1800,   441,
   -1800, -1800,  3516, -1800, 19686,  1245, 18257,  1060, -1800, -1800,
   -1800, -1800, -1800, -1800, -1800, -1800, -1800,  1076,  1249, 16217,
   15490,  1061, 18000, 18181,  1251, -1800, 14758, -1800, 14758, -1800,
   14758, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,  1066,
   -1800, 14758, -1800, -1800,  5669, -1800,  4911, 15490,  1069, -1800,
   -1800, -1800, -1800,  1097,  1261,  1080, 14758, 19157, -1800, -1800,
   15110, -1800,  1075, -1800,  4911, -1800,  1089,  6699,  1254,    84,
   -1800,  4911, -1800,   106,  2232,  2232, -1800,  4911, -1800, -1800,
     581, -1800, -1800,  1217, 19592, -1800, 11580, -1800, 17772, 13487,
     825, 13487, -1800,   825,   825, -1800, 11983, -1800, -1800,  6905,
   -1800,    61,  1092, 15490,  1016, -1800, -1800, -1800, -1800, 19969,
   14758, -1800, -1800, 14758, -1800, 14758, -1800,  3963,  1095, 11401,
     992,  1256,  1016,  4911,  1281,  1058, 16217, 19233,   581,  4141,
    1102,   249,  1105, -1800, -1800,  1292,  1678,  1678, 17848, -1800,
   -1800, -1800,  1262,  1116,  1243,  1246,  1247,  1250,  1252,    76,
    1125,  1130,   563, -1800, -1800, -1800, -1800, -1800, -1800,  1169,
   -1800, -1800, -1800, -1800,  1321,  1133,   835,   581,   581, 14382,
    1016,  3945, -1800,  3945, -1800,  4211,   704,   198, 10819, -1800,
    7111,  1135,  7317,  1136, 17696, 18257,  1141,  1196,   581, 19686,
    1340, -1800, -1800, -1800, -1800,   688, -1800,   353,  4911,  1174,
    1218,  1193,  4911, 16217,  2394, -1800, -1800,  4911,  1350,  1160,
    1186,  1187,  1164,   856,   856,  1295,  1295, 18805,  1161,  1357,
   17772, 17772, 17772, 17772, 17772, 17772, 19157, 17772,  2631, 16671,
   17772, 17772, 17772, 17772, 17620, 17772, 17772, 17772, 17772, 17772,
   17772, 17772, 17772, 17772, 17772, 17772, 17772, 17772, 17772, 17772,
   17772, 17772, 17772, 17772, 17772, 17772, 17772, 17772, 16217, -1800,
   -1800,  1284, -1800, -1800,  1166,  1167,  1175, -1800,  1179, -1800,
   -1800,   442, 16027, -1800,  1180, -1800, 17772,   581, -1800, -1800,
     122, -1800,   740,  1362, -1800, -1800,   150,  1184,   581, 12180,
   19592, 18696, -1800,  2861, -1800,  5875,   912,  1362, -1800,   415,
   14758,    29, -1800, 19592,  1234,  1189, -1800,  1188,  1254, -1800,
   -1800, -1800, 14579,  4911,   965, 17541,  1290,    87,  1360,  1297,
     362, -1800,   776,   376, -1800,   776, -1800, 14758, 18257,   649,
   14758, 19592, 19686, -1800, -1800, -1800,  4524, -1800, -1800, -1800,
   -1800, -1800, -1800,  1191,    61, -1800,  1190,    61,  1194, 19969,
   19592, 18757,  1198, 11401,  1200,  1201,  4911,  1202,  1204,  4911,
    1016, -1800,   769,   399, 11401, 14758, -1800, -1800, -1800, -1800,
   -1800, -1800,  1258,  1205,  1386,  1310, 17848, 17848, 17848, 17848,
   17848, 17848,  1255, -1800, 19157, 17848,   117, 17848, -1800, -1800,
   -1800, 18257, 19592,  1206, -1800,   198,  1374,  1336, 10819, -1800,
   -1800, -1800,  1222, 14758,  1196,   581, 18000, 17696,  1224, 17772,
    7523,   759,  1225, 14758,    88,   423, -1800,  1241, -1800,  4911,
   16217, -1800,  1293, -1800, -1800, -1800, 17397, -1800,  1400, -1800,
    1233, 17772, -1800, 17772, -1800,  1235,  1230,  1427, 18865,  1237,
   19686,  1429,  1239,  1242,  1248,  1301,  1436,  1253,  1266, -1800,
   -1800, -1800, 18911,  1259,  1439, 19778, 16513, 19932, 17772, 19640,
   20040, 20108, 17704, 15956, 18008, 18188, 18188, 18188, 18188,  3104,
    3104,  3104,  3104,  3104,  1257,  1257,   856,   856,   856,  1295,
    1295,  1295,  1295, -1800,  1267, -1800,  1260,  1269,  1275,  1276,
   -1800, -1800, 19686, 16217,  4911,  4911, -1800,   740, 15490,   174,
   -1800, 18000, -1800, -1800, 19859, 14758,  1279, -1800,  1282,  1392,
   -1800,   160, 14758, -1800, -1800,  4921, -1800, 14758, -1800, 14758,
   -1800,   965, 13487,  1277,   379,   825,   379,   309, -1800, -1800,
    4911,   161, -1800,  1442,  1367, 14758, -1800,  1287,  1289,  1285,
     581,  1217, 19592,  1254,  1283, -1800,  1291,    61, 14758, 11401,
    1294, -1800, -1800,   912, -1800, -1800,  1300,  1302,  1298, -1800,
    1306, 17848, -1800, 17848, -1800, -1800,  1307,  1288,  1449,  1355,
    1304, -1800,  1486,  1305,  1308,  1316, -1800,  1358,  1323,  1493,
    1329, -1800, -1800,   581, -1800,  1498, -1800,  1330, -1800, -1800,
    1333,  1334,   151, -1800, -1800, 19686,  1337,  1338, -1800, 17044,
   -1800, -1800, -1800, -1800, -1800, -1800,  1403,  4911,  4911,  1186,
    1366,  4911, -1800, 19686, 18971, -1800, -1800, 17772, -1800, 17772,
   -1800, 17772, -1800, -1800, -1800, -1800, 17772, 19157, -1800, -1800,
   -1800, 17772, -1800, 17772, -1800, 20005, 17772,  1341,  7729, -1800,
   -1800, -1800, -1800,   740, -1800, -1800, -1800, -1800,   712, 15955,
   15490,  1432, -1800,  2011,  1377,  5515, -1800, -1800,  1466,   917,
    4717,   129,   131,  1349,   912,   930,   155, 19592, -1800, -1800,
   -1800,  1384,  5133, -1800, 16996, 19592, -1800,  3612, -1800,  6081,
    1470,    92,  1543,  1475, 14758, -1800, 19592, 11401, 11401, -1800,
    1441,  1254,  1837,  1254,  1363, 19592,  1365, -1800,  2156,  1364,
    2270, -1800, -1800,    61, -1800, -1800,  1433, -1800, -1800, 17848,
   -1800, 17848, -1800, 17848, -1800, -1800, -1800, -1800, 17848, -1800,
   19157, -1800, -1800,  2378, -1800,  7935, -1800, -1800, -1800, -1800,
   10407, -1800, -1800, -1800,  6287,  4911, -1800, -1800, -1800,  1370,
   17772, 19017, 19686, 19686, 19686,  1434, 19686, 19077, 20005, -1800,
   -1800,   740, 15490, 15490, 16217, -1800,  1553, 16825,   101, -1800,
   15955,   912,  3720, -1800,  1390, -1800,   133,  1372,   134, -1800,
   16324, -1800, -1800, -1800,   138, -1800, -1800,  3543, -1800,  1375,
   -1800,  1561,   139,   733,  1466, 16145, -1800, 16145, -1800, -1800,
    1563,   917, -1800, 15418, -1800, -1800, -1800, -1800,  3003, -1800,
    1564,  1497, 14758, -1800, 19592,  1381,  1382,  1385,  1254,  1387,
   -1800,  1441,  1254, -1800, -1800, -1800, -1800,  2406,  1383, 17848,
    1450, -1800, -1800, -1800,  1452, -1800,  6287, 10613, 10407, -1800,
   -1800, -1800,  6287, -1800, -1800, 19686, 17772, 17772, 17772,  8141,
    1391,  1393, -1800, 17772, -1800, 15490, -1800, -1800, -1800, -1800,
   -1800,  4911,   770,  2011, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,   536, -1800,
    1377, -1800, -1800, -1800, -1800, -1800,   116,   606, -1800, 17772,
    1504, -1800, 16517,   140,  1577, -1800,  4911,   733, -1800, -1800,
    1395,  1579, 14758, -1800, 19592, -1800, -1800,   153,  1396,  4911,
     650,  1254,  1387, 15597, -1800,  1254, -1800, 17848, 17848, -1800,
   -1800, -1800, -1800,  8347, 19686, 19686, 19686, -1800, -1800, -1800,
   19686, -1800,  2648,  1588,  1590,  1397, -1800, -1800, 17772, 16324,
   16324,  1533, -1800,  3543,  3543,   709, -1800, -1800, -1800, 19686,
    1591,  1424,  1409, -1800, 17772, -1800, 16517, -1800, 17772, 19592,
    1528, -1800,  1605, -1800,  1606, -1800,   526, -1800, -1800, -1800,
    1417,   650, -1800,   650, -1800, -1800,  8553,  1419,  1505, -1800,
    1519,  1461, -1800, -1800,  1522,  4911,  1443,   770, -1800, -1800,
   19686, -1800, -1800,  1454, -1800,  1595, -1800, -1800, -1800, -1800,
   17772,   169, -1800, 19686,  1435, 19686, -1800,   163,  1426,  8759,
    4911, -1800,  4911, -1800,  8965, -1800, -1800, -1800,  1431, -1800,
    1438,  1453, 16217,   930,  1451, -1800, -1800, -1800, 19686,  1457,
      83, -1800,  1555, -1800, -1800, -1800, -1800, -1800, -1800,  9171,
   -1800, 15490,  1052, -1800,  1465, 16217,   644, -1800, -1800,  1444,
    1634,   665,    83, -1800, -1800,  1566, -1800, 15490,  1446, -1800,
    1254,   118, -1800,  4911, -1800, -1800, -1800,  4911, -1800,  1455,
    1463,   146, -1800,  1387,   674,  1567,   186,  1254,  1459, -1800,
     722,  4911,  4911, -1800,   109,  1638,  1574,  1387, -1800, -1800,
   -1800, -1800,  1581,   191,  1651,  1585, 14758, -1800,   722,  9377,
    9583, -1800,   112,  1655,  1587, 14758, -1800, 19592, -1800, -1800,
   -1800,  1658,  1592, 14758, -1800, 19592, 14758, -1800, 19592, 19592
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   204,   471,     0,   912,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1005,
     993,     0,   776,     0,   782,   783,   784,    29,   849,   981,
     982,   171,   172,   785,     0,   152,     0,     0,     0,     0,
      30,     0,     0,     0,     0,   223,     0,     0,     0,     0,
       0,     0,   440,   441,   442,   439,   438,   437,     0,     0,
       0,     0,   252,     0,     0,     0,    37,    38,    40,    41,
      39,   789,   791,   792,   786,   787,     0,     0,     0,   793,
     788,     0,   759,    32,    33,    34,    36,    35,     0,   790,
       0,     0,     0,     0,     0,   794,   443,   581,    31,     0,
     170,   140,     0,   777,     0,     0,     4,   126,   128,   848,
       0,   758,     0,     6,     0,     0,   222,     7,     9,     8,
      10,     0,     0,   435,     0,   485,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   541,   483,   969,   970,   563,
     556,   557,   558,   559,   562,   560,   561,   466,   566,     0,
     465,   940,   760,   767,     0,   851,   555,   434,   943,   944,
     956,   484,     0,     0,     0,   487,   486,   941,   942,   939,
     977,   980,   545,   850,    11,   440,   441,   442,     0,     0,
      36,     0,   126,   222,     0,  1045,   484,  1046,     0,  1048,
    1049,   565,   479,     0,   472,   477,     0,     0,   527,   528,
     529,   530,    29,   981,   785,   762,    37,    38,    40,    41,
      39,     0,     0,  1069,   962,   760,     0,   761,   506,     0,
     508,   546,   547,   548,   549,   550,   551,   552,   554,     0,
    1009,     0,   858,   772,   242,     0,  1069,   463,   771,   765,
       0,   781,   761,   988,   989,   995,   987,   773,     0,   464,
       0,   775,   553,     0,   205,     0,     0,   468,   205,   150,
     470,     0,     0,   156,   158,     0,     0,   160,     0,    75,
      76,    82,    83,    67,    68,    59,    80,    91,    92,     0,
      62,     0,    66,    74,    72,    94,    86,    85,    57,   108,
      81,   101,   102,    58,    97,    55,    98,    56,    99,    54,
     103,    90,    95,   100,    87,    88,    61,    89,    93,    53,
      84,    69,   104,    77,   106,    70,    60,    47,    48,    49,
      50,    51,    52,    71,   107,   105,   110,    64,    45,    46,
      73,  1122,  1123,    65,  1127,    44,    63,    96,     0,    79,
       0,   126,   109,  1060,  1121,     0,  1124,     0,     0,     0,
     162,     0,     0,     0,   213,     0,     0,     0,     0,     0,
       0,     0,     0,   860,     0,   114,   116,   350,     0,     0,
     349,   355,     0,     0,   253,     0,   256,     0,     0,     0,
       0,  1066,   238,   250,  1001,  1005,   600,   630,   630,   600,
     630,     0,  1030,     0,   796,     0,     0,     0,  1028,     0,
      16,     0,   130,   230,   244,   251,   660,   593,   630,     0,
    1054,   573,   575,   577,   916,   471,   485,     0,     0,   483,
     484,   486,   205,     0,   984,   778,     0,   779,     0,     0,
       0,   202,     0,     0,   132,   339,     0,    28,     0,     0,
       0,     0,     0,   203,   221,     0,   249,   234,   248,   440,
     443,   222,   436,   986,     0,   932,   192,   193,   194,   195,
     196,   198,   199,   201,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   993,     0,   191,   986,   986,  1015,     0,     0,
       0,     0,     0,     0,     0,     0,   433,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     505,   507,   917,   918,     0,   931,   930,   339,   339,   986,
       0,  1001,     0,   222,     0,     0,   164,     0,   914,   909,
     858,     0,   485,   483,     0,  1013,     0,   598,   857,  1004,
     781,   485,   483,   484,   132,     0,   339,   462,     0,   933,
     774,     0,   140,   292,     0,   580,     0,   167,     0,   205,
     469,     0,     0,     0,     0,     0,   159,   190,   161,  1122,
    1123,  1119,  1120,     0,  1126,  1112,     0,     0,     0,     0,
      78,    43,    65,    42,  1061,   197,   200,   163,   140,     0,
     180,   189,     0,     0,     0,     0,     0,     0,   117,     0,
       0,     0,   859,   115,    18,     0,   111,     0,   351,     0,
     165,     0,     0,   166,   254,   255,  1050,     0,     0,   485,
     483,   484,   487,   486,     0,  1102,   262,     0,  1002,     0,
       0,     0,     0,   858,   858,     0,     0,     0,     0,   168,
       0,     0,   795,  1029,   849,     0,     0,  1027,   854,  1026,
     129,     5,    13,    14,     0,   260,     0,     0,   586,     0,
       0,   858,     0,   769,     0,   768,   763,   587,     0,     0,
       0,     0,     0,   916,   136,     0,   860,   915,  1131,   461,
     474,   488,   949,   968,   147,   139,   143,   144,   145,   146,
     434,     0,   564,   852,   853,   127,   858,     0,  1070,     0,
       0,     0,     0,   860,   340,     0,     0,     0,   485,   209,
     210,   208,   483,   484,   205,   184,   182,   183,   185,   569,
     224,   258,     0,   985,     0,     0,   511,   513,   512,   524,
       0,     0,   544,   509,   510,   514,   516,   515,   533,   534,
     531,   532,   535,   536,   537,   538,   539,   525,   526,   518,
     519,   517,   520,   521,   523,   540,   522,     0,     0,  1019,
       0,   858,  1053,     0,  1052,  1069,   946,   240,   232,   246,
       0,  1054,   236,   222,     0,   475,   478,   480,   490,   493,
     494,   495,   496,   497,   498,   499,   500,   501,   502,   503,
     504,   920,     0,   919,   922,   945,   926,  1069,   923,     0,
       0,     0,     0,     0,     0,  1047,   473,   907,   911,   857,
     913,   460,   764,     0,  1008,     0,  1007,   258,     0,   764,
     992,   991,   977,   980,     0,     0,   919,   922,   990,   923,
     482,   294,   296,   136,   584,   583,   467,     0,   140,   276,
     151,   470,     0,     0,     0,     0,   205,   288,   288,   157,
     858,     0,     0,  1111,     0,  1108,   858,     0,  1082,     0,
       0,     0,     0,     0,   856,     0,    37,    38,    40,    41,
      39,     0,     0,     0,   798,   802,   803,   804,   807,   805,
     806,   809,     0,   797,   134,   847,   808,  1069,  1125,   205,
       0,     0,     0,    21,     0,    22,     0,    19,     0,   112,
       0,    20,     0,     0,     0,   123,   860,     0,   121,   116,
     113,   118,     0,   348,   356,   353,     0,     0,  1039,  1044,
    1041,  1040,  1043,  1042,    12,  1100,  1101,     0,   858,     0,
       0,     0,  1001,   998,     0,   597,     0,   611,   857,   599,
     857,   629,   614,   623,   626,   617,  1038,  1037,  1036,     0,
    1032,     0,  1033,  1035,   205,     5,     0,     0,     0,   655,
     656,   665,   664,     0,     0,   483,     0,   857,   592,   596,
       0,   620,     0,  1055,     0,   574,     0,   205,  1089,   916,
     320,  1131,  1130,     0,     0,     0,   983,   857,  1072,  1068,
     342,   336,   337,   341,   343,   757,   859,   338,     0,     0,
       0,     0,   460,     0,     0,   488,     0,   950,   212,   205,
     142,   916,     0,     0,   260,   571,   226,   928,   929,   543,
       0,   637,   638,     0,   635,   857,  1014,     0,     0,   339,
     262,     0,   260,     0,     0,   258,     0,   993,   491,     0,
       0,   947,   948,   978,   979,     0,     0,     0,   895,   865,
     866,   867,   874,     0,    37,    38,    40,    41,    39,     0,
       0,     0,   880,   886,   887,   888,   891,   889,   890,     0,
     878,   876,   877,   901,   858,     0,   909,  1012,  1011,     0,
     260,     0,   934,     0,   780,     0,   298,     0,   205,   148,
     205,     0,   205,     0,     0,     0,     0,   268,   269,   280,
       0,   140,   278,   177,   288,     0,   288,     0,   857,     0,
       0,     0,     0,     0,   857,  1110,  1113,  1078,   858,     0,
    1073,     0,   858,   830,   831,   828,   829,   864,     0,   858,
     856,   604,   632,   632,   604,   632,   595,   632,     0,     0,
    1021,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1116,
     214,     0,   217,   181,     0,     0,     0,   119,     0,   124,
     125,   117,   859,   122,     0,   352,     0,  1051,   169,  1067,
    1102,  1093,  1097,   261,   263,   362,     0,     0,   999,     0,
     602,     0,  1031,     0,    17,   205,  1054,   259,   362,     0,
       0,     0,   764,   589,     0,   770,  1056,     0,  1089,   578,
     135,   137,     0,     0,     0,  1131,     0,     0,   325,   323,
     922,   935,  1069,   922,   936,  1069,  1071,   986,     0,     0,
       0,   344,   133,   207,   209,   210,   484,   188,   206,   186,
     187,   211,   141,     0,   916,   257,     0,   916,     0,   542,
    1018,  1017,     0,   339,     0,     0,     0,     0,     0,     0,
     260,   228,   781,   921,   339,     0,   870,   871,   872,   873,
     881,   882,   899,     0,   858,     0,   895,   608,   634,   634,
     608,   634,     0,   869,   903,   634,     0,   857,   906,   908,
     910,     0,  1006,     0,   921,     0,     0,     0,   205,   295,
     585,   153,     0,   470,   268,   270,  1001,     0,     0,     0,
     205,     0,     0,     0,     0,     0,   282,     0,  1117,     0,
       0,  1103,     0,  1109,  1107,  1074,   857,  1080,     0,  1081,
       0,     0,   800,   857,   855,     0,     0,   858,     0,     0,
     844,   858,     0,     0,     0,     0,   858,     0,     0,   810,
     845,   846,  1025,     0,   858,   813,   815,   814,     0,     0,
     811,   812,   816,   818,   817,   834,   835,   832,   833,   836,
     837,   838,   839,   840,   825,   826,   820,   821,   819,   822,
     823,   824,   827,  1115,     0,   140,     0,     0,     0,     0,
     120,    23,   354,     0,     0,     0,  1094,  1099,     0,   434,
    1003,  1001,   476,   481,   489,     0,     0,    15,     0,   434,
     668,     0,     0,   670,   663,     0,   666,     0,   662,     0,
    1058,     0,     0,     0,     0,   541,     0,   487,  1090,   582,
    1131,     0,   326,   327,     0,     0,   321,     0,     0,     0,
     346,   347,   345,  1089,     0,   362,     0,   916,     0,   339,
       0,   975,   362,  1054,   362,  1057,     0,     0,     0,   492,
       0,     0,   884,   857,   894,   875,     0,     0,   858,     0,
       0,   893,   858,     0,     0,     0,   868,     0,     0,   858,
       0,   879,   900,  1010,   362,     0,   140,     0,   291,   277,
       0,     0,     0,   267,   173,   281,     0,     0,   284,     0,
     289,   290,   140,   283,  1118,  1104,     0,     0,  1077,  1076,
       0,     0,  1129,   863,   862,   799,   612,   857,   603,     0,
     615,   857,   631,   624,   627,   618,     0,   857,   594,   801,
     621,     0,   636,   857,  1020,   842,     0,     0,   205,    24,
      25,    26,    27,  1096,  1091,  1092,  1095,   264,     0,     0,
       0,   441,   432,     0,     0,     0,   239,   361,     0,     0,
     431,     0,     0,     0,  1054,   434,     0,   601,  1034,   358,
     245,   658,     0,   661,     0,   588,   576,   484,   138,   205,
       0,     0,   330,   319,     0,   322,   329,   339,   339,   335,
     568,  1089,   434,  1089,     0,  1016,     0,   974,   434,     0,
     434,  1059,   362,   916,   971,   898,   897,   883,   613,   857,
     607,     0,   616,   857,   633,   625,   628,   619,     0,   885,
     857,   902,   622,   434,   140,   205,   149,   154,   175,   271,
     205,   279,   285,   140,   287,     0,  1105,  1075,  1079,     0,
       0,     0,   606,   843,   591,     0,  1024,  1023,   841,   140,
     218,  1098,     0,     0,     0,  1062,     0,     0,     0,   265,
       0,  1054,     0,   397,   393,   399,   759,    36,     0,   387,
       0,   392,   396,   409,     0,   407,   412,     0,   411,     0,
     410,   451,     0,   222,     0,     0,   365,     0,   366,   367,
       0,     0,  1000,     0,   659,   657,   669,   667,     0,   331,
     332,     0,     0,   317,   328,     0,     0,     0,  1089,  1083,
     235,   568,  1089,   976,   241,   358,   247,   434,     0,     0,
       0,   610,   892,   905,     0,   243,   293,   205,   205,   140,
     274,   174,   286,  1106,  1128,   861,     0,     0,     0,   205,
       0,     0,   459,     0,  1063,     0,   377,   381,   456,   457,
     391,     0,     0,     0,   372,   718,   719,   717,   720,   721,
     738,   740,   739,   709,   681,   679,   680,   699,   714,   715,
     675,   686,   687,   689,   688,   756,   708,   692,   690,   691,
     693,   694,   695,   696,   697,   698,   700,   701,   702,   703,
     704,   705,   707,   706,   676,   677,   678,   682,   683,   685,
     755,   723,   724,   728,   729,   730,   731,   732,   733,   716,
     735,   725,   726,   727,   710,   711,   712,   713,   736,   737,
     741,   743,   742,   744,   745,   722,   747,   746,   749,   751,
     750,   684,   754,   752,   753,   748,   734,   674,   404,   671,
       0,   373,   425,   426,   424,   417,     0,   418,   374,     0,
       0,   363,     0,     0,     0,   455,     0,   222,   231,   357,
       0,     0,     0,   318,   334,   972,   973,     0,     0,     0,
       0,  1089,  1083,     0,   237,  1089,   896,     0,     0,   140,
     272,   155,   176,   205,   605,   590,  1022,   216,   375,   376,
     454,   266,     0,   858,   858,     0,   400,   388,     0,     0,
       0,   406,   408,     0,     0,   413,   420,   421,   419,   452,
     449,  1064,     0,   364,     0,   458,     0,   359,     0,   333,
       0,   653,   860,   136,   860,  1085,     0,   427,   136,   225,
       0,     0,   233,     0,   609,   904,   205,     0,   178,   378,
     126,     0,   379,   380,     0,   857,     0,   857,   402,   398,
     403,   672,   673,     0,   389,   422,   423,   415,   416,   414,
       0,  1102,   368,   453,     0,   360,   654,   859,     0,   205,
     859,  1084,     0,  1088,   205,   136,   227,   229,     0,   275,
       0,   220,     0,   434,     0,   394,   401,   405,   450,     0,
     916,   370,     0,   651,   567,   570,  1086,  1087,   428,   205,
     273,     0,     0,   179,   385,     0,   433,   395,  1065,     0,
     860,   445,   916,   652,   572,     0,   219,     0,     0,   384,
    1089,   916,   302,  1131,   448,   447,   446,  1131,   444,     0,
       0,     0,   383,  1083,   445,     0,     0,  1089,     0,   382,
       0,  1131,  1131,   308,     0,   307,   305,  1083,   140,   429,
     136,   369,     0,     0,   309,     0,     0,   303,     0,   205,
     205,   313,     0,   312,   301,     0,   304,   311,   371,   215,
     430,   314,     0,     0,   299,   310,     0,   300,   316,   315
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1800, -1800, -1800,  -577, -1800, -1800, -1800,   550,  -449,   -47,
     444, -1800,  -190,  -514, -1800, -1800,   471,   273,  1825, -1800,
    2780, -1800,  -811, -1800,  -541, -1800,  -706,    11, -1800, -1800,
   -1800, -1800, -1800, -1800, -1800, -1800,  -934, -1800, -1800,  -921,
    -289, -1800, -1800, -1800,  -358, -1800, -1800,  -191,    21,    39,
   -1800, -1800, -1800, -1800, -1800, -1800,    57, -1800, -1800, -1800,
   -1800, -1800, -1800,    58, -1800, -1800,  1163,  1182,  1183,   -97,
    -731,  -917,   628,   705,  -366,   350, -1012, -1800,   -72, -1800,
   -1800, -1800, -1800,  -767,   159, -1800, -1800, -1800, -1800,  -353,
   -1800,  -614, -1800,   433,  -473, -1800, -1800,  1068, -1800,   -50,
   -1800, -1800, -1072, -1800, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800,   -85, -1800,     8, -1800, -1800, -1800, -1800, -1800,  -165,
   -1800,   123, -1032, -1800, -1534,  -384, -1800,  -138,    71,  -105,
    -357, -1800,    -4, -1800, -1800, -1800,   135,   -89,   -77,    -8,
    -774,   -12, -1800, -1800,    38, -1800,    20,  -364, -1800,     7,
      -5,   -79,   -95,   -35, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800, -1800, -1800,  -635,  -899, -1800, -1800, -1800, -1800, -1800,
     366,  1312, -1800,   559, -1800,   405, -1800, -1800, -1800, -1800,
   -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800, -1800, -1800, -1800, -1800,   473,  -508,  -716, -1800, -1800,
   -1800, -1800, -1800,   488, -1800, -1800, -1800, -1800, -1800, -1800,
   -1800, -1800, -1003,  -373,  2785,    41, -1800,   616,  -429, -1800,
   -1800,  -475,  3442,  3859, -1800,   478, -1800, -1800,   568,    23,
    -655, -1800, -1800,   652,   416,   430, -1800,   414, -1800, -1800,
   -1800, -1800, -1800,   627, -1800, -1800, -1800,    26,  -930,  -152,
    -442,  -435, -1800,   -68,  -135, -1800, -1800,    44,    46,   693,
     -64, -1800, -1800,  1516,   -76, -1800,  -371,    36,  -383,   130,
    -415, -1800, -1800,  -455,  1339, -1800, -1800, -1800, -1800, -1800,
     791,   549, -1800, -1800, -1800,  -369,  -719, -1800,  1286, -1352,
    -221,   -48,  -166,   850, -1800, -1800, -1800, -1799, -1800,  -272,
   -1117, -1345,  -258,   166, -1800,   531,   609, -1800, -1800, -1800,
   -1800,   556, -1800,  3230,  -827
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   116,   975,   671,   192,   352,   785,
     372,   373,   374,   375,   926,   927,   928,   118,   119,   120,
     121,   122,   997,  1240,   432,  1029,   705,   706,   579,   268,
    1757,   585,  1660,  1758,  2011,   911,   124,   125,   726,   727,
     735,   365,   608,  1968,  1193,  1415,  2033,   455,   193,   707,
    1032,  1278,  1488,   128,   674,  1051,   708,   741,  1055,   646,
    1050,   247,   560,   709,   675,  1052,   457,   392,   414,   131,
    1034,   978,   951,  1213,  1688,  1338,  1117,  1911,  1761,   860,
    1123,   584,   869,  1125,  1532,   852,  1106,  1109,  1327,  2039,
    2040,   695,   696,  1013,   722,   723,   379,   380,   382,  1723,
    1889,  1890,  1429,  1587,  2020,  2042,  1922,  1972,  1973,  1974,
    1698,  1699,  1700,  1701,  1924,  1925,  1931,  1984,  1704,  1705,
    1709,  1875,  1876,  1877,  1959,  2081,  1588,  1589,   194,   133,
    2057,  2058,  1712,  1591,  1592,  1593,  1594,   134,   135,   654,
     581,   136,   137,   138,   139,   140,   141,   142,   143,   261,
     144,   145,   146,  1738,   147,  1031,  1277,   148,   692,   693,
     694,   265,   424,   575,   680,   681,  1376,   682,  1377,   149,
     150,   652,   653,  1366,  1367,  1497,  1498,   151,   895,  1083,
     152,   896,  1084,   153,   897,  1085,   154,   898,  1086,   155,
     899,  1087,   156,   900,  1088,   655,  1369,  1500,   157,   901,
     158,   159,  1952,   160,   676,  1725,   677,  1229,   984,  1448,
    1444,  1868,  1869,   161,   162,   163,   250,   164,   251,   262,
     436,   567,   165,  1370,  1371,   905,   906,   166,  1148,   559,
     623,  1149,  1091,  1300,  1092,  1501,  1502,  1303,  1304,  1094,
    1508,  1509,  1095,   828,   550,   206,   207,   710,   698,   534,
    1250,  1251,   816,   817,   465,   168,   253,   169,   170,   196,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     744,   257,   258,   649,   241,   242,   780,   781,  1383,  1384,
     407,   408,   969,   182,   637,   183,   691,   184,   355,  1891,
    1942,   393,   444,   716,   717,  1138,  1139,  1900,  1954,  1955,
    1244,  1426,   947,  1427,   948,   949,   875,   876,   877,   356,
     357,   908,   594,  1002,  1003
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     195,   197,   542,   199,   200,   201,   202,   204,   205,   353,
     208,   209,   210,   211,   429,   123,   231,   232,   233,   234,
     235,   236,   237,   238,   240,   126,   259,   462,  1030,   515,
     167,   851,   426,   431,   686,   449,   266,   535,   536,   267,
     264,  1000,  1110,   127,   683,   450,   685,   275,   687,   278,
     415,  1141,   363,   269,   366,   419,   420,   995,   273,   784,
     451,   129,   130,  1242,   819,   820,   256,   909,  1017,  1245,
     514,   249,  1054,   775,   254,   132,   255,   362,   401,   996,
     564,   267,  1576,   458,   730,   837,   823,  1113,  1234,   462,
     777,   778,   814,   844,   974,   361,   429,   427,  1263,   815,
    1268,  1127,  1334,  1961,   428,   568,  1100,   925,   930,    14,
    1775,   867,   446,   -43,   426,   431,   865,  1276,   -43,   936,
    1530,  1451,   576,   569,   821,  1933,   847,   553,   629,    14,
     632,    14,    14,   848,   -78,  1287,   -42,   576,  1715,   -78,
    1717,   -42,  -390,  1783,  1247,   552,   431,  1870,  1880,  1880,
     562,   576,  1934,   461,   953,  1775,  1439,  1019,   953,   953,
     953,   736,   737,   738,   953,   551,    14,  1312,   561,   381,
    1463,   625,   945,   946,  1246,  1730,  1242,   613,   615,   617,
    1578,   620,   842,  1323,  -761,   660,   428,   532,   533,  1248,
   -1069,  1950,  2084,   545,    55,  2101,   609,  1446,  1511,  1611,
     502,  2022,  -639,   459,   186,   187,    65,    66,    67,   532,
     533,   443,   503,     3,  1601,   543,  1159,   428,  1188,   945,
     946,  -963,    14,   918,  2074,  -110,   463,  1686,  -952,  2092,
   -1069,  1447,   198,   404,   626,  1313,  1951,  -994,   516,   443,
     428,  -110,   434,  -650,  1612,  -647,  2023,   260,  -762,  1380,
     661,   532,   533,   980,   973,   571,  1160,  1375,   571,  -647,
     610,  -324,   580,  1464,  2070,   267,   582,  -579,  1731,  2075,
     742,  1203,  -965,  -954,  2093,   573,   460,  -324,  2088,   578,
    -859,   868,  1249,  -306,  -859,  2085,   919,  1579,  2102,  1531,
    -951,  1241,  -950,  1580,   640,   459,  1581,   187,    65,    66,
      67,  1582,  1776,  1777,   447,   -43,  -857,   866,   593,  -460,
     937,   604,   639,   643,   577,   539,  1935,  1112,  -859,  -957,
     630,  1453,   633,  1272,  1290,  1523,   -78,   416,   -42,   659,
    1716,  -962,  1718,   376,  -390,  1784,  1576,  1613,  1778,  1871,
    1881,  1943,   938,  1583,  1584,   954,  1585,  2069,  1020,  1065,
    1430,  1659,   824,   732,   202,  1722,  1620,  1341,   464,  1345,
    -952,   411,  2076,  1884,   412,  1885,   981,  2094,   460,  -994,
    -649,   728,   431,  1487,   538,   532,   533,  1586,   463,  -647,
     638,   982,  -769,   267,   428,   224,   224,   377,   538,   942,
     240,   651,   267,   267,   651,   267,  -955,   263,  1225,   353,
     665,   983,  -960,  1622,  1241,  -954,   385,   740,  1199,  1200,
    1628,   462,  1630,   267,  -648,  1507,   386,  1273,  1461,  -961,
     204,  -964,  -951,  -109,  -950,  -997,  -996,  2006,   711,  2007,
     213,    40,  -937,  -958,  -953,   539,   532,   533,  -924,  -109,
     724,  -460,  1653,   731,   532,   533,  -938,   540,  -967,   463,
     697,  -957,   270,   126,  -924,   213,    40,   271,   743,   745,
    1343,  1344,  -764,   415,   790,   791,   458,   421,   729,   746,
     747,   748,   749,   751,   752,   753,   754,   755,   756,   757,
     758,   759,   760,   761,   762,   763,   764,   765,   766,   767,
     768,   769,   770,   771,   772,   773,   774,  1216,   776,  -965,
     743,   743,   779,   132,  1739,   795,  1741,  1438,   538,   378,
     464,   734,   798,   799,   800,   801,   802,   803,   804,   805,
     806,   807,   808,   809,   810,  -770,  -768,  -763,  -955,  1540,
    1343,  1344,   724,   724,   743,   822,   793,   797,   256,   798,
    2002,   784,   826,   249,   625,   918,   254,  1004,   255,  1005,
    1928,   834,   112,   836,   117,   796,  1346,  -997,  -996,  1520,
    1747,   724,  1049,  1253,  -937,  -958,  -953,   540,  1929,   855,
    1254,   856,   515,   830,   433,  -927,  1284,   112,  -938,   422,
    1340,  1609,  -925,  1440,   985,   272,   423,  1930,   387,   402,
     859,  -927,   402,  1503,   854,  1505,  1441,   667,  -925,  1510,
     435,   841,  1056,   276,   686,   224,   351,   442,   442,  1459,
     364,  2003,   402,   514,   683,   402,   685,  1442,   687,  1048,
     667,  1898,   932,   391,   402,  1902,  1533,   388,  1292,  1424,
    1425,   402,   403,  1610,   389,   226,   226,  1936,   786,   667,
     376,   376,   376,   618,   376,  1372,   413,  1374,   391,  1378,
     390,  1060,   394,   391,   391,  1265,  1937,  1265,  1675,  1938,
    1474,  1253,   395,  1476,   818,   402,   405,   406,  1254,   405,
     406,   402,   396,   438,   428,   397,   959,   961,   398,  1004,
    1005,   391,   670,   399,   786,   442,  1101,  1103,   925,   405,
     406,   662,   405,   406,  1036,   843,   383,   171,   849,   400,
     404,   405,   406,  1102,   988,   384,   416,   668,   405,   406,
    1381,   417,   228,   230,   714,   402,  1014,   532,   533,   697,
     672,   673,  1194,   667,  1195,  -126,  1196,   418,   564,  -126,
    1198,  1267,  1011,  1012,  1269,  1270,   713, -1069,   441,  1008,
    1987,  1189,   405,   406,   224,  1039,  -126,   871,   405,   406,
     442,  1754,  2053,   224,   445,   126,   448,   549,   443,  1988,
     224,  2071,  1989,   453,  1629,   459,   186,   187,    65,    66,
      67,   454,   224,   686, -1069,  1107,  1108, -1069,  1047,  1325,
    1326,   516,   507,   683,  1960,   685,   466,   687,  1963,  2054,
    2055,  2056,   405,   406,  1342,  1343,  1344,   212,  2054,  2055,
    2056,   467,   430,   508,  1046,   132,  -640,  1059,   872,  -641,
    1480,   612,   614,   616,   117,   619,  1606,  1489,   117,   468,
      50,  1490,   583,   469,   517,   518,   519,   520,   521,   522,
     523,   524,   525,   526,   527,   528,   529,   470,   460,   471,
    1105,   472,  1469,  1424,  1425,  2049,  1682,  1683,    34,    35,
      36,  1957,  1958,   580,   473,   226,   267,   216,   217,   218,
     219,   220,   214,  1624,  1111,  1527,  1343,  1344,  -642,   530,
     531,   656,   126,   658,  1568,  1720,  -645,  1122,  1265,   189,
     537,  1522,    91,  -643,   430,    93,    94,  -644,    95,   190,
      97,   688,   873,  1129,   929,   929,   499,   500,   501,  1135,
     502,  1985,  1986,  1030,   437,   439,   440,   224,   603,  1184,
    1185,  1186,   503,  -959,   108,   430,   505,    81,    82,    83,
      84,    85,   132,  2079,  2080,  1187,  1981,  1982,   221,   506,
     126,   686,   555,  2063,    89,    90,  1252,  1255,   563,  -646,
    -762,   683,   541,   685,   548,   687,   532,   533,    99,   409,
    2077,  1220,   663,  1221,   546,   856,   669,   171,   503,   443,
    -963,   171,   554,   105,   538,   558,  1223,   557,   566,  -760,
     565,  1211,  1779,   574,   587,  1655,  1596, -1114,   595,   621,
     132,  1233,   117,   598,   663,   123,   669,   663,   669,   669,
     599,  1664,   715,   605,   226,   126,   351,   606,   622,   631,
     167,   645,   624,   226,   634,   391,  1626,   635,  1291,   644,
     226,  1261,   689,   127,   731,   690,   731,   712,   126,  1748,
     699,   798,   226,   700,   701,   697,   703,   734,  -131,    55,
     739,   129,   130,   684,   827,  1279,   829,   662,  1280,  1264,
    1281,  1264,   831,   857,   724,   132,   797,   832,   838,   839,
     126,   459,   186,   187,    65,    66,    67,   697,   603,   391,
     788,   391,   391,   391,   391,  1242,   576,   861,   132,   864,
    1242,   628,   496,   497,   498,   499,   500,   501,   878,   502,
     636,   593,   641,   879,   813,   224,  1467,   648,   730,  1468,
    2041,   503,   910,   256,  1322,  1242,   912,   913,   249,   666,
     132,   254,   914,   255,   603,   915,   935,   916,   917,   939,
     920,   921,  2041,  1756,  1328,   940,   943,  1318,   846,  1329,
    1235,  2064,  1762,   944,   460,   171,   950,   952,   957,   117,
    1685,   126,   955,   126,   818,   818,   958,   960,  1769,   962,
     963,   733,  1999,   964,  1735,  1736,   965,  2004,   971,   907,
     976,   977,   979,  1454,   987,  -785,  1242,   226,   986,   989,
     224,  1357,   990,  1455,   991,  1360,   736,   737,   738,   994,
     998,   999,  1364,  1007,   686,   931,   715,  1015,  1456,  1009,
    1016,   132,  1018,   132,   683,   929,   685,   929,   687,   929,
    1021,  1022,  1033,   929,  2029,   929,   929,  1201,  1037,   224,
    1041,   224,  1023,  1024,  1025,  1042,  1045,  1432,  1044,  1053,
     968,   970,  1061,  1062,  1434,  1063,  1035,  1114,  1913,  1104,
    1128,  -766,  1124,  1132,  1133,  1445,  2065,  1126,  1134,   224,
    2066,   849,  1136,   849,   648,  1150,   123,   731,  1774,  1433,
    1151,  1685,  1152,  1153,  2082,  2083,   126,  1154,  1155,  1156,
    1157,   167,   743,  1202,  1192,  1472,  1204,  1093,  1210,  1206,
    1209,  1208,  1264,  1215,   127,  1219,  1685,   686,  1685,  2090,
    1222,  1228,   171,  1230,  1685,  1231,  1236,   683,   724,   685,
    1232,   687,   129,   130,   117,  1238,  1243,  1257,  1286,   724,
    1434,  1274,   391,  1241,  1283,  1289,   132,  1998,  1241,  2001,
     697,  1294,   224,   697,  -966,  1090,  1295,  1181,  1182,  1183,
    1184,  1185,  1186,  1307,  1305,  1306,  1308,  1309,   224,   224,
    1310,   535,  1311,  1241,  1314,   580,  1187,  1494,   267,  1315,
    1317,  1316,  1515,  1319,  1337,   226,  1331,  1333,  1529,  1518,
    1336,   544,   518,   519,   520,   521,   522,   523,   524,   525,
     526,   527,   528,   529,  1339,  1350,  1349,   429,  1348,  1356,
    1358,   126, -1130,  1359,  1187,  1362,  1363,  1414,  1966,  1416,
    1417,  1428,  1449,  1462,  1465,   426,   431,  1082,  1418,  1096,
    1466,  1421,  1419,  1431,  1241,  2052,   530,   531,  1049,  1450,
    1548,  1473,  1475,  1477,  1552,  1493,  1491,  1479,  1578,  1558,
    1481,   117,  1072,  1482,  1484,  1485,  1516,  1564,  1514,  1492,
     226,   132,  1010,  1517,  1506,  1120,   117,   459,    63,    64,
      65,    66,    67,  1519,  1524,  1534,  1528,   171,    72,   509,
    1597,  1537,  1541,  1542,  1546,  1545,  1547,  1602,  1551,  1556,
      14,  1550,  1604,  1553,  1605,  1557,  1554,   731,  1563,   226,
    1615,   226,  1555,  1559,  1685,  1595,  1614,  1721,  1639,   117,
    1616,  1569,  1562,   532,   533,  1595,  1560,  1567,  1197,   715,
    1570,   511,   729,  1625,   724,   929,  1571,  1572,  1608,   226,
     224,   224,  1598,  1621,  1599,   462,  1617,  1058,  1618,  1619,
     460,  1623,  1638,  1641,  1627,  1643,  1648,  1633,  1302,  1212,
    1590,  1631,  1650,   697,  1632,  1579,  1634,  1637,  1642,  1645,
    1590,  1580,  1646,   459,  1581,   187,    65,    66,    67,  1582,
    1647,  1640,  1882,  1649,   117,  1644,  1097,   702,  1098,  1652,
    1654,  1656,  1651,  1657,  1658,   229,   229,  2089,  1661,  1662,
     603,  1665,  1668,  1679,   171,  1690,  1090,   117,  1703,  1711,
    1719,  1724,   226,  1729,   813,   813,  1118,  1732,  1733,   171,
    1737,  1583,  1584,  1742,  1585,  1743,  1745,  1773,   226,   226,
    1764,  1749,  1767,  1781,  1782,  1879,  1878,  1886,  1892,   117,
    1893,  1895,  1896,  1905,  1897,  1899,   460,  1940,  1907,   126,
    1908,  1944,  1918,  1948,  1919,  1600,  1947,  1975,  1953,  1977,
    1979,  1983,   171,   684,  1728,  1990,   391,  1991,  1992,  1734,
    1713,  1996,   724,   724,  1997,  2000,  1299,  1299,  1082,  2005,
    2009,  2010,  -386,  2012,   224,  2013,  2024,  2015,  2017,  1207,
     126,  1934,  2030,  2032,  2021,  2037,  1772,  2031,  2043,   132,
     452,  2038,  2047,  2051,  2050,   648,  1218,  2062,  1595,  2060,
    2073,   846,  2086,   846,  1595,  2067,  1595,  2087,   117,   697,
     117,  2078,   117,  2068,  2091,  2095,   516,   171,  2096,  2103,
    2104,  1760,  2106,  1420,  2046,  2107,   126,   792,  1285,  1595,
     132,  2061,  1227,  1352,  1521,   126,  1912,   224,  1663,  2059,
     171,   787,  1471,  1590,   789,  1903,  1946,   933,  1927,  1590,
    1780,  1590,   224,   224,  2098,  1932,  1901,  2072,  1710,   603,
    1883,   657,  1266,  1373,  1266,  1504,  1691,  1443,  1365,  1301,
    1496,  1495,   171,  1320,  1590,  1994,   132,  1894,  2026,  1142,
     226,   226,   725,  2019,   650,   132,  1302,  1499,   907,  1681,
    1499,  1423,  1887,  1354,  1413,     0,     0,  1512,     0,     0,
       0,     0,     0,     0,     0,   229,  1296,  1297,  1298,   212,
       0,     0,     0,     0,     0,     0,     0,     0,  1910,  1760,
       0,     0,   684,  1595,     0,   117,     0,   126,     0,     0,
       0,     0,    50,   126,  1090,  1090,  1090,  1090,  1090,  1090,
     126,     0,     0,  1090,     0,  1090,     0,   224,     0,     0,
       0,   171,     0,   171,     0,   171,     0,  1118,  1335,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1590,   216,
     217,   218,   219,   220,     0,     0,     0,   132,     0,     0,
       0,     0,     0,   132,     0,  1941,     0,     0,     0,     0,
     132,     0,     0,  1578,   223,   223,     0,    93,    94,     0,
      95,   190,    97,     0,     0,   246,  1082,  1082,  1082,  1082,
    1082,  1082,     0,     0,     0,  1082,     0,  1082,     0,     0,
       0,     0,     0,     0,   226,  2035,   108,     0,   117,     0,
       0,   246,     0,     0,     0,    14,     0,  1949,     0,     0,
     117,     0,     0,     0,   229,     0,     0,     0,     0,  1941,
    1536,     0,     0,   229,     0,   642,     0,     0,     0,     0,
     229,     0,     0,     0,     0,     0,     0,     0,   171,     0,
       0,  1635,   229,  1636,     0,     0,     0,     0,     0,     0,
     684,   462,     0,     0,   126,  1266,     0,   226,     0,     0,
       0,     0,     0,     0,     0,     0,  1976,  1978,     0,     0,
    1579,  1470,   226,   226,     0,     0,  1580,     0,   459,  1581,
     187,    65,    66,    67,  1582,     0,     0,     0,     0,  1090,
       0,  1090,     0,  1573,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   132,     0,     0,   126,     0,     0,
     544,   518,   519,   520,   521,   522,   523,   524,   525,   526,
     527,   528,   529,     0,     0,     0,  1583,  1584,     0,  1585,
       0,     0,     0,     0,  1513,     0,     0,     0,     0,     0,
     126,   171,     0,     0,     0,   126,     0,     0,     0,   648,
    1118,   460,     0,   171,     0,   530,   531,   132,     0,     0,
    1740,  1082,  1692,  1082,     0,     0,   697,   226,     0,     0,
     126,     0,     0,     0,     0,     0,     0,   229,     0,     0,
       0,     0,     0,     0,   223,     0,     0,     0,   697,  1750,
     132,  1751,     0,  1752,     0,   132,     0,   697,  1753,   212,
       0,  2097,     0,     0,  2036,     0,     0,     0,     0,     0,
    2105,     0,   212,     0,     0,     0,     0,     0,  2108,     0,
     132,  2109,    50,     0,     0,     0,     0,     0,     0,     0,
     126,   126,   532,   533,   246,    50,   246,  1090,   117,  1090,
       0,  1090,     0,     0,   648,     0,  1090,     0,     0,   351,
       0,     0,     0,     0,     0,  1708,     0,     0,  1693,   216,
     217,   218,   219,   220,     0,  1607,     0,     0,     0,     0,
       0,  1694,   216,   217,   218,   219,   220,  1695,     0,   117,
     132,   132,  1578,     0,   409,     0,     0,    93,    94,     0,
      95,   190,    97,   684,   189,   246,   840,    91,  1696,  1906,
      93,    94,     0,    95,  1697,    97,     0,     0,     0,  1082,
       0,  1082,     0,  1082,     0,     0,   108,     0,  1082,     0,
     410,     0,     0,   223,    14,   117,     0,     0,     0,   108,
     117,     0,   223,     0,   117,     0,     0,     0,     0,   223,
       0,     0,     0,     0,     0,     0,     0,  1090,     0,     0,
       0,   223,     0,     0,   391,   229,     0,   603,     0,     0,
     351,     0,   223,     0,     0,     0,     0,     0,     0,     0,
    1867,     0,     0,     0,     0,     0,     0,  1874,     0,     0,
       0,   171,     0,     0,     0,   351,   684,   351,   246,  1579,
       0,   246,     0,   351,     0,  1580,  1578,   459,  1581,   187,
      65,    66,    67,  1582,     0,   544,   518,   519,   520,   521,
     522,   523,   524,   525,   526,   527,   528,   529,     0,  1082,
       0,     0,   171,     0,     0,     0,   117,   117,   117,     0,
     229,     0,   117,   212,     0,   213,    40,     0,    14,   117,
       0,     0,     0,     0,     0,  1583,  1584,   246,  1585,     0,
     530,   531,     0,     0,     0,     0,    50,  1964,  1965,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   171,   229,
     460,   229,     0,   171,     0,     0,     0,   171,     0,  1744,
       0,     0,     0,     0,     0,     0,   223,     0,     0,     0,
       0,     0,     0,   216,   217,   218,   219,   220,     0,   229,
       0,     0,     0,  1579,  1578,  1090,  1090,     0,     0,  1580,
       0,   459,  1581,   187,    65,    66,    67,  1582,     0,   811,
       0,    93,    94,     0,    95,   190,    97,   532,   533,     0,
       0,     0,  1578,     0,     0,     0,     0,     0,   246,     0,
     246,     0,     0,   894,     0,   871,    14,     0,     0,     0,
     108,     0,   603,     0,   812,     0,     0,   112,     0,  1583,
    1584,     0,  1585,     0,     0,     0,     0,     0,     0,   171,
     171,   171,   229,   351,    14,   171,   894,  1082,  1082,     0,
       0,     0,   171,   117,   460,     0,     0,     0,   229,   229,
       0,   941,  1970,  1746,     0,   212,     0,     0,     0,  1867,
    1867,     0,     0,  1874,  1874,     0,   872,     0,     0,     0,
       0,  1579,     0,     0,     0,     0,   603,  1580,    50,   459,
    1581,   187,    65,    66,    67,  1582,     0,     0,     0,     0,
       0,     0,     0,     0,   246,   246,   117,     0,     0,  1579,
       0,     0,     0,   246,     0,  1580,     0,   459,  1581,   187,
      65,    66,    67,  1582,     0,   216,   217,   218,   219,   220,
       0,     0,     0,     0,   223,     0,     0,  1583,  1584,   117,
    1585,     0,     0,     0,   117,     0,     0,   189,     0,     0,
      91,     0,  2034,    93,    94,     0,    95,   190,    97,     0,
    1353,     0,   460,     0,     0,  1583,  1584,     0,  1585,   117,
       0,  1755,     0,     0,     0,  2048,     0,     0,     0,     0,
       0,     0,   108,     0,     0,     0,     0,     0,     0,     0,
     460,     0,     0,     0,     0,     0,   171,     0,     0,  1904,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   223,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     229,   229,     0,     0,     0,     0,     0,     0,     0,   117,
     117,  1161,  1162,  1163,     0,     0,     0,     0,     0,     0,
       0,     0,   246,     0,     0,     0,     0,     0,   223,   171,
     223,     0,  1164,     0,     0,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,     0,   223,   894,
       0,     0,   171,     0,     0,     0,   246,   171,     0,     0,
    1187,     0,     0,   246,   246,   894,   894,   894,   894,   894,
       0,     0,     0,     0,     0,     0,     0,     0,   894,     0,
       0,     0,   171,     0,     0,     0,     0,     0,     0,   212,
       0,     0,     0,     0,     0,   246,     0,     0,     0,  1026,
     518,   519,   520,   521,   522,   523,   524,   525,   526,   527,
     528,   529,    50,     0,     0,     0,     0,     0,  1457,     0,
       0,   223,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   229,   246,     0,   223,   223,     0,
       0,     0,   171,   171,   530,   531,     0,     0,     0,   216,
     217,   218,   219,   220,     0,   459,    63,    64,    65,    66,
      67,   246,   246,     0,   225,   225,    72,   509,     0,     0,
       0,   189,   223,     0,    91,   248,     0,    93,    94,   246,
      95,   190,    97,     0,     0,     0,   246,     0,     0,     0,
       0,  1379,   246,     0,     0,     0,   354,   229,     0,     0,
       0,     0,     0,   894,     0,     0,   108,   510,     0,   511,
       0,  1969,   229,   229,     0,     0,     0,     0,   246,     0,
       0,   532,   533,   512,     0,   513,     0,     0,   460,     0,
       0,   474,   475,   476,     0,     0,     0,     0,   246,     0,
       0,     0,   246,     0,     0,     0,     0,     0,     0,     0,
       0,   477,   478,   246,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
       0,     0,     0,     0,     0,  1027,     0,     0,     0,     0,
     503,     0,     0,     0,     0,     0,     0,     0,     0,   223,
     223,     0,     0,     0,     0,     0,     0,   229,     0,     0,
       0,     0,     0,   246,     0,     0,     0,   246,     0,   246,
       0,     0,   246,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   894,   894,   894,   894,   894,
     894,   223,   894,     0,     0,   894,   894,   894,   894,   894,
     894,   894,   894,   894,   894,   894,   894,   894,   894,   894,
     894,   894,   894,   894,   894,   894,   894,   894,   894,   894,
     894,   894,   894,   474,   475,   476,     0,     0,     0,     0,
       0,     0,     0,     0,   225,     0,     0,     0,     0,     0,
       0,   894,     0,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,   474,   475,   476,     0,  1436,     0,     0,   246,   354,
     246,   354,   503,     0,     0,     0,     0,     0,     0,     0,
       0,   477,   478,   223,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
       0,   246,     0,     0,   246,     0,     0,     0,     0,     0,
     503,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     354,   246,   246,   246,   246,   246,   246,     0,     0,   223,
     246,     0,   246,     0,     0,     0,   223, -1132, -1132, -1132,
   -1132, -1132,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,
       0,   223,   223,   225,   894,     0,     0,     0,     0,     0,
       0,     0,   225,  1187,   246,     0,     0,     0,     0,   225,
       0,   246,     0,     0,     0,     0,   894,     0,   894,     0,
       0,   225,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   225,     0,   504,     0,  1038,     0,     0,     0,
       0,     0,     0,   894,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   354,     0,     0,   354,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   474,   475,   476,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   246,
     246,     0,     0,   246,   972,     0,   223,   477,   478,     0,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,   246,   358,   248,     0,     0,
       0,     0,     0,     0,     0,     0,   503,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   474,   475,
     476,     0,     0,     0,     0,     0,   246,     0,   246,     0,
       0,     0,     0,     0,     0,     0,   225,     0,   477,   478,
       0,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,     0,     0,     0,
       0,     0,   246,   246,     0,     0,   246,   503,     0,     0,
       0,     0,   894,   354,   894,   874,   894,     0,     0,     0,
       0,   894,   223,   902,     0,     0,   894,     0,   894,     0,
       0,   894,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   246,   246,     0,     0,   246,     0,
       0,     0,   474,   475,   476,   246,   902,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1006,     0,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
       0,   227,   227,     0,   246,     0,   246,     0,   246,   354,
     354,   503,   252,   246,     0,   223,     0,     0,   354,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     246,     0,     0,     0,     0,   894,     0,     0,     0,     0,
       0,  1038,     0,     0,   225,     0,     0,   246,   246,     0,
       0,     0,     0,     0,     0,   246,     0,   246,     0,   591,
       0,   592,     0,     0,     0,     0,   474,   475,   476,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     246,     0,   246,     0,     0,     0,   477,   478,   246,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,   246,     0,     0,     0,     0,   225,
     597,     0,     0,     0,     0,   503,     0,     0,     0,     0,
       0,   894,   894,   894,     0,     0,     0,     0,   894,     0,
     246,     0,     0,     0,     0,  1064,   246,     0,   246,     0,
       0,     0,  1089,     0,     0,     0,     0,     0,   225,     0,
     225,     0,     0,     0,   212,     0,  1026,   518,   519,   520,
     521,   522,   523,   524,   525,   526,   527,   528,   529,     0,
       0,     0,     0,     0,     0,     0,     0,    50,   225,   902,
       0,  1131,     0,     0,     0,     0,     0,     0,   354,   354,
       0,     0,     0,     0,     0,   902,   902,   902,   902,   902,
       0,   530,   531,   718,     0,     0,   358,     0,   902,     0,
       0,   227,     0,     0,   216,   217,   218,   219,   220,     0,
       0,     0,     0,     0,     0,  1191,     0,   212,     0,     0,
       0,     0,     0,     0,   894,     0,     0,     0,  1130,     0,
    1872,   246,    93,    94,  1873,    95,   190,    97,     0,  1205,
      50,   225,     0,     0,   246,     0,     0,     0,   246,     0,
       0,     0,   246,   246,     0,  1214,     0,   225,   225,     0,
       0,   108,  1707,     0,     0,     0,     0,   246,   532,   533,
       0,     0,     0,   894,     0,     0,   354,   216,   217,   218,
     219,   220,  1214,     0,     0,     0,     0,     0,     0,   894,
       0,     0,   225,   894,   354,     0,     0,     0,     0,   189,
       0,   354,    91,     0,     0,    93,    94,   354,    95,   190,
      97,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     246,   212,     0,   902,     0,     0,     0,     0,     0,     0,
       0,     0,   702,     0,   108,   894,     0,     0,  1275,     0,
     227,     0,     0,   870,    50,   246,     0,   246,     0,   227,
       0,     0,     0,   354,     0,     0,   227,     0,     0,     0,
       0,     0,   248,     0,     0,     0,     0,  1693,   227,     0,
       0,     0,     0,  1089,     0,     0,   246,     0,     0,   252,
    1694,   216,   217,   218,   219,   220,  1695,     0,     0,     0,
       0,     0,   246,     0,     0,     0,     0,     0,   246,     0,
       0,     0,   246,   189,     0,     0,    91,    92,     0,    93,
      94,     0,    95,  1697,    97,     0,   246,   246,     0,   225,
     225,     0,     0,     0,     0,     0,     0,     0,   354,     0,
       0,     0,   354,     0,   874,     0,     0,   354,   108,   992,
     993,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   902,   902,   902,   902,   902,
     902,   225,   902,     0,   252,   902,   902,   902,   902,   902,
     902,   902,   902,   902,   902,   902,   902,   902,   902,   902,
     902,   902,   902,   902,   902,   902,   902,   902,   902,   902,
     902,   902,   902,   474,   475,   476,     0,     0,     0,     0,
       0,     0,     0,   227,     0,     0,     0,     0,     0,     0,
       0,   902,     0,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,     0,     0,   354,     0,   354,   212,     0,   213,    40,
       0,     0,   503,     0,     0,     0,     0,     0,     0,     0,
     903,     0,     0,   225,     0,     0,     0,     0,     0,    50,
   -1132, -1132, -1132, -1132, -1132,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,     0,   354,     0,     0,   354,
       0,     0,     0,   903,     0,     0,   503,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   216,   217,   218,   219,
     220,  1089,  1089,  1089,  1089,  1089,  1089,     0,     0,   225,
    1089,     0,  1089,     0,     0,     0,   225,     0,  1140,   718,
       0,     0,   811,     0,    93,    94,     0,    95,   190,    97,
       0,   225,   225,     0,   902,     0,     0,     0,     0,   354,
       0,     0,     0,     0,     0,     0,   354,     0,     0,     0,
       0,     0,     0,   108,     0,     0,   902,   845,   902,     0,
     112,   474,   475,   476,     0,     0,     0,     0,     0,     0,
       0,   227,     0,     0,     0,     0,  1282,     0,     0,     0,
       0,   477,   478,   902,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
       0,     0,     0,     0,   354,   354,  1226,     0,     0,     0,
     503,     0,     0,  1577,     0,     0,   225,     0,     0,     0,
       0,   474,   475,   476,  1237,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   227,  1256,     0,     0,
     354,   477,   478,     0,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
       0,     0,     0,     0,     0,   227,  1089,   227,  1089,     0,
     503,     0,     0,  1288,     0,   544,   518,   519,   520,   521,
     522,   523,   524,   525,   526,   527,   528,   529,     0,     0,
       0,     0,     0,     0,     0,   227,   903,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   354,   354,     0,
       0,   354,   903,   903,   903,   903,   903,     0,     0,   212,
     530,   531,   902,     0,   902,   903,   902,     0,     0,     0,
       0,   902,   225,     0,  1293,     0,   902,     0,   902,     0,
       0,   902,    50,     0,     0,     0,     0,     0,  1347,   354,
       0,     0,  1351,     0,     0,  1689,     0,  1355,  1702,   289,
     354,     0,     0,     0,     0,     0,     0,     0,   227,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   216,
     217,   218,   219,   220,   227,   227,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   291,   532,   533,     0,
       0,     0,     0,     0,  1324,     0,     0,    93,    94,   212,
      95,   190,    97,     0,  1089,  1137,  1089,     0,  1089,   252,
       0,     0,     0,  1089,     0,   225,     0,     0,     0,     0,
       0,     0,    50,     0,     0,   354,   108,   739,     0,     0,
       0,     0,     0,     0,     0,   902,     0,   904,     0,     0,
     903,     0,     0,     0,     0,     0,     0,  1770,  1771,     0,
     354,     0,     0,  1458,     0,     0,     0,  1702,   589,   216,
     217,   218,   219,   220,   590,     0,     0,     0,     0,     0,
     934,     0,     0,     0,     0,   354,     0,   354,     0,   252,
       0,   189,     0,   354,    91,   344,     0,    93,    94,     0,
      95,   190,    97,     0, -1131,     0,  1483,     0,     0,  1486,
       0,     0,     0,     0,     0,   348,     0,     0,     0,     0,
       0,     0,     0,     0,  1089,     0,   108,   350,  1026,   518,
     519,   520,   521,   522,   523,   524,   525,   526,   527,   528,
     529,   902,   902,   902,     0,     0,   227,   227,   902,     0,
    1921,   354,     0,     0,     0,     0,     0,     0,  1702,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1535,
       0,     0,     0,   530,   531,     0,  1539,     0,     0,     0,
       0,     0,   903,   903,   903,   903,   903,   903,   252,   903,
       0,     0,   903,   903,   903,   903,   903,   903,   903,   903,
     903,   903,   903,   903,   903,   903,   903,   903,   903,   903,
     903,   903,   903,   903,   903,   903,   903,   903,   903,   903,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   903,     0,
       0,     0,     0,     0,  1574,  1575,     0,     0,     0,     0,
     532,   533,     0,     0,   902,     0,   354,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   354,
       0,     0,     0,   354,     0,     0,     0,     0,     0,     0,
       0,     0,  1089,  1089,     0,     0,     0,     0,   212,     0,
     227,     0,  1971,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   902,     0,     0,     0,     0,     0,     0,
       0,    50,     0,  1119,     0,     0,     0,     0,     0,   902,
       0,     0,     0,   902,     0,     0,     0,     0,     0,  1143,
    1144,  1145,  1146,  1147,     0,     0,     0,     0,   289,     0,
       0,     0,  1158,     0,     0,   354,   252,     0,   216,   217,
     218,   219,   220,   227,     0,     0,     0,  1666,  1667,     0,
       0,  1669,     0,     0,     0,   902,     0,     0,   227,   227,
     354,   903,   354,     0,   456,   291,    93,    94,     0,    95,
     190,    97,     0,     0,     0,     0,     0,     0,   212,     0,
       0,     0,     0,   903,     0,   903,     0,     0,   212,  1687,
       0,     0,     0,     0,     0,   108,  2045,     0,     0,     0,
    1714,    50,     0,     0,     0,     0,     0,     0,     0,  -433,
     903,    50,  1689,   354,     0,     0,     0,   354,   459,   186,
     187,    65,    66,    67,     0,     0,     0,     0,     0,     0,
       0,   354,   354,     0,     0,     0,     0,   589,   216,   217,
     218,   219,   220,   590,     0,     0,     0,     0,   216,   217,
     218,   219,   220,   227,     0,     0,     0,  1262,     0,     0,
     189,     0,     0,    91,   344,     0,    93,    94,     0,    95,
     190,    97,     0,     0,     0,  1763,    93,    94,     0,    95,
     190,    97,     0,     0,   348,     0,     0,     0,     0,     0,
       0,   460,     0,     0,     0,   108,   350,     0,     0,     0,
    1687,     0,     0,     0,     0,   108,  1035,     0,     0,     0,
       0,   474,   475,   476,     0,     0,     0,     0,     0,     0,
       0,     0,   289,     0,     0,  1687,     0,  1687,     0,     0,
       0,   477,   478,  1687,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,   291,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   903,
     503,   903,   212,   903,     0,     0,     0,     0,   903,   252,
       0,     0,     0,   903,     0,   903,     0,     0,   903,  1147,
    1368,  1923,     0,  1368,     0,    50,     0,     0,     0,  1382,
    1385,  1386,  1387,  1389,  1390,  1391,  1392,  1393,  1394,  1395,
    1396,  1397,  1398,  1399,  1400,  1401,  1402,  1403,  1404,  1405,
    1406,  1407,  1408,  1409,  1410,  1411,  1412,     0,     0,     0,
       0,   589,   216,   217,   218,   219,   220,   590,     0,     0,
       0,     0,     0,     0,     0,  1422,     0,     0,     0,     0,
       0,     0,     0,     0,   189,     0,     0,    91,   344,     0,
      93,    94,     0,    95,   190,    97,     0,     0,     0,     0,
       0,     0,   252,     0,     0,     0,     0,     0,   348,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   108,
     350,     0,   903,     0,     0,     0,  1945,     0,     0,     0,
       0,     0,     0,     0,  1603,     0,     0,     0,     0,  1956,
       0,     0,     0,  1687,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   474,   475,   476,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,     0,     0,     5,     6,     7,     8,     9,  1525,     0,
       0,     0,   503,    10,     0,  2014,     0,     0,   903,   903,
     903,     0,     0,     0,     0,   903,     0,    11,    12,    13,
    1543,     0,  1544,     0,  1926,     0,     0,     0,     0,     0,
    1956,     0,  2027,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,  1565,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,   903,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,  1726,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   189,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   190,    97,    98,     0,     0,    99,     0,
     903,   100,     0,     0,     0,     0,     0,   101,   102,     0,
       0,     0,     0,   105,   106,   107,   903,     0,   108,   109,
     903,   110,   111,   704,     0,   112,   113,   114,   115,     0,
       0,     0,     0,     0,     0,     0,  1671,     0,  1672,     0,
    1673,     0,     0,     0,     0,  1674,     0,     0,     0,  2016,
    1676,     0,  1677,     0,     0,  1678,     0,     0,     0,     0,
       0,     0,   903,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,  1765,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,    56,    57,    58,     0,    59,
    -205,    60,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,   212,    86,     0,     0,
      87,     0,     0,     0,     0,    88,    89,    90,    91,    92,
       0,    93,    94,     0,    95,    96,    97,    98,     0,    50,
      99,     0,     0,   100,     0,  1914,  1915,  1916,     0,   101,
     102,   103,  1920,   104,     0,   105,   106,   107,     0,     0,
     108,   109,     0,   110,   111,  1706,     0,   112,   113,   114,
     115,     0,     0,     0,     0,     0,   216,   217,   218,   219,
     220,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,    93,    94,     0,    95,   190,    97,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   108,  1707,     0,     0,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,  1939,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,     0,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,    56,    57,    58,     0,    59,  1980,    60,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
      71,    72,    73,  1993,     0,     0,     0,  1995,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,    88,    89,    90,    91,    92,     0,    93,    94,
       0,    95,    96,    97,    98,     0,     0,    99,     0,  2018,
     100,     0,     0,     0,     0,     0,   101,   102,   103,     0,
     104,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,  1224,     0,   112,   113,   114,   115,     5,     6,
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
      51,     0,     0,     0,    52,    53,    54,    55,    56,    57,
      58,     0,    59,     0,    60,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,    88,    89,
      90,    91,    92,     0,    93,    94,     0,    95,    96,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,   103,     0,   104,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,  1437,     0,
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
     109,     0,   110,   111,  1028,     0,   112,   113,   114,   115,
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
       0,    57,    58,     0,    59,  -205,     0,    61,    62,    63,
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
       0,   108,   109,     0,   110,   111,  1190,     0,   112,   113,
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
     110,   111,  1239,     0,   112,   113,   114,   115,     5,     6,
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
     107,     0,     0,   108,   109,     0,   110,   111,  1271,     0,
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
     109,     0,   110,   111,  1330,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,     0,    42,     0,     0,     0,    43,
      44,    45,    46,  1332,    47,     0,    48,     0,    49,     0,
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
      47,     0,    48,     0,    49,  1526,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   189,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   190,    97,    98,     0,
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
       0,     0,   189,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   190,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,  1680,     0,   112,   113,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,  -297,    34,    35,    36,    37,    38,    39,    40,
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
       0,     0,     0,     0,   189,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   190,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,  1917,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,  1967,    49,     0,
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
      47,  2008,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   189,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   190,    97,    98,     0,
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
       0,     0,   189,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   190,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,  2025,     0,   112,   113,   114,   115,     5,     6,
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
     107,     0,     0,   108,   109,     0,   110,   111,  2028,     0,
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
     109,     0,   110,   111,  2044,     0,   112,   113,   114,   115,
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
    2099,     0,   112,   113,   114,   115,     5,     6,     7,     8,
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
       0,   108,   109,     0,   110,   111,  2100,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,   572,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,    11,    12,    13,     0,     0,   858,     0,     0,
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
      13,     0,     0,  1121,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,    11,    12,    13,     0,     0,  1759,
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
      11,    12,    13,     0,     0,  1909,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,   359,   425,    13,     0,     0,     0,     0,     0,
       0,     0,     0,   794,     0,     0,     0,     0,     0,     0,
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
     107,     0,     0,   108,   109,     5,     6,     7,     8,     9,
     112,   113,   114,   115,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   359,
       0,    13,     0,     0,     0,     0,     0,     0,     0,     0,
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
     108,   191,     0,   360,     0,     0,     0,   112,   113,   114,
     115,     0,     0,     0,     0,     0,     0,     0,     0,   719,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,   720,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,   185,   186,   187,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     188,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   189,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   190,    97,     0,   721,     0,    99,
       0,     0,   100,     5,     6,     7,     8,     9,   101,   102,
       0,     0,     0,    10,   105,   106,   107,     0,     0,   108,
     191,     0,     0,     0,     0,     0,   112,   113,   114,   115,
       0,     0,     0,     0,     0,     0,     0,     0,  1258,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,  1259,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,   185,   186,   187,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   188,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   189,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   190,    97,     0,  1260,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
       0,     0,     0,   105,   106,   107,     0,     0,   108,   191,
       5,     6,     7,     8,     9,   112,   113,   114,   115,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   359,     0,     0,     0,     0,     0,
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
     190,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   191,     0,     0,   853,
       0,     0,   112,   113,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     359,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   794,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   191,     5,     6,     7,     8,     9,   112,   113,
     114,   115,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   359,   425,     0,
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
       0,     0,     0,    50,     0,     0,     0,     0,   203,     0,
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
       0,     0,     0,     0,     0,     0,   239,     0,     0,     0,
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
     105,   106,   107,     0,     0,   108,   191,     0,   274,     0,
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
     106,   107,     0,     0,   108,   191,     0,   277,     0,     0,
       0,   112,   113,   114,   115,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     425,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     108,   109,     0,     0,     0,     0,     0,   112,   113,   114,
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
     191,   570,     0,     0,     0,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   359,     0,     0,     0,     0,     0,
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
     105,   106,   107,     0,     0,   108,   191,     0,     0,     0,
       0,     0,   112,   113,   114,   115,     0,     0,   750,     0,
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
     106,   107,     0,     0,   108,   191,     0,     0,     0,     0,
       0,   112,   113,   114,   115,     0,     0,     0,     0,     0,
       0,     0,     0,   794,     0,     0,     0,     0,     0,     0,
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
     107,     0,     0,   108,   191,     0,     0,     0,     0,     0,
     112,   113,   114,   115,     0,     0,     0,     0,     0,     0,
       0,     0,   833,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,   108,   191,     0,     0,     0,     0,     0,   112,
     113,   114,   115,     0,     0,     0,     0,     0,     0,     0,
       0,   835,     0,     0,     0,     0,     0,     0,     0,     0,
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
    1321,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     108,   191,     5,     6,     7,     8,     9,   112,   113,   114,
     115,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   359,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,    10,   105,   106,   107,     0,     0,   108,  1452,     0,
       0,     0,     0,     0,   112,   113,   114,   115,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,   664,
      39,    40,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   185,   186,
     187,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   188,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     189,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     190,    97,     0,   279,   280,    99,   281,   282,   100,     0,
     283,   284,   285,   286,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   191,     0,   287,   288,
       0,     0,   112,   113,   114,   115,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   290,   502,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     503,   292,   293,   294,   295,   296,   297,   298,     0,     0,
       0,   212,     0,   213,    40,     0,     0,   299,     0,     0,
       0,     0,     0,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,    50,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   212,
     335,     0,   782,   337,   338,   339,     0,     0,     0,   340,
     600,   216,   217,   218,   219,   220,   601,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,   279,   280,     0,
     281,   282,     0,   602,   283,   284,   285,   286,     0,    93,
      94,     0,    95,   190,    97,   345,     0,   346,     0,     0,
     347,     0,   287,   288,     0,     0,     0,     0,   349,   216,
     217,   218,   219,   220,     0,     0,     0,     0,   108,     0,
       0,     0,   783,     0,     0,   112,     0,     0,     0,     0,
       0,   290,     0,     0,   370,     0,     0,    93,    94,     0,
      95,   190,    97,     0,     0,   292,   293,   294,   295,   296,
     297,   298,     0,     0,     0,   212,     0,   213,    40,     0,
       0,   299,     0,     0,     0,     0,   108,   300,   301,   302,
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
       0,   343,     0,     0,    91,   344,     0,    93,    94,     0,
      95,   190,    97,   345,    50,   346,     0,     0,   347,     0,
     279,   280,     0,   281,   282,   348,   349,   283,   284,   285,
     286,     0,     0,     0,     0,     0,   108,   350,     0,     0,
       0,  1888,     0,     0,     0,   287,   288,     0,   289,     0,
       0,   216,   217,   218,   219,   220,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   189,   290,     0,    91,    92,     0,    93,
      94,     0,    95,   190,    97,   291,     0,     0,   292,   293,
     294,   295,   296,   297,   298,     0,     0,     0,   212,     0,
       0,     0,     0,     0,   299,     0,     0,     0,   108,     0,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,    50,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,     0,   335,     0,     0,
     337,   338,   339,     0,     0,     0,   340,   341,   216,   217,
     218,   219,   220,   342,     0,     0,     0,     0,     0,     0,
     212,     0,   966,     0,   967,     0,     0,     0,     0,     0,
     343,     0,     0,    91,   344,     0,    93,    94,     0,    95,
     190,    97,   345,    50,   346,     0,     0,   347,     0,   279,
     280,     0,   281,   282,   348,   349,   283,   284,   285,   286,
       0,     0,     0,     0,     0,   108,   350,     0,     0,     0,
    1962,     0,     0,     0,   287,   288,     0,   289,     0,     0,
     216,   217,   218,   219,   220,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,   290,   502,     0,     0,     0,    93,    94,
       0,    95,   190,    97,   291,     0,   503,   292,   293,   294,
     295,   296,   297,   298,     0,     0,     0,   212,     0,     0,
       0,     0,     0,   299,     0,     0,     0,   108,     0,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
      50,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,     0,   335,     0,   336,   337,
     338,   339,     0,     0,     0,   340,   341,   216,   217,   218,
     219,   220,   342,     0,     0,     0,     0,     0,     0,   212,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   343,
       0,     0,    91,   344,     0,    93,    94,     0,    95,   190,
      97,   345,    50,   346,     0,     0,   347,     0,   279,   280,
       0,   281,   282,   348,   349,   283,   284,   285,   286,     0,
       0,     0,     0,     0,   108,   350,     0,     0,     0,     0,
       0,     0,     0,   287,   288,     0,   289,     0,     0,   216,
     217,   218,   219,   220,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,   290,     0,   924,     0,     0,    93,    94,     0,
      95,   190,    97,   291,     0,  1187,   292,   293,   294,   295,
     296,   297,   298,     0,     0,     0,   212,     0,     0,     0,
       0,     0,   299,     0,     0,     0,   108,     0,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,    50,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,     0,   335,     0,     0,   337,   338,
     339,     0,     0,     0,   340,   341,   216,   217,   218,   219,
     220,   342,     0,     0,     0,     0,     0,     0,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   343,     0,
       0,    91,   344,     0,    93,    94,     0,    95,   190,    97,
     345,    50,   346,     0,     0,   347,     0,     0,     0,   922,
     923,     0,   348,   349,  1684,     0,     0,     0,   279,   280,
       0,   281,   282,   108,   350,   283,   284,   285,   286,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   216,   217,
     218,   219,   220,   287,   288,     0,   289,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   924,     0,     0,    93,    94,     0,    95,
     190,    97,   290,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   291,     0,     0,   292,   293,   294,   295,
     296,   297,   298,     0,     0,   108,   212,     0,     0,     0,
       0,     0,   299,     0,     0,     0,     0,     0,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,    50,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,     0,   335,     0,     0,   337,   338,
     339,     0,     0,     0,   340,   341,   216,   217,   218,   219,
     220,   342,     0,     0,     0,     0,     0,     0,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   343,     0,
       0,    91,   344,     0,    93,    94,     0,    95,   190,    97,
     345,    50,   346,     0,     0,   347,     0,  1785,  1786,  1787,
    1788,  1789,   348,   349,  1790,  1791,  1792,  1793,     0,     0,
       0,     0,     0,   108,   350,     0,     0,     0,     0,     0,
       0,  1794,  1795,  1796,     0,     0,     0,     0,   216,   217,
     218,   219,   220,     0,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,  1797,   502,     0,     0,     0,    93,    94,     0,    95,
     190,    97,     0,     0,   503,  1798,  1799,  1800,  1801,  1802,
    1803,  1804,     0,     0,     0,   212,     0,     0,     0,     0,
       0,  1805,     0,     0,     0,   108,     0,  1806,  1807,  1808,
    1809,  1810,  1811,  1812,  1813,  1814,  1815,  1816,    50,  1817,
    1818,  1819,  1820,  1821,  1822,  1823,  1824,  1825,  1826,  1827,
    1828,  1829,  1830,  1831,  1832,  1833,  1834,  1835,  1836,  1837,
    1838,  1839,  1840,  1841,  1842,  1843,  1844,  1845,  1846,  1847,
       0,     0,     0,  1848,  1849,   216,   217,   218,   219,   220,
       0,  1850,  1851,  1852,  1853,  1854,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1855,  1856,  1857,
       0,     0,     0,    93,    94,     0,    95,   190,    97,  1858,
       0,  1859,  1860,     0,  1861,     0,     0,     0,     0,     0,
       0,  1862,     0,  1863,     0,  1864,     0,  1865,  1866,     0,
     279,   280,   108,   281,   282,  1163,     0,   283,   284,   285,
     286,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1164,   287,   288,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,     0,
       0,     0,     0,     0,   290,     0,     0,     0,     0,     0,
       0,     0,  1187,     0,     0,     0,     0,     0,   292,   293,
     294,   295,   296,   297,   298,     0,     0,     0,   212,     0,
       0,     0,     0,     0,   299,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,   279,   280,
       0,   281,   282,     0,   602,   283,   284,   285,   286,     0,
      93,    94,     0,    95,   190,    97,   345,     0,   346,     0,
       0,   347,     0,   287,   288,     0,     0,     0,     0,   349,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   108,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   290,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   292,   293,   294,   295,
     296,   297,   298,     0,     0,     0,   212,     0,     0,     0,
       0,     0,   299,     0,     0,     0,     0,     0,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,    50,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,     0,   335,     0,     0,   337,   338,
     339,     0,     0,     0,   340,   600,   216,   217,   218,   219,
     220,   601,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   602,     0,
       0,     0,     0,     0,    93,    94,     0,    95,   190,    97,
     345,     0,   346,     0,     0,   347,   474,   475,   476,     0,
       0,     0,     0,   349,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   108,     0,     0,   477,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,   474,   475,   476,     0,     0,     0,
       0,     0,     0,     0,     0,   503,     0,     0,     0,     0,
       0,     0,     0,     0,   477,   478,  1530,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,     0,     0,     0,     0,
       0,     0,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,     0,     0,     0,     0,     0,     0,     0,
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,     0,  1727,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   503,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   474,   475,   476,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   477,   478,  1531,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,   474,   475,   476,     0,     0,     0,     0,     0,     0,
       0,     0,   503,     0,     0,     0,     0,     0,     0,     0,
       0,   477,   478,   504,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,   474,
     475,   476,     0,     0,     0,     0,     0,     0,     0,     0,
     503,     0,     0,     0,     0,     0,     0,     0,     0,   477,
     478,   586,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,     0,   474,   475,
     476,     0,     0,     0,     0,     0,     0,     0,   503,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   477,   478,
       0,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,   588,   502,   474,   475,   476,     0,
       0,     0,     0,     0,     0,     0,     0,   503,   289,     0,
       0,     0,     0,     0,     0,     0,   477,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,   607,   502,     0,   291,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   503,   289,     0,   212,     0,
       0,     0,     0,     0,  1538,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
     611,     0,     0,   291,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   289,     0,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   589,   216,   217,
     218,   219,   220,   590,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,   596,   825,     0,
     189,   291,     0,    91,   344,     0,    93,    94,     0,    95,
     190,    97,   289, -1131,   212,     0,     0,     0,     0,     0,
    1001,     0,     0,     0,   348,   589,   216,   217,   218,   219,
     220,   590,     0,     0,     0,   108,   350,    50,     0,     0,
       0,     0,     0,     0,     0,     0,   850,     0,   189,   291,
       0,    91,   344,     0,    93,    94,     0,    95,   190,    97,
       0,     0,   212,     0,     0,     0,     0,     0,  1460,     0,
       0,     0,   348,   589,   216,   217,   218,   219,   220,   590,
       0,     0,     0,   108,   350,    50,     0,     0,     0,     0,
       0,     0,  1388,     0,     0,     0,   189,     0,     0,    91,
     344,     0,    93,    94,     0,    95,   190,    97,     0,     0,
     880,   881,     0,     0,     0,     0,   882,     0,   883,     0,
     348,   589,   216,   217,   218,   219,   220,   590,     0,     0,
     884,   108,   350,     0,     0,     0,     0,     0,    34,    35,
      36,   212,     0,     0,   189,     0,     0,    91,   344,     0,
      93,    94,   214,    95,   190,    97,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,   348,     0,
       0,     0,     0,     0,  1115,     0,     0,     0,     0,   108,
     350,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,
     885,   886,   887,   888,   889,   890,    29,    81,    82,    83,
      84,    85,     0,  1187,    34,    35,    36,   212,   221,   213,
      40,     0,     0,   189,    89,    90,    91,    92,   214,    93,
      94,     0,    95,   190,    97,     0,     0,     0,    99,     0,
      50,     0,     0,     0,     0,     0,     0,   891,   892,     0,
       0,     0,     0,   105,     0,     0,     0,   215,   108,   893,
       0,     0,   880,   881,     0,     0,     0,     0,   882,     0,
     883,     0,     0,     0,     0,  1116,    75,   216,   217,   218,
     219,   220,   884,    81,    82,    83,    84,    85,     0,     0,
      34,    35,    36,   212,   221,     0,     0,     0,     0,   189,
      89,    90,    91,    92,   214,    93,    94,     0,    95,   190,
      97,     0,     0,     0,    99,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   105,
       0,     0,     0,     0,   108,   222,     0,     0,  1066,  1067,
       0,   112,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   885,   886,   887,   888,   889,   890,  1068,    81,
      82,    83,    84,    85,     0,     0,  1069,  1070,  1071,   212,
     221,     0,     0,     0,     0,   189,    89,    90,    91,    92,
    1072,    93,    94,     0,    95,   190,    97,     0,     0,     0,
      99,     0,    50,     0,     0,     0,     0,     0,     0,   891,
     892,     0,     0,     0,     0,   105,     0,     0,     0,     0,
     108,   893,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1073,  1074,
    1075,  1076,  1077,  1078,    29,     0,     0,     0,     0,     0,
       0,     0,    34,    35,    36,   212,  1079,   213,    40,     0,
       0,   189,     0,     0,    91,    92,   214,    93,    94,     0,
      95,   190,    97,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,  1080,  1081,     0,     0,     0,
       0,     0,     0,     0,     0,   215,   108,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,  1186,    75,   216,   217,   218,   219,   220,
      29,    81,    82,    83,    84,    85,     0,  1187,    34,    35,
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
       0,     0,     0,     0,     0,     0,   215, -1132, -1132, -1132,
   -1132,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,  1186,     0,    75,   216,   217,   218,   219,
     220,    29,    81,    82,    83,    84,    85,  1187,     0,    34,
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
       0,     0,     0,     0,     0,     0,     0,     0,  1164,  1670,
       0,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,    34,    35,    36,   212,     0,   213,    40,
       0,     0,     0,     0,     0,     0,  1187,   214,     0,     0,
       0,     0,     0,     0,     0,  1766,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   243,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   244,     0,     0,
       0,     0,     0,     0,     0,     0,   216,   217,   218,   219,
     220,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,   221,     0,  1768,     0,     0,   189,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   190,    97,
       0,     0,     0,    99,     0,    34,    35,    36,   212,     0,
     213,    40,     0,     0,     0,     0,     0,     0,   105,   678,
       0,     0,     0,   108,   245,     0,     0,     0,     0,     0,
     112,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   215, -1132,
   -1132, -1132, -1132,   489,   490,   491,   492,   493,   494,   495,
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
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   477,
     478,   503,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   478,   503,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1164,     0,   503,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1187,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   503,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
    1186,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1187,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1187,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   503,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,  1186,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1187,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     503
};

static const yytype_int16 yycheck[] =
{
       5,     6,   193,     8,     9,    10,    11,    12,    13,    56,
      15,    16,    17,    18,   109,     4,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     4,    31,   132,   734,   167,
       4,   572,   109,   109,   417,   124,    44,   172,   173,    44,
      33,   696,   853,     4,   417,   124,   417,    52,   417,    54,
      98,   878,    57,    46,    59,   103,   104,   692,    51,   508,
     124,     4,     4,   997,   537,   538,    30,   608,   723,   999,
     167,    30,   791,   502,    30,     4,    30,    57,    86,   693,
     246,    86,  1427,   131,   448,   560,   541,   861,   987,   194,
     505,   506,   534,   566,   671,    57,   191,   109,  1019,   534,
    1021,   868,  1114,  1902,   109,   257,   837,   621,   622,    48,
       9,    32,     9,     9,   191,   191,     9,  1034,    14,     9,
      32,  1238,     9,   258,   539,     9,   568,   222,     9,    48,
       9,    48,    48,   568,     9,  1052,     9,     9,     9,    14,
       9,    14,     9,     9,    38,   222,   222,     9,     9,     9,
     245,     9,    36,   132,     9,     9,  1228,     9,     9,     9,
       9,   450,   451,   452,     9,    91,    48,    91,   245,    83,
      83,   103,    50,    51,  1001,    83,  1110,   367,   368,   369,
       6,   371,   565,  1100,   162,    70,   191,   136,   137,    83,
     162,    38,    83,   198,   112,    83,   116,   168,    81,    38,
      57,    38,    70,   121,   122,   123,   124,   125,   126,   136,
     137,   183,    69,     0,    54,   194,   162,   222,   162,    50,
      51,   199,    48,   103,    38,   183,    70,  1579,    70,    38,
     202,   202,   199,   159,   166,   159,    83,    70,   167,   183,
     245,   199,   112,    70,    83,    70,    83,   199,   162,   132,
     402,   136,   137,    54,   203,   260,   202,  1156,   263,    70,
     180,   200,   270,   176,  2063,   270,   271,     8,   176,    83,
     461,   926,   199,    70,    83,   264,   194,   196,  2077,   268,
     196,   202,   176,   200,   200,   176,   166,   113,   176,   201,
      70,   997,    70,   119,   389,   121,   122,   123,   124,   125,
     126,   127,   201,   202,   201,   201,   184,   200,   183,    70,
     200,   358,   389,   389,   201,    70,   200,   858,   200,    70,
     201,  1242,   201,  1029,  1055,  1337,   201,   167,   201,   201,
     201,   199,   201,    60,   201,   201,  1681,   176,  1690,   201,
     201,   201,   200,   169,   170,   200,   172,   201,   200,   200,
     200,   200,   543,   448,   359,   200,  1473,  1124,   202,  1126,
     202,    88,   176,  1715,    91,  1717,   167,   176,   194,   202,
      70,   448,   448,  1290,   199,   136,   137,   203,    70,    70,
     388,   182,   162,   388,   389,    19,    20,    83,   199,   200,
     395,   396,   397,   398,   399,   400,    70,   199,   975,   446,
     405,   202,   199,  1475,  1110,   202,   122,   455,   922,   923,
    1482,   516,  1484,   418,    70,  1314,   132,  1031,  1245,   199,
     425,   199,   202,   183,   202,    70,    70,  1961,   433,  1963,
      83,    84,    70,    70,    70,    70,   136,   137,   183,   199,
     445,   202,  1514,   448,   136,   137,    70,   202,   199,    70,
     424,   202,   199,   432,   199,    83,    84,   199,   463,   464,
     107,   108,   162,   511,   512,   513,   514,    83,   448,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   952,   503,   199,
     505,   506,   507,   432,  1621,   517,  1623,  1226,   199,   205,
     202,   202,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   162,   162,   162,   202,  1356,
     107,   108,   537,   538,   539,   540,   515,   517,   502,   544,
      14,   990,   547,   502,   103,   103,   502,   699,   502,   701,
      14,   556,   205,   558,     4,   517,   203,   202,   202,  1333,
    1632,   566,   199,  1005,   202,   202,   202,   202,    32,   574,
    1005,   576,   710,   550,   202,   183,  1049,   205,   202,   195,
    1121,   202,   183,   168,   679,   199,   202,    51,   199,    83,
     579,   199,    83,  1309,   574,  1311,   181,    91,   199,  1315,
      91,   565,   793,    53,   987,   239,    56,   166,   166,  1244,
     202,  1956,    83,   710,   987,    83,   987,   202,   987,   785,
      91,  1738,   627,    73,    83,  1742,   203,   199,  1057,   103,
     104,    83,    91,  1460,   199,    19,    20,    31,   508,    91,
     367,   368,   369,   370,   371,  1153,    96,  1155,    98,  1157,
     199,   817,   199,   103,   104,  1019,    50,  1021,  1557,    53,
    1274,  1103,   199,  1277,   534,    83,   160,   161,  1103,   160,
     161,    83,    70,    91,   679,    70,   653,   654,    70,   831,
     832,   131,   409,    70,   554,   166,   838,   839,  1202,   160,
     161,   159,   160,   161,   742,   565,   123,     4,   568,    70,
     159,   160,   161,   838,   681,   132,   167,   159,   160,   161,
    1159,   199,    19,    20,   208,    83,   721,   136,   137,   693,
     201,   202,   912,    91,   914,   162,   916,   199,   894,   166,
     920,  1020,    83,    84,  1023,  1024,   207,   162,    32,   716,
      31,   907,   160,   161,   378,   750,   183,    31,   160,   161,
     166,  1650,    87,   387,   199,   734,   199,   207,   183,    50,
     394,    87,    53,   118,  1483,   121,   122,   123,   124,   125,
     126,    38,   406,  1156,   199,    75,    76,   202,   783,    75,
      76,   710,   202,  1156,  1901,  1156,   201,  1156,  1905,   124,
     125,   126,   160,   161,   106,   107,   108,    81,   124,   125,
     126,   201,   109,   162,   781,   734,    70,   812,    92,    70,
    1283,   367,   368,   369,   264,   371,  1451,  1292,   268,   201,
     104,  1294,   272,   201,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,   201,   194,   201,
     845,   201,  1257,   103,   104,   201,   134,   135,    78,    79,
      80,   201,   202,   861,   201,   239,   861,   141,   142,   143,
     144,   145,    92,  1477,   857,   106,   107,   108,    70,    59,
      60,   398,   851,   400,  1415,  1594,    70,   866,  1242,   163,
     199,  1336,   166,    70,   191,   169,   170,    70,   172,   173,
     174,   418,   176,   870,   621,   622,    53,    54,    55,   876,
      57,  1933,  1934,  1609,   113,   114,   115,   541,   358,    53,
      54,    55,    69,   199,   198,   222,    70,   147,   148,   149,
     150,   151,   851,   201,   202,    69,  1929,  1930,   158,    70,
     909,  1314,   239,  2050,   164,   165,  1004,  1005,   245,    70,
     162,  1314,   199,  1314,    49,  1314,   136,   137,   178,   166,
    2067,   956,   403,   958,   201,   960,   407,   264,    69,   183,
     199,   268,   162,   193,   199,     9,   971,   204,   199,   162,
     162,   948,  1691,     8,   201,  1516,  1431,   162,   199,   202,
     909,   986,   432,    14,   435,   974,   437,   438,   439,   440,
     162,  1532,   442,   201,   378,   974,   446,   201,     9,    14,
     974,   183,   201,   387,   132,   455,  1479,   132,  1056,   200,
     394,  1016,    14,   974,  1019,   103,  1021,   206,   997,  1633,
     200,  1026,   406,   200,   200,   999,   200,   202,   199,   112,
     199,   974,   974,   417,   199,  1040,     9,   159,  1043,  1019,
    1045,  1021,   200,    95,  1049,   974,  1026,   200,   200,   200,
    1029,   121,   122,   123,   124,   125,   126,  1031,   508,   509,
     510,   511,   512,   513,   514,  1999,     9,   201,   997,    14,
    2004,   378,    50,    51,    52,    53,    54,    55,   199,    57,
     387,   183,   389,     9,   534,   719,  1252,   394,  1452,  1255,
    2020,    69,   199,  1057,  1099,  2029,   202,   201,  1057,   406,
    1029,  1057,   202,  1057,   554,   201,    83,   202,   201,   200,
     202,   201,  2042,  1654,  1107,   200,   200,  1094,   568,  1108,
     990,  2051,  1663,   201,   194,   432,   134,   199,   204,   579,
    1579,  1110,   200,  1112,  1004,  1005,     9,     9,  1679,   204,
     204,   448,  1953,   204,  1617,  1618,   204,  1958,    70,   599,
      32,   135,   182,  1242,     9,   162,  2090,   541,   138,   200,
     794,  1138,   162,  1242,   200,  1142,  1455,  1456,  1457,    14,
     196,     9,  1149,     9,  1557,   625,   626,   200,  1242,   184,
       9,  1110,    14,  1112,  1557,   912,  1557,   914,  1557,   916,
       9,   200,   134,   920,  2005,   922,   923,   924,   204,   833,
     204,   835,   200,   200,   200,   204,     9,  1219,   203,    14,
     660,   661,   200,   200,  1219,   204,   199,   103,  1759,   200,
       9,   162,   201,   138,   162,  1230,  2053,   201,     9,   863,
    2057,  1101,   200,  1103,   541,   199,  1225,  1242,  1687,  1219,
      70,  1690,    70,    70,  2071,  2072,  1225,    70,    70,   199,
     199,  1225,  1257,     9,   202,  1260,   203,   827,     9,    14,
     184,   201,  1242,   202,  1225,    14,  1715,  1650,  1717,  2080,
     204,   202,   579,   176,  1723,    14,   201,  1650,  1283,  1650,
     200,  1650,  1225,  1225,   734,   196,    32,    70,    32,  1294,
    1295,   199,   742,  1999,   199,    14,  1225,  1952,  2004,  1954,
    1274,   199,   936,  1277,   199,   827,    14,    50,    51,    52,
      53,    54,    55,    70,    52,   199,    70,    70,   952,   953,
      70,  1456,    70,  2029,   199,  1333,    69,  1304,  1333,   199,
       9,   162,  1325,   200,   138,   719,   201,   201,  1343,  1328,
     199,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    14,   162,   138,  1452,   184,     9,
     200,  1340,   176,   176,    69,   204,     9,    83,  1909,   203,
     203,     9,   138,    83,    14,  1452,  1452,   827,   203,   829,
      83,   201,   203,   199,  2090,  2040,    59,    60,   199,   201,
    1367,   200,   202,   199,  1371,     9,   138,   199,     6,  1376,
     200,   851,    92,   202,   202,   201,    32,  1384,   202,   204,
     794,  1340,   719,    77,   159,   865,   866,   121,   122,   123,
     124,   125,   126,   201,   200,   184,   201,   734,   132,   133,
    1435,   138,    32,   200,   204,   200,     9,  1442,     9,   138,
      48,   204,  1447,   204,  1449,     9,   204,  1452,     9,   833,
      83,   835,   204,   200,  1903,  1429,    14,  1595,     9,   909,
    1465,   201,   203,   136,   137,  1439,   200,   200,   918,   919,
     201,   175,  1452,  1478,  1479,  1202,   201,   201,   201,   863,
    1114,  1115,   203,   200,   202,  1590,   199,   794,   199,   204,
     194,   200,   204,   138,   200,     9,   138,   199,  1068,   949,
    1429,   201,     9,  1477,   202,   113,   200,   200,   204,   204,
    1439,   119,   204,   121,   122,   123,   124,   125,   126,   127,
     204,  1498,  1713,   200,   974,  1502,   833,   200,   835,   200,
      32,   201,  1509,   200,   200,    19,    20,  2078,   201,   201,
     990,   138,   176,   202,   851,   113,  1068,   997,   171,    83,
     201,   167,   936,    83,  1004,  1005,   863,    14,    83,   866,
     119,   169,   170,   200,   172,   200,   202,    14,   952,   953,
     200,   138,   138,   183,   202,    14,   201,    14,    14,  1029,
      83,   200,   200,   200,   199,   198,   194,    83,   138,  1568,
     138,    14,   201,    14,   201,   203,   201,     9,   202,     9,
     203,    68,   909,   987,  1609,    14,  1056,   183,   199,  1614,
    1589,    83,  1617,  1618,     9,     9,  1066,  1067,  1068,   202,
     201,   116,   103,   162,  1258,   103,   200,   184,   174,   936,
    1609,    36,   201,   180,   199,   184,  1684,   199,    83,  1568,
     124,   184,   177,     9,   200,   952,   953,   201,  1622,    83,
      83,  1101,    14,  1103,  1628,   200,  1630,    83,  1108,  1633,
    1110,   202,  1112,   200,    83,    14,  1595,   974,    83,    14,
      83,  1660,    14,  1202,  2032,    83,  1655,   514,  1050,  1653,
    1609,  2047,   977,  1133,  1334,  1664,  1758,  1321,  1529,  2042,
     997,   509,  1259,  1622,   511,  1745,  1887,   629,  1783,  1628,
    1692,  1630,  1336,  1337,  2088,  1870,  1741,  2064,  1585,  1159,
    1714,   399,  1019,  1154,  1021,  1310,  1581,  1229,  1150,  1067,
    1306,  1305,  1029,  1096,  1653,  1946,  1655,  1732,  2000,   879,
    1114,  1115,   446,  1991,   395,  1664,  1306,  1307,  1188,  1573,
    1310,  1210,  1721,  1134,  1188,    -1,    -1,  1317,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   239,    78,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1757,  1758,
      -1,    -1,  1156,  1747,    -1,  1225,    -1,  1756,    -1,    -1,
      -1,    -1,   104,  1762,  1306,  1307,  1308,  1309,  1310,  1311,
    1769,    -1,    -1,  1315,    -1,  1317,    -1,  1431,    -1,    -1,
      -1,  1108,    -1,  1110,    -1,  1112,    -1,  1114,  1115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1747,   141,
     142,   143,   144,   145,    -1,    -1,    -1,  1756,    -1,    -1,
      -1,    -1,    -1,  1762,    -1,  1882,    -1,    -1,    -1,    -1,
    1769,    -1,    -1,     6,    19,    20,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    30,  1306,  1307,  1308,  1309,
    1310,  1311,    -1,    -1,    -1,  1315,    -1,  1317,    -1,    -1,
      -1,    -1,    -1,    -1,  1258,  2013,   198,    -1,  1328,    -1,
      -1,    56,    -1,    -1,    -1,    48,    -1,  1892,    -1,    -1,
    1340,    -1,    -1,    -1,   378,    -1,    -1,    -1,    -1,  1946,
    1350,    -1,    -1,   387,    -1,   389,    -1,    -1,    -1,    -1,
     394,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1225,    -1,
      -1,  1491,   406,  1493,    -1,    -1,    -1,    -1,    -1,    -1,
    1314,  2036,    -1,    -1,  1913,  1242,    -1,  1321,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1923,  1924,    -1,    -1,
     113,  1258,  1336,  1337,    -1,    -1,   119,    -1,   121,   122,
     123,   124,   125,   126,   127,    -1,    -1,    -1,    -1,  1491,
      -1,  1493,    -1,  1423,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1913,    -1,    -1,  1966,    -1,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,   169,   170,    -1,   172,
      -1,    -1,    -1,    -1,  1321,    -1,    -1,    -1,    -1,    -1,
    1999,  1328,    -1,    -1,    -1,  2004,    -1,    -1,    -1,  1336,
    1337,   194,    -1,  1340,    -1,    59,    60,  1966,    -1,    -1,
     203,  1491,    31,  1493,    -1,    -1,  2020,  1431,    -1,    -1,
    2029,    -1,    -1,    -1,    -1,    -1,    -1,   541,    -1,    -1,
      -1,    -1,    -1,    -1,   239,    -1,    -1,    -1,  2042,  1639,
    1999,  1641,    -1,  1643,    -1,  2004,    -1,  2051,  1648,    81,
      -1,  2086,    -1,    -1,  2013,    -1,    -1,    -1,    -1,    -1,
    2095,    -1,    81,    -1,    -1,    -1,    -1,    -1,  2103,    -1,
    2029,  2106,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    2089,  2090,   136,   137,   289,   104,   291,  1639,  1568,  1641,
      -1,  1643,    -1,    -1,  1431,    -1,  1648,    -1,    -1,  1579,
      -1,    -1,    -1,    -1,    -1,  1585,    -1,    -1,   127,   141,
     142,   143,   144,   145,    -1,  1452,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,    -1,  1609,
    2089,  2090,     6,    -1,   166,    -1,    -1,   169,   170,    -1,
     172,   173,   174,  1557,   163,   350,   200,   166,   167,  1749,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,  1639,
      -1,  1641,    -1,  1643,    -1,    -1,   198,    -1,  1648,    -1,
     202,    -1,    -1,   378,    48,  1655,    -1,    -1,    -1,   198,
    1660,    -1,   387,    -1,  1664,    -1,    -1,    -1,    -1,   394,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1749,    -1,    -1,
      -1,   406,    -1,    -1,  1684,   719,    -1,  1687,    -1,    -1,
    1690,    -1,   417,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1700,    -1,    -1,    -1,    -1,    -1,    -1,  1707,    -1,    -1,
      -1,  1568,    -1,    -1,    -1,  1715,  1650,  1717,   443,   113,
      -1,   446,    -1,  1723,    -1,   119,     6,   121,   122,   123,
     124,   125,   126,   127,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,  1749,
      -1,    -1,  1609,    -1,    -1,    -1,  1756,  1757,  1758,    -1,
     794,    -1,  1762,    81,    -1,    83,    84,    -1,    48,  1769,
      -1,    -1,    -1,    -1,    -1,   169,   170,   502,   172,    -1,
      59,    60,    -1,    -1,    -1,    -1,   104,  1907,  1908,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1655,   833,
     194,   835,    -1,  1660,    -1,    -1,    -1,  1664,    -1,   203,
      -1,    -1,    -1,    -1,    -1,    -1,   541,    -1,    -1,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,    -1,   863,
      -1,    -1,    -1,   113,     6,  1907,  1908,    -1,    -1,   119,
      -1,   121,   122,   123,   124,   125,   126,   127,    -1,   167,
      -1,   169,   170,    -1,   172,   173,   174,   136,   137,    -1,
      -1,    -1,     6,    -1,    -1,    -1,    -1,    -1,   593,    -1,
     595,    -1,    -1,   598,    -1,    31,    48,    -1,    -1,    -1,
     198,    -1,  1882,    -1,   202,    -1,    -1,   205,    -1,   169,
     170,    -1,   172,    -1,    -1,    -1,    -1,    -1,    -1,  1756,
    1757,  1758,   936,  1903,    48,  1762,   631,  1907,  1908,    -1,
      -1,    -1,  1769,  1913,   194,    -1,    -1,    -1,   952,   953,
      -1,   200,  1922,   203,    -1,    81,    -1,    -1,    -1,  1929,
    1930,    -1,    -1,  1933,  1934,    -1,    92,    -1,    -1,    -1,
      -1,   113,    -1,    -1,    -1,    -1,  1946,   119,   104,   121,
     122,   123,   124,   125,   126,   127,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   689,   690,  1966,    -1,    -1,   113,
      -1,    -1,    -1,   698,    -1,   119,    -1,   121,   122,   123,
     124,   125,   126,   127,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,   719,    -1,    -1,   169,   170,  1999,
     172,    -1,    -1,    -1,  2004,    -1,    -1,   163,    -1,    -1,
     166,    -1,  2012,   169,   170,    -1,   172,   173,   174,    -1,
     176,    -1,   194,    -1,    -1,   169,   170,    -1,   172,  2029,
      -1,   203,    -1,    -1,    -1,  2035,    -1,    -1,    -1,    -1,
      -1,    -1,   198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     194,    -1,    -1,    -1,    -1,    -1,  1913,    -1,    -1,   203,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   794,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1114,  1115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2089,
    2090,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   827,    -1,    -1,    -1,    -1,    -1,   833,  1966,
     835,    -1,    31,    -1,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,   863,   864,
      -1,    -1,  1999,    -1,    -1,    -1,   871,  2004,    -1,    -1,
      69,    -1,    -1,   878,   879,   880,   881,   882,   883,   884,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   893,    -1,
      -1,    -1,  2029,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,   910,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   104,    -1,    -1,    -1,    -1,    -1,  1242,    -1,
      -1,   936,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1258,   950,    -1,   952,   953,    -1,
      -1,    -1,  2089,  2090,    59,    60,    -1,    -1,    -1,   141,
     142,   143,   144,   145,    -1,   121,   122,   123,   124,   125,
     126,   976,   977,    -1,    19,    20,   132,   133,    -1,    -1,
      -1,   163,   987,    -1,   166,    30,    -1,   169,   170,   994,
     172,   173,   174,    -1,    -1,    -1,  1001,    -1,    -1,    -1,
      -1,   200,  1007,    -1,    -1,    -1,    56,  1321,    -1,    -1,
      -1,    -1,    -1,  1018,    -1,    -1,   198,   173,    -1,   175,
      -1,   203,  1336,  1337,    -1,    -1,    -1,    -1,  1033,    -1,
      -1,   136,   137,   189,    -1,   191,    -1,    -1,   194,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,  1053,    -1,
      -1,    -1,  1057,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,  1068,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1114,
    1115,    -1,    -1,    -1,    -1,    -1,    -1,  1431,    -1,    -1,
      -1,    -1,    -1,  1128,    -1,    -1,    -1,  1132,    -1,  1134,
      -1,    -1,  1137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1150,  1151,  1152,  1153,  1154,
    1155,  1156,  1157,    -1,    -1,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,  1187,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   239,    -1,    -1,    -1,    -1,    -1,
      -1,  1206,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    10,    11,    12,    -1,   204,    -1,    -1,  1243,   289,
    1245,   291,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,  1258,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,  1286,    -1,    -1,  1289,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     350,  1306,  1307,  1308,  1309,  1310,  1311,    -1,    -1,  1314,
    1315,    -1,  1317,    -1,    -1,    -1,  1321,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,  1336,  1337,   378,  1339,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   387,    69,  1349,    -1,    -1,    -1,    -1,   394,
      -1,  1356,    -1,    -1,    -1,    -1,  1361,    -1,  1363,    -1,
      -1,   406,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   417,    -1,   201,    -1,   203,    -1,    -1,    -1,
      -1,    -1,    -1,  1388,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   443,    -1,    -1,   446,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1424,
    1425,    -1,    -1,  1428,   203,    -1,  1431,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,  1460,    56,   502,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,  1491,    -1,  1493,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   541,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,  1537,  1538,    -1,    -1,  1541,    69,    -1,    -1,
      -1,    -1,  1547,   593,  1549,   595,  1551,    -1,    -1,    -1,
      -1,  1556,  1557,   598,    -1,    -1,  1561,    -1,  1563,    -1,
      -1,  1566,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1579,  1580,    -1,    -1,  1583,    -1,
      -1,    -1,    10,    11,    12,  1590,   631,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     203,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    19,    20,    -1,  1639,    -1,  1641,    -1,  1643,   689,
     690,    69,    30,  1648,    -1,  1650,    -1,    -1,   698,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1665,    -1,    -1,    -1,    -1,  1670,    -1,    -1,    -1,    -1,
      -1,   203,    -1,    -1,   719,    -1,    -1,  1682,  1683,    -1,
      -1,    -1,    -1,    -1,    -1,  1690,    -1,  1692,    -1,   289,
      -1,   291,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1715,    -1,  1717,    -1,    -1,    -1,    30,    31,  1723,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,  1749,    -1,    -1,    -1,    -1,   794,
     350,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,  1766,  1767,  1768,    -1,    -1,    -1,    -1,  1773,    -1,
    1775,    -1,    -1,    -1,    -1,   203,  1781,    -1,  1783,    -1,
      -1,    -1,   827,    -1,    -1,    -1,    -1,    -1,   833,    -1,
     835,    -1,    -1,    -1,    81,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,   863,   864,
      -1,   871,    -1,    -1,    -1,    -1,    -1,    -1,   878,   879,
      -1,    -1,    -1,    -1,    -1,   880,   881,   882,   883,   884,
      -1,    59,    60,   443,    -1,    -1,   446,    -1,   893,    -1,
      -1,   239,    -1,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,    -1,    -1,    -1,   910,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,  1879,    -1,    -1,    -1,    92,    -1,
     167,  1886,   169,   170,   171,   172,   173,   174,    -1,   203,
     104,   936,    -1,    -1,  1899,    -1,    -1,    -1,  1903,    -1,
      -1,    -1,  1907,  1908,    -1,   950,    -1,   952,   953,    -1,
      -1,   198,   199,    -1,    -1,    -1,    -1,  1922,   136,   137,
      -1,    -1,    -1,  1928,    -1,    -1,   976,   141,   142,   143,
     144,   145,   977,    -1,    -1,    -1,    -1,    -1,    -1,  1944,
      -1,    -1,   987,  1948,   994,    -1,    -1,    -1,    -1,   163,
      -1,  1001,   166,    -1,    -1,   169,   170,  1007,   172,   173,
     174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1975,    81,    -1,  1018,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   200,    -1,   198,  1990,    -1,    -1,  1033,    -1,
     378,    -1,    -1,   593,   104,  2000,    -1,  2002,    -1,   387,
      -1,    -1,    -1,  1053,    -1,    -1,   394,    -1,    -1,    -1,
      -1,    -1,  1057,    -1,    -1,    -1,    -1,   127,   406,    -1,
      -1,    -1,    -1,  1068,    -1,    -1,  2031,    -1,    -1,   417,
     140,   141,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,  2047,    -1,    -1,    -1,    -1,    -1,  2053,    -1,
      -1,    -1,  2057,   163,    -1,    -1,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,  2071,  2072,    -1,  1114,
    1115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1128,    -1,
      -1,    -1,  1132,    -1,  1134,    -1,    -1,  1137,   198,   689,
     690,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1150,  1151,  1152,  1153,  1154,
    1155,  1156,  1157,    -1,   502,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,  1187,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   541,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1206,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,  1243,    -1,  1245,    81,    -1,    83,    84,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     598,    -1,    -1,  1258,    -1,    -1,    -1,    -1,    -1,   104,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,  1286,    -1,    -1,  1289,
      -1,    -1,    -1,   631,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,  1306,  1307,  1308,  1309,  1310,  1311,    -1,    -1,  1314,
    1315,    -1,  1317,    -1,    -1,    -1,  1321,    -1,   878,   879,
      -1,    -1,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,  1336,  1337,    -1,  1339,    -1,    -1,    -1,    -1,  1349,
      -1,    -1,    -1,    -1,    -1,    -1,  1356,    -1,    -1,    -1,
      -1,    -1,    -1,   198,    -1,    -1,  1361,   202,  1363,    -1,
     205,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   719,    -1,    -1,    -1,    -1,   203,    -1,    -1,    -1,
      -1,    30,    31,  1388,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,  1424,  1425,   976,    -1,    -1,    -1,
      69,    -1,    -1,  1428,    -1,    -1,  1431,    -1,    -1,    -1,
      -1,    10,    11,    12,   994,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   794,  1007,    -1,    -1,
    1460,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,   833,  1491,   835,  1493,    -1,
      69,    -1,    -1,  1053,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   863,   864,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1537,  1538,    -1,
      -1,  1541,   880,   881,   882,   883,   884,    -1,    -1,    81,
      59,    60,  1547,    -1,  1549,   893,  1551,    -1,    -1,    -1,
      -1,  1556,  1557,    -1,   203,    -1,  1561,    -1,  1563,    -1,
      -1,  1566,   104,    -1,    -1,    -1,    -1,    -1,  1128,  1579,
      -1,    -1,  1132,    -1,    -1,  1580,    -1,  1137,  1583,    31,
    1590,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   936,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   952,   953,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,   136,   137,    -1,
      -1,    -1,    -1,    -1,   203,    -1,    -1,   169,   170,    81,
     172,   173,   174,    -1,  1639,    87,  1641,    -1,  1643,   987,
      -1,    -1,    -1,  1648,    -1,  1650,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,  1665,   198,   199,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1670,    -1,   598,    -1,    -1,
    1018,    -1,    -1,    -1,    -1,    -1,    -1,  1682,  1683,    -1,
    1690,    -1,    -1,  1243,    -1,    -1,    -1,  1692,   140,   141,
     142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,
     631,    -1,    -1,    -1,    -1,  1715,    -1,  1717,    -1,  1057,
      -1,   163,    -1,  1723,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,   176,    -1,  1286,    -1,    -1,  1289,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1749,    -1,   198,   199,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,  1766,  1767,  1768,    -1,    -1,  1114,  1115,  1773,    -1,
    1775,  1781,    -1,    -1,    -1,    -1,    -1,    -1,  1783,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1349,
      -1,    -1,    -1,    59,    60,    -1,  1356,    -1,    -1,    -1,
      -1,    -1,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,
      -1,    -1,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1206,    -1,
      -1,    -1,    -1,    -1,  1424,  1425,    -1,    -1,    -1,    -1,
     136,   137,    -1,    -1,  1879,    -1,  1886,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1899,
      -1,    -1,    -1,  1903,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1907,  1908,    -1,    -1,    -1,    -1,    81,    -1,
    1258,    -1,  1922,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1928,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,   864,    -1,    -1,    -1,    -1,    -1,  1944,
      -1,    -1,    -1,  1948,    -1,    -1,    -1,    -1,    -1,   880,
     881,   882,   883,   884,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,   893,    -1,    -1,  1975,  1314,    -1,   141,   142,
     143,   144,   145,  1321,    -1,    -1,    -1,  1537,  1538,    -1,
      -1,  1541,    -1,    -1,    -1,  1990,    -1,    -1,  1336,  1337,
    2000,  1339,  2002,    -1,   167,    68,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,  1361,    -1,  1363,    -1,    -1,    81,  1579,
      -1,    -1,    -1,    -1,    -1,   198,  2031,    -1,    -1,    -1,
    1590,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
    1388,   104,  2047,  2053,    -1,    -1,    -1,  2057,   121,   122,
     123,   124,   125,   126,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  2071,  2072,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,   146,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,  1431,    -1,    -1,    -1,  1018,    -1,    -1,
     163,    -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,  1665,   169,   170,    -1,   172,
     173,   174,    -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,
      -1,   194,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,
    1690,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,  1715,    -1,  1717,    -1,    -1,
      -1,    30,    31,  1723,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1547,
      69,  1549,    81,  1551,    -1,    -1,    -1,    -1,  1556,  1557,
      -1,    -1,    -1,  1561,    -1,  1563,    -1,    -1,  1566,  1150,
    1151,  1781,    -1,  1154,    -1,   104,    -1,    -1,    -1,  1160,
    1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1206,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,
      -1,    -1,  1650,    -1,    -1,    -1,    -1,    -1,   187,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,
     199,    -1,  1670,    -1,    -1,    -1,  1886,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   203,    -1,    -1,    -1,    -1,  1899,
      -1,    -1,    -1,  1903,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,     3,     4,     5,     6,     7,  1339,    -1,
      -1,    -1,    69,    13,    -1,  1975,    -1,    -1,  1766,  1767,
    1768,    -1,    -1,    -1,    -1,  1773,    -1,    27,    28,    29,
    1361,    -1,  1363,    -1,  1782,    -1,    -1,    -1,    -1,    -1,
    2000,    -1,  2002,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,  1388,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,  1879,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,   203,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
    1928,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,
      -1,    -1,    -1,   193,   194,   195,  1944,    -1,   198,   199,
    1948,   201,   202,   203,    -1,   205,   206,   207,   208,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1547,    -1,  1549,    -1,
    1551,    -1,    -1,    -1,    -1,  1556,    -1,    -1,    -1,  1977,
    1561,    -1,  1563,    -1,    -1,  1566,    -1,    -1,    -1,    -1,
      -1,    -1,  1990,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,  1670,
      88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,
      -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,
      -1,   109,   110,   111,   112,   113,   114,   115,    -1,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,   129,   130,   131,   132,   133,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    -1,    -1,    81,   155,    -1,    -1,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,   104,
     178,    -1,    -1,   181,    -1,  1766,  1767,  1768,    -1,   187,
     188,   189,  1773,   191,    -1,   193,   194,   195,    -1,    -1,
     198,   199,    -1,   201,   202,   130,    -1,   205,   206,   207,
     208,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   198,   199,    -1,    -1,    48,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,  1879,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,
      -1,    92,    93,    94,    95,    -1,    97,    -1,    99,    -1,
     101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,
     111,   112,   113,   114,   115,    -1,   117,  1928,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,   129,   130,
     131,   132,   133,  1944,    -1,    -1,    -1,  1948,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,  1990,
     181,    -1,    -1,    -1,    -1,    -1,   187,   188,   189,    -1,
     191,    -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,
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
     105,    -1,    -1,    -1,   109,   110,   111,   112,   113,   114,
     115,    -1,   117,    -1,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,   131,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,   188,   189,    -1,   191,    -1,   193,   194,
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
      -1,   114,   115,    -1,   117,   118,    -1,   120,   121,   122,
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
      93,    94,    95,    96,    97,    -1,    99,    -1,   101,    -1,
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
      97,    -1,    99,    -1,   101,   102,    -1,   104,   105,    -1,
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
      -1,    -1,    77,    78,    79,    80,    81,    82,    83,    84,
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
      93,    94,    95,    -1,    97,    -1,    99,   100,   101,    -1,
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
      97,    98,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,
     195,    -1,    -1,   198,   199,     3,     4,     5,     6,     7,
     205,   206,   207,   208,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   181,     3,     4,     5,     6,     7,   187,   188,
      -1,    -1,    -1,    13,   193,   194,   195,    -1,    -1,   198,
     199,    -1,    -1,    -1,    -1,    -1,   205,   206,   207,   208,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   121,   122,   123,   124,   125,   126,    -1,    -1,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,   176,    -1,   178,    -1,
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
     173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,    -1,
      -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,
     193,   194,   195,    -1,    -1,   198,   199,    -1,    -1,   202,
      -1,    -1,   205,   206,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
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
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,   109,    -1,
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
     194,   195,    -1,    -1,   198,   199,    -1,   201,    -1,    -1,
      -1,   205,   206,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
      -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,
     199,   200,    -1,    -1,    -1,    -1,   205,   206,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   205,   206,   207,   208,    -1,    -1,    32,    -1,
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
     178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,
     188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,
     198,   199,     3,     4,     5,     6,     7,   205,   206,   207,
     208,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,
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
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,
     123,   124,   125,   126,    -1,    -1,   129,   130,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,     3,     4,   178,     6,     7,   181,    -1,
      10,    11,    12,    13,   187,   188,    -1,    -1,    -1,    -1,
     193,   194,   195,    -1,    -1,   198,   199,    -1,    28,    29,
      -1,    -1,   205,   206,   207,   208,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    57,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    83,    84,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    81,
     130,    -1,   132,   133,   134,   135,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,   163,    10,    11,    12,    13,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,   177,    -1,    -1,
     180,    -1,    28,    29,    -1,    -1,    -1,    -1,   188,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,   198,    -1,
      -1,    -1,   202,    -1,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    83,    84,    -1,
      -1,    87,    -1,    -1,    -1,    -1,   198,    93,    94,    95,
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
      -1,   163,    -1,    -1,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,   104,   177,    -1,    -1,   180,    -1,
       3,     4,    -1,     6,     7,   187,   188,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,
      -1,   203,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   163,    57,    -1,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    68,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,   198,    -1,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,    -1,   130,    -1,    -1,
     133,   134,   135,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    83,    -1,    85,    -1,    -1,    -1,    -1,    -1,
     163,    -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,   175,   104,   177,    -1,    -1,   180,    -1,     3,
       4,    -1,     6,     7,   187,   188,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,
     203,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,    -1,
     141,   142,   143,   144,   145,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    57,    57,    -1,    -1,    -1,   169,   170,
      -1,   172,   173,   174,    68,    -1,    69,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    87,    -1,    -1,    -1,   198,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    -1,   130,    -1,   132,   133,
     134,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,
      -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,   175,   104,   177,    -1,    -1,   180,    -1,     3,     4,
      -1,     6,     7,   187,   188,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    29,    -1,    31,    -1,    -1,   141,
     142,   143,   144,   145,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    -1,   166,    -1,    -1,   169,   170,    -1,
     172,   173,   174,    68,    -1,    69,    71,    72,    73,    74,
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
     175,   104,   177,    -1,    -1,   180,    -1,    -1,    -1,   112,
     113,    -1,   187,   188,   189,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,   198,   199,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,    28,    29,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,   172,
     173,   174,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,   198,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    -1,   130,    -1,    -1,   133,   134,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,   104,   177,    -1,    -1,   180,    -1,     3,     4,     5,
       6,     7,   187,   188,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,    -1,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    57,    57,    -1,    -1,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    69,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    87,    -1,    -1,    -1,   198,    -1,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,   164,   165,
      -1,    -1,    -1,   169,   170,    -1,   172,   173,   174,   175,
      -1,   177,   178,    -1,   180,    -1,    -1,    -1,    -1,    -1,
      -1,   187,    -1,   189,    -1,   191,    -1,   193,   194,    -1,
       3,     4,   198,     6,     7,    12,    -1,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    28,    29,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,   163,    10,    11,    12,    13,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,   177,    -1,
      -1,   180,    -1,    28,    29,    -1,    -1,    -1,    -1,   188,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    -1,   130,    -1,    -1,   133,   134,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,
      -1,    -1,    -1,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,   177,    -1,    -1,   180,    10,    11,    12,    -1,
      -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   198,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    32,    33,    34,    35,
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
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,   203,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,   201,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,   201,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,   201,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,   201,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   201,    57,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    31,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     201,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   200,    -1,
     163,    68,    -1,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    31,   176,    81,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,   187,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,   198,   199,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,   163,    68,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,
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
      -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    50,    51,
      -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,   145,    70,   147,
     148,   149,   150,   151,    -1,    -1,    78,    79,    80,    81,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      92,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
     178,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,   187,
     188,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,
     198,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,   158,    83,    84,    -1,
      -1,   163,    -1,    -1,   166,   167,    92,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   121,   198,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,   140,   141,   142,   143,   144,   145,
      70,   147,   148,   149,   150,   151,    -1,    69,    78,    79,
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
      -1,    -1,    -1,    -1,    -1,    -1,   121,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,   140,   141,   142,   143,   144,
     145,    70,   147,   148,   149,   150,   151,    69,    -1,    78,
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
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    69,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    69,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,    69,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69
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
     119,   122,   127,   169,   170,   172,   203,   312,   335,   336,
     337,   342,   343,   344,   345,   456,   482,   359,   203,   202,
     203,    54,   359,   203,   359,   359,   372,   468,   201,   202,
     523,    38,    83,   176,    14,    83,   359,   199,   199,   204,
     509,   200,   311,   200,   300,   359,   303,   200,   311,   495,
     311,   201,   202,   199,   200,   444,   444,   200,   204,     9,
     438,   138,   204,     9,   438,   204,   204,   204,   138,   200,
       9,   438,   200,   311,    32,   233,   201,   200,   200,   200,
     241,   201,   201,   293,   233,   138,   522,   522,   176,   522,
     138,   432,   432,   432,   432,   373,   432,   432,   432,   202,
     203,   512,   134,   135,   189,   217,   498,   522,   283,   423,
     113,   345,    31,   127,   140,   146,   167,   173,   319,   320,
     321,   322,   423,   171,   327,   328,   130,   199,   216,   329,
     330,    83,   341,   257,   522,     9,   201,     9,   201,   201,
     495,   336,   200,   308,   167,   414,   203,   203,   359,    83,
      83,   176,    14,    83,   359,   303,   303,   119,   362,   509,
     203,   509,   200,   200,   203,   202,   203,   311,   300,   138,
     444,   444,   444,   444,   373,   203,   233,   239,   242,    32,
     236,   287,   233,   522,   200,   432,   138,   138,   138,   233,
     423,   423,   500,    14,   217,     9,   201,   202,   498,   495,
     322,   183,   202,     9,   201,     3,     4,     5,     6,     7,
      10,    11,    12,    13,    27,    28,    29,    57,    71,    72,
      73,    74,    75,    76,    77,    87,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   139,   140,
     147,   148,   149,   150,   151,   163,   164,   165,   175,   177,
     178,   180,   187,   189,   191,   193,   194,   216,   420,   421,
       9,   201,   167,   171,   216,   330,   331,   332,   201,    14,
       9,   201,   256,   341,   498,   498,    14,   257,   203,   309,
     310,   498,    14,    83,   359,   200,   200,   199,   509,   198,
     506,   362,   509,   308,   203,   200,   444,   138,   138,    32,
     236,   286,   287,   233,   432,   432,   432,   203,   201,   201,
     432,   423,   315,   522,   323,   324,   431,   320,    14,    32,
      51,   325,   328,     9,    36,   200,    31,    50,    53,   432,
      83,   218,   499,   201,    14,   522,   256,   201,    14,   359,
      38,    83,   411,   202,   507,   508,   522,   201,   202,   333,
     509,   506,   203,   509,   444,   444,   233,   100,   252,   203,
     216,   229,   316,   317,   318,     9,   438,     9,   438,   203,
     432,   421,   421,    68,   326,   331,   331,    31,    50,    53,
      14,   183,   199,   432,   499,   432,    83,     9,   439,   231,
       9,   439,    14,   510,   231,   202,   333,   333,    98,   201,
     116,   243,   162,   103,   522,   184,   431,   174,   432,   511,
     313,   199,    38,    83,   200,   203,   508,   522,   203,   231,
     201,   199,   180,   255,   216,   336,   337,   184,   184,   298,
     299,   457,   314,    83,   203,   423,   253,   177,   216,   201,
     200,     9,   439,    87,   124,   125,   126,   339,   340,   298,
      83,   283,   201,   509,   457,   523,   523,   200,   200,   201,
     506,    87,   339,    83,    38,    83,   176,   509,   202,   201,
     202,   334,   523,   523,    83,   176,    14,    83,   506,   233,
     231,    83,    38,    83,   176,    14,    83,   359,   334,   203,
     203,    83,   176,    14,    83,   359,    14,    83,   359,   359
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
     469,   470,   470,   470,   470,   470,   470,   470,   470,   471,
     471,   471,   471,   471,   471,   471,   471,   471,   472,   473,
     473,   474,   474,   474,   475,   475,   475,   476,   477,   477,
     477,   478,   478,   478,   478,   479,   479,   480,   480,   480,
     480,   480,   480,   481,   481,   481,   481,   481,   482,   482,
     482,   482,   482,   482,   483,   483,   484,   484,   484,   484,
     484,   484,   484,   484,   485,   485,   486,   486,   486,   486,
     487,   487,   488,   488,   488,   488,   489,   489,   489,   489,
     490,   490,   490,   490,   490,   490,   491,   491,   491,   492,
     492,   492,   492,   492,   492,   492,   492,   492,   492,   492,
     493,   493,   494,   494,   495,   495,   496,   496,   496,   496,
     497,   497,   498,   498,   499,   499,   500,   500,   501,   501,
     502,   502,   503,   504,   504,   504,   504,   504,   504,   505,
     505,   505,   505,   506,   506,   507,   507,   508,   508,   509,
     509,   510,   510,   511,   512,   512,   513,   513,   513,   513,
     514,   514,   514,   515,   515,   515,   515,   516,   516,   517,
     517,   517,   517,   518,   519,   520,   520,   521,   521,   522,
     522,   522,   522,   522,   522,   522,   522,   522,   522,   522,
     523,   523
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
       3,     1,     1,     1,     1,     3,     1,     4,     3,     1,
       1,     1,     1,     1,     3,     3,     4,     4,     3,     1,
       1,     7,     9,     9,     7,     6,     8,     1,     4,     4,
       1,     1,     1,     4,     2,     1,     0,     1,     1,     1,
       3,     3,     3,     0,     1,     1,     3,     3,     2,     3,
       6,     0,     1,     4,     2,     0,     5,     3,     3,     1,
       6,     4,     4,     2,     2,     0,     5,     3,     3,     1,
       2,     0,     5,     3,     3,     1,     2,     2,     1,     2,
       1,     4,     3,     3,     6,     3,     1,     1,     1,     4,
       4,     4,     4,     4,     4,     2,     2,     4,     2,     2,
       1,     3,     3,     3,     0,     2,     5,     6,     6,     7,
       1,     2,     1,     2,     1,     4,     1,     4,     3,     0,
       1,     3,     2,     1,     2,     4,     3,     3,     1,     4,
       2,     2,     0,     0,     3,     1,     3,     3,     2,     0,
       2,     2,     2,     2,     1,     2,     4,     2,     5,     3,
       1,     1,     0,     3,     4,     5,     6,     3,     1,     3,
       2,     1,     0,     4,     1,     3,     2,     4,     5,     2,
       2,     1,     1,     1,     1,     3,     2,     1,     8,     6,
       1,     0
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
#line 7334 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 763 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 7342 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 770 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 7348 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 771 "hphp.y" /* yacc.c:1646  */
    { }
#line 7354 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 7360 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 775 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7366 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7372 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7378 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7384 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 779 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 7390 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 7398 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 783 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 7405 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 785 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 7411 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 786 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7417 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 787 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 7423 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 788 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7429 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 789 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7437 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 793 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7446 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 798 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7455 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 803 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7462 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 806 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7469 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 809 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7477 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 813 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 817 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 821 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7501 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 825 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7509 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 828 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7516 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7522 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7528 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7534 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7540 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7546 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7552 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7558 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 840 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7564 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7570 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 842 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7576 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 843 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7582 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 844 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7588 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 845 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 927 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7600 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 929 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7606 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 934 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7612 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 935 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7619 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 941 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7625 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 945 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7631 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 946 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7637 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 948 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7643 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 950 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7649 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 955 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7655 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 956 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 962 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 966 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7675 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 968 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7682 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 970 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7689 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 975 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7695 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 977 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7701 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 980 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7707 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 982 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7713 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 983 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7719 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 988 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 995 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7737 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1003 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7744 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1006 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7751 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1012 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 7757 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1013 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval)); }
#line 7763 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1018 "hphp.y" /* yacc.c:1646  */
    {
                                         _p->onUsing((yyval), (yyvsp[-2]), true, (yyvsp[-1]), nullptr);
                                       }
#line 7771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1025 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7777 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1026 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7783 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1031 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 7789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1032 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval), (yyval), (yyvsp[0])); }
#line 7796 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1037 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1039 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7814 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1040 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7820 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7826 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1047 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7832 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1052 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7838 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1053 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7845 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7853 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1059 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7860 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1062 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1066 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1068 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7883 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1071 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7890 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1073 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7898 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7904 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1077 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7910 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1078 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7916 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1079 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7922 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1080 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7928 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1081 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7934 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1082 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7940 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7946 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7952 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1085 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7958 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7964 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1087 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7970 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1088 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7976 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1089 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7982 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1090 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1094 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7996 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1096 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 8004 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1101 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 8011 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1103 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 8019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1107 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 8027 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1116 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 8033 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1117 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8039 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1120 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false); }
#line 8045 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1121 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 8051 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1123 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8059 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1127 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8067 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1131 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8075 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1135 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8083 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1139 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1143 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1148 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8107 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1151 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 8113 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 8122 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1156 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8128 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1157 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8134 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1158 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8140 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1159 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8146 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1160 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8152 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1161 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8158 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1162 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 8164 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1163 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8170 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1164 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8176 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1165 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 8182 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1166 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8188 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1167 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 8198 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1189 "hphp.y" /* yacc.c:1646  */
    { _p->pushLabelScope(LS::Using);
                                         _p->onNewLabelScope(false);
                                         (yyval) = (yyvsp[-1]); }
#line 8206 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1195 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1; }
#line 8212 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1196 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8218 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1205 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), nullptr, (yyvsp[-2]));
                                         _p->onExprListElem((yyval), &(yyval), (yyvsp[0])); }
#line 8225 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1207 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0])); }
#line 8231 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1217 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8237 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1218 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8243 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1222 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false); }
#line 8249 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1223 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1232 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 8261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1233 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8267 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1237 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::Finally);}
#line 8274 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1239 "hphp.y" /* yacc.c:1646  */
    { _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->popLabelScope();
                                         _p->onCompleteLabelScope(false);}
#line 8282 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1245 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8288 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1246 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8294 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1250 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 8300 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1251 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8306 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1255 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 8312 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1261 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8321 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1268 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1276 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8340 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1283 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8350 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1291 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8359 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1297 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8369 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1306 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8376 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1310 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 8382 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1314 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8389 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1318 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 8395 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1324 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8402 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 8420 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1342 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8427 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 8445 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1359 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8452 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1362 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1367 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8467 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1370 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8475 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1376 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 8481 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1379 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 8487 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1383 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8494 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1386 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1394 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8512 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1397 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8523 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1405 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8529 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1406 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 8536 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1410 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8542 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1413 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8548 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1416 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 8554 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1417 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 8560 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1418 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 8568 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1421 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 8574 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1422 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 8580 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1426 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8586 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1427 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8592 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1430 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8598 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1431 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8604 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1434 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8610 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1435 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8616 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1438 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1440 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1443 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8634 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1445 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8640 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8646 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1450 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8652 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8658 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8664 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1455 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8670 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8676 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8682 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8688 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8694 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1469 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8700 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1471 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8706 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1474 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8712 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1476 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8718 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1480 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8724 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1482 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8731 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8737 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8743 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8749 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8755 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1495 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8761 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1497 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8767 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1498 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8773 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1501 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8779 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1502 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8785 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1507 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8791 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1508 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8797 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1513 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8803 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1514 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8809 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8815 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1518 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8821 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1521 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8827 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1522 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1530 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8840 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1536 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8847 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1542 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8855 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1546 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8861 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1550 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1555 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1560 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8883 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1563 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1569 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8897 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1574 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1579 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8913 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1585 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8921 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1591 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1597 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8937 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1603 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8945 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1609 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1616 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8961 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1623 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8969 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1632 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8976 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1637 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1642 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8991 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1646 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8997 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1649 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 9004 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1653 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 9011 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1657 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 9019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1660 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1665 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9033 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1669 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9041 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1673 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1678 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9057 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1683 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9065 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1688 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1693 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9081 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1698 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9089 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1704 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1710 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9105 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1716 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 9111 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1717 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 9117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1718 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 9123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1724 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1727 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,false);}
#line 9142 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1729 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::InOut,false);}
#line 9149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1731 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::Ref,false);}
#line 9156 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1733 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,true);}
#line 9163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1736 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::In,false);}
#line 9170 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1739 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::In,true);}
#line 9177 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1742 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::Ref,false);}
#line 9184 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1745 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::InOut,false);}
#line 9191 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1750 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9197 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1751 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 9203 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1754 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9209 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1755 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 9215 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1756 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 9221 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1760 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9227 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1762 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9233 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1763 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 9239 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1764 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9245 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1769 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9251 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1770 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9257 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1773 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9264 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1778 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9270 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1784 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9276 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1785 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9282 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1789 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-2]),(yyvsp[-1]),NULL);}
#line 9289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1793 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-2]));}
#line 9296 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1795 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9303 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1798 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 9310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1800 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9316 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1803 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1810 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9334 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1818 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9342 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1825 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9352 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1831 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 9358 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1833 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9364 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1835 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9370 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1837 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 9376 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1839 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 9382 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1840 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 9389 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1843 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 9395 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9401 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1847 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9407 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1848 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 9413 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1854 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 9419 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1859 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 9426 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1862 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 9434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1869 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 9440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1870 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 9447 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1875 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 9454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1878 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 9460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1885 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 9467 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1887 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 9473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1891 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1896 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1898 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1900 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9497 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1901 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 9508 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1907 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 9514 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1909 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 9520 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1910 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 9526 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1914 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval),nullptr,(yyvsp[0])); }
#line 9532 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1916 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval),&(yyvsp[-2]),(yyvsp[0])); }
#line 9538 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1921 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9544 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1924 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9550 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1925 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 9556 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1929 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 9562 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1930 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 9568 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1934 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 9575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1937 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 9582 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1942 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 9589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1947 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 9595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1948 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 9602 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 9608 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1954 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 9614 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1955 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 9620 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 9626 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 9632 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1961 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9638 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 9644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 9650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 9656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 9662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1967 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 9668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1969 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 9674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9682 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9688 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1977 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9694 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1981 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9700 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9706 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9712 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1987 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9718 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1990 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9724 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9730 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9736 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1995 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9742 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1998 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9748 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9754 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9760 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9766 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9772 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 2006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9778 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9784 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 2008 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9790 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 2009 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9796 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9814 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9820 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 2019 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9826 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 2023 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9832 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 2025 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9838 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 2026 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9844 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 2031 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9856 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 2033 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9862 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 2037 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 2039 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9874 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 2043 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9882 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 2047 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 2051 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 2055 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9901 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 2059 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9907 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 2061 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9913 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 2062 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 2063 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9925 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 2064 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9931 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 2065 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9937 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 2068 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9943 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 2069 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9949 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 2073 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9955 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 2074 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9961 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 2078 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9967 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 2079 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9973 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 2080 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9979 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 2081 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9985 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 2085 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9991 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 2090 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9997 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 2094 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 10003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 2098 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 2102 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 10015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 2106 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 2111 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 10027 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 2115 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10033 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 2119 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10039 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 2120 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10045 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 2121 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10051 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 2122 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10057 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 2123 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10063 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 2127 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10069 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 2132 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 10075 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 2133 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 10081 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 2134 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 10087 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 2137 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 10093 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 2138 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 10099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 2139 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 10105 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 2140 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 10111 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2141 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 10117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2142 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 10123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2143 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 10129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2144 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 10135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2145 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 10141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2146 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 10147 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2147 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 10153 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 10159 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2149 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 10165 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2150 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 10171 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2151 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 10177 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2152 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 10183 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2153 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 10189 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2154 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 10195 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2155 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 10201 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2156 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 10207 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2157 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 10213 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2158 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 10219 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2159 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 10225 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2160 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 10231 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2161 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 10237 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2162 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 10243 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2163 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 10249 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2164 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 10255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2165 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 10261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2166 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 10267 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2167 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 10273 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2168 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 10279 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2169 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 10285 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2170 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 10291 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2171 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 10297 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2172 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 10303 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2173 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 10309 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2174 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 10315 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2175 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 10321 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2176 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 10327 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2177 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 10333 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2178 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 10339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2179 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 10345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2180 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 10351 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2181 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 10358 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2183 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 10364 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2184 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 10371 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2186 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 10377 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2188 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 10383 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2189 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10389 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2190 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 10395 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2191 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 10401 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2192 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 10407 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2193 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10413 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2194 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 10419 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2195 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 10425 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2196 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 10431 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2197 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 10437 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2198 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 10443 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2199 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 10449 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2200 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 10455 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2201 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 10461 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2202 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 10467 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2203 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2204 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2205 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2206 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2207 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10497 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10503 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2209 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10509 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2210 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2211 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10521 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2212 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 10527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2213 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 10533 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2214 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2221 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 10545 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2222 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2227 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10560 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10572 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2242 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10581 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10593 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10607 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2279 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10632 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10647 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2298 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10657 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10674 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10688 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10701 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2333 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10711 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10724 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2352 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10730 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2353 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10736 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2355 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10742 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2359 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10749 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2361 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10755 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2368 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10761 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2371 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10767 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2378 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10773 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2381 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10779 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2386 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10785 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2387 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10791 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2392 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10797 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2393 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10803 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2397 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval), (yyvsp[-1]));}
#line 10809 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2401 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10815 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2402 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10821 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2407 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10827 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2408 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2413 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10839 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2414 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10845 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2419 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10851 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2420 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10857 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2426 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2428 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10869 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2433 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2434 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10881 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2440 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2442 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10893 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2446 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2450 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2454 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2458 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2462 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2466 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2470 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2474 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2478 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2482 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2486 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2490 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10965 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2494 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2498 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2502 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2506 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2510 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2514 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2519 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2520 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2525 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2526 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2531 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2532 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2537 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11045 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2544 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11053 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2551 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11059 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2553 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11065 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2557 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11071 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2558 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2559 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11083 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2560 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11089 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2561 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11095 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2562 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11101 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2563 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11107 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2564 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11113 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2565 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11119 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2566 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 11126 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2568 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11132 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2569 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11138 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11144 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2574 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 11150 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2575 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 11156 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2576 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 11162 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2583 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 11168 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 11186 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 11204 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2616 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 11210 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2617 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 11216 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2620 "hphp.y" /* yacc.c:1646  */
    { _p->onXhpAttributesStart(); (yyval).reset();}
#line 11222 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2622 "hphp.y" /* yacc.c:1646  */
    { _p->onXhpAttributeSpread((yyval), &(yyvsp[-4]), (yyvsp[-1]));}
#line 11228 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2625 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11234 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2628 "hphp.y" /* yacc.c:1646  */
    { _p->onOptExprListElem((yyval), &(yyvsp[-1]), (yyvsp[0])); }
#line 11240 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2629 "hphp.y" /* yacc.c:1646  */
    {  (yyval).reset();}
#line 11246 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2632 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11267 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 11279 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2649 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11285 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11291 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2654 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11297 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2656 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 11303 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2658 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 11309 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2662 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11315 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11321 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11327 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2665 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11333 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2666 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2667 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11351 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2669 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11357 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11363 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2671 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11369 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2672 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11375 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2673 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11381 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2674 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11387 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2675 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11393 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2676 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11399 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2677 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11405 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2678 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11411 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2679 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11417 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2680 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11423 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2681 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11429 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2682 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11435 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2683 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11441 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2684 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11447 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2685 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11453 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11459 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2687 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11465 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2688 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11471 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2689 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11477 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2690 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11483 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2691 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11489 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11495 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2693 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11501 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2694 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11507 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2695 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11513 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2696 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11519 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2697 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11525 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2698 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11531 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2699 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11537 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2700 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11543 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2701 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11549 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2702 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11555 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2703 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11561 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2704 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11567 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2705 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11573 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2706 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11579 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2707 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11585 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2708 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11591 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2709 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11597 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2710 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11603 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2711 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11609 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11615 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2713 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11621 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2714 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11627 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2715 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11633 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2716 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11639 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2717 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11645 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11651 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11657 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11663 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11669 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2722 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11675 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11681 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2724 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11687 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11693 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2726 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11699 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2727 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11705 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2728 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11711 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11717 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11723 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2731 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11729 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2732 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2733 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11741 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11747 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2735 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11753 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11759 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2737 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11765 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11777 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11783 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2742 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2743 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11801 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2744 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11807 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2749 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11813 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2753 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11819 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2754 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2758 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11831 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2759 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11837 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2760 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11843 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2761 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2763 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11857 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2767 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2776 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11869 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2779 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2780 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11882 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2782 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2792 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2796 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11901 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2797 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11907 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2798 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11913 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2802 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2803 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11925 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2804 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11931 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2808 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11937 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2809 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11943 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2810 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11949 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2814 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11955 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2815 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11961 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2819 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11967 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2820 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11973 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2821 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11979 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2822 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11986 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2824 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11992 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2825 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11998 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2826 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 12004 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2827 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 12010 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2828 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 12016 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2829 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 12022 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2830 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 12028 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2831 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 12034 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2832 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 12040 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2835 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12046 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2837 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12052 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12058 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2842 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12064 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2844 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12070 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2845 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12076 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2847 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1]));}
#line 12082 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2848 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12088 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12094 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2850 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12100 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2851 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12106 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2852 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12112 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2853 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12118 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2854 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12124 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2855 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12130 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2856 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12136 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2858 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 12142 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2860 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 12148 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2862 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 12154 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2864 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 12160 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2866 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 12166 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 12172 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2868 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 12178 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2869 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 12184 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2870 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 12190 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2871 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 12196 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2872 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 12202 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2873 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 12208 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2874 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 12214 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2875 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 12220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2876 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 12226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2877 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 12232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2878 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 12238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2879 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 12244 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2880 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 12250 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2881 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12256 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2882 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12262 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2884 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 12268 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2886 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 12274 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2888 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 12280 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2890 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 12286 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2891 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 12292 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2893 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 12299 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2895 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 12305 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2898 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 12312 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2902 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 12318 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2905 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 12324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2906 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 12330 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2910 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12336 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2911 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12342 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2917 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12348 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2923 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12354 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2924 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12360 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2928 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12366 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2929 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12372 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2930 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12378 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2931 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12384 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2932 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 12390 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2933 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 12396 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2935 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 12403 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2940 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12409 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2941 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2945 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12421 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2946 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12427 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2949 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 12433 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2950 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12439 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2956 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12445 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2958 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12451 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2960 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12457 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2961 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12463 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2965 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12469 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2966 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12475 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2967 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12481 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2970 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12487 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2972 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2975 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 12499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2976 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 12505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2977 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 12511 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2978 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12517 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2982 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 12524 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2985 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 12532 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2992 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12538 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2993 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12544 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2996 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 12552 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2999 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12558 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 3000 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12564 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 3001 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12570 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 3003 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12576 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 3004 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12582 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 3006 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1]));}
#line 12588 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 3007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 3008 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12600 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 3009 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12606 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 3010 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12612 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 3011 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12618 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 3012 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12624 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12630 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 3018 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12636 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 3023 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12642 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 3024 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12648 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 3029 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12654 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 3031 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12660 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 3033 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12666 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 3034 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12672 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3038 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12678 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3039 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12684 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3044 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12690 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3045 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12696 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3050 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12702 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3053 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12708 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3058 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12714 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3059 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12720 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3062 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12726 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3063 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 12733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3070 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 12739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3072 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 12745 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3075 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 12751 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3077 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12757 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3080 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12763 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3083 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12769 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3084 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12775 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3088 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12781 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3089 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12787 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3093 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12793 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3094 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12799 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3095 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12805 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3099 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12811 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3104 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12817 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3109 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12823 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3110 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12829 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3114 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12835 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3119 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12841 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3124 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12847 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3125 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12853 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3130 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12859 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3131 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12865 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3133 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12871 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3138 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12877 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3140 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12883 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 12897 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 12911 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 12925 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 12939 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3196 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12945 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3197 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12951 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3198 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12957 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3199 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12963 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3200 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12969 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3201 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12975 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 12989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3220 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3222 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3224 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3225 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3229 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3233 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3234 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3235 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3236 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13043 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 13057 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3253 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13063 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3255 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13069 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3259 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13075 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3264 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13081 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3265 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13087 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3266 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13093 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3267 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3268 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13105 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3269 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13111 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3270 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3272 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3274 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3278 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3282 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3283 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13147 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3289 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13153 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3293 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13159 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3297 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13165 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3304 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 13171 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3313 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 13177 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3317 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 13183 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3321 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13189 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3330 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13195 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3331 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13201 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3332 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13207 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3336 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13213 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3337 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 13219 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3338 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 13225 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3340 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]) = 1; _p->onIndirectRef((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 13231 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3345 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13237 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3346 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13243 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13249 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3358 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3359 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3362 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13275 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3373 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13281 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3374 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13287 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3378 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13293 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3379 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13299 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3382 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13313 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3391 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13319 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3395 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 13325 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3396 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 13331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3398 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 13337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3399 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 13343 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3400 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 13349 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3401 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 13355 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3406 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13361 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3407 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13367 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3411 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3412 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13379 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3413 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13385 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3414 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13391 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3417 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13397 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3419 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 13403 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3420 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13409 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3421 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 13415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3426 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13421 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3427 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13427 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3431 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13433 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3432 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13439 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3433 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13445 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3434 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13451 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3439 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13457 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3440 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13463 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3445 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13469 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3447 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13475 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3449 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13481 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3450 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13487 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3454 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 13493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3456 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 13499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3457 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 13505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3459 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 13512 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1030:
#line 3464 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13518 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1031:
#line 3466 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13524 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1032:
#line 3468 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 13538 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1033:
#line 3478 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 13544 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1034:
#line 3480 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 13550 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1035:
#line 3481 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13556 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1036:
#line 3484 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 13562 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 13568 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1038:
#line 3486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 13574 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1039:
#line 3490 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 13580 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3491 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 13586 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3492 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13592 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3493 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13598 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3494 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13604 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3495 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13610 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3496 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 13616 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3497 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 13622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3498 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 13628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3499 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 13634 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3500 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 13640 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3504 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 13646 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3505 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 13652 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3510 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13658 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3512 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13664 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3526 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13672 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3531 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 13680 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3535 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13688 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3540 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 13696 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3546 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13702 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3547 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13708 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3551 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13714 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3552 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13720 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3558 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13726 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3562 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 13732 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3568 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13738 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3572 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 13745 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3579 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13751 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3580 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13757 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3584 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 13765 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3587 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 13772 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3593 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13778 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3597 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13786 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3600 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13794 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3603 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-3]); }
#line 13801 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3605 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1077:
#line 3607 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13815 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1078:
#line 3609 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13821 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1079:
#line 3614 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]); }
#line 13827 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1080:
#line 3615 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1081:
#line 3616 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13839 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1082:
#line 3617 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13845 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1089:
#line 3638 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13851 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1090:
#line 3639 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13857 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1093:
#line 3648 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1096:
#line 3659 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13869 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1097:
#line 3661 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1098:
#line 3665 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13881 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1099:
#line 3668 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1100:
#line 3672 "hphp.y" /* yacc.c:1646  */
    {}
#line 13893 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1101:
#line 3673 "hphp.y" /* yacc.c:1646  */
    {}
#line 13899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1102:
#line 3674 "hphp.y" /* yacc.c:1646  */
    {}
#line 13905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1103:
#line 3680 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13912 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1104:
#line 3685 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13922 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1105:
#line 3694 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13928 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1106:
#line 3700 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13937 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1107:
#line 3708 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13943 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1108:
#line 3709 "hphp.y" /* yacc.c:1646  */
    { }
#line 13949 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1109:
#line 3715 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13955 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1110:
#line 3717 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13961 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1111:
#line 3718 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1112:
#line 3723 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13978 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1113:
#line 3729 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("HH\\darray"); }
#line 13985 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1114:
#line 3734 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13991 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1115:
#line 3739 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13999 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1116:
#line 3743 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14005 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1117:
#line 3748 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 14011 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1118:
#line 3750 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 14017 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1119:
#line 3756 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 14024 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1120:
#line 3758 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 14032 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1121:
#line 3761 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14038 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1122:
#line 3762 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14046 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1123:
#line 3765 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14054 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1124:
#line 3768 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14060 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1125:
#line 3771 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 14068 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1126:
#line 3774 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14075 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1127:
#line 3776 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 14084 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1128:
#line 3782 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 14093 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1129:
#line 3788 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("HH\\varray");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 14103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1130:
#line 3796 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1131:
#line 3797 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 14115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;


#line 14119 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 3800 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}
