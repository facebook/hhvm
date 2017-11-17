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
#define YYLAST   20174

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  209
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  317
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1133
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
    1777,  1783,  1785,  1788,  1788,  1792,  1791,  1795,  1797,  1800,
    1803,  1801,  1818,  1815,  1830,  1832,  1834,  1836,  1838,  1840,
    1842,  1846,  1847,  1848,  1851,  1857,  1861,  1867,  1870,  1875,
    1877,  1882,  1887,  1891,  1892,  1896,  1897,  1899,  1901,  1907,
    1908,  1910,  1914,  1915,  1920,  1924,  1925,  1929,  1930,  1934,
    1936,  1942,  1947,  1948,  1950,  1954,  1955,  1956,  1957,  1961,
    1962,  1963,  1964,  1965,  1966,  1968,  1973,  1976,  1977,  1981,
    1982,  1986,  1987,  1990,  1991,  1994,  1995,  1998,  1999,  2003,
    2004,  2005,  2006,  2007,  2008,  2009,  2013,  2014,  2017,  2018,
    2019,  2022,  2024,  2026,  2027,  2030,  2032,  2036,  2038,  2042,
    2046,  2050,  2055,  2059,  2060,  2062,  2063,  2064,  2065,  2068,
    2069,  2073,  2074,  2078,  2079,  2080,  2081,  2085,  2089,  2094,
    2098,  2102,  2106,  2110,  2115,  2119,  2120,  2121,  2122,  2123,
    2127,  2131,  2133,  2134,  2135,  2138,  2139,  2140,  2141,  2142,
    2143,  2144,  2145,  2146,  2147,  2148,  2149,  2150,  2151,  2152,
    2153,  2154,  2155,  2156,  2157,  2158,  2159,  2160,  2161,  2162,
    2163,  2164,  2165,  2166,  2167,  2168,  2169,  2170,  2171,  2172,
    2173,  2174,  2175,  2176,  2177,  2178,  2179,  2180,  2181,  2183,
    2184,  2186,  2187,  2189,  2190,  2191,  2192,  2193,  2194,  2195,
    2196,  2197,  2198,  2199,  2200,  2201,  2202,  2203,  2204,  2205,
    2206,  2207,  2208,  2209,  2210,  2211,  2212,  2213,  2214,  2218,
    2222,  2227,  2226,  2242,  2240,  2259,  2258,  2279,  2278,  2298,
    2297,  2316,  2316,  2333,  2333,  2352,  2353,  2354,  2359,  2361,
    2365,  2369,  2375,  2379,  2385,  2387,  2391,  2393,  2397,  2401,
    2402,  2406,  2408,  2412,  2414,  2418,  2420,  2424,  2427,  2432,
    2434,  2438,  2441,  2446,  2450,  2454,  2458,  2462,  2466,  2470,
    2474,  2478,  2482,  2486,  2490,  2494,  2498,  2502,  2506,  2510,
    2514,  2518,  2520,  2524,  2526,  2530,  2532,  2536,  2543,  2550,
    2552,  2557,  2558,  2559,  2560,  2561,  2562,  2563,  2564,  2565,
    2566,  2568,  2569,  2573,  2574,  2575,  2576,  2580,  2586,  2599,
    2616,  2617,  2620,  2621,  2623,  2628,  2629,  2632,  2636,  2639,
    2642,  2649,  2650,  2654,  2655,  2657,  2662,  2663,  2664,  2665,
    2666,  2667,  2668,  2669,  2670,  2671,  2672,  2673,  2674,  2675,
    2676,  2677,  2678,  2679,  2680,  2681,  2682,  2683,  2684,  2685,
    2686,  2687,  2688,  2689,  2690,  2691,  2692,  2693,  2694,  2695,
    2696,  2697,  2698,  2699,  2700,  2701,  2702,  2703,  2704,  2705,
    2706,  2707,  2708,  2709,  2710,  2711,  2712,  2713,  2714,  2715,
    2716,  2717,  2718,  2719,  2720,  2721,  2722,  2723,  2724,  2725,
    2726,  2727,  2728,  2729,  2730,  2731,  2732,  2733,  2734,  2735,
    2736,  2737,  2738,  2739,  2740,  2741,  2742,  2743,  2744,  2748,
    2753,  2754,  2758,  2759,  2760,  2761,  2763,  2767,  2768,  2779,
    2780,  2782,  2784,  2796,  2797,  2798,  2802,  2803,  2804,  2808,
    2809,  2810,  2813,  2815,  2819,  2820,  2821,  2822,  2824,  2825,
    2826,  2827,  2828,  2829,  2830,  2831,  2832,  2833,  2836,  2841,
    2842,  2843,  2845,  2846,  2848,  2849,  2850,  2851,  2852,  2853,
    2854,  2855,  2856,  2857,  2859,  2861,  2863,  2865,  2867,  2868,
    2869,  2870,  2871,  2872,  2873,  2874,  2875,  2876,  2877,  2878,
    2879,  2880,  2881,  2882,  2883,  2885,  2887,  2889,  2891,  2892,
    2895,  2896,  2900,  2904,  2906,  2910,  2911,  2915,  2921,  2924,
    2928,  2929,  2930,  2931,  2932,  2933,  2934,  2939,  2941,  2945,
    2946,  2949,  2950,  2954,  2957,  2959,  2961,  2965,  2966,  2967,
    2968,  2971,  2975,  2976,  2977,  2978,  2982,  2984,  2991,  2992,
    2993,  2994,  2999,  3000,  3001,  3002,  3004,  3005,  3007,  3008,
    3009,  3010,  3011,  3012,  3016,  3018,  3022,  3024,  3027,  3030,
    3032,  3034,  3037,  3039,  3043,  3045,  3048,  3051,  3057,  3059,
    3062,  3063,  3068,  3071,  3075,  3075,  3080,  3083,  3084,  3088,
    3089,  3093,  3094,  3095,  3099,  3104,  3109,  3110,  3114,  3119,
    3124,  3125,  3129,  3131,  3132,  3137,  3139,  3144,  3155,  3169,
    3181,  3196,  3197,  3198,  3199,  3200,  3201,  3202,  3212,  3221,
    3223,  3225,  3229,  3233,  3234,  3235,  3236,  3237,  3253,  3254,
    3257,  3264,  3265,  3266,  3267,  3268,  3269,  3270,  3271,  3273,
    3278,  3282,  3283,  3287,  3290,  3294,  3301,  3305,  3314,  3321,
    3329,  3331,  3332,  3336,  3337,  3338,  3340,  3345,  3346,  3357,
    3358,  3359,  3360,  3371,  3374,  3377,  3378,  3379,  3380,  3391,
    3395,  3396,  3397,  3399,  3400,  3401,  3405,  3407,  3410,  3412,
    3413,  3414,  3415,  3418,  3420,  3421,  3425,  3427,  3430,  3432,
    3433,  3434,  3438,  3440,  3443,  3446,  3448,  3450,  3454,  3455,
    3457,  3458,  3464,  3465,  3467,  3477,  3479,  3481,  3484,  3485,
    3486,  3490,  3491,  3492,  3493,  3494,  3495,  3496,  3497,  3498,
    3499,  3500,  3504,  3505,  3509,  3511,  3519,  3521,  3525,  3529,
    3534,  3538,  3546,  3547,  3551,  3552,  3558,  3559,  3568,  3569,
    3577,  3580,  3584,  3587,  3592,  3597,  3600,  3603,  3605,  3607,
    3609,  3613,  3615,  3616,  3617,  3620,  3622,  3628,  3629,  3633,
    3634,  3638,  3639,  3643,  3644,  3647,  3652,  3653,  3657,  3660,
    3662,  3666,  3672,  3673,  3674,  3678,  3682,  3690,  3695,  3707,
    3709,  3713,  3716,  3718,  3723,  3728,  3734,  3737,  3742,  3747,
    3749,  3756,  3758,  3761,  3762,  3765,  3768,  3769,  3774,  3776,
    3780,  3786,  3796,  3797
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
  "class_type_constant", "parenthesis_expr_with_parens",
  "expr_with_parens", "expr_list", "for_expr", "yield_expr",
  "yield_assign_expr", "yield_list_assign_expr", "yield_from_expr",
  "yield_from_assign_expr", "await_expr", "await_assign_expr",
  "await_list_assign_expr", "parenthesis_expr", "expr",
  "parenthesis_expr_no_variable", "expr_no_variable", "lambda_use_vars",
  "closure_expression", "$@30", "$@31", "lambda_expression", "$@32",
  "$@33", "$@34", "$@35", "$@36", "lambda_body", "shape_keyname",
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
  "non_empty_user_attribute_list", "user_attribute_list", "$@37",
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

#define YYPACT_NINF -1775

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1775)))

