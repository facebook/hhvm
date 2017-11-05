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

#line 661 "hphp.7.tab.cpp" /* yacc.c:339  */

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

#line 905 "hphp.7.tab.cpp" /* yacc.c:358  */

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
#define YYLAST   19993

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  209
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  317
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1132
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  2108

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
       0,   759,   759,   759,   768,   770,   773,   774,   775,   776,
     777,   778,   779,   782,   784,   784,   786,   786,   788,   791,
     796,   801,   804,   807,   811,   815,   819,   823,   827,   832,
     833,   834,   835,   836,   837,   838,   839,   840,   841,   842,
     843,   844,   848,   849,   850,   851,   852,   853,   854,   855,
     856,   857,   858,   859,   860,   861,   862,   863,   864,   865,
     866,   867,   868,   869,   870,   871,   872,   873,   874,   875,
     876,   877,   878,   879,   880,   881,   882,   883,   884,   885,
     886,   887,   888,   889,   890,   891,   892,   893,   894,   895,
     896,   897,   898,   899,   900,   901,   902,   903,   904,   905,
     906,   907,   908,   909,   910,   911,   912,   916,   917,   921,
     922,   926,   927,   932,   934,   939,   944,   945,   946,   948,
     953,   955,   960,   965,   967,   969,   974,   975,   979,   980,
     982,   986,   993,  1000,  1004,  1010,  1012,  1016,  1017,  1023,
    1025,  1029,  1031,  1036,  1037,  1038,  1039,  1042,  1043,  1047,
    1052,  1052,  1058,  1058,  1065,  1064,  1070,  1070,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1093,  1091,  1100,  1098,  1105,  1115,  1109,
    1119,  1117,  1121,  1125,  1129,  1133,  1137,  1141,  1145,  1150,
    1151,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,
    1164,  1165,  1166,  1188,  1194,  1195,  1204,  1206,  1210,  1211,
    1212,  1216,  1217,  1221,  1221,  1226,  1232,  1236,  1236,  1244,
    1245,  1249,  1250,  1254,  1260,  1258,  1275,  1272,  1290,  1287,
    1305,  1304,  1313,  1311,  1323,  1322,  1341,  1339,  1358,  1357,
    1366,  1364,  1375,  1375,  1382,  1381,  1393,  1391,  1404,  1405,
    1409,  1412,  1415,  1416,  1417,  1420,  1421,  1424,  1426,  1429,
    1430,  1433,  1434,  1437,  1438,  1442,  1443,  1448,  1449,  1452,
    1453,  1454,  1458,  1459,  1463,  1464,  1468,  1469,  1473,  1474,
    1479,  1480,  1486,  1487,  1488,  1489,  1492,  1495,  1497,  1500,
    1501,  1505,  1507,  1510,  1513,  1516,  1517,  1520,  1521,  1525,
    1531,  1537,  1544,  1546,  1551,  1556,  1562,  1566,  1571,  1576,
    1581,  1587,  1593,  1599,  1605,  1611,  1618,  1628,  1633,  1638,
    1644,  1646,  1650,  1654,  1659,  1663,  1667,  1671,  1675,  1680,
    1685,  1690,  1695,  1700,  1706,  1715,  1716,  1717,  1721,  1723,
    1726,  1728,  1730,  1732,  1734,  1737,  1740,  1743,  1749,  1750,
    1753,  1754,  1755,  1759,  1760,  1762,  1763,  1767,  1769,  1772,
    1776,  1782,  1784,  1787,  1787,  1791,  1790,  1794,  1796,  1799,
    1802,  1800,  1817,  1814,  1829,  1831,  1833,  1835,  1837,  1839,
    1841,  1845,  1846,  1847,  1850,  1856,  1860,  1866,  1869,  1874,
    1876,  1881,  1886,  1890,  1891,  1895,  1896,  1898,  1900,  1906,
    1907,  1909,  1913,  1914,  1919,  1923,  1924,  1928,  1929,  1933,
    1935,  1941,  1946,  1947,  1949,  1953,  1954,  1955,  1956,  1960,
    1961,  1962,  1963,  1964,  1965,  1967,  1972,  1975,  1976,  1980,
    1981,  1985,  1986,  1989,  1990,  1993,  1994,  1997,  1998,  2002,
    2003,  2004,  2005,  2006,  2007,  2008,  2012,  2013,  2016,  2017,
    2018,  2021,  2023,  2025,  2026,  2029,  2031,  2035,  2037,  2041,
    2045,  2049,  2054,  2058,  2059,  2061,  2062,  2063,  2064,  2067,
    2068,  2072,  2073,  2077,  2078,  2079,  2080,  2084,  2088,  2093,
    2097,  2101,  2105,  2109,  2114,  2118,  2119,  2120,  2121,  2122,
    2126,  2130,  2132,  2133,  2134,  2137,  2138,  2139,  2140,  2141,
    2142,  2143,  2144,  2145,  2146,  2147,  2148,  2149,  2150,  2151,
    2152,  2153,  2154,  2155,  2156,  2157,  2158,  2159,  2160,  2161,
    2162,  2163,  2164,  2165,  2166,  2167,  2168,  2169,  2170,  2171,
    2172,  2173,  2174,  2175,  2176,  2177,  2178,  2179,  2180,  2182,
    2183,  2185,  2186,  2188,  2189,  2190,  2191,  2192,  2193,  2194,
    2195,  2196,  2197,  2198,  2199,  2200,  2201,  2202,  2203,  2204,
    2205,  2206,  2207,  2208,  2209,  2210,  2211,  2212,  2213,  2217,
    2221,  2226,  2225,  2241,  2239,  2258,  2257,  2278,  2277,  2297,
    2296,  2315,  2315,  2332,  2332,  2351,  2352,  2353,  2358,  2360,
    2364,  2368,  2374,  2378,  2384,  2386,  2390,  2392,  2396,  2400,
    2401,  2405,  2407,  2411,  2413,  2417,  2419,  2423,  2426,  2431,
    2433,  2437,  2440,  2445,  2449,  2453,  2457,  2461,  2465,  2469,
    2473,  2477,  2481,  2485,  2489,  2493,  2497,  2501,  2505,  2509,
    2513,  2517,  2519,  2523,  2525,  2529,  2531,  2535,  2542,  2549,
    2551,  2556,  2557,  2558,  2559,  2560,  2561,  2562,  2563,  2564,
    2565,  2567,  2568,  2572,  2573,  2574,  2575,  2579,  2585,  2598,
    2615,  2616,  2619,  2622,  2625,  2626,  2629,  2633,  2636,  2639,
    2646,  2647,  2651,  2652,  2654,  2659,  2660,  2661,  2662,  2663,
    2664,  2665,  2666,  2667,  2668,  2669,  2670,  2671,  2672,  2673,
    2674,  2675,  2676,  2677,  2678,  2679,  2680,  2681,  2682,  2683,
    2684,  2685,  2686,  2687,  2688,  2689,  2690,  2691,  2692,  2693,
    2694,  2695,  2696,  2697,  2698,  2699,  2700,  2701,  2702,  2703,
    2704,  2705,  2706,  2707,  2708,  2709,  2710,  2711,  2712,  2713,
    2714,  2715,  2716,  2717,  2718,  2719,  2720,  2721,  2722,  2723,
    2724,  2725,  2726,  2727,  2728,  2729,  2730,  2731,  2732,  2733,
    2734,  2735,  2736,  2737,  2738,  2739,  2740,  2741,  2745,  2750,
    2751,  2755,  2756,  2757,  2758,  2760,  2764,  2765,  2776,  2777,
    2779,  2781,  2793,  2794,  2795,  2799,  2800,  2801,  2805,  2806,
    2807,  2810,  2812,  2816,  2817,  2818,  2819,  2821,  2822,  2823,
    2824,  2825,  2826,  2827,  2828,  2829,  2830,  2833,  2838,  2839,
    2840,  2842,  2843,  2845,  2846,  2847,  2848,  2849,  2850,  2851,
    2852,  2853,  2854,  2856,  2858,  2860,  2862,  2864,  2865,  2866,
    2867,  2868,  2869,  2870,  2871,  2872,  2873,  2874,  2875,  2876,
    2877,  2878,  2879,  2880,  2882,  2884,  2886,  2888,  2889,  2892,
    2893,  2897,  2901,  2903,  2907,  2908,  2912,  2918,  2921,  2925,
    2926,  2927,  2928,  2929,  2930,  2931,  2936,  2938,  2942,  2943,
    2946,  2947,  2951,  2954,  2956,  2958,  2962,  2963,  2964,  2965,
    2968,  2972,  2973,  2974,  2975,  2979,  2981,  2988,  2989,  2990,
    2991,  2996,  2997,  2998,  2999,  3001,  3002,  3004,  3005,  3006,
    3007,  3008,  3009,  3013,  3015,  3019,  3021,  3024,  3027,  3029,
    3031,  3034,  3036,  3040,  3042,  3045,  3048,  3054,  3056,  3059,
    3060,  3065,  3068,  3072,  3072,  3077,  3080,  3081,  3085,  3086,
    3090,  3091,  3092,  3096,  3101,  3106,  3107,  3111,  3116,  3121,
    3122,  3126,  3128,  3129,  3134,  3136,  3141,  3152,  3166,  3178,
    3193,  3194,  3195,  3196,  3197,  3198,  3199,  3209,  3218,  3220,
    3222,  3226,  3230,  3231,  3232,  3233,  3234,  3250,  3251,  3254,
    3261,  3262,  3263,  3264,  3265,  3266,  3267,  3268,  3270,  3275,
    3279,  3280,  3284,  3287,  3291,  3298,  3302,  3311,  3318,  3326,
    3328,  3329,  3333,  3334,  3335,  3337,  3342,  3343,  3354,  3355,
    3356,  3357,  3368,  3371,  3374,  3375,  3376,  3377,  3388,  3392,
    3393,  3394,  3396,  3397,  3398,  3402,  3404,  3407,  3409,  3410,
    3411,  3412,  3415,  3417,  3418,  3422,  3424,  3427,  3429,  3430,
    3431,  3435,  3437,  3440,  3443,  3445,  3447,  3451,  3452,  3454,
    3455,  3461,  3462,  3464,  3474,  3476,  3478,  3481,  3482,  3483,
    3487,  3488,  3489,  3490,  3491,  3492,  3493,  3494,  3495,  3496,
    3497,  3501,  3502,  3506,  3508,  3516,  3518,  3522,  3526,  3531,
    3535,  3543,  3544,  3548,  3549,  3555,  3556,  3565,  3566,  3574,
    3577,  3581,  3584,  3589,  3594,  3597,  3600,  3602,  3604,  3606,
    3610,  3612,  3613,  3614,  3617,  3619,  3625,  3626,  3630,  3631,
    3635,  3636,  3640,  3641,  3644,  3649,  3650,  3654,  3657,  3659,
    3663,  3669,  3670,  3671,  3675,  3679,  3687,  3692,  3704,  3706,
    3710,  3713,  3715,  3720,  3725,  3731,  3734,  3739,  3744,  3746,
    3753,  3755,  3758,  3759,  3762,  3765,  3766,  3771,  3773,  3777,
    3783,  3793,  3794
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

#define YYPACT_NINF -1707

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1707)))

#define YYTABLE_NINF -1133

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1133)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1707,   192, -1707, -1707,  5605, 14909, 14909,    22, 14909, 14909,
   14909, 14909, 12510, 14909, -1707, 14909, 14909, 14909, 14909, 18160,
   18160, 14909, 14909, 14909, 14909, 14909, 14909, 14909, 14909, 12689,
   18958, 14909,    50,    61, -1707, -1707, -1707,   193, -1707,   259,
   -1707, -1707, -1707,   208, 14909, -1707,    61,   220,   223,   231,
   -1707,    61, 12868, 15999, 13047, -1707, 15927, 11373,   245, 14909,
   19207,    66,    86,   462,   495, -1707, -1707, -1707,   300,   345,
     383,   387, -1707, 15999,   400,   407,   298,   390,   559,   601,
     613, -1707, -1707, -1707, -1707, -1707, 14909,   596,  2696, -1707,
   -1707, 15999, -1707, -1707, -1707, -1707, 15999, -1707, 15999, -1707,
     468,   440,   487, 15999, 15999, -1707,   361, -1707, -1707, 13253,
   -1707, -1707,   355,   578,   626,   626, -1707,   664,   523,   606,
     501, -1707,   102, -1707,   507,   593,   675, -1707, -1707, -1707,
   -1707, 15309,   569, -1707,   148, -1707,   517,   520,   531,   539,
     546,   566,   568,   580,  5131, -1707, -1707, -1707, -1707, -1707,
     130,   692,   694,   714,   724,   741,   750, -1707,   762,   779,
   -1707,   219,   635, -1707,   636,   225, -1707,  1380,   265, -1707,
   -1707,  3845,   148,   148,   652,   226, -1707,   180,   126,   663,
     214, -1707, -1707,   794, -1707,   704, -1707, -1707,   669,   707,
   -1707, 14909, -1707,   675,   569, 19495,  4429, 19495, 14909, 19495,
   19495, 16476, 16476,   696, 18333, 19495,   821, 15999,   824,   824,
     481,   824, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707,
   -1707,    82, 14909,   721, -1707, -1707,   744,   711,   607,   753,
     607,   824,   824,   824,   824,   824,   824,   824,   824, 18160,
   18381,   709,   940,   704, -1707, 14909,   721, -1707,   792, -1707,
     795,   768, -1707,   186, -1707, -1707, -1707,   607,   148, -1707,
   13432, -1707, -1707, 14909, 10137,   954,   123, 19495, 11167, -1707,
   14909, 14909, 15999, -1707, -1707,  5253,   773, -1707,  5538, -1707,
   -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, 17453,
   -1707, 17453, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707,
   -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707,
   -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707,
   -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707,
   -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707,
   -1707,   103,   104,   707, -1707, -1707, -1707, -1707,   777, -1707,
   17402,   116, -1707, -1707,   817,   970, -1707,   828, 16668, 14909,
   -1707,   791,   793, 17147, -1707,    46, 17195,  1948,  1948,  1948,
   15999,  1948,   800,   987,   802, -1707,   472, -1707, 17827,   124,
   -1707,   990,   129,   877, -1707,   879, -1707, 18160, 14909, 14909,
     822,   838, -1707, -1707, 17903, 12689, 14909, 14909, 14909, 14909,
   14909,   133,   115,    78, -1707, 15088, 18160,   748, -1707, 15999,
   -1707,   261,   523, -1707, -1707, -1707, -1707, 19060, 14909,  1009,
     921, -1707, -1707, -1707,    87, 14909,   826,   827, 19495,   830,
    1524,   832,  6223, 14909, -1707,   428,   829,   632,   428,   581,
     543, -1707, 15999, 17453,   834, 11552, 15927, -1707, 13638,   836,
     836,   836,   836, -1707, -1707,  4064, -1707, -1707, -1707, -1707,
   -1707,   675, -1707, 14909, 14909, -1707, -1707, -1707, -1707, -1707,
   -1707, -1707, -1707, -1707, 14909, 14909, 14909, 14909, 13817, 14909,
   14909, 14909, 14909, 14909, 14909, 14909, 14909, 14909, 14909, 14909,
   14909, 14909, 14909, 14909, 14909, 14909, 14909, 14909, 14909, 14909,
   14909, 14909, 19136, 14909, -1707, 14909, 14909, 14909, 15261, 15999,
   15999, 15999, 15999, 15999, 15309,   924,   649,  5347, 14909, 14909,
   14909, 14909, 14909, 14909, 14909, 14909, 14909, 14909, 14909, 14909,
   -1707, -1707, -1707, -1707,  2312, -1707, -1707, 11552, 11552, 14909,
   14909, 17903,   841,   675, 13996, 17243, -1707, 14909, -1707,   842,
    1033,   885,   845,   846, 15415,   607, 14175, -1707, 14354, -1707,
     768,   852,   854,  3032, -1707,   443, 11552, -1707,  4082, -1707,
   -1707, 17292, -1707, -1707, 11928, -1707, 14909, -1707,   967, 10343,
    1056,   865, 19373,  1057,    85,    90, -1707, -1707, -1707,   889,
   -1707, -1707, -1707, 17453, -1707,  2327,   871,  1067, 17751, 15999,
   -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707,   878,
   -1707, -1707,   880,   883,   888,   891,   892,   895,   563,   896,
     898, 16178,  5456, -1707, -1707, 15999, 15999, 14909,   607,    66,
   -1707, 17751,   996, -1707, -1707, -1707,   607,    93,   100,   900,
     901,  3221,   146,   905,   908,   778,   972,   911,   607,   110,
     907, 18442,   912,  1102,  1103,   931,   946,   948,   957, -1707,
   15820, 15999, -1707, -1707,  1045,  2970,    44, -1707, -1707, -1707,
     523, -1707, -1707, -1707,  1096,  1028,   988,   269,  1007, 14909,
    1035,  1165,   975, -1707,  1014, -1707,   203, -1707,   977, 17453,
   17453,  1167,   954,    87, -1707,   983,  1173, -1707,  4222,    97,
   -1707,   513,   371, -1707, -1707, -1707, -1707, -1707, -1707, -1707,
    1023,  3103, -1707, -1707, -1707, -1707,  1175,  1002, -1707, 18160,
     763, 14909,  1003,  1179, 19495,  1188,   132,  1195,  1011,  1016,
    1018, 19495,  1019,  3394,  6429, -1707, -1707, -1707, -1707, -1707,
   -1707,  1073,  4334, 19495,  1005,  3726, 19588, 19679, 16476, 19716,
   14909, 19447, 19789, 15262, 17328,  4903, 15928, 19143, 19924, 19924,
   19924, 19924,  2532,  2532,  2532,  2532,  2532,  1068,  1068,   887,
     887,   887,   481,   481,   481, -1707,   824,  1017,  1020, 18490,
    1022,  1211,    36, 14909,    37,   721,   227, -1707, -1707, -1707,
    1209,   921, -1707,   675, 18008, -1707, -1707, -1707, 16476, 16476,
   16476, 16476, 16476, 16476, 16476, 16476, 16476, 16476, 16476, 16476,
   16476, -1707, 14909,   250, -1707,   204, -1707,   721,   391,  1026,
    1027,  1024,  3906,   151,  1030, -1707, 19495,  3985, -1707, 15999,
   -1707,   607,   478, 18160, 19495, 18160, 18551,  1073,   145,   607,
     232, -1707,   203,  1070,  1034, 14909, -1707,   305, -1707, -1707,
   -1707,  6635,   784, -1707, -1707, 19495, 19495,    61, -1707, -1707,
   -1707, 14909,  1127, 17675, 17751, 15999, 10549,  1032,  1036, -1707,
    1227,  4558,  1100, -1707,  1078, -1707,  1232,  1042,  5171, 17453,
   17751, 17751, 17751, 17751, 17751,  1044,  1174,  1177,  1178,  1182,
    1184,  1059,  1060, 17751,    42, -1707, -1707, -1707, -1707, -1707,
   -1707, -1707,    26, -1707, 19543, -1707, -1707,   417, -1707,  6841,
   15641,  1053,  5456, -1707,  5456, -1707,  5456, -1707, 15999, 15999,
    5456, -1707,  5456,  5456, 15999, -1707,  1251,  1058, -1707,   565,
   -1707, -1707,  3968, -1707, 19543,  1249, 18160,  1064, -1707, -1707,
   -1707, -1707, -1707, -1707, -1707, -1707, -1707,  1085,  1262, 15999,
   15641,  1071, 17903, 18084,  1258, -1707, 14909, -1707, 14909, -1707,
   14909, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707,  1077,
   -1707, 14909, -1707, -1707,  5811, -1707, 17453, 15641,  1081, -1707,
   -1707, -1707, -1707,  1270,  1098, 14909, 19060, -1707, -1707, 15261,
   -1707,  1084, -1707, 17453, -1707,  1104,  7047,  1272,    76, -1707,
   17453, -1707,   101,  2312,  2312, -1707, 17453, -1707, -1707,   607,
   -1707, -1707,  1235, 19495, -1707, 11731, -1707, 17751, 13638,   836,
   13638, -1707,   836,   836, -1707, 12134, -1707, -1707,  7253, -1707,
      79,  1109, 15641,  1028, -1707, -1707, -1707, -1707, 19789, 14909,
   -1707, -1707, 14909, -1707, 14909, -1707,  4302,  1111, 11552,   972,
    1282,  1028, 17453,  1288,  1073, 15999, 19136,   607,  4544,  1120,
     382,  1121, -1707, -1707,  1308,  1799,  1799,  3985, -1707, -1707,
   -1707,  1273,  1128,  1259,  1260,  1264,  1265,  1267,    98,  1129,
    1139,    48, -1707, -1707, -1707, -1707, -1707, -1707,  1180, -1707,
   -1707, -1707, -1707,  1330,  1140,   842,   607,   607, 14533,  1028,
    4082, -1707,  4082, -1707,  4857,   812,    61, 11167, -1707,  7459,
    1142,  7665,  1143, 17675, 18160,  1160,  1225,   607, 19543,  1351,
   -1707, -1707, -1707, -1707,   747, -1707,   105, 17453,  1183,  1228,
    1206, 17453, 15999,  3374, -1707, -1707, 17453,  1360,  1170,  1197,
    1199,  1175,   906,   906,  1307,  1307, 18708,  1176,  1373, 17751,
   17751, 17751, 17751, 17751, 17751, 19060, 17751,  2592, 16822, 17751,
   17751, 17751, 17751, 17599, 17751, 17751, 17751, 17751, 17751, 17751,
   17751, 17751, 17751, 17751, 17751, 17751, 17751, 17751, 17751, 17751,
   17751, 17751, 17751, 17751, 17751, 17751, 17751, 15999, -1707, -1707,
    1300, -1707, -1707,  1181,  1186,  1189, -1707,  1191, -1707, -1707,
     582, 16178, -1707,  1190, -1707, 17751,   607, -1707, -1707,   172,
   -1707,   787,  1376, -1707, -1707,   162,  1196,   607, 12331, 19495,
   18599, -1707,  2719, -1707,  6017,   921,  1376, -1707,   628,   264,
   -1707, 19495,  1261,  1203, -1707,  1208,  1272, -1707, -1707, -1707,
   14730, 17453,   954, 17469,  1304,   354,  1384,  1327,   334, -1707,
     721,   338, -1707,   721, -1707, 14909, 18160,   763, 14909, 19495,
   19543, -1707, -1707, -1707,  4858, -1707, -1707, -1707, -1707, -1707,
   -1707,  1213,    79, -1707,  1214,    79,  1212, 19789, 19495, 18660,
    1216, 11552,  1217,  1218, 17453,  1219,  1221, 17453,  1028, -1707,
     768,   394, 11552, 14909, -1707, -1707, -1707, -1707, -1707, -1707,
    1280,  1215,  1414,  1332,  3985,  3985,  3985,  3985,  3985,  3985,
    1268, -1707, 19060,  3985,   113,  3985, -1707, -1707, -1707, 18160,
   19495,  1224, -1707,    61,  1397,  1353, 11167, -1707, -1707, -1707,
    1230, 14909,  1225,   607, 17903, 17675,  1233, 17751,  7871,   770,
    1237, 14909,    94,   307, -1707,  1252, -1707, 17453, 15999, -1707,
    1305, -1707, -1707, -1707, 17353, -1707,  1410, -1707,  1245, 17751,
   -1707, 17751, -1707,  1246,  1247,  1440, 18768,  1248, 19543,  1444,
    1250,  1255,  1256,  1317,  1447,  1266,  1275, -1707, -1707, -1707,
   18814,  1274,  1452, 19635, 16664, 19752, 17751, 17683, 19860,  5167,
   19893, 16107,  3020, 18091, 18091, 18091, 18091,  1303,  1303,  1303,
    1303,  1303,  1006,  1006,   906,   906,   906,  1307,  1307,  1307,
    1307, -1707,  1276, -1707,  1263,  1281,  1283,  1284, -1707, -1707,
   19543, 15999, 17453, 17453, -1707,   787, 15641,  1604, -1707, 17903,
   -1707, -1707, 16476, 14909,  1278, -1707,  1285,  1839, -1707,   381,
   14909, -1707, -1707, -1707, 14909, -1707, 14909, -1707,   954, 13638,
    1289,   353,   836,   353,   455, -1707, -1707, 17453,   144, -1707,
    1453,  1385, 14909, -1707,  1287,  1290,  1279,   607,  1235, 19495,
    1272,  1291, -1707,  1292,    79, 14909, 11552,  1294, -1707, -1707,
     921, -1707, -1707,  1295,  1297,  1298, -1707,  1309,  3985, -1707,
    3985, -1707, -1707,  1311,  1296,  1498,  1377,  1312, -1707,  1509,
    1315,  1318,  1319, -1707,  1383,  1324,  1516,  1326, -1707, -1707,
     607, -1707,  1495, -1707,  1328, -1707, -1707,  1331,  1333,   166,
   -1707, -1707, 19543,  1329,  1335, -1707,  4637, -1707, -1707, -1707,
   -1707, -1707, -1707,  1390, 17453, 17453,  1197,  1356, 17453, -1707,
   19543, 18874, -1707, -1707, 17751, -1707, 17751, -1707, 17751, -1707,
   -1707, -1707, -1707, 17751, 19060, -1707, -1707, -1707, 17751, -1707,
   17751, -1707, 19825, 17751,  1352,  8077, -1707, -1707, -1707, -1707,
     787, -1707, -1707, -1707, -1707,   767, 16106, 15641,  1424, -1707,
    3228,  1386,  3123, -1707, -1707, -1707,   924, 17282,   136,   137,
    1355,   921,   649,   174, 19495, -1707, -1707, -1707,  1391,  5034,
    5082, 19495, -1707,  3828, -1707,  6429,  1476,   420,  1546,  1480,
   14909, -1707, 19495, 11552, 11552, -1707,  1445,  1272,  2141,  1272,
    1365, 19495,  1366, -1707,  2161,  1368,  2177, -1707, -1707,    79,
   -1707, -1707,  1430, -1707, -1707,  3985, -1707,  3985, -1707,  3985,
   -1707, -1707, -1707, -1707,  3985, -1707, 19060, -1707, -1707,  2196,
   -1707,  8283, -1707, -1707, -1707, -1707, 10755, -1707, -1707, -1707,
    6635, 17453, -1707, -1707, -1707,  1375, 17751, 18920, 19543, 19543,
   19543,  1438, 19543, 18980, 19825, -1707, -1707,   787, 15641, 15641,
   15999, -1707,  1564, 16976,    89, -1707, 16106,   921,  4835, -1707,
    1396, -1707,   139,  1378,   147, -1707, 16475, -1707, -1707, -1707,
     156, -1707, -1707,  3326, -1707,  1381, -1707,  1502,   675, -1707,
   16296, -1707, 16296, -1707, -1707,  1567,   924, -1707, 15569, -1707,
   -1707, -1707, -1707,  2905, -1707,  1572,  1505, 14909, -1707, 19495,
    1392,  1393,  1395,  1272,  1398, -1707,  1445,  1272, -1707, -1707,
   -1707, -1707,  2319,  1399,  3985,  1459, -1707, -1707, -1707,  1463,
   -1707,  6635, 10961, 10755, -1707, -1707, -1707,  6635, -1707, -1707,
   19543, 17751, 17751, 17751,  8489,  1389,  1401, -1707, 17751, -1707,
   15641, -1707, -1707, -1707, -1707, -1707, 17453,   734,  3228, -1707,
   -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707,
   -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707,
   -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707,
   -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707,
   -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707,
   -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707,
   -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707,
   -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707,
   -1707, -1707, -1707,   210, -1707,  1386, -1707, -1707, -1707, -1707,
   -1707,   141,   415, -1707,  1577,   157, 16668,  1502,  1589, -1707,
   17453,   675, -1707, -1707,  1403,  1592, 14909, -1707, 19495, -1707,
   -1707,   149,  1405, 17453,   728,  1272,  1398, 15748, -1707,  1272,
   -1707,  3985,  3985, -1707, -1707, -1707, -1707,  8695, 19543, 19543,
   19543, -1707, -1707, -1707, 19543, -1707,  1912,  1602,  1603,  1411,
   -1707, -1707, 17751, 16475, 16475,  1545, -1707,  3326,  3326,   657,
   -1707, -1707, -1707, 17751,  1533, -1707,  1435,  1421,   159, 17751,
   -1707, 16668, -1707, 17751, 19495,  1538, -1707,  1614, -1707,  1615,
   -1707,   165, -1707, -1707, -1707,  1423,   728, -1707,   728, -1707,
   -1707,  8901,  1425,  1513, -1707,  1527,  1469, -1707, -1707,  1529,
   17453,  1449,   734, -1707, -1707, 19543, -1707, -1707,  1460, -1707,
    1599, -1707, -1707, -1707, -1707, 19543,  1622,   778, -1707, -1707,
   19543,  1439, 19543, -1707,   170,  1437,  9107, 17453, -1707, 17453,
   -1707,  9313, -1707, -1707, -1707,  1441, -1707,  1454,  1464, 15999,
     649,  1462, -1707, -1707, -1707, 17751,  1467,   109, -1707,  1560,
   -1707, -1707, -1707, -1707, -1707, -1707,  9519, -1707, 15641,  1053,
   -1707,  1471, 15999,   813, -1707, 19543, -1707,  1455,  1645,   693,
     109, -1707, -1707,  1573, -1707, 15641,  1456, -1707,  1272,   143,
   -1707, 17453, -1707, -1707, -1707, 17453, -1707,  1466,  1468,   169,
   -1707,  1398,   710,  1576,   160,  1272,  1461, -1707,   730, 17453,
   17453, -1707,   425,  1648,  1584,  1398, -1707, -1707, -1707, -1707,
    1586,   163,  1658,  1590, 14909, -1707,   730,  9725,  9931, -1707,
     445,  1660,  1593, 14909, -1707, 19495, -1707, -1707, -1707,  1665,
    1597, 14909, -1707, 19495, 14909, -1707, 19495, 19495
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   204,   473,     0,   913,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1006,
     994,     0,   777,     0,   783,   784,   785,    29,   850,   982,
     983,   171,   172,   786,     0,   152,     0,     0,     0,     0,
      30,     0,     0,     0,     0,   223,     0,     0,     0,     0,
       0,     0,   442,   443,   444,   441,   440,   439,     0,     0,
       0,     0,   252,     0,     0,     0,    37,    38,    40,    41,
      39,   790,   792,   793,   787,   788,     0,     0,     0,   794,
     789,     0,   760,    32,    33,    34,    36,    35,     0,   791,
       0,     0,     0,     0,     0,   795,   445,   583,    31,     0,
     170,   140,     0,   778,     0,     0,     4,   126,   128,   849,
       0,   759,     0,     6,     0,     0,   222,     7,     9,     8,
      10,     0,     0,   437,     0,   487,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   543,   485,   970,   971,   565,
     558,   559,   560,   561,   564,   562,   563,   468,   568,     0,
     467,   941,   761,   768,     0,   852,   557,   436,   944,   945,
     957,   486,     0,     0,     0,   489,   488,   942,   943,   940,
     978,   981,   547,   851,    11,   442,   443,   444,     0,     0,
      36,     0,   126,   222,     0,  1046,   486,  1047,     0,  1049,
    1050,   567,   481,     0,   474,   479,     0,     0,   529,   530,
     531,   532,    29,   982,   786,   763,    37,    38,    40,    41,
      39,     0,     0,  1070,   963,   761,     0,   762,   508,     0,
     510,   548,   549,   550,   551,   552,   553,   554,   556,     0,
    1010,     0,   859,   773,   242,     0,  1070,   465,   772,   766,
       0,   782,   762,   989,   990,   996,   988,   774,     0,   466,
       0,   776,   555,     0,   205,     0,     0,   470,   205,   150,
     472,     0,     0,   156,   158,     0,     0,   160,     0,    75,
      76,    82,    83,    67,    68,    59,    80,    91,    92,     0,
      62,     0,    66,    74,    72,    94,    86,    85,    57,   108,
      81,   101,   102,    58,    97,    55,    98,    56,    99,    54,
     103,    90,    95,   100,    87,    88,    61,    89,    93,    53,
      84,    69,   104,    77,   106,    70,    60,    47,    48,    49,
      50,    51,    52,    71,   107,   105,   110,    64,    45,    46,
      73,  1123,  1124,    65,  1128,    44,    63,    96,     0,    79,
       0,   126,   109,  1061,  1122,     0,  1125,     0,     0,     0,
     162,     0,     0,     0,   213,     0,     0,     0,     0,     0,
       0,     0,     0,   861,     0,   114,   116,   350,     0,     0,
     349,   355,     0,     0,   253,     0,   256,     0,     0,     0,
       0,  1067,   238,   250,  1002,  1006,   602,   632,   632,   602,
     632,     0,  1031,     0,   797,     0,     0,     0,  1029,     0,
      16,     0,   130,   230,   244,   251,   663,   595,   632,     0,
    1055,   575,   577,   579,   917,   473,   487,     0,     0,   485,
     486,   488,   205,     0,   985,   779,     0,   780,     0,     0,
       0,   202,     0,     0,   132,   339,     0,    28,     0,     0,
       0,     0,     0,   203,   221,     0,   249,   234,   248,   442,
     445,   222,   438,   987,     0,   933,   192,   193,   194,   195,
     196,   198,   199,   201,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   994,     0,   191,   987,   987,  1016,     0,     0,
       0,     0,     0,     0,     0,     0,   435,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     507,   509,   918,   919,     0,   932,   931,   339,   339,   987,
       0,  1002,     0,   222,     0,     0,   164,     0,   915,   910,
     859,     0,   487,   485,     0,  1014,     0,   600,   858,  1005,
     782,   487,   485,   486,   132,     0,   339,   464,     0,   934,
     775,     0,   140,   292,     0,   582,     0,   167,     0,   205,
     471,     0,     0,     0,     0,     0,   159,   190,   161,  1123,
    1124,  1120,  1121,     0,  1127,  1113,     0,     0,     0,     0,
      78,    43,    65,    42,  1062,   197,   200,   163,   140,     0,
     180,   189,     0,     0,     0,     0,     0,     0,   117,     0,
       0,     0,   860,   115,    18,     0,   111,     0,   351,     0,
     165,     0,     0,   166,   254,   255,  1051,     0,     0,   487,
     485,   486,   489,   488,     0,  1103,   262,     0,  1003,     0,
       0,     0,     0,   859,   859,     0,     0,     0,     0,   168,
       0,     0,   796,  1030,   850,     0,     0,  1028,   855,  1027,
     129,     5,    13,    14,     0,   260,     0,     0,   588,     0,
       0,   859,     0,   770,     0,   769,   764,   589,     0,     0,
       0,     0,     0,   917,   136,     0,   861,   916,  1132,   463,
     476,   490,   950,   969,   147,   139,   143,   144,   145,   146,
     436,     0,   566,   853,   854,   127,   859,     0,  1071,     0,
       0,     0,     0,   861,   340,     0,     0,     0,   487,   209,
     210,   208,   485,   486,   205,   184,   182,   183,   185,   571,
     224,   258,     0,   986,     0,     0,   513,   515,   514,   526,
       0,     0,   546,   511,   512,   516,   518,   517,   535,   536,
     533,   534,   537,   538,   539,   540,   541,   527,   528,   520,
     521,   519,   522,   523,   525,   542,   524,     0,     0,  1020,
       0,   859,  1054,     0,  1053,  1070,   947,   240,   232,   246,
       0,  1055,   236,   222,     0,   477,   480,   482,   492,   495,
     496,   497,   498,   499,   500,   501,   502,   503,   504,   505,
     506,   921,     0,   920,   923,   946,   927,  1070,   924,     0,
       0,     0,     0,     0,     0,  1048,   475,   908,   912,   858,
     914,   462,   765,     0,  1009,     0,  1008,   258,     0,   765,
     993,   992,   978,   981,     0,     0,   920,   923,   991,   924,
     484,   294,   296,   136,   586,   585,   469,     0,   140,   276,
     151,   472,     0,     0,     0,     0,   205,   288,   288,   157,
     859,     0,     0,  1112,     0,  1109,   859,     0,  1083,     0,
       0,     0,     0,     0,   857,     0,    37,    38,    40,    41,
      39,     0,     0,     0,   799,   803,   804,   805,   808,   806,
     807,   810,     0,   798,   134,   848,   809,  1070,  1126,   205,
       0,     0,     0,    21,     0,    22,     0,    19,     0,   112,
       0,    20,     0,     0,     0,   123,   861,     0,   121,   116,
     113,   118,     0,   348,   356,   353,     0,     0,  1040,  1045,
    1042,  1041,  1044,  1043,    12,  1101,  1102,     0,   859,     0,
       0,     0,  1002,   999,     0,   599,     0,   613,   858,   601,
     858,   631,   616,   625,   628,   619,  1039,  1038,  1037,     0,
    1033,     0,  1034,  1036,   205,     5,     0,     0,     0,   657,
     658,   666,   665,     0,   485,     0,   858,   594,   598,     0,
     622,     0,  1056,     0,   576,     0,   205,  1090,   917,   320,
    1132,  1131,     0,     0,     0,   984,   858,  1073,  1069,   342,
     336,   337,   341,   343,   758,   860,   338,     0,     0,     0,
       0,   462,     0,     0,   490,     0,   951,   212,   205,   142,
     917,     0,     0,   260,   573,   226,   929,   930,   545,     0,
     639,   640,     0,   637,   858,  1015,     0,     0,   339,   262,
       0,   260,     0,     0,   258,     0,   994,   493,     0,     0,
     948,   949,   979,   980,     0,     0,     0,   896,   866,   867,
     868,   875,     0,    37,    38,    40,    41,    39,     0,     0,
       0,   881,   887,   888,   889,   892,   890,   891,     0,   879,
     877,   878,   902,   859,     0,   910,  1013,  1012,     0,   260,
       0,   935,     0,   781,     0,   298,     0,   205,   148,   205,
       0,   205,     0,     0,     0,     0,   268,   269,   280,     0,
     140,   278,   177,   288,     0,   288,     0,   858,     0,     0,
       0,     0,     0,   858,  1111,  1114,  1079,   859,     0,  1074,
       0,   859,   831,   832,   829,   830,   865,     0,   859,   857,
     606,   634,   634,   606,   634,   597,   634,     0,     0,  1022,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1117,   214,
       0,   217,   181,     0,     0,     0,   119,     0,   124,   125,
     117,   860,   122,     0,   352,     0,  1052,   169,  1068,  1103,
    1094,  1098,   261,   263,   362,     0,     0,  1000,     0,   604,
       0,  1032,     0,    17,   205,  1055,   259,   362,     0,     0,
     765,   591,     0,   771,  1057,     0,  1090,   580,   135,   137,
       0,     0,     0,  1132,     0,     0,   325,   323,   923,   936,
    1070,   923,   937,  1070,  1072,   987,     0,     0,     0,   344,
     133,   207,   209,   210,   486,   188,   206,   186,   187,   211,
     141,     0,   917,   257,     0,   917,     0,   544,  1019,  1018,
       0,   339,     0,     0,     0,     0,     0,     0,   260,   228,
     782,   922,   339,     0,   871,   872,   873,   874,   882,   883,
     900,     0,   859,     0,   896,   610,   636,   636,   610,   636,
       0,   870,   904,   636,     0,   858,   907,   909,   911,     0,
    1007,     0,   922,     0,     0,     0,   205,   295,   587,   153,
       0,   472,   268,   270,  1002,     0,     0,     0,   205,     0,
       0,     0,     0,     0,   282,     0,  1118,     0,     0,  1104,
       0,  1110,  1108,  1075,   858,  1081,     0,  1082,     0,     0,
     801,   858,   856,     0,     0,   859,     0,     0,   845,   859,
       0,     0,     0,     0,   859,     0,     0,   811,   846,   847,
    1026,     0,   859,   814,   816,   815,     0,     0,   812,   813,
     817,   819,   818,   835,   836,   833,   834,   837,   838,   839,
     840,   841,   826,   827,   821,   822,   820,   823,   824,   825,
     828,  1116,     0,   140,     0,     0,     0,     0,   120,    23,
     354,     0,     0,     0,  1095,  1100,     0,   436,  1004,  1002,
     478,   483,   491,     0,     0,    15,     0,   436,   669,     0,
       0,   671,   664,   667,     0,   662,     0,  1059,     0,     0,
       0,     0,   543,     0,   489,  1091,   584,  1132,     0,   326,
     327,     0,     0,   321,     0,     0,     0,   346,   347,   345,
    1090,     0,   362,     0,   917,     0,   339,     0,   976,   362,
    1055,   362,  1058,     0,     0,     0,   494,     0,     0,   885,
     858,   895,   876,     0,     0,   859,     0,     0,   894,   859,
       0,     0,     0,   869,     0,     0,   859,     0,   880,   901,
    1011,   362,     0,   140,     0,   291,   277,     0,     0,     0,
     267,   173,   281,     0,     0,   284,     0,   289,   290,   140,
     283,  1119,  1105,     0,     0,  1078,  1077,     0,     0,  1130,
     864,   863,   800,   614,   858,   605,     0,   617,   858,   633,
     626,   629,   620,     0,   858,   596,   802,   623,     0,   638,
     858,  1021,   843,     0,     0,   205,    24,    25,    26,    27,
    1097,  1092,  1093,  1096,   264,     0,     0,     0,   443,   434,
       0,     0,     0,   239,   361,   363,     0,   433,     0,     0,
       0,  1055,   436,     0,   603,  1035,   358,   245,   660,     0,
       0,   590,   578,   486,   138,   205,     0,     0,   330,   319,
       0,   322,   329,   339,   339,   335,   570,  1090,   436,  1090,
       0,  1017,     0,   975,   436,     0,   436,  1060,   362,   917,
     972,   899,   898,   884,   615,   858,   609,     0,   618,   858,
     635,   627,   630,   621,     0,   886,   858,   903,   624,   436,
     140,   205,   149,   154,   175,   271,   205,   279,   285,   140,
     287,     0,  1106,  1076,  1080,     0,     0,     0,   608,   844,
     593,     0,  1025,  1024,   842,   140,   218,  1099,     0,     0,
       0,  1063,     0,     0,     0,   265,     0,  1055,     0,   399,
     395,   401,   760,    36,     0,   389,     0,   394,   398,   411,
       0,   409,   414,     0,   413,     0,   412,     0,   222,   365,
       0,   367,     0,   368,   369,     0,     0,  1001,     0,   661,
     659,   670,   668,     0,   331,   332,     0,     0,   317,   328,
       0,     0,     0,  1090,  1084,   235,   570,  1090,   977,   241,
     358,   247,   436,     0,     0,     0,   612,   893,   906,     0,
     243,   293,   205,   205,   140,   274,   174,   286,  1107,  1129,
     862,     0,     0,     0,   205,     0,     0,   461,     0,  1064,
       0,   379,   383,   458,   459,   393,     0,     0,     0,   374,
     719,   720,   718,   721,   722,   739,   741,   740,   710,   682,
     680,   681,   700,   715,   716,   676,   687,   688,   690,   689,
     757,   709,   693,   691,   692,   694,   695,   696,   697,   698,
     699,   701,   702,   703,   704,   705,   706,   708,   707,   677,
     678,   679,   683,   684,   686,   756,   724,   725,   729,   730,
     731,   732,   733,   734,   717,   736,   726,   727,   728,   711,
     712,   713,   714,   737,   738,   742,   744,   743,   745,   746,
     723,   748,   747,   750,   752,   751,   685,   755,   753,   754,
     749,   735,   675,   406,   672,     0,   375,   427,   428,   426,
     419,     0,   420,   376,   453,     0,     0,     0,     0,   457,
       0,   222,   231,   357,     0,     0,     0,   318,   334,   973,
     974,     0,     0,     0,     0,  1090,  1084,     0,   237,  1090,
     897,     0,     0,   140,   272,   155,   176,   205,   607,   592,
    1023,   216,   377,   378,   456,   266,     0,   859,   859,     0,
     402,   390,     0,     0,     0,   408,   410,     0,     0,   415,
     422,   423,   421,     0,     0,   364,  1065,     0,     0,     0,
     460,     0,   359,     0,   333,     0,   655,   861,   136,   861,
    1086,     0,   429,   136,   225,     0,     0,   233,     0,   611,
     905,   205,     0,   178,   380,   126,     0,   381,   382,     0,
     858,     0,   858,   404,   400,   405,   673,   674,     0,   391,
     424,   425,   417,   418,   416,   454,   451,  1103,   370,   366,
     455,     0,   360,   656,   860,     0,   205,   860,  1085,     0,
    1089,   205,   136,   227,   229,     0,   275,     0,   220,     0,
     436,     0,   396,   403,   407,     0,     0,   917,   372,     0,
     653,   569,   572,  1087,  1088,   430,   205,   273,     0,     0,
     179,   387,     0,   435,   397,   452,  1066,     0,   861,   447,
     917,   654,   574,     0,   219,     0,     0,   386,  1090,   917,
     302,  1132,   450,   449,   448,  1132,   446,     0,     0,     0,
     385,  1084,   447,     0,     0,  1090,     0,   384,     0,  1132,
    1132,   308,     0,   307,   305,  1084,   140,   431,   136,   371,
       0,     0,   309,     0,     0,   303,     0,   205,   205,   313,
       0,   312,   301,     0,   304,   311,   373,   215,   432,   314,
       0,     0,   299,   310,     0,   300,   316,   315
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1707, -1707, -1707,  -579, -1707, -1707, -1707,    99,  -409,   -47,
     551, -1707,  -162,  -521, -1707, -1707,   482,   273,  1542, -1707,
    2747, -1707,  -822, -1707,  -542, -1707,  -702,    51, -1707, -1707,
   -1707, -1707, -1707, -1707, -1707, -1707,  -946, -1707, -1707,  -913,
    -367, -1707, -1707, -1707,  -347, -1707, -1707,  -191,   155,    38,
   -1707, -1707, -1707, -1707, -1707, -1707,    54, -1707, -1707, -1707,
   -1707, -1707, -1707,    55, -1707, -1707,  1171,  1187,  1192,  -119,
    -741,  -923,   637,   712,  -361,   356, -1023, -1707,   -66, -1707,
   -1707, -1707, -1707,  -781,   171, -1707, -1707, -1707, -1707,  -348,
   -1707,  -613, -1707,   434,  -423, -1707, -1707,  1066, -1707,   -42,
   -1707, -1707, -1069, -1707, -1707, -1707, -1707, -1707, -1707, -1707,
   -1707, -1707, -1707,   -79, -1707,    12, -1707, -1707, -1707, -1707,
   -1707,  -164, -1707,   120,  -958, -1707, -1259,  -377, -1707,  -152,
     140,  -118,  -351, -1707,  -163, -1707, -1707, -1707,   138,   -97,
     -71,   -16,  -779,   -56, -1707, -1707,    32, -1707,    49,  -360,
   -1707,    18,    -5,   -81,   -75,   -21, -1707, -1707, -1707, -1707,
   -1707, -1707, -1707, -1707, -1707,  -606,  -926, -1707, -1707, -1707,
   -1707, -1707,  1271,  1320, -1707,   584, -1707,   410, -1707, -1707,
   -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707, -1707,
   -1707, -1707, -1707, -1707, -1707, -1707, -1707,   404,  -329,  -530,
   -1707, -1707, -1707, -1707, -1707,   493, -1707, -1707, -1707, -1707,
   -1707, -1707, -1707, -1707,  -925,  -349,  2496,     7, -1707,   359,
    -411, -1707, -1707,  -499,  3680,  3600, -1707,   561, -1707, -1707,
     586,  -517,  -667, -1707, -1707,   667,   436,  -505, -1707,   438,
   -1707, -1707, -1707, -1707, -1707,   645, -1707, -1707, -1707,    21,
    -936,  -104,  -439,  -437, -1707,    -3,  -137, -1707, -1707,    35,
      41,   599,   -80, -1707, -1707,   527,   -68, -1707,  -340,    27,
    -354,   168,  -431, -1707, -1707,  -448,  1349, -1707, -1707, -1707,
   -1707, -1707,   810,   508, -1707, -1707, -1707,  -339,  -719, -1707,
    1301, -1301,  -195,   -58,  -112,   869, -1707, -1707, -1707, -1706,
   -1707,  -248, -1169, -1346,  -236,   182, -1707,   544,   621, -1707,
   -1707, -1707, -1707,   570, -1707,  2220,  -738
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   116,   975,   671,   192,   352,   785,
     372,   373,   374,   375,   926,   927,   928,   118,   119,   120,
     121,   122,   996,  1238,   432,  1028,   705,   706,   579,   268,
    1752,   585,  1656,  1753,  2008,   911,   124,   125,   726,   727,
     735,   365,   608,  1963,  1192,  1413,  2030,   455,   193,   707,
    1031,  1276,  1485,   128,   674,  1050,   708,   741,  1054,   646,
    1049,   247,   560,   709,   675,  1051,   457,   392,   414,   131,
    1033,   978,   951,  1212,  1684,  1336,  1116,  1905,  1756,   860,
    1122,   584,   869,  1124,  1529,   852,  1105,  1108,  1325,  2037,
    2038,   695,   696,  1012,   722,   723,   379,   380,   382,  1718,
    1883,  1884,  1427,  1584,  1707,  1877,  2017,  2040,  1916,  1967,
    1968,  1969,  1694,  1695,  1696,  1697,  1918,  1919,  1925,  1979,
    1700,  1701,  1705,  1870,  1871,  1872,  1954,  2079,  1585,  1586,
     194,   133,  2055,  2056,  1875,  1588,  1589,  1590,  1591,   134,
     135,   654,   581,   136,   137,   138,   139,   140,   141,   142,
     143,   261,   144,   145,   146,  1733,   147,  1030,  1275,   148,
     692,   693,   694,   265,   424,   575,   680,   681,  1374,   682,
    1375,   149,   150,   652,   653,  1364,  1365,  1494,  1495,   151,
     895,  1082,   152,   896,  1083,   153,   897,  1084,   154,   898,
    1085,   155,   899,  1086,   156,   900,  1087,   655,  1367,  1497,
     157,   901,   158,   159,  1947,   160,   676,  1720,   677,  1228,
     983,  1445,  1442,  1863,  1864,   161,   162,   163,   250,   164,
     251,   262,   436,   567,   165,  1368,  1369,   905,   906,   166,
    1147,   559,   623,  1148,  1090,  1298,  1091,  1498,  1499,  1301,
    1302,  1093,  1505,  1506,  1094,   828,   550,   206,   207,   710,
     698,   534,  1248,  1249,   816,   817,   465,   168,   253,   169,
     170,   196,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   744,   257,   258,   649,   241,   242,   780,   781,
    1381,  1382,   407,   408,   969,   182,   637,   183,   691,   184,
     355,  1885,  1937,   393,   444,   716,   717,  1137,  1138,  1894,
    1949,  1950,  1242,  1424,   947,  1425,   948,   949,   875,   876,
     877,   356,   357,   908,   594,  1001,  1002
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     195,   197,   542,   199,   200,   201,   202,   204,   205,   353,
     208,   209,   210,   211,   462,   515,   231,   232,   233,   234,
     235,   236,   237,   238,   240,   167,   259,   449,   266,   999,
     851,  1109,  1029,   830,   429,   535,   536,   249,   426,   267,
     415,   431,   127,   450,   451,   419,   420,   275,   514,   278,
    1240,   264,   363,   427,   366,   123,  1016,   256,   129,   130,
    1232,   837,  1243,   686,   269,   254,   909,  1448,   683,   273,
     401,   255,  1053,   458,   777,   778,   462,   685,   687,  1573,
     995,   267,  1112,   736,   737,   738,   994,  1126,   730,   361,
    1332,   775,   974,   823,   865,   814,  1099,   815,  1770,   784,
     925,   930,   936,   117,   428,  1261,   362,  1266,   821,   576,
    1274,   446,   -78,   -43,   819,   820,   429,   -78,   -43,   953,
     426,   569,   867,   431,    14,   -42,  1527,    14,  1285,   847,
     -42,   848,   576,   629,   564,    14,   959,   961,   632,  1245,
    1140,  1018,   576,   844,   132,  1710,  1712,   553,  -392,   377,
    1927,   552,   276,   568,   431,   351,  1778,    14,  1437,   126,
     953,   402,   609,  1240,   987,  1865,  1934,  -462,  1934,   381,
     562,   953,   391,   551,   561,   953,  1321,  1928,  1770,  1999,
     532,   533,  1607,   953,  1246,   660,   428,  1945,  1158,  1310,
    1956,    14,     3,   545,  1508,   413,  -954,   391,  2072,  1007,
    -641,  2090,   391,   391, -1070,   613,   615,   617,  2019,   620,
    -126,   842,  1341,  1342,  -126,   463,  -649,   428,   463,  -110,
    -109,   198,   945,   946,  1922,   443,   610,  1608,  1159,  1373,
     391,  -126,  1946,   532,   533,  -110,  -109,   662,   405,   406,
     428,   404,  1923,  2073, -1070,  1378,  2091,   973,  -763,   260,
    -953,   532,   533,  2020,   580,   571,  -995,  1311,   571,  1202,
     263,  1924,  1244,  -652,  1045,   267,   582,  -581,  1422,  1423,
     742,   378,  -860,   539,  -956,  1682,  -860,  1247,  -650,  -324,
     434,   532,   533,  -324,   539,   866,   593,   461,  -769,  -952,
    1771,  1772,   868,   937,  1239,  1528,  -649,  -959,   661,  -462,
     938,  1616,  -998,   447,   -78,   -43,   549,   516,  1344,  -306,
     954,   604,  1520,  1288,   640,   573,  1111,   -42,   639,   578,
    1609,   643,  1092,   980,   577,   630,  1270,  1450,  -954,  -963,
     633,  1573,  1019,   376,   659,  -955,  2074,  1711,  1713,  2092,
    -392,  1929,  1339,  -860,  1343,   538,   942,   464,  1779,   543,
     464,  1064,   824,  1128,   202,  2068,  -858,  1866,  1935,  1134,
    1989,   411,  1428,   117,   412,  1484,  1655,   117,   396,  2086,
    2067,   583,   638,   732,  1717,  -997,  -764,   728,   226,   226,
     431,  -770,  -953,   267,   428,  1773,  1504,  -762,  -995,  -771,
     240,   651,   267,   267,   651,   267,  1224,   740,   462,   353,
     665,  1198,  1199,  1618,  -938,   540,  -956,  1239,  -939,  1878,
    1624,  1879,  1626,   267,  1341,  1342,   540,  1271,  -962,   270,
     204,  -952,   271,   463,  -964,   538,  1048,   442,   711,  -959,
     272,  1210,  1443,  -925,  -998,  1598,   981,  1460,   213,    40,
     724,  -951,  1649,   731,   421,   697,  1930,   364,  1734,  -925,
    1736,   982,  -958,   415,   790,   791,   458,   603,   743,   745,
     397,   795,   672,   673,  -961,  1931,  1444,  -955,  1932,   746,
     747,   748,   749,   751,   752,   753,   754,   755,   756,   757,
     758,   759,   760,   761,   762,   763,   764,   765,   766,   767,
     768,   769,   770,   771,   772,   773,   774,   729,   776,   387,
     743,   743,   779,  1725,  1215,  1458,  1436,  -997,  2082,   249,
    1530,   402,   798,   799,   800,   801,   802,   803,   804,   805,
     806,   807,   808,   809,   810,  -649,   213,    40,  2099,   256,
    1461,   117,   724,   724,   743,   822,  -938,   254,   502,   798,
    -939,   715,   826,   255,   388,   351,   229,   229,   416,   796,
     503,   834,  1517,   836,   391,  1605,   422,   433,   515,  1742,
     112,   724,  1300,   423,  1892,  1251,   797,  1252,  1896,   855,
    -965,   856,   132,  -951,  -928,   625,  1316,  -926,  1338,  1187,
     784,  -968,   389,  -651,  -958,   383,   390,   126,   405,   406,
    -928,   514,   841,  -926,   384,  1003,  1726,  1004,   226,   394,
     443,  2083,  1055,   171,   984,  2000,   395,   603,   391,   788,
     391,   391,   391,   391,   532,   533,  1537,   385,   228,   230,
    1355,  2100,   932,   854,  1358,  1282,   402,   386,  1671,   398,
     859,  1362,   686,   813,   667,   416,  1456,   683,   626,   417,
     376,   376,   376,   618,   376,  1290,   685,   687,   112,   532,
     533,   452,  1265,   603,   538,  1267,  1268,   734,  1263,  1471,
    1263,   402,  1473,  1251,   402,  1252,   918,   846,   625,   435,
     793,   399,   667,  1047,   428,  -765,   786,  -966,   117,   402,
     925,    55,   670,   400,  1035,   918,   418,   403,  1982,   442,
     459,   186,   187,    65,    66,    67,   441,  2003,   907,  2004,
     445,  1101,   818,   405,   406,  1059,   448,  1983,   430,   402,
    1984,   453,  -966,   454,   697,   402,  1013,   438,   466,  1606,
    1749,   467,   786,   667,   931,   715,  1955,  1003,  1004,   919,
    1958,   442,   468,   843,  1100,  1102,   849,   226,   405,   406,
     469,   405,   406,   532,   533,  1038,   226,   470,   442,  1379,
    1193,   714,  1194,   226,  1195,   404,   405,   406,  1197,   968,
     970,  1625,  -642,   460,  -643,   226,   229,   471, -1070,   472,
     459,   186,   187,    65,    66,    67,   684,  1500,  1046,  1502,
    2051,   473,   564,  1507,  -644,  1491,   405,   406,   713,   443,
     430,  1486,   405,   406,  -647,  1188,  1438,  2069,   508,  1300,
    1496,   686,   656,  1496,   658, -1070,   683,  1058, -1070,  1439,
    1509,  -645,    34,    35,    36,   685,   687,  2052,  2053,  2054,
    -646,   430,   688,  1370,  1466,  1372,   214,  1376,   945,   946,
    1440,   402,   505,   117,  2052,  2053,  2054,   507,   555,   667,
    1104,   391,  1602,   460,   563,   580,  1010,  1011,  1545,   506,
     516,   537,  1549,  1340,  1341,  1342,   267,  1555,  1477,  1106,
    1107,  1620,  -960,   171,  -648,  1561,  -763,   171,   541,  1487,
     548,  1565,  1715,   409,   132,  1110,  1524,  1341,  1342,  2061,
    1263,    81,    82,    83,    84,    85,  1519,  1323,  1324,   126,
    1422,  1423,   221,   503,   929,   929,  2075,   546,    89,    90,
     226,  1678,  1679,  1029,   443,   229,   554,   668,   405,   406,
    -964,   663,    99,   557,   229,   669,   642,  1121,   612,   614,
     616,   229,   619,   437,   439,   440,  1081,   105,  1095,  1952,
    1953,  2077,  2078,   229,   459,   186,   187,    65,    66,    67,
     499,   500,   501,   663,   502,   669,   663,   669,   669,   558,
     117,  1219,   538,  1220,  -761,   856,   503,   565,   686,  1183,
    1184,  1185,   574,   683,  1119,   117,  1222,   566,  1774,  1980,
    1981,  1651,   685,   687,   587,  1186,   595,   628,  1636, -1115,
    1231,  1593,  1640,  1631,   598,  1632,   636,  1660,   641,  1647,
     599,   132,   605,   648,   606,   167,   622,  1289,  1976,  1977,
    1250,  1253,   621,   624,   631,   666,   126,   460,   117,   634,
    1259,   635,   127,   731,  2047,   731,  1743,  1196,   715,   697,
     798,   645,   644,   689,   690,   123,   699,   700,   129,   130,
     701,   171,   703,  -131,  1277,   712,    55,  1278,   734,  1279,
     739,   827,   829,   724,   662,   831,   832,   733,  1211,   132,
    1240,   697,   838,  1622,   839,  1240,  1180,  1181,  1182,  1183,
    1184,  1185,   857,   249,   126,   576,   861,  1262,   229,  1262,
     878,   864,   593,   117,   797,  1186,   879,   910,   226,   935,
    1240,  2039,   912,   256,   913,   736,   737,   738,   603,   730,
     914,   254,   915,  1320,   916,   117,   917,   255,   920,   921,
     939,   940,   813,   813,  2039,   943,   950,   955,  1751,   944,
     952,   958,   960,  2062,   132,   971,   957,  1757,   496,   497,
     498,   499,   500,   501,  1326,   502,  1996,   117,   976,   126,
    1745,  2001,  1746,  1764,  1747,   962,   132,   503,  1464,  1748,
     648,  1465,  1240,  1451,   459,    63,    64,    65,    66,    67,
     963,   126,   964,   226,   391,    72,   509,  1233,  1327,  1452,
    1453,   965,  1430,   977,  1297,  1297,  1081,  1681,   132,  -786,
     979,   818,   818,   985,   986,   988,   989,   990,   171,   997,
    2026,   993,   998,   126,  1006,   929,  1008,   929,  1015,   929,
    1730,  1731,   226,   929,   226,   929,   929,  1200,   511,   846,
     686,   846,  1017,  1014,  1020,   683,   117,  1032,   117,  1036,
     117,  1021,  1907,  1432,   685,   687,  1022,   460,  1023,  1024,
    1044,  1040,   226,  1052,  1041,  1043,  1060,  1061,  1062,  1034,
    1113,  1350,  -767,  1123,  1103,   731,  1127,  1125,  1131,  1900,
    1132,  1133,  1135,  1149,  1150,   167,   229,  1151,  1152,   132,
     743,   132,  1153,  1469,  1154,  1191,  2088,   603,  1155,  1156,
    1201,  1203,   127,  1205,   126,  1207,   126,  1431,   849,  1208,
     849,  1209,  1218,  1214,  1769,   123,   724,  1681,   129,   130,
    1995,  1221,  1998,  1227,  1229,  1234,   907,   724,  1432,  1262,
     224,   224,   686,   697,  1239,   226,   697,   683,  1230,  1239,
    1236,  1681,  1287,  1681,  1241,  1255,   685,   687,  1272,  1681,
    1281,   226,   226,  2063,  1284,   580,   535,  2064,  1009,  1292,
    -967,   229,  1293,   117,  1239,  1303,   267,  1304,  1312,  1305,
    1306,  2080,  2081,   171,  1307,  1308,  1526,  1309,  1313,  1315,
    1317,  1512,  1314,  1329,  1331,   684, -1133, -1133, -1133, -1133,
   -1133,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1334,
     229,  1961,   229,  1335,   132,  1337,  1347,  1346,  1348,  1354,
    1356,  2050,  1186, -1131,   429,  1357,  1186,  1515,   426,   126,
    1360,   431,  1361,  1412,  1414,  1426,  1239,  1459,  1089,  1415,
     229,  1419,  1416,  1057,  1417,  1429,  1959,  1960,  1462,  1446,
    1971,  1973,  1048,  1081,  1081,  1081,  1081,  1081,  1081,  1447,
    1463,  1474,  1081,  1470,  1081,  1476,  1472,  1478,  1488,  1489,
    1479,  1481,  1482,  1490,  1071,   117,  1511,  1503,  1594,  1513,
    1514,  1516,  1096,  1521,  1097,  1599,  1531,   117,  1525,  1600,
    1716,  1601,  1538,  1534,   731,  1539,  1542,  1533,  1592,  1544,
     171,  1543,  1547,  1548,  1550,  1553,  1554,  1612,  1592,  1551,
    1552,  1560,  1117,   229,  1566,   171,  1556,  1610,  1611,   462,
    1621,   724,   226,   226,   929,  1557,  1564,  1559,   132,   229,
     229,  1595,  1567,  1615,  1568,  1569,  1613,  1596,  1681,  1614,
    1604,  1617,  1619,   126,  1623,   697,  1627,  1629,   729,  1628,
    1634,   459,    63,    64,    65,    66,    67,  1635,   171,  1630,
     224,  1633,    72,   509,   684,  1637,  1638,  1876,  1639,  1641,
    1570,  1644,  1642,  1643,  1645,  1646,  1648,  1650,  1661,  1652,
    1657,  1653,  1664,  1654,  2087,  1206,  1658,  1686,   544,   518,
     519,   520,   521,   522,   523,   524,   525,   526,   527,   528,
     529,   648,  1217,   510,  1675,   511,  1714,  1699,  1719,  1724,
    1727,   223,   223,  1728,  1732,  1737,  1738,  1587,  1744,   512,
    1740,   513,   246,   171,   460,  1759,  1762,  1587,  1768,  1776,
    1777,  1880,  1873,   530,   531,  1874,  1886,  1081,  1887,  1081,
    1912,  1933,  1889,  1890,  1891,   171,  1893,  1901,   246,  1899,
    1723,  1902,  1913,  1939,  1942,  1729,  1943,  1948,   724,   724,
    1575,  1970,  1972,  1978,  1974,   226,  1986,  1264,  1987,  1264,
    1988,  1993,  1767,  1994,  1997,  2002,  2006,   171,  1089,  2007,
    -388,  2009,  2010,  2012,  2014,  1928,  2015,  2021,  2018,  1592,
     229,   229,  2027,  2041,  2029,  1592,  2034,  1592,  2045,   224,
     697,  2036,    14,  2028,  2049,  2048,  2058,  2060,   224,  2071,
     532,   533,  2084,  2076,   117,   224,  2065,  2085,  2066,  2089,
    1592,   684,  2093,  2094,  2101,   351,  2102,   224,   226,  2104,
    2105,  1704,  2044,  1418,  2059,   792,  1283,  1906,  1518,  1226,
    1941,  1468,  2057,   226,   226,   933,   787,  1659,  1897,  1921,
    1775,  1926,  1706,   789,   117,   132,   171,  1755,   171,  2096,
     171,  2070,  1117,  1333,  1938,  1895,  1687,  1576,  1501,   657,
     126,  1441,  1888,  1577,   702,   459,  1578,   187,    65,    66,
      67,  1579,   516,  1299,  1081,  1363,  1081,  1371,  1081,  1492,
    1318,  1708,  1493,  1081,   650,   132,  1991,   725,  1141,  2023,
     117,  2016,  1677,  1421,  1352,   117,     0,  1411,  1587,   117,
     126,     0,     0,  1592,  1587,     0,  1587,  1454,     0,     0,
       0,     0,     0,  1580,  1581,     0,  1582,     0,     0,   391,
       0,   223,   603,   229,     0,   351,     0,     0,   226,  1587,
       0,   132,     0,     0,     0,  1862,     0,     0,   460,     0,
     132,     0,  1869,  1904,  1755,     0,   126,  1583,     0,   351,
       0,   351,   224,     0,     0,   126,     0,   351,     0,     0,
       0,     0,     0,   171,     0,     0,     0,     0,     0,  1936,
       0,   246,     0,   246,     0,     0,     0,     0,     0,  1264,
       0,     0,     0,  1081,     0,  1575,   229,     0,     0,     0,
     117,   117,   117,     0,     0,  1467,   117,     0,  2032,     0,
       0,   229,   229,   117,     0,  1089,  1089,  1089,  1089,  1089,
    1089,  1881,     0,     0,  1089,     0,  1089,  1294,  1295,  1296,
     212,  1944,  1587,     0,     0,     0,     0,    14,     0,     0,
       0,   132,   246,     0,  1936,     0,     0,   132,     0,     0,
       0,     0,     0,    50,   132,     0,   126,     0,     0,     0,
       0,     0,   126,   684,     0,   462,     0,     0,  1510,   126,
     223,     0,     0,     0,     0,   171,     0,     0,     0,   223,
       0,     0,     0,   648,  1117,     0,   223,   171,     0,     0,
     216,   217,   218,   219,   220,     0,     0,     0,   223,     0,
       0,     0,  1576,     0,     0,     0,   229,     0,  1577,   223,
     459,  1578,   187,    65,    66,    67,  1579,     0,    93,    94,
       0,    95,   190,    97,     0,   603,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   246,     0,     0,   246,     0,
     224,     0,     0,   212,     0,     0,   351,   108,     0,     0,
    1081,  1081,     0,     0,     0,   684,   117,     0,  1580,  1581,
       0,  1582,     0,     0,     0,  1965,    50,     0,     0,     0,
       0,     0,  1862,  1862,     0,     0,  1869,  1869,   648,   212,
       0,     0,     0,   460,     0,     0,     0,     0,   697,     0,
     603,     0,  1597,     0,   246,     0,     0,   132,  1603,  1089,
       0,  1089,    50,   216,   217,   218,   219,   220,     0,     0,
     117,   697,   126,     0,     0,   224,     0,     0,     0,     0,
     697,     0,     0,     0,     0,   189,     0,     0,    91,  2095,
       0,    93,    94,   223,    95,   190,    97,     0,  2103,   216,
     217,   218,   219,   220,     0,   117,  2106,     0,     0,  2107,
     117,   132,     0,     0,   224,     0,   224,     0,  2031,     0,
     108,     0,     0,     0,   370,  1964,   126,    93,    94,     0,
      95,   190,    97,     0,     0,   117,     0,     0,     0,     0,
       0,  2046,     0,     0,   224,   246,   132,   246,     0,     0,
     894,   132,     0,     0,     0,     0,   108,  1575,     0,     0,
    2033,   126,     0,     0,     0,     0,   126,     0,     0,     0,
       0,     0,     0,     0,   171,     0,   132,  1575,     0,     0,
       0,     0,     0,   894,     0,     0,     0,     0,     0,     0,
       0,   126,     0,  1575,     0,     0,   117,   117,     0,    14,
       0,     0,     0,     0,     0,     0,  1089,     0,  1089,     0,
    1089,     0,  1575,     0,   171,  1089,     0,   224,     0,    14,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   224,   224,    14,     0,   132,   132,     0,
       0,   246,   246,     0,     0,     0,     0,     0,     0,     0,
     246,     0,   126,   126,    14,     0,     0,     0,     0,     0,
     171,     0,     0,     0,  1576,   171,     0,     0,     0,   171,
    1577,   223,   459,  1578,   187,    65,    66,    67,  1579,     0,
       0,     0,     0,     0,  1576,     0,   358,     0,     0,     0,
    1577,     0,   459,  1578,   187,    65,    66,    67,  1579,     0,
    1576,     0,     0,     0,     0,     0,  1577,     0,   459,  1578,
     187,    65,    66,    67,  1579,  1089,     0,     0,     0,  1576,
    1580,  1581,     0,  1582,     0,  1577,     0,   459,  1578,   187,
      65,    66,    67,  1579,     0,  1575,     0,     0,     0,     0,
    1580,  1581,     0,  1582,     0,   460,   223,     0,     0,     0,
       0,     0,     0,     0,  1735,     0,  1580,  1581,     0,  1582,
     171,   171,   171,     0,     0,   460,   171,     0,   871,     0,
       0,     0,     0,   171,  1739,  1580,  1581,    14,  1582,   246,
       0,   460,     0,     0,     0,   223,     0,   223,     0,     0,
    1741,     0,     0,     0,   224,   224,     0,     0,     0,     0,
     460,     0,     0,   212,     0,   213,    40,     0,     0,  1750,
       0,     0,     0,     0,     0,   223,   894,     0,   212,     0,
       0,     0,     0,   246,     0,     0,    50,     0,     0,   872,
     246,   246,   894,   894,   894,   894,   894,     0,     0,     0,
       0,    50,  1576,     0,     0,   894,     0,     0,  1577,     0,
     459,  1578,   187,    65,    66,    67,  1579,     0,     0,     0,
       0,     0,   246,   216,   217,   218,   219,   220,     0,     0,
       0,     0,  1089,  1089,     0,     0,     0,     0,   216,   217,
     218,   219,   220,     0,     0,     0,     0,     0,   223,   811,
       0,    93,    94,     0,    95,   190,    97,     0,  1580,  1581,
     189,  1582,   246,    91,   223,   223,    93,    94,     0,    95,
     190,    97,     0,   873,     0,     0,   171,     0,     0,   591,
     108,   592,     0,   460,   812,   225,   225,   112,   246,   246,
       0,     0,  1898,     0,     0,   108,   248,   224,   223,     0,
       0,     0,     0,     0,     0,   246,     0,     0,     0,     0,
       0,     0,   246,     0,     0,     0,     0,     0,   246,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   894,
     171,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     597,     0,     0,     0,   246, -1133, -1133, -1133, -1133, -1133,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
     224,     0,     0,     0,   246,   171,     0,     0,   246,     0,
     171,   503,  1160,  1161,  1162,   224,   224,     0,     0,   246,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1163,     0,   171,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,     0,     0,
       0,     0,     0,     0,     0,   223,   223,     0,     0,     0,
       0,  1186,     0,   718,     0,     0,   358,     0,     0,   246,
       0,     0,     0,   246,     0,   246,     0,     0,   246,     0,
       0,     0,     0,     0,     0,     0,   171,   171,     0,     0,
       0,   894,   894,   894,   894,   894,   894,   223,   894,     0,
     224,   894,   894,   894,   894,   894,   894,   894,   894,   894,
     894,   894,   894,   894,   894,   894,   894,   894,   894,   894,
     894,   894,   894,   894,   894,   894,   894,   894,   894,   474,
     475,   476,     0,     0,     0,   225,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   894,     0,   477,
     478,     0,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,   212,     0,     0,
       0,     0,     0,   246,     0,   246,     0,     0,   503,     0,
       0,     0,  1377,     0,     0,     0,     0,     0,   223,     0,
      50,     0,     0,   354,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   870,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   246,     0,     0,   246,
       0,     0,     0,     0,     0,     0,     0,   216,   217,   218,
     219,   220,     0,     0,     0,     0,   246,   246,   246,   246,
     246,   246,     0,     0,   223,   246,     0,   246,     0,     0,
       0,   223,   409,     0,     0,    93,    94,     0,    95,   190,
      97,     0,     0,     0,   225,     0,   223,   223,     0,   894,
       0,     0,     0,   225,     0,     0,     0,     0,     0,   246,
     225,     0,     0,     0,   108,     0,   246,     0,   410,     0,
       0,   894,   225,   894,     0,     0,     0,     0,     0,   991,
     992,     0,     0,   225,     0,   474,   475,   476,     0,     0,
       0,     0,     0,  1434,     0,     0,     0,     0,   894,     0,
       0,     0,     0,     0,     0,   477,   478,     0,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,     0,   246,   246,     0,     0,   246,     0,
       0,   223,     0,     0,   503,     0,     0,     0,     0,     0,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   248,   246,
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,     0,     0,
     246,     0,   246,     0,     0,     0,   354,   225,   354,   503,
       0,     0,     0,     0,     0,     0,   544,   518,   519,   520,
     521,   522,   523,   524,   525,   526,   527,   528,   529,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,   246,   246,     0,     0,
     246,     0,     0,     0,     0,     0,   894,     0,   894,  1186,
     894,   530,   531,     0,   902,   894,   223,   354,  1139,   718,
     894,     0,   894,     0,     0,   894,   504,     0,  1037,     0,
       0,     0,     0,   474,   475,   476,     0,     0,   246,   246,
       0,     0,   246,     0,     0,     0,     0,   902,     0,   246,
       0,     0,     0,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,     0,     0,     0,     0,     0,     0,     0,   532,   533,
       0,     0,   503,   972,     0,     0,     0,   246,     0,   246,
       0,   246,     0,     0,     0,     0,   246,     0,   223,     0,
     354,     0,     0,   354,     0,     0,  1225,     0,     0,     0,
       0,     0,     0,   246,   212,     0,     0,     0,   894,     0,
       0,     0,     0,  1235,     0,   225,     0,     0,     0,     0,
     246,   246,     0,     0,     0,     0,  1254,    50,   246,     0,
     246,     0,   840,     0,     0,   544,   518,   519,   520,   521,
     522,   523,   524,   525,   526,   527,   528,   529,     0,     0,
       0,     0,   246,  1702,   246,     0,     0,     0,     0,  1688,
     246,     0,     0,     0,   216,   217,   218,   219,   220,     0,
       0,     0,  1286,     0,     0,     0,     0,     0,     0,     0,
     530,   531,     0,     0,     0,     0,   246,     0,     0,     0,
     225,     0,    93,    94,     0,    95,   190,    97,     0,     0,
       0,     0,     0,   894,   894,   894,  1005,     0,     0,   212,
     894,     0,   246,     0,     0,     0,     0,     0,   246,     0,
     246,   108,  1703,  1088,     0,     0,     0,     0,     0,   225,
       0,   225,    50,     0,     0,     0,     0,     0,     0,     0,
     354,     0,   874,     0,     0,     0,     0,  1345,     0,     0,
       0,  1349,     0,     0,     0,  1689,  1353,   532,   533,   225,
     902,     0,     0,     0,     0,     0,     0,     0,  1690,   216,
     217,   218,   219,   220,  1691,     0,   902,   902,   902,   902,
     902,     0,     0,     0,     0,     0,     0,     0,     0,   902,
       0,   189,     0,     0,    91,  1692,     0,    93,    94,     0,
      95,  1693,    97,     0,     0,   871,  1190,   212,  1025,   518,
     519,   520,   521,   522,   523,   524,   525,   526,   527,   528,
     529,   941,   246,     0,     0,     0,   108,     0,     0,     0,
      50,     0,   225,     0,     0,   246,   354,   354,     0,   246,
       0,     0,     0,   246,   246,   354,  1213,     0,   225,   225,
       0,     0,     0,   530,   531,   212,     0,     0,   246,     0,
       0,  1455,     0,     0,   894,     0,   872,   216,   217,   218,
     219,   220,     0,  1213,     0,   894,     0,     0,    50,     0,
       0,   894,   225,     0,     0,   894,     0,     0,     0,     0,
       0,     0,     0,  1867,     0,    93,    94,  1868,    95,   190,
      97,     0,     0,     0,  1480,     0,     0,  1483,     0,     0,
       0,     0,   246,   902,     0,   216,   217,   218,   219,   220,
       0,     0,     0,     0,   108,  1703,     0,     0,  1273,     0,
     532,   533,     0,     0,     0,     0,     0,   189,     0,   246,
      91,   246,     0,    93,    94,     0,    95,   190,    97,     0,
    1351,     0,   248,     0,     0,     0,     0,   894,     0,     0,
       0,     0,     0,  1088,     0,     0,     0,  1532,     0,     0,
     246,     0,   108,     0,  1536,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   246,     0,     0,
       0,     0,     0,   246,  1026,     0,     0,   246,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   225,
     225,   246,   246,     0,     0,     0,     0,     0,  1130,     0,
       0,     0,     0,     0,     0,   354,   354,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1571,  1572,     0,   902,   902,   902,   902,   902,
     902,   225,   902,     0,     0,   902,   902,   902,   902,   902,
     902,   902,   902,   902,   902,   902,   902,   902,   902,   902,
     902,   902,   902,   902,   902,   902,   902,   902,   902,   902,
     902,   902,   902,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   227,
     227,   902,     0,     0,     0,     0,     0,     0,     0,     0,
     252,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   354,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   474,   475,   476,     0,
     354,     0,     0,     0,     0,     0,     0,   354,     0,     0,
       0,     0,   225,   354,  1662,  1663,   477,   478,  1665,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   503,  1683,     0,     0,   354,
    1088,  1088,  1088,  1088,  1088,  1088,     0,  1709,   225,  1088,
       0,  1088,     0,     0,     0,   225,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     225,   225,     0,   902,     0,     0,     0,     0,     0,     0,
       0,     0,  1025,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   902,     0,   902,     0,   517,
     518,   519,   520,   521,   522,   523,   524,   525,   526,   527,
     528,   529,     0,     0,   354,     0,     0,     0,   354,     0,
     874,  1758,   902,   354,     0,     0,     0,   530,   531,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   530,   531,  1683,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   474,   475,   476,   227,
       0,     0,  1574,     0,     0,   225,     0,     0,     0,  1037,
    1683,     0,  1683,     0,     0,     0,   477,   478,  1683,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,   532,   533,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   503,     0,     0,   474,   475,
     476,   532,   533,     0,  1088,     0,  1088,     0,   354,     0,
     354,     0,     0,     0,     0,     0,  1917,     0,   477,   478,
       0,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,     0,   702,     0,
       0,   354,     0,     0,   354,  1065,  1066,   503,     0,     0,
     902,     0,   902,     0,   902,     0,     0,     0,     0,   902,
     225,     0,     0,     0,   902,  1067,   902,     0,   227,   902,
       0,     0,     0,  1068,  1069,  1070,   212,   227,     0,     0,
       0,     0,     0,  1685,   227,     0,  1698,  1071,     0,     0,
       0,     0,     0,     0,     0,     0,   227,     0,     0,    50,
       0,     0,     0,     0,   354,     0,     0,   252,     0,     0,
    1940,   354,     0,     0,     0,     0,     0,     0,     0,  1063,
       0,     0,     0,  1951,     0,     0,     0,  1683,     0,     0,
       0,     0,     0,     0,     0,  1072,  1073,  1074,  1075,  1076,
    1077,  1088,     0,  1088,     0,  1088,     0,     0,     0,     0,
    1088,     0,   225,  1078,     0,   212,     0,     0,   189,     0,
       0,    91,    92,     0,    93,    94,     0,    95,   190,    97,
       0,     0,   902,   212,     0,   213,    40,     0,    50,   354,
     354,  1204,  1079,  1080,  1765,  1766,     0,     0,     0,     0,
       0,     0,   252,   108,  1698,     0,    50,     0,     0,     0,
    2011,     0,     0,     0,     0,     0,     0,     0,   904,     0,
       0,     0,     0,     0,   354,   216,   217,   218,   219,   220,
       0,     0,     0,     0,     0,     0,     0,  1951,     0,  2024,
       0,   227,     0,   216,   217,   218,   219,   220,     0,     0,
       0,   934,     0,    93,    94,     0,    95,   190,    97,     0,
    1088,     0,     0,     0,     0,     0,     0,     0,     0,   811,
       0,    93,    94,   289,    95,   190,    97,   902,   902,   902,
       0,     0,   108,   739,   902,     0,  1915,     0,     0,     0,
       0,     0,     0,     0,  1698,     0,     0,     0,   903,     0,
     108,   354,   354,     0,   845,   354,     0,   112,     0,     0,
     291,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   212,     0,     0,     0,     0,     0,  1000,
       0,   903,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,   354,     0,     0,    50,     0,     0,     0,
       0,     0,   477,   478,   354,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
       0,     0,   589,   216,   217,   218,   219,   220,   590,     0,
       0,   503,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   189,     0,     0,    91,   344,
       0,    93,    94,     0,    95,   190,    97,  1088,  1088,   227,
       0,     0,     0,     0,     0,     0,     0,     0,   354,   348,
       0,     0,     0,     0,     0,   212,     0,     0,   902,     0,
     108,   350,     0,     0,     0,     0,     0,     0,     0,   902,
       0,     0,     0,   354,     0,   902,     0,     0,    50,   902,
       0,     0,     0,   544,   518,   519,   520,   521,   522,   523,
     524,   525,   526,   527,   528,   529,     0,   354,     0,   354,
       0,     0,     0,     0,  1118,   354,     0,     0,     0,     0,
       0,     0,     0,     0,   227,   216,   217,   218,   219,   220,
    1142,  1143,  1144,  1145,  1146,     0,     0,     0,   530,   531,
       0,     0,     0,  1157,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    93,    94,  1280,    95,   190,    97,     0,
       0,   902,     0,   227,     0,   227,     0,     0,     0,     0,
       0,     0,     0,   354,  2043,     0,     0,     0,     0,     0,
       0,     0,   108,  1034,     0,     0,     0,     0,     0,     0,
       0,  1685,     0,   227,   903,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   474,   475,   476,     0,     0,     0,
     903,   903,   903,   903,   903,   532,   533,     0,     0,     0,
       0,     0,     0,   903,   477,   478,     0,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,   227,  1260,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   354,     0,     0,
       0,     0,   227,   227,     0,     0,     0,     0,     0,   212,
     354,     0,     0,     0,   354,     0,     0,   474,   475,   476,
    1129,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,  1966,     0,     0,   252,   477,   478,  1527,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,     0,     0,   903,     0,   216,
     217,   218,   219,   220,     0,     0,   503,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   354,     0,     0,
       0,   189,     0,     0,    91,     0,     0,    93,    94,     0,
      95,   190,    97,     0,     0,     0,   252,     0,     0,     0,
       0,     0,     0,     0,   354,     0,   354,  1291,     0,  1146,
    1366,     0,     0,  1366,     0,     0,   108,     0,     0,  1380,
    1383,  1384,  1385,  1387,  1388,  1389,  1390,  1391,  1392,  1393,
    1394,  1395,  1396,  1397,  1398,  1399,  1400,  1401,  1402,  1403,
    1404,  1405,  1406,  1407,  1408,  1409,  1410,     0,     0,     0,
       0,     0,     0,   227,   227,     0,     0,     0,   354,     0,
       0,     0,   354,     0,     0,  1420,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   354,   354,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   903,
     903,   903,   903,   903,   903,   252,   903,     0,  1528,   903,
     903,   903,   903,   903,   903,   903,   903,   903,   903,   903,
     903,   903,   903,   903,   903,   903,   903,   903,   903,   903,
     903,   903,   903,   903,   903,   903,   903,   474,   475,   476,
       0,     0,  1025,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   903,     0,   477,   478,     0,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,     0,   212,   530,   531,     0,
       0,     0,     0,     0,     0,     0,   503,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   227,  1522,     0,    50,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,  1540,
     502,  1541,  1689,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   503,     0,     0,  1690,   216,   217,   218,   219,
     220,  1691,     0,     0,     0,     0,  1562,     0,     0,     0,
       0,     0,   252,     0,   532,   533,     0,     0,   189,   227,
       0,    91,    92,     0,    93,    94,     0,    95,  1693,    97,
       0,     0,     0,     0,   227,   227,     0,   903,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   108,     0,     0,     0,     0,     0,   903,
       0,   903,     0,     0,   474,   475,   476,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1322,     0,     0,     0,   477,   478,   903,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,     0,     0,     0,   227,
       0,     0,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
       0,   474,   475,   476,  1667,     0,  1668,     0,  1669,     0,
       0,   503,     0,  1670,     0,     0,     0,     0,  1672,     0,
    1673,   477,   478,  1674,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     503,     0,   289,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,     0,   903,     0,   903,     0,   903,     0,
       0,     0,     0,   903,   252,     0,  1186,  1721,   903,   291,
     903,     0,     0,   903,     0,     0,     0,     0,     0,     0,
       0,     0,   212,     0,     0,     0,     0,     0,  1136,     0,
       0,     0,     0,   474,   475,   476,  1760,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,   477,   478,  1722,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,   589,   216,   217,   218,   219,   220,   590,     0,     0,
       0,     0,   503,     0,     0,     0,   252,     0,     0,     0,
       0,     0,   504,     0,   189,     0,     0,    91,   344,     0,
      93,    94,     0,    95,   190,    97,   903, -1132,     0,     0,
       5,     6,     7,     8,     9,     0,     0,     0,   348,     0,
      10,  1908,  1909,  1910,     0,     0,     0,     0,  1914,   108,
     350,     0,     0,     0,   359,   425,    13,     0,     0,     0,
       0,     0,     0,     0,     0,   794,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,     0,    43,
       0,   903,   903,   903,     0,     0,     0,     0,   903,     0,
       0,    50,     0,     0,   586,     0,     0,  1920,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   185,   186,
     187,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   188,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     189,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     190,    97,  1975,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,  1985,   101,   102,     0,   212,     0,  1990,
     105,   106,   107,  1992,     0,   108,   109,     0,   474,   475,
     476,     0,   112,   113,   114,   115,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,   477,   478,
       0,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,   216,   217,   218,
     219,   220,   903,     0,     0,     0,     0,   503,     5,     6,
       7,     8,     9,   903,     0,  2035,     0,     0,    10,   903,
       0,     0,   924,   903,     0,    93,    94,     0,    95,   190,
      97,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  2013,    14,   108,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,   903,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,    56,    57,
      58,     0,    59,  -205,    60,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,    71,    72,    73,   588,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,    88,    89,
      90,    91,    92,     0,    93,    94,     0,    95,    96,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,   103,     0,   104,     0,   105,   106,
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
      52,    53,    54,    55,    56,    57,    58,     0,    59,     0,
      60,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,    88,    89,    90,    91,    92,     0,
      93,    94,     0,    95,    96,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
     103,     0,   104,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,  1223,     0,   112,   113,   114,   115,
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
    1435,     0,   112,   113,   114,   115,     5,     6,     7,     8,
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
       0,   108,   109,     0,   110,   111,   704,     0,   112,   113,
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
     110,   111,  1027,     0,   112,   113,   114,   115,     5,     6,
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
     109,     0,   110,   111,  1189,     0,   112,   113,   114,   115,
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
    1237,     0,   112,   113,   114,   115,     5,     6,     7,     8,
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
       0,   108,   109,     0,   110,   111,  1269,     0,   112,   113,
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
     110,   111,  1328,     0,   112,   113,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,  1330,    47,     0,    48,     0,    49,     0,     0,    50,
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
      48,     0,    49,  1523,     0,    50,    51,     0,     0,     0,
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
    1676,     0,   112,   113,   114,   115,     5,     6,     7,     8,
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
     110,   111,  1911,     0,   112,   113,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,  1962,    49,     0,     0,    50,
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
       0,     0,     0,    43,    44,    45,    46,     0,    47,  2005,
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
    2022,     0,   112,   113,   114,   115,     5,     6,     7,     8,
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
       0,   108,   109,     0,   110,   111,  2025,     0,   112,   113,
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
     110,   111,  2042,     0,   112,   113,   114,   115,     5,     6,
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
     107,     0,     0,   108,   109,     0,   110,   111,  2097,     0,
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
     109,     0,   110,   111,  2098,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,   572,
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
      11,    12,    13,     0,     0,   858,     0,     0,     0,     0,
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
       0,  1120,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,    11,    12,    13,     0,     0,  1754,     0,     0,
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
      13,     0,     0,  1903,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
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
     359,     0,    13,     0,     0,     0,     0,     0,     0,     0,
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
       0,   108,   191,     0,   360,     0,     0,     0,   112,   113,
     114,   115,     0,     0,     0,     0,     0,     0,     0,     0,
     719,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,   720,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   185,   186,   187,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   188,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   189,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   190,    97,     0,   721,     0,
      99,     0,     0,   100,     5,     6,     7,     8,     9,   101,
     102,     0,     0,     0,    10,   105,   106,   107,     0,     0,
     108,   191,     0,     0,     0,     0,     0,   112,   113,   114,
     115,     0,     0,     0,     0,     0,     0,     0,     0,  1256,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,  1257,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,   185,   186,   187,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     188,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   189,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   190,    97,     0,  1258,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     191,     5,     6,     7,     8,     9,   112,   113,   114,   115,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   359,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   191,     0,     0,
     853,     0,     0,   112,   113,   114,   115,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   359,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   794,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,     0,     0,     0,   105,   106,   107,
       0,     0,   108,   191,     5,     6,     7,     8,     9,   112,
     113,   114,   115,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   359,   425,
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
     109,     0,     0,     0,     0,     0,   112,   113,   114,   115,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,   203,
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
       0,     0,     0,     0,     0,     0,     0,   239,     0,     0,
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
      10,   105,   106,   107,     0,     0,   108,   191,     0,   274,
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
     190,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   191,     0,   277,     0,
       0,     0,   112,   113,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   425,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,   185,   186,   187,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   188,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   189,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   190,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   191,   570,     0,     0,     0,     0,   112,   113,   114,
     115,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   359,     0,     0,     0,     0,
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
       0,     0,     0,   112,   113,   114,   115,     0,     0,   750,
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
       0,     0,   112,   113,   114,   115,     0,     0,     0,     0,
       0,     0,     0,     0,   794,     0,     0,     0,     0,     0,
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
       0,     0,     0,   833,     0,     0,     0,     0,     0,     0,
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
       0,     0,   835,     0,     0,     0,     0,     0,     0,     0,
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
       0,  1319,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,   359,     0,     0,
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
       0,     0,    10,   105,   106,   107,     0,     0,   108,  1449,
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
     185,   186,   187,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   188,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   189,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   190,    97,     0,     0,     0,    99,     0,     0,
     100,     5,     6,     7,     8,     9,   101,   102,     0,     0,
       0,    10,   105,   106,   107,     0,     0,   108,   191,     0,
       0,     0,     0,     0,   112,   113,   114,   115,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
     664,    39,    40,     0,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   185,
     186,   187,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   188,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   189,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   190,    97,     0,   279,   280,    99,   281,   282,   100,
       0,   283,   284,   285,   286,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   191,     0,   287,
     288,     0,     0,   112,   113,   114,   115,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,   290,   502,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   503,   292,   293,   294,   295,   296,   297,   298,     0,
       0,     0,   212,     0,   213,    40,     0,     0,   299,     0,
       0,     0,     0,     0,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,    50,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     212,   335,     0,   782,   337,   338,   339,     0,     0,     0,
     340,   600,   216,   217,   218,   219,   220,   601,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,   279,   280,
       0,   281,   282,     0,   602,   283,   284,   285,   286,     0,
      93,    94,     0,    95,   190,    97,   345,     0,   346,     0,
       0,   347,     0,   287,   288,     0,     0,     0,     0,   349,
     216,   217,   218,   219,   220,     0,     0,     0,     0,   108,
       0,     0,     0,   783,     0,     0,   112,     0,     0,     0,
       0,     0,   290,     0,     0,     0,   456,     0,    93,    94,
       0,    95,   190,    97,     0,     0,   292,   293,   294,   295,
     296,   297,   298,     0,     0,     0,   212,     0,   213,    40,
       0,     0,   299,     0,     0,     0,     0,   108,   300,   301,
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
       0,     0,  1882,     0,     0,     0,   287,   288,     0,   289,
       0,     0,   216,   217,   218,   219,   220,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   189,   290,     0,    91,    92,     0,
      93,    94,     0,    95,   190,    97,   291,     0,     0,   292,
     293,   294,   295,   296,   297,   298,     0,     0,     0,   212,
       0,     0,     0,     0,     0,   299,     0,     0,     0,   108,
       0,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,    50,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,     0,   335,     0,
       0,   337,   338,   339,     0,     0,     0,   340,   341,   216,
     217,   218,   219,   220,   342,     0,     0,     0,     0,     0,
       0,   212,     0,   966,     0,   967,     0,     0,     0,     0,
       0,   343,     0,     0,    91,   344,     0,    93,    94,     0,
      95,   190,    97,   345,    50,   346,     0,     0,   347,     0,
     279,   280,     0,   281,   282,   348,   349,   283,   284,   285,
     286,     0,     0,     0,     0,     0,   108,   350,     0,     0,
       0,  1957,     0,     0,     0,   287,   288,     0,   289,     0,
       0,   216,   217,   218,   219,   220,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,   290,   502,     0,     0,     0,    93,
      94,     0,    95,   190,    97,   291,     0,   503,   292,   293,
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
     216,   217,   218,   219,   220,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,   290,     0,     0,     0,     0,    93,    94,
       0,    95,   190,    97,   291,     0,  1186,   292,   293,   294,
     295,   296,   297,   298,     0,     0,     0,   212,     0,     0,
       0,     0,     0,   299,     0,     0,     0,   108,     0,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
      50,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,     0,   335,     0,     0,   337,
     338,   339,     0,     0,     0,   340,   341,   216,   217,   218,
     219,   220,   342,     0,     0,     0,     0,     0,     0,   212,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   343,
       0,     0,    91,   344,     0,    93,    94,     0,    95,   190,
      97,   345,    50,   346,     0,     0,   347,     0,     0,     0,
     922,   923,     0,   348,   349,  1680,     0,     0,     0,   279,
     280,     0,   281,   282,   108,   350,   283,   284,   285,   286,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   216,
     217,   218,   219,   220,   287,   288,     0,   289,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   924,     0,     0,    93,    94,     0,
      95,   190,    97,   290,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   291,     0,     0,   292,   293,   294,
     295,   296,   297,   298,     0,     0,   108,   212,     0,     0,
       0,     0,     0,   299,     0,     0,     0,     0,     0,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
      50,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,     0,   335,     0,     0,   337,
     338,   339,     0,     0,     0,   340,   341,   216,   217,   218,
     219,   220,   342,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   343,
       0,     0,    91,   344,     0,    93,    94,     0,    95,   190,
      97,   345,     0,   346,     0,     0,   347,     0,  1780,  1781,
    1782,  1783,  1784,   348,   349,  1785,  1786,  1787,  1788,     0,
       0,     0,     0,     0,   108,   350,     0,     0,     0,     0,
       0,     0,  1789,  1790,  1791,     0,   477,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,  1792,   502,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   503,  1793,  1794,  1795,  1796,
    1797,  1798,  1799,     0,     0,     0,   212,     0,     0,     0,
       0,     0,  1800,     0,     0,     0,     0,     0,  1801,  1802,
    1803,  1804,  1805,  1806,  1807,  1808,  1809,  1810,  1811,    50,
    1812,  1813,  1814,  1815,  1816,  1817,  1818,  1819,  1820,  1821,
    1822,  1823,  1824,  1825,  1826,  1827,  1828,  1829,  1830,  1831,
    1832,  1833,  1834,  1835,  1836,  1837,  1838,  1839,  1840,  1841,
    1842,     0,     0,     0,  1843,  1844,   216,   217,   218,   219,
     220,     0,  1845,  1846,  1847,  1848,  1849,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1850,  1851,
    1852,     0,     0,     0,    93,    94,     0,    95,   190,    97,
    1853,     0,  1854,  1855,     0,  1856,     0,     0,     0,     0,
       0,     0,  1857,     0,  1858,     0,  1859,     0,  1860,  1861,
       0,   279,   280,   108,   281,   282,  1162,     0,   283,   284,
     285,   286,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1163,   287,   288,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
       0,     0,     0,     0,     0,   290,     0,     0,     0,     0,
       0,     0,     0,  1186,     0,     0,     0,     0,     0,   292,
     293,   294,   295,   296,   297,   298,     0,     0,     0,   212,
       0,     0,     0,     0,     0,   299,     0,     0,     0,     0,
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
     334,     0,   335,     0,  1378,   337,   338,   339,     0,     0,
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
     502,     0,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,   503,   289,     0,     0,     0,     0,     0,     0,
       0,     0,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,   607,   502,
     291,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,   212,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,   289,   502,    50,     0,     0,     0,
       0,     0,     0,     0,  -435,     0,   611,   503,     0,     0,
       0,     0,     0,   459,   186,   187,    65,    66,    67,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   291,   589,   216,   217,   218,   219,   220,   590,     0,
       0,     0,     0,   289,   212,     0,     0,     0,     0,     0,
    1535,     0,     0,   825,     0,   189,     0,     0,    91,   344,
       0,    93,    94,     0,    95,   190,    97,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   348,
     291,     0,     0,     0,     0,     0,   460,     0,     0,     0,
     108,   350,     0,   212,   289,     0,     0,     0,     0,     0,
       0,     0,   850,   589,   216,   217,   218,   219,   220,   590,
     289,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,   596,     0,   189,     0,     0,    91,
     344,   291,    93,    94,     0,    95,   190,    97,     0, -1132,
       0,     0,     0,     0,   212,     0,     0,   291,     0,     0,
     348,     0,   589,   216,   217,   218,   219,   220,   590,     0,
     212,   108,   350,     0,     0,     0,  1457,    50,     0,     0,
       0,     0,     0,     0,     0,   189,     0,     0,    91,   344,
       0,    93,    94,    50,    95,   190,    97,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   348,
       0,     0,     0,   589,   216,   217,   218,   219,   220,   590,
     108,   350,     0,     0,     0,     0,     0,     0,     0,   589,
     216,   217,   218,   219,   220,   590,   189,     0,     0,    91,
     344,     0,    93,    94,     0,    95,   190,    97,     0,     0,
       0,  1386,   189,     0,     0,    91,   344,     0,    93,    94,
     348,    95,   190,    97,     0,     0,     0,     0,     0,   880,
     881,   108,   350,     0,     0,   882,   348,   883,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   108,   350,   884,
       0,     0,     0,     0,     0,     0,     0,    34,    35,    36,
     212,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   214,     0,  1160,  1161,  1162,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1114,  1163,  1563,     0,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,   885,
     886,   887,   888,   889,   890,    29,    81,    82,    83,    84,
      85,     0,  1186,    34,    35,    36,   212,   221,   213,    40,
       0,     0,   189,    89,    90,    91,    92,   214,    93,    94,
       0,    95,   190,    97,     0,     0,     0,    99,     0,    50,
       0,     0,     0,     0,     0,     0,   891,   892,     0,     0,
       0,     0,   105,     0,     0,     0,   215,   108,   893,     0,
       0,   880,   881,     0,     0,     0,     0,   882,     0,   883,
       0,     0,     0,     0,  1115,    75,   216,   217,   218,   219,
     220,   884,    81,    82,    83,    84,    85,     0,     0,    34,
      35,    36,   212,   221,     0,     0,     0,     0,   189,    89,
      90,    91,    92,   214,    93,    94,     0,    95,   190,    97,
       0,     0,     0,    99,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   105,     0,
       0,     0,     0,   108,   222,     0,     0,     0,     0,     0,
     112,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   885,   886,   887,   888,   889,   890,    29,    81,    82,
      83,    84,    85,     0,     0,    34,    35,    36,   212,   221,
     213,    40,     0,     0,   189,    89,    90,    91,    92,   214,
      93,    94,     0,    95,   190,    97,     0,     0,     0,    99,
       0,    50,     0,     0,     0,     0,     0,     0,   891,   892,
       0,     0,     0,     0,   105,     0,     0,     0,   215,   108,
     893,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,   216,   217,
     218,   219,   220,    29,    81,    82,    83,    84,    85,     0,
       0,    34,    35,    36,   212,   221,   213,    40,     0,     0,
     189,    89,    90,    91,    92,   214,    93,    94,     0,    95,
     190,    97,     0,     0,     0,    99,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     105,     0,     0,     0,   215,   108,   222,     0,     0,   627,
       0,     0,   112,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   647,    75,   216,   217,   218,   219,   220,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,   221,     0,     0,     0,     0,   189,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   190,    97,    29,  1056,
       0,    99,     0,     0,     0,     0,    34,    35,    36,   212,
       0,   213,    40,     0,     0,     0,   105,     0,     0,     0,
     214,   108,   222,     0,     0,     0,     0,     0,   112,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   215,
   -1133, -1133, -1133, -1133,  1173,  1174,  1175,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,  1183,  1184,  1185,     0,    75,   216,
     217,   218,   219,   220,    29,    81,    82,    83,    84,    85,
    1186,     0,    34,    35,    36,   212,   221,   213,    40,     0,
       0,   189,    89,    90,    91,    92,   214,    93,    94,     0,
      95,   190,    97,     0,     0,     0,    99,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   105,     0,     0,     0,   215,   108,   222,     0,     0,
       0,     0,     0,   112,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1216,    75,   216,   217,   218,   219,   220,
      29,    81,    82,    83,    84,    85,     0,     0,    34,    35,
      36,   212,   221,   213,    40,     0,     0,   189,    89,    90,
      91,    92,   214,    93,    94,     0,    95,   190,    97,     0,
       0,     0,    99,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   105,     0,     0,
       0,   215,   108,   222,     0,     0,     0,     0,     0,   112,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,   216,   217,   218,   219,   220,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   221,     0,
       0,     0,     0,   189,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   190,    97,     0,     0,     0,    99,     0,
       0,     0,     0,   474,   475,   476,     0,     0,     0,     0,
       0,     0,     0,   105,     0,     0,     0,     0,   108,   222,
       0,     0,     0,   477,   478,   112,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,   474,   475,   476,     0,     0,     0,     0,     0,     0,
       0,     0,   503,     0,     0,     0,     0,     0,     0,     0,
       0,   477,   478,     0,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     503,     0,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   547,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,     0,     0,     0,     0,     0,     0,   556,
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   503,
       0,   474,   475,   476,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     956,   477,   478,     0,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,   474,
     475,   476,     0,     0,     0,     0,     0,     0,     0,     0,
     503,     0,     0,     0,     0,     0,     0,     0,  1042,   477,
     478,     0,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   503,     0,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1098,
     477,   478,     0,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,  1160,  1161,
    1162,     0,     0,     0,     0,     0,     0,     0,     0,   503,
       0,     0,     0,     0,     0,     0,     0,  1433,     0,  1163,
       0,     0,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,
    1182,  1183,  1184,  1185,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1186,  1160,  1161,
    1162,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1475,  1163,
       0,     0,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,
    1182,  1183,  1184,  1185,  1160,  1161,  1162,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1186,     0,     0,
       0,     0,     0,     0,     0,  1163,  1359,     0,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1186,  1160,  1161,  1162,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1163,  1546,     0,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
    1160,  1161,  1162,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1186,     0,     0,     0,     0,     0,     0,
       0,  1163,  1558,     0,  1164,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1186,
    1160,  1161,  1162,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1163,  1666,     0,  1164,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,    34,    35,    36,   212,
       0,   213,    40,     0,     0,     0,     0,     0,     0,  1186,
     214,     0,     0,     0,     0,     0,     0,     0,  1761,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   243,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     244,     0,     0,     0,     0,     0,     0,     0,     0,   216,
     217,   218,   219,   220,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   221,     0,  1763,     0,
       0,   189,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   190,    97,     0,     0,     0,    99,     0,    34,    35,
      36,   212,     0,   213,    40,     0,     0,     0,     0,     0,
       0,   105,   678,     0,     0,     0,   108,   245,     0,     0,
       0,     0,     0,   112,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   215,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,   216,   217,   218,   219,   220,     0,    81,    82,    83,
      84,    85,   503,     0,    34,    35,    36,   212,   221,   213,
      40,     0,     0,   189,    89,    90,    91,    92,   214,    93,
      94,     0,    95,   190,    97,     0,     0,     0,    99,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   105,     0,     0,     0,   243,   108,   679,
       0,     0,     0,     0,     0,   112,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   216,   217,   218,
     219,   220,     0,    81,    82,    83,    84,    85,   212,     0,
       0,     0,     0,     0,   221,     0,     0,     0,     0,   189,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   190,
      97,    50,     0,     0,    99,     0,     0,     0,     0,   367,
     368,     0,     0,     0,     0,     0,     0,     0,     0,   105,
       0,     0,     0,     0,   108,   245,     0,     0,     0,     0,
       0,   112,     0,     0,     0,     0,     0,     0,   216,   217,
     218,   219,   220,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     369,     0,     0,   370,     0,     0,    93,    94,     0,    95,
     190,    97,     0,   474,   475,   476,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   371,     0,     0,     0,
     862,     0,     0,   477,   478,   108,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   503,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   474,   475,   476,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   863,   477,   478,  1039,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,   474,   475,   476,     0,     0,
       0,     0,     0,     0,     0,     0,   503,     0,     0,     0,
       0,     0,     0,     0,     0,   477,   478,     0,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,  1160,  1161,  1162,     0,     0,     0,     0,
       0,     0,     0,     0,   503,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1163,     0,     0,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,   475,
     476,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1186,     0,     0,     0,     0,     0,   477,   478,
       0,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,  1161,  1162,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   503,     0,     0,
       0,     0,     0,     0,     0,     0,  1163,     0,     0,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,   476,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1186,     0,     0,     0,     0,   477,
     478,     0,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   478,   503,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1163,     0,   503,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1186,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   503,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1186,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1186,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1186, -1133, -1133, -1133, -1133,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   503
};