#define YYTABLE_NINF -1134

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1134)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1775,   214, -1775, -1775,  5335, 14918, 14918,     5, 14918, 14918,
   14918, 14918, 12519, 14918, -1775, 14918, 14918, 14918, 14918, 18262,
   18262, 14918, 14918, 14918, 14918, 14918, 14918, 14918, 14918, 12698,
   19060, 14918,    28,   262, -1775, -1775, -1775,   363, -1775,   416,
   -1775, -1775, -1775,   377, 14918, -1775,   262,   266,   269,   382,
   -1775,   262, 12877,  2617, 13056, -1775, 15936, 11382,   387, 14918,
   19309,    77,    79,    83,   287, -1775, -1775, -1775,   400,   421,
     446,   456, -1775,  2617,   470,   490,   527,   644,   688,   692,
     696, -1775, -1775, -1775, -1775, -1775, 14918,   500,  3526, -1775,
   -1775,  2617, -1775, -1775, -1775, -1775,  2617, -1775,  2617, -1775,
     606,   580,   582,  2617,  2617, -1775,   365, -1775, -1775, 13262,
   -1775, -1775,   354,   561,   590,   590, -1775,   754,   622,   481,
     592, -1775,   101, -1775,   605,   699,   787, -1775, -1775, -1775,
   -1775, 15318,   706, -1775,   179, -1775,   633,   645,   647,   649,
     653,   654,   656,   657, 17197, -1775, -1775, -1775, -1775, -1775,
     169,   775,   791,   792,   793,   794,   795, -1775,   798,   801,
   -1775,   176,   650, -1775,   705,    22, -1775,   797,   180, -1775,
   -1775,  4783,   179,   179,   671,   338, -1775,   187,    61,   674,
     202, -1775, -1775,   804, -1775,   717, -1775, -1775,   681,   715,
   -1775, 14918, -1775,   787,   706, 19597,  5199, 19597, 14918, 19597,
   19597, 19861, 19861,   689, 18435, 19597,   833,  2617,   820,   820,
     175,   820, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775,
   -1775,    89, 14918,   709, -1775, -1775,   732,   698,    71,   700,
      71,   820,   820,   820,   820,   820,   820,   820,   820, 18262,
   18483,   691,   889,   717, -1775, 14918,   709, -1775,   742, -1775,
     745,   702, -1775,   204, -1775, -1775, -1775,    71,   179, -1775,
   13441, -1775, -1775, 14918,  9949,   897,   111, 19597, 10979, -1775,
   14918, 14918,  2617, -1775, -1775, 17245,   707, -1775, 17315, -1775,
   -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, 17622,
   -1775, 17622, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775,
   -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775,
   -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775,
   -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775,
   -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775,
   -1775,   100,   123,   715, -1775, -1775, -1775, -1775,   712, -1775,
    2206,   124, -1775, -1775,   752,   913, -1775,   769,  4650, 14918,
   -1775,   731,   733, 17366, -1775,    65, 17414, 16377, 16377, 16377,
    2617, 16377,   734,   924,   738, -1775,    68, -1775, 17929,   125,
   -1775,   921,   126,   808, -1775,   811, -1775, 18262, 14918, 14918,
     744,   764, -1775, -1775, 18005, 12698, 14918, 14918, 14918, 14918,
   14918,   127,   440,   549, -1775, 15097, 18262,   533, -1775,  2617,
   -1775,   404,   622, -1775, -1775, -1775, -1775, 19162, 14918,   934,
     846, -1775, -1775, -1775,   134, 14918,   756,   757, 19597,   758,
    1376,   759,  6035, 14918, -1775,    82,   746,   596,    82,   496,
     467, -1775,  2617, 17622,   755, 11561, 15936, -1775, 13647,   760,
     760,   760,   760, -1775, -1775,  4477, -1775, -1775, -1775, -1775,
   -1775,   787, -1775, 14918, 14918, -1775, -1775, -1775, -1775, -1775,
   -1775, -1775, -1775, -1775, 14918, 14918, 14918, 14918, 13826, 14918,
   14918, 14918, 14918, 14918, 14918, 14918, 14918, 14918, 14918, 14918,
   14918, 14918, 14918, 14918, 14918, 14918, 14918, 14918, 14918, 14918,
   14918, 14918, 19238, 14918, -1775, 14918, 14918, 14918, 15270,  2617,
    2617,  2617,  2617,  2617, 15318,   848,   675, 11185, 14918, 14918,
   14918, 14918, 14918, 14918, 14918, 14918, 14918, 14918, 14918, 14918,
   -1775, -1775, -1775, -1775,  3796, -1775, -1775, 11561, 11561, 14918,
   14918, 18005,   772,   787, 14005, 17484, -1775, 14918, -1775,   776,
     954,   809,   774,   780, 15424,    71, 14184, -1775, 14363, -1775,
     702,   783,   784,  1513, -1775,   368, 11561, -1775,  4358, -1775,
   -1775, 17535, -1775, -1775, 11937, -1775, 14918, -1775,   887, 10155,
     978,   788, 19475,   979,   107,    90, -1775, -1775, -1775,   814,
   -1775, -1775, -1775, 17622, -1775,  2434,   803,   983, 17853,  2617,
   -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775,   805,
   -1775, -1775,   807,   799,   810,   806,   825,   829,    95,   830,
     832, 16187, 16725, -1775, -1775,  2617,  2617, 14918,    71,    77,
   -1775, 17853,   911, -1775, -1775, -1775,    71,   109,   144,   831,
     836,  1923,   226,   837,   840,   380,   865,   843,    71,   145,
     845, 18544,   802,   996,  1001,   839,   842,   849,   852, -1775,
   16008,  2617, -1775, -1775,   980,  3484,    51, -1775, -1775, -1775,
     622, -1775, -1775, -1775,  1017,   917,   875,   168,   896, 14918,
     922,  1052,   862, -1775,   901, -1775,   225, -1775,   866, 17622,
   17622,  1051,   897,   134, -1775,   871,  1063, -1775,  3921,   140,
   -1775,   459,   203, -1775, -1775, -1775, -1775, -1775, -1775, -1775,
    1187,  3536, -1775, -1775, -1775, -1775,  1065,   892, -1775, 18262,
     491, 14918,   877,  1069, 19597,  1067,   147,  1073,   890,   891,
     905, 19597,   906,  3010,  6241, -1775, -1775, -1775, -1775, -1775,
   -1775,   967,  5156, 19597,   904,  3733,  3780, 19780, 19861, 19898,
   14918, 19549, 19971,  3608, 20041, 20074, 16116, 19245, 20105, 20105,
   20105, 20105,  2169,  2169,  2169,  2169,  2169,  1194,  1194,   782,
     782,   782,   175,   175,   175, -1775,   820,   908,   909, 18592,
     907,  1100,     2, 14918,   364,   709,   215, -1775, -1775, -1775,
    1106,   846, -1775,   787, 18110, -1775, -1775, -1775, 19861, 19861,
   19861, 19861, 19861, 19861, 19861, 19861, 19861, 19861, 19861, 19861,
   19861, -1775, 14918,   381, -1775,   230, -1775,   709,   448,   914,
     923,   920,  4331,   158,   926, -1775, 19597,  4865, -1775,  2617,
   -1775,    71,    53, 18262, 19597, 18262, 18653,   967,   390,    71,
     233, -1775,   225,   965,   930, 14918, -1775,   234, -1775, -1775,
   -1775,  6447,   510, -1775, -1775, 19597, 19597,   262, -1775, -1775,
   -1775, 14918,  1028, 17777, 17853,  2617, 10361,   932,   933, -1775,
    1126, 15650,   999, -1775,   977, -1775,  1132,   944,  4126, 17622,
   17853, 17853, 17853, 17853, 17853,   948,  1079,  1080,  1083,  1084,
    1088,   961,   970, 17853,   503, -1775, -1775, -1775, -1775, -1775,
   -1775, -1775,    17, -1775, 19691, -1775, -1775,    43, -1775,  6653,
   15829,   969, 16725, -1775, 16725, -1775, 16725, -1775,  2617,  2617,
   16725, -1775, 16725, 16725,  2617, -1775,  1165,   972, -1775,   122,
   -1775, -1775,  4944, -1775, 19691,  1163, 18262,   981, -1775, -1775,
   -1775, -1775, -1775, -1775, -1775, -1775, -1775,   995,  1171,  2617,
   15829,   982, 18005, 18186,  1167, -1775, 14918, -1775, 14918, -1775,
   14918, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775,   988,
   -1775, 14918, -1775, -1775,  5623, -1775, 17622, 15829,   984, -1775,
   -1775, -1775, -1775,  1019,  1179,   998, 14918, 19162, -1775, -1775,
   15270, -1775,  1000, -1775, 17622, -1775,  1007,  6859,  1164,   120,
   -1775, 17622, -1775,   173,  3796,  3796, -1775, 17622, -1775, -1775,
      71, -1775, -1775,  1135, 19597, -1775, 11740, -1775, 17853, 13647,
     760, 13647, -1775,   760,   760, -1775, 12143, -1775, -1775,  7065,
   -1775,   115,  1009, 15829,   917, -1775, -1775, -1775, -1775, 19971,
   14918, -1775, -1775, 14918, -1775, 14918, -1775,  5091,  1013, 11561,
     865,  1181,   917, 17622,  1201,   967,  2617, 19238,    71,  5143,
    1018,   208,  1020, -1775, -1775,  1202,  3263,  3263,  4865, -1775,
   -1775, -1775,  1170,  1024,  1154,  1156,  1157,  1158,  1160,    92,
    1034,  1035,   623, -1775, -1775, -1775, -1775, -1775, -1775,  1076,
   -1775, -1775, -1775, -1775,  1226,  1039,   776,    71,    71, 14542,
     917,  4358, -1775,  4358, -1775,  5536,   620,   262, 10979, -1775,
    7271,  1041,  7477,  1049, 17777, 18262,  1054,  1116,    71, 19691,
    1242, -1775, -1775, -1775, -1775,   612, -1775,   359, 17622,  1078,
    1122,  1103, 17622,  2617,  4253, -1775, -1775, 17622,  1257,  1070,
    1107,  1109,  1065,   739,   739,  1219,  1219, 18810,  1095,  1291,
   17853, 17853, 17853, 17853, 17853, 17853, 19162, 17853,  3878, 16677,
   17853, 17853, 17853, 17853, 17701, 17853, 17853, 17853, 17853, 17853,
   17853, 17853, 17853, 17853, 17853, 17853, 17853, 17853, 17853, 17853,
   17853, 17853, 17853, 17853, 17853, 17853, 17853, 17853,  2617, -1775,
   -1775,  1218, -1775, -1775,  1099,  1101,  1104, -1775,  1114, -1775,
   -1775,   427, 16187, -1775,  1105, -1775, 17853,    71, -1775, -1775,
     149, -1775,   625,  1312, -1775, -1775,   161,  1127,    71, 12340,
   19597, 18701, -1775,  3041, -1775,  5829,   846,  1312, -1775,   261,
   14918,   244, -1775, 19597,  1189,  1130, -1775,  1134,  1164, -1775,
   -1775, -1775, 14739, 17622,   897, 17574,  1250,   424,  1326,  1267,
     248, -1775,   709,   326, -1775,   709, -1775, 14918, 18262,   491,
   14918, 19597, 19691, -1775, -1775, -1775,  5539, -1775, -1775, -1775,
   -1775, -1775, -1775,  1151,   115, -1775,  1150,   115,  1155, 19971,
   19597, 18762,  1159, 11561,  1153,  1161, 17622,  1166,  1168, 17622,
     917, -1775,   702,   507, 11561, 14918, -1775, -1775, -1775, -1775,
   -1775, -1775,  1217,  1152,  1348,  1273,  4865,  4865,  4865,  4865,
    4865,  4865,  1200, -1775, 19162,  4865,   307,  4865, -1775, -1775,
   -1775, 18262, 19597,  1172, -1775,   262,  1334,  1290, 10979, -1775,
   -1775, -1775,  1177, 14918,  1116,    71, 18005, 17777,  1182, 17853,
    7683,   708,  1183, 14918,    98,   398, -1775,  1188, -1775, 17622,
    2617, -1775,  1248, -1775, -1775, -1775, 17525, -1775,  1356, -1775,
    1203, 17853, -1775, 17853, -1775,  1204,  1185,  1402, 18870,  1208,
   19691,  1404,  1211,  1213,  1214,  1281,  1411,  1221,  1222, -1775,
   -1775, -1775, 18916,  1220,  1415, 19736, 19824, 19934, 17853, 19645,
   15271,  5570,  5265, 16485, 17784, 18193, 18193, 18193, 18193,  2749,
    2749,  2749,  2749,  2749,  1292,  1292,   739,   739,   739,  1219,
    1219,  1219,  1219, -1775,  1225, -1775,  1227,  1228,  1230,  1231,
   -1775, -1775, 19691,  2617, 17622, 17622, -1775,   625, 15829,  2074,
   -1775, 18005, -1775, -1775, 19861, 14918,  1235, -1775,  1232,  2297,
   -1775,   193, 14918, -1775, -1775, 17002, -1775, 14918, -1775, 14918,
   -1775,   897, 13647,  1239,   353,   760,   353,   219, -1775, -1775,
   17622,   186, -1775,  1419,  1358, 14918, -1775,  1244,  1247,  1246,
      71,  1135, 19597,  1164,  1251, -1775,  1252,   115, 14918, 11561,
    1255, -1775, -1775,   846, -1775, -1775,  1256,  1254,  1249, -1775,
    1258,  4865, -1775,  4865, -1775, -1775,  1259,  1260,  1452,  1324,
    1261, -1775,  1454,  1262,  1263,  1264, -1775,  1331,  1271,  1466,
    1276, -1775, -1775,    71, -1775,  1445, -1775,  1277, -1775, -1775,
    1279,  1280,   165, -1775, -1775, 19691,  1282,  1283, -1775, 17146,
   -1775, -1775, -1775, -1775, -1775, -1775,  1343, 17622, 17622,  1107,
    1306, 17622, -1775, 19691, 18976, -1775, -1775, 17853, -1775, 17853,
   -1775, 17853, -1775, -1775, -1775, -1775, 17853, 19162, -1775, -1775,
   -1775, 17853, -1775, 17853, -1775, 20007, 17853,  1284,  7889, -1775,
   -1775, -1775, -1775,   625, -1775, -1775, -1775, -1775,   607, 16115,
   15829,  1374, -1775,  3119,  1317,  4410, -1775, -1775, -1775,   848,
    4005,   130,   131,  1288,   846,   675,   166, 19597, -1775, -1775,
   -1775,  1323, 17050, -1775, 17098, 19597, -1775,  3097, -1775,  6241,
    1408,   453,  1479,  1412, 14918, -1775, 19597, 11561, 11561, -1775,
    1375,  1164,  2524,  1164,  1296, 19597,  1297, -1775,  2545,  1298,
    2561, -1775, -1775,   115, -1775, -1775,  1360, -1775, -1775,  4865,
   -1775,  4865, -1775,  4865, -1775, -1775, -1775, -1775,  4865, -1775,
   19162, -1775, -1775,  2726, -1775,  8095, -1775, -1775, -1775, -1775,
   10567, -1775, -1775, -1775,  6447, 17622, -1775, -1775, -1775,  1299,
   17853, 19022, 19691, 19691, 19691,  1365, 19691, 19082, 20007, -1775,
   -1775,   625, 15829, 15829,  2617, -1775,  1490, 16831,    96, -1775,
   16115,   846, 16570, -1775,  1322, -1775,   133,  1304,   135, -1775,
   16484, -1775, -1775, -1775,   136, -1775, -1775,  2840, -1775,  1308,
   -1775,  1424,   787, -1775, 16305, -1775, 16305, -1775, -1775,  1501,
     848, -1775, 15578, -1775, -1775, -1775, -1775,  3342, -1775,  1502,
    1434, 14918, -1775, 19597,  1318,  1320,  1325,  1164,  1327, -1775,
    1375,  1164, -1775, -1775, -1775, -1775,  2761,  1340,  4865,  1385,
   -1775, -1775, -1775,  1403, -1775,  6447, 10773, 10567, -1775, -1775,
   -1775,  6447, -1775, -1775, 19691, 17853, 17853, 17853,  8301,  1341,
    1345, -1775, 17853, -1775, 15829, -1775, -1775, -1775, -1775, -1775,
   17622,  1551,  3119, -1775, -1775, -1775, -1775, -1775, -1775, -1775,
   -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775,
   -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775,
   -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775,
   -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775,
   -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775,
   -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775,
   -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775,
   -1775, -1775, -1775, -1775, -1775, -1775, -1775,   177, -1775,  1317,
   -1775, -1775, -1775, -1775, -1775,    93,   163, -1775,  1530,   138,
    4650,  1424,  1533, -1775, 17622,   787, -1775, -1775,  1347,  1535,
   14918, -1775, 19597, -1775, -1775,   154,  1349, 17622,   547,  1164,
    1327, 15757, -1775,  1164, -1775,  4865,  4865, -1775, -1775, -1775,
   -1775,  8507, 19691, 19691, 19691, -1775, -1775, -1775, 19691, -1775,
     743,  1541,  1544,  1351, -1775, -1775, 17853, 16484, 16484,  1487,
   -1775,  2840,  2840,   391, -1775, -1775, -1775, 17853,  1473, -1775,
    1378,  1359,   139, 17853, -1775,  4650, -1775, 17853, 19597,  1476,
   -1775,  1553, -1775,  1554, -1775,   164, -1775, -1775, -1775,  1363,
     547, -1775,   547, -1775, -1775,  8713,  1367,  1458, -1775,  1467,
    1418, -1775, -1775,  1478, 17622,  1398,  1551, -1775, -1775, 19691,
   -1775, -1775,  1409, -1775,  1549, -1775, -1775, -1775, -1775, 19691,
    1573,   380, -1775, -1775, 19691,  1389, 19691, -1775,   534,  1391,
    8919, 17622, -1775, 17622, -1775,  9125, -1775, -1775, -1775,  1394,
   -1775,  1397,  1417,  2617,   675,  1416, -1775, -1775, -1775, 17853,
    1421,   129, -1775,  1516, -1775, -1775, -1775, -1775, -1775, -1775,
    9331, -1775, 15829,   969, -1775,  1426,  2617,   611, -1775, 19691,
   -1775,  1401,  1599,   639,   129, -1775, -1775,  1527, -1775, 15829,
    1410, -1775,  1164,   148, -1775, 17622, -1775, -1775, -1775, 17622,
   -1775,  1420,  1422,   142, -1775,  1327,   651,  1532,   197,  1164,
    1414, -1775,   552, 17622, 17622, -1775,   463,  1604,  1536,  1327,
   -1775, -1775, -1775, -1775,  1538,   198,  1609,  1545, 14918, -1775,
     552,  9537,  9743, -1775,   474,  1610,  1555, 14918, -1775, 19597,
   -1775, -1775, -1775,  1613,  1556, 14918, -1775, 19597, 14918, -1775,
   19597, 19597
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   204,   473,     0,   914,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1007,
     995,     0,   778,     0,   784,   785,   786,    29,   851,   983,
     984,   171,   172,   787,     0,   152,     0,     0,     0,     0,
      30,     0,     0,     0,     0,   223,     0,     0,     0,     0,
       0,     0,   442,   443,   444,   441,   440,   439,     0,     0,
       0,     0,   252,     0,     0,     0,    37,    38,    40,    41,
      39,   791,   793,   794,   788,   789,     0,     0,     0,   795,
     790,     0,   761,    32,    33,    34,    36,    35,     0,   792,
       0,     0,     0,     0,     0,   796,   445,   583,    31,     0,
     170,   140,     0,   779,     0,     0,     4,   126,   128,   850,
       0,   760,     0,     6,     0,     0,   222,     7,     9,     8,
      10,     0,     0,   437,     0,   487,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   543,   485,   971,   972,   565,
     558,   559,   560,   561,   564,   562,   563,   468,   568,     0,
     467,   942,   762,   769,     0,   853,   557,   436,   945,   946,
     958,   486,     0,     0,     0,   489,   488,   943,   944,   941,
     979,   982,   547,   852,    11,   442,   443,   444,     0,     0,
      36,     0,   126,   222,     0,  1047,   486,  1048,     0,  1050,
    1051,   567,   481,     0,   474,   479,     0,     0,   529,   530,
     531,   532,    29,   983,   787,   764,    37,    38,    40,    41,
      39,     0,     0,  1071,   964,   762,     0,   763,   508,     0,
     510,   548,   549,   550,   551,   552,   553,   554,   556,     0,
    1011,     0,   860,   774,   242,     0,  1071,   465,   773,   767,
       0,   783,   763,   990,   991,   997,   989,   775,     0,   466,
       0,   777,   555,     0,   205,     0,     0,   470,   205,   150,
     472,     0,     0,   156,   158,     0,     0,   160,     0,    75,
      76,    82,    83,    67,    68,    59,    80,    91,    92,     0,
      62,     0,    66,    74,    72,    94,    86,    85,    57,   108,
      81,   101,   102,    58,    97,    55,    98,    56,    99,    54,
     103,    90,    95,   100,    87,    88,    61,    89,    93,    53,
      84,    69,   104,    77,   106,    70,    60,    47,    48,    49,
      50,    51,    52,    71,   107,   105,   110,    64,    45,    46,
      73,  1124,  1125,    65,  1129,    44,    63,    96,     0,    79,
       0,   126,   109,  1062,  1123,     0,  1126,     0,     0,     0,
     162,     0,     0,     0,   213,     0,     0,     0,     0,     0,
       0,     0,     0,   862,     0,   114,   116,   350,     0,     0,
     349,   355,     0,     0,   253,     0,   256,     0,     0,     0,
       0,  1068,   238,   250,  1003,  1007,   602,   632,   632,   602,
     632,     0,  1032,     0,   798,     0,     0,     0,  1030,     0,
      16,     0,   130,   230,   244,   251,   662,   595,   632,     0,
    1056,   575,   577,   579,   918,   473,   487,     0,     0,   485,
     486,   488,   205,     0,   986,   780,     0,   781,     0,     0,
       0,   202,     0,     0,   132,   339,     0,    28,     0,     0,
       0,     0,     0,   203,   221,     0,   249,   234,   248,   442,
     445,   222,   438,   988,     0,   934,   192,   193,   194,   195,
     196,   198,   199,   201,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   995,     0,   191,   988,   988,  1017,     0,     0,
       0,     0,     0,     0,     0,     0,   435,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     507,   509,   919,   920,     0,   933,   932,   339,   339,   988,
       0,  1003,     0,   222,     0,     0,   164,     0,   916,   911,
     860,     0,   487,   485,     0,  1015,     0,   600,   859,  1006,
     783,   487,   485,   486,   132,     0,   339,   464,     0,   935,
     776,     0,   140,   292,     0,   582,     0,   167,     0,   205,
     471,     0,     0,     0,     0,     0,   159,   190,   161,  1124,
    1125,  1121,  1122,     0,  1128,  1114,     0,     0,     0,     0,
      78,    43,    65,    42,  1063,   197,   200,   163,   140,     0,
     180,   189,     0,     0,     0,     0,     0,     0,   117,     0,
       0,     0,   861,   115,    18,     0,   111,     0,   351,     0,
     165,     0,     0,   166,   254,   255,  1052,     0,     0,   487,
     485,   486,   489,   488,     0,  1104,   262,     0,  1004,     0,
       0,     0,     0,   860,   860,     0,     0,     0,     0,   168,
       0,     0,   797,  1031,   851,     0,     0,  1029,   856,  1028,
     129,     5,    13,    14,     0,   260,     0,     0,   588,     0,
       0,   860,     0,   771,     0,   770,   765,   589,     0,     0,
       0,     0,     0,   918,   136,     0,   862,   917,  1133,   463,
     476,   490,   951,   970,   147,   139,   143,   144,   145,   146,
     436,     0,   566,   854,   855,   127,   860,     0,  1072,     0,
       0,     0,     0,   862,   340,     0,     0,     0,   487,   209,
     210,   208,   485,   486,   205,   184,   182,   183,   185,   571,
     224,   258,     0,   987,     0,     0,   513,   515,   514,   526,
       0,     0,   546,   511,   512,   516,   518,   517,   535,   536,
     533,   534,   537,   538,   539,   540,   541,   527,   528,   520,
     521,   519,   522,   523,   525,   542,   524,     0,     0,  1021,
       0,   860,  1055,     0,  1054,  1071,   948,   240,   232,   246,
       0,  1056,   236,   222,     0,   477,   480,   482,   492,   495,
     496,   497,   498,   499,   500,   501,   502,   503,   504,   505,
     506,   922,     0,   921,   924,   947,   928,  1071,   925,     0,
       0,     0,     0,     0,     0,  1049,   475,   909,   913,   859,
     915,   462,   766,     0,  1010,     0,  1009,   258,     0,   766,
     994,   993,   979,   982,     0,     0,   921,   924,   992,   925,
     484,   294,   296,   136,   586,   585,   469,     0,   140,   276,
     151,   472,     0,     0,     0,     0,   205,   288,   288,   157,
     860,     0,     0,  1113,     0,  1110,   860,     0,  1084,     0,
       0,     0,     0,     0,   858,     0,    37,    38,    40,    41,
      39,     0,     0,     0,   800,   804,   805,   806,   809,   807,
     808,   811,     0,   799,   134,   849,   810,  1071,  1127,   205,
       0,     0,     0,    21,     0,    22,     0,    19,     0,   112,
       0,    20,     0,     0,     0,   123,   862,     0,   121,   116,
     113,   118,     0,   348,   356,   353,     0,     0,  1041,  1046,
    1043,  1042,  1045,  1044,    12,  1102,  1103,     0,   860,     0,
       0,     0,  1003,  1000,     0,   599,     0,   613,   859,   601,
     859,   631,   616,   625,   628,   619,  1040,  1039,  1038,     0,
    1034,     0,  1035,  1037,   205,     5,     0,     0,     0,   657,
     658,   667,   666,     0,     0,   485,     0,   859,   594,   598,
       0,   622,     0,  1057,     0,   576,     0,   205,  1091,   918,
     320,  1133,  1132,     0,     0,     0,   985,   859,  1074,  1070,
     342,   336,   337,   341,   343,   759,   861,   338,     0,     0,
       0,     0,   462,     0,     0,   490,     0,   952,   212,   205,
     142,   918,     0,     0,   260,   573,   226,   930,   931,   545,
       0,   639,   640,     0,   637,   859,  1016,     0,     0,   339,
     262,     0,   260,     0,     0,   258,     0,   995,   493,     0,
       0,   949,   950,   980,   981,     0,     0,     0,   897,   867,
     868,   869,   876,     0,    37,    38,    40,    41,    39,     0,
       0,     0,   882,   888,   889,   890,   893,   891,   892,     0,
     880,   878,   879,   903,   860,     0,   911,  1014,  1013,     0,
     260,     0,   936,     0,   782,     0,   298,     0,   205,   148,
     205,     0,   205,     0,     0,     0,     0,   268,   269,   280,
       0,   140,   278,   177,   288,     0,   288,     0,   859,     0,
       0,     0,     0,     0,   859,  1112,  1115,  1080,   860,     0,
    1075,     0,   860,   832,   833,   830,   831,   866,     0,   860,
     858,   606,   634,   634,   606,   634,   597,   634,     0,     0,
    1023,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1118,
     214,     0,   217,   181,     0,     0,     0,   119,     0,   124,
     125,   117,   861,   122,     0,   352,     0,  1053,   169,  1069,
    1104,  1095,  1099,   261,   263,   362,     0,     0,  1001,     0,
     604,     0,  1033,     0,    17,   205,  1056,   259,   362,     0,
       0,     0,   766,   591,     0,   772,  1058,     0,  1091,   580,
     135,   137,     0,     0,     0,  1133,     0,     0,   325,   323,
     924,   937,  1071,   924,   938,  1071,  1073,   988,     0,     0,
       0,   344,   133,   207,   209,   210,   486,   188,   206,   186,
     187,   211,   141,     0,   918,   257,     0,   918,     0,   544,
    1020,  1019,     0,   339,     0,     0,     0,     0,     0,     0,
     260,   228,   783,   923,   339,     0,   872,   873,   874,   875,
     883,   884,   901,     0,   860,     0,   897,   610,   636,   636,
     610,   636,     0,   871,   905,   636,     0,   859,   908,   910,
     912,     0,  1008,     0,   923,     0,     0,     0,   205,   295,
     587,   153,     0,   472,   268,   270,  1003,     0,     0,     0,
     205,     0,     0,     0,     0,     0,   282,     0,  1119,     0,
       0,  1105,     0,  1111,  1109,  1076,   859,  1082,     0,  1083,
       0,     0,   802,   859,   857,     0,     0,   860,     0,     0,
     846,   860,     0,     0,     0,     0,   860,     0,     0,   812,
     847,   848,  1027,     0,   860,   815,   817,   816,     0,     0,
     813,   814,   818,   820,   819,   836,   837,   834,   835,   838,
     839,   840,   841,   842,   827,   828,   822,   823,   821,   824,
     825,   826,   829,  1117,     0,   140,     0,     0,     0,     0,
     120,    23,   354,     0,     0,     0,  1096,  1101,     0,   436,
    1005,  1003,   478,   483,   491,     0,     0,    15,     0,   436,
     670,     0,     0,   672,   665,     0,   668,     0,   664,     0,
    1060,     0,     0,     0,     0,   543,     0,   489,  1092,   584,
    1133,     0,   326,   327,     0,     0,   321,     0,     0,     0,
     346,   347,   345,  1091,     0,   362,     0,   918,     0,   339,
       0,   977,   362,  1056,   362,  1059,     0,     0,     0,   494,
       0,     0,   886,   859,   896,   877,     0,     0,   860,     0,
       0,   895,   860,     0,     0,     0,   870,     0,     0,   860,
       0,   881,   902,  1012,   362,     0,   140,     0,   291,   277,
       0,     0,     0,   267,   173,   281,     0,     0,   284,     0,
     289,   290,   140,   283,  1120,  1106,     0,     0,  1079,  1078,
       0,     0,  1131,   865,   864,   801,   614,   859,   605,     0,
     617,   859,   633,   626,   629,   620,     0,   859,   596,   803,
     623,     0,   638,   859,  1022,   844,     0,     0,   205,    24,
      25,    26,    27,  1098,  1093,  1094,  1097,   264,     0,     0,
       0,   443,   434,     0,     0,     0,   239,   361,   363,     0,
     433,     0,     0,     0,  1056,   436,     0,   603,  1036,   358,
     245,   660,     0,   663,     0,   590,   578,   486,   138,   205,
       0,     0,   330,   319,     0,   322,   329,   339,   339,   335,
     570,  1091,   436,  1091,     0,  1018,     0,   976,   436,     0,
     436,  1061,   362,   918,   973,   900,   899,   885,   615,   859,
     609,     0,   618,   859,   635,   627,   630,   621,     0,   887,
     859,   904,   624,   436,   140,   205,   149,   154,   175,   271,
     205,   279,   285,   140,   287,     0,  1107,  1077,  1081,     0,
       0,     0,   608,   845,   593,     0,  1026,  1025,   843,   140,
     218,  1100,     0,     0,     0,  1064,     0,     0,     0,   265,
       0,  1056,     0,   399,   395,   401,   761,    36,     0,   389,
       0,   394,   398,   411,     0,   409,   414,     0,   413,     0,
     412,     0,   222,   365,     0,   367,     0,   368,   369,     0,
       0,  1002,     0,   661,   659,   671,   669,     0,   331,   332,
       0,     0,   317,   328,     0,     0,     0,  1091,  1085,   235,
     570,  1091,   978,   241,   358,   247,   436,     0,     0,     0,
     612,   894,   907,     0,   243,   293,   205,   205,   140,   274,
     174,   286,  1108,  1130,   863,     0,     0,     0,   205,     0,
       0,   461,     0,  1065,     0,   379,   383,   458,   459,   393,
       0,     0,     0,   374,   720,   721,   719,   722,   723,   740,
     742,   741,   711,   683,   681,   682,   701,   716,   717,   677,
     688,   689,   691,   690,   758,   710,   694,   692,   693,   695,
     696,   697,   698,   699,   700,   702,   703,   704,   705,   706,
     707,   709,   708,   678,   679,   680,   684,   685,   687,   757,
     725,   726,   730,   731,   732,   733,   734,   735,   718,   737,
     727,   728,   729,   712,   713,   714,   715,   738,   739,   743,
     745,   744,   746,   747,   724,   749,   748,   751,   753,   752,
     686,   756,   754,   755,   750,   736,   676,   406,   673,     0,
     375,   427,   428,   426,   419,     0,   420,   376,   453,     0,
       0,     0,     0,   457,     0,   222,   231,   357,     0,     0,
       0,   318,   334,   974,   975,     0,     0,     0,     0,  1091,
    1085,     0,   237,  1091,   898,     0,     0,   140,   272,   155,
     176,   205,   607,   592,  1024,   216,   377,   378,   456,   266,
       0,   860,   860,     0,   402,   390,     0,     0,     0,   408,
     410,     0,     0,   415,   422,   423,   421,     0,     0,   364,
    1066,     0,     0,     0,   460,     0,   359,     0,   333,     0,
     655,   862,   136,   862,  1087,     0,   429,   136,   225,     0,
       0,   233,     0,   611,   906,   205,     0,   178,   380,   126,
       0,   381,   382,     0,   859,     0,   859,   404,   400,   405,
     674,   675,     0,   391,   424,   425,   417,   418,   416,   454,
     451,  1104,   370,   366,   455,     0,   360,   656,   861,     0,
     205,   861,  1086,     0,  1090,   205,   136,   227,   229,     0,
     275,     0,   220,     0,   436,     0,   396,   403,   407,     0,
       0,   918,   372,     0,   653,   569,   572,  1088,  1089,   430,
     205,   273,     0,     0,   179,   387,     0,   435,   397,   452,
    1067,     0,   862,   447,   918,   654,   574,     0,   219,     0,
       0,   386,  1091,   918,   302,  1133,   450,   449,   448,  1133,
     446,     0,     0,     0,   385,  1085,   447,     0,     0,  1091,
       0,   384,     0,  1133,  1133,   308,     0,   307,   305,  1085,
     140,   431,   136,   371,     0,     0,   309,     0,     0,   303,
       0,   205,   205,   313,     0,   312,   301,     0,   304,   311,
     373,   215,   432,   314,     0,     0,   299,   310,     0,   300,
     316,   315
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1775, -1775, -1775,  -574, -1775, -1775, -1775,    99,  -447,   -47,
     333, -1775,  -244,  -536, -1775, -1775,   430,   655,  1275, -1775,
    2622, -1775,  -809, -1775,  -537, -1775,  -707,    11, -1775, -1775,
   -1775, -1775, -1775, -1775, -1775, -1775,  -946, -1775, -1775,  -923,
    -293, -1775, -1775, -1775,  -399, -1775, -1775,  -191,   162,    46,
   -1775, -1775, -1775, -1775, -1775, -1775,    54, -1775, -1775, -1775,
   -1775, -1775, -1775,    55, -1775, -1775,  1123,  1131,  1125,   -84,
    -747,  -939,   591,   665,  -405,   311, -1014, -1775,  -111, -1775,
   -1775, -1775, -1775,  -767,   118, -1775, -1775, -1775, -1775,  -396,
   -1775,  -586, -1775,   392,  -445, -1775, -1775,  1023, -1775,   -89,
   -1775, -1775, -1129, -1775, -1775, -1775, -1775, -1775, -1775, -1775,
   -1775, -1775, -1775,  -126, -1775,   -35, -1775, -1775, -1775, -1775,
   -1775,  -211, -1775,    74, -1163, -1775, -1729,  -429, -1775,  -142,
     267,  -118,  -400, -1775,  -209, -1775, -1775, -1775,   102,   -90,
     -72,    -8,  -781,   -68, -1775, -1775,    32, -1775,    49,  -383,
   -1775,     9,    -5,   -86,   -76,   -70, -1775, -1775, -1775, -1775,
   -1775, -1775, -1775, -1775, -1775,  -630,  -903, -1775, -1775, -1775,
   -1775, -1775,  1669,  1274, -1775,   520, -1775,   366, -1775, -1775,
   -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775, -1775,
   -1775, -1775, -1775, -1775, -1775, -1775, -1775,  -180,  -373,  -598,
   -1775, -1775, -1775, -1775, -1775,   450, -1775, -1775, -1775, -1775,
   -1775, -1775, -1775, -1775, -1117,  -369,  2303,    36, -1775,   396,
    -420, -1775, -1775,  -497,  3303,  3722, -1775,   618, -1775, -1775,
     530,   222,  -667, -1775, -1775,   610,   385,   358, -1775,   376,
   -1775, -1775, -1775, -1775, -1775,   589, -1775, -1775, -1775,   283,
    -942,  -190,  -457,  -440, -1775,  -182,  -141, -1775, -1775,    38,
      42,   751,   -81, -1775, -1775,   800,   -79, -1775,  -353,    23,
    -389,   117,  -431, -1775, -1775,  -453,  1300, -1775, -1775, -1775,
   -1775, -1775,   729,   648, -1775, -1775, -1775,  -347,  -722, -1775,
    1245, -1430,  -252,   -58,  -155,   818, -1775, -1775, -1775, -1774,
   -1775,  -296, -1159, -1340,  -284,   141, -1775,   501,   585, -1775,
   -1775, -1775, -1775,   535, -1775,  3149,  -737
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   116,   975,   671,   192,   352,   785,
     372,   373,   374,   375,   926,   927,   928,   118,   119,   120,
     121,   122,   997,  1240,   432,  1029,   705,   706,   579,   268,
    1756,   585,  1660,  1757,  2012,   911,   124,   125,   726,   727,
     735,   365,   608,  1967,  1193,  1415,  2034,   455,   193,   707,
    1032,  1278,  1488,   128,   674,  1051,   708,   741,  1055,   646,
    1050,   247,   560,   709,   675,  1052,   457,   392,   414,   131,
    1034,   978,   951,  1213,  1688,  1338,  1117,  1909,  1760,   860,
    1123,   584,   869,  1125,  1532,   852,  1106,  1109,  1327,  2041,
    2042,   695,   696,  1013,   722,   723,   379,   380,   382,  1722,
    1887,  1888,  1429,  1587,  1711,  1881,  2021,  2044,  1920,  1971,
    1972,  1973,  1698,  1699,  1700,  1701,  1922,  1923,  1929,  1983,
    1704,  1705,  1709,  1874,  1875,  1876,  1958,  2083,  1588,  1589,
     194,   133,  2059,  2060,  1879,  1591,  1592,  1593,  1594,   134,
     135,   654,   581,   136,   137,   138,   139,   140,   141,   142,
     143,   261,   144,   145,   146,  1737,   147,  1031,  1277,   148,
     692,   693,   694,   265,   424,   575,   680,   681,  1376,   682,
    1377,   149,   150,   652,   653,  1366,  1367,  1497,  1498,   151,
     895,  1083,   152,   896,  1084,   153,   897,  1085,   154,   898,
    1086,   155,   899,  1087,   156,   900,  1088,   655,  1369,  1500,
     157,   901,   158,   159,  1951,   160,   676,  1724,   677,  1229,
     984,  1448,  1444,  1867,  1868,   161,   162,   163,   250,   164,
     251,   262,   436,   567,   165,  1370,  1371,   905,   906,   166,
    1148,   559,   623,  1149,  1091,  1300,  1092,  1501,  1502,  1303,
    1304,  1094,  1508,  1509,  1095,   828,   550,   206,   207,   710,
     698,   534,  1250,  1251,   816,   817,   465,   168,   253,   169,
     170,   196,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   744,   257,   258,   649,   241,   242,   780,   781,
    1383,  1384,   407,   408,   969,   182,   637,   183,   691,   184,
     355,  1889,  1941,   393,   444,   716,   717,  1138,  1139,  1898,
    1953,  1954,  1244,  1426,   947,  1427,   948,   949,   875,   876,
     877,   356,   357,   908,   594,  1002,  1003
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     195,   197,   542,   199,   200,   201,   202,   204,   205,   353,
     208,   209,   210,   211,   462,   123,   231,   232,   233,   234,
     235,   236,   237,   238,   240,   515,   259,  1030,   686,  1000,
     431,   535,   536,   429,   449,   851,   266,   426,   450,   267,
     415,   427,   264,   451,  1110,   419,   420,   275,   683,   278,
     127,  1242,   363,   256,   366,   269,  1017,  1245,   129,   130,
     273,   784,   995,   837,   685,   730,   249,   568,   254,  1054,
     687,   909,   255,   458,   777,   778,   462,   814,   401,  1451,
    1113,   267,   775,   514,  1234,   925,   930,  1576,   823,   361,
    1100,   564,   819,   820,   815,  1276,  1263,   974,  1268,  1439,
    1334,  1127,  1931,   117,   428,  1774,   362,   996,   821,   -78,
     446,   847,   431,  1287,   -78,   429,   865,   569,   936,   426,
     576,   844,   867,   613,   615,   617,  1960,   620,   848,  1932,
    1530,  -955,   -43,   -42,   629,   632,   576,   -43,   -42,  1714,
    1716,  1141,  -392,   431,  1782,  1869,   553,  1938,  1938,  1686,
     552,  1774,   276,   576,   953,   351,  1019,   736,   737,   738,
     377,  1323,   381,    14,  1242,   402,   126,   953,    14,   562,
     953,   625,   391,   561,   953,   953,   842,    14,  2003,  1159,
     551,   609,    14,  1312,  -763,  -110,   428,   532,   533,   532,
     533,  1926,  1949,   545,  1934,   413,    14,   391,   918,   945,
     946,  -110,   391,   391,   198,  1188,   383,   532,   533,  1927,
    -462,  1247,   661,  1935,     3,   384,  1936,   428,   656,  1160,
     658,  -965,   980,  -770,  1611,   625,   443,   260,  1928,   434,
     391,  2007,   502,  2008,   626,  2076,  2094,  1950,   688,  -641,
     428,  -764,   405,   406,   503,   610,  -953,  1601,   404,   463,
    -956,  1313,  -967,  1375,   973,   571,  1248,  -954,   571,  1203,
    1777,   919,   580,  -955,  1246,   267,   582,  1424,  1425,  1612,
     742,   132,   539,  -952,  -996,   573,   532,   533,  -959,   578,
    2077,  2095,   378,   593,  1882,  -960,  1883,   167,   442,  -649,
    1241,  2072,   868,  1933,   461,   539,  -649,  1775,  1776,  1531,
    -957,   -78,   447,  -999,  -998,  2090,   549,   866,  1290,   937,
     643,   604,   577,   640,  1620,  -324,  -861,   639,  -939,  1453,
    -861,  1112,  1272,  1523,   -43,   -42,   630,   633,   659,  -306,
    -324,  1715,  1717,  -859,  -392,   981,  1783,  1870,  -771,  1939,
    1993,  1576,  -462,  2071,   938,   954,  1622,  1020,  -861,  1249,
     982,  1487,   824,  1628,   202,  1630,   543,  1341,  1065,  1345,
     416,  1430,  1613,   117,  -765,  1659,  1721,   117,  -964,   431,
     983,   583,   732,  2078,  2096,  -963,   728,  -772,  -953,  -962,
     638,   464,  -956,   267,   428,  1653,  1199,  1200,  1511,  -954,
     240,   651,   267,   267,   651,   267,  -940,   740,   462,   353,
     665,  1225,  -966,  1241,   540,  -952,  -996,  -969,  -649,   385,
    -959,  1507,  1446,   267,  1049,   226,   226,  -960,   538,   386,
     204,   734,  1986,   463,  -581,   538,   942,   540,   711,  1440,
     945,   946,  -957,  -652,   516,  -999,  -998,   213,    40,  1380,
     724,  1987,  1441,   731,  1988,  1273,  1447,  -650,   421,   795,
    -939,   213,    40,   415,   790,   791,   458,   603,   743,   745,
     463,   263,  1738,  1442,  1740,   270,  1343,  1344,   271,   746,
     747,   748,   749,   751,   752,   753,   754,   755,   756,   757,
     758,   759,   760,   761,   762,   763,   764,   765,   766,   767,
     768,   769,   770,   771,   772,   773,   774,   729,   776,  1216,
     743,   743,   779,  1746,  1438,  1343,  1344,  1463,  1461,  1004,
     660,  1005,   798,   799,   800,   801,   802,   803,   804,   805,
     806,   807,   808,   809,   810,   256,   532,   533,  -940,  -651,
     918,   117,   724,   724,   743,   822,  1729,   538,   249,   798,
     254,   715,   826,   784,   255,   351,  2086,  -109,  1253,   796,
     402,   834,  1520,   836,   391,  1609,   433,  2103,   667,   112,
     422,   724,  1346,  -109,  -926,  1254,   797,   423,   515,   855,
     442,   856,  2023,   112,  1011,  1012,   532,   533,  1896,   402,
    -926,   272,  1900,   402,  1340,  1107,  1108,   667,   841,   364,
     859,   403,   464,   442,   126,   532,   533,   396,   686,   387,
    1464,  1533,  1056,   985,  1284,   672,   673,   603,   391,   788,
     391,   391,   391,   391,  1459,  2004,   402,  2024,   683,  1540,
     388,  -766,   932,   854,   667,   786,   514,   405,   406,  1730,
    1048,  -929,   402,   813,   685,   226,  1265,  1292,  1265,  2087,
     687,  1004,  1005, -1071,   402,   389,  1253,  -929,  1101,  1103,
    2104,   818,   435,   603,  1675,   390,   405,   406,  -967,   404,
     405,   406,  1060,  1254,   443, -1071,   925,   846,  1194,   394,
    1195,   786,  1196,   402,   428,   714,  1198,   793,   117,   402,
   -1071,   438,   843, -1071,  1036,   849,   443,   667,  1474,   395,
    -927,  1476,   668,   405,   406,  1325,  1326,  1102,   907,   132,
     612,   614,   616,   713,   619, -1071,  -927,   697,   662,   405,
     406,  1503,  1381,  1505,   397,   376,  1014,  1510,  1342,  1343,
    1344,   405,   406,  1610,   931,   715,  2055,  1267,  1424,  1425,
    1269,  1270,   459,   186,   187,    65,    66,    67,  2073,   564,
    1959,  1682,  1683,   411,  1962,  1039,   412,  1753,  1956,  1957,
     405,   406,  1189,  2081,  2082,   171,   405,   406,   398,   968,
     970,  1629,   399,  2056,  2057,  2058,   400,   686,  1984,  1985,
     228,   230,   830,   416,   226,  2056,  2057,  2058,  1047,   417,
    1372,   418,  1374,   226,  1378,  -126,   441,   683,   442,  -126,
     226,   445,  1184,  1185,  1186,  1489,   459,   186,   187,    65,
      66,    67,   226,   685,   448,   460,  -126,  1059,  1187,   687,
    1980,  1981,  2051,   684,  1527,  1343,  1344,   453,    55,   229,
     229,  1606,  1252,  1255,   212,   454,  1469,   459,   186,   187,
      65,    66,    67,   117,   466,   499,   500,   501,  1480,   502,
    1105,   391,   437,   439,   440,  -642,   467,    50,   468,  1490,
     469,   503,   507,   580,   470,   471,   267,   472,   473,  1265,
     430,  -643,  -644,  -647,  -645,  -646,  1111,   508,   505,   460,
     537,   506,  1719,  -961,  -648,   959,   961,  1122,  1568,  -764,
     541,   409,   548,  1522,   216,   217,   218,   219,   220,   503,
     546,  1624,   443,  2065,   554,   557,   126,  -965,   558,   538,
     460,   566,  1030,   988,  -762,   574,   189,   565,   587,    91,
    2079,   595,    93,    94, -1116,    95,   190,    97,   459,    63,
      64,    65,    66,    67,   452,   686,  1082,   598,  1096,    72,
     509,   599,   605,   622,   606,   631,   621,   226,  1008,   624,
     634,   108,   430,   635,   644,   683,  1968,   645,   689,   690,
     117,  1220,   712,  1221,  -131,   856,   699,   700,   701,   703,
      55,   685,   734,   829,  1120,   117,  1223,   687,   662,  1778,
     510,   739,   511,   430,   831,   827,   697,   516,  1596,  1655,
     832,  1233,   857,   838,   839,   123,   512,   576,   513,   861,
     555,   460,   879,   864,   935,  1664,   563,   593,  1291,   950,
     913,   132,   878,  1046,   910,   958,   957,   915,   117,   912,
     960,  1261,   914,   126,   731,   171,   731,  1197,   715,   171,
     127,   798,   376,   376,   376,   618,   376,   916,   129,   130,
     917,   939,   920,   921,  1626,  1279,   940,   943,  1280,   229,
    1281,   944,   952,   962,   724,   955,   963,  1747,  1212,   976,
     971,   663,   977,   964,  1242,   669,   965,   979,  -787,  1242,
     986,   987,   989,   990,   670,   994,   991,   998,  1264,   730,
    1264,   126,   999,   117,  1007,   797,  1009,  1015,  1016,  2043,
     256,  1018,  1021,   663,  1242,   669,   663,   669,   669,   603,
    1022,  1023,  1129,   249,  1322,   254,   117,  1467,  1135,   255,
    1468,  1033,  2043,   813,   813,  1024,  1025,  1235,  1037,  1045,
    1044,  2066,  1041,  1042,  1061,   226,  1328,  1755,   132,  1329,
    1053,   818,   818,  1062,  1063,  1035,  1761,  -768,   117,   628,
    1104,  1114,  1685,  1124,  1126,  1128,   126,  1132,   636,  1133,
     641,  1134,  1768,  2000,  1136,   648,  1242,  1150,  2005,  1151,
    1152,  1432,  1454,  1153,  1154,   391,  1455,   666,  1155,   126,
    1156,  1456,   736,   737,   738,  1299,  1299,  1082,   686,  1157,
    1211,  1192,  1734,  1735,  1202,  1204,   132,  1206,   229,  1209,
    1210,  1219,  1208,   171,  1215,  1093,  1228,   229,   683,   642,
     226,   126,  1222,  1231,   229,  1230,  1243,  2030,  1232,   733,
     846,  1236,   846,  1238,   685,  1257,   229,   117,  1274,   117,
     687,   117,  1283,  1286,  1434,  1289,  1295,  1294,   849,  -968,
     849,  1911,  1305,  1306,  1307,  1445,  1308,  1309,  1310,   226,
    1311,   226,  1352,  1314,  1315,  1317,   123,   731,  1316,  1319,
    1773,   132,  1331,  1685,   496,   497,   498,   499,   500,   501,
    1333,   502,   743,  1336,  1337,  1472,  1339,   167,   603,   226,
    1349,   686,  1348,   503,   132,  1350,  1356,  1685,  1433,  1685,
    1358,   127,   126,  2092,   126,  1685,   929,   929,   724,   129,
     130,   683,   697, -1132,  1999,  1359,  2002,   907,  1187,   724,
    1434,  1264,   648,  1241,   223,   223,   132,   685,  1241,  1362,
    1363,  1414,  1416,   687,  1417,   246,  1421,  1418,   459,    63,
      64,    65,    66,    67,   697,   535,  1318,  1419,  2067,    72,
     509,  1428,  2068,  1241,   117,   580,  1431,  1449,   267,  1049,
     171,   246,   226,  1462,  1515,  1450,  2084,  2085,  1529,  1518,
    1465,   229,  1181,  1182,  1183,  1184,  1185,  1186,   226,   226,
    1466,  1473,  1475,  1481,  1477,  1491,  1492,  1493,  1479,  1506,
    1357,  1187,   511,  1482,  1360,  1072,  1516,  1517,  1484,  1485,
    1965,  1364,  1534,   431,  1514,  2054,   429,   132,  1519,   132,
     426,   460,  1524,   684,  1528,  1241,  1537,   126,  1541,  1546,
     544,   518,   519,   520,   521,   522,   523,   524,   525,   526,
     527,   528,   529,  1542,  1545,  1082,  1082,  1082,  1082,  1082,
    1082,  1547,  1550,  1551,  1082,  1553,  1082,  1554,  1555,  1556,
    1557,  1559,  1560,  1562,  1563,  1567,  1302,   117,  1569,  1570,
    1597,  1571,  1572,  1614,  1599,   530,   531,  1602,  1598,   117,
    1608,  1615,  1604,  1617,  1605,  1090,  1618,   731,  1633,  1536,
    1619,  1621,  1623,  1720,  1685,  1627,  1632,  1631,  1634,  1637,
    1616,  1639,  1641,  1643,  1638,  1642,  1645,  1646,  1647,  1648,
    1010,  1649,   462,  1625,   724,  1650,  1652,  1654,  1656,  1657,
    1658,  1665,  1668,  1661,  1662,   171,  1679,  1690,  1703,  1718,
    1723,  1728,   132,  1731,  1736,  1732,  1741,  1742,  1748,  1763,
    1744,   729,   126,  1766,  1772,  1780,  1781,  1878,   167,  1877,
     226,   226,   532,   533,   223,  1884,  1890,  1891,  1893,   229,
    1894,  1880,  1573,  1905,  1895,  1897,  1494,   544,   518,   519,
     520,   521,   522,   523,   524,   525,   526,   527,   528,   529,
    1903,  1906,  1916,  2091,  1937,  1058,  1917,  1943,  1946,  1947,
    1974,  1952,   684,  1976,  1978,  1982,  1990,   697,  1992,  1997,
     697,  1991,  1998,  2001,   246,  2006,   246,   929,  2010,   929,
    -388,   929,   530,   531,  2011,   929,   702,   929,   929,  1201,
    2013,  2014,  2016,  2018,  1097,  1932,  1098,  2019,  2022,  1548,
    1082,  2025,  1082,  1552,   229,  2031,  2032,  2033,  1558,  2045,
    2038,  2052,   171,  2049,  1727,  2040,  1564,   132,  2053,  1733,
    2062,  2064,   724,   724,  1118,  2075,  2080,   171,  2088,  2089,
    2069,  2093,  2070,  2097,  2105,   246,  1771,  2108,  2098,    34,
      35,    36,  1420,   229,  2048,   229,   789,   792,  2106,  2109,
     787,  1285,  1227,   214,  2063,  1521,  1910,  1663,  2061,   532,
     533,  1471,   933,   223,   226,  1901,  1925,  1779,  1930,  1710,
     171,  2100,   223,   229,  1302,  1499,  2074,   117,  1499,   223,
    1899,  1759,  1942,   657,  1373,  1512,  1504,  1301,   351,  1443,
    1365,   223,  1496,  1691,  1708,  1320,  1090,  1207,   224,   224,
    1495,   725,   223,  1995,  1945,   650,  1590,  1142,    81,    82,
      83,    84,    85,   648,  1218,  2027,  1590,  2020,   117,   221,
     684,  1423,  1595,   840,  1681,    89,    90,   226,   246,  1354,
    1640,   246,  1595,  1413,  1644,   171,  1892,     0,     0,    99,
     126,  1651,   226,   226,     0,     0,   229,     0,  1082,     0,
    1082,     0,  1082,     0,   105,     0,     0,  1082,   171,     0,
       0,  1712,   229,   229,   117,     0,     0,     0,     0,   117,
     697,     0,     0,   117,     0,     0,     0,  1908,  1759,     0,
    1266,   126,  1266,     0,     0,     0,     0,   246,     0,     0,
     171,     0,     0,   391,     0,     0,   603,     0,     0,   351,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1866,
       0,     0,     0,     0,     0,     0,  1873,     0,     0,     0,
       0,     0,     0,   351,     0,   351,   223,   126,     0,     0,
       0,   351,     0,     0,     0,     0,   126,   226,     0,     0,
       0,     0,     0,  1940,     0,   132,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1082,     0,  1635,
       0,  1636,     0,     0,   117,   117,   117,   929,     0,   171,
     117,   171,   516,   171,     0,  1118,  1335,   117,   246,     0,
     246,     0,  2036,   894,     0,     0,   132,     0,     0,     0,
       0,     0,  1885,     0,     0,  1948,     0,     0,     0,  1590,
       0,     0,     0,     0,     0,  1590,     0,  1590,  1940,     0,
       0,     0,     0,     0,     0,  1595,   894,     0,   224,     0,
       0,  1595,     0,  1595,   229,   229,   697,   126,     0,   462,
    1590,     0,   132,   126,  1090,  1090,  1090,  1090,  1090,  1090,
     126,   132,     0,  1090,     0,  1090,  1595,   544,   518,   519,
     520,   521,   522,   523,   524,   525,   526,   527,   528,   529,
       0,     0,     0,   684,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   246,   246,     0,     0,     0,     0,
       0,     0,     0,   246,     0,     0,   171,     0,     0,   603,
       0,     0,   530,   531,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1266,   223,     0,     0,  1749,     0,  1750,
     351,  1751,     0,     0,  1082,  1082,  1752,     0,     0,  1470,
     117,     0,     0,  1590,     0,     0,     0,     0,     0,  1969,
       0,     0,   132,     0,     0,     0,  1866,  1866,   132,  1595,
    1873,  1873,     0,     0,     0,   132,     0,     0,     0,     0,
       0,     0,  1457,     0,   603,     0,   684,   224,     0,     0,
       0,     0,     0,     0,     0,     0,   224,     0,   229,   532,
     533,     0,     0,   224,   117,     0,     0,     0,     0,   223,
       0,     0,  1513,   126,     0,   224,     0,     0,     0,   171,
    1578,     0,     0,  2099,     0,     0,     0,   648,  1118,     0,
       0,   171,  2107,     0,     0,     0,     0,     0,     0,   117,
    2110,     0,   246,  2111,   117,     0,  1904,     0,   223,  1090,
     223,  1090,  2035,     0,     0,     0,     0,     0,     0,     0,
       0,   229,    14,   941,     0,     0,     0,   126,     0,   117,
       0,     0,     0,     0,     0,  2050,   229,   229,   223,   894,
       0,     0,     0,  1975,  1977,     0,   246,     0,     0,     0,
       0,     0,     0,   246,   246,   894,   894,   894,   894,   894,
       0,     0,   126,     0,     0,     0,     0,   126,   894,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   132,     0,
       0,     0,   648,     0,     0,   246,     0,  1579,     0,     0,
     117,   117,   126,  1580,     0,   459,  1581,   187,    65,    66,
      67,  1582,     0,  1607,     0,     0,     0,     0,     0,     0,
     224,   223, -1134, -1134, -1134, -1134, -1134,   494,   495,   496,
     497,   498,   499,   500,   501,   246,   502,   223,   223,     0,
       0,   229,   132,     0,     0,     0,     0,   289,   503,     0,
       0,     0,     0,  1583,  1584,     0,  1585,     0,     0,     0,
       0,   246,   246,   126,   126,     0,     0,  1090,     0,  1090,
       0,  1090,   223,  1963,  1964,     0,  1090,   132,   460,   246,
       0,     0,   132,     0,   291,     0,   246,  1586,     0,     0,
       0,  2037,   246,     0,     0,     0,     0,   212,     0,     0,
       0,     0,     0,   894,     0,     0,     0,   132,     0,     0,
       0,     0,     0,  1578,   697,     0,     0,     0,   246,     0,
      50,     0,     0,     0,     0,     0,     0,     0,   596,   171,
       0,     0,   225,   225,     0,     0,     0,   697,   246,     0,
       0,     0,   246,   248,     0,     0,   697,     0,     0,     0,
       0,     0,     0,   246,     0,    14,   589,   216,   217,   218,
     219,   220,   590,     0,     0,     0,     0,     0,   132,   132,
     171,     0,     0,     0,     0,     0,  1090,     0,     0,   189,
       0,     0,    91,   344,     0,    93,    94,     0,    95,   190,
      97,     0,     0,     0,     0,     0,     0,     0,   224,   223,
     223,     0,     0,   348,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   246,   108,   350,   171,   246,     0,   246,
    1579,   171,   246,     0,     0,   171,  1580,     0,   459,  1581,
     187,    65,    66,    67,  1582,   894,   894,   894,   894,   894,
     894,   223,   894,     0,     0,   894,   894,   894,   894,   894,
     894,   894,   894,   894,   894,   894,   894,   894,   894,   894,
     894,   894,   894,   894,   894,   894,   894,   894,   894,   894,
     894,   894,   894,   224,     0,   871,  1583,  1584,     0,  1585,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   894,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   460,     0,     0,     0,     0,     0,     0,     0,     0,
    1600,     0,   224,     0,   224,     0,   171,   171,   171,     0,
       0,     0,   171,     0,     0,   212,     0,     0,   246,   171,
     246,     0,     0,  1090,  1090,     0,   872,     0,     0,     0,
    1578,     0,   224,   223,     0,     0,     0,     0,    50,     0,
       0,     0,   225,     0,     0,     0,     0,     0,     0,     0,
       0,  1578,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   246,     0,     0,   246,     0,     0,  1578,     0,     0,
       0,     0,    14,     0,     0,   216,   217,   218,   219,   220,
       0,   246,   246,   246,   246,   246,   246,     0,     0,   223,
     246,     0,   246,    14,     0,     0,   223,   189,     0,     0,
      91,     0,     0,    93,    94,   224,    95,   190,    97,    14,
     873,   223,   223,     0,   894,     0,     0,     0,     0,     0,
       0,   224,   224,     0,   246,     0,     0,     0,     0,     0,
       0,   246,   108,     0,     0,     0,   894,  1579,   894,     0,
       0,     0,     0,  1580,     0,   459,  1581,   187,    65,    66,
      67,  1582,     0,     0,     0,     0,     0,     0,  1579,     0,
       0,     0,   171,   894,  1580,     0,   459,  1581,   187,    65,
      66,    67,  1582,     0,  1579,     0,     0,     0,   354,     0,
    1580,   225,   459,  1581,   187,    65,    66,    67,  1582,     0,
     225,     0,     0,  1583,  1584,     0,  1585,   225,   212,   246,
     246,     0,     0,   246,     0,     0,   223,     0,     0,   225,
       0,     0,     0,     0,  1583,  1584,   171,  1585,   460,     0,
     225,    50,     0,     0,     0,     0,     0,  1739,     0,     0,
    1583,  1584,  1578,  1585,     0,   246,     0,     0,     0,   460,
       0,     0,     0,     0,     0,     0,     0,     0,  1743,     0,
       0,   171,     0,     0,     0,   460,   171,     0,   216,   217,
     218,   219,   220,     0,  1745,     0,   246,  1578,   246,     0,
       0,     0,     0,     0,    14,     0,     0,     0,     0,     0,
       0,   171,     0,   224,   224,     0,    93,    94,     0,    95,
     190,    97, -1134, -1134, -1134, -1134, -1134,  1179,  1180,  1181,
    1182,  1183,  1184,  1185,  1186,   248,     0,     0,     0,    14,
       0,     0,   246,   246,     0,   108,   246,     0,  1187,     0,
       0,     0,   894,     0,   894,     0,   894,     0,     0,     0,
       0,   894,   223,     0,     0,     0,   894,     0,   894,  1579,
       0,   894,   171,   171,   225,  1580,     0,   459,  1581,   187,
      65,    66,    67,  1582,   246,   246,     0,     0,   246,     0,
       0,     0,     0,     0,     0,   246,     0,     0,     0,     0,
       0,     0,     0,     0,  1579,     0,     0,     0,     0,     0,
    1580,     0,   459,  1581,   187,    65,    66,    67,  1582,     0,
       0,     0,     0,     0,     0,  1583,  1584,     0,  1585,     0,
       0,   902,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   354,     0,   354,   246,     0,   246,     0,   246,     0,
     460,   212,     0,   246,     0,   223,     0,   224,     0,  1754,
    1583,  1584,     0,  1585,   902,     0,     0,     0,     0,     0,
     246,     0,     0,     0,    50,   894,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   460,     0,   246,   246,     0,
       0,     0,     0,     0,  1902,   246,     0,   246,     0,     0,
       0,     0,   354,     0,     0,     0,     0,     0,     0,     0,
       0,   216,   217,   218,   219,   220,     0,     0,     0,   246,
     224,   246,     0,     0,     0,     0,     0,   246,     0,     0,
       0,     0,     0,     0,     0,   224,   224,  1871,     0,    93,
      94,  1872,    95,   190,    97,     0,     0,     0,     0,     0,
       0,     0,   225,   246,  1026,   518,   519,   520,   521,   522,
     523,   524,   525,   526,   527,   528,   529,     0,   108,  1707,
     894,   894,   894,     0,     0,     0,     0,   894,     0,   246,
       0,   474,   475,   476,     0,   246,     0,   246,     0,     0,
       0,     0,     0,     0,     0,   354,     0,     0,   354,   530,
     531,   477,   478,     0,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   225,   502,     0,
     224,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     503,  1026,   518,   519,   520,   521,   522,   523,   524,   525,
     526,   527,   528,   529,     0,     0,     0,     0,     0,     0,
    1089,     0,     0,     0,     0,     0,   225,     0,   225,     0,
       0,     0,     0,     0,     0,     0,   532,   533,     0,     0,
    1692,     0,     0,     0,     0,     0,   530,   531,     0,   246,
       0,     0,     0,     0,     0,     0,   225,   902,     0,     0,
       0,     0,   246,     0,     0,     0,   246,     0,     0,     0,
     246,   246,     0,   902,   902,   902,   902,   902,     0,     0,
       0,     0,     0,     0,     0,   246,   902,     0,     0,     0,
     212,   894,     0,     0,     0,   358,     0,     0,     0,     0,
    1027,     0,   894,  1191,     0,   354,     0,   874,   894,     0,
       0,     0,   894,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   532,   533,     0,     0,     0,     0,   225,
       0,     0,     0,     0,     0,  1436,  1693,     0,     0,   246,
       0,     0,     0,  1214,     0,   225,   225,     0,     0,  1694,
     216,   217,   218,   219,   220,  1695,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   246,     0,   246,     0,
    1214,     0,   189,     0,     0,    91,  1696,     0,    93,    94,
     225,    95,  1697,    97,   894,     0,     0,   702,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   246,     0,     0,
       0,   354,   354,     0,     0,     0,     0,   108,     0,     0,
     354,   902,   227,   227,   246,     0,     0,     0,     0,     0,
     246,     0,     0,   252,   246,     0,  1275,     0,     0,     0,
       0,  1296,  1297,  1298,   212,     0,     0,     0,   246,   246,
       0,     0,   474,   475,   476,     0,     0,     0,     0,     0,
     248,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,  1089,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
       0,     0,     0,     0,   216,   217,   218,   219,   220,     0,
       0,   503,     0,     0,     0,     0,     0,   225,   225,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    93,    94,     0,    95,   190,    97,   591,     0,
     592,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   902,   902,   902,   902,   902,   902,   225,
     902,   108,     0,   902,   902,   902,   902,   902,   902,   902,
     902,   902,   902,   902,   902,   902,   902,   902,   902,   902,
     902,   902,   902,   902,   902,   902,   902,   902,   902,   902,
     902,     0,     0,  1131,   474,   475,   476,     0,     0,   597,
     354,   354,     0,     0,     0,     0,     0,     0,     0,   902,
       0,     0,     0,     0,   477,   478,     0,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,   227,   504,     0,  1038,   474,   475,   476,     0,
       0,     0,     0,   503,     0,     0,     0,     0,     0,     0,
       0,   225,     0,     0,     0,     0,   477,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,   718,   502,     0,   358,     0,     0,   354,     0,
       0,     0,     0,     0,     0,   503,     0,   212,     0,  1089,
    1089,  1089,  1089,  1089,  1089,     0,   354,   225,  1089,     0,
    1089,     0,     0,   354,   225,     0,     0,     0,     0,   354,
      50,     0,     0,     0,     0,     0,     0,     0,     0,   225,
     225,     0,   902,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,   902,   502,   902,   216,   217,   218,
     219,   220,     0,     0,     0,   354,     0,   503,     0,     0,
       0,   227,     0,     0,     0,     0,     0,   972,     0,     0,
     227,   902,   409,     0,     0,    93,    94,   227,    95,   190,
      97,     0,     0,     0,     0,     0,     0,     0,     0,   227,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     252,     0,     0,     0,   108,     0,     0,     0,   410,     0,
       0,  1577,     0,     0,   225,     0,     0,     0,     0,  1006,
       0,     0,   870,   474,   475,   476,     0,     0,     0,     0,
     354,     0,     0,     0,   354,     0,   874,     0,     0,   354,
       0,     0,     0,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,   475,   476,     0,  1089,     0,  1089,     0,     0,     0,
       0,     0,   503,     0,     0,   252,     0,     0,     0,     0,
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,   992,   993,
       0,     0,     0,     0,   227,     0,     0,     0,     0,   503,
     902,     0,   902,     0,   902,     0,     0,     0,     0,   902,
     225,     0,     0,     0,   902,   354,   902,   354,     0,   902,
       0,     0,     0,     0,     0,     0,     0,   212,     0,   213,
      40,     0,     0,  1689,     0,     0,  1702,     0,  1161,  1162,
    1163,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,   903,     0,     0,     0,     0,     0,     0,   354,  1164,
       0,   354,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,  1186,   903,     0,  1038,   216,   217,   218,
     219,   220,  1089,     0,  1089,     0,  1089,  1187,     0,     0,
       0,  1089,   289,   225,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   811,     0,    93,    94,     0,    95,   190,
      97,   354,     0,   902,     0,     0,     0,     0,   354,     0,
       0,     0,     0,     0,     0,  1769,  1770,     0,     0,   291,
       0,     0,     0,     0,   108,  1702,     0,     0,   812,     0,
       0,   112,   212,     0,     0,     0,     0,     0,  1001,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   227,     0,     0,    50,     0,  1140,   718,     0,
       0,     0,     0,     0,     0,     0,   289,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   354,   354,     0,     0,
       0,  1089,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   589,   216,   217,   218,   219,   220,   590,   902,   902,
     902,     0,     0,   291,     0,   902,     0,  1919,  1379,     0,
       0,     0,   354,     0,   189,  1702,   212,    91,   344,     0,
      93,    94,     0,    95,   190,    97,     0,   227,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   348,    50,
       0,     0,     0,     0,     0,     0,     0,  -435,     0,   108,
     350,     0,     0,     0,     0,  1226,   459,   186,   187,    65,
      66,    67,     0,     0,     0,     0,   227,     0,   227,     0,
       0,     0,     0,  1237,     0,   589,   216,   217,   218,   219,
     220,   590,     0,     0,     0,     0,  1256,   289,     0,   354,
     354,     0,     0,   354,     0,     0,   227,   903,   189,     0,
       0,    91,   344,     0,    93,    94,     0,    95,   190,    97,
       0,     0,     0,   903,   903,   903,   903,   903,     0,     0,
       0,     0,   348,     0,   291,     0,   903,     0,     0,   460,
       0,   354,  1288,   108,   350,     0,     0,   212,  1089,  1089,
       0,     0,   354,  1137,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   902,
      50,     0,     0,     0,     0,     0,     0,     0,     0,   227,
     902,     0,     0,     0,     0,     0,   902,     0,     0,     0,
     902,     0,     0,     0,     0,   227,   227,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   589,   216,   217,   218,
     219,   220,   590,     0,     0,     0,     0,  1347,     0,     0,
       0,  1351,     0,     0,   871,     0,  1355,   354,     0,   189,
     252,     0,    91,   344,     0,    93,    94,     0,    95,   190,
      97,     0, -1133,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   354,   348,     0,     0,     0,     0,     0,     0,
     904,   903,   902,     0,   108,   350,     0,     0,     0,     0,
       0,     0,     0,     0,   212,  2047,   354,     0,   354,     0,
       0,   474,   475,   476,   354,   872,     0,     0,     0,     0,
       0,     0,  1689,   934,     0,     0,     0,    50,     0,     0,
     252,   477,   478,     0,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
       0,     0,  1458,     0,   216,   217,   218,   219,   220,     0,
     503,     0,   354,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   189,   227,   227,    91,
       0,     0,    93,    94,     0,    95,   190,    97,     0,  1353,
       0,     0,     0,     0,     0,  1483,     0,     0,  1486,   212,
       0,   213,    40,     0,     0,     0,     0,     0,     0,     0,
       0,   108,     0,   903,   903,   903,   903,   903,   903,   252,
     903,     0,    50,   903,   903,   903,   903,   903,   903,   903,
     903,   903,   903,   903,   903,   903,   903,   903,   903,   903,
     903,   903,   903,   903,   903,   903,   903,   903,   903,   903,
     903,   212,     0,     0,     0,     0,     0,     0,  1535,   216,
     217,   218,   219,   220,     0,  1539,   354,     0,     0,   903,
       0,     0,     0,     0,    50,     0,     0,     0,     0,   354,
       0,     0,     0,   354,     0,   811,     0,    93,    94,     0,
      95,   190,    97,     0,  1064,     0,     0,     0,     0,     0,
    1706,     0,  1970,     0,     0,     0,     0,     0,     0,     0,
       0,   216,   217,   218,   219,   220,   108,     0,   212,     0,
     845,   227,     0,   112,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1574,  1575,     0,     0,     0,     0,    93,
      94,    50,    95,   190,    97,     0,  1119,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   354,     0,     0,     0,
       0,     0,  1143,  1144,  1145,  1146,  1147,     0,   108,  1707,
       0,     0,     0,     0,     0,  1158,     0,   252,   216,   217,
     218,   219,   220,   354,   227,   354,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   227,
     227,     0,   903,     0,     0,     0,    93,    94,     0,    95,
     190,    97,     0,   279,   280,     0,   281,   282,     0,     0,
     283,   284,   285,   286,   903,     0,   903,     0,     0,     0,
       0,     0,     0,     0,     0,   108,   739,   354,   287,   288,
       0,   354,     0,     0,     0,     0,  1666,  1667,     0,     0,
    1669,   903,     0,     0,     0,   354,   354,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   290,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   292,   293,   294,   295,   296,   297,   298,  1687,     0,
       0,   212,     0,     0,   227,     0,     0,   299,     0,  1713,
    1262,     0,     0,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,    50,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,     0,
     335,     0,   336,   337,   338,   339,     0,     0,     0,   340,
     600,   216,   217,   218,   219,   220,   601,   517,   518,   519,
     520,   521,   522,   523,   524,   525,   526,   527,   528,   529,
       0,     0,     0,   602,  1762,     0,     0,     0,     0,    93,
      94,     0,    95,   190,    97,   345,     0,   346,     0,     0,
     347,     0,     0,     0,     0,     0,     0,     0,   349,  1687,
       0,     0,   530,   531,     0,     0,     0,     0,   108,     0,
     903,     0,   903,     0,   903,     0,     0,     0,     0,   903,
     252,     0,     0,  1687,   903,  1687,   903,     0,     0,   903,
       0,  1687,  1147,  1368,     0,     0,  1368,     0,     0,     0,
       0,     0,  1382,  1385,  1386,  1387,  1389,  1390,  1391,  1392,
    1393,  1394,  1395,  1396,  1397,  1398,  1399,  1400,  1401,  1402,
    1403,  1404,  1405,  1406,  1407,  1408,  1409,  1410,  1411,  1412,
       0,     0,     0,     0,     0,  1066,  1067,     0,     0,   532,
     533,     0,     0,     0,     0,     0,     0,     0,  1422,  1921,
       0,     0,     0,     0,     0,  1068,     0,     0,     0,     0,
       0,     0,     0,  1069,  1070,  1071,   212,     0,     0,     0,
       0,     0,     0,   252,   474,   475,   476,  1072,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,   903,   477,   478,     0,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,     0,     0,     0,  1073,  1074,  1075,  1076,  1077,
    1078,     0,     0,   503,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1079,     0,     0,     0,     0,   189,     0,
       0,    91,    92,  1944,    93,    94,     0,    95,   190,    97,
       0,     0,     0,     0,     0,     0,  1955,     0,     0,     0,
    1687,     0,  1080,  1081,     0,     0,     0,     0,     0,     0,
       0,  1525,     0,   108,     0,     0,     0,     0,   903,   903,
     903,     0,     0,     0,     0,   903,     0,     0,     0,     0,
       0,     0,     0,  1543,  1924,  1544,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   474,   475,   476,     0,     0,     0,     0,     0,     0,
    1565,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   477,   478,  2015,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,  1205,   502,     0,
    1955,     0,  2028,   474,   475,   476,     0,     0,     0,     0,
     503,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   503,   544,   518,   519,   520,   521,   522,   523,
     524,   525,   526,   527,   528,   529,     0,     0,     0,   903,
       0,     0,     0,     0,     0,     0,     0,   212,     0,     0,
     903,     0,     0,     0,     0,     0,   903,     0,     0,     0,
     903,     0,     0,     0,     0,     0,     0,     0,   530,   531,
      50,     0,     0,     0,     0,     0,     0,     0,     0,  1671,
       0,  1672,     0,  1673,     0,     0,     0,     0,  1674,  2017,
       0,     0,     0,  1676,     0,  1677,     0,     0,  1678,     0,
       0,     0,     0,     0,  1282,     0,     0,   216,   217,   218,
     219,   220,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
    1186,     0,   903,     0,     0,    93,    94,     0,    95,   190,
      97,     0,     0,     0,  1187,   532,   533,     0,     5,     6,
       7,     8,     9,     0,     0,     0,  1293,     0,    10,     0,
       0,     0,     0,     0,   108,  1035,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,  1764,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,    56,    57,
      58,     0,    59,  -205,    60,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,  1912,  1913,  1914,
      86,     0,     0,    87,  1918,     0,     0,     0,    88,    89,
      90,    91,    92,     0,    93,    94,     0,    95,    96,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,   103,     0,   104,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,     0,     0,
     112,   113,   114,   115,     0,     0,   474,   475,   476,     0,
       0,     0,     0,  1026,   518,   519,   520,   521,   522,   523,
     524,   525,   526,   527,   528,   529,   477,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,     0,     0,     0,     0,   530,   531,
       0,     0,     0,     0,     0,   503,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,  1187,
       0,     0,     0,     0,     0,     0,     0,     0,  1979,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,  1989,
       0,     0,     0,     0,     0,  1994,     0,     0,     0,  1996,
       0,    14,     0,    15,    16,   532,   533,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,    56,    57,    58,  1324,
      59,  2039,    60,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,    88,    89,    90,    91,
      92,     0,    93,    94,     0,    95,    96,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,   103,     0,   104,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,  1224,     0,   112,   113,
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
      54,    55,    56,    57,    58,     0,    59,     0,    60,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,    88,    89,    90,    91,    92,     0,    93,    94,
       0,    95,    96,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,   103,     0,
     104,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,  1437,     0,   112,   113,   114,   115,     5,     6,
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
     107,     0,     0,   108,   109,     0,   110,   111,   704,     0,
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
     109,     0,   110,   111,  1915,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,  1966,    49,     0,
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
      47,  2009,    48,     0,    49,     0,     0,    50,    51,     0,
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
     110,   111,  2026,     0,   112,   113,   114,   115,     5,     6,
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
     107,     0,     0,   108,   109,     0,   110,   111,  2029,     0,
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
     109,     0,   110,   111,  2046,     0,   112,   113,   114,   115,
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
    2101,     0,   112,   113,   114,   115,     5,     6,     7,     8,
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
       0,   108,   109,     0,   110,   111,  2102,     0,   112,   113,
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
       0,     0,     0,     0,    11,    12,    13,     0,     0,  1758,
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
      11,    12,    13,     0,     0,  1907,     0,     0,     0,     0,
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
       0,     0,   112,   113,   114,   115,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,   290,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1187,   292,   293,   294,   295,   296,   297,   298,     0,     0,
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
       0,   290,     0,     0,     0,   456,     0,    93,    94,     0,
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
       0,   343,  1130,     0,    91,   344,     0,    93,    94,     0,
      95,   190,    97,   345,    50,   346,     0,     0,   347,     0,
     279,   280,     0,   281,   282,   348,   349,   283,   284,   285,
     286,     0,     0,     0,     0,     0,   108,   350,     0,     0,
       0,  1886,     0,     0,     0,   287,   288,     0,   289,     0,
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
    1961,     0,     0,     0,   287,   288,     0,   289,     0,     0,
     216,   217,   218,   219,   220,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   189,   290,     0,    91,    92,     0,    93,    94,
       0,    95,   190,    97,   291,     0,     0,   292,   293,   294,
     295,   296,   297,   298,     0,     0,     0,   212,     0,     0,
       0,     0,     0,   299,     0,     0,     0,   108,     0,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
      50,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,     0,   335,     0,   336,   337,
     338,   339,     0,     0,     0,   340,   341,   216,   217,   218,
     219,   220,   342,     0,     0,     0,     0,     0,     0,   212,
       0,   966,     0,   967,     0,     0,     0,     0,     0,   343,
       0,     0,    91,   344,     0,    93,    94,     0,    95,   190,
      97,   345,    50,   346,     0,     0,   347,     0,   279,   280,
       0,   281,   282,   348,   349,   283,   284,   285,   286,     0,
       0,     0,     0,     0,   108,   350,     0,     0,     0,     0,
       0,     0,     0,   287,   288,     0,   289,     0,     0,   216,
     217,   218,   219,   220,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,   290,   502,     0,     0,     0,    93,    94,     0,
      95,   190,    97,   291,     0,   503,   292,   293,   294,   295,
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
     345,    50,   346,     0,     0,   347,     0,  1784,  1785,  1786,
    1787,  1788,   348,   349,  1789,  1790,  1791,  1792,     0,     0,
       0,     0,     0,   108,   350,     0,     0,     0,     0,     0,
       0,  1793,  1794,  1795,     0,     0,     0,     0,   216,   217,
     218,   219,   220,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
    1186,  1796,     0,   370,     0,     0,    93,    94,     0,    95,
     190,    97,     0,     0,  1187,  1797,  1798,  1799,  1800,  1801,
    1802,  1803,     0,     0,     0,   212,     0,     0,     0,     0,
       0,  1804,     0,     0,     0,   108,     0,  1805,  1806,  1807,
    1808,  1809,  1810,  1811,  1812,  1813,  1814,  1815,    50,  1816,
    1817,  1818,  1819,  1820,  1821,  1822,  1823,  1824,  1825,  1826,
    1827,  1828,  1829,  1830,  1831,  1832,  1833,  1834,  1835,  1836,
    1837,  1838,  1839,  1840,  1841,  1842,  1843,  1844,  1845,  1846,
       0,     0,     0,  1847,  1848,   216,   217,   218,   219,   220,
       0,  1849,  1850,  1851,  1852,  1853,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1854,  1855,  1856,
       0,   212,     0,    93,    94,     0,    95,   190,    97,  1857,
       0,  1858,  1859,     0,  1860,     0,     0,     0,     0,     0,
       0,  1861,     0,  1862,    50,  1863,     0,  1864,  1865,     0,
     279,   280,   108,   281,   282,     0,     0,   283,   284,   285,
     286,     0,     0,     0,     0,     0,     0,  1693,     0,     0,
       0,     0,     0,     0,     0,   287,   288,     0,     0,     0,
    1694,   216,   217,   218,   219,   220,  1695,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   189,   290,     0,    91,    92,     0,    93,
      94,     0,    95,  1697,    97,     0,     0,     0,   292,   293,
     294,   295,   296,   297,   298,     0,     0,     0,   212,     0,
       0,     0,     0,     0,   299,     0,     0,     0,   108,     0,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,    50,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   212,   335,     0,  1380,
     337,   338,   339,     0,     0,     0,   340,   600,   216,   217,
     218,   219,   220,   601,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,   279,   280,     0,   281,   282,     0,
     602,   283,   284,   285,   286,     0,    93,    94,     0,    95,
     190,    97,   345,     0,   346,     0,     0,   347,     0,   287,
     288,     0,     0,     0,     0,   349,   216,   217,   218,   219,
     220,     0,     0,     0,     0,   108,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   290,     0,
       0,   924,     0,     0,    93,    94,     0,    95,   190,    97,
       0,     0,   292,   293,   294,   295,   296,   297,   298,     0,
       0,     0,   212,     0,     0,     0,     0,     0,   299,     0,
       0,     0,     0,   108,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,    50,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
       0,   335,     0,     0,   337,   338,   339,     0,     0,     0,
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
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,   474,   475,
     476,     0,     0,     0,     0,     0,     0,     0,     0,   503,
       0,     0,     0,     0,     0,     0,     0,     0,   477,   478,
       0,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,   474,   475,   476,     0,
       0,     0,     0,     0,     0,     0,     0,   503,     0,     0,
       0,     0,     0,     0,     0,     0,   477,   478,  1530,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,     0,  1603,     0,   474,   475,   476,
       0,     0,     0,     0,     0,   503,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   477,   478,     0,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,  1725,   502,   474,   475,   476,     0,     0,
       0,     0,     0,     0,     0,     0,   503,     0,     0,     0,
       0,     0,     0,     0,     0,   477,   478,     0,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,  1726,   502,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   503,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   474,   475,   476,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   477,   478,  1531,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,     0,     0,     0,   474,   475,   476,     0,
       0,     0,     0,     0,   503,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   477,   478,   504,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,   474,   475,   476,     0,     0,     0,
       0,     0,     0,     0,     0,   503,     0,     0,     0,     0,
       0,     0,     0,     0,   477,   478,   586,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   474,   475,   476,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   477,   478,   588,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,     0,     0,     0,   474,   475,   476,     0,     0,
       0,     0,     0,   503,     0,     0,   289,     0,     0,     0,
       0,     0,     0,     0,     0,   477,   478,   607,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,   291,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   503,   289,   212,     0,     0,     0,
       0,     0,  1538,     0,     0,   611,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   291,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   289,     0,   212,     0,     0,     0,     0,
       0,  1460,     0,     0,     0,   589,   216,   217,   218,   219,
     220,   590,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,   825,     0,     0,     0,   189,     0,
     291,    91,   344,     0,    93,    94,     0,    95,   190,    97,
       0, -1133,     0,   212,     0,     0,     0,     0,     0,     0,
       0,     0,   348,     0,   589,   216,   217,   218,   219,   220,
     590,     0,     0,   108,   350,     0,    50,     0,     0,     0,
       0,     0,     0,  1388,     0,   850,     0,   189,     0,     0,
      91,   344,     0,    93,    94,     0,    95,   190,    97,     0,
       0,   880,   881,     0,     0,     0,     0,   882,     0,   883,
       0,   348,   589,   216,   217,   218,   219,   220,   590,     0,
       0,   884,   108,   350,     0,     0,     0,     0,     0,    34,
      35,    36,   212,     0,     0,   189,     0,     0,    91,   344,
       0,    93,    94,   214,    95,   190,    97,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,   348,
       0,     0,     0,     0,     0,  1115,     0,     0,     0,     0,
     108,   350,     0,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,
       0,   885,   886,   887,   888,   889,   890,    29,    81,    82,
      83,    84,    85,  1187,     0,    34,    35,    36,   212,   221,
     213,    40,     0,     0,   189,    89,    90,    91,    92,   214,
      93,    94,     0,    95,   190,    97,     0,     0,     0,    99,
       0,    50,     0,     0,     0,     0,     0,     0,   891,   892,
       0,     0,     0,     0,   105,     0,     0,     0,   215,   108,
     893,     0,     0,   880,   881,     0,     0,     0,     0,   882,
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
       0,   215, -1134, -1134, -1134, -1134,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,     0,
      75,   216,   217,   218,   219,   220,    29,    81,    82,    83,
      84,    85,  1187,     0,    34,    35,    36,   212,   221,   213,
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
       0,     0,     0,   547,   477,   478,     0,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,     0,     0,     0,     0,
       0,   556,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,   474,   475,   476,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   956,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,   474,   475,   476,     0,     0,     0,     0,     0,     0,
       0,     0,   503,     0,     0,     0,     0,     0,     0,     0,
    1043,   477,   478,     0,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     503,     0,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1099,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
    1161,  1162,  1163,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,     0,     0,     0,     0,     0,     0,  1435,
       0,  1164,     0,     0,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1187,
    1161,  1162,  1163,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1478,  1164,     0,     0,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1161,  1162,  1163,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1187,
       0,     0,     0,     0,     0,     0,     0,  1164,  1361,     0,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1187,  1161,  1162,  1163,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1164,  1549,     0,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,  1161,  1162,  1163,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1187,     0,     0,     0,     0,
       0,     0,     0,  1164,  1561,     0,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1187,  1161,  1162,  1163,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1164,  1670,     0,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,    34,    35,
      36,   212,     0,   213,    40,     0,     0,     0,     0,     0,
       0,  1187,   214,     0,     0,     0,     0,     0,     0,     0,
    1765,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   243,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   244,     0,     0,     0,     0,     0,     0,     0,
       0,   216,   217,   218,   219,   220,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   221,     0,
    1767,     0,     0,   189,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   190,    97,     0,     0,     0,    99,     0,
      34,    35,    36,   212,     0,   213,    40,     0,     0,     0,
       0,     0,     0,   105,   678,     0,     0,     0,   108,   245,
       0,     0,     0,     0,     0,   112,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   215,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,   216,   217,   218,   219,   220,     0,    81,
      82,    83,    84,    85,   503,     0,    34,    35,    36,   212,
     221,   213,    40,     0,     0,   189,    89,    90,    91,    92,
     214,    93,    94,     0,    95,   190,    97,     0,     0,     0,
      99,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   105,     0,     0,     0,   243,
     108,   679,     0,     0,     0,     0,     0,   112,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   216,
     217,   218,   219,   220,     0,    81,    82,    83,    84,    85,
     212,     0,     0,     0,     0,     0,   221,     0,     0,     0,
       0,   189,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   190,    97,    50,     0,     0,    99,     0,     0,     0,
       0,   367,   368,     0,     0,     0,     0,     0,     0,     0,
       0,   105,     0,     0,     0,     0,   108,   245,     0,     0,
       0,     0,     0,   112,     0,     0,     0,     0,     0,     0,
     216,   217,   218,   219,   220,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   369,     0,     0,   370,     0,     0,    93,    94,
       0,    95,   190,    97,     0,   474,   475,   476,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   371,     0,
       0,     0,   862,     0,     0,   477,   478,   108,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   503,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   474,
     475,   476,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   863,   477,
     478,  1040,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,   474,   475,   476,
       0,     0,     0,     0,     0,     0,     0,     0,   503,     0,
       0,     0,     0,     0,     0,     0,     0,   477,   478,     0,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,  1161,  1162,  1163,     0,     0,
       0,     0,     0,     0,     0,     0,   503,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1164,  1566,     0,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
    1186,  1161,  1162,  1163,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1187,     0,     0,     0,     0,     0,
       0,     0,  1164,     0,     0,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,  1162,  1163,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1187,     0,     0,     0,     0,     0,     0,  1164,     0,     0,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,   476,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1187,     0,     0,     0,     0,
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,  1163,   502,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   503,
       0,     0,     0,     0,     0,  1164,     0,     0,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   477,   478,  1187,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   478,
     503,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1164,     0,   503,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1187,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     503,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1187,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     503,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   503, -1134, -1134, -1134, -1134,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   503
};

static const yytype_int16 yycheck[] =
{
       5,     6,   193,     8,     9,    10,    11,    12,    13,    56,
      15,    16,    17,    18,   132,     4,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   167,    31,   734,   417,   696,
     109,   172,   173,   109,   124,   572,    44,   109,   124,    44,
      98,   109,    33,   124,   853,   103,   104,    52,   417,    54,
       4,   997,    57,    30,    59,    46,   723,   999,     4,     4,
      51,   508,   692,   560,   417,   448,    30,   257,    30,   791,
     417,   608,    30,   131,   505,   506,   194,   534,    86,  1238,
     861,    86,   502,   167,   987,   621,   622,  1427,   541,    57,
     837,   246,   537,   538,   534,  1034,  1019,   671,  1021,  1228,
    1114,   868,     9,     4,   109,     9,    57,   693,   539,     9,
       9,   568,   191,  1052,    14,   191,     9,   258,     9,   191,
       9,   566,    32,   367,   368,   369,  1900,   371,   568,    36,
      32,    70,     9,     9,     9,     9,     9,    14,    14,     9,
       9,   878,     9,   222,     9,     9,   222,     9,     9,  1579,
     222,     9,    53,     9,     9,    56,     9,   450,   451,   452,
      83,  1100,    83,    48,  1110,    83,     4,     9,    48,   245,
       9,   103,    73,   245,     9,     9,   565,    48,    14,   162,
      91,   116,    48,    91,   162,   183,   191,   136,   137,   136,
     137,    14,    38,   198,    31,    96,    48,    98,   103,    50,
      51,   199,   103,   104,   199,   162,   123,   136,   137,    32,
      70,    38,   402,    50,     0,   132,    53,   222,   398,   202,
     400,   199,    54,   162,    38,   103,   183,   199,    51,   112,
     131,  1960,    57,  1962,   166,    38,    38,    83,   418,    70,
     245,   162,   160,   161,    69,   180,    70,    54,   159,    70,
      70,   159,   199,  1156,   203,   260,    83,    70,   263,   926,
    1690,   166,   270,   202,  1001,   270,   271,   103,   104,    83,
     461,     4,    70,    70,    70,   264,   136,   137,    70,   268,
      83,    83,   205,   183,  1714,    70,  1716,     4,   166,    70,
     997,  2065,   202,   200,   132,    70,    70,   201,   202,   201,
      70,   201,   201,    70,    70,  2079,   207,   200,  1055,   200,
     389,   358,   201,   389,  1473,   200,   196,   389,    70,  1242,
     200,   858,  1029,  1337,   201,   201,   201,   201,   201,   200,
     196,   201,   201,   184,   201,   167,   201,   201,   162,   201,
     201,  1681,   202,   201,   200,   200,  1475,   200,   200,   176,
     182,  1290,   543,  1482,   359,  1484,   194,  1124,   200,  1126,
     167,   200,   176,   264,   162,   200,   200,   268,   199,   448,
     202,   272,   448,   176,   176,   199,   448,   162,   202,   199,
     388,   202,   202,   388,   389,  1514,   922,   923,    81,   202,
     395,   396,   397,   398,   399,   400,    70,   455,   516,   446,
     405,   975,   199,  1110,   202,   202,   202,   199,    70,   122,
     202,  1314,   168,   418,   199,    19,    20,   202,   199,   132,
     425,   202,    31,    70,     8,   199,   200,   202,   433,   168,
      50,    51,   202,    70,   167,   202,   202,    83,    84,   132,
     445,    50,   181,   448,    53,  1031,   202,    70,    83,   517,
     202,    83,    84,   511,   512,   513,   514,   358,   463,   464,
      70,   199,  1621,   202,  1623,   199,   107,   108,   199,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   448,   503,   952,
     505,   506,   507,  1632,  1226,   107,   108,    83,  1245,   699,
      70,   701,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   502,   136,   137,   202,    70,
     103,   432,   537,   538,   539,   540,    83,   199,   502,   544,
     502,   442,   547,   990,   502,   446,    83,   183,  1005,   517,
      83,   556,  1333,   558,   455,   202,   202,    83,    91,   205,
     195,   566,   203,   199,   183,  1005,   517,   202,   710,   574,
     166,   576,    38,   205,    83,    84,   136,   137,  1737,    83,
     199,   199,  1741,    83,  1121,    75,    76,    91,   565,   202,
     579,    91,   202,   166,   432,   136,   137,    70,   987,   199,
     176,   203,   793,   679,  1049,   201,   202,   508,   509,   510,
     511,   512,   513,   514,  1244,  1955,    83,    83,   987,  1356,
     199,   162,   627,   574,    91,   508,   710,   160,   161,   176,
     785,   183,    83,   534,   987,   239,  1019,  1057,  1021,   176,
     987,   831,   832,   162,    83,   199,  1103,   199,   838,   839,
     176,   534,    91,   554,  1557,   199,   160,   161,   199,   159,
     160,   161,   817,  1103,   183,   162,  1202,   568,   912,   199,
     914,   554,   916,    83,   679,   208,   920,   515,   579,    83,
     199,    91,   565,   202,   742,   568,   183,    91,  1274,   199,
     183,  1277,   159,   160,   161,    75,    76,   838,   599,   432,
     367,   368,   369,   207,   371,   202,   199,   424,   159,   160,
     161,  1309,  1159,  1311,    70,    60,   721,  1315,   106,   107,
     108,   160,   161,  1460,   625,   626,    87,  1020,   103,   104,
    1023,  1024,   121,   122,   123,   124,   125,   126,    87,   894,
    1899,   134,   135,    88,  1903,   750,    91,  1650,   201,   202,
     160,   161,   907,   201,   202,     4,   160,   161,    70,   660,
     661,  1483,    70,   124,   125,   126,    70,  1156,  1931,  1932,
      19,    20,   550,   167,   378,   124,   125,   126,   783,   199,
    1153,   199,  1155,   387,  1157,   162,    32,  1156,   166,   166,
     394,   199,    53,    54,    55,  1292,   121,   122,   123,   124,
     125,   126,   406,  1156,   199,   194,   183,   812,    69,  1156,
    1927,  1928,   201,   417,   106,   107,   108,   118,   112,    19,
      20,  1451,  1004,  1005,    81,    38,  1257,   121,   122,   123,
     124,   125,   126,   734,   201,    53,    54,    55,  1283,    57,
     845,   742,   113,   114,   115,    70,   201,   104,   201,  1294,
     201,    69,   202,   861,   201,   201,   861,   201,   201,  1242,
     109,    70,    70,    70,    70,    70,   857,   162,    70,   194,
     199,    70,  1594,   199,    70,   653,   654,   866,  1415,   162,
     199,   166,    49,  1336,   141,   142,   143,   144,   145,    69,
     201,  1477,   183,  2052,   162,   204,   734,   199,     9,   199,
     194,   199,  1609,   681,   162,     8,   163,   162,   201,   166,
    2069,   199,   169,   170,   162,   172,   173,   174,   121,   122,
     123,   124,   125,   126,   124,  1314,   827,    14,   829,   132,
     133,   162,   201,     9,   201,    14,   202,   541,   716,   201,
     132,   198,   191,   132,   200,  1314,   203,   183,    14,   103,
     851,   956,   206,   958,   199,   960,   200,   200,   200,   200,
     112,  1314,   202,     9,   865,   866,   971,  1314,   159,  1691,
     173,   199,   175,   222,   200,   199,   693,   710,  1431,  1516,
     200,   986,    95,   200,   200,   974,   189,     9,   191,   201,
     239,   194,     9,    14,    83,  1532,   245,   183,  1056,   134,
     201,   734,   199,   781,   199,     9,   204,   201,   909,   202,
       9,  1016,   202,   851,  1019,   264,  1021,   918,   919,   268,
     974,  1026,   367,   368,   369,   370,   371,   202,   974,   974,
     201,   200,   202,   201,  1479,  1040,   200,   200,  1043,   239,
    1045,   201,   199,   204,  1049,   200,   204,  1633,   949,    32,
      70,   403,   135,   204,  2000,   407,   204,   182,   162,  2005,
     138,     9,   200,   162,   409,    14,   200,   196,  1019,  1452,
    1021,   909,     9,   974,     9,  1026,   184,   200,     9,  2021,
    1057,    14,     9,   435,  2030,   437,   438,   439,   440,   990,
     200,   200,   870,  1057,  1099,  1057,   997,  1252,   876,  1057,
    1255,   134,  2044,  1004,  1005,   200,   200,   990,   204,     9,
     203,  2053,   204,   204,   200,   719,  1107,  1654,   851,  1108,
      14,  1004,  1005,   200,   204,   199,  1663,   162,  1029,   378,
     200,   103,  1579,   201,   201,     9,   974,   138,   387,   162,
     389,     9,  1679,  1952,   200,   394,  2092,   199,  1957,    70,
      70,  1219,  1242,    70,    70,  1056,  1242,   406,    70,   997,
     199,  1242,  1455,  1456,  1457,  1066,  1067,  1068,  1557,   199,
     948,   202,  1617,  1618,     9,   203,   909,    14,   378,   184,
       9,    14,   201,   432,   202,   827,   202,   387,  1557,   389,
     794,  1029,   204,    14,   394,   176,    32,  2006,   200,   448,
    1101,   201,  1103,   196,  1557,    70,   406,  1108,   199,  1110,
    1557,  1112,   199,    32,  1219,    14,    14,   199,  1101,   199,
    1103,  1758,    52,   199,    70,  1230,    70,    70,    70,   833,
      70,   835,  1133,   199,   199,     9,  1225,  1242,   162,   200,
    1687,   974,   201,  1690,    50,    51,    52,    53,    54,    55,
     201,    57,  1257,   199,   138,  1260,    14,   974,  1159,   863,
     138,  1650,   184,    69,   997,   162,     9,  1714,  1219,  1716,
     200,  1225,  1110,  2082,  1112,  1722,   621,   622,  1283,  1225,
    1225,  1650,   999,   176,  1951,   176,  1953,  1188,    69,  1294,
    1295,  1242,   541,  2000,    19,    20,  1029,  1650,  2005,   204,
       9,    83,   203,  1650,   203,    30,   201,   203,   121,   122,
     123,   124,   125,   126,  1031,  1456,  1094,   203,  2055,   132,
     133,     9,  2059,  2030,  1225,  1333,   199,   138,  1333,   199,
     579,    56,   936,    83,  1325,   201,  2073,  2074,  1343,  1328,
      14,   541,    50,    51,    52,    53,    54,    55,   952,   953,
      83,   200,   202,   200,   199,   138,   204,     9,   199,   159,
    1138,    69,   175,   202,  1142,    92,    32,    77,   202,   201,
    1907,  1149,   184,  1452,   202,  2042,  1452,  1110,   201,  1112,
    1452,   194,   200,   987,   201,  2092,   138,  1225,    32,   204,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,   200,   200,  1306,  1307,  1308,  1309,  1310,
    1311,     9,   204,     9,  1315,   204,  1317,   204,   204,   138,
       9,   200,   200,   203,     9,   200,  1068,  1328,   201,   201,
    1435,   201,   201,    14,   202,    59,    60,  1442,   203,  1340,
     201,    83,  1447,   199,  1449,   827,   199,  1452,   199,  1350,
     204,   200,   200,  1595,  1901,   200,   202,   201,   200,   200,
    1465,     9,   138,     9,   204,   204,   204,   204,   204,   138,
     719,   200,  1590,  1478,  1479,     9,   200,    32,   201,   200,
     200,   138,   176,   201,   201,   734,   202,   113,   171,   201,
     167,    83,  1225,    14,   119,    83,   200,   200,   138,   200,
     202,  1452,  1340,   138,    14,   183,   202,    83,  1225,   201,
    1114,  1115,   136,   137,   239,    14,    14,    83,   200,   719,
     200,  1712,  1423,   138,   199,   198,  1304,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     200,   138,   201,  2080,    14,   794,   201,    14,   201,    14,
       9,   202,  1156,     9,   203,    68,    83,  1274,   199,    83,
    1277,   183,     9,     9,   289,   202,   291,   912,   201,   914,
     103,   916,    59,    60,   116,   920,   200,   922,   923,   924,
     162,   103,   184,   174,   833,    36,   835,    14,   199,  1367,
    1491,   200,  1493,  1371,   794,   201,   199,   180,  1376,    83,
     184,   200,   851,   177,  1609,   184,  1384,  1340,     9,  1614,
      83,   201,  1617,  1618,   863,    83,   202,   866,    14,    83,
     200,    83,   200,    14,    14,   350,  1684,    14,    83,    78,
      79,    80,  1202,   833,  2033,   835,   511,   514,    83,    83,
     509,  1050,   977,    92,  2049,  1334,  1757,  1529,  2044,   136,
     137,  1259,   629,   378,  1258,  1744,  1782,  1692,  1869,  1585,
     909,  2090,   387,   863,  1306,  1307,  2066,  1568,  1310,   394,
    1740,  1660,  1881,   399,  1154,  1317,  1310,  1067,  1579,  1229,
    1150,   406,  1306,  1581,  1585,  1096,  1068,   936,    19,    20,
    1305,   446,   417,  1945,  1885,   395,  1429,   879,   147,   148,
     149,   150,   151,   952,   953,  2001,  1439,  1991,  1609,   158,
    1314,  1210,  1429,   200,  1573,   164,   165,  1321,   443,  1134,
    1498,   446,  1439,  1188,  1502,   974,  1731,    -1,    -1,   178,
    1568,  1509,  1336,  1337,    -1,    -1,   936,    -1,  1639,    -1,
    1641,    -1,  1643,    -1,   193,    -1,    -1,  1648,   997,    -1,
      -1,  1589,   952,   953,  1655,    -1,    -1,    -1,    -1,  1660,
    1477,    -1,    -1,  1664,    -1,    -1,    -1,  1756,  1757,    -1,
    1019,  1609,  1021,    -1,    -1,    -1,    -1,   502,    -1,    -1,
    1029,    -1,    -1,  1684,    -1,    -1,  1687,    -1,    -1,  1690,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1700,
      -1,    -1,    -1,    -1,    -1,    -1,  1707,    -1,    -1,    -1,
      -1,    -1,    -1,  1714,    -1,  1716,   541,  1655,    -1,    -1,
      -1,  1722,    -1,    -1,    -1,    -1,  1664,  1431,    -1,    -1,
      -1,    -1,    -1,  1880,    -1,  1568,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1748,    -1,  1491,
      -1,  1493,    -1,    -1,  1755,  1756,  1757,  1202,    -1,  1108,
    1761,  1110,  1595,  1112,    -1,  1114,  1115,  1768,   593,    -1,
     595,    -1,  2014,   598,    -1,    -1,  1609,    -1,    -1,    -1,
      -1,    -1,  1720,    -1,    -1,  1890,    -1,    -1,    -1,  1622,
      -1,    -1,    -1,    -1,    -1,  1628,    -1,  1630,  1945,    -1,
      -1,    -1,    -1,    -1,    -1,  1622,   631,    -1,   239,    -1,
      -1,  1628,    -1,  1630,  1114,  1115,  1633,  1755,    -1,  2037,
    1653,    -1,  1655,  1761,  1306,  1307,  1308,  1309,  1310,  1311,
    1768,  1664,    -1,  1315,    -1,  1317,  1653,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,  1557,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   689,   690,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   698,    -1,    -1,  1225,    -1,    -1,  1880,
      -1,    -1,    59,    60,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1242,   719,    -1,    -1,  1639,    -1,  1641,
    1901,  1643,    -1,    -1,  1905,  1906,  1648,    -1,    -1,  1258,
    1911,    -1,    -1,  1746,    -1,    -1,    -1,    -1,    -1,  1920,
      -1,    -1,  1755,    -1,    -1,    -1,  1927,  1928,  1761,  1746,
    1931,  1932,    -1,    -1,    -1,  1768,    -1,    -1,    -1,    -1,
      -1,    -1,  1242,    -1,  1945,    -1,  1650,   378,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   387,    -1,  1258,   136,
     137,    -1,    -1,   394,  1965,    -1,    -1,    -1,    -1,   794,
      -1,    -1,  1321,  1911,    -1,   406,    -1,    -1,    -1,  1328,
       6,    -1,    -1,  2088,    -1,    -1,    -1,  1336,  1337,    -1,
      -1,  1340,  2097,    -1,    -1,    -1,    -1,    -1,    -1,  2000,
    2105,    -1,   827,  2108,  2005,    -1,  1748,    -1,   833,  1491,
     835,  1493,  2013,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1321,    48,   200,    -1,    -1,    -1,  1965,    -1,  2030,
      -1,    -1,    -1,    -1,    -1,  2036,  1336,  1337,   863,   864,
      -1,    -1,    -1,  1921,  1922,    -1,   871,    -1,    -1,    -1,
      -1,    -1,    -1,   878,   879,   880,   881,   882,   883,   884,
      -1,    -1,  2000,    -1,    -1,    -1,    -1,  2005,   893,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1911,    -1,
      -1,    -1,  1431,    -1,    -1,   910,    -1,   113,    -1,    -1,
    2091,  2092,  2030,   119,    -1,   121,   122,   123,   124,   125,
     126,   127,    -1,  1452,    -1,    -1,    -1,    -1,    -1,    -1,
     541,   936,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,   950,    57,   952,   953,    -1,
      -1,  1431,  1965,    -1,    -1,    -1,    -1,    31,    69,    -1,
      -1,    -1,    -1,   169,   170,    -1,   172,    -1,    -1,    -1,
      -1,   976,   977,  2091,  2092,    -1,    -1,  1639,    -1,  1641,
      -1,  1643,   987,  1905,  1906,    -1,  1648,  2000,   194,   994,
      -1,    -1,  2005,    -1,    68,    -1,  1001,   203,    -1,    -1,
      -1,  2014,  1007,    -1,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,  1018,    -1,    -1,    -1,  2030,    -1,    -1,
      -1,    -1,    -1,     6,  2021,    -1,    -1,    -1,  1033,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,  1568,
      -1,    -1,    19,    20,    -1,    -1,    -1,  2044,  1053,    -1,
      -1,    -1,  1057,    30,    -1,    -1,  2053,    -1,    -1,    -1,
      -1,    -1,    -1,  1068,    -1,    48,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,  2091,  2092,
    1609,    -1,    -1,    -1,    -1,    -1,  1748,    -1,    -1,   163,
      -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   719,  1114,
    1115,    -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1128,   198,   199,  1655,  1132,    -1,  1134,
     113,  1660,  1137,    -1,    -1,  1664,   119,    -1,   121,   122,
     123,   124,   125,   126,   127,  1150,  1151,  1152,  1153,  1154,
    1155,  1156,  1157,    -1,    -1,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,  1187,   794,    -1,    31,   169,   170,    -1,   172,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     203,    -1,   833,    -1,   835,    -1,  1755,  1756,  1757,    -1,
      -1,    -1,  1761,    -1,    -1,    81,    -1,    -1,  1243,  1768,
    1245,    -1,    -1,  1905,  1906,    -1,    92,    -1,    -1,    -1,
       6,    -1,   863,  1258,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,   239,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     6,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1286,    -1,    -1,  1289,    -1,    -1,     6,    -1,    -1,
      -1,    -1,    48,    -1,    -1,   141,   142,   143,   144,   145,
      -1,  1306,  1307,  1308,  1309,  1310,  1311,    -1,    -1,  1314,
    1315,    -1,  1317,    48,    -1,    -1,  1321,   163,    -1,    -1,
     166,    -1,    -1,   169,   170,   936,   172,   173,   174,    48,
     176,  1336,  1337,    -1,  1339,    -1,    -1,    -1,    -1,    -1,
      -1,   952,   953,    -1,  1349,    -1,    -1,    -1,    -1,    -1,
      -1,  1356,   198,    -1,    -1,    -1,  1361,   113,  1363,    -1,
      -1,    -1,    -1,   119,    -1,   121,   122,   123,   124,   125,
     126,   127,    -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,
      -1,    -1,  1911,  1388,   119,    -1,   121,   122,   123,   124,
     125,   126,   127,    -1,   113,    -1,    -1,    -1,    56,    -1,
     119,   378,   121,   122,   123,   124,   125,   126,   127,    -1,
     387,    -1,    -1,   169,   170,    -1,   172,   394,    81,  1424,
    1425,    -1,    -1,  1428,    -1,    -1,  1431,    -1,    -1,   406,
      -1,    -1,    -1,    -1,   169,   170,  1965,   172,   194,    -1,
     417,   104,    -1,    -1,    -1,    -1,    -1,   203,    -1,    -1,
     169,   170,     6,   172,    -1,  1460,    -1,    -1,    -1,   194,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   203,    -1,
      -1,  2000,    -1,    -1,    -1,   194,  2005,    -1,   141,   142,
     143,   144,   145,    -1,   203,    -1,  1491,     6,  1493,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,
      -1,  2030,    -1,  1114,  1115,    -1,   169,   170,    -1,   172,
     173,   174,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,   502,    -1,    -1,    -1,    48,
      -1,    -1,  1537,  1538,    -1,   198,  1541,    -1,    69,    -1,
      -1,    -1,  1547,    -1,  1549,    -1,  1551,    -1,    -1,    -1,
      -1,  1556,  1557,    -1,    -1,    -1,  1561,    -1,  1563,   113,
      -1,  1566,  2091,  2092,   541,   119,    -1,   121,   122,   123,
     124,   125,   126,   127,  1579,  1580,    -1,    -1,  1583,    -1,
      -1,    -1,    -1,    -1,    -1,  1590,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   113,    -1,    -1,    -1,    -1,    -1,
     119,    -1,   121,   122,   123,   124,   125,   126,   127,    -1,
      -1,    -1,    -1,    -1,    -1,   169,   170,    -1,   172,    -1,
      -1,   598,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   289,    -1,   291,  1639,    -1,  1641,    -1,  1643,    -1,
     194,    81,    -1,  1648,    -1,  1650,    -1,  1258,    -1,   203,
     169,   170,    -1,   172,   631,    -1,    -1,    -1,    -1,    -1,
    1665,    -1,    -1,    -1,   104,  1670,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   194,    -1,  1682,  1683,    -1,
      -1,    -1,    -1,    -1,   203,  1690,    -1,  1692,    -1,    -1,
      -1,    -1,   350,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,  1714,
    1321,  1716,    -1,    -1,    -1,    -1,    -1,  1722,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1336,  1337,   167,    -1,   169,
     170,   171,   172,   173,   174,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   719,  1748,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,   198,   199,
    1765,  1766,  1767,    -1,    -1,    -1,    -1,  1772,    -1,  1774,
      -1,    10,    11,    12,    -1,  1780,    -1,  1782,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   443,    -1,    -1,   446,    59,
      60,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,   794,    57,    -1,
    1431,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,
     827,    -1,    -1,    -1,    -1,    -1,   833,    -1,   835,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   136,   137,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    59,    60,    -1,  1884,
      -1,    -1,    -1,    -1,    -1,    -1,   863,   864,    -1,    -1,
      -1,    -1,  1897,    -1,    -1,    -1,  1901,    -1,    -1,    -1,
    1905,  1906,    -1,   880,   881,   882,   883,   884,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1920,   893,    -1,    -1,    -1,
      81,  1926,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,
     200,    -1,  1937,   910,    -1,   593,    -1,   595,  1943,    -1,
      -1,    -1,  1947,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   136,   137,    -1,    -1,    -1,    -1,   936,
      -1,    -1,    -1,    -1,    -1,   204,   127,    -1,    -1,  1974,
      -1,    -1,    -1,   950,    -1,   952,   953,    -1,    -1,   140,
     141,   142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  2001,    -1,  2003,    -1,
     977,    -1,   163,    -1,    -1,   166,   167,    -1,   169,   170,
     987,   172,   173,   174,  2019,    -1,    -1,   200,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  2032,    -1,    -1,
      -1,   689,   690,    -1,    -1,    -1,    -1,   198,    -1,    -1,
     698,  1018,    19,    20,  2049,    -1,    -1,    -1,    -1,    -1,
    2055,    -1,    -1,    30,  2059,    -1,  1033,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    -1,    -1,    -1,  2073,  2074,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
    1057,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,  1068,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,  1114,  1115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   169,   170,    -1,   172,   173,   174,   289,    -1,
     291,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1150,  1151,  1152,  1153,  1154,  1155,  1156,
    1157,   198,    -1,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,
    1187,    -1,    -1,   871,    10,    11,    12,    -1,    -1,   350,
     878,   879,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1206,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,   239,   201,    -1,   203,    10,    11,    12,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1258,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   443,    57,    -1,   446,    -1,    -1,   976,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    81,    -1,  1306,
    1307,  1308,  1309,  1310,  1311,    -1,   994,  1314,  1315,    -1,
    1317,    -1,    -1,  1001,  1321,    -1,    -1,    -1,    -1,  1007,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1336,
    1337,    -1,  1339,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,  1361,    57,  1363,   141,   142,   143,
     144,   145,    -1,    -1,    -1,  1053,    -1,    69,    -1,    -1,
      -1,   378,    -1,    -1,    -1,    -1,    -1,   203,    -1,    -1,
     387,  1388,   166,    -1,    -1,   169,   170,   394,   172,   173,
     174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   406,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     417,    -1,    -1,    -1,   198,    -1,    -1,    -1,   202,    -1,
      -1,  1428,    -1,    -1,  1431,    -1,    -1,    -1,    -1,   203,
      -1,    -1,   593,    10,    11,    12,    -1,    -1,    -1,    -1,
    1128,    -1,    -1,    -1,  1132,    -1,  1134,    -1,    -1,  1137,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    11,    12,    -1,  1491,    -1,  1493,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,   502,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,   689,   690,
      -1,    -1,    -1,    -1,   541,    -1,    -1,    -1,    -1,    69,
    1547,    -1,  1549,    -1,  1551,    -1,    -1,    -1,    -1,  1556,
    1557,    -1,    -1,    -1,  1561,  1243,  1563,  1245,    -1,  1566,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    83,
      84,    -1,    -1,  1580,    -1,    -1,  1583,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,   598,    -1,    -1,    -1,    -1,    -1,    -1,  1286,    31,
      -1,  1289,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,   631,    -1,   203,   141,   142,   143,
     144,   145,  1639,    -1,  1641,    -1,  1643,    69,    -1,    -1,
      -1,  1648,    31,  1650,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   167,    -1,   169,   170,    -1,   172,   173,
     174,  1349,    -1,  1670,    -1,    -1,    -1,    -1,  1356,    -1,
      -1,    -1,    -1,    -1,    -1,  1682,  1683,    -1,    -1,    68,
      -1,    -1,    -1,    -1,   198,  1692,    -1,    -1,   202,    -1,
      -1,   205,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   719,    -1,    -1,   104,    -1,   878,   879,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1424,  1425,    -1,    -1,
      -1,  1748,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,  1765,  1766,
    1767,    -1,    -1,    68,    -1,  1772,    -1,  1774,   200,    -1,
      -1,    -1,  1460,    -1,   163,  1782,    81,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,   794,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,   198,
     199,    -1,    -1,    -1,    -1,   976,   121,   122,   123,   124,
     125,   126,    -1,    -1,    -1,    -1,   833,    -1,   835,    -1,
      -1,    -1,    -1,   994,    -1,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,  1007,    31,    -1,  1537,
    1538,    -1,    -1,  1541,    -1,    -1,   863,   864,   163,    -1,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,   880,   881,   882,   883,   884,    -1,    -1,
      -1,    -1,   187,    -1,    68,    -1,   893,    -1,    -1,   194,
      -1,  1579,  1053,   198,   199,    -1,    -1,    81,  1905,  1906,
      -1,    -1,  1590,    87,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1926,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   936,
    1937,    -1,    -1,    -1,    -1,    -1,  1943,    -1,    -1,    -1,
    1947,    -1,    -1,    -1,    -1,   952,   953,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,  1128,    -1,    -1,
      -1,  1132,    -1,    -1,    31,    -1,  1137,  1665,    -1,   163,
     987,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1690,   187,    -1,    -1,    -1,    -1,    -1,    -1,
     598,  1018,  2019,    -1,   198,   199,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,  2032,  1714,    -1,  1716,    -1,
      -1,    10,    11,    12,  1722,    92,    -1,    -1,    -1,    -1,
      -1,    -1,  2049,   631,    -1,    -1,    -1,   104,    -1,    -1,
    1057,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,  1243,    -1,   141,   142,   143,   144,   145,    -1,
      69,    -1,  1780,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   163,  1114,  1115,   166,
      -1,    -1,   169,   170,    -1,   172,   173,   174,    -1,   176,
      -1,    -1,    -1,    -1,    -1,  1286,    -1,    -1,  1289,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   198,    -1,  1150,  1151,  1152,  1153,  1154,  1155,  1156,
    1157,    -1,   104,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,
    1187,    81,    -1,    -1,    -1,    -1,    -1,    -1,  1349,   141,
     142,   143,   144,   145,    -1,  1356,  1884,    -1,    -1,  1206,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,  1897,
      -1,    -1,    -1,  1901,    -1,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,   203,    -1,    -1,    -1,    -1,    -1,
     130,    -1,  1920,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   198,    -1,    81,    -1,
     202,  1258,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1424,  1425,    -1,    -1,    -1,    -1,   169,
     170,   104,   172,   173,   174,    -1,   864,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1974,    -1,    -1,    -1,
      -1,    -1,   880,   881,   882,   883,   884,    -1,   198,   199,
      -1,    -1,    -1,    -1,    -1,   893,    -1,  1314,   141,   142,
     143,   144,   145,  2001,  1321,  2003,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1336,
    1337,    -1,  1339,    -1,    -1,    -1,   169,   170,    -1,   172,
     173,   174,    -1,     3,     4,    -1,     6,     7,    -1,    -1,
      10,    11,    12,    13,  1361,    -1,  1363,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   198,   199,  2055,    28,    29,
      -1,  2059,    -1,    -1,    -1,    -1,  1537,  1538,    -1,    -1,
    1541,  1388,    -1,    -1,    -1,  2073,  2074,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,  1579,    -1,
      -1,    81,    -1,    -1,  1431,    -1,    -1,    87,    -1,  1590,
    1018,    -1,    -1,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    -1,
     130,    -1,   132,   133,   134,   135,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,   146,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,   163,  1665,    -1,    -1,    -1,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,   177,    -1,    -1,
     180,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   188,  1690,
      -1,    -1,    59,    60,    -1,    -1,    -1,    -1,   198,    -1,
    1547,    -1,  1549,    -1,  1551,    -1,    -1,    -1,    -1,  1556,
    1557,    -1,    -1,  1714,  1561,  1716,  1563,    -1,    -1,  1566,
      -1,  1722,  1150,  1151,    -1,    -1,  1154,    -1,    -1,    -1,
      -1,    -1,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,   136,
     137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1206,  1780,
      -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,  1650,    10,    11,    12,    92,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,  1670,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,   140,   141,   142,   143,   144,
     145,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,    -1,
      -1,   166,   167,  1884,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,    -1,    -1,    -1,  1897,    -1,    -1,    -1,
    1901,    -1,   187,   188,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1339,    -1,   198,    -1,    -1,    -1,    -1,  1765,  1766,
    1767,    -1,    -1,    -1,    -1,  1772,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1361,  1781,  1363,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
    1388,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,  1974,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,   203,    57,    -1,
    2001,    -1,  2003,    10,    11,    12,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    -1,    -1,  1926,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,
    1937,    -1,    -1,    -1,    -1,    -1,  1943,    -1,    -1,    -1,
    1947,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1547,
      -1,  1549,    -1,  1551,    -1,    -1,    -1,    -1,  1556,  1976,
      -1,    -1,    -1,  1561,    -1,  1563,    -1,    -1,  1566,    -1,
      -1,    -1,    -1,    -1,   203,    -1,    -1,   141,   142,   143,
     144,   145,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,  2019,    -1,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,    69,   136,   137,    -1,     3,     4,
       5,     6,     7,    -1,    -1,    -1,   203,    -1,    13,    -1,
      -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,  1670,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,   113,   114,
     115,    -1,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,   131,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,  1765,  1766,  1767,
     155,    -1,    -1,   158,  1772,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,   188,   189,    -1,   191,    -1,   193,   194,
     195,    -1,    -1,   198,   199,    -1,   201,   202,    -1,    -1,
     205,   206,   207,   208,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    59,    60,
      -1,    -1,    -1,    -1,    -1,    69,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1926,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,  1937,
      -1,    -1,    -1,    -1,    -1,  1943,    -1,    -1,    -1,  1947,
      -1,    48,    -1,    50,    51,   136,   137,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,
      97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,    -1,   109,   110,   111,   112,   113,   114,   115,   203,
     117,  2019,   119,   120,   121,   122,   123,   124,   125,   126,
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
     111,   112,   113,   114,   115,    -1,   117,    -1,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,   129,   130,
     131,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,
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
      49,    50,    51,    52,    53,    54,    55,    57,    -1,    -1,
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
      -1,    57,    -1,    -1,    -1,   167,    -1,   169,   170,    -1,
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
     141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   163,    57,    -1,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    68,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    87,    -1,    -1,    -1,   198,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    -1,   130,    -1,   132,   133,
     134,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    83,    -1,    85,    -1,    -1,    -1,    -1,    -1,   163,
      -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,   175,   104,   177,    -1,    -1,   180,    -1,     3,     4,
      -1,     6,     7,   187,   188,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    29,    -1,    31,    -1,    -1,   141,
     142,   143,   144,   145,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    57,    -1,    -1,    -1,   169,   170,    -1,
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
     143,   144,   145,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    57,    -1,   166,    -1,    -1,   169,   170,    -1,   172,
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
     123,   124,   125,   126,   127,   128,    81,   130,    -1,   132,
     133,   134,   135,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
     163,    10,    11,    12,    13,    -1,   169,   170,    -1,   172,
     173,   174,   175,    -1,   177,    -1,    -1,   180,    -1,    28,
      29,    -1,    -1,    -1,    -1,   188,   141,   142,   143,   144,
     145,    -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,    -1,   198,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
      -1,   130,    -1,    -1,   133,   134,   135,    -1,    -1,    -1,
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
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,   203,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,   203,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,   203,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,   201,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   201,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   201,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   201,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,   201,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    31,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,   201,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    87,    -1,    -1,    -1,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,   200,    -1,    -1,    -1,   163,    -1,
      68,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,   176,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   187,    -1,   140,   141,   142,   143,   144,   145,
     146,    -1,    -1,   198,   199,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    32,    -1,   200,    -1,   163,    -1,    -1,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      -1,   187,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,    70,   198,   199,    -1,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    -1,    -1,   163,    -1,    -1,   166,   167,
      -1,   169,   170,    92,   172,   173,   174,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,   187,
      -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
     198,   199,    -1,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,   140,   141,   142,   143,   144,   145,    70,   147,   148,
     149,   150,   151,    69,    -1,    78,    79,    80,    81,   158,
      83,    84,    -1,    -1,   163,   164,   165,   166,   167,    92,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,   187,   188,
      -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,   121,   198,
     199,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
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
      -1,   121,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
     140,   141,   142,   143,   144,   145,    70,   147,   148,   149,
     150,   151,    69,    -1,    78,    79,    80,    81,   158,    83,
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
      69,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
      -1,    31,    -1,    -1,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,    31,    -1,    -1,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,   138,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,   138,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,   138,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,   138,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
     138,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,   193,    92,    -1,    -1,    -1,   198,   199,
      -1,    -1,    -1,    -1,    -1,   205,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   121,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    69,    -1,    78,    79,    80,    81,
     158,    83,    84,    -1,    -1,   163,   164,   165,   166,   167,
      92,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
     178,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,   121,
     198,   199,    -1,    -1,    -1,    -1,    -1,   205,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      81,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   104,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   112,   113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   193,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,
      -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   163,    -1,    -1,   166,    -1,    -1,   169,   170,
      -1,   172,   173,   174,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   189,    -1,
      -1,    -1,    27,    -1,    -1,    30,    31,   198,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    32,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    12,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    69,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69
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
     272,   278,   339,   340,   348,   349,   352,   353,   354,   355,
     356,   357,   358,   359,   361,   362,   363,   365,   368,   380,
     381,   388,   391,   394,   397,   400,   403,   409,   411,   412,
     414,   424,   425,   426,   428,   433,   438,   458,   466,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,   494,   496,   498,   121,   122,   123,   139,   163,
     173,   199,   216,   257,   339,   361,   470,   361,   199,   361,
     361,   361,   361,   109,   361,   361,   456,   457,   361,   361,
     361,   361,    81,    83,    92,   121,   141,   142,   143,   144,
     145,   158,   199,   227,   381,   425,   428,   433,   470,   474,
     470,   361,   361,   361,   361,   361,   361,   361,   361,    38,
     361,   485,   486,   121,   132,   199,   227,   270,   425,   426,
     427,   429,   433,   467,   468,   469,   478,   482,   483,   361,
     199,   360,   430,   199,   360,   372,   350,   361,   238,   360,
     199,   199,   199,   360,   201,   361,   216,   201,   361,     3,
       4,     6,     7,    10,    11,    12,    13,    28,    29,    31,
      57,    68,    71,    72,    73,    74,    75,    76,    77,    87,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   130,   132,   133,   134,   135,
     139,   140,   146,   163,   167,   175,   177,   180,   187,   188,
     199,   216,   217,   218,   229,   499,   520,   521,   524,    27,
     201,   355,   357,   361,   202,   250,   361,   112,   113,   163,
     166,   189,   219,   220,   221,   222,   226,    83,   205,   305,
     306,    83,   307,   123,   132,   122,   132,   199,   199,   199,
     199,   216,   276,   502,   199,   199,    70,    70,    70,    70,
      70,   350,    83,    91,   159,   160,   161,   491,   492,   166,
     202,   226,   226,   216,   277,   502,   167,   199,   199,   502,
     502,    83,   195,   202,   373,    28,   349,   352,   361,   363,
     470,   475,   233,   202,   480,    91,   431,   491,    91,   491,
     491,    32,   166,   183,   503,   199,     9,   201,   199,   348,
     362,   471,   474,   118,    38,   256,   167,   275,   502,   121,
     194,   257,   340,    70,   202,   465,   201,   201,   201,   201,
     201,   201,   201,   201,    10,    11,    12,    30,    31,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    69,   201,    70,    70,   202,   162,   133,
     173,   175,   189,   191,   278,   338,   339,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      59,    60,   136,   137,   460,   465,   465,   199,   199,    70,
     202,   199,   256,   257,    14,   361,   201,   138,    49,   216,
     455,    91,   349,   363,   162,   470,   138,   204,     9,   440,
     271,   349,   363,   470,   503,   162,   199,   432,   460,   465,
     200,   361,    32,   236,     8,   374,     9,   201,   236,   237,
     350,   351,   361,   216,   290,   240,   201,   201,   201,   140,
     146,   524,   524,   183,   523,   199,   112,   524,    14,   162,
     140,   146,   163,   216,   218,   201,   201,   201,   251,   116,
     180,   201,   219,   221,   219,   221,   219,   221,   226,   219,
     221,   202,     9,   441,   201,   103,   166,   202,   470,     9,
     201,    14,     9,   201,   132,   132,   470,   495,   350,   349,
     363,   470,   474,   475,   200,   183,   268,   139,   470,   484,
     485,   361,   382,   383,   350,   406,   406,   382,   406,   201,
      70,   460,   159,   492,    82,   361,   470,    91,   159,   492,
     226,   215,   201,   202,   263,   273,   415,   417,    92,   199,
     375,   376,   378,   424,   428,   477,   479,   496,   406,    14,
     103,   497,   369,   370,   371,   300,   301,   458,   459,   200,
     200,   200,   200,   200,   203,   235,   236,   258,   265,   272,
     458,   361,   206,   207,   208,   216,   504,   505,   524,    38,
      87,   176,   303,   304,   361,   499,   247,   248,   349,   357,
     358,   361,   363,   470,   202,   249,   249,   249,   249,   199,
     502,   266,   256,   361,   481,   361,   361,   361,   361,   361,
      32,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   429,   361,   481,   481,   361,
     487,   488,   132,   202,   217,   218,   480,   276,   216,   277,
     502,   502,   275,   257,    38,   352,   355,   357,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   167,   202,   216,   461,   462,   463,   464,   480,   303,
     303,   481,   361,   484,   256,   200,   361,   199,   454,     9,
     440,   200,   200,    38,   361,    38,   361,   432,   200,   200,
     200,   478,   479,   480,   303,   202,   216,   461,   462,   480,
     200,   233,   294,   202,   357,   361,   361,    95,    32,   236,
     288,   201,    27,   103,    14,     9,   200,    32,   202,   291,
     524,    31,    92,   176,   229,   517,   518,   519,   199,     9,
      50,    51,    56,    58,    70,   140,   141,   142,   143,   144,
     145,   187,   188,   199,   227,   389,   392,   395,   398,   401,
     404,   410,   425,   433,   434,   436,   437,   216,   522,   233,
     199,   244,   202,   201,   202,   201,   202,   201,   103,   166,
     202,   201,   112,   113,   166,   222,   223,   224,   225,   226,
     222,   216,   361,   306,   434,    83,     9,   200,   200,   200,
     200,   200,   200,   200,   201,    50,    51,   513,   515,   516,
     134,   281,   199,     9,   200,   200,   138,   204,     9,   440,
       9,   440,   204,   204,   204,   204,    83,    85,   216,   493,
     216,    70,   203,   203,   212,   214,    32,   135,   280,   182,
      54,   167,   182,   202,   419,   363,   138,     9,   440,   200,
     162,   200,   524,   524,    14,   374,   300,   231,   196,     9,
     441,    87,   524,   525,   460,   460,   203,     9,   440,   184,
     470,    83,    84,   302,   361,   200,     9,   441,    14,     9,
     200,     9,   200,   200,   200,   200,    14,   200,   203,   234,
     235,   366,   259,   134,   279,   199,   502,   204,   203,   361,
      32,   204,   204,   138,   203,     9,   440,   361,   503,   199,
     269,   264,   274,    14,   497,   267,   256,    71,   470,   361,
     503,   200,   200,   204,   203,   200,    50,    51,    70,    78,
      79,    80,    92,   140,   141,   142,   143,   144,   145,   158,
     187,   188,   216,   390,   393,   396,   399,   402,   405,   425,
     436,   443,   445,   446,   450,   453,   216,   470,   470,   138,
     279,   460,   465,   460,   200,   361,   295,    75,    76,   296,
     231,   360,   233,   351,   103,    38,   139,   285,   470,   434,
     216,    32,   236,   289,   201,   292,   201,   292,     9,   440,
      92,   229,   138,   162,     9,   440,   200,    87,   506,   507,
     524,   525,   504,   434,   434,   434,   434,   434,   439,   442,
     199,    70,    70,    70,    70,    70,   199,   199,   434,   162,
     202,    10,    11,    12,    31,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    69,   162,   503,
     203,   425,   202,   253,   221,   221,   221,   216,   221,   222,
     222,   226,     9,   441,   203,   203,    14,   470,   201,   184,
       9,   440,   216,   282,   425,   202,   484,   139,   470,    14,
     361,   361,   204,   361,   203,   212,   524,   282,   202,   418,
     176,    14,   200,   361,   375,   480,   201,   524,   196,   203,
     232,   235,   245,    32,   511,   459,   525,    38,    83,   176,
     461,   462,   464,   461,   462,   464,   524,    70,    38,    87,
     176,   361,   434,   248,   357,   358,   470,   249,   248,   249,
     249,   203,   235,   300,   199,   425,   280,   367,   260,   361,
     361,   361,   203,   199,   303,   281,    32,   280,   524,    14,
     279,   502,   429,   203,   199,    14,    78,    79,    80,   216,
     444,   444,   446,   448,   449,    52,   199,    70,    70,    70,
      70,    70,    91,   159,   199,   199,   162,     9,   440,   200,
     454,    38,   361,   280,   203,    75,    76,   297,   360,   236,
     203,   201,    96,   201,   285,   470,   199,   138,   284,    14,
     233,   292,   106,   107,   108,   292,   203,   524,   184,   138,
     162,   524,   216,   176,   517,   524,     9,   440,   200,   176,
     440,   138,   204,     9,   440,   439,   384,   385,   434,   407,
     434,   435,   407,   384,   407,   375,   377,   379,   407,   200,
     132,   217,   434,   489,   490,   434,   434,   434,    32,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   522,    83,   254,   203,   203,   203,   203,
     225,   201,   434,   516,   103,   104,   512,   514,     9,   311,
     200,   199,   352,   357,   361,   138,   204,   203,   497,   311,
     168,   181,   202,   414,   421,   361,   168,   202,   420,   138,
     201,   511,   199,   248,   348,   362,   471,   474,   524,   374,
      87,   525,    83,    83,   176,    14,    83,   503,   503,   481,
     470,   302,   361,   200,   300,   202,   300,   199,   138,   199,
     303,   200,   202,   524,   202,   201,   524,   280,   261,   432,
     303,   138,   204,     9,   440,   445,   448,   386,   387,   446,
     408,   446,   447,   408,   386,   408,   159,   375,   451,   452,
     408,    81,   446,   470,   202,   360,    32,    77,   236,   201,
     351,   284,   484,   285,   200,   434,   102,   106,   201,   361,
      32,   201,   293,   203,   184,   524,   216,   138,    87,   524,
     525,    32,   200,   434,   434,   200,   204,     9,   440,   138,
     204,     9,   440,   204,   204,   204,   138,     9,   440,   200,
     200,   138,   203,     9,   440,   434,    32,   200,   233,   201,
     201,   201,   201,   216,   524,   524,   512,   425,     6,   113,
     119,   122,   127,   169,   170,   172,   203,   312,   337,   338,
     339,   344,   345,   346,   347,   458,   484,   361,   203,   202,
     203,    54,   361,   203,   361,   361,   374,   470,   201,   202,
     525,    38,    83,   176,    14,    83,   361,   199,   199,   204,
     511,   200,   311,   200,   300,   361,   303,   200,   311,   497,
     311,   201,   202,   199,   200,   446,   446,   200,   204,     9,
     440,   138,   204,     9,   440,   204,   204,   204,   138,   200,
       9,   440,   200,   311,    32,   233,   201,   200,   200,   200,
     241,   201,   201,   293,   233,   138,   524,   524,   176,   524,
     138,   434,   434,   434,   434,   375,   434,   434,   434,   202,
     203,   514,   134,   135,   189,   217,   500,   524,   283,   425,
     113,   347,    31,   127,   140,   146,   167,   173,   321,   322,
     323,   324,   425,   171,   329,   330,   130,   199,   216,   331,
     332,   313,   257,   524,     9,   201,     9,   201,   201,   497,
     338,   200,   308,   167,   416,   203,   203,   361,    83,    83,
     176,    14,    83,   361,   303,   303,   119,   364,   511,   203,
     511,   200,   200,   203,   202,   203,   311,   300,   138,   446,
     446,   446,   446,   375,   203,   233,   239,   242,    32,   236,
     287,   233,   524,   200,   434,   138,   138,   138,   233,   425,
     425,   502,    14,   217,     9,   201,   202,   500,   497,   324,
     183,   202,     9,   201,     3,     4,     5,     6,     7,    10,
      11,    12,    13,    27,    28,    29,    57,    71,    72,    73,
      74,    75,    76,    77,    87,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   139,   140,   147,
     148,   149,   150,   151,   163,   164,   165,   175,   177,   178,
     180,   187,   189,   191,   193,   194,   216,   422,   423,     9,
     201,   167,   171,   216,   332,   333,   334,   201,    83,   343,
     256,   314,   500,   500,    14,   257,   203,   309,   310,   500,
      14,    83,   361,   200,   200,   199,   511,   198,   508,   364,
     511,   308,   203,   200,   446,   138,   138,    32,   236,   286,
     287,   233,   434,   434,   434,   203,   201,   201,   434,   425,
     317,   524,   325,   326,   433,   322,    14,    32,    51,   327,
     330,     9,    36,   200,    31,    50,    53,    14,     9,   201,
     218,   501,   343,    14,   524,   256,   201,    14,   361,    38,
      83,   413,   202,   509,   510,   524,   201,   202,   335,   511,
     508,   203,   511,   446,   446,   233,   100,   252,   203,   216,
     229,   318,   319,   320,     9,   440,     9,   440,   203,   434,
     423,   423,    68,   328,   333,   333,    31,    50,    53,   434,
      83,   183,   199,   201,   434,   501,   434,    83,     9,   441,
     231,     9,   441,    14,   512,   231,   202,   335,   335,    98,
     201,   116,   243,   162,   103,   524,   184,   433,   174,    14,
     513,   315,   199,    38,    83,   200,   203,   510,   524,   203,
     231,   201,   199,   180,   255,   216,   338,   339,   184,   434,
     184,   298,   299,   459,   316,    83,   203,   425,   253,   177,
     216,   201,   200,     9,   441,    87,   124,   125,   126,   341,
     342,   298,    83,   283,   201,   511,   459,   525,   525,   200,
     200,   201,   508,    87,   341,    83,    38,    83,   176,   511,
     202,   201,   202,   336,   525,   525,    83,   176,    14,    83,
     508,   233,   231,    83,    38,    83,   176,    14,    83,   361,
     336,   203,   203,    83,   176,    14,    83,   361,    14,    83,
     361,   361
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
     310,   311,   311,   313,   312,   314,   312,   312,   312,   312,
     315,   312,   316,   312,   312,   312,   312,   312,   312,   312,
     312,   317,   317,   317,   318,   319,   319,   320,   320,   321,
     321,   322,   322,   323,   323,   324,   324,   324,   324,   324,
     324,   324,   325,   325,   326,   327,   327,   328,   328,   329,
     329,   330,   331,   331,   331,   332,   332,   332,   332,   333,
     333,   333,   333,   333,   333,   333,   334,   334,   334,   335,
     335,   336,   336,   337,   337,   338,   338,   339,   339,   340,
     340,   340,   340,   340,   340,   340,   341,   341,   342,   342,
     342,   343,   343,   343,   343,   344,   344,   345,   345,   346,
     346,   347,   348,   349,   349,   349,   349,   349,   349,   350,
     350,   351,   351,   352,   352,   352,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   361,   361,   361,   361,
     362,   363,   363,   363,   363,   363,   363,   363,   363,   363,
     363,   363,   363,   363,   363,   363,   363,   363,   363,   363,
     363,   363,   363,   363,   363,   363,   363,   363,   363,   363,
     363,   363,   363,   363,   363,   363,   363,   363,   363,   363,
     363,   363,   363,   363,   363,   363,   363,   363,   363,   363,
     363,   363,   363,   363,   363,   363,   363,   363,   363,   363,
     363,   363,   363,   363,   363,   363,   363,   363,   363,   363,
     363,   363,   363,   363,   363,   363,   363,   363,   363,   364,
     364,   366,   365,   367,   365,   369,   368,   370,   368,   371,
     368,   372,   368,   373,   368,   374,   374,   374,   375,   375,
     376,   376,   377,   377,   378,   378,   379,   379,   380,   381,
     381,   382,   382,   383,   383,   384,   384,   385,   385,   386,
     386,   387,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   406,   407,   407,   408,   408,   409,   410,   411,
     411,   412,   412,   412,   412,   412,   412,   412,   412,   412,
     412,   412,   412,   413,   413,   413,   413,   414,   415,   415,
     416,   416,   417,   417,   417,   418,   418,   419,   420,   420,
     421,   421,   421,   422,   422,   422,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   424,
     425,   425,   426,   426,   426,   426,   426,   427,   427,   428,
     428,   428,   428,   429,   429,   429,   430,   430,   430,   431,
     431,   431,   432,   432,   433,   433,   433,   433,   433,   433,
     433,   433,   433,   433,   433,   433,   433,   433,   433,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   434,   435,   435,   436,   437,   437,
     438,   438,   438,   438,   438,   438,   438,   439,   439,   440,
     440,   441,   441,   442,   442,   442,   442,   443,   443,   443,
     443,   443,   444,   444,   444,   444,   445,   445,   446,   446,
     446,   446,   446,   446,   446,   446,   446,   446,   446,   446,
     446,   446,   446,   446,   447,   447,   448,   448,   449,   449,
     449,   449,   450,   450,   451,   451,   452,   452,   453,   453,
     454,   454,   455,   455,   457,   456,   458,   459,   459,   460,
     460,   461,   461,   461,   462,   462,   463,   463,   464,   464,
     465,   465,   466,   466,   466,   467,   467,   468,   468,   469,
     469,   470,   470,   470,   470,   470,   470,   470,   470,   470,
     470,   470,   471,   472,   472,   472,   472,   472,   472,   472,
     472,   473,   473,   473,   473,   473,   473,   473,   473,   473,
     474,   475,   475,   476,   476,   476,   477,   477,   477,   478,
     479,   479,   479,   480,   480,   480,   480,   481,   481,   482,
     482,   482,   482,   482,   482,   483,   483,   483,   483,   483,
     484,   484,   484,   484,   484,   484,   485,   485,   486,   486,
     486,   486,   486,   486,   486,   486,   487,   487,   488,   488,
     488,   488,   489,   489,   490,   490,   490,   490,   491,   491,
     491,   491,   492,   492,   492,   492,   492,   492,   493,   493,
     493,   494,   494,   494,   494,   494,   494,   494,   494,   494,
     494,   494,   495,   495,   496,   496,   497,   497,   498,   498,
     498,   498,   499,   499,   500,   500,   501,   501,   502,   502,
     503,   503,   504,   504,   505,   506,   506,   506,   506,   506,
     506,   507,   507,   507,   507,   508,   508,   509,   509,   510,
     510,   511,   511,   512,   512,   513,   514,   514,   515,   515,
     515,   515,   516,   516,   516,   517,   517,   517,   517,   518,
     518,   519,   519,   519,   519,   520,   521,   522,   522,   523,
     523,   524,   524,   524,   524,   524,   524,   524,   524,   524,
     524,   524,   525,   525
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
       3,     2,     0,     0,     4,     0,     5,     2,     2,     2,
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
       3,     1,     1,     1,     1,     1,     3,     3,     4,     4,
       3,     1,     1,     7,     9,     9,     7,     6,     8,     1,
       4,     4,     1,     1,     1,     4,     2,     1,     0,     1,
       1,     1,     3,     3,     3,     0,     1,     1,     3,     3,
       2,     3,     6,     0,     1,     4,     2,     0,     5,     3,
       3,     1,     6,     4,     4,     2,     2,     0,     5,     3,
       3,     1,     2,     0,     5,     3,     3,     1,     2,     2,
       1,     2,     1,     4,     3,     3,     6,     3,     1,     1,
       1,     4,     4,     4,     4,     4,     4,     2,     2,     4,
       2,     2,     1,     3,     3,     3,     0,     2,     5,     6,
       6,     7,     1,     2,     1,     2,     1,     4,     1,     4,
       3,     0,     1,     3,     2,     1,     2,     4,     3,     3,
       1,     4,     2,     2,     0,     0,     3,     1,     3,     3,
       2,     0,     2,     2,     2,     2,     1,     2,     4,     2,
       5,     3,     1,     1,     0,     3,     4,     5,     6,     3,
       1,     3,     2,     1,     0,     4,     1,     3,     2,     4,
       5,     2,     2,     1,     1,     1,     1,     3,     2,     1,
       8,     6,     1,     0
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
#line 7329 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 763 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 7337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 770 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 7343 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 771 "hphp.y" /* yacc.c:1646  */
    { }
#line 7349 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 7355 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 775 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7361 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7367 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7379 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 779 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 7385 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 7393 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 783 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 7400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 785 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 7406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 786 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 787 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 7418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 788 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 789 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7432 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 793 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7441 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 798 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7450 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 803 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7457 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 806 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7464 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 809 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7472 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 813 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7480 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 817 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7488 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 821 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7496 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 825 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7504 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 828 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7511 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7517 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7523 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7529 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7541 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7547 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7553 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 840 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7565 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 842 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7571 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 843 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7577 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 844 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7583 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 845 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 927 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 929 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7601 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 934 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 935 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7614 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 941 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7620 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 945 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7626 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 946 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7632 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 948 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7638 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 950 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 955 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 956 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7657 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 962 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7663 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 966 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7670 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 968 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7677 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 970 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7684 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 975 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7690 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 977 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7696 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 980 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7702 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 982 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7708 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 983 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7714 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 988 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7723 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 995 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7732 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1003 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1006 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1012 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 7752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1013 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval)); }
#line 7758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1018 "hphp.y" /* yacc.c:1646  */
    {
                                         _p->onUsing((yyval), (yyvsp[-2]), true, (yyvsp[-1]), nullptr);
                                       }
#line 7766 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1025 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7772 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1026 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7778 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1031 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 7784 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1032 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval), (yyval), (yyvsp[0])); }
#line 7791 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1037 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7797 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7803 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1039 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7809 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1040 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7815 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7821 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1047 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7827 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1052 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1053 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7840 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7848 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1059 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7855 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1062 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1066 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7870 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1068 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7878 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1071 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7885 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1073 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7893 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1077 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1078 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1079 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1080 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1081 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1082 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1085 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1087 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7965 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1088 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1089 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1090 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7984 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1094 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7991 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1096 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7999 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1101 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 8006 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1103 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 8014 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1107 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 8022 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1116 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 8028 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1117 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8034 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1120 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false); }
#line 8040 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1121 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 8046 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1123 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8054 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1127 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8062 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1131 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8070 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1135 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8078 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1139 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8086 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1143 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8094 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1148 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8102 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1151 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 8108 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 8117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1156 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1157 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1158 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1159 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1160 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8147 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1161 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8153 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1162 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 8159 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1163 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8165 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1164 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8171 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1165 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 8177 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1166 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8183 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1167 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 8193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1189 "hphp.y" /* yacc.c:1646  */
    { _p->pushLabelScope(LS::Using);
                                         _p->onNewLabelScope(false);
                                         (yyval) = (yyvsp[-1]); }
#line 8201 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1195 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1; }
#line 8207 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1196 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8213 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1205 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), nullptr, (yyvsp[-2]));
                                         _p->onExprListElem((yyval), &(yyval), (yyvsp[0])); }