static const yytype_int16 yycheck[] =
{
       5,     6,   193,     8,     9,    10,    11,    12,    13,    56,
      15,    16,    17,    18,   132,   167,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     4,    31,   124,    44,   696,
     572,   853,   734,   550,   109,   172,   173,    30,   109,    44,
      98,   109,     4,   124,   124,   103,   104,    52,   167,    54,
     996,    33,    57,   109,    59,     4,   723,    30,     4,     4,
     986,   560,   998,   417,    46,    30,   608,  1236,   417,    51,
      86,    30,   791,   131,   505,   506,   194,   417,   417,  1425,
     693,    86,   861,   450,   451,   452,   692,   868,   448,    57,
    1113,   502,   671,   541,     9,   534,   837,   534,     9,   508,
     621,   622,     9,     4,   109,  1018,    57,  1020,   539,     9,
    1033,     9,     9,     9,   537,   538,   191,    14,    14,     9,
     191,   258,    32,   191,    48,     9,    32,    48,  1051,   568,
      14,   568,     9,     9,   246,    48,   653,   654,     9,    38,
     878,     9,     9,   566,     4,     9,     9,   222,     9,    83,
       9,   222,    53,   257,   222,    56,     9,    48,  1227,     4,
       9,    83,   116,  1109,   681,     9,     9,    70,     9,    83,
     245,     9,    73,    91,   245,     9,  1099,    36,     9,    14,
     136,   137,    38,     9,    83,    70,   191,    38,   162,    91,
    1896,    48,     0,   198,    81,    96,    70,    98,    38,   716,
      70,    38,   103,   104,   162,   367,   368,   369,    38,   371,
     162,   565,   107,   108,   166,    70,    70,   222,    70,   183,
     183,   199,    50,    51,    14,   183,   180,    83,   202,  1155,
     131,   183,    83,   136,   137,   199,   199,   159,   160,   161,
     245,   159,    32,    83,   202,   132,    83,   203,   162,   199,
      70,   136,   137,    83,   270,   260,    70,   159,   263,   926,
     199,    51,  1000,    70,   781,   270,   271,     8,   103,   104,
     461,   205,   196,    70,    70,  1576,   200,   176,    70,   200,
     112,   136,   137,   196,    70,   200,   183,   132,   162,    70,
     201,   202,   202,   200,   996,   201,    70,    70,   402,   202,
     200,  1470,    70,   201,   201,   201,   207,   167,   203,   200,
     200,   358,  1335,  1054,   389,   264,   858,   201,   389,   268,
     176,   389,   827,    54,   201,   201,  1028,  1240,   202,   199,
     201,  1677,   200,    60,   201,    70,   176,   201,   201,   176,
     201,   200,  1123,   200,  1125,   199,   200,   202,   201,   194,
     202,   200,   543,   870,   359,  2061,   184,   201,   201,   876,
     201,    88,   200,   264,    91,  1288,   200,   268,    70,  2075,
     201,   272,   388,   448,   200,    70,   162,   448,    19,    20,
     448,   162,   202,   388,   389,  1686,  1312,   162,   202,   162,
     395,   396,   397,   398,   399,   400,   975,   455,   516,   446,
     405,   922,   923,  1472,    70,   202,   202,  1109,    70,  1710,
    1479,  1712,  1481,   418,   107,   108,   202,  1030,   199,   199,
     425,   202,   199,    70,   199,   199,   199,   166,   433,   202,
     199,   948,   168,   183,   202,    54,   167,    83,    83,    84,
     445,    70,  1511,   448,    83,   424,    31,   202,  1617,   199,
    1619,   182,    70,   511,   512,   513,   514,   358,   463,   464,
      70,   517,   201,   202,   199,    50,   202,   202,    53,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   448,   503,   199,
     505,   506,   507,    83,   952,  1243,  1225,   202,    83,   502,
     203,    83,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,    70,    83,    84,    83,   502,
     176,   432,   537,   538,   539,   540,   202,   502,    57,   544,
     202,   442,   547,   502,   199,   446,    19,    20,   167,   517,
      69,   556,  1331,   558,   455,   202,   195,   202,   710,  1628,
     205,   566,  1067,   202,  1733,  1004,   517,  1004,  1737,   574,
     199,   576,   432,   202,   183,   103,  1093,   183,  1120,   162,
     989,   199,   199,    70,   202,   123,   199,   432,   160,   161,
     199,   710,   565,   199,   132,   699,   176,   701,   239,   199,
     183,   176,   793,     4,   679,  1951,   199,   508,   509,   510,
     511,   512,   513,   514,   136,   137,  1354,   122,    19,    20,
    1137,   176,   627,   574,  1141,  1048,    83,   132,  1554,    70,
     579,  1148,   986,   534,    91,   167,  1242,   986,   166,   199,
     367,   368,   369,   370,   371,  1056,   986,   986,   205,   136,
     137,   124,  1019,   554,   199,  1022,  1023,   202,  1018,  1272,
    1020,    83,  1275,  1102,    83,  1102,   103,   568,   103,    91,
     515,    70,    91,   785,   679,   162,   508,   199,   579,    83,
    1201,   112,   409,    70,   742,   103,   199,    91,    31,   166,
     121,   122,   123,   124,   125,   126,    32,  1956,   599,  1958,
     199,   838,   534,   160,   161,   817,   199,    50,   109,    83,
      53,   118,   199,    38,   693,    83,   721,    91,   201,  1457,
    1646,   201,   554,    91,   625,   626,  1895,   831,   832,   166,
    1899,   166,   201,   565,   838,   839,   568,   378,   160,   161,
     201,   160,   161,   136,   137,   750,   387,   201,   166,  1158,
     912,   208,   914,   394,   916,   159,   160,   161,   920,   660,
     661,  1480,    70,   194,    70,   406,   239,   201,   162,   201,
     121,   122,   123,   124,   125,   126,   417,  1307,   783,  1309,
      87,   201,   894,  1313,    70,  1302,   160,   161,   207,   183,
     191,  1290,   160,   161,    70,   907,   168,    87,   162,  1304,
    1305,  1155,   398,  1308,   400,   199,  1155,   812,   202,   181,
    1315,    70,    78,    79,    80,  1155,  1155,   124,   125,   126,
      70,   222,   418,  1152,  1255,  1154,    92,  1156,    50,    51,
     202,    83,    70,   734,   124,   125,   126,   202,   239,    91,
     845,   742,  1448,   194,   245,   861,    83,    84,  1365,    70,
     710,   199,  1369,   106,   107,   108,   861,  1374,  1281,    75,
      76,  1474,   199,   264,    70,  1382,   162,   268,   199,  1292,
      49,  1413,  1591,   166,   734,   857,   106,   107,   108,  2048,
    1240,   147,   148,   149,   150,   151,  1334,    75,    76,   734,
     103,   104,   158,    69,   621,   622,  2065,   201,   164,   165,
     541,   134,   135,  1605,   183,   378,   162,   159,   160,   161,
     199,   403,   178,   204,   387,   407,   389,   866,   367,   368,
     369,   394,   371,   113,   114,   115,   827,   193,   829,   201,
     202,   201,   202,   406,   121,   122,   123,   124,   125,   126,
      53,    54,    55,   435,    57,   437,   438,   439,   440,     9,
     851,   956,   199,   958,   162,   960,    69,   162,  1312,    53,
      54,    55,     8,  1312,   865,   866,   971,   199,  1687,  1927,
    1928,  1513,  1312,  1312,   201,    69,   199,   378,  1495,   162,
     985,  1429,  1499,  1488,    14,  1490,   387,  1529,   389,  1506,
     162,   851,   201,   394,   201,   974,     9,  1055,  1923,  1924,
    1003,  1004,   202,   201,    14,   406,   851,   194,   909,   132,
    1015,   132,   974,  1018,   201,  1020,  1629,   918,   919,   998,
    1025,   183,   200,    14,   103,   974,   200,   200,   974,   974,
     200,   432,   200,   199,  1039,   206,   112,  1042,   202,  1044,
     199,   199,     9,  1048,   159,   200,   200,   448,   949,   909,
    1996,  1030,   200,  1476,   200,  2001,    50,    51,    52,    53,
      54,    55,    95,  1056,   909,     9,   201,  1018,   541,  1020,
     199,    14,   183,   974,  1025,    69,     9,   199,   719,    83,
    2026,  2017,   202,  1056,   201,  1452,  1453,  1454,   989,  1449,
     202,  1056,   201,  1098,   202,   996,   201,  1056,   202,   201,
     200,   200,  1003,  1004,  2040,   200,   134,   200,  1650,   201,
     199,     9,     9,  2049,   974,    70,   204,  1659,    50,    51,
      52,    53,    54,    55,  1106,    57,  1948,  1028,    32,   974,
    1635,  1953,  1637,  1675,  1639,   204,   996,    69,  1250,  1644,
     541,  1253,  2088,  1240,   121,   122,   123,   124,   125,   126,
     204,   996,   204,   794,  1055,   132,   133,   989,  1107,  1240,
    1240,   204,  1218,   135,  1065,  1066,  1067,  1576,  1028,   162,
     182,  1003,  1004,   138,     9,   200,   162,   200,   579,   196,
    2002,    14,     9,  1028,     9,   912,   184,   914,     9,   916,
    1613,  1614,   833,   920,   835,   922,   923,   924,   175,  1100,
    1554,  1102,    14,   200,     9,  1554,  1107,   134,  1109,   204,
    1111,   200,  1754,  1218,  1554,  1554,   200,   194,   200,   200,
       9,   204,   863,    14,   204,   203,   200,   200,   204,   199,
     103,  1132,   162,   201,   200,  1240,     9,   201,   138,  1744,
     162,     9,   200,   199,    70,  1224,   719,    70,    70,  1109,
    1255,  1111,    70,  1258,    70,   202,  2078,  1158,   199,   199,
       9,   203,  1224,    14,  1109,   201,  1111,  1218,  1100,   184,
    1102,     9,    14,   202,  1683,  1224,  1281,  1686,  1224,  1224,
    1947,   204,  1949,   202,    14,   201,  1187,  1292,  1293,  1240,
      19,    20,  1646,  1272,  1996,   936,  1275,  1646,   200,  2001,
     196,  1710,    14,  1712,    32,    70,  1646,  1646,   199,  1718,
     199,   952,   953,  2051,    32,  1331,  1453,  2055,   719,   199,
     199,   794,    14,  1224,  2026,    52,  1331,   199,   199,    70,
      70,  2069,  2070,   734,    70,    70,  1341,    70,   199,     9,
     200,  1323,   162,   201,   201,   986,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,   199,
     833,  1903,   835,   138,  1224,    14,   138,   184,   162,     9,
     200,  2038,    69,   176,  1449,   176,    69,  1326,  1449,  1224,
     204,  1449,     9,    83,   203,     9,  2088,    83,   827,   203,
     863,   201,   203,   794,   203,   199,  1901,  1902,    14,   138,
    1917,  1918,   199,  1304,  1305,  1306,  1307,  1308,  1309,   201,
      83,   199,  1313,   200,  1315,   199,   202,   200,   138,   204,
     202,   202,   201,     9,    92,  1326,   202,   159,  1433,    32,
      77,   201,   833,   200,   835,  1440,   184,  1338,   201,  1444,
    1592,  1446,    32,   138,  1449,   200,   200,  1348,  1427,     9,
     851,   204,   204,     9,   204,   138,     9,  1462,  1437,   204,
     204,     9,   863,   936,   201,   866,   200,    14,    83,  1587,
    1475,  1476,  1113,  1114,  1201,   200,   200,   203,  1338,   952,
     953,   203,   201,   204,   201,   201,   199,   202,  1897,   199,
     201,   200,   200,  1338,   200,  1474,   201,   199,  1449,   202,
     204,   121,   122,   123,   124,   125,   126,     9,   909,   200,
     239,   200,   132,   133,  1155,   138,   204,  1708,     9,   204,
    1421,   138,   204,   204,   200,     9,   200,    32,   138,   201,
     201,   200,   176,   200,  2076,   936,   201,   113,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,   952,   953,   173,   202,   175,   201,   171,   167,    83,
      14,    19,    20,    83,   119,   200,   200,  1427,   138,   189,
     202,   191,    30,   974,   194,   200,   138,  1437,    14,   183,
     202,    14,   201,    59,    60,    83,    14,  1488,    83,  1490,
     201,    14,   200,   200,   199,   996,   198,   138,    56,   200,
    1605,   138,   201,    14,   201,  1610,    14,   202,  1613,  1614,
       6,     9,     9,    68,   203,  1256,    83,  1018,   183,  1020,
     199,    83,  1680,     9,     9,   202,   201,  1028,  1067,   116,
     103,   162,   103,   184,   174,    36,    14,   200,   199,  1618,
    1113,  1114,   201,    83,   180,  1624,   184,  1626,   177,   378,
    1629,   184,    48,   199,     9,   200,    83,   201,   387,    83,
     136,   137,    14,   202,  1565,   394,   200,    83,   200,    83,
    1649,  1312,    14,    83,    14,  1576,    83,   406,  1319,    14,
      83,  1582,  2029,  1201,  2045,   514,  1049,  1753,  1332,   977,
    1881,  1257,  2040,  1334,  1335,   629,   509,  1526,  1740,  1778,
    1688,  1865,  1582,   511,  1605,  1565,  1107,  1656,  1109,  2086,
    1111,  2062,  1113,  1114,  1877,  1736,  1578,   113,  1308,   399,
    1565,  1228,  1727,   119,   200,   121,   122,   123,   124,   125,
     126,   127,  1592,  1066,  1635,  1149,  1637,  1153,  1639,  1303,
    1095,  1586,  1304,  1644,   395,  1605,  1941,   446,   879,  1997,
    1651,  1987,  1570,  1209,  1133,  1656,    -1,  1187,  1618,  1660,
    1605,    -1,    -1,  1742,  1624,    -1,  1626,  1240,    -1,    -1,
      -1,    -1,    -1,   169,   170,    -1,   172,    -1,    -1,  1680,
      -1,   239,  1683,  1256,    -1,  1686,    -1,    -1,  1429,  1649,
      -1,  1651,    -1,    -1,    -1,  1696,    -1,    -1,   194,    -1,
    1660,    -1,  1703,  1752,  1753,    -1,  1651,   203,    -1,  1710,
      -1,  1712,   541,    -1,    -1,  1660,    -1,  1718,    -1,    -1,
      -1,    -1,    -1,  1224,    -1,    -1,    -1,    -1,    -1,  1876,
      -1,   289,    -1,   291,    -1,    -1,    -1,    -1,    -1,  1240,
      -1,    -1,    -1,  1744,    -1,     6,  1319,    -1,    -1,    -1,
    1751,  1752,  1753,    -1,    -1,  1256,  1757,    -1,  2010,    -1,
      -1,  1334,  1335,  1764,    -1,  1304,  1305,  1306,  1307,  1308,
    1309,  1716,    -1,    -1,  1313,    -1,  1315,    78,    79,    80,
      81,  1886,  1742,    -1,    -1,    -1,    -1,    48,    -1,    -1,
      -1,  1751,   350,    -1,  1941,    -1,    -1,  1757,    -1,    -1,
      -1,    -1,    -1,   104,  1764,    -1,  1751,    -1,    -1,    -1,
      -1,    -1,  1757,  1554,    -1,  2033,    -1,    -1,  1319,  1764,
     378,    -1,    -1,    -1,    -1,  1326,    -1,    -1,    -1,   387,
      -1,    -1,    -1,  1334,  1335,    -1,   394,  1338,    -1,    -1,
     141,   142,   143,   144,   145,    -1,    -1,    -1,   406,    -1,
      -1,    -1,   113,    -1,    -1,    -1,  1429,    -1,   119,   417,
     121,   122,   123,   124,   125,   126,   127,    -1,   169,   170,
      -1,   172,   173,   174,    -1,  1876,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   443,    -1,    -1,   446,    -1,
     719,    -1,    -1,    81,    -1,    -1,  1897,   198,    -1,    -1,
    1901,  1902,    -1,    -1,    -1,  1646,  1907,    -1,   169,   170,
      -1,   172,    -1,    -1,    -1,  1916,   104,    -1,    -1,    -1,
      -1,    -1,  1923,  1924,    -1,    -1,  1927,  1928,  1429,    81,
      -1,    -1,    -1,   194,    -1,    -1,    -1,    -1,  2017,    -1,
    1941,    -1,   203,    -1,   502,    -1,    -1,  1907,  1449,  1488,
      -1,  1490,   104,   141,   142,   143,   144,   145,    -1,    -1,
    1961,  2040,  1907,    -1,    -1,   794,    -1,    -1,    -1,    -1,
    2049,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,  2084,
      -1,   169,   170,   541,   172,   173,   174,    -1,  2093,   141,
     142,   143,   144,   145,    -1,  1996,  2101,    -1,    -1,  2104,
    2001,  1961,    -1,    -1,   833,    -1,   835,    -1,  2009,    -1,
     198,    -1,    -1,    -1,   166,   203,  1961,   169,   170,    -1,
     172,   173,   174,    -1,    -1,  2026,    -1,    -1,    -1,    -1,
      -1,  2032,    -1,    -1,   863,   593,  1996,   595,    -1,    -1,
     598,  2001,    -1,    -1,    -1,    -1,   198,     6,    -1,    -1,
    2010,  1996,    -1,    -1,    -1,    -1,  2001,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1565,    -1,  2026,     6,    -1,    -1,
      -1,    -1,    -1,   631,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  2026,    -1,     6,    -1,    -1,  2087,  2088,    -1,    48,
      -1,    -1,    -1,    -1,    -1,    -1,  1635,    -1,  1637,    -1,
    1639,    -1,     6,    -1,  1605,  1644,    -1,   936,    -1,    48,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   952,   953,    48,    -1,  2087,  2088,    -1,
      -1,   689,   690,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     698,    -1,  2087,  2088,    48,    -1,    -1,    -1,    -1,    -1,
    1651,    -1,    -1,    -1,   113,  1656,    -1,    -1,    -1,  1660,
     119,   719,   121,   122,   123,   124,   125,   126,   127,    -1,
      -1,    -1,    -1,    -1,   113,    -1,    56,    -1,    -1,    -1,
     119,    -1,   121,   122,   123,   124,   125,   126,   127,    -1,
     113,    -1,    -1,    -1,    -1,    -1,   119,    -1,   121,   122,
     123,   124,   125,   126,   127,  1744,    -1,    -1,    -1,   113,
     169,   170,    -1,   172,    -1,   119,    -1,   121,   122,   123,
     124,   125,   126,   127,    -1,     6,    -1,    -1,    -1,    -1,
     169,   170,    -1,   172,    -1,   194,   794,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   203,    -1,   169,   170,    -1,   172,
    1751,  1752,  1753,    -1,    -1,   194,  1757,    -1,    31,    -1,
      -1,    -1,    -1,  1764,   203,   169,   170,    48,   172,   827,
      -1,   194,    -1,    -1,    -1,   833,    -1,   835,    -1,    -1,
     203,    -1,    -1,    -1,  1113,  1114,    -1,    -1,    -1,    -1,
     194,    -1,    -1,    81,    -1,    83,    84,    -1,    -1,   203,
      -1,    -1,    -1,    -1,    -1,   863,   864,    -1,    81,    -1,
      -1,    -1,    -1,   871,    -1,    -1,   104,    -1,    -1,    92,
     878,   879,   880,   881,   882,   883,   884,    -1,    -1,    -1,
      -1,   104,   113,    -1,    -1,   893,    -1,    -1,   119,    -1,
     121,   122,   123,   124,   125,   126,   127,    -1,    -1,    -1,
      -1,    -1,   910,   141,   142,   143,   144,   145,    -1,    -1,
      -1,    -1,  1901,  1902,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,    -1,    -1,    -1,    -1,    -1,   936,   167,
      -1,   169,   170,    -1,   172,   173,   174,    -1,   169,   170,
     163,   172,   950,   166,   952,   953,   169,   170,    -1,   172,
     173,   174,    -1,   176,    -1,    -1,  1907,    -1,    -1,   289,
     198,   291,    -1,   194,   202,    19,    20,   205,   976,   977,
      -1,    -1,   203,    -1,    -1,   198,    30,  1256,   986,    -1,
      -1,    -1,    -1,    -1,    -1,   993,    -1,    -1,    -1,    -1,
      -1,    -1,  1000,    -1,    -1,    -1,    -1,    -1,  1006,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1017,
    1961,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     350,    -1,    -1,    -1,  1032,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
    1319,    -1,    -1,    -1,  1052,  1996,    -1,    -1,  1056,    -1,
    2001,    69,    10,    11,    12,  1334,  1335,    -1,    -1,  1067,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,  2026,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1113,  1114,    -1,    -1,    -1,
      -1,    69,    -1,   443,    -1,    -1,   446,    -1,    -1,  1127,
      -1,    -1,    -1,  1131,    -1,  1133,    -1,    -1,  1136,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  2087,  2088,    -1,    -1,
      -1,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,    -1,
    1429,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,    10,
      11,    12,    -1,    -1,    -1,   239,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1205,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    81,    -1,    -1,
      -1,    -1,    -1,  1241,    -1,  1243,    -1,    -1,    69,    -1,
      -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,  1256,    -1,
     104,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   593,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1284,    -1,    -1,  1287,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,    -1,    -1,    -1,    -1,  1304,  1305,  1306,  1307,
    1308,  1309,    -1,    -1,  1312,  1313,    -1,  1315,    -1,    -1,
      -1,  1319,   166,    -1,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,   378,    -1,  1334,  1335,    -1,  1337,
      -1,    -1,    -1,   387,    -1,    -1,    -1,    -1,    -1,  1347,
     394,    -1,    -1,    -1,   198,    -1,  1354,    -1,   202,    -1,
      -1,  1359,   406,  1361,    -1,    -1,    -1,    -1,    -1,   689,
     690,    -1,    -1,   417,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,   204,    -1,    -1,    -1,    -1,  1386,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,  1422,  1423,    -1,    -1,  1426,    -1,
      -1,  1429,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   502,  1457,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
    1488,    -1,  1490,    -1,    -1,    -1,   289,   541,   291,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,  1534,  1535,    -1,    -1,
    1538,    -1,    -1,    -1,    -1,    -1,  1544,    -1,  1546,    69,
    1548,    59,    60,    -1,   598,  1553,  1554,   350,   878,   879,
    1558,    -1,  1560,    -1,    -1,  1563,   201,    -1,   203,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,  1576,  1577,
      -1,    -1,  1580,    -1,    -1,    -1,    -1,   631,    -1,  1587,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,   137,
      -1,    -1,    69,   203,    -1,    -1,    -1,  1635,    -1,  1637,
      -1,  1639,    -1,    -1,    -1,    -1,  1644,    -1,  1646,    -1,
     443,    -1,    -1,   446,    -1,    -1,   976,    -1,    -1,    -1,
      -1,    -1,    -1,  1661,    81,    -1,    -1,    -1,  1666,    -1,
      -1,    -1,    -1,   993,    -1,   719,    -1,    -1,    -1,    -1,
    1678,  1679,    -1,    -1,    -1,    -1,  1006,   104,  1686,    -1,
    1688,    -1,   200,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,    -1,  1710,   130,  1712,    -1,    -1,    -1,    -1,    31,
    1718,    -1,    -1,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,  1052,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      59,    60,    -1,    -1,    -1,    -1,  1744,    -1,    -1,    -1,
     794,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,    -1,    -1,  1761,  1762,  1763,   203,    -1,    -1,    81,
    1768,    -1,  1770,    -1,    -1,    -1,    -1,    -1,  1776,    -1,
    1778,   198,   199,   827,    -1,    -1,    -1,    -1,    -1,   833,
      -1,   835,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     593,    -1,   595,    -1,    -1,    -1,    -1,  1127,    -1,    -1,
      -1,  1131,    -1,    -1,    -1,   127,  1136,   136,   137,   863,
     864,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,   146,    -1,   880,   881,   882,   883,
     884,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   893,
      -1,   163,    -1,    -1,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    31,   910,    81,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,   200,  1880,    -1,    -1,    -1,   198,    -1,    -1,    -1,
     104,    -1,   936,    -1,    -1,  1893,   689,   690,    -1,  1897,
      -1,    -1,    -1,  1901,  1902,   698,   950,    -1,   952,   953,
      -1,    -1,    -1,    59,    60,    81,    -1,    -1,  1916,    -1,
      -1,  1241,    -1,    -1,  1922,    -1,    92,   141,   142,   143,
     144,   145,    -1,   977,    -1,  1933,    -1,    -1,   104,    -1,
      -1,  1939,   986,    -1,    -1,  1943,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   167,    -1,   169,   170,   171,   172,   173,
     174,    -1,    -1,    -1,  1284,    -1,    -1,  1287,    -1,    -1,
      -1,    -1,  1970,  1017,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,   198,   199,    -1,    -1,  1032,    -1,
     136,   137,    -1,    -1,    -1,    -1,    -1,   163,    -1,  1997,
     166,  1999,    -1,   169,   170,    -1,   172,   173,   174,    -1,
     176,    -1,  1056,    -1,    -1,    -1,    -1,  2015,    -1,    -1,
      -1,    -1,    -1,  1067,    -1,    -1,    -1,  1347,    -1,    -1,
    2028,    -1,   198,    -1,  1354,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  2045,    -1,    -1,
      -1,    -1,    -1,  2051,   200,    -1,    -1,  2055,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1113,
    1114,  2069,  2070,    -1,    -1,    -1,    -1,    -1,   871,    -1,
      -1,    -1,    -1,    -1,    -1,   878,   879,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1422,  1423,    -1,  1149,  1150,  1151,  1152,  1153,
    1154,  1155,  1156,    -1,    -1,  1159,  1160,  1161,  1162,  1163,
    1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,
      20,  1205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   976,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
     993,    -1,    -1,    -1,    -1,    -1,    -1,  1000,    -1,    -1,
      -1,    -1,  1256,  1006,  1534,  1535,    30,    31,  1538,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,  1576,    -1,    -1,  1052,
    1304,  1305,  1306,  1307,  1308,  1309,    -1,  1587,  1312,  1313,
      -1,  1315,    -1,    -1,    -1,  1319,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1334,  1335,    -1,  1337,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,  1359,    -1,  1361,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,  1127,    -1,    -1,    -1,  1131,    -1,
    1133,  1661,  1386,  1136,    -1,    -1,    -1,    59,    60,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    59,    60,  1686,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,   239,
      -1,    -1,  1426,    -1,    -1,  1429,    -1,    -1,    -1,   203,
    1710,    -1,  1712,    -1,    -1,    -1,    30,    31,  1718,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,   136,   137,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    10,    11,
      12,   136,   137,    -1,  1488,    -1,  1490,    -1,  1241,    -1,
    1243,    -1,    -1,    -1,    -1,    -1,  1776,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,   200,    -1,
      -1,  1284,    -1,    -1,  1287,    50,    51,    69,    -1,    -1,
    1544,    -1,  1546,    -1,  1548,    -1,    -1,    -1,    -1,  1553,
    1554,    -1,    -1,    -1,  1558,    70,  1560,    -1,   378,  1563,
      -1,    -1,    -1,    78,    79,    80,    81,   387,    -1,    -1,
      -1,    -1,    -1,  1577,   394,    -1,  1580,    92,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   406,    -1,    -1,   104,
      -1,    -1,    -1,    -1,  1347,    -1,    -1,   417,    -1,    -1,
    1880,  1354,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   203,
      -1,    -1,    -1,  1893,    -1,    -1,    -1,  1897,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,
     145,  1635,    -1,  1637,    -1,  1639,    -1,    -1,    -1,    -1,
    1644,    -1,  1646,   158,    -1,    81,    -1,    -1,   163,    -1,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,  1666,    81,    -1,    83,    84,    -1,   104,  1422,
    1423,   203,   187,   188,  1678,  1679,    -1,    -1,    -1,    -1,
      -1,    -1,   502,   198,  1688,    -1,   104,    -1,    -1,    -1,
    1970,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   598,    -1,
      -1,    -1,    -1,    -1,  1457,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1997,    -1,  1999,
      -1,   541,    -1,   141,   142,   143,   144,   145,    -1,    -1,
      -1,   631,    -1,   169,   170,    -1,   172,   173,   174,    -1,
    1744,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   167,
      -1,   169,   170,    31,   172,   173,   174,  1761,  1762,  1763,
      -1,    -1,   198,   199,  1768,    -1,  1770,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1778,    -1,    -1,    -1,   598,    -1,
     198,  1534,  1535,    -1,   202,  1538,    -1,   205,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,
      -1,   631,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1576,    -1,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    30,    31,  1587,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,  1901,  1902,   719,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1661,   187,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,  1922,    -1,
     198,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1933,
      -1,    -1,    -1,  1686,    -1,  1939,    -1,    -1,   104,  1943,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,  1710,    -1,  1712,
      -1,    -1,    -1,    -1,   864,  1718,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   794,   141,   142,   143,   144,   145,
     880,   881,   882,   883,   884,    -1,    -1,    -1,    59,    60,
      -1,    -1,    -1,   893,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   169,   170,   203,   172,   173,   174,    -1,
      -1,  2015,    -1,   833,    -1,   835,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1776,  2028,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  2045,    -1,   863,   864,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
     880,   881,   882,   883,   884,   136,   137,    -1,    -1,    -1,
      -1,    -1,    -1,   893,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,   936,  1017,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1880,    -1,    -1,
      -1,    -1,   952,   953,    -1,    -1,    -1,    -1,    -1,    81,
    1893,    -1,    -1,    -1,  1897,    -1,    -1,    10,    11,    12,
      92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,  1916,    -1,    -1,   986,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,  1017,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1970,    -1,    -1,
      -1,   163,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,  1056,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1997,    -1,  1999,   203,    -1,  1149,
    1150,    -1,    -1,  1153,    -1,    -1,   198,    -1,    -1,  1159,
    1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,    -1,    -1,    -1,
      -1,    -1,    -1,  1113,  1114,    -1,    -1,    -1,  2051,    -1,
      -1,    -1,  2055,    -1,    -1,  1205,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  2069,  2070,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1149,
    1150,  1151,  1152,  1153,  1154,  1155,  1156,    -1,   201,  1159,
    1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,    10,    11,    12,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,  1205,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    81,    59,    60,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1256,  1337,    -1,   104,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,  1359,
      57,  1361,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,  1386,    -1,    -1,    -1,
      -1,    -1,  1312,    -1,   136,   137,    -1,    -1,   163,  1319,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,    -1,  1334,  1335,    -1,  1337,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   198,    -1,    -1,    -1,    -1,    -1,  1359,
      -1,  1361,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     203,    -1,    -1,    -1,    30,    31,  1386,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,  1429,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    10,    11,    12,  1544,    -1,  1546,    -1,  1548,    -1,
      -1,    69,    -1,  1553,    -1,    -1,    -1,    -1,  1558,    -1,
    1560,    30,    31,  1563,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    31,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,  1544,    -1,  1546,    -1,  1548,    -1,
      -1,    -1,    -1,  1553,  1554,    -1,    69,   203,  1558,    68,
    1560,    -1,    -1,  1563,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    10,    11,    12,  1666,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,   203,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,   140,   141,   142,   143,   144,   145,   146,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,  1646,    -1,    -1,    -1,
      -1,    -1,   201,    -1,   163,    -1,    -1,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,  1666,   176,    -1,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,   187,    -1,
      13,  1761,  1762,  1763,    -1,    -1,    -1,    -1,  1768,   198,
     199,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      -1,  1761,  1762,  1763,    -1,    -1,    -1,    -1,  1768,    -1,
      -1,   104,    -1,    -1,   201,    -1,    -1,  1777,    -1,   112,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,
     123,   124,   125,   126,    -1,    -1,   129,   130,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,  1922,    -1,    -1,   178,    -1,    -1,   181,    -1,
      -1,    -1,    -1,  1933,   187,   188,    -1,    81,    -1,  1939,
     193,   194,   195,  1943,    -1,   198,   199,    -1,    10,    11,
      12,    -1,   205,   206,   207,   208,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,   141,   142,   143,
     144,   145,  1922,    -1,    -1,    -1,    -1,    69,     3,     4,
       5,     6,     7,  1933,    -1,  2015,    -1,    -1,    13,  1939,
      -1,    -1,   166,  1943,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1972,    48,   198,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,  2015,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,   113,   114,
     115,    -1,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,   131,   132,   133,   201,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,   188,   189,    -1,   191,    -1,   193,   194,
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
     109,   110,   111,   112,   113,   114,   115,    -1,   117,    -1,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
     129,   130,   131,   132,   133,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
     189,    -1,   191,    -1,   193,   194,   195,    -1,    -1,   198,
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
      -1,    -1,    57,    -1,    -1,    -1,   167,    -1,   169,   170,
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
      -1,    -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   163,    57,    -1,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    68,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,   198,
      -1,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,    -1,   130,    -1,
      -1,   133,   134,   135,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    83,    -1,    85,    -1,    -1,    -1,    -1,
      -1,   163,    -1,    -1,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,   104,   177,    -1,    -1,   180,    -1,
       3,     4,    -1,     6,     7,   187,   188,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,
      -1,   203,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,
      -1,   141,   142,   143,   144,   145,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    57,    57,    -1,    -1,    -1,   169,
     170,    -1,   172,   173,   174,    68,    -1,    69,    71,    72,
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
     141,   142,   143,   144,   145,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    57,    -1,    -1,    -1,    -1,   169,   170,
      -1,   172,   173,   174,    68,    -1,    69,    71,    72,    73,
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
     165,    -1,    -1,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,   177,   178,    -1,   180,    -1,    -1,    -1,    -1,
      -1,    -1,   187,    -1,   189,    -1,   191,    -1,   193,   194,
      -1,     3,     4,   198,     6,     7,    12,    -1,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    28,    29,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,
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
      57,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   201,    57,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    81,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    31,    57,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,    -1,   201,    69,    -1,    -1,
      -1,    -1,    -1,   121,   122,   123,   124,   125,   126,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    31,    81,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,   200,    -1,   163,    -1,    -1,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,
      68,    -1,    -1,    -1,    -1,    -1,   194,    -1,    -1,    -1,
     198,   199,    -1,    81,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   200,   140,   141,   142,   143,   144,   145,   146,
      31,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,    -1,   163,    -1,    -1,   166,
     167,    68,   169,   170,    -1,   172,   173,   174,    -1,   176,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    68,    -1,    -1,
     187,    -1,   140,   141,   142,   143,   144,   145,   146,    -1,
      81,   198,   199,    -1,    -1,    -1,    87,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,
      -1,   169,   170,   104,   172,   173,   174,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,
      -1,    -1,    -1,   140,   141,   142,   143,   144,   145,   146,
     198,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,   145,   146,   163,    -1,    -1,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,    32,   163,    -1,    -1,   166,   167,    -1,   169,   170,
     187,   172,   173,   174,    -1,    -1,    -1,    -1,    -1,    50,
      51,   198,   199,    -1,    -1,    56,   187,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,   199,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    38,    31,    32,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,   140,
     141,   142,   143,   144,   145,    70,   147,   148,   149,   150,
     151,    -1,    69,    78,    79,    80,    81,   158,    83,    84,
      -1,    -1,   163,   164,   165,   166,   167,    92,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,
      -1,    -1,   193,    -1,    -1,    -1,   121,   198,   199,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    70,   147,   148,   149,   150,   151,    -1,    -1,    78,
      79,    80,    81,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    92,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,   178,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,
      -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,
     205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,    70,   147,   148,
     149,   150,   151,    -1,    -1,    78,    79,    80,    81,   158,
      83,    84,    -1,    -1,   163,   164,   165,   166,   167,    92,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,   187,   188,
      -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,   121,   198,
     199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,    70,   147,   148,   149,   150,   151,    -1,
      -1,    78,    79,    80,    81,   158,    83,    84,    -1,    -1,
     163,   164,   165,   166,   167,    92,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     193,    -1,    -1,    -1,   121,   198,   199,    -1,    -1,   202,
      -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,    70,    71,
      -1,   178,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,   193,    -1,    -1,    -1,
      92,   198,   199,    -1,    -1,    -1,    -1,    -1,   205,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,   140,   141,
     142,   143,   144,   145,    70,   147,   148,   149,   150,   151,
      69,    -1,    78,    79,    80,    81,   158,    83,    84,    -1,
      -1,   163,   164,   165,   166,   167,    92,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,   178,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   193,    -1,    -1,    -1,   121,   198,   199,    -1,    -1,
      -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      70,   147,   148,   149,   150,   151,    -1,    -1,    78,    79,
      80,    81,   158,    83,    84,    -1,    -1,   163,   164,   165,
     166,   167,    92,   169,   170,    -1,   172,   173,   174,    -1,
      -1,    -1,   178,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,
      -1,   121,   198,   199,    -1,    -1,    -1,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,   198,   199,
      -1,    -1,    -1,    30,    31,   205,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    31,
      -1,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    31,
      -1,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,   138,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,   138,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,   138,    -1,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,   138,    -1,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,   138,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,   178,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,   193,    92,    -1,    -1,    -1,   198,   199,    -1,    -1,
      -1,    -1,    -1,   205,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   121,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    69,    -1,    78,    79,    80,    81,   158,    83,
      84,    -1,    -1,   163,   164,   165,   166,   167,    92,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   193,    -1,    -1,    -1,   121,   198,   199,
      -1,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,    -1,   147,   148,   149,   150,   151,    81,    -1,
      -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,   104,    -1,    -1,   178,    -1,    -1,    -1,    -1,   112,
     113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,
      -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,
      -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     163,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   189,    -1,    -1,    -1,
      27,    -1,    -1,    30,    31,   198,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
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
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      54,   167,   182,   419,   363,   138,     9,   440,   200,   162,
     200,   524,   524,    14,   374,   300,   231,   196,     9,   441,
      87,   524,   525,   460,   460,   203,     9,   440,   184,   470,
      83,    84,   302,   361,   200,     9,   441,    14,     9,   200,
       9,   200,   200,   200,   200,    14,   200,   203,   234,   235,
     366,   259,   134,   279,   199,   502,   204,   203,   361,    32,
     204,   204,   138,   203,     9,   440,   361,   503,   199,   269,
     264,   274,    14,   497,   267,   256,    71,   470,   361,   503,
     200,   200,   204,   203,   200,    50,    51,    70,    78,    79,
      80,    92,   140,   141,   142,   143,   144,   145,   158,   187,
     188,   216,   390,   393,   396,   399,   402,   405,   425,   436,
     443,   445,   446,   450,   453,   216,   470,   470,   138,   279,
     460,   465,   460,   200,   361,   295,    75,    76,   296,   231,
     360,   233,   351,   103,    38,   139,   285,   470,   434,   216,
      32,   236,   289,   201,   292,   201,   292,     9,   440,    92,
     229,   138,   162,     9,   440,   200,    87,   506,   507,   524,
     525,   504,   434,   434,   434,   434,   434,   439,   442,   199,
      70,    70,    70,    70,    70,   199,   199,   434,   162,   202,
      10,    11,    12,    31,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    69,   162,   503,   203,
     425,   202,   253,   221,   221,   221,   216,   221,   222,   222,
     226,     9,   441,   203,   203,    14,   470,   201,   184,     9,
     440,   216,   282,   425,   202,   484,   139,   470,    14,   361,
     361,   204,   361,   203,   212,   524,   282,   202,   418,    14,
     200,   361,   375,   480,   201,   524,   196,   203,   232,   235,
     245,    32,   511,   459,   525,    38,    83,   176,   461,   462,
     464,   461,   462,   464,   524,    70,    38,    87,   176,   361,
     434,   248,   357,   358,   470,   249,   248,   249,   249,   203,
     235,   300,   199,   425,   280,   367,   260,   361,   361,   361,
     203,   199,   303,   281,    32,   280,   524,    14,   279,   502,
     429,   203,   199,    14,    78,    79,    80,   216,   444,   444,
     446,   448,   449,    52,   199,    70,    70,    70,    70,    70,
      91,   159,   199,   199,   162,     9,   440,   200,   454,    38,
     361,   280,   203,    75,    76,   297,   360,   236,   203,   201,
      96,   201,   285,   470,   199,   138,   284,    14,   233,   292,
     106,   107,   108,   292,   203,   524,   184,   138,   162,   524,
     216,   176,   517,   524,     9,   440,   200,   176,   440,   138,
     204,     9,   440,   439,   384,   385,   434,   407,   434,   435,
     407,   384,   407,   375,   377,   379,   407,   200,   132,   217,
     434,   489,   490,   434,   434,   434,    32,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   522,    83,   254,   203,   203,   203,   203,   225,   201,
     434,   516,   103,   104,   512,   514,     9,   311,   200,   199,
     352,   357,   361,   138,   204,   203,   497,   311,   168,   181,
     202,   414,   421,   168,   202,   420,   138,   201,   511,   199,
     248,   348,   362,   471,   474,   524,   374,    87,   525,    83,
      83,   176,    14,    83,   503,   503,   481,   470,   302,   361,
     200,   300,   202,   300,   199,   138,   199,   303,   200,   202,
     524,   202,   201,   524,   280,   261,   432,   303,   138,   204,
       9,   440,   445,   448,   386,   387,   446,   408,   446,   447,
     408,   386,   408,   159,   375,   451,   452,   408,    81,   446,
     470,   202,   360,    32,    77,   236,   201,   351,   284,   484,
     285,   200,   434,   102,   106,   201,   361,    32,   201,   293,
     203,   184,   524,   216,   138,    87,   524,   525,    32,   200,
     434,   434,   200,   204,     9,   440,   138,   204,     9,   440,
     204,   204,   204,   138,     9,   440,   200,   200,   138,   203,
       9,   440,   434,    32,   200,   233,   201,   201,   201,   201,
     216,   524,   524,   512,   425,     6,   113,   119,   122,   127,
     169,   170,   172,   203,   312,   337,   338,   339,   344,   345,
     346,   347,   458,   484,   361,   203,   202,   203,    54,   361,
     361,   361,   374,   470,   201,   202,   525,    38,    83,   176,
      14,    83,   361,   199,   199,   204,   511,   200,   311,   200,
     300,   361,   303,   200,   311,   497,   311,   201,   202,   199,
     200,   446,   446,   200,   204,     9,   440,   138,   204,     9,
     440,   204,   204,   204,   138,   200,     9,   440,   200,   311,
      32,   233,   201,   200,   200,   200,   241,   201,   201,   293,
     233,   138,   524,   524,   176,   524,   138,   434,   434,   434,
     434,   375,   434,   434,   434,   202,   203,   514,   134,   135,
     189,   217,   500,   524,   283,   425,   113,   347,    31,   127,
     140,   146,   167,   173,   321,   322,   323,   324,   425,   171,
     329,   330,   130,   199,   216,   331,   332,   313,   257,   524,
       9,   201,     9,   201,   201,   497,   338,   200,   308,   167,
     416,   203,   203,   361,    83,    83,   176,    14,    83,   361,
     303,   303,   119,   364,   511,   203,   511,   200,   200,   203,
     202,   203,   311,   300,   138,   446,   446,   446,   446,   375,
     203,   233,   239,   242,    32,   236,   287,   233,   524,   200,
     434,   138,   138,   138,   233,   425,   425,   502,    14,   217,
       9,   201,   202,   500,   497,   324,   183,   202,     9,   201,
       3,     4,     5,     6,     7,    10,    11,    12,    13,    27,
      28,    29,    57,    71,    72,    73,    74,    75,    76,    77,
      87,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   139,   140,   147,   148,   149,   150,   151,
     163,   164,   165,   175,   177,   178,   180,   187,   189,   191,
     193,   194,   216,   422,   423,     9,   201,   167,   171,   216,
     332,   333,   334,   201,    83,   343,   256,   314,   500,   500,
      14,   257,   203,   309,   310,   500,    14,    83,   361,   200,
     200,   199,   511,   198,   508,   364,   511,   308,   203,   200,
     446,   138,   138,    32,   236,   286,   287,   233,   434,   434,
     434,   203,   201,   201,   434,   425,   317,   524,   325,   326,
     433,   322,    14,    32,    51,   327,   330,     9,    36,   200,
      31,    50,    53,    14,     9,   201,   218,   501,   343,    14,
     524,   256,   201,    14,   361,    38,    83,   413,   202,   509,
     510,   524,   201,   202,   335,   511,   508,   203,   511,   446,
     446,   233,   100,   252,   203,   216,   229,   318,   319,   320,
       9,   440,     9,   440,   203,   434,   423,   423,    68,   328,
     333,   333,    31,    50,    53,   434,    83,   183,   199,   201,
     434,   501,   434,    83,     9,   441,   231,     9,   441,    14,
     512,   231,   202,   335,   335,    98,   201,   116,   243,   162,
     103,   524,   184,   433,   174,    14,   513,   315,   199,    38,
      83,   200,   203,   510,   524,   203,   231,   201,   199,   180,
     255,   216,   338,   339,   184,   434,   184,   298,   299,   459,
     316,    83,   203,   425,   253,   177,   216,   201,   200,     9,
     441,    87,   124,   125,   126,   341,   342,   298,    83,   283,
     201,   511,   459,   525,   525,   200,   200,   201,   508,    87,
     341,    83,    38,    83,   176,   511,   202,   201,   202,   336,
     525,   525,    83,   176,    14,    83,   508,   233,   231,    83,
      38,    83,   176,    14,    83,   361,   336,   203,   203,    83,
     176,    14,    83,   361,    14,    83,   361,   361
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
     416,   416,   417,   417,   418,   418,   419,   420,   420,   421,
     421,   421,   422,   422,   422,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   424,   425,
     425,   426,   426,   426,   426,   426,   427,   427,   428,   428,
     428,   428,   429,   429,   429,   430,   430,   430,   431,   431,
     431,   432,   432,   433,   433,   433,   433,   433,   433,   433,
     433,   433,   433,   433,   433,   433,   433,   433,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   435,   435,   436,   437,   437,   438,
     438,   438,   438,   438,   438,   438,   439,   439,   440,   440,
     441,   441,   442,   442,   442,   442,   443,   443,   443,   443,
     443,   444,   444,   444,   444,   445,   445,   446,   446,   446,
     446,   446,   446,   446,   446,   446,   446,   446,   446,   446,
     446,   446,   446,   447,   447,   448,   448,   449,   449,   449,
     449,   450,   450,   451,   451,   452,   452,   453,   453,   454,
     454,   455,   455,   457,   456,   458,   459,   459,   460,   460,
     461,   461,   461,   462,   462,   463,   463,   464,   464,   465,
     465,   466,   466,   466,   467,   467,   468,   468,   469,   469,
     470,   470,   470,   470,   470,   470,   470,   470,   470,   470,
     470,   471,   472,   472,   472,   472,   472,   472,   472,   472,
     473,   473,   473,   473,   473,   473,   473,   473,   473,   474,
     475,   475,   476,   476,   476,   477,   477,   477,   478,   479,
     479,   479,   480,   480,   480,   480,   481,   481,   482,   482,
     482,   482,   482,   482,   483,   483,   483,   483,   483,   484,
     484,   484,   484,   484,   484,   485,   485,   486,   486,   486,
     486,   486,   486,   486,   486,   487,   487,   488,   488,   488,
     488,   489,   489,   490,   490,   490,   490,   491,   491,   491,
     491,   492,   492,   492,   492,   492,   492,   493,   493,   493,
     494,   494,   494,   494,   494,   494,   494,   494,   494,   494,
     494,   495,   495,   496,   496,   497,   497,   498,   498,   498,
     498,   499,   499,   500,   500,   501,   501,   502,   502,   503,
     503,   504,   504,   505,   506,   506,   506,   506,   506,   506,
     507,   507,   507,   507,   508,   508,   509,   509,   510,   510,
     511,   511,   512,   512,   513,   514,   514,   515,   515,   515,
     515,   516,   516,   516,   517,   517,   517,   517,   518,   518,
     519,   519,   519,   519,   520,   521,   522,   522,   523,   523,
     524,   524,   524,   524,   524,   524,   524,   524,   524,   524,
     524,   525,   525
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
       0,     1,     4,     0,     2,     0,     1,     1,     3,     1,
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
       1,     1,     1,     1,     1,     3,     3,     4,     4,     3,
       1,     1,     7,     9,     9,     7,     6,     8,     1,     4,
       4,     1,     1,     1,     4,     2,     1,     0,     1,     1,
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
#line 759 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
#line 7289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 762 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 7297 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 769 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 7303 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 770 "hphp.y" /* yacc.c:1646  */
    { }
#line 7309 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 773 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 7315 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7321 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 775 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7327 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7333 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 778 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 7345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 779 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 7353 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 782 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 7360 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 784 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 7366 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 785 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7372 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 786 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 7378 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 787 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7384 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 788 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7392 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 792 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7401 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 797 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7410 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 802 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7417 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 805 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 808 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7432 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 812 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 816 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 820 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7456 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 824 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7464 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 827 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7471 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7477 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7483 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7489 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7495 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7501 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7507 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7513 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7519 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 840 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7525 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7531 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 842 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7537 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 843 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7543 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 844 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7549 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 926 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7555 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 928 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7561 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 933 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7567 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 934 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7574 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 940 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7580 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 944 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7586 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 945 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7592 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 947 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7598 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 949 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7604 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 954 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7610 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 955 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7617 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 961 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7623 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 965 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7630 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 967 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7637 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 969 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 974 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 976 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 979 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 981 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 982 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 987 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7683 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 994 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1002 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7699 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1005 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7706 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1011 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 7712 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1012 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval)); }
#line 7718 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1017 "hphp.y" /* yacc.c:1646  */
    {
                                         _p->onUsing((yyval), (yyvsp[-2]), true, (yyvsp[-1]), nullptr);
                                       }
#line 7726 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1024 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7732 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1025 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7738 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1030 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 7744 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1031 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval), (yyval), (yyvsp[0])); }
#line 7751 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1036 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7757 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1037 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7763 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7769 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1039 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7775 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1042 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7781 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1046 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7787 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1051 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7793 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1052 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7800 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1054 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1058 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7815 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1061 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7823 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1065 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7830 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1067 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7838 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1070 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7845 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1072 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7853 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1075 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7859 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7865 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1077 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7871 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1078 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7877 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1079 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7883 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1080 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1081 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1082 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7901 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7907 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7913 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1085 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7925 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1087 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7931 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1088 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7937 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1089 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7944 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1093 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7951 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1095 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1100 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7966 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1102 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 7974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1106 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 7982 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1115 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7988 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1116 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 7994 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1119 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false); }
#line 8000 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1120 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 8006 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1122 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8014 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1126 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8022 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1130 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8030 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1134 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8038 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1138 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8046 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1142 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8054 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1147 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8062 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1150 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 8068 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1151 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 8077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1155 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8083 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1156 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8089 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1157 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8095 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1158 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8101 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1159 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8107 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1160 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8113 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1161 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 8119 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1162 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8125 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1163 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8131 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1164 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 8137 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1165 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8143 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1166 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 8153 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1188 "hphp.y" /* yacc.c:1646  */
    { _p->pushLabelScope(LS::Using);
                                         _p->onNewLabelScope(false);
                                         (yyval) = (yyvsp[-1]); }
#line 8161 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1194 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1; }
#line 8167 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1195 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8173 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1204 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), nullptr, (yyvsp[-2]));
                                         _p->onExprListElem((yyval), &(yyval), (yyvsp[0])); }