#line 8220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1207 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0])); }
#line 8226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1217 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1218 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1222 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false); }
#line 8244 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1223 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8250 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1232 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 8256 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1233 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8262 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1237 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::Finally);}
#line 8269 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1239 "hphp.y" /* yacc.c:1646  */
    { _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->popLabelScope();
                                         _p->onCompleteLabelScope(false);}
#line 8277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1245 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1246 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1250 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 8295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1251 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1255 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 8307 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1261 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8316 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1268 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8326 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1276 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8335 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1283 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1291 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8354 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1297 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8364 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1306 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8371 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1310 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 8377 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1314 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8384 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1318 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 8390 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1324 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8397 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 8415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1342 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8422 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 8440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1359 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8447 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1362 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8455 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1367 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8462 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1370 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8470 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1376 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 8476 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1379 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 8482 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1383 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8489 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1386 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8500 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1394 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8507 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1397 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8518 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1405 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8524 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1406 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 8531 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1410 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8537 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1413 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8543 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1416 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 8549 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1417 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 8555 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1418 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 8563 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1421 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 8569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1422 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 8575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1426 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1427 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1430 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8593 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1431 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8599 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1434 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8605 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1435 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8611 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1438 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8617 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1440 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8623 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1443 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8629 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1445 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8635 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8641 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1450 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8647 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8653 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8659 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1455 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8665 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8671 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8677 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8683 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8689 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1469 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8695 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1471 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8701 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1474 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8707 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1476 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8713 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1480 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8719 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1482 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8726 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8732 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8738 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8744 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8750 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1495 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8756 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1497 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8762 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1498 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8768 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1501 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8774 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1502 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8780 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1507 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8786 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1508 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8792 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1513 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8798 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1514 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8804 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8810 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1518 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8816 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1521 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8822 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1522 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8828 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1530 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8835 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1536 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8842 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1542 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1546 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8856 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1550 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1555 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8870 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1560 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8878 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1563 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8884 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1569 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1574 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8900 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1579 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8908 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1585 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8916 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1591 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8924 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1597 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1603 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8940 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1609 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8948 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1616 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8956 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1623 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8964 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1632 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1637 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8978 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1642 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8986 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1646 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8992 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1649 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8999 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1653 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 9006 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1657 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 9014 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1660 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9020 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1665 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9028 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1669 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9036 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1673 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9044 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1678 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9052 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1683 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9060 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1688 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9068 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1693 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9076 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1698 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9084 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1704 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9092 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1710 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9100 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1716 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 9106 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1717 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 9112 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1718 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 9118 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9124 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1724 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9130 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1727 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,false);}
#line 9137 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1729 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::InOut,false);}
#line 9144 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1731 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::Ref,false);}
#line 9151 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1733 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,true);}
#line 9158 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1736 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::In,false);}
#line 9165 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1739 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::In,true);}
#line 9172 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1742 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::Ref,false);}
#line 9179 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1745 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::InOut,false);}
#line 9186 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1750 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9192 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1751 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 9198 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1754 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9204 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1755 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 9210 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1756 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 9216 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1760 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9222 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1762 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9228 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1763 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 9234 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1764 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9240 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1769 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9246 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1770 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9252 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1773 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9259 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1778 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1784 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1785 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1788 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 9283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1789 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 9290 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1792 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 9296 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1793 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 9303 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1795 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1798 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 9317 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1800 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9323 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1803 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1810 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9341 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1818 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9349 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1825 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9359 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1831 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 9365 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1833 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9371 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1835 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9377 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1837 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 9383 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1839 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 9389 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1840 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 9396 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1843 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 9402 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9408 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1847 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9414 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1848 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 9420 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1854 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 9426 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1859 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 9433 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1862 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 9441 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1869 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 9447 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1870 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 9454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1875 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 9461 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1878 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 9467 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1885 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 9474 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1887 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 9480 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1891 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9486 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1896 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9492 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1898 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9498 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1900 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9504 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1901 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 9515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1907 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 9521 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1909 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 9527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1910 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 9533 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1914 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval),nullptr,(yyvsp[0])); }
#line 9539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1916 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval),&(yyvsp[-2]),(yyvsp[0])); }
#line 9545 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1921 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1924 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9557 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1925 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 9563 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1929 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 9569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1930 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 9575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1934 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 9582 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1937 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 9589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1942 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 9596 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1947 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 9602 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1948 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 9609 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 9615 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1954 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 9621 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1955 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 9627 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 9633 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 9639 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1961 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9645 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 9651 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 9657 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 9663 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 9669 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1967 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 9675 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1969 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 9681 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9689 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9695 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1977 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9701 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1981 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9707 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9713 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9719 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1987 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9725 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1990 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9731 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9737 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9743 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1995 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9749 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1998 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9755 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9761 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9767 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9773 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9779 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 2006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9785 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9791 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 2008 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9797 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 2009 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9803 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9809 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9815 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9821 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9827 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 2019 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 2023 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9839 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 2025 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9845 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 2026 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9851 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9857 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 2031 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 2033 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9869 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 2037 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 2039 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9881 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 2043 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 2047 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9896 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 2051 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9902 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 2055 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9908 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 2059 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9914 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 2061 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9920 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 2062 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9926 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 2063 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 2064 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9938 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 2065 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9944 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 2068 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9950 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 2069 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9956 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 2073 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9962 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 2074 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 2078 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 2079 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9980 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 2080 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9986 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 2081 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9992 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 2085 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9998 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 2090 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 10004 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 2094 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 10010 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 2098 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10016 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 2102 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 10022 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 2106 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10028 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 2111 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 10034 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 2115 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10040 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 2119 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10046 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 2120 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10052 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 2121 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10058 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 2122 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10064 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 2123 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10070 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 2127 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10076 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 2132 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 10082 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 2133 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 10088 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 2134 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 10094 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 2137 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 10100 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 2138 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 10106 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2139 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 10112 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2140 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 10118 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2141 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 10124 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2142 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 10130 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2143 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 10136 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2144 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 10142 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2145 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 10148 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2146 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 10154 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2147 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 10160 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 10166 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2149 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 10172 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2150 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 10178 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2151 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 10184 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2152 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 10190 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2153 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 10196 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2154 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 10202 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2155 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 10208 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2156 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 10214 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2157 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 10220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2158 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 10226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2159 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 10232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2160 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 10238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2161 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 10244 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2162 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 10250 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2163 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 10256 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2164 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 10262 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2165 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 10268 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2166 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 10274 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2167 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 10280 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2168 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 10286 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2169 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 10292 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2170 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 10298 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2171 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 10304 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2172 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 10310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2173 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 10316 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2174 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 10322 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2175 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 10328 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2176 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 10334 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2177 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 10340 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2178 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 10346 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2179 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 10352 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2180 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 10358 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2181 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 10365 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2183 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 10371 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2184 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 10378 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2186 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 10384 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2188 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 10390 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2189 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10396 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2190 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 10402 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2191 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 10408 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2192 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 10414 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2193 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10420 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2194 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 10426 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2195 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 10432 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2196 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 10438 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2197 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 10444 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2198 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 10450 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2199 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 10456 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2200 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 10462 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2201 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 10468 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2202 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 10474 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2203 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10480 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2204 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10486 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2205 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10492 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2206 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10498 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2207 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10504 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10510 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2209 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10516 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2210 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10522 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2211 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10528 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2212 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 10534 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2213 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 10540 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2214 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10546 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2221 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 10552 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2222 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10558 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2227 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10567 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2233 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10579 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2242 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10588 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2248 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10600 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
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
#line 10614 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
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
#line 10629 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2279 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10639 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
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
#line 10654 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2298 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10664 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
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
#line 10681 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
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
#line 10695 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2325 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10708 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2333 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10718 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2341 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10731 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2352 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10737 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2353 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10743 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2355 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10749 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2359 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10756 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2361 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10762 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2368 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10768 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2371 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10774 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2378 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10780 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2381 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10786 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2386 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10792 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2387 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10798 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2392 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10804 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2393 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10810 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2397 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_DARRAY);}
#line 10816 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2401 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10822 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2402 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10828 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2407 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10834 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2408 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10840 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2413 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10846 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2414 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10852 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2419 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10858 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2420 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10864 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2426 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10870 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2428 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10876 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2433 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10882 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2434 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10888 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2440 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10894 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2442 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10900 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2446 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10906 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2450 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10912 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2454 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10918 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2458 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10924 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2462 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10930 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2466 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10936 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2470 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10942 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2474 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10948 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2478 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10954 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2482 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10960 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2486 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10966 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2490 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10972 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2494 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10978 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2498 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10984 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2502 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10990 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2506 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10996 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2510 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11002 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2514 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11008 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2519 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11014 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2520 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11020 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2525 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11026 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2526 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11032 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2531 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11038 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2532 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11044 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2537 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11052 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2544 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11060 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2551 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11066 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2553 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11072 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2557 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11078 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2558 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11084 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2559 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11090 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2560 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11096 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2561 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11102 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2562 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11108 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2563 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11114 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2564 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11120 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2565 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11126 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2566 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 11133 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2568 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2569 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11145 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11151 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2574 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 11157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2575 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 11163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2576 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 11169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2583 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 11175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
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
#line 11193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
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
#line 11211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2616 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 11217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2617 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 11223 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2620 "hphp.y" /* yacc.c:1646  */
    { _p->onXhpAttributesStart(); (yyval).reset();}
#line 11229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2622 "hphp.y" /* yacc.c:1646  */
    { _p->onXhpAttributeSpread((yyval), &(yyvsp[-4]), (yyvsp[-1]));}
#line 11235 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2625 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2628 "hphp.y" /* yacc.c:1646  */
    { _p->onOptExprListElem((yyval), &(yyvsp[-1]), (yyvsp[0])); }
#line 11247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2629 "hphp.y" /* yacc.c:1646  */
    {  (yyval).reset();}
#line 11253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2632 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11260 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11268 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11274 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 11286 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2649 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11292 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11298 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2654 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11304 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2656 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 11310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2658 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 11316 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2662 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11322 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11328 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11334 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2665 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11340 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2666 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11346 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2667 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11352 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11358 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2669 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11364 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11370 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2671 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11376 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2672 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11382 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2673 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11388 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2674 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11394 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2675 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2676 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2677 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2678 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2679 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2680 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2681 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2682 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2683 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2684 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2685 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11466 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2687 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11472 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2688 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11478 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2689 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11484 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2690 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11490 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2691 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11496 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11502 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2693 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11508 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2694 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11514 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2695 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11520 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2696 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11526 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2697 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11532 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2698 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11538 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2699 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11544 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2700 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11550 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2701 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11556 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2702 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11562 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2703 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11568 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2704 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11574 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2705 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11580 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2706 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11586 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2707 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11592 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2708 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11598 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2709 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11604 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2710 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11610 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2711 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11616 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2713 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2714 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11634 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2715 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11640 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2716 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11646 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2717 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11652 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11658 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11664 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11670 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11676 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2722 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11682 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11688 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2724 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11694 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11700 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2726 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11706 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2727 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11712 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2728 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11718 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11724 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11730 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2731 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11736 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2732 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11742 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2733 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11748 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11754 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2735 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11760 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11766 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2737 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11772 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11778 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11784 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11790 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11796 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2742 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2743 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2744 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11814 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2749 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11820 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2753 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11826 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2754 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11832 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2758 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11838 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2759 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11844 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2760 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2761 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11857 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2763 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11864 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2767 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11870 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2776 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11876 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2779 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11882 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2780 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2782 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11896 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2792 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11902 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2796 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11908 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2797 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11914 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2798 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11920 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2802 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11926 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2803 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2804 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11938 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2808 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11944 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2809 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11950 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2810 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11956 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2814 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11962 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2815 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2819 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2820 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11980 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2821 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11986 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2822 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11993 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2824 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11999 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2825 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 12005 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2826 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 12011 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2827 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 12017 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2828 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 12023 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2829 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 12029 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2830 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 12035 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2831 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 12041 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2832 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 12047 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2835 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12053 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2837 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12059 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12065 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2842 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12071 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2844 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2845 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12083 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2847 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_DARRAY);}
#line 12089 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2848 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12095 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12101 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2850 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12107 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2851 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12113 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2852 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12119 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2853 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12125 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2854 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12131 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2855 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12137 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2856 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12143 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2858 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 12149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2860 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 12155 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2862 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 12161 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2864 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 12167 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2866 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 12173 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 12179 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2868 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 12185 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2869 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 12191 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2870 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 12197 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2871 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 12203 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2872 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 12209 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2873 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 12215 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2874 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 12221 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2875 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 12227 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2876 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 12233 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2877 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 12239 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2878 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 12245 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2879 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 12251 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2880 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 12257 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2881 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12263 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2882 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12269 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2884 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 12275 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2886 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 12281 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2888 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 12287 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2890 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 12293 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2891 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 12299 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2893 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 12306 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2895 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 12312 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2898 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 12319 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2902 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 12325 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2905 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 12331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2906 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 12337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2910 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12343 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2911 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12349 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2917 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12355 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2923 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12361 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2924 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12367 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2928 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2929 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12379 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2930 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12385 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2931 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12391 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2932 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 12397 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2933 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 12403 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2935 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 12410 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2940 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12416 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2941 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12422 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2945 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12428 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2946 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2949 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 12440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2950 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12446 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2956 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12452 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2958 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12458 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2960 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12464 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2961 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12470 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2965 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12476 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2966 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12482 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2967 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12488 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2970 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12494 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2972 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12500 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2975 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 12506 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2976 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 12512 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2977 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 12518 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2978 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12524 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2982 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 12531 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2985 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 12539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2992 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12545 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2993 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2996 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 12559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2999 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12565 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 3000 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12571 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 3001 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12577 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 3003 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12583 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 3004 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 3006 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_DARRAY);}
#line 12595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 3007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12601 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 3008 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 3009 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 3010 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12619 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 3011 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12625 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 3012 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12631 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12637 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 3018 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12643 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 3023 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12649 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 3024 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12655 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 3029 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12661 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 3031 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12667 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3033 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12673 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3034 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12679 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3038 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12685 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3039 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12691 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3044 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12697 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3045 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12703 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3050 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12709 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3053 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12715 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3058 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12721 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3059 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12727 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3062 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3063 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 12740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3070 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 12746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3072 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 12752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3075 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 12758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3077 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12764 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3080 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3083 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3084 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3088 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12788 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3089 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12794 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3093 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12800 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3094 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12806 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3095 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12812 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3099 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3104 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12824 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3109 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12830 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3110 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12836 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3114 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12842 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3119 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12848 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3124 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12854 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3125 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12860 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3130 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12866 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3131 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12872 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3133 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12878 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3138 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12884 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3140 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12890 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
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
#line 12904 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
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
#line 12918 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
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
#line 12932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
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
#line 12946 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3196 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12952 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3197 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12958 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3198 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12964 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3199 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12970 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3200 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12976 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3201 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12982 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
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
#line 12996 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3220 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13002 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3222 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13008 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3224 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13014 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3225 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13020 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3229 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13026 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3233 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13032 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3234 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13038 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3235 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13044 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3236 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13050 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
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
#line 13064 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3253 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13070 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3255 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13076 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3259 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13082 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3264 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13088 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3265 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13094 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3266 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13100 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3267 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13106 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3268 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13112 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3269 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13118 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3270 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13124 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3272 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13130 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3274 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13136 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3278 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13142 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3282 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13148 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3283 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13154 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3289 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13160 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3293 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13166 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3297 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13172 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3304 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 13178 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3313 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 13184 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3317 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 13190 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3321 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13196 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3330 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13202 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3331 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13208 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3332 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13214 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3336 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3337 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 13226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3338 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 13232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3340 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]) = 1; _p->onIndirectRef((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 13238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3345 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13244 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3346 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13250 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13256 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3358 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13262 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3359 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13268 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
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
#line 13282 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3373 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13288 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3374 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13294 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3378 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13300 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3379 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13306 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
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
#line 13320 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3391 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13326 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3395 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 13332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3396 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 13338 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3398 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 13344 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3399 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 13350 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3400 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 13356 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3401 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 13362 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3406 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13368 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3407 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13374 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3411 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13380 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3412 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13386 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3413 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13392 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3414 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13398 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3417 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13404 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3419 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 13410 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3420 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13416 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3421 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 13422 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3426 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13428 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3427 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3431 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3432 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13446 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3433 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13452 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3434 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13458 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3439 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13464 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3440 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13470 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3445 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13476 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3447 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13482 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3449 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13488 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3450 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13494 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3454 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 13500 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3456 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 13506 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1030:
#line 3457 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 13512 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1031:
#line 3459 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 13519 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1032:
#line 3464 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13525 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1033:
#line 3466 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13531 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1034:
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
#line 13545 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1035:
#line 3478 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 13551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1036:
#line 3480 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 13557 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3481 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13563 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1038:
#line 3484 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 13569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1039:
#line 3485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 13575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 13581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3490 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 13587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3491 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 13593 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3492 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13599 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3493 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13605 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3494 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13611 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3495 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13617 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3496 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 13623 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3497 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 13629 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3498 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 13635 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3499 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 13641 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3500 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 13647 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3504 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 13653 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3505 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 13659 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3510 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13665 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3512 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13671 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3526 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13679 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3531 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 13687 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3535 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13695 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3540 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 13703 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3546 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13709 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3547 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13715 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3551 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13721 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3552 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13727 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3558 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3562 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 13739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3568 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13745 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3572 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 13752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3579 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3580 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13764 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3584 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 13772 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3587 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 13779 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3593 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13785 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3597 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13793 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3600 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13801 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1077:
#line 3603 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-3]); }
#line 13808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1078:
#line 3605 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13815 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1079:
#line 3607 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13822 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1080:
#line 3609 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13828 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1081:
#line 3614 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]); }
#line 13834 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1082:
#line 3615 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13840 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1083:
#line 3616 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13846 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1084:
#line 3617 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13852 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1091:
#line 3638 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13858 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1092:
#line 3639 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13864 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1095:
#line 3648 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13870 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1098:
#line 3659 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13876 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1099:
#line 3661 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13882 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1100:
#line 3665 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13888 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1101:
#line 3668 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13894 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1102:
#line 3672 "hphp.y" /* yacc.c:1646  */
    {}