#line 8180 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1206 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0])); }
#line 8186 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1216 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8192 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1217 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8198 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1221 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false); }
#line 8204 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1222 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8210 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1231 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 8216 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1232 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8222 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1236 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::Finally);}
#line 8229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1238 "hphp.y" /* yacc.c:1646  */
    { _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->popLabelScope();
                                         _p->onCompleteLabelScope(false);}
#line 8237 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1244 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8243 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1245 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8249 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1249 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 8255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1250 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1254 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 8267 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1260 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8276 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1267 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8286 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1275 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1282 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8305 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1290 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8314 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1296 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1305 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1309 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 8337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1313 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8344 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1317 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 8350 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1323 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8357 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1326 "hphp.y" /* yacc.c:1646  */
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
#line 8375 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1341 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8382 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1344 "hphp.y" /* yacc.c:1646  */
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
#line 8400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1358 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8407 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1361 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1366 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8422 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1369 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1375 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 8436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1378 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 8442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1382 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8449 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1385 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1393 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8467 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1396 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8478 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1404 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8484 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1405 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 8491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1409 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8497 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1412 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8503 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1415 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 8509 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1416 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 8515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1417 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 8523 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1420 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 8529 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1421 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 8535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1425 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8541 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1426 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8547 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1429 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8553 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1430 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1433 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8565 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1434 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8571 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1437 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8577 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1439 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8583 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1442 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1444 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1448 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8601 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1449 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1452 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8619 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1454 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8625 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1458 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8631 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1460 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8637 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1463 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8643 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1465 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8649 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1468 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8655 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1470 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8661 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1473 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8667 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1475 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8673 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1479 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8679 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1481 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8686 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8698 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8704 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8710 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1494 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8716 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1496 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8722 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1497 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1500 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8734 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1501 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1506 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1507 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1512 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1513 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8764 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1516 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1517 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1520 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1521 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8788 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1529 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1535 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1541 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8810 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1545 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8816 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1549 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8823 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1554 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8830 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1559 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8838 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1562 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8844 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1568 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8852 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1573 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8860 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1578 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1584 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8876 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1590 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8884 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1596 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1602 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8900 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1608 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8908 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1615 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8916 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1622 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8924 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1631 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8931 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1636 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8938 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1641 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8946 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1645 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8952 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1648 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1652 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 8966 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1656 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 8974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1659 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8980 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1664 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8988 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1668 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8996 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1672 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9004 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1677 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9012 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1682 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9020 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1687 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9028 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1692 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9036 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1697 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9044 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1703 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9052 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1709 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9060 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1715 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 9066 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1716 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 9072 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1717 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 9078 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1722 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9084 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1723 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9090 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1726 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,false);}
#line 9097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1728 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::InOut,false);}
#line 9104 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1730 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::Ref,false);}
#line 9111 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1732 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,true);}
#line 9118 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1735 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::In,false);}
#line 9125 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1738 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::In,true);}
#line 9132 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1741 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::Ref,false);}
#line 9139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1744 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::InOut,false);}
#line 9146 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1749 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9152 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1750 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 9158 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1753 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9164 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1754 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 9170 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1755 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 9176 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1759 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9182 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1761 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9188 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1762 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 9194 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1763 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9200 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1768 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9206 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1769 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9212 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1772 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9219 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1777 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9225 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1783 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9231 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1784 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9237 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1787 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 9243 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1788 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 9250 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1791 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 9256 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1792 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 9263 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1794 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9270 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1797 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 9277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1799 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1802 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9291 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1809 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1817 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9309 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1824 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9319 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1830 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 9325 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1832 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1834 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1836 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 9343 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1838 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 9349 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1839 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 9356 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1842 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 9362 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1845 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9368 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9374 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1847 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 9380 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1853 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 9386 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1858 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 9393 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1861 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 9401 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1868 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 9407 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1869 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 9414 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1874 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 9421 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1877 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 9427 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1884 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 9434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1886 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 9440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1890 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9446 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1895 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9452 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1897 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9458 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1899 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9464 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1900 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 9475 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1906 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 9481 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1908 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 9487 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1909 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 9493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1913 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval),nullptr,(yyvsp[0])); }
#line 9499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1915 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval),&(yyvsp[-2]),(yyvsp[0])); }
#line 9505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1920 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9511 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1923 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9517 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1924 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 9523 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1928 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 9529 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1929 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 9535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1933 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 9542 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1936 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 9549 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1941 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 9556 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1946 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 9562 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1947 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 9569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1949 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 9575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1953 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 9581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1954 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 9587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1955 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 9593 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 9599 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1960 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9605 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1961 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 9611 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 9617 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 9623 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 9629 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1966 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 9635 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1968 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 9641 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1972 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9649 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1975 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9655 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9661 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1980 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9667 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1981 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9673 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1985 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9679 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9685 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1989 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9691 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1990 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9697 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1993 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9703 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9709 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1997 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9715 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9721 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 2002 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9727 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9745 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 2006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9751 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9757 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 2008 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9763 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 2012 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9769 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9775 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 2016 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9781 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9787 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9793 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 2022 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9799 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 2024 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9805 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 2025 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9811 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 2026 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9817 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 2030 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9823 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 2032 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9829 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 2036 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9835 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 2038 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9841 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 2042 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9849 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 2046 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9856 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 2050 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9862 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 2054 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 2058 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9874 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 2060 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9880 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 2061 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9886 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 2062 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 2063 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9898 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 2064 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9904 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 2067 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9910 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 2068 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9916 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 2072 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9922 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 2073 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9928 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 2077 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9934 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 2078 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9940 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 2079 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9946 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 2080 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9952 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 2084 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9958 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 2089 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9964 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 2093 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 9970 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 2097 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9976 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 2101 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 9982 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 2105 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9988 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 2110 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9994 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 2114 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10000 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 2118 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10006 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 2119 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10012 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 2120 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10018 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 2121 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10024 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 2122 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10030 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 2126 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10036 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 2131 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 10042 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 2132 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 10048 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 2133 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 10054 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 2136 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 10060 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 2137 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 10066 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2138 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 10072 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2139 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 10078 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2140 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 10084 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2141 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 10090 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2142 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 10096 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2143 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 10102 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2144 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 10108 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2145 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 10114 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2146 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 10120 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2147 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 10126 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 10132 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2149 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 10138 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2150 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 10144 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2151 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 10150 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2152 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 10156 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2153 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 10162 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2154 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 10168 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2155 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 10174 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2156 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 10180 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2157 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 10186 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2158 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 10192 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2159 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 10198 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2160 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 10204 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2161 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 10210 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2162 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 10216 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2163 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 10222 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2164 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 10228 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2165 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 10234 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2166 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 10240 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2167 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 10246 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2168 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 10252 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2169 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 10258 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2170 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 10264 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2171 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 10270 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2172 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 10276 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2173 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 10282 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2174 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 10288 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2175 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 10294 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2176 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 10300 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2177 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 10306 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2178 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 10312 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2179 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 10318 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2180 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 10325 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2182 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 10331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2183 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 10338 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2185 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 10344 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2187 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 10350 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2188 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10356 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2189 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 10362 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2190 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 10368 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2191 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 10374 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2192 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10380 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2193 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 10386 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2194 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 10392 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2195 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 10398 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2196 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 10404 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2197 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 10410 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2198 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 10416 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2199 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 10422 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2200 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 10428 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2201 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 10434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2202 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2203 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10446 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2204 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10452 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2205 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10458 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2206 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10464 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2207 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10470 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10476 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2209 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10482 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2210 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10488 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2211 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 10494 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2212 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 10500 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2213 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10506 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2220 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 10512 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2221 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10518 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2226 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2232 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2241 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10548 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2247 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10560 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2258 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,NULL,NULL);}
#line 10574 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2267 "hphp.y" /* yacc.c:1646  */
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
#line 10589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2278 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10599 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2286 "hphp.y" /* yacc.c:1646  */
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
#line 10614 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2297 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10624 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2303 "hphp.y" /* yacc.c:1646  */
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
#line 10641 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2315 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,NULL,NULL);}
#line 10655 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2324 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2332 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10678 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2340 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10691 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2351 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10697 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2352 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10703 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2354 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10709 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2358 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10716 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2360 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10722 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2367 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2370 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10734 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2377 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2380 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2385 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2386 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2391 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10764 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2392 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2396 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_DARRAY);}
#line 10776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2400 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2401 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10788 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2406 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10794 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2407 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10800 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2412 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10806 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2413 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10812 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2418 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2419 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10824 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2425 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10830 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2427 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10836 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2432 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10842 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2433 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10848 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2439 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10854 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2441 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10860 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2445 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10866 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2449 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10872 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2453 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10878 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2457 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10884 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2461 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10890 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2465 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10896 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2469 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10902 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2473 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10908 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2477 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10914 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2481 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10920 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2485 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10926 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2489 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2493 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10938 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2497 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10944 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2501 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10950 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2505 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10956 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2509 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10962 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2513 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2518 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2519 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10980 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2524 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10986 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2525 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10992 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2530 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10998 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2531 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11004 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2536 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11012 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2543 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11020 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2550 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11026 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2552 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11032 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2556 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11038 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2557 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11044 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2558 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11050 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2559 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11056 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2560 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11062 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2561 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11068 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2562 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11074 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2563 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11080 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2564 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11086 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2565 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 11093 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2567 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2568 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11105 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2572 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11111 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 11117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2574 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 11123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2575 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 11129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2582 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 11135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2585 "hphp.y" /* yacc.c:1646  */
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
#line 11153 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2600 "hphp.y" /* yacc.c:1646  */
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
#line 11171 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2615 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 11177 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2616 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 11183 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2621 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11189 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2622 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11195 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2625 "hphp.y" /* yacc.c:1646  */
    { _p->onOptExprListElem((yyval), &(yyvsp[-1]), (yyvsp[0])); }
#line 11201 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2626 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11207 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2629 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11214 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2633 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11222 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11228 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 11240 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11246 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2647 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11252 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2651 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11258 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2653 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 11264 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2655 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 11270 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11276 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2660 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11282 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2661 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11288 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2662 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11294 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11300 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11306 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2665 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11312 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2666 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11318 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2667 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11330 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2669 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11336 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11342 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2671 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11348 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2672 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11354 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2673 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11360 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2674 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11366 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2675 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11372 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2676 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11378 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2677 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11384 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2678 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11390 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2679 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11396 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2680 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11402 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2681 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11408 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2682 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11414 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2683 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11420 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2684 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11426 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2685 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11432 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11438 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2687 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11444 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2688 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11450 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2689 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11456 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2690 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11462 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2691 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11468 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11474 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2693 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11480 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2694 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11486 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2695 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11492 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2696 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11498 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2697 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11504 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2698 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11510 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2699 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11516 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2700 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11522 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2701 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11528 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2702 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11534 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2703 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11540 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2704 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11546 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2705 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11552 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2706 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11558 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2707 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11564 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2708 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11570 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2709 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11576 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2710 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11582 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2711 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11588 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2713 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11600 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2714 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11606 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2715 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11612 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2716 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11618 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2717 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11624 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11630 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11636 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11642 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11648 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2722 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11654 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11660 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2724 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11666 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11672 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2726 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11678 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2727 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11684 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2728 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11690 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11696 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11702 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2731 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11708 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2732 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11714 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2733 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11720 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11726 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2735 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11732 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11738 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2737 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11744 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11750 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11756 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11762 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11768 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2746 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11774 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2750 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11780 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2751 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11786 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2755 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11792 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2756 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11798 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2757 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11804 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2758 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11811 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2760 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2764 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11824 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2773 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11830 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2776 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11836 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2777 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11843 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2779 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2789 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11856 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2793 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11862 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2794 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2795 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11874 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2799 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11880 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2800 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11886 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2801 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2805 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11898 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2806 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11904 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2807 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11910 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2811 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11916 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2812 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11922 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2816 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11928 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2817 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11934 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2818 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11940 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2819 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2821 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2822 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2823 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 11965 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2824 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 11971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2825 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 11977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2826 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 11983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2827 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 11989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2828 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 11995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2829 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 12001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2832 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2834 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2839 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2841 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2842 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2844 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_DARRAY);}
#line 12043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2845 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2846 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12055 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2847 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12061 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2848 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12067 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2850 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12079 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2851 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2852 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2853 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2855 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 12103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2857 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 12109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2859 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 12115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2861 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 12121 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2863 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 12127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2864 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 12133 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2865 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 12139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2866 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 12145 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 12151 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2868 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 12157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2869 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 12163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2870 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 12169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2871 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 12175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2872 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 12181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2873 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 12187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2874 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 12193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2875 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 12199 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2876 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 12205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2877 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 12211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2878 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2879 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12223 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2881 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 12229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2883 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 12235 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2885 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 12241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2887 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 12247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2888 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 12253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2890 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 12260 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2892 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 12266 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2895 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 12273 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2899 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 12279 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2902 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 12285 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2903 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 12291 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2907 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12297 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2908 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12303 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2914 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12309 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2920 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12315 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2921 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12321 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2925 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12327 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2926 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12333 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2927 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2928 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2929 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 12351 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2930 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 12357 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2932 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 12364 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2937 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12370 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2938 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12376 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2942 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12382 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2943 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12388 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2946 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 12394 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2947 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2953 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2955 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2957 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2958 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2962 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2963 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2964 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2967 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2969 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2972 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 12460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2973 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 12466 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2974 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 12472 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2975 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12478 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2979 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 12485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2982 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 12493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2989 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2990 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2993 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 12513 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2996 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12519 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2997 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12525 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 2998 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12531 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 3000 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12537 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 3001 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12543 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 3003 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_DARRAY);}
#line 12549 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 3004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12555 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 3005 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12561 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 3006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12567 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 3007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12573 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 3008 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12579 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 3009 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12585 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 3014 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12591 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 3015 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12597 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 3020 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12603 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 3021 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12609 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 3026 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12615 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 3028 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12621 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 3030 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12627 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3031 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12633 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3035 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12639 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3036 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12645 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3041 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12651 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3042 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12657 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3047 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12663 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3050 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12669 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3055 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12675 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3056 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12681 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3059 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12687 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3060 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 12694 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3067 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 12700 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3069 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 12706 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3072 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 12712 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3074 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12718 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3077 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12724 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3080 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12730 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3081 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12736 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3085 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12742 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3086 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12748 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3090 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12754 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3091 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12760 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3092 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12766 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3096 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12772 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3101 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12778 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3106 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12784 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3107 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12790 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3111 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12796 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3116 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3121 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3122 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12814 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3127 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12820 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3128 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12826 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3130 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12832 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3135 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12838 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3137 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12844 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3143 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12858 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3154 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12872 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3169 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12886 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3181 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12900 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3193 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12906 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3194 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12912 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3195 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12918 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3196 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12924 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3197 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12930 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3198 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12936 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3200 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12950 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3217 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12956 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3219 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12962 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3221 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3222 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3226 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12980 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3230 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12986 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3231 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12992 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3232 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12998 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3233 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13004 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3241 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13018 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3250 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13024 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3252 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13030 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3256 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13036 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3261 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13042 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3262 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13048 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3263 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13054 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3264 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13060 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3265 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13066 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3266 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13072 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3267 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13078 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3269 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13084 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3271 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13090 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3275 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13096 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3279 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13102 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3280 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13108 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3286 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13114 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3290 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13120 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3294 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13126 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3301 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 13132 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3310 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 13138 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3314 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 13144 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3318 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13150 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3327 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13156 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3328 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13162 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3329 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13168 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3333 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13174 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3334 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 13180 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3335 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 13186 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3337 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]) = 1; _p->onIndirectRef((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 13192 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3342 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13198 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3343 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13204 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3354 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13210 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3355 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13216 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3356 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13222 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3359 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13236 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3370 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13242 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3371 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13248 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3375 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13254 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3376 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13260 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3379 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13274 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3388 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13280 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3392 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 13286 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3393 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 13292 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3395 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 13298 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3396 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 13304 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3397 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 13310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3398 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 13316 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3403 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13322 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3404 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13328 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3408 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13334 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3409 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13340 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3410 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13346 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3411 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13352 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3414 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13358 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3416 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 13364 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3417 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13370 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3418 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 13376 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3423 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13382 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3424 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13388 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3428 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13394 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3429 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3430 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3431 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3436 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3437 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3442 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3444 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3446 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3447 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3451 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 13454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3453 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 13460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3454 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 13466 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1030:
#line 3456 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 13473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1031:
#line 3461 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1032:
#line 3463 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1033:
#line 3465 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 13499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1034:
#line 3475 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 13505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1035:
#line 3477 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 13511 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1036:
#line 3478 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13517 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3481 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 13523 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1038:
#line 3482 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 13529 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1039:
#line 3483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 13535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3487 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 13541 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3488 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 13547 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3489 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13553 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3490 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3491 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13565 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3492 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13571 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3493 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 13577 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3494 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 13583 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3495 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 13589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3496 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 13595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3497 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 13601 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3501 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 13607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3502 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 13613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3507 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13619 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3509 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13625 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3523 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13633 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3528 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 13641 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3532 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13649 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3537 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 13657 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3543 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13663 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3544 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13669 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3548 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13675 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3549 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13681 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3555 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13687 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3559 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 13693 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3565 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13699 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3569 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 13706 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3576 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13712 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3577 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13718 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3581 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 13726 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3584 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 13733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3590 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3594 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13747 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3597 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13755 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3600 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-3]); }
#line 13762 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1077:
#line 3602 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13769 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1078:
#line 3604 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1079:
#line 3606 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1080:
#line 3611 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]); }
#line 13788 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1081:
#line 3612 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13794 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1082:
#line 3613 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13800 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1083:
#line 3614 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13806 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1090:
#line 3635 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13812 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1091:
#line 3636 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1094:
#line 3645 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13824 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1097:
#line 3656 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13830 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1098:
#line 3658 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13836 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1099:
#line 3662 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13842 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1100:
#line 3665 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13848 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1101:
#line 3669 "hphp.y" /* yacc.c:1646  */
    {}