#line 13900 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1103:
#line 3673 "hphp.y" /* yacc.c:1646  */
    {}
#line 13906 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1104:
#line 3674 "hphp.y" /* yacc.c:1646  */
    {}
#line 13912 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1105:
#line 3680 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1106:
#line 3685 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1107:
#line 3694 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1108:
#line 3700 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13944 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1109:
#line 3708 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13950 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1110:
#line 3709 "hphp.y" /* yacc.c:1646  */
    { }
#line 13956 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1111:
#line 3715 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13962 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1112:
#line 3717 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1113:
#line 3718 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13978 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1114:
#line 3723 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13985 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1115:
#line 3729 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("HH\\darray"); }
#line 13992 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1116:
#line 3734 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13998 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1117:
#line 3739 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 14006 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1118:
#line 3743 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14012 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1119:
#line 3748 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 14018 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1120:
#line 3750 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 14024 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1121:
#line 3756 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 14031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1122:
#line 3758 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 14039 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1123:
#line 3761 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14045 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1124:
#line 3762 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14053 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1125:
#line 3765 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14061 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1126:
#line 3768 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14067 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1127:
#line 3771 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 14075 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1128:
#line 3774 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14082 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1129:
#line 3776 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 14091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1130:
#line 3782 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 14100 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1131:
#line 3788 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("HH\\varray");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 14110 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1132:
#line 3796 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14116 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1133:
#line 3797 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 14122 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;


#line 14126 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