#line 13854 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1102:
#line 3670 "hphp.y" /* yacc.c:1646  */
    {}
#line 13860 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1103:
#line 3671 "hphp.y" /* yacc.c:1646  */
    {}
#line 13866 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1104:
#line 3677 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13873 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1105:
#line 3682 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13883 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1106:
#line 3691 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1107:
#line 3697 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13898 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1108:
#line 3705 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13904 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1109:
#line 3706 "hphp.y" /* yacc.c:1646  */
    { }
#line 13910 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1110:
#line 3712 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13916 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1111:
#line 3714 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13922 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1112:
#line 3715 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1113:
#line 3720 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13939 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1114:
#line 3726 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("HH\\darray"); }
#line 13946 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1115:
#line 3731 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13952 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1116:
#line 3736 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13960 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1117:
#line 3740 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13966 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1118:
#line 3745 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 13972 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1119:
#line 3747 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 13978 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1120:
#line 3753 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 13985 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1121:
#line 3755 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 13993 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1122:
#line 3758 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13999 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1123:
#line 3759 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1124:
#line 3762 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1125:
#line 3765 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1126:
#line 3768 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 14029 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1127:
#line 3771 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14036 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1128:
#line 3773 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 14045 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1129:
#line 3779 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 14054 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1130:
#line 3785 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("HH\\varray");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 14064 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1131:
#line 3793 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14070 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1132:
#line 3794 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 14076 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;


#line 14080 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 3797 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}
