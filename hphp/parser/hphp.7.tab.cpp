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
    _p->onArrayPair(out, list, &name, &decl, 0);
    if (list) {
      out.setText(list->text());
    } else {
      out.setText("");
    }
  }
}

static void xhp_attribute_property_stmt(Parser *_p, Token &out) {
  // private static darray $__xhpAttributeDeclarationCache = null;
  Token modifiers;
  {
    Token m;
    Token m1; m1 = T_PRIVATE; _p->onMemberModifier(m, NULL, m1);
    Token m2; m2 = T_STATIC;  _p->onMemberModifier(modifiers, &m, m2);
  }
  Token var;      var.set(T_VARIABLE, "__xhpAttributeDeclarationCache");
  Token null;     scalar_null(_p, null);
  Token cvout;    _p->onClassVariable(cvout, 0, var, &null);
  _p->onClassVariableStart(out, &modifiers, cvout, NULL, NULL);
}

static void xhp_attribute_method_stmt(Parser *_p, Token &out, Token &attributes) {
  Token modifiers;
  Token fname; fname.setText("__xhpAttributeDeclaration");
  {
    Token m;
    Token m1; m1.setNum(T_PROTECTED); _p->onMemberModifier(m, NULL, m1);
    Token m2; m2.setNum(T_STATIC);    _p->onMemberModifier(modifiers, &m, m2);
  }
  _p->pushFuncLocation();
  _p->onMethodStart(fname, modifiers);

  Token dummy;

  std::vector<std::string> classes;
  folly::split(':', attributes.text(), classes, true);
  Token arrAttributes; _p->onDArray(arrAttributes, attributes);

  auto _cache = [&]() {
    // self::__xhpAttributeDeclarationCache
    const char *cacheName = "__xhpAttributeDeclarationCache";
    Token self;  self.set(T_STRING, "self");
    Token cls;   _p->onName(cls, self, Parser::StringName);
    Token var;   var.set(T_VARIABLE, cacheName);
    Token sv;    _p->onSimpleVariable(sv, var);
    Token out;
    _p->onStaticMember(out, cls, sv);
    return out;
  };

  auto _r = [&]() {
    // $r
    Token r; r.set(T_VARIABLE, "r");
    Token out; _p->onSimpleVariable(out, r);
    return out;
  };

  auto _assign_stmt = [&](Token &dest, Token &src) {
    // [dest] = [src];
    Token assign; _p->onAssign(assign, dest, src, 0);
    Token stmt; _p->onExpStatement(stmt, assign);
    return stmt;
  };

  auto _start_statements = [&]() {
    Token stmts;
    _p->onStatementListStart(stmts);
    return stmts;
  };

  auto _add_statement = [&](Token &stmts, Token &new_stmt) {
    Token stmts_in = stmts;
    _p->addStatement(stmts, stmts_in, new_stmt);
  };

  auto _invoke = [&](std::string clsName) {
    // Invokes clsName::__xhpAttributeDeclaration()
    Token dummy;
    Token name;    name.set(T_STRING, clsName);
    Token cls;     _p->onName(cls, name, Parser::StringName);
    Token fname;   fname.setText("__xhpAttributeDeclaration");
    Token out;
    _p->onCall(out, 0, fname, dummy, &cls);
    return out;
  };

  auto _add_call_param_first = [&](Token &newParam) {
    Token out;
    _p->onCallParam(out, NULL, newParam, ParamMode::In, false);
    return out;
  };

  auto _add_call_param = [&](Token &params, Token &newParam) {
    Token params_in = params;
    _p->onCallParam(params, &params_in, newParam, ParamMode::In, false);
  };

  Token body = _start_statements();
  {
    // $r = self::$__xhpAttributeDeclarationCache;
    Token src = _cache();
    Token dest = _r();
    Token stmt = _assign_stmt(dest, src);
    _add_statement(body, stmt);
  }

  {
    // if ($r === null) {
    //   self::$__xhpAttributeDeclarationCache =
    //     __SystemLib\\merge_xhp_attr_declarations(
    //          parent::__xhpAttributeDeclaration(),
    //          firstInheritedClass::__xhpAttributeDeclaration(),
    //          ...
    //          lastInheritedClass::__xhpAttributeDeclaration(),
    //          attributes
    //        );
    //   $r = self::$__xhpAttributeDeclarationCache;
    // }
    Token blockbody = _start_statements();
    {
      Token param1 = _invoke("parent");
      Token params = _add_call_param_first(param1);
      for (unsigned int i = 0; i < classes.size(); i++) {
        Token param = _invoke(classes[i]);
        _add_call_param(params, param);
      }
      _add_call_param(params, arrAttributes);

      Token name;    name.set(T_STRING, "__SystemLib\\merge_xhp_attr_declarations");
                   name = name.num() | 2; // WTH???
      Token call;    _p->onCall(call, 0, name, params, NULL);
      Token dest = _cache();
      Token stmt = _assign_stmt(dest, call);
      _add_statement(blockbody, stmt);
    }
    {
      Token dest = _r();
      Token src = _cache();
      Token stmt = _assign_stmt(dest, src);
      _add_statement(blockbody, stmt);
    }
    Token block;   _p->onBlock(block, blockbody);

    {
      Token r = _r();
      Token null;    scalar_null(_p, null);
      Token cond;    BEXP(cond, r, null, T_IS_IDENTICAL);
      Token dummy1, dummy2;
      Token sif;     _p->onIf(sif, cond, block, dummy1, dummy2);
      _add_statement(body, sif);
    }
  }

  {
    // return $r;
    Token r = _r();
    Token ret; _p->onReturn(ret, &r);
    _add_statement(body, ret);
  }
  Token stmt;
  {
    _p->finishStatement(stmt, body);
    stmt = 1;
  }
  {
    Token params, ret, ref; ref = 0;
    _p->onMethod(out, modifiers, ret, ref, fname, params, stmt, nullptr, false);
  }
}

static void xhp_attribute_stmts(Parser *_p, Token &out, Token &stmts, Token &attr) {
    Token stmts1;
    {
      Token stmt;
      xhp_attribute_property_stmt(_p, stmt);
      _p->onClassStatement(stmts1, stmts, stmt);
    }
    {
      Token stmt;
      xhp_attribute_method_stmt(_p, stmt, attr);
      _p->onClassStatement(out, stmts1, stmt);
    }
}

static void xhp_collect_attributes(Parser *_p, Token &out, Token &stmts) {
  Token *attr = _p->xhpGetAttributes();
  if (attr) {
    xhp_attribute_stmts(_p, out, stmts, *attr);
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
    // return categories;
    Token arr;     _p->onDArray(arr, categories);
    Token ret;     _p->onReturn(ret, &arr);
    _p->addStatement(stmts1, stmts0, ret);
  }
  Token stmt;
  {
    _p->finishStatement(stmt, stmts1);
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
    // return children
    Token arr;
    if (std::equal(children.num(), 2)) {
      arr = children;
    } else if (children.num() >= 0) {
      scalar_num(_p, arr, children.num());
    } else {
      HPHP_PARSER_ERROR("XHP: XHP unknown children declaration", _p);
    }
    Token ret;     _p->onReturn(ret, &arr);
    _p->addStatement(stmts1, stmts0, ret);
  }
  Token stmt;
  {
    _p->finishStatement(stmt, stmts1);
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

#line 723 "hphp.7.tab.cpp" /* yacc.c:339  */

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

#line 967 "hphp.7.tab.cpp" /* yacc.c:358  */

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
#define YYLAST   20201

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  209
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  313
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1131
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  2105

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
       0,   821,   821,   821,   830,   832,   835,   836,   837,   838,
     839,   840,   841,   844,   846,   846,   848,   848,   850,   853,
     858,   863,   866,   869,   873,   877,   881,   885,   889,   894,
     895,   896,   897,   898,   899,   900,   901,   902,   903,   904,
     905,   906,   910,   911,   912,   913,   914,   915,   916,   917,
     918,   919,   920,   921,   922,   923,   924,   925,   926,   927,
     928,   929,   930,   931,   932,   933,   934,   935,   936,   937,
     938,   939,   940,   941,   942,   943,   944,   945,   946,   947,
     948,   949,   950,   951,   952,   953,   954,   955,   956,   957,
     958,   959,   960,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   978,   979,   983,
     984,   988,   989,   994,   996,  1001,  1006,  1007,  1008,  1010,
    1015,  1017,  1022,  1027,  1029,  1031,  1036,  1037,  1041,  1042,
    1044,  1048,  1055,  1062,  1066,  1072,  1074,  1078,  1079,  1085,
    1087,  1091,  1093,  1098,  1099,  1100,  1101,  1104,  1105,  1109,
    1114,  1114,  1120,  1120,  1127,  1126,  1132,  1132,  1137,  1138,
    1139,  1140,  1141,  1142,  1143,  1144,  1145,  1146,  1147,  1148,
    1149,  1150,  1151,  1155,  1153,  1162,  1160,  1167,  1177,  1171,
    1181,  1179,  1183,  1187,  1191,  1195,  1199,  1203,  1207,  1212,
    1213,  1217,  1218,  1219,  1220,  1221,  1222,  1223,  1224,  1225,
    1226,  1227,  1228,  1250,  1256,  1257,  1266,  1268,  1272,  1273,
    1274,  1278,  1279,  1283,  1283,  1288,  1294,  1298,  1298,  1306,
    1307,  1311,  1312,  1316,  1322,  1320,  1337,  1334,  1352,  1349,
    1367,  1366,  1375,  1373,  1385,  1384,  1403,  1401,  1420,  1419,
    1428,  1426,  1437,  1437,  1444,  1443,  1455,  1453,  1466,  1467,
    1471,  1474,  1477,  1478,  1479,  1482,  1483,  1486,  1488,  1491,
    1492,  1495,  1496,  1499,  1500,  1504,  1505,  1510,  1511,  1514,
    1515,  1516,  1520,  1521,  1525,  1526,  1530,  1531,  1535,  1536,
    1541,  1542,  1548,  1549,  1550,  1551,  1554,  1557,  1559,  1562,
    1563,  1567,  1569,  1572,  1575,  1578,  1579,  1582,  1583,  1587,
    1593,  1599,  1606,  1608,  1613,  1618,  1624,  1628,  1633,  1638,
    1643,  1649,  1655,  1661,  1667,  1673,  1680,  1690,  1695,  1700,
    1706,  1708,  1712,  1716,  1721,  1725,  1729,  1733,  1737,  1742,
    1747,  1752,  1757,  1762,  1768,  1777,  1778,  1779,  1783,  1785,
    1788,  1790,  1792,  1794,  1796,  1799,  1802,  1805,  1811,  1812,
    1815,  1816,  1817,  1821,  1822,  1824,  1825,  1829,  1831,  1834,
    1838,  1844,  1846,  1849,  1852,  1856,  1860,  1865,  1867,  1870,
    1873,  1871,  1888,  1885,  1900,  1902,  1904,  1906,  1908,  1910,
    1912,  1916,  1917,  1918,  1921,  1927,  1931,  1937,  1940,  1945,
    1947,  1952,  1957,  1961,  1962,  1966,  1967,  1969,  1971,  1977,
    1978,  1980,  1984,  1985,  1990,  1994,  1995,  1999,  2000,  2004,
    2006,  2012,  2017,  2018,  2020,  2024,  2025,  2026,  2027,  2031,
    2032,  2033,  2034,  2035,  2036,  2038,  2043,  2046,  2047,  2051,
    2052,  2056,  2057,  2060,  2061,  2064,  2065,  2068,  2069,  2073,
    2074,  2075,  2076,  2077,  2078,  2079,  2083,  2084,  2087,  2088,
    2089,  2092,  2094,  2096,  2097,  2100,  2102,  2105,  2111,  2113,
    2117,  2121,  2125,  2130,  2134,  2135,  2137,  2138,  2139,  2140,
    2143,  2144,  2148,  2149,  2153,  2154,  2155,  2156,  2160,  2164,
    2169,  2173,  2177,  2181,  2185,  2190,  2194,  2195,  2196,  2197,
    2198,  2202,  2206,  2208,  2209,  2210,  2213,  2214,  2215,  2216,
    2217,  2218,  2219,  2220,  2221,  2222,  2223,  2224,  2225,  2226,
    2227,  2228,  2229,  2230,  2231,  2232,  2233,  2234,  2235,  2236,
    2237,  2238,  2239,  2240,  2241,  2242,  2243,  2244,  2245,  2246,
    2247,  2248,  2249,  2250,  2251,  2252,  2253,  2254,  2255,  2256,
    2258,  2259,  2261,  2262,  2264,  2265,  2266,  2267,  2268,  2269,
    2270,  2271,  2272,  2273,  2274,  2275,  2276,  2277,  2278,  2279,
    2280,  2281,  2282,  2283,  2284,  2285,  2286,  2287,  2288,  2289,
    2293,  2297,  2302,  2301,  2317,  2315,  2334,  2333,  2354,  2353,
    2373,  2372,  2391,  2391,  2408,  2408,  2427,  2428,  2429,  2434,
    2436,  2440,  2444,  2450,  2454,  2460,  2462,  2466,  2468,  2472,
    2476,  2477,  2478,  2482,  2484,  2488,  2490,  2494,  2496,  2500,
    2503,  2508,  2510,  2514,  2517,  2522,  2526,  2530,  2534,  2538,
    2542,  2546,  2550,  2554,  2558,  2562,  2566,  2570,  2574,  2578,
    2582,  2586,  2590,  2594,  2596,  2600,  2602,  2606,  2608,  2612,
    2619,  2626,  2628,  2633,  2634,  2635,  2636,  2637,  2638,  2639,
    2640,  2641,  2642,  2644,  2645,  2649,  2650,  2651,  2652,  2656,
    2662,  2675,  2692,  2693,  2696,  2697,  2699,  2704,  2705,  2708,
    2712,  2715,  2718,  2725,  2726,  2730,  2731,  2733,  2738,  2739,
    2740,  2741,  2742,  2743,  2744,  2745,  2746,  2747,  2748,  2749,
    2750,  2751,  2752,  2753,  2754,  2755,  2756,  2757,  2758,  2759,
    2760,  2761,  2762,  2763,  2764,  2765,  2766,  2767,  2768,  2769,
    2770,  2771,  2772,  2773,  2774,  2775,  2776,  2777,  2778,  2779,
    2780,  2781,  2782,  2783,  2784,  2785,  2786,  2787,  2788,  2789,
    2790,  2791,  2792,  2793,  2794,  2795,  2796,  2797,  2798,  2799,
    2800,  2801,  2802,  2803,  2804,  2805,  2806,  2807,  2808,  2809,
    2810,  2811,  2812,  2813,  2814,  2815,  2816,  2817,  2818,  2819,
    2820,  2824,  2829,  2830,  2834,  2835,  2836,  2837,  2839,  2843,
    2844,  2855,  2856,  2858,  2860,  2872,  2873,  2874,  2878,  2879,
    2880,  2884,  2885,  2886,  2889,  2891,  2895,  2896,  2897,  2898,
    2900,  2901,  2902,  2903,  2904,  2905,  2906,  2907,  2908,  2909,
    2912,  2917,  2918,  2919,  2921,  2922,  2924,  2925,  2926,  2927,
    2928,  2929,  2930,  2931,  2932,  2933,  2935,  2937,  2939,  2941,
    2943,  2944,  2945,  2946,  2947,  2948,  2949,  2950,  2951,  2952,
    2953,  2954,  2955,  2956,  2957,  2958,  2959,  2961,  2963,  2965,
    2967,  2968,  2971,  2972,  2976,  2980,  2982,  2986,  2987,  2991,
    2997,  3000,  3004,  3005,  3006,  3007,  3008,  3009,  3010,  3015,
    3017,  3021,  3022,  3025,  3026,  3030,  3033,  3035,  3037,  3041,
    3042,  3043,  3044,  3047,  3051,  3052,  3053,  3054,  3058,  3060,
    3067,  3068,  3069,  3070,  3075,  3076,  3077,  3078,  3080,  3081,
    3083,  3084,  3085,  3086,  3087,  3088,  3092,  3094,  3098,  3100,
    3103,  3106,  3108,  3110,  3113,  3115,  3119,  3121,  3124,  3127,
    3133,  3135,  3138,  3139,  3144,  3147,  3151,  3151,  3156,  3159,
    3160,  3164,  3165,  3169,  3170,  3171,  3175,  3180,  3185,  3186,
    3190,  3195,  3200,  3201,  3205,  3207,  3208,  3213,  3215,  3220,
    3231,  3245,  3257,  3272,  3273,  3274,  3275,  3276,  3277,  3278,
    3288,  3297,  3299,  3301,  3305,  3309,  3310,  3311,  3312,  3313,
    3329,  3330,  3333,  3340,  3341,  3342,  3343,  3344,  3345,  3346,
    3348,  3349,  3351,  3353,  3358,  3362,  3363,  3367,  3370,  3374,
    3381,  3385,  3394,  3401,  3409,  3411,  3412,  3416,  3417,  3418,
    3420,  3425,  3426,  3437,  3438,  3439,  3440,  3451,  3454,  3457,
    3458,  3459,  3460,  3471,  3476,  3477,  3478,  3479,  3481,  3483,
    3485,  3486,  3490,  3492,  3496,  3498,  3501,  3503,  3504,  3505,
    3509,  3511,  3514,  3517,  3519,  3521,  3525,  3526,  3528,  3529,
    3535,  3536,  3538,  3548,  3550,  3552,  3555,  3556,  3557,  3561,
    3562,  3563,  3564,  3565,  3566,  3567,  3568,  3569,  3570,  3571,
    3575,  3576,  3580,  3582,  3590,  3592,  3596,  3600,  3605,  3609,
    3617,  3618,  3622,  3623,  3629,  3630,  3639,  3640,  3648,  3651,
    3655,  3658,  3663,  3668,  3671,  3674,  3676,  3678,  3680,  3684,
    3686,  3687,  3688,  3691,  3693,  3699,  3700,  3704,  3705,  3709,
    3710,  3714,  3715,  3718,  3723,  3724,  3728,  3731,  3733,  3737,
    3743,  3744,  3745,  3749,  3753,  3761,  3766,  3778,  3780,  3784,
    3787,  3789,  3794,  3799,  3805,  3808,  3813,  3818,  3820,  3827,
    3829,  3832,  3833,  3836,  3839,  3840,  3845,  3847,  3851,  3857,
    3867,  3868
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
  "array_pair_list", "collection_init", "non_empty_collection_init",
  "static_collection_init", "non_empty_static_collection_init",
  "encaps_list", "encaps_var", "encaps_var_offset", "internal_functions",
  "variable_list", "class_constant", "hh_opt_constraint",
  "hh_type_alias_statement", "hh_name_with_type", "hh_constname_with_type",
  "hh_name_with_typevar", "hh_name_no_semireserved_with_typevar",
  "hh_typeargs_opt", "hh_non_empty_type_list", "hh_type_list",
  "hh_non_empty_func_type_list", "hh_func_type_list",
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
     426,   427,   428,   429,   430,   431,   432,   433,   434,    40,
      41,    59,   123,   125,    93,    36,    96,    34,    39
};
# endif

#define YYPACT_NINF -1840

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1840)))

#define YYTABLE_NINF -1132

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1132)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1840,   192, -1840, -1840,  5643, 14953, 14953,     4, 14953, 14953,
   14953, 14953, 12554, 14953, -1840, 14953, 14953, 14953, 14953, 18203,
   18203, 14953, 14953, 14953, 14953, 14953, 14953, 14953, 14953, 12733,
   18938, 14953,     7,    13, -1840, -1840, -1840,   236, -1840,   412,
   -1840, -1840, -1840,   308, 14953, -1840,    13,   248,   326,   360,
   -1840,    13, 12912, 16412, 13091, -1840, 15971, 11417,   380, 14953,
   19187,    92,    86,   510,   292, -1840, -1840, -1840,   385,   416,
     454,   474, -1840, 16412,   481,   484,   527,   650,   665,   671,
     694, -1840, -1840, -1840, -1840, -1840, 14953,   601,  1659, -1840,
   -1840, 16412, -1840, -1840, -1840, -1840, 16412, -1840, 16412, -1840,
     444,   567,   582, 16412, 16412, -1840,   443, -1840, -1840, 13297,
   -1840, -1840,   502,   107,   627,   627, -1840,   767,   645,   655,
     606, -1840,    93, -1840,   620,   746,   790, -1840, -1840, -1840,
   -1840, 15353,   701, -1840,    76, -1840,   667,   672,   678,   681,
     698,   703,   719,   720, 17434, -1840, -1840, -1840, -1840, -1840,
     163,   858,   861,   863,   864,   869,   872, -1840,   874,   875,
   -1840,   211,   744, -1840,   785,    16, -1840,  1209,   257, -1840,
   -1840,  2504,    76,    76,   758,   170, -1840,   348,   392,   759,
     493, -1840, -1840,   889, -1840,   799, -1840, -1840,   796, -1840,
   14953, -1840,   790,   701, 19475,   165,  3584, 19475, 14953, 19475,
   19475, 19786, 19786,   768,  5560, 19475,   921, 16412,   902,   902,
     398,   902, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840,    90, 14953,   789, -1840, -1840,   812,   776,    63,   780,
      63,   902,   902,   902,   902,   902,   902,   902,   902, 18203,
   18376,    89,   799, -1840, 14953,   789, -1840,   818, -1840,   819,
     783, -1840,   134, -1840, -1840, -1840,    63,    76, -1840, 13476,
   -1840, -1840, 14953, 10181,   975,   108, 19475, 11211, -1840, 14953,
   14953, 16412, -1840, -1840, 17482,   786, -1840, 17531, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840,  4499, -1840,
    4499, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840,
     120,   119,   796, -1840, -1840, -1840, -1840,   792, -1840,  2821,
     121, -1840, -1840,   826,   978, -1840,   832, 16712, 14953, -1840,
     794,   795, 17579, -1840,    78, 17627, 15864, 15864, 15864, 16412,
   15864,   801,   988,   800, -1840,    85, -1840, 18022,   115, -1840,
     991,   122,   878, -1840,   880, -1840, 18203, 14953, 14953,   815,
     833, -1840, -1840, 12733, 12733, 14953, 14953, 14953, 14953, 14953,
     127,   467,   508, -1840, 15132, 18203,   608, -1840, 16412, -1840,
     504,   645, -1840, -1840, -1840, -1840, 19040, 14953,  1003,   917,
   -1840, -1840, -1840,   143, 14953,   821,   822, 19475,   824,  1947,
     825,  6267, 14953, -1840,    81,   828,   654,    81,   573,   548,
   -1840, 16412,  4499,   829, 11596, 15971, -1840, 13682,   827,   827,
     827,   827, -1840, -1840,  1441, -1840, -1840, -1840, -1840, -1840,
     790, -1840, 14953, 14953, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840, -1840, -1840, 14953, 14953, 14953, 14953, 13861, 14953, 14953,
   14953, 14953, 14953, 14953, 14953, 14953, 14953, 14953, 14953, 14953,
   14953, 14953, 14953, 14953, 14953, 14953, 14953, 14953, 14953, 14953,
   14953, 19116, 14953, -1840, 11972, 14953, 14953, 14953, 15305, 16412,
   16412, 16412, 16412, 16412, 15353,   918,  1054,  5369, 14953, 14953,
   14953, 14953, 14953, 14953, 14953, 14953, 14953, 14953, 14953, 14953,
   -1840, -1840, -1840, -1840,  1553, -1840, -1840, 11596, 11596, 14953,
   14953,   837,   790, 14953, 14040, 17676, -1840, 14953, -1840,   839,
    1022,   883,   847,   851, 15459,    63, 14219, 14398, -1840,   783,
     862,   866,  2411, -1840,   125, 11596, -1840,  3094, -1840, -1840,
   17724, -1840, -1840, 12169, -1840, 14953, -1840,   968, 10387,  1059,
     868, 19353,  1057,   100,    80, -1840, -1840, -1840,   890, -1840,
   -1840, -1840,  4499, -1840,  2206,   876,  1068, 17870, 16412, -1840,
   -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840,   881, -1840,
   -1840,   882,   884,   893,   885,   895,   897,   453,   900,   903,
   16222, 16043, -1840, -1840, 16412, 16412, 14953,    63,    92, -1840,
   17870,  1008, -1840, -1840, -1840,    63,   145,   149,   905,   907,
    2656,   237,   911,   913,   355,   969,   156,   159, 18424,   915,
    1115,  1116,   922,   926,   927,   929, -1840,  4380, 16412, -1840,
   -1840,  1066,  3098,    77, -1840, -1840, -1840,   645, -1840, -1840,
   -1840,  1105,  1005,   963,   198,   985, 14953,  1011,  1141,   957,
   -1840,  1002, -1840,   166, -1840,   965,  4499,  4499,  1159,   975,
     143, -1840,   989,  1175, -1840,  3637,   428, -1840,   525,   359,
   -1840, -1840, -1840, -1840, -1840, -1840, -1840,  1498,  3368, -1840,
   -1840, -1840, -1840,  1177,  1004, -1840, 18203,   552, 14953,   987,
    1186, 19475,  1182,   161,  1188,   998,   999,  1009, 19475,  1010,
    2676,  6473, -1840, -1840, -1840, -1840, -1840, -1840,  1070,  3739,
   19475,  1001,  3467, 19614, 19705, 19786, 19823, 14953, 19427, 19896,
   15306, 20001, 20068, 15972, 16520, 19123, 19123, 19123, 19123,  3136,
    3136,  3136,  3136,  3136,  1173,  1173,   749,   749,   749,   398,
     398,   398, -1840,   902, -1840, -1840, 19786,  1012,  1017, 18485,
    1019,  1199,   388, 14953,   549,   789,   346, -1840, -1840, -1840,
    1198,   917, -1840,   790, 18127, -1840, -1840, -1840, 19786, 19786,
   19786, 19786, 19786, 19786, 19786, 19786, 19786, 19786, 19786, 19786,
   19786, -1840, 14953,   580, -1840,   183, -1840,   789,   587,  1013,
    1014,  1027,  4405,  1016, -1840, 19475, 17946, -1840, 16412, -1840,
      63,   541, 18203, 19475, 18203, 18533,  1070,    74,    63,   185,
   -1840,   166,  1071,  1034, 14953, -1840,   200, -1840, -1840, -1840,
    6679,   626, -1840, -1840, 19475, 19475,    13, -1840, -1840, -1840,
   14953,  1134,  3981, 17870, 16412, 10593,  1037,  1038, -1840,  1236,
    5043,  1108, -1840,  1090, -1840,  1244,  1060,  3141,  4499, 17870,
   17870, 17870, 17870, 17870,  1055,  1187,  1191,  1194,  1196,  1204,
    1069,  1076, 17870,   463, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840,   239, -1840, 19569, -1840, -1840,    39, -1840,  6885, 15685,
    1075, 16043, -1840, 16043, -1840, 16043, -1840, 16412, 16412, 16043,
   -1840, 16043, 16043, 16412, -1840,  1269,  1080, -1840,   486, -1840,
   -1840,  4701, -1840, 19569,  1265, 18203,  1084, -1840, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840, -1840,  1102,  1278, 16412, 15685,
    1087, -1840, -1840, 14953, -1840, 14953, -1840, 14953, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840, -1840,  1086, -1840, 14953, -1840,
   -1840,  5855, -1840,  4499, 15685,  1089, -1840, -1840, -1840, -1840,
    1117,  1280,  1095, 14953, 19040, -1840, -1840, 15305, -1840,  1097,
   -1840,  4499, -1840,  1100,  7091,  1268,    66, -1840,  4499, -1840,
     164,  1553,  1553, -1840,  4499, -1840, -1840,    63, -1840, -1840,
    1231, 19475, -1840, 11775, -1840, 17870, 13682,   827, 13682, -1840,
     827,   827, -1840, 12375, -1840, -1840,  7297, -1840,    71,  1103,
   15685,  1005, -1840, -1840, -1840, -1840, 19896, 14953, -1840, -1840,
   14953, -1840, 14953, -1840,  4947,  1104, 11596,   969,  1282,  1005,
    4499,  1298,  1070, 16412, 19116,    63,  5126,  1118,   370,  1119,
   -1840, -1840,   717,   717, 17946, -1840, -1840, -1840,  1263,  1121,
    1246,  1251,  1253,  1256,  1257,   275,  1126,  1130,   682, -1840,
   -1840, -1840, -1840, -1840, -1840,  1174, -1840, -1840, -1840, -1840,
    1328,  1138,   839,    63,    63, 14577,  1005,  3094, -1840,  3094,
   -1840, 17191,   663,    13, 11211, -1840,  7503,  1139,  7709,  1144,
    3981, 18203,  1201,  1147,    63, 19569,  1333, -1840, -1840, -1840,
   -1840,   743, -1840,   325,  4499,  1164,  1212,  1189,  4499, 16412,
    4754, -1840, -1840,  4499,  1346,  1158,  1183,  1190,  1177,   816,
     816,  1294,  1294, 18702,  1163,  1360, 17870, 17870, 17870, 17870,
   17870, 17870, 19040, 17870,  2721, 16866, 17870, 17870, 17870, 17870,
   17794, 17870, 17870, 17870, 17870, 17870, 17870, 17870, 17870, 17870,
   17870, 17870, 17870, 17870, 17870, 17870, 17870, 17870, 17870, 17870,
   17870, 17870, 17870, 17870, 16412, -1840, -1840,  1285, -1840, -1840,
    1169,  1172,  1176, -1840,  1178, -1840, -1840,   509, 16222, -1840,
    1179, -1840, 17870,    63, -1840, -1840,   187, -1840,   687,  1368,
   -1840, -1840, 19475, 18594, -1840,  2546, -1840,  6061,   917,  1368,
   -1840,   249, 14953,   229, -1840, 19475,  1247,  1192, -1840,  1185,
    1268, -1840, -1840, -1840, 14774,  4499,   975, 17715,  1305,   269,
    1376,  1309,   209, -1840,   789,   213, -1840,   789, -1840, 14953,
   18203,   552, 14953, 19475, 19569, -1840, -1840, -1840,  3847, -1840,
   -1840, -1840, -1840, -1840, -1840,  1197,    71, -1840,  1202,    71,
    1195, 19896, 19475, 18642,  1200, 11596,  1206,  1210,  4499,  1211,
    1213,  4499,  1005, -1840,   783,   593, 11596, -1840, -1840, -1840,
   -1840, -1840, -1840,  1258,  1207,  1392,  1324, 17946, 17946, 17946,
   17946, 17946, 17946,  1259, -1840, 19040, 17946,   102, 17946, -1840,
   -1840, -1840, 18203, 19475,  1219, -1840,    13,  1391,  1347, 11211,
   -1840, -1840, -1840,  1224, 14953,  1201,    63,  3981,  1228, 17870,
    7915,   810,  1229, 14953,    62,   471, -1840,  1249, -1840,  4499,
   16412, -1840,  1296, -1840, -1840, -1840,  5124, -1840,  1403, -1840,
    1237, 17870, -1840, 17870, -1840,  1238,  1232,  1430, 18748,  1239,
   19569,  1431,  1240,  1242,  1245,  1304,  1439,  1250,  1252, -1840,
   -1840, -1840, 18808,  1248,  1445, 19661, 19749, 19859, 17870, 19523,
   19967, 20035, 20101, 16151, 18210, 20132, 20132, 20132, 20132,  2004,
    2004,  2004,  2004,  2004,  1255,  1255,   816,   816,   816,  1294,
    1294,  1294,  1294, -1840,  1260, -1840,  1254,  1266,  1267,  1270,
   -1840, -1840, 19569, 16412,  4499,  4499, -1840,   687, 15685,   105,
   14953,  1261, -1840,  1264,  1359, -1840,   191, 14953, -1840, -1840,
   17239, -1840, 14953, -1840, 14953, -1840,   975, 13682,  1272,   247,
     827,   247,   374, -1840, -1840,  4499,   208, -1840,  1443,  1386,
   14953, -1840,  1271,  1275,  1273,    63,  1231, 19475,  1268,  1276,
   -1840,  1279,    71, 14953, 11596,  1288, -1840, -1840,   917, -1840,
   -1840,  1274,  1287,  1292, -1840,  1295, 17946, -1840, 17946, -1840,
   -1840,  1297,  1299,  1483,  1361,  1301, -1840,  1489,  1302,  1303,
    1306, -1840,  1362,  1308,  1492,  1314, -1840, -1840,    63, -1840,
    1470, -1840,  1317, -1840, -1840,  1315,  1323, -1840, -1840, 19569,
    1325,  1329, -1840, 17386, -1840, -1840, -1840, -1840, -1840, -1840,
    1387,  4499,  4499,  1183,  1357,  4499, -1840, 19569, 18854, -1840,
   -1840, 17870, -1840, 17870, -1840, 17870, -1840, -1840, -1840, -1840,
   17870, 19040, -1840, -1840, -1840, 17870, -1840, 17870, -1840, 19932,
   17870,  1334,  8121, -1840, -1840, -1840, -1840,   687, -1840, -1840,
   -1840, -1840,   696, 16150, 15685,  1422,  1424,  1425,  1427, -1840,
    4181,  1370,  4187, -1840, -1840,  1460,   918,  4107,  1435,   128,
     131,  1348,   917,  1054, 19475, -1840, -1840, -1840,  1383, 17287,
   -1840, 17335, 19475, -1840,  2922, -1840,  6473,  1468,   284,  1538,
    1472, 14953, -1840, 19475, 11596, 11596, -1840,  1437,  1268,  1874,
    1268,  1364, 19475,  1365, -1840,  1892,  1355,  2196, -1840, -1840,
      71, -1840, -1840,  1423, -1840, -1840, 17946, -1840, 17946, -1840,
   17946, -1840, -1840, -1840, -1840, 17946, -1840, 19040, -1840, -1840,
    2219, -1840,  8327, -1840, -1840, -1840, 10799, -1840, -1840, -1840,
    6679,  4499, -1840, -1840, -1840,  1366, 17870, 18914, 19569, 19569,
   19569,  1436, 19569, 18960, 19932, -1840, -1840,   687, 15685, 15685,
   16412, -1840,  1554, 17020,    83, -1840, 16150,   917, 16605, -1840,
    1390, -1840,   133,  1367,   140, -1840, 16519, -1840, -1840, -1840,
     141, -1840, -1840,  4868, -1840,  1377, -1840,  1563,   142,   790,
    1460, 16340, 16340, -1840, 16340, -1840, -1840,  1566,   918,  5191,
   15613, -1840, -1840, -1840, -1840,  2796, -1840,  1567,  1504, 14953,
   -1840, 19475,  1388,  1389,  1393,  1268,  1395, -1840,  1437,  1268,
   -1840, -1840, -1840, -1840,  2368,  1394, 17946,  1457, -1840, -1840,
   -1840,  1464, -1840,  6679, 11005, 10799, -1840, -1840, -1840,  6679,
   -1840, -1840, 19569, 17870, 17870, 17870,  8533,  1402,  1404, -1840,
   17870, -1840, 15685, -1840, -1840, -1840, -1840, -1840,  4499,  1042,
    4181, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840,   193, -1840,  1370, -1840, -1840,
   -1840, -1840, -1840,   112,   705, -1840, 17870,  1521, -1840, 16712,
     146,  1592,  1598, -1840,  4499,   790,   150,  1460, -1840, -1840,
    1415,  1603, 14953, -1840, 19475, -1840, -1840,   167,  1426,  4499,
     639,  1268,  1395, 15792, -1840,  1268, -1840, 17946, 17946, -1840,
   -1840, -1840, -1840,  8739, 19569, 19569, 19569, -1840, -1840, -1840,
   19569, -1840,  1994,  1616,  1618,  1429, -1840, -1840, 17870, 16519,
   16519,  1561, -1840,  4868,  4868,   722, -1840, -1840, -1840, 19569,
    1621,  1455,  1442, -1840, 17870, 17870, -1840, 16712, -1840,   153,
   -1840, 17870, 19475,  1560, -1840,  1637, -1840,  1638, -1840,   623,
   -1840, -1840, -1840,  1446,   639, -1840,   639, -1840, -1840,  8945,
    1449,  1536, -1840,  1551,  1493, -1840, -1840,  1555,  4499,  1475,
    1042, -1840, -1840, 19569, -1840, -1840,  1486, -1840,  1625, -1840,
   -1840, -1840, -1840, 17870,   355, -1840, 19569, 19569,  1465, -1840,
   19569, -1840,   178,  1463,  9151,  4499, -1840,  4499, -1840,  9357,
   -1840, -1840, -1840,  1466, -1840,  1469,  1490, 16412,  1054,  1485,
   -1840, -1840, -1840, 19569,  1487,   118, -1840,  1589, -1840, -1840,
   -1840, -1840, -1840, -1840,  9563, -1840, 15685,  1075, -1840,  1499,
   16412,   711, -1840, -1840,  1477,  1666,   625,   118, -1840, -1840,
    1596, -1840, 15685,  1481, -1840,  1268,   148, -1840,  4499, -1840,
   -1840, -1840,  4499, -1840,  1491,  1501,   154, -1840,  1395,   659,
    1600,   231,  1268,  1488, -1840,   644,  4499,  4499, -1840,   426,
    1671,  1606,  1395, -1840, -1840, -1840, -1840,  1610,   252,  1688,
    1620, 14953, -1840,   644,  9769,  9975, -1840,   448,  1691,  1623,
   14953, -1840, 19475, -1840, -1840, -1840,  1693,  1628, 14953, -1840,
   19475, 14953, -1840, 19475, 19475
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   204,   474,     0,   916,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1013,
     999,     0,   780,     0,   786,   787,   788,    29,   853,   987,
     988,   171,   172,   789,     0,   152,     0,     0,     0,     0,
      30,     0,     0,     0,     0,   223,     0,     0,     0,     0,
       0,     0,   442,   443,   444,   441,   440,   439,     0,     0,
       0,     0,   252,     0,     0,     0,    37,    38,    40,    41,
      39,   793,   795,   796,   790,   791,     0,     0,     0,   797,
     792,     0,   763,    32,    33,    34,    36,    35,     0,   794,
       0,     0,     0,     0,     0,   798,   445,   584,    31,     0,
     170,   140,     0,   781,     0,     0,     4,   126,   128,   852,
       0,   762,     0,     6,     0,     0,   222,     7,     9,     8,
      10,     0,     0,   437,   970,   488,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   544,   486,   975,   976,   566,
     559,   560,   561,   562,   565,   563,   564,   469,   569,     0,
     468,   944,   764,   771,     0,   855,   558,   436,   947,   948,
     960,   487,     0,     0,     0,   490,   489,   945,   946,   943,
     983,   986,   548,   854,    11,   442,   443,   444,     0,    36,
       0,   126,   222,     0,  1045,   559,   487,  1046,     0,  1048,
    1049,   568,   482,     0,   475,   480,     0,     0,   530,   531,
     532,   533,    29,   987,   789,   766,    37,    38,    40,    41,
      39,     0,     0,  1069,   966,   764,     0,   765,   509,     0,
     511,   549,   550,   551,   552,   553,   554,   555,   557,     0,
    1007,     0,   776,   242,     0,  1069,   466,   775,   769,     0,
     785,   765,   994,   995,  1001,   993,   777,     0,   467,     0,
     779,   556,     0,   205,     0,     0,   471,   205,   150,   473,
       0,     0,   156,   158,     0,     0,   160,     0,    75,    76,
      82,    83,    67,    68,    59,    80,    91,    92,     0,    62,
       0,    66,    74,    72,    94,    86,    85,    57,   108,    81,
     101,   102,    58,    97,    55,    98,    56,    99,    54,   103,
      90,    95,   100,    87,    88,    61,    89,    93,    53,    84,
      69,   104,    77,   106,    70,    60,    47,    48,    49,    50,
      51,    52,    71,   107,   105,   110,    64,    45,    46,    73,
    1122,  1123,    65,  1127,    44,    63,    96,     0,    79,     0,
     126,   109,  1060,  1121,     0,  1124,     0,     0,     0,   162,
       0,     0,     0,   213,     0,     0,     0,     0,     0,     0,
       0,     0,   864,     0,   114,   116,   350,     0,     0,   349,
     355,     0,     0,   253,     0,   256,     0,     0,     0,     0,
    1066,   238,   250,  1013,  1013,   604,   634,   634,   604,   634,
       0,  1030,     0,   800,     0,     0,     0,  1028,     0,    16,
       0,   130,   230,   244,   251,   664,   596,   634,     0,  1054,
     576,   578,   580,   920,   474,   488,     0,     0,   486,   487,
     489,   205,     0,   990,   782,     0,   783,     0,     0,     0,
     202,     0,     0,   132,   339,     0,    28,     0,     0,     0,
       0,     0,   203,   221,     0,   249,   234,   248,   442,   445,
     222,   438,   992,     0,   936,   192,   193,   194,   195,   196,
     198,   199,   201,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   999,     0,   191,     0,   992,   992,  1015,     0,     0,
       0,     0,     0,     0,     0,     0,   435,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     508,   510,   921,   922,     0,   935,   934,   339,   339,   992,
       0,     0,   222,     0,     0,     0,   164,     0,   918,   913,
     862,     0,   488,   486,     0,  1006,     0,  1012,   601,   785,
     488,   486,   487,   132,     0,   339,   465,     0,   937,   778,
       0,   140,   292,     0,   583,     0,   167,     0,   205,   472,
       0,     0,     0,     0,     0,   159,   190,   161,  1122,  1123,
    1119,  1120,     0,  1126,  1112,     0,     0,     0,     0,    78,
      43,    65,    42,  1061,   197,   200,   163,   140,     0,   180,
     189,     0,     0,     0,     0,     0,     0,   117,     0,     0,
       0,   863,   115,    18,     0,   111,     0,   351,     0,   165,
       0,     0,   166,   254,   255,  1050,     0,     0,   488,   486,
     487,   490,   489,     0,  1102,   262,     0,     0,     0,     0,
     862,   862,     0,     0,     0,     0,   168,     0,     0,   799,
    1029,   853,     0,     0,  1027,   858,  1026,   129,     5,    13,
      14,     0,   260,     0,     0,   589,     0,     0,   862,     0,
     773,     0,   772,   767,   590,     0,     0,     0,     0,     0,
     920,   136,     0,   864,   919,  1131,   464,   477,   491,   953,
     974,   147,   139,   143,   144,   145,   146,   436,     0,   567,
     856,   857,   127,   862,     0,  1070,     0,     0,     0,     0,
     864,   340,     0,     0,     0,   488,   209,   210,   208,   486,
     487,   205,   184,   182,   183,   185,   572,   224,   258,     0,
     991,     0,     0,   514,   516,   515,   527,     0,     0,   547,
     512,   513,   517,   519,   518,   536,   537,   534,   535,   538,
     539,   540,   541,   542,   528,   529,   521,   522,   520,   523,
     524,   526,   543,   525,   479,   484,   492,     0,     0,  1019,
       0,   862,  1053,     0,  1052,  1069,   950,   240,   232,   246,
       0,  1054,   236,   222,     0,   478,   481,   483,   493,   496,
     497,   498,   499,   500,   501,   502,   503,   504,   505,   506,
     507,   924,     0,   923,   926,   949,   930,  1069,   927,     0,
       0,     0,     0,     0,  1047,   476,   911,   915,   861,   917,
     463,   768,     0,  1005,     0,  1011,   258,     0,   768,   998,
     997,   983,   986,     0,     0,   923,   926,   996,   927,   485,
     294,   296,   136,   587,   586,   470,     0,   140,   276,   151,
     473,     0,     0,     0,     0,   205,   288,   288,   157,   862,
       0,     0,  1111,     0,  1108,   862,     0,  1082,     0,     0,
       0,     0,     0,   860,     0,    37,    38,    40,    41,    39,
       0,     0,     0,   802,   806,   807,   808,   811,   809,   810,
     813,     0,   801,   134,   851,   812,  1069,  1125,   205,     0,
       0,     0,    21,     0,    22,     0,    19,     0,   112,     0,
      20,     0,     0,     0,   123,   864,     0,   121,   116,   113,
     118,     0,   348,   356,   353,     0,     0,  1039,  1044,  1041,
    1040,  1043,  1042,    12,  1100,  1101,     0,   862,     0,     0,
       0,   602,   600,     0,   615,   861,   603,   861,   633,   618,
     627,   630,   621,  1038,  1037,  1036,     0,  1032,     0,  1033,
    1035,   205,     5,     0,     0,     0,   659,   660,   669,   668,
       0,     0,   486,     0,   861,   595,   599,     0,   624,     0,
    1055,     0,   577,     0,   205,  1089,   920,   320,  1131,  1130,
       0,     0,     0,   989,   861,  1072,  1068,   342,   336,   337,
     341,   343,   761,   863,   338,     0,     0,     0,     0,   463,
       0,     0,   491,     0,   954,   212,   205,   142,   920,     0,
       0,   260,   574,   226,   932,   933,   546,     0,   641,   642,
       0,   639,   861,  1014,     0,     0,   339,   262,     0,   260,
       0,     0,   258,     0,   999,   494,     0,     0,   951,   952,
     984,   985,     0,     0,   899,   869,   870,   871,   878,     0,
      37,    38,    40,    41,    39,     0,     0,     0,   884,   890,
     891,   892,   895,   893,   894,     0,   882,   880,   881,   905,
     862,     0,   913,  1004,  1010,     0,   260,     0,   938,     0,
     784,     0,   298,     0,   205,   148,   205,     0,   205,     0,
       0,     0,   268,   271,   269,   280,     0,   140,   278,   177,
     288,     0,   288,     0,   861,     0,     0,     0,     0,     0,
     861,  1110,  1113,  1078,   862,     0,  1073,     0,   862,   834,
     835,   832,   833,   868,     0,   862,   860,   608,   636,   636,
     608,   636,   598,   636,     0,     0,  1021,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1116,   214,     0,   217,   181,
       0,     0,     0,   119,     0,   124,   125,   117,   863,   122,
       0,   352,     0,  1051,   169,  1067,  1102,  1093,  1097,   261,
     263,   362,   606,     0,  1031,     0,    17,   205,  1054,   259,
     362,     0,     0,     0,   768,   592,     0,   774,  1056,     0,
    1089,   581,   135,   137,     0,     0,     0,  1131,     0,     0,
     325,   323,   926,   939,  1069,   926,   940,  1069,  1071,   992,
       0,     0,     0,   344,   133,   207,   209,   210,   487,   188,
     206,   186,   187,   211,   141,     0,   920,   257,     0,   920,
       0,   545,  1018,  1017,     0,   339,     0,     0,     0,     0,
       0,     0,   260,   228,   785,   925,   339,   874,   875,   876,
     877,   885,   886,   903,     0,   862,     0,   899,   612,   638,
     638,   612,   638,     0,   873,   907,   638,     0,   861,   910,
     912,   914,     0,  1008,     0,   925,     0,     0,     0,   205,
     295,   588,   153,     0,   473,   268,   270,     0,     0,     0,
     205,     0,     0,     0,     0,     0,   282,     0,  1117,     0,
       0,  1103,     0,  1109,  1107,  1074,   861,  1080,     0,  1081,
       0,     0,   804,   861,   859,     0,     0,   862,     0,     0,
     848,   862,     0,     0,     0,     0,   862,     0,     0,   814,
     849,   850,  1025,     0,   862,   817,   819,   818,     0,     0,
     815,   816,   820,   822,   821,   838,   839,   836,   837,   840,
     841,   842,   843,   844,   829,   830,   824,   825,   823,   826,
     827,   828,   831,  1115,     0,   140,     0,     0,     0,     0,
     120,    23,   354,     0,     0,     0,  1094,  1099,     0,   436,
       0,     0,    15,     0,   436,   672,     0,     0,   674,   667,
       0,   670,     0,   666,     0,  1058,     0,     0,     0,   970,
     544,     0,   490,  1090,   585,  1131,     0,   326,   327,     0,
       0,   321,     0,     0,     0,   346,   347,   345,  1089,     0,
     362,     0,   920,     0,   339,     0,   981,   362,  1054,   362,
    1057,     0,     0,     0,   495,     0,     0,   888,   861,   898,
     879,     0,     0,   862,     0,     0,   897,   862,     0,     0,
       0,   872,     0,     0,   862,     0,   883,   904,  1009,   362,
       0,   140,     0,   291,   277,     0,     0,   267,   173,   281,
       0,     0,   284,     0,   289,   290,   140,   283,  1118,  1104,
       0,     0,  1077,  1076,     0,     0,  1129,   867,   866,   803,
     616,   861,   607,     0,   619,   861,   635,   628,   631,   622,
       0,   861,   597,   805,   625,     0,   640,   861,  1020,   846,
       0,     0,   205,    24,    25,    26,    27,  1096,  1091,  1092,
    1095,   264,     0,     0,     0,   443,   441,   440,   439,   434,
       0,     0,     0,   239,   361,     0,     0,   433,     0,     0,
       0,     0,  1054,   436,   605,  1034,   358,   245,   662,     0,
     665,     0,   591,   579,   487,   138,   205,     0,     0,   330,
     319,     0,   322,   329,   339,   339,   335,   571,  1089,   436,
    1089,     0,  1016,     0,   980,   436,     0,   436,  1059,   362,
     920,   977,   902,   901,   887,   617,   861,   611,     0,   620,
     861,   637,   629,   632,   623,     0,   889,   861,   906,   626,
     436,   140,   205,   149,   154,   175,   205,   279,   285,   140,
     287,     0,  1105,  1075,  1079,     0,     0,     0,   610,   847,
     594,     0,  1024,  1023,   845,   140,   218,  1098,     0,     0,
       0,  1062,     0,     0,     0,   265,     0,  1054,     0,   399,
     395,   401,   763,    36,     0,   389,     0,   394,   398,   411,
       0,   409,   414,     0,   413,     0,   412,   453,     0,   222,
       0,     0,     0,   367,     0,   368,   369,     0,     0,   435,
       0,   663,   661,   673,   671,     0,   331,   332,     0,     0,
     317,   328,     0,     0,     0,  1089,  1083,   235,   571,  1089,
     982,   241,   358,   247,   436,     0,     0,     0,   614,   896,
     909,     0,   243,   293,   205,   205,   140,   274,   174,   286,
    1106,  1128,   865,     0,     0,     0,   205,     0,     0,   462,
       0,  1063,     0,   379,   383,   459,   460,   393,     0,     0,
       0,   374,   722,   723,   721,   724,   725,   742,   744,   743,
     713,   685,   683,   684,   703,   718,   719,   679,   690,   691,
     693,   692,   760,   712,   696,   694,   695,   697,   698,   699,
     700,   701,   702,   704,   705,   706,   707,   708,   709,   711,
     710,   680,   681,   682,   686,   687,   689,   759,   727,   728,
     732,   733,   734,   735,   736,   737,   720,   739,   729,   730,
     731,   714,   715,   716,   717,   740,   741,   745,   747,   746,
     748,   749,   726,   751,   750,   753,   755,   754,   688,   758,
     756,   757,   752,   738,   678,   406,   675,     0,   375,   427,
     428,   426,   419,     0,   420,   376,     0,     0,   363,     0,
       0,     0,     0,   458,     0,   222,     0,     0,   231,   357,
       0,     0,     0,   318,   334,   978,   979,     0,     0,     0,
       0,  1089,  1083,     0,   237,  1089,   900,     0,     0,   140,
     272,   155,   176,   205,   609,   593,  1022,   216,   377,   378,
     456,   266,     0,   862,   862,     0,   402,   390,     0,     0,
       0,   408,   410,     0,     0,   415,   422,   423,   421,   454,
     451,  1064,     0,   364,     0,     0,   461,     0,   365,     0,
     359,     0,   333,     0,   657,   864,   136,   864,  1085,     0,
     429,   136,   225,     0,     0,   233,     0,   613,   908,   205,
       0,   178,   380,   126,     0,   381,   382,     0,   861,     0,
     861,   404,   400,   405,   676,   677,     0,   391,   424,   425,
     417,   418,   416,     0,  1102,   370,   457,   455,     0,   366,
     360,   658,   863,     0,   205,   863,  1084,     0,  1088,   205,
     136,   227,   229,     0,   275,     0,   220,     0,   436,     0,
     396,   403,   407,   452,     0,   920,   372,     0,   655,   570,
     573,  1086,  1087,   430,   205,   273,     0,     0,   179,   387,
       0,   435,   397,  1065,     0,   864,   447,   920,   656,   575,
       0,   219,     0,     0,   386,  1089,   920,   302,  1131,   450,
     449,   448,  1131,   446,     0,     0,     0,   385,  1083,   447,
       0,     0,  1089,     0,   384,     0,  1131,  1131,   308,     0,
     307,   305,  1083,   140,   431,   136,   371,     0,     0,   309,
       0,     0,   303,     0,   205,   205,   313,     0,   312,   301,
       0,   304,   311,   373,   215,   432,   314,     0,     0,   299,
     310,     0,   300,   316,   315
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1840, -1840, -1840,  -577, -1840, -1840, -1840,   545,   -47,   -42,
     530, -1840,  -194,  -513, -1840, -1840,   514,   598,  1919, -1840,
    2662, -1840,  -819, -1840,  -497, -1840,  -702,    38, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840, -1840,  -911, -1840, -1840,  -915,
    -324, -1840, -1840, -1840,  -319, -1840, -1840,  -165,   157,    52,
   -1840, -1840, -1840, -1840, -1840, -1840,    55, -1840, -1840, -1840,
   -1840, -1840, -1840,    56, -1840, -1840,  1203,  1215,  1205,   -90,
    -754,  -944,   666,   741,  -323,   396, -1031, -1840,   -17, -1840,
   -1840, -1840, -1840,  -789,   217, -1840, -1840, -1840, -1840,  -305,
   -1840,  -606, -1840,   483,  -466, -1840, -1840,  1107, -1840,     5,
   -1840, -1840, -1123, -1840, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840,   -32, -1840,    61, -1840, -1840, -1840, -1840, -1840,  -116,
   -1840,   171, -1047, -1840, -1733,  -341, -1840,  -152,   135,  -130,
    -314, -1839, -1520, -1840, -1840, -1840,   181,   -80,   -77,   -13,
    -780,    23, -1840, -1840,    29, -1840,   130,  -359, -1840,    -3,
      -5,   -79,   -84,    19, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840, -1840, -1840,  -636,  -909, -1840, -1840, -1840, -1840, -1840,
     710,  1351, -1840,   607, -1840,   459, -1840, -1840, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840, -1840, -1840, -1840, -1840,    26,  -701,  -612, -1840, -1840,
   -1840, -1840, -1840,   540, -1840, -1840, -1840, -1840, -1840, -1840,
   -1840, -1840, -1027,  -379,  2864,    10, -1840,    49,  -444, -1840,
   -1840,  -495,  3703,  3929, -1840,   635, -1840, -1840,   618,  -494,
    -665, -1840, -1840,   702,   473,  -554, -1840,   470, -1840, -1840,
   -1840, -1840, -1840,   679, -1840, -1840, -1840,   139,  -945,  -115,
    -449,  -445, -1840,   -92,  -137, -1840, -1840,    28,    40,   662,
     -62, -1840, -1840,   555,   -75, -1840,  -375,    35,  -378,    59,
    -439, -1840, -1840,   520, -1840, -1840, -1840, -1840,   823,   517,
   -1840, -1840, -1840,  -370,  -675, -1840,  1331, -1238,  -163,    -8,
    -195,   906, -1840, -1840, -1840, -1799, -1840,  -217,  -851, -1341,
    -204,   224, -1840,   581,   656, -1840, -1840, -1840, -1840,   604,
   -1840,  3265,  -816
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   116,   972,   668,   191,  1671,   785,
     371,   372,   373,   374,   925,   926,   927,   118,   119,   120,
     121,   122,   994,  1232,   431,  1026,   702,   703,   578,   267,
    1744,   584,  1646,  1745,  2006,   910,   124,   125,   723,   724,
     732,   364,   607,  1961,  1189,  1405,  2028,   454,   192,   704,
    1029,  1270,  1473,   128,   671,  1048,   705,   738,  1052,   645,
    1047,   246,   559,   706,   672,  1049,   456,   391,   413,   131,
    1031,   975,   950,  1209,  1674,  1328,  1112,  1901,  1748,   859,
    1119,   583,   868,  1121,  1516,   851,  1102,  1105,  1318,  2034,
    2035,   692,   693,  1010,   719,   720,   378,   379,   381,  1710,
    1879,  1880,  1419,  1574,  2015,  2037,  1912,  1965,  1966,  1967,
    1684,  1685,  1686,  1687,  1914,  1915,  1921,  1977,  1690,  1691,
    1695,  1862,  1863,  1864,  1952,  2076,  1575,  1576,   193,   133,
    2052,  1578,  1698,  1579,  1580,  1581,  1582,   134,   135,   651,
     580,   136,   137,   138,   139,   140,   141,   142,   143,   260,
     144,   145,   146,  1725,   147,  1028,  1269,   148,   689,   690,
     691,   264,   423,   574,   677,   678,  1366,   679,  1367,   149,
     195,   649,   650,  1356,  1357,  1482,  1483,   151,   894,  1079,
     152,   895,  1080,   153,   896,  1081,   154,   897,  1082,   155,
     898,  1083,   156,   899,  1084,   652,  1359,  1485,   157,   900,
     158,   159,  1945,   160,   673,  1712,   674,  1221,   981,  1433,
    1429,  1855,  1856,   161,   162,   163,   249,   164,   250,   261,
     435,   566,   165,  1360,  1361,   904,   905,   166,  1144,   829,
     622,  1145,  1087,  1291,  1088,  1486,  1487,  1294,  1295,  1090,
    1493,  1494,  1091,   827,   550,   206,   207,   707,   695,   534,
    1242,  1243,   816,   817,   464,   168,   252,   169,   170,   196,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     741,   256,   257,   241,   780,   781,  1373,  1374,   406,   407,
     966,   182,   636,   183,   688,   184,   354,  1881,  1932,   392,
     443,   713,   714,  1134,  1135,  1890,  1947,  1948,  1236,  1416,
     946,  1417,   947,   948,   874,   875,   876,   355,   356,   907,
     593,   999,  1000
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     194,   197,   461,   199,   200,   201,   202,   204,   205,   351,
     208,   209,   210,   211,   352,   515,   231,   232,   233,   234,
     235,   236,   237,   238,   240,   428,   258,   541,   997,  1027,
     263,   265,   425,  1106,   430,   535,   536,   680,   683,   266,
     248,   682,   123,   268,   448,   449,   684,   274,   272,   277,
     563,  1237,   362,   992,   365,  1014,   127,   772,   253,   129,
     130,  1137,   450,   461,   836,   255,   777,   778,   226,   226,
     254,   819,   820,   400,   850,  1226,  1560,   514,  1123,  1325,
    1109,   266,  1096,  1234,   993,   814,   360,  1268,   727,   815,
     414,   971,  1762,  1954,  1514,   418,   419,  1424,   557,   843,
     821,  1255,   445,  1260,   427,  1279,   428,   924,   929,   864,
     908,  1562,   866,   425,    14,   430,  1051,   575,   846,    14,
     568,  1923,   847,   457,   628,   733,   734,   735,   -43,   -78,
     -42,   631,   426,   -43,   -78,   -42,   575,  1702,   553,   132,
    1704,   567,  -392,   167,   462,   552,   462,   430,  1924,  1770,
    1857,  1867,  1314,    14,   935,  1867,   956,   958,   575,  1867,
     561,   126,  1867,  1762,   401,   557,    14,   560,   557,   380,
    1016,   433,   612,   614,   616,   376,   619,   504,  -765,   543,
    1870,   551,  1238,  1496,   985,   427,   841,   361,   624,  1876,
     401,    14,     3,   545,   608,  1234,    14,  2053,   434,   532,
     533,  1184,  1239,   198, -1000,  1943,   259,  1918,   213,    40,
     532,   533,   262,   532,   533,  -967,  2017,   427,  1563,  1005,
    2053,  2001,   442,  2002,  1564,  1919,   458,  1565,   187,  1566,
    1567,  1568,  1569,  -643,  1370,  -643,   539,   944,   945,   427,
    -651,   404,   405,  1365,  1920,  1588,  1598,  1240,  -766,   403,
    1944,   625,   977,  -959,   570, -1003,   579,   570,   609,  2065,
    1199,  2018,  -863,  1515,   266,   581,  -863,   404,   405,  2069,
   -1002,  -324,  1089,  2083,  1570,  1571,   463,  1572,   463,  -941,
     970,  -955,   867,  -942,  1763,  1764,   658,  1043,   226,   460,
    2087,  1599,  1233,   558,   446,   739,  1507,   377,  1282,   459,
     865,   572,   516,   592,   639,   577,  -654,  -651,  1573,   576,
     351,   638,  1925,   642,  2070,   603,   629,   462,  -306,  1438,
     -43,   -78,   -42,   632,  1264,  1672,  1560,  -958,   656,  1703,
     112,  1331,  1705,  1335,  -392,  2088, -1000,  1609,  1472,  -324,
    1241,  1771,  1858,  1868,  1615,   936,  1617,  1933,  -863,   937,
     542,  1938,  1448,   202,  1989,  2064,   951,  1939,   415,   952,
    1108,  1017,  -966,   729,  -966,   978,  1303,  1717,   540,   538,
     725,  -861,   430,  -773,   637,  1125,  1640,   823,  -652,  1436,
     979,  1131,   266,   427,  1600,  -959,   461, -1003,   240,   240,
     648,   266,   266,   648,   266,  1217,  1492,  1431,   351,   662,
     980,  1155, -1002,   352,  1233,   944,   945,  2071,  1195,  1196,
    -965,  -941,   266,  -955,   384,  -942,  -962,  1425,  -956,   204,
    -582,  1446,  1265,   653,   385,   655,   226,   708,  2089,  -954,
    1426,  1432,  1333,  1334,  1304,   226,   538,   941,  1765,   721,
    -961,  1156,   728,   685,  -651,  1449,   737,   269,  1362,  1596,
    1364,  1427,  1368,  1207,   226,   501,  -964,   740,   742,  -958,
    1718,   784,  -957,  1871,  1872,   681,  1873,   502,   743,   744,
     745,   746,   748,   749,   750,   751,   752,   753,   754,   755,
     756,   757,   758,   759,   760,   761,   762,   763,   764,   765,
     766,   767,   768,   769,   770,   771,  1734,   773,  -463,   776,
     740,   740,   779,   414,   790,   791,   457,   351,  -774,  2079,
    1293,   248,   798,   799,   800,   801,   802,   803,   804,   805,
     806,   807,   808,   809,   810,   270,   420,   774,  1336,   253,
    1524,  2096,   721,   721,   740,   822,   255,   657,   776,   798,
     795,   254,   825,  1423,  1505,  1046,   796,  -971,  -962,   117,
    -956,   833,   835,  1245,  -772,   515,   917,  1246,  -968,   271,
     721,  -954,   694,   539,   532,   533,   132,   786,   854,  -973,
     855,  -110,  -961,   538,   229,   229,   731,   726,  1333,  1334,
    1276,  1001,   363,  1002,   386,   213,    40,  -110,   126,   624,
    1045,   401,   982,   818,  -957,  -653,  1309,   395,   275,   840,
    1444,   350,  2080,   532,   533,   680,   683,  1607,  1998,   682,
    1284,   415,   917,   786,   684,   387,   858,   514,   390,   918,
    1330,   931,  1057,   842,  2097, -1069,   848,  -463,  1053,  1597,
    -463,   401,  1661,   382,   775,  1008,  1009,  1997,   421,   664,
    1347,   412,   383,   390,  1350,   422,   442,   797,   390,   390,
    1245,  1354,   441,   388,  1246,  -767,   401,  1257,   375,  1257,
    1459,   532,   533,  1461,   664, -1069,   171,   659,   404,   405,
     441,   427,   793,   389,  1517,   441,   390,   532,   533,   451,
     393,   228,   230,   394,   401,   924,   410,  -768,  1488,   411,
    1490,   401,   402,  1259,  1495,   540,  1261,  1262,   563,   664,
    1098,  1103,  1104,   853,   432,   669,   670,   112,   404,   405,
     401,  1185,  2048,  1011,   150,  1001,  1002,  1190,   437,  1191,
     396,  1192,  1097,  1099,  -969,  1194,  1414,  1415,  1741,   224,
     224,  1033,  -109,   404,   405,   397,  1926,   401,  1316,  1317,
    -969,   398,  1036,  1293,  1484,   664,  2066,  1484,  -109,  2049,
    2050,  2051,   549,  1980,  1497,  1927,   711,  1726,  1928,  1728,
     403,   404,   405,  -928,   399,   226,   416,   665,   404,   405,
    -931,   429,  1981,   680,   683,  1982,  -929,   682,  1044,  -928,
     710,   417,   684,  2049,  2050,  2051,  -931,   404,   405,  1474,
    1414,  1415,  -929,  1616,   229,  1287,  1288,  1289,   212,   440,
    1593,  1479,   498,   499,   500,   444,   501,  1056,   117,  1465,
    1454,   441,   117,    55,   404,   405,   582, -1069,   502,   447,
    1475,    50,   458,   186,   187,    65,    66,    67,   453,   694,
    1668,  1669,   458,   186,   187,    65,    66,    67,   442,  1101,
    1950,  1951,   516,   226,  -126,  2074,  2075,   579,  -126,  1332,
    1333,  1334,   429,  1107, -1069,   266,  1611, -1069,   216,   217,
     218,   219,   220,  1532,   452,  -126,   132,  1536,   465,  1180,
    1181,  1182,  1542,   466,  1888,  1257,  1978,  1979,  1892,   467,
    1548,   226,   468,   226,   429,  1183,    93,    94,   126,    95,
     189,    97,  1974,  1975,  1027,   459,   611,   613,   615,   469,
     618,   555,   602,  1118,   470,   459,   562,  1707,  1552,  1244,
    1247,   226,  2044,   646,   647,   108,  1511,  1333,  1334,   660,
     471,   472,  1622,   666,  1623,   171,   680,   683,  -644,   171,
     682,  -645,   229,  -646,  -649,   684,   436,   438,   439,  -647,
     784,   229,  -648,   641,   505,   506,   507,   508,  1212,   224,
    1213,   660,   855,   666,   660,   666,   666,   537,  -963,  -650,
     229,  -766,   408,  1215,   375,   375,   375,   617,   375,   546,
     548,   502,   442,   150,   554,  -967,   117,   150,  1225,   538,
    -764,   564,   565,   573,   226,   132,   712,   586, -1114,  1627,
     350,   594,   597,  1631,   598,   604,   605,   621,  1613,   390,
    1638,   623,  1766,   620,  1642,   630,   667,   126,  1253,   123,
     633,   728,   634,   728,  1735,   643,   644,   686,   798,  1650,
     687,   696,   697,   127,   698,   700,   129,   130,  -131,   731,
      55,   828,  1271,   681,   709,  1272,   736,  1273,   826,   627,
    1953,   721,   659,   132,  1956,  1283,  1227,   830,   635,  1452,
     640,   831,  1453,   602,   390,   788,   390,   390,   390,   390,
     818,   818,   837,   856,   248,   126,   838,   663,   575,   860,
    2036,   863,  1737,   592,  1738,   877,  1739,   878,   727,   813,
     909,  1740,   253,  1234,   911,   912,   914,   224,  1234,   255,
    1313,   934,  2036,   171,   254,   913,   224,   915,   916,   602,
    1319,  2059,   919,   949,   920,   938,   132,   939,  1371,   730,
     167,   942,   845,  1234,   943,   224,   733,   734,   735,   954,
      34,    35,    36,   117,   955,   957,   959,  1994,   126,   132,
     960,   961,  1999,   962,   214,   694,   968,   973,  1722,  1723,
     974,   150,  1320,   906,  1743,   976,  1256,  -789,  1256,   983,
     984,   126,  1749,   797,  1439,  1440,   848,   986,   848,   226,
     226,   132,   680,   683,   987,   988,   682,   694,  1756,   930,
     712,   684,  1441,   991,  1234,   458,   186,   187,    65,    66,
      67,  2024,  1896,   126,   996,   995,  1004,  1012,  1006,    81,
      82,    83,    84,    85,  2058,  1013,  1015,  1018,  1019,  1020,
     221,   681,   965,   967,  1030,  1034,    89,    90,  1042,  1021,
    1022,  2072,  1050,  1058,  1059,  1032,  1038,  1430,   928,   928,
      99,  1039,  1041,   495,   496,   497,   498,   499,   500,   728,
     501,  1060,  2060,  -770,  1100,   105,  2061,  1110,  1120,  1122,
     171,   132,   502,   132,   740,  1124,  1128,  1457,   459,  1903,
    2077,  2078,  1129,  1130,  1146,   123,  2085,  1147,   680,   683,
    1132,  1148,   682,   126,  1149,   126,  1150,   684,  1152,   127,
     721,   229,   129,   130,  1151,  1153,   117,  1188,  1198,  1202,
    1993,   721,  1996,  1200,   390,  1204,  1205,  1206,   150,  1211,
    1214,  1220,  1233,  1222,  1223,  1224,  1230,  1233,  1228,   226,
    1235,  1249,  1266,  1275,   535,  1177,  1178,  1179,  1180,  1181,
    1182,   579,  1281,  1500,  1278,  1296,  1298,  1286,  -972,   266,
    1297,  1299,  1233,  1300,  1183,  1305,  1301,  1302,  1513,  1306,
     458,    63,    64,    65,    66,    67,  1307,  1308,  1310,  1327,
    1322,    72,   509,  1957,  1958,  1324,  -966,  1329,  1338,   229,
    1339,  1340,   132,   428,   681,  1346,   167,  1503,  1348, -1130,
     425,   226,   430,  1183,  1256,  1562,  1349,  1352,  1404,  1353,
    2047,  1078,  1406,  1092,   126,  1407,   226,  1418,  1007,  1408,
    1411,  1409,   510,  1233,   511,  1434,  1435,   229,  1447,   229,
    1450,  1046,  1451,   171,  1462,   117,  1476,  1458,   512,  1464,
     513,  1478,  1959,   459,  1460,   694,  1466,    14,   694,  1116,
     117,  1477,  1467,  1469,  1470,  1584,  1068,   229,  1491,  1969,
    1971,  1499,  1589,  1501,  1502,  1504,   224,  1591,  1508,  1592,
    1512,  1708,   728,  1518,  1521,  1525,  1530,  1526,  1529,  1531,
    1535,   150,  1540,  1534,  1537,  1603,  1538,   461,  1541,  1539,
    1543,  1546,  1544,   117,  1547,  1553,  1055,  1601,  1612,   721,
    1551,  1086,  1193,   712,  1585,   132,  1586,  1554,  1555,  1602,
    1604,  1556,  1563,  1595,  1605,  1618,  1608,  1606,  1564,  1610,
     458,  1565,   187,  1566,  1567,  1568,  1569,   126,  1614,  1619,
     229,  1620,  1626,  1208,  1093,  1621,  1094,  1624,  1630,  1628,
    1635,  1637,  1641,  1625,   224,  1629,  1632,  1633,  1636,   928,
    1634,   928,   171,   928,  1639,  1644,   117,   928,  1643,   928,
     928,  1197,   212,  1645,  1114,  1651,  1647,   171,  1570,  1571,
    1648,  1572,   602,  1654,  1869,  1676,  1665,  -450,  -449,   117,
    -448,  1689,   224,  1697,   224,    50,   813,   813,  1701,  1706,
    1711,  1716,  1719,   459,  1577,  1720,  1724,  1732,  1583,  1577,
     150,  1736,  1587,  1583,  1729,  1730,  1751,   726,  1760,  1769,
     171,   117,  1113,  1768,  1754,   150,  2084,  1866,  1865,   461,
    1874,  1882,   216,   217,   218,   219,   220,  1883,  1885,  1886,
     681,  1715,  1887,  1889,  1895,  1897,  1721,  1203,   390,   721,
     721,   694,  1898,  1908,  1930,  1909,  1934,  1290,  1290,  1078,
      93,    94,  1935,    95,   189,    97,  1940,  1941,   150,   458,
      63,    64,    65,    66,    67,  1968,  1761,  1970,  1946,  1976,
      72,   509,  1972,   171,   212,  1983,   213,    40,  1984,   108,
     736,  1985,   845,  1991,   845,   224,  1992,  1995,  2000,   117,
    2004,   117,  2005,   117,  -388,  2007,   171,    50,  2008,  2010,
    2012,  1924,  1759,  2019,  2016,   229,   229,  2025,  2026,  2032,
    2027,  2033,  2038,   511,  1342,  2046,  2042,  2045,  1258,  2055,
    1258,   150,  2057,  2068,  1747,  2081,   681,   132,   171,  2082,
    2073,  2062,   459,  2086,   216,   217,   218,   219,   220,  1086,
     602,  2063,  2090,  2091,   150,  2098,  2099,  2101,  2041,   126,
    1937,  2102,  1410,  1277,  1884,  1219,   789,   792,  1709,  2056,
     811,  1506,    93,    94,   787,    95,   189,    97,  1902,   906,
    1649,   132,  2054,  1699,  1456,   932,   150,  1893,  1917,  1767,
     212,  1922,  2093,  1696,  1577,  2067,  1677,  1891,  1583,   654,
    1577,   108,  1577,   126,  1583,   812,  1583,  1363,   112,   694,
    1489,  1428,   117,    50,  1355,  1292,   171,  1481,   171,  1480,
     171,  1311,  1114,  1326,  1988,  1577,   722,   132,  2021,  1583,
    2014,  1667,  1900,  1747,  1138,   132,  1344,  1413,  1403,  1442,
       0,     0,     0,     0,     0,     0,   928,     0,     0,   126,
     216,   217,   218,   219,   220,   229,     0,   126,     0,     0,
       0,     0,     0,     0,   150,     0,   150,     0,   150,     0,
    1113,   224,   351,     0,     0,   408,     0,  1931,    93,    94,
       0,    95,   189,    97,     0,     0,     0,     0,     0,     0,
       0,     0,  1078,  1078,  1078,  1078,  1078,  1078,     0,     0,
       0,  1078,     0,  1078,     0,     0,  2030,   108,     0,     0,
       0,   409,     0,     0,   117,  1875,     0,   229,     0,  1577,
       0,     0,     0,  1583,     0,   117,     0,  1942,   132,   171,
    1562,     0,   229,     0,   132,  1520,     0,     0,     0,     0,
     351,   132,     0,     0,     0,  1931,  1258,     0,  1562,     0,
     126,   461,     0,     0,     0,     0,   126,     0,     0,     0,
       0,     0,  1455,   126,     0,     0,     0,     0,     0,     0,
       0,     0,    14,     0,     0,     0,     0,   150,     0,     0,
       0,     0,  1086,  1086,  1086,  1086,  1086,  1086,   223,   223,
      14,  1086,     0,  1086,     0,     0,     0,     0,     0,   245,
       0,     0,     0,     0,     0,     0,     0,     0,  1557,     0,
     224,   544,   518,   519,   520,   521,   522,   523,   524,   525,
     526,   527,   528,   529,  1498,   245,     0,     0,     0,     0,
       0,   171,     0,     0,     0,     0,     0,  1563,     0,  1114,
       0,     0,   171,  1564,     0,   458,  1565,   187,  1566,  1567,
    1568,  1569,     0,     0,     0,  1563,   530,   531,     0,     0,
       0,  1564,     0,   458,  1565,   187,  1566,  1567,  1568,  1569,
       0,  1078,   224,  1078,     0,     0,     0,     0,     0,   150,
       0,     0,     0,     0,     0,     0,     0,  1113,   132,     0,
     150,     0,     0,  1570,  1571,     0,  1572, -1132, -1132, -1132,
   -1132, -1132,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
     126,  1570,  1571,     0,  1572,     0,     0,     0,   459,     0,
       0,     0,     0,  1183,     0,   212,  2092,  1727,     0,     0,
       0,     0,     0,   532,   533,  2100,   459,     0,     0,     0,
       0,     0,     0,  2103,   132,  1731,  2104,   117,    50,  1594,
       0,     0,     0,     0,     0,     0,     0,     0,   350,     0,
       0,  1086,     0,  1086,     0,     0,   126,  1694,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   132,
       0,     0,     0,     0,   132,   216,   217,   218,   219,   220,
       0,   117,     0,  2031,     0,     0,     0,   699,     0,     0,
       0,   126,     0,     0,   694,     0,   126,   188,   223,   132,
      91,     0,     0,    93,    94,     0,    95,   189,    97,     0,
       0,  1078,     0,  1078,     0,  1078,   694,     0,     0,     0,
    1078,   126,     0,     0,     0,   694,     0,   117,     0,     0,
       0,   117,   108,     0,     0,   117,     0,  1962,     0,     0,
       0,     0,  1562,     0,     0,     0,     0,   245,     0,   245,
       0,     0,     0,     0,   171,   390,     0,     0,   602,   132,
     132,   350,     0,     0,     0,  1562,     0,     0,     0,     0,
       0,  1854,     0,     0,     0,     0,     0,   870,  1861,     0,
       0,   126,   126,     0,    14,     0,   350,   350,     0,   350,
       0,     0,     0,     0,     0,   350,     0,     0,   171,     0,
       0,  1086,   150,  1086,     0,  1086,     0,    14,   245,     0,
    1086,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1078,     0,     0,     0,     0,     0,   212,   117,   117,
     117,     0,     0,     0,   117,     0,   223,     0,   871,     0,
       0,   117,     0,     0,   171,   223,   150,     0,   171,  1563,
      50,     0,   171,     0,     0,  1564,     0,   458,  1565,   187,
    1566,  1567,  1568,  1569,   223,     0,     0,     0,     0,     0,
       0,     0,  1563,     0,     0,   223,     0,     0,  1564,     0,
     458,  1565,   187,  1566,  1567,  1568,  1569,   216,   217,   218,
     219,   220,   150,     0,     0,     0,   150,     0,     0,     0,
     150,   245,     0,     0,   245,  1570,  1571,     0,  1572,   188,
       0,  1086,    91,     0,  1562,    93,    94,     0,    95,   189,
      97,     0,   872,     0,     0,     0,     0,     0,  1570,  1571,
     459,  1572,     0,     0,     0,     0,     0,     0,     0,  1733,
       0,     0,     0,     0,   108,   171,   171,   171,     0,     0,
       0,   171,     0,   459,   602,     0,    14,     0,   171,     0,
     245,     0,  1742,     0,     0,   544,   518,   519,   520,   521,
     522,   523,   524,   525,   526,   527,   528,   529,   350,     0,
       0,     0,  1078,  1078,     0,     0,     0,     0,   117,     0,
       0,     0,     0,   150,   150,   150,     0,  1963,     0,   150,
       0,     0,     0,     0,  1854,  1854,   150,     0,  1861,  1861,
     530,   531,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1563,   602,     0,     0,     0,     0,  1564,     0,   458,
    1565,   187,  1566,  1567,  1568,  1569,     0,     0,     0,     0,
       0,     0,     0,     0,   117,     0,     0,     0,     0,     0,
       0,   245,     0,   245,     0,     0,   893,     0,   517,   518,
     519,   520,   521,   522,   523,   524,   525,   526,   527,   528,
     529,     0,  1086,  1086,     0,     0,     0,  1570,  1571,   117,
    1572,     0,     0,     0,   117,     0,     0,   532,   533,   893,
       0,     0,  2029,     0,     0,     0,   473,   474,   475,     0,
       0,     0,   459,   530,   531,   171,     0,     0,     0,   117,
       0,  1894,     0,     0,     0,  2043,   476,   477,     0,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,     0,   501,     0,   245,   245,     0,     0,     0,
       0,   839,     0,   150,   245,   502,     0,     0,     0,     0,
       0,   171,     0,     0,     0,     0,     0,     0,     0,   117,
     117,     0,     0,     0,     0,   223,     0,     0,     0,     0,
     532,   533,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   171,     0,     0,     0,
       0,   171,     0,     0,     0,     0,     0,     0,     0,   150,
     544,   518,   519,   520,   521,   522,   523,   524,   525,   526,
     527,   528,   529,     0,     0,     0,   171,     0,     0,     0,
    1023,   518,   519,   520,   521,   522,   523,   524,   525,   526,
     527,   528,   529,     0,   150,     0,     0,     0,     0,   150,
       0,     0,     0,   223,     0,   530,   531,     0,   353,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1157,  1158,  1159,   150,   530,   531,     0,     0,     0,
       0,     0,     0,     0,     0,   245,   171,   171,     0,     0,
    1421,   223,  1160,   223,     0,  1161,  1162,  1163,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,     0,     0,     0,
       0,   223,   893,     0,     0,     0,     0,     0,     0,   245,
    1183,     0,   532,   533,   150,   150,   245,   245,   893,   893,
     893,   893,   893,     0,     0,     0,   473,   474,   475,     0,
       0,   893,   532,   533,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   476,   477,   245,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   288,   501,   223,     0,   940,     0,     0,     0,
       0,     0,     0,     0,     0,   502,     0,     0,   245,     0,
       0,     0,     0,     0,     0,     0,  1024,     0,     0,     0,
       0,     0,     0,   225,   225,     0,     0,     0,     0,   290,
       0,     0,   245,   245,   247,     0,     0,     0,     0,     0,
       0,     0,   212,   223,     0,     0,     0,     0,     0,     0,
     245,     0,     0,     0,     0,     0,     0,   245,     0,     0,
       0,  1369,     0,   245,     0,    50,     0,     0,     0,     0,
       0,     0,     0,   595,   893,     0,  1023,   518,   519,   520,
     521,   522,   523,   524,   525,   526,   527,   528,   529,   245,
     353,     0,   353,     0,     0,     0,     0,     0,     0,     0,
       0,   588,   216,   217,   218,   219,   220,   589,     0,   245,
       0,     0,     0,   245,     0,     0,     0,     0,     0,     0,
       0,   530,   531,   245,   188,     0,     0,    91,   343,     0,
      93,    94,     0,    95,   189,    97,     0,   503,     0,  1035,
       0,     0,     0,     0,     0,     0,     0,     0,   347,     0,
       0,   353,     0,     0,     0,     0,     0,     0,     0,   108,
     349,     0,     0,     0,     0,     0,     0,     0,     0,   223,
     223,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   245,     0,     0,     0,   245,     0,   245,
       0,     0,   245,     0,     0,     0,     0,     0,   532,   533,
       0,     0,     0,     0,     0,   893,   893,   893,   893,   893,
     893,   223,   893,     0,     0,   893,   893,   893,   893,   893,
     893,   893,   893,   893,   893,   893,   893,   893,   893,   893,
     893,   893,   893,   893,   893,   893,   893,   893,   893,   893,
     893,   893,   893,   225,   353,     0,     0,   353,   473,   474,
     475,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   893,   699,     0,     0,     0,     0,     0,   476,   477,
       0,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   245,   501,   245,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   502,     0,   223,
       0,     0,   288,     0,     0,   212,     0,   213,    40, -1132,
   -1132, -1132, -1132, -1132,   493,   494,   495,   496,   497,   498,
     499,   500,     0,   501,     0,     0,     0,   245,    50,     0,
     245,     0,     0,     0,     0,   502,     0,     0,     0,   290,
       0,     0,     0,     0,     0,     0,   245,   245,   245,   245,
     245,   245,   212,     0,   223,   245,     0,   245,  1133,     0,
       0,   223,     0,     0,     0,   216,   217,   218,   219,   220,
       0,   225,     0,     0,     0,    50,   223,     0,   893,     0,
     225,     0,     0,     0,   353,     0,   873,     0,   245,     0,
       0,   811,     0,    93,    94,   245,    95,   189,    97,   225,
     893,     0,   893,     0,     0,     0,     0,     0,     0,     0,
     225,   588,   216,   217,   218,   219,   220,   589,     0,     0,
       0,     0,   108,     0,     0,     0,   844,   893,     0,   112,
       0,   969,     0,     0,   188,     0,     0,    91,   343,     0,
      93,    94,     0,    95,   189,    97,     0, -1131,     0,     0,
       0,   357,     0,     0,     0,     0,     0,     0,   347,     0,
       0,     0,     0,   245,   245,     0,     0,   245,     0,   108,
     349,     0,     0,     0,     0,     0,     0,     0,   353,   353,
       0,     0,     0,     0,     0,     0,     0,   353,     0,     0,
       0,     0,     0,     0,   245,   247,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   473,   474,
     475,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   245,     0,   245,   476,   477,
       0,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,     0,   501,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   502,     0,     0,
     245,   245,     0,     0,   245,     0,     0,     0,     0,     0,
     893,     0,   893,     0,   893,     0,     0,     0,     0,   893,
     223,   901,     0,     0,   893,     0,   893,     0,     0,   893,
       0,     0,     0,     0,     0,     0,     0,   473,   474,   475,
       0,     0,   245,   245,     0,     0,     0,     0,     0,   245,
       0,     0,     0,     0,   901,     0,   245,   476,   477,     0,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,     0,   501,     0,     0,     0,     0,     0,
       0,     0,  1127,     0,     0,     0,   502,     0,     0,   353,
     353,     0,     0,     0,     0,   245,     0,   245,     0,   245,
       0,     0,     0,   590,   245,   591,   223,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     245,  1003,     0,     0,     0,   893,     0,     0,     0,     0,
     225,     0,     0,     0,     0,     0,     0,   245,   245,     0,
       0,     0,     0,     0,     0,   245,     0,   245,   544,   518,
     519,   520,   521,   522,   523,   524,   525,   526,   527,   528,
     529,     0,     0,     0,   596,     0,     0,     0,     0,     0,
     245,   245,     0,   245,     0,     0,     0,     0,   245,   245,
       0,     0,     0,     0,     0,   353,     0,     0,     0,     0,
       0,     0,     0,   530,   531,     0,     0,     0,     0,     0,
       0,     0,     0,   353,     0,   245,     0,     0,   225,     0,
     353,     0,     0,     0,     0,     0,   353,     0,   288,     0,
    1035,     0,   893,   893,   893,     0,     0,     0,     0,   893,
       0,   245,     0,     0,     0,     0,     0,   245,     0,   245,
    1085,     0,     0,     0,     0,     0,   225,     0,   225,     0,
       0,     0,     0,     0,     0,   290,     0,   715,     0,     0,
     357,     0,   353,     0,     0,     0,     0,     0,   212,     0,
     532,   533,   227,   227,   998,     0,   225,   901,     0,     0,
       0,     0,     0,   251,     0,     0,     0,     0,     0,     0,
       0,    50,     0,   901,   901,   901,   901,   901,     0,     0,
       0,     0,     0,     0,     0,     0,   901,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1187,     0,     0,     0,   588,   216,   217,
     218,   219,   220,   589,     0,   893,   353,     0,     0,     0,
     353,     0,   873,   245,     0,   353,     0,     0,     0,   225,
     188,     0,     0,    91,   343,     0,    93,    94,   245,    95,
     189,    97,   245,  1210,     0,     0,   245,   245,     0,     0,
     212,     0,     0,     0,   347,     0,     0,     0,     0,     0,
       0,   245,     0,     0,     0,   108,   349,   893,  1210,     0,
       0,     0,     0,    50,     0,     0,     0,     0,   225,     0,
       0,     0,     0,   893,   893,     0,     0,   869,     0,     0,
     893,  1023,   518,   519,   520,   521,   522,   523,   524,   525,
     526,   527,   528,   529,     0,     0,     0,     0,     0,   901,
     216,   217,   218,   219,   220,     0,     0,   245,     0,     0,
       0,     0,     0,     0,  1267,     0,     0,   353,     0,   353,
       0,     0,   893,     0,     0,     0,   530,   531,    93,    94,
       0,    95,   189,    97,   245,     0,   245,     0,   247,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1085,     0,
       0,     0,     0,     0,     0,     0,     0,   108,  1032,     0,
     353,     0,   227,   353,     0,   245,     0,     0,     0,     0,
       0,   989,   990,     0,     0,     0,     0,     0,     0,     0,
       0,   245,     0,     0,     0,     0,     0,   245,     0,     0,
       0,   245,     0,     0,   225,   225,     0,     0,     0,     0,
       0,     0,     0,   532,   533,   245,   245,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   353,     0,     0,     0,     0,     0,     0,   353,     0,
     901,   901,   901,   901,   901,   901,   225,   901,     0,  1111,
     901,   901,   901,   901,   901,   901,   901,   901,   901,   901,
     901,   901,   901,   901,   901,   901,   901,   901,   901,   901,
     901,   901,   901,   901,   901,   901,   901,   901,     0,     0,
       0,    29,     0,     0,     0,     0,     0,     0,     0,    34,
      35,    36,   212,     0,   213,    40,   901,     0,     0,     0,
       0,     0,     0,   214,     0,     0,   353,   353,     0,     0,
     227,     0,     0,     0,     0,    50,     0,     0,     0,   227,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   215,     0,     0,     0,     0,   353,   227,     0,
       0,     0,     0,     0,   225,     0,     0,     0,     0,   251,
      74,    75,   216,   217,   218,   219,   220,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,   288,   221,
       0,     0,  1136,   715,   188,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   189,    97,     0,     0,     0,    99,
       0,  1085,  1085,  1085,  1085,  1085,  1085,     0,     0,   225,
    1085,     0,  1085,     0,   105,   290,   225,     0,     0,   108,
     222,     0,     0,   353,   353,     0,   112,   353,   212,     0,
       0,   225,     0,   901,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   251,     0,     0,     0,     0,     0,
       0,    50,  1678,     0,     0,   901,     0,   901,     0,  -435,
       0,     0,     0,     0,     0,   353,     0,     0,   458,   186,
     187,    65,    66,    67,     0,     0,     0,     0,  1218,   353,
       0,     0,   901,     0,     0,     0,     0,   588,   216,   217,
     218,   219,   220,   589,     0,     0,  1229,     0,     0,     0,
       0,     0,   212,     0,     0,     0,     0,     0,   212,  1248,
     188,     0,     0,    91,   343,     0,    93,    94,     0,    95,
     189,    97,  1561,     0,     0,    50,     0,     0,     0,     0,
       0,    50,     0,     0,   347,     0,     0,     0,     0,     0,
     902,   459,     0,     0,     0,   108,   349,     0,  1679,     0,
       0,     0,     0,   353,     0,  1280,     0,  1692,     0,     0,
       0,  1680,   216,   217,   218,   219,   220,  1681,   216,   217,
     218,   219,   220,   902,     0,     0,     0,     0,   353,     0,
    1085,     0,  1085,     0,   188,     0,     0,    91,  1682,     0,
      93,    94,     0,    95,  1683,    97,    93,    94,     0,    95,
     189,    97,     0,   353,   353,     0,   353,     0,     0,     0,
       0,   353,   353,     0,     0,     0,     0,     0,     0,   108,
       0,     0,     0,     0,     0,   108,  1693,     0,     0,  1337,
       0,     0,     0,  1341,     0,   901,     0,   901,  1345,   901,
       0,     0,     0,     0,   901,   225,     0,     0,     0,   901,
       0,   901,     0,     0,   901,   473,   474,   475,     0,   227,
       0,     0,     0,     0,     0,     0,     0,     0,  1675,     0,
     353,     0,     0,     0,  1688,   476,   477,     0,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   212,   501,   963,     0,   964,     0,     0,     0,     0,
       0,     0,     0,     0,   502,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
    1085,     0,  1085,     0,  1085,     0,     0,   227,     0,  1085,
    1443,   225,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     901,   216,   217,   218,   219,   220,   903,     0,     0,     0,
     288,     0,  1757,  1758,     0,   227,   353,   227,     0,     0,
       0,     0,  1688,  1468,     0,     0,  1471,     0,     0,    93,
      94,   353,    95,   189,    97,   353,     0,     0,     0,   933,
       0,     0,     0,     0,     0,   227,   902,   290,     0,     0,
       0,     0,     0,     0,  1964,     0,     0,     0,   108,     0,
     212,     0,   902,   902,   902,   902,   902,     0,     0,     0,
       0,     0,     0,     0,     0,   902,     0,     0,     0,     0,
    1085,     0,     0,    50,  1519,     0,     0,     0,  1061,     0,
       0,  1523,     0,     0,     0,     0,     0,   901,   901,   901,
       0,     0,     0,     0,   901,     0,  1911,     0,     0,     0,
     353,     0,     0,     0,  1688,     0,     0,     0,   227,   588,
     216,   217,   218,   219,   220,   589,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   353,     0,   353,
       0,     0,   188,     0,     0,    91,   343,     0,    93,    94,
       0,    95,   189,    97,     0,     0,     0,     0,     0,  1558,
    1559,     0,     0,     0,     0,     0,   347,   251,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   108,   349,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     353,   473,   474,   475,   353,     0,     0,     0,   902,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   353,   353,
     901,   476,   477,     0,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   251,   501,     0,
       0,  1085,  1085,     0,     0,     0,     0,     0,     0,     0,
     502,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   901,     0,     0,   870,  1652,  1653,     0,     0,
    1655,     0,  1115,     0,     0,     0,     0,     0,   901,   901,
       0,     0,     0,     0,     0,   901,     0,     0,  1139,  1140,
    1141,  1142,  1143,   227,   227,     0,     0,     0,     0,     0,
       0,  1154,     0,     0,     0,     0,     0,     0,  1673,     0,
       0,     0,     0,     0,     0,   212,     0,     0,     0,     0,
       0,     0,  1700,     0,     0,     0,   871,   901,     0,   902,
     902,   902,   902,   902,   902,   251,   902,     0,    50,   902,
     902,   902,   902,   902,   902,   902,   902,   902,   902,   902,
     902,   902,   902,   902,   902,   902,   902,   902,   902,   902,
     902,   902,   902,   902,   902,   902,   902,     0,     0,     0,
    2040,     0,     0,     0,     0,   216,   217,   218,   219,   220,
       0,     0,     0,     0,  1201,   902,  1675,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1750,   188,     0,     0,
      91,     0,     0,    93,    94,     0,    95,   189,    97,     0,
    1343,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1673,     0,     0,  1254,     0,     0,     0,     0,   212,
       0,     0,   108,   227,     0,     0,     0,   473,   474,   475,
       0,     0,     0,     0,     0,     0,  1673,  1673,     0,  1673,
       0,     0,    50,     0,  1877,  1673,     0,   476,   477,     0,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,     0,   501,     0,     0,     0,   251,   216,
     217,   218,   219,   220,     0,   227,   502,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     227,     0,   902,  1913,     0,  1859,     0,    93,    94,  1860,
      95,   189,    97,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   902,     0,   902,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   108,  1693,     0,     0,
       0,     0,     0,     0,     0,  1143,  1358,     0,     0,  1358,
       0,   902,     0,     0,     0,  1372,  1375,  1376,  1377,  1379,
    1380,  1381,  1382,  1383,  1384,  1385,  1386,  1387,  1388,  1389,
    1390,  1391,  1392,  1393,  1394,  1395,  1396,  1397,  1398,  1399,
    1400,  1401,  1402,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   212,     0,     0,     0,     0,     0,
       0,  1412,     0,     0,     0,  1126,   473,   474,   475,  1936,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
    1274,     0,     0,     0,  1949,   288,   476,   477,  1673,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,     0,   501,   216,   217,   218,   219,   220,     0,
       0,     0,   290,     0,     0,   502,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   212,   188,     0,     0,    91,
       0,  1522,    93,    94,     0,    95,   189,    97,     0,     0,
       0,     0,   288,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,  2009,   902,     0,   902,     0,   902,     0,
       0,   108,     0,   902,   251,     0,     0,     0,   902,     0,
     902,     0,     0,   902,     0,     0,     0,     0,  1509,   290,
    1949,     0,  2022,     0,   588,   216,   217,   218,   219,   220,
     589,     0,   212,     0,  1697,     0,     0,     0,     0,     0,
    1527,     0,  1528,     0,     0,     0,     0,   188,     0,     0,
      91,   343,     0,    93,    94,    50,    95,   189,    97,     0,
   -1131,     0,     0,     0,     0,     0,     0,  1549,     0,     0,
       0,   347,   458,   186,   187,    65,    66,    67,     0,     0,
       0,     0,   108,   349,     0,     0,     0,     0,     0,  1285,
       0,   588,   216,   217,   218,   219,   220,   589,     0,     0,
     251,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   188,     0,     0,    91,   343,   902,
      93,    94,     0,    95,   189,    97,     0,     0,     0,     0,
       0,     0,     5,     6,     7,     8,     9,     0,   347,     0,
       0,     0,    10,     0,     0,   459,     0,     0,     0,   108,
     349,     0,     0,     0,     0,     0,   358,   424,    13,     0,
       0,     0,     0,     0,     0,     0,     0,   794,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,   902,   902,   902,     0,
    1657,    43,  1658,   902,  1659,     0,     0,     0,     0,  1660,
       0,     0,  1916,    50,  1662,     0,  1663,     0,     0,  1664,
       0,    55,     0,     0,     0,     0,     0,     0,     0,     0,
     185,   186,   187,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   188,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   189,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,   902,
     473,   474,   475,     0,   112,   113,   114,   115,     0,     0,
       0,     0,     0,     0,     0,  1752,     0,     0,     0,     0,
     476,   477,     0,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,     0,   501,     0,     0,
       0,   902,     0,     0,     0,     0,     0,     0,     0,   502,
       0,     0,     0,     0,     0,     0,     0,   902,   902,     0,
       0,     0,     0,     0,   902,     0,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,  2011,     0,     0,     0,     0,     0,     0,
       0,     0,  1904,  1905,  1906,     0,   902,     0,     0,  1910,
       0,    14,     0,    15,    16,     0,     0,     0,   547,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,    56,    57,    58,     0,
      59,  -205,    60,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,  1929,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,    88,    89,    90,    91,
      92,     0,    93,    94,     0,    95,    96,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,   103,     0,   104,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,     0,  1973,   112,   113,
     114,   115,     0,     0,     0,     0,     0,     0,     5,     6,
       7,     8,     9,  1986,  1987,     0,     0,     0,    10,     0,
    1990,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,  2013,    18,    19,    20,    21,    22,    23,    24,
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
     107,     0,     0,   108,   109,     0,   110,   111,  1216,     0,
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
     109,     0,   110,   111,  1422,     0,   112,   113,   114,   115,
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
     188,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     189,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
     701,     0,   112,   113,   114,   115,     5,     6,     7,     8,
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
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,  1025,     0,   112,   113,
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
      54,    55,     0,    57,    58,     0,    59,  -205,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   188,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   189,    97,    98,     0,     0,    99,     0,     0,
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
      86,     0,     0,    87,     0,     0,     0,     0,   188,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   189,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,  1186,     0,
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
       0,     0,     0,     0,   188,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   189,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,  1231,     0,   112,   113,   114,   115,
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
     188,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     189,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
    1263,     0,   112,   113,   114,   115,     5,     6,     7,     8,
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
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,  1321,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,     0,    42,     0,     0,
       0,    43,    44,    45,    46,  1323,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   188,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   189,    97,    98,     0,     0,    99,     0,     0,
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
      46,     0,    47,     0,    48,     0,    49,  1510,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   188,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   189,    97,
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
       0,     0,     0,     0,   188,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   189,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,  1666,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,  -297,    34,    35,    36,    37,    38,
      39,    40,     0,    41,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     188,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     189,    97,    98,     0,     0,    99,     0,     0,   100,     0,
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
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,  1907,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,     0,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,  1960,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   188,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   189,    97,    98,     0,     0,    99,     0,     0,
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
      46,     0,    47,  2003,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   188,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   189,    97,
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
       0,     0,     0,     0,   188,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   189,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,  2020,     0,   112,   113,   114,   115,
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
     188,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     189,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
    2023,     0,   112,   113,   114,   115,     5,     6,     7,     8,
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
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,  2039,     0,   112,   113,
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
       0,     0,   188,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   189,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,  2094,     0,   112,   113,   114,   115,     5,     6,
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
      86,     0,     0,    87,     0,     0,     0,     0,   188,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   189,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,  2095,     0,
     112,   113,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,   571,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,   188,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   189,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,     0,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,   857,
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
     188,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     189,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
       0,     0,   112,   113,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,  1117,     0,     0,     0,     0,
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
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,     0,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,  1746,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,   188,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   189,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,     0,     0,   112,   113,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,  1899,     0,     0,
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
      86,     0,     0,    87,     0,     0,     0,     0,   188,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   189,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,     0,     0,
     112,   113,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,   188,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   189,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,     0,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   358,     0,    13,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     188,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     189,    97,     0,     0,     0,    99,     0,     0,   100,     5,
       6,     7,     8,     9,   101,   102,     0,     0,     0,    10,
     105,   106,   107,     0,     0,   108,   190,     0,   359,     0,
       0,     0,   112,   113,   114,   115,     0,     0,     0,     0,
       0,     0,     0,     0,   716,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,   717,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,   185,   186,   187,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
      97,     0,   718,     0,    99,     0,     0,   100,     5,     6,
       7,     8,     9,   101,   102,     0,     0,     0,    10,   105,
     106,   107,     0,     0,   108,   190,     0,     0,     0,     0,
       0,   112,   113,   114,   115,     0,     0,     0,     0,     0,
       0,     0,     0,  1250,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,  1251,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,   185,   186,   187,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   188,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   189,    97,
       0,  1252,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   190,     5,     6,     7,     8,     9,
     112,   113,   114,   115,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   358,
     424,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   109,     5,     6,     7,     8,     9,   112,   113,   114,
     115,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   358,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   188,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   189,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   190,     0,
       0,   852,     0,     0,   112,   113,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   358,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   188,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   189,    97,
       0,     0,     0,    99,     0,     0,   100,     5,     6,     7,
       8,     9,   101,   102,     0,     0,     0,    10,   105,   106,
     107,     0,     0,   108,   190,     0,     0,     0,     0,     0,
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
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,     0,
       0,     0,    99,     0,     0,   100,     5,     6,     7,     8,
       9,   101,   102,     0,     0,     0,    10,   105,   106,   107,
       0,     0,   108,   190,     0,     0,     0,     0,     0,   112,
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
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,     0,     0,
       0,    99,     0,     0,   100,     5,     6,     7,     8,     9,
     101,   102,     0,     0,     0,    10,   105,   106,   107,     0,
       0,   108,   190,     0,     0,     0,     0,     0,   112,   113,
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
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,     0,     0,     0,
      99,     0,     0,   100,     5,     6,     7,     8,     9,   101,
     102,     0,     0,     0,    10,   105,   106,   107,     0,     0,
     108,   190,     0,   273,     0,     0,     0,   112,   113,   114,
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
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   188,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   189,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     190,     0,   276,     0,     0,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   424,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     188,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     189,    97,     0,     0,     0,    99,     0,     0,   100,     5,
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
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   190,   569,     0,     0,     0,
       0,   112,   113,   114,   115,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   358,
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
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,     0,     0,     0,
      99,     0,     0,   100,     5,     6,     7,     8,     9,   101,
     102,     0,     0,     0,    10,   105,   106,   107,     0,     0,
     108,   190,     0,     0,     0,     0,     0,   112,   113,   114,
     115,     0,     0,   747,     0,     0,     0,     0,     0,     0,
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
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   188,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   189,    97,     0,     0,     0,    99,
       0,     0,   100,     5,     6,     7,     8,     9,   101,   102,
       0,     0,     0,    10,   105,   106,   107,     0,     0,   108,
     190,     0,     0,     0,     0,     0,   112,   113,   114,   115,
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
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   188,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   189,    97,     0,     0,     0,    99,     0,
       0,   100,     5,     6,     7,     8,     9,   101,   102,     0,
       0,     0,    10,   105,   106,   107,     0,     0,   108,   190,
       0,     0,     0,     0,     0,   112,   113,   114,   115,     0,
       0,     0,     0,     0,     0,     0,     0,   832,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,     0,
     185,   186,   187,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   188,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   189,    97,     0,     0,     0,    99,     0,     0,
     100,     5,     6,     7,     8,     9,   101,   102,     0,     0,
       0,    10,   105,   106,   107,     0,     0,   108,   190,     0,
       0,     0,     0,     0,   112,   113,   114,   115,     0,     0,
       0,     0,     0,     0,     0,     0,   834,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   185,
     186,   187,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,     0,     0,     0,    99,     0,     0,   100,
       5,     6,     7,     8,     9,   101,   102,     0,     0,     0,
      10,   105,   106,   107,     0,     0,   108,   190,     0,     0,
       0,     0,     0,   112,   113,   114,   115,     0,     0,     0,
       0,     0,     0,     0,     0,  1312,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   185,   186,
     187,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     188,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     189,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   190,     5,     6,     7,
       8,     9,   112,   113,   114,   115,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   358,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,     0,
       0,     0,    99,     0,     0,   100,     5,     6,     7,     8,
       9,   101,   102,     0,     0,     0,    10,   105,   106,   107,
       0,     0,   108,  1437,     0,     0,     0,     0,     0,   112,
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
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,     0,     0,
       0,    99,     0,     0,   100,     5,     6,     7,     8,     9,
     101,   102,     0,     0,     0,    10,   105,   106,   107,     0,
       0,   108,   190,     0,     0,     0,     0,     0,   112,   113,
     114,   115,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,   661,    39,    40,     0,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   185,   186,   187,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,     0,   278,   279,
      99,   280,   281,   100,     0,   282,   283,   284,   285,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   190,     0,   286,   287,     0,     0,   112,   113,   114,
     115,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   289,   501,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   502,   291,   292,   293,   294,
     295,   296,   297,     0,     0,     0,   212,     0,   213,    40,
       0,     0,   298,     0,     0,     0,     0,     0,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,    50,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   212,   334,     0,   782,   336,   337,
     338,     0,     0,     0,   339,   599,   216,   217,   218,   219,
     220,   600,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,   278,   279,     0,   280,   281,     0,   601,   282,
     283,   284,   285,     0,    93,    94,     0,    95,   189,    97,
     344,     0,   345,     0,     0,   346,     0,   286,   287,     0,
       0,     0,     0,   348,   216,   217,   218,   219,   220,     0,
       0,     0,     0,   108,     0,     0,     0,   783,     0,     0,
     112,     0,     0,     0,     0,     0,   289,     0,     0,     0,
     455,     0,    93,    94,     0,    95,   189,    97,     0,     0,
     291,   292,   293,   294,   295,   296,   297,     0,     0,     0,
     212,     0,   213,    40,     0,     0,   298,     0,     0,     0,
       0,   108,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,    50,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,     0,   334,
       0,   335,   336,   337,   338,     0,     0,     0,   339,   599,
     216,   217,   218,   219,   220,   600,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   278,   279,     0,   280,
     281,     0,   601,   282,   283,   284,   285,     0,    93,    94,
       0,    95,   189,    97,   344,     0,   345,     0,     0,   346,
       0,   286,   287,     0,   288,     0,     0,   348,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   108,     0,     0,
       0,   783,     0,     0,   112,     0,     0,     0,     0,     0,
     289,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   290,     0,     0,   291,   292,   293,   294,   295,   296,
     297,     0,     0,     0,   212,     0,     0,     0,     0,     0,
     298,     0,     0,     0,     0,     0,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,    50,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,     0,   334,     0,     0,   336,   337,   338,     0,
       0,     0,   339,   340,   216,   217,   218,   219,   220,   341,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   342,     0,     0,    91,
     343,     0,    93,    94,     0,    95,   189,    97,   344,    50,
     345,     0,     0,   346,     0,   278,   279,     0,   280,   281,
     347,   348,   282,   283,   284,   285,     0,     0,     0,     0,
       0,   108,   349,     0,     0,     0,  1878,     0,     0,     0,
     286,   287,     0,   288,     0,     0,   216,   217,   218,   219,
     220,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   188,   289,
       0,    91,    92,     0,    93,    94,     0,    95,   189,    97,
     290,     0,     0,   291,   292,   293,   294,   295,   296,   297,
       0,     0,     0,   212,     0,     0,     0,     0,     0,   298,
       0,     0,     0,   108,     0,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,    50,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,     0,   334,     0,     0,   336,   337,   338,     0,     0,
       0,   339,   340,   216,   217,   218,   219,   220,   341,     0,
       0,     0,     0,     0,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   342,     0,     0,    91,   343,
       0,    93,    94,     0,    95,   189,    97,   344,    50,   345,
       0,     0,   346,     0,   278,   279,     0,   280,   281,   347,
     348,   282,   283,   284,   285,     0,     0,     0,     0,     0,
     108,   349,     0,     0,     0,  1955,     0,     0,     0,   286,
     287,     0,   288,     0,     0,   216,   217,   218,   219,   220,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   289,   501,
     369,     0,     0,    93,    94,     0,    95,   189,    97,   290,
       0,   502,   291,   292,   293,   294,   295,   296,   297,     0,
       0,     0,   212,     0,     0,     0,     0,     0,   298,     0,
       0,     0,   108,     0,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,    50,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
       0,   334,     0,   335,   336,   337,   338,     0,     0,     0,
     339,   340,   216,   217,   218,   219,   220,   341,     0,     0,
       0,     0,     0,     0,   212,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   342,     0,     0,    91,   343,     0,
      93,    94,     0,    95,   189,    97,   344,    50,   345,     0,
       0,   346,     0,   278,   279,     0,   280,   281,   347,   348,
     282,   283,   284,   285,     0,     0,     0,     0,     0,   108,
     349,     0,     0,     0,     0,     0,     0,     0,   286,   287,
       0,   288,     0,     0,   216,   217,   218,   219,   220,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,   289,     0,   923,
       0,     0,    93,    94,     0,    95,   189,    97,   290,     0,
    1183,   291,   292,   293,   294,   295,   296,   297,     0,     0,
       0,   212,     0,     0,     0,     0,     0,   298,     0,     0,
       0,   108,     0,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,    50,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,     0,
     334,     0,     0,   336,   337,   338,     0,     0,     0,   339,
     340,   216,   217,   218,   219,   220,   341,     0,     0,     0,
       0,     0,     0,   212,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   342,     0,     0,    91,   343,     0,    93,
      94,     0,    95,   189,    97,   344,    50,   345,     0,     0,
     346,     0,     0,     0,   921,   922,     0,   347,   348,  1670,
       0,     0,     0,   278,   279,     0,   280,   281,   108,   349,
     282,   283,   284,   285,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   216,   217,   218,   219,   220,   286,   287,
       0,   288,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   923,     0,
       0,    93,    94,     0,    95,   189,    97,   289,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   290,     0,
       0,   291,   292,   293,   294,   295,   296,   297,     0,     0,
     108,   212,     0,     0,     0,     0,     0,   298,     0,     0,
       0,     0,     0,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,    50,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,     0,
     334,     0,     0,   336,   337,   338,     0,     0,     0,   339,
     340,   216,   217,   218,   219,   220,   341,     0,     0,     0,
       0,     0,     0,   212,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   342,     0,     0,    91,   343,     0,    93,
      94,     0,    95,   189,    97,   344,    50,   345,     0,     0,
     346,     0,  1772,  1773,  1774,  1775,  1776,   347,   348,  1777,
    1778,  1779,  1780,     0,     0,     0,     0,     0,   108,   349,
       0,     0,     0,     0,     0,     0,  1781,  1782,  1783,     0,
       0,     0,     0,   216,   217,   218,   219,   220,     0,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,  1784,   501,     0,     0,
       0,    93,    94,     0,    95,   189,    97,     0,     0,   502,
    1785,  1786,  1787,  1788,  1789,  1790,  1791,     0,     0,     0,
     212,     0,     0,     0,     0,     0,  1792,     0,     0,     0,
     108,     0,  1793,  1794,  1795,  1796,  1797,  1798,  1799,  1800,
    1801,  1802,  1803,    50,  1804,  1805,  1806,  1807,  1808,  1809,
    1810,  1811,  1812,  1813,  1814,  1815,  1816,  1817,  1818,  1819,
    1820,  1821,  1822,  1823,  1824,  1825,  1826,  1827,  1828,  1829,
    1830,  1831,  1832,  1833,  1834,     0,     0,     0,  1835,  1836,
     216,   217,   218,   219,   220,     0,  1837,  1838,  1839,  1840,
    1841,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1842,  1843,  1844,     0,   212,     0,    93,    94,
       0,    95,   189,    97,  1845,     0,  1846,  1847,     0,  1848,
       0,     0,     0,     0,     0,     0,  1849,     0,  1850,    50,
    1851,     0,  1852,  1853,     0,   278,   279,   108,   280,   281,
       0,     0,   282,   283,   284,   285,     0,     0,     0,     0,
       0,     0,  1679,     0,     0,     0,     0,     0,     0,     0,
     286,   287,     0,     0,     0,  1680,   216,   217,   218,   219,
     220,  1681,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   188,   289,
       0,    91,    92,     0,    93,    94,     0,    95,  1683,    97,
       0,     0,     0,   291,   292,   293,   294,   295,   296,   297,
       0,     0,     0,   212,     0,     0,     0,     0,     0,   298,
       0,     0,     0,   108,     0,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,    50,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,     0,   334,     0,   335,   336,   337,   338,     0,     0,
       0,   339,   599,   216,   217,   218,   219,   220,   600,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   278,
     279,     0,   280,   281,     0,   601,   282,   283,   284,   285,
       0,    93,    94,     0,    95,   189,    97,   344,     0,   345,
       0,     0,   346,     0,   286,   287,     0,     0,     0,     0,
     348,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     108,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   289,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   291,   292,   293,
     294,   295,   296,   297,     0,     0,     0,   212,     0,     0,
       0,     0,     0,   298,     0,     0,     0,     0,     0,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
      50,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,     0,   334,     0,  1370,   336,
     337,   338,     0,     0,     0,   339,   599,   216,   217,   218,
     219,   220,   600,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   278,   279,     0,   280,   281,     0,   601,
     282,   283,   284,   285,     0,    93,    94,     0,    95,   189,
      97,   344,     0,   345,     0,     0,   346,     0,   286,   287,
       0,     0,     0,     0,   348,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   108,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   289,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   291,   292,   293,   294,   295,   296,   297,     0,     0,
       0,   212,     0,     0,     0,     0,     0,   298,     0,     0,
       0,     0,     0,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,    50,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,     0,
     334,     0,     0,   336,   337,   338,     0,     0,     0,   339,
     599,   216,   217,   218,   219,   220,   600,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   601,     0,     0,     0,     0,     0,    93,
      94,     0,    95,   189,    97,   344,     0,   345,     0,     0,
     346,   473,   474,   475,     0,     0,     0,     0,   348,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   108,     0,
       0,   476,   477,     0,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,     0,   501,   473,
     474,   475,     0,     0,     0,     0,     0,     0,     0,     0,
     502,     0,     0,     0,     0,     0,     0,     0,     0,   476,
     477,     0,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,     0,   501,   473,   474,   475,
       0,     0,     0,     0,     0,     0,     0,     0,   502,     0,
       0,     0,     0,     0,     0,     0,     0,   476,   477,     0,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,     0,   501,   473,   474,   475,     0,     0,
       0,     0,     0,     0,     0,     0,   502,     0,     0,     0,
       0,     0,     0,     0,     0,   476,   477,     0,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,     0,   501,     0,  1315,     0,   473,   474,   475,     0,
       0,     0,     0,     0,   502,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   476,   477,  1514,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,  1590,   501,   473,   474,   475,     0,     0,     0,
       0,     0,     0,     0,     0,   502,     0,     0,     0,     0,
       0,     0,     0,     0,   476,   477,     0,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
    1713,   501,   473,   474,   475,     0,     0,     0,     0,     0,
       0,     0,     0,   502,     0,     0,     0,     0,     0,     0,
       0,     0,   476,   477,     0,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,  1714,   501,
       0,   473,   474,   475,     0,     0,     0,     0,     0,     0,
       0,   502,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   476,   477,     0,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,  1515,   501,   473,
     474,   475,     0,     0,     0,     0,     0,     0,     0,     0,
     502,     0,     0,     0,     0,     0,     0,     0,     0,   476,
     477,     0,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   503,   501,   473,   474,   475,
       0,     0,     0,     0,     0,     0,     0,     0,   502,     0,
       0,     0,     0,     0,     0,     0,     0,   476,   477,     0,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   585,   501,     0,   473,   474,   475,     0,
       0,     0,     0,     0,     0,     0,   502,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   476,   477,     0,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   587,   501,   473,   474,   475,     0,     0,     0,
       0,     0,     0,     0,     0,   502,   288,     0,     0,     0,
       0,     0,     0,     0,   476,   477,     0,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     606,   501,     0,   290,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   502,     0,     0,   212,     0,     0,     0,
       0,     0,  1445,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,  1378,     0,   610,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   879,   880,     0,     0,     0,     0,
     881,     0,   882,     0,     0,   588,   216,   217,   218,   219,
     220,   589,     0,     0,   883,     0,     0,     0,     0,     0,
       0,     0,    34,    35,    36,   212,   824,     0,   188,     0,
       0,    91,   343,     0,    93,    94,   214,    95,   189,    97,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,   347,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   108,   349,     0,     0,     0,     0,     0,
     879,   880,     0,     0,   849,     0,   881,     0,   882,     0,
       0,     0,     0,     0,   884,   885,   886,   887,   888,   889,
     883,    81,    82,    83,    84,    85,     0,     0,    34,    35,
      36,   212,   221,     0,     0,     0,     0,   188,    89,    90,
      91,    92,   214,    93,    94,     0,    95,   189,    97,     0,
       0,     0,    99,     0,    50,     0,     0,     0,     0,     0,
       0,   890,   891,     0,     0,     0,     0,   105,     0,     0,
       0,     0,   108,   892,     0,     0,  1062,  1063,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     884,   885,   886,   887,   888,   889,  1064,    81,    82,    83,
      84,    85,     0,     0,  1065,  1066,  1067,   212,   221,     0,
       0,     0,     0,   188,    89,    90,    91,    92,  1068,    93,
      94,     0,    95,   189,    97,     0,     0,     0,    99,     0,
      50,     0,     0,     0,     0,     0,     0,   890,   891,     0,
       0,     0,     0,   105,     0,     0,     0,     0,   108,   892,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1069,  1070,  1071,  1072,
    1073,  1074,    29,     0,     0,     0,     0,     0,     0,     0,
      34,    35,    36,   212,  1075,   213,    40,     0,     0,   188,
       0,     0,    91,    92,   214,    93,    94,     0,    95,   189,
      97,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,  1076,  1077,     0,     0,     0,     0,     0,
       0,     0,     0,   215,   108,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,   216,   217,   218,   219,   220,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
     221,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,    29,  1054,     0,
      99,     0,     0,     0,     0,    34,    35,    36,   212,     0,
     213,    40,     0,     0,     0,   105,     0,     0,     0,   214,
     108,   222,     0,     0,   626,     0,     0,   112,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   215,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,    74,    75,   216,   217,
     218,   219,   220,    29,    81,    82,    83,    84,    85,  1183,
       0,    34,    35,    36,   212,   221,   213,    40,     0,     0,
     188,    89,    90,    91,    92,   214,    93,    94,     0,    95,
     189,    97,     0,     0,     0,    99,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     105,     0,     0,     0,   215,   108,   222,     0,     0,     0,
       0,     0,   112,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,   216,   217,   218,   219,   220,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,   221,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,     0,     0,
       0,    99,     0,     0,     0,     0,   473,   474,   475,     0,
       0,     0,     0,     0,     0,     0,   105,     0,     0,     0,
       0,   108,   222,     0,     0,     0,   476,   477,   112,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,     0,   501,   473,   474,   475,     0,     0,     0,
       0,     0,     0,     0,     0,   502,     0,     0,     0,     0,
       0,     0,     0,     0,   476,   477,     0,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
       0,   501,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   502,     0,   473,   474,   475,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   556,   476,   477,     0,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,     0,   501,   473,   474,   475,     0,     0,     0,     0,
       0,     0,     0,     0,   502,     0,     0,     0,     0,     0,
       0,     0,   953,   476,   477,     0,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,     0,
     501,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   502,     0,   473,   474,   475,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1040,   476,   477,     0,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
       0,   501,   473,   474,   475,     0,     0,     0,     0,     0,
       0,     0,     0,   502,     0,     0,     0,     0,     0,     0,
       0,  1095,   476,   477,     0,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,     0,   501,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   502,  1157,  1158,  1159,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1420,  1160,     0,     0,  1161,  1162,  1163,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1157,  1158,
    1159,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1183,     0,     0,     0,     0,     0,     0,     0,  1160,
    1463,     0,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1183,  1157,  1158,
    1159,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1160,
    1351,     0,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,  1157,  1158,  1159,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1183,     0,     0,
       0,     0,     0,     0,     0,  1160,  1533,     0,  1161,  1162,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1183,  1157,  1158,  1159,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1160,  1545,     0,  1161,  1162,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1157,  1158,  1159,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1183,     0,     0,     0,     0,     0,     0,
       0,  1160,  1656,     0,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,    34,    35,    36,   212,
       0,   213,    40,     0,     0,     0,     0,     0,     0,  1183,
     214,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1753,     0,     0,     0,     0,     0,     0,   242,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     243,     0,     0,     0,     0,     0,     0,     0,     0,   216,
     217,   218,   219,   220,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   221,     0,  1755,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,     0,     0,     0,    99,     0,    34,    35,
      36,   212,     0,   213,    40,     0,     0,     0,     0,     0,
       0,   105,   675,     0,     0,     0,   108,   244,     0,     0,
       0,     0,     0,   112,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   215, -1132, -1132, -1132, -1132,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,     0,
     501,   216,   217,   218,   219,   220,     0,    81,    82,    83,
      84,    85,   502,     0,    34,    35,    36,   212,   221,   213,
      40,     0,     0,   188,    89,    90,    91,    92,   214,    93,
      94,     0,    95,   189,    97,     0,     0,     0,    99,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   105,     0,     0,     0,   242,   108,   676,
       0,     0,     0,     0,     0,   112,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   216,   217,   218,
     219,   220,     0,    81,    82,    83,    84,    85,   212,     0,
       0,     0,     0,     0,   221,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
      97,    50,     0,     0,    99,     0,     0,     0,     0,   366,
     367,     0,     0,     0,     0,     0,     0,     0,     0,   105,
       0,     0,     0,     0,   108,   244,     0,     0,     0,     0,
       0,   112,     0,     0,     0,     0,     0,     0,   216,   217,
     218,   219,   220,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     368,     0,     0,   369,     0,     0,    93,    94,     0,    95,
     189,    97,     0,   473,   474,   475,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   370,     0,     0,     0,
     861,     0,     0,   476,   477,   108,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,     0,
     501,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   502,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   473,   474,   475,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   862,   476,   477,  1037,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,     0,   501,   473,   474,   475,     0,     0,
       0,     0,     0,     0,     0,     0,   502,     0,     0,     0,
       0,     0,     0,     0,     0,   476,   477,     0,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,     0,   501,  1157,  1158,  1159,     0,     0,     0,     0,
       0,     0,     0,     0,   502,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1160,  1550,     0,  1161,  1162,  1163,
    1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1157,
    1158,  1159,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1183,     0,     0,     0,     0,     0,     0,     0,
    1160,     0,     0,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,   474,   475,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1183,     0,
       0,     0,     0,     0,   476,   477,     0,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
       0,   501,  1158,  1159,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   502,     0,     0,     0,     0,     0,     0,
       0,     0,  1160,     0,     0,  1161,  1162,  1163,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,   475,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1183,     0,     0,     0,     0,   476,   477,     0,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,  1159,   501,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   502,     0,     0,     0,     0,     0,
    1160,     0,     0,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   476,   477,  1183,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,     0,   501,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   477,   502,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,     0,
     501,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1160,     0,   502,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1183,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,     0,   501,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   502,  1161,  1162,  1163,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1183,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1183,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,     0,   501,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     502,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,
    1182,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1183,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,     0,   501,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   502,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1183, -1132, -1132, -1132, -1132,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1183
};

static const yytype_int16 yycheck[] =
{
       5,     6,   132,     8,     9,    10,    11,    12,    13,    56,
      15,    16,    17,    18,    56,   167,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   109,    31,   192,   693,   731,
      33,    44,   109,   852,   109,   172,   173,   416,   416,    44,
      30,   416,     4,    46,   124,   124,   416,    52,    51,    54,
     245,   996,    57,   689,    59,   720,     4,   501,    30,     4,
       4,   877,   124,   193,   559,    30,   505,   506,    19,    20,
      30,   537,   538,    86,   571,   984,  1417,   167,   867,  1110,
     860,    86,   836,   994,   690,   534,    57,  1031,   447,   534,
      98,   668,     9,  1892,    32,   103,   104,  1220,     9,   565,
     539,  1016,     9,  1018,   109,  1049,   190,   620,   621,     9,
     607,     6,    32,   190,    48,   190,   791,     9,   567,    48,
     257,     9,   567,   131,     9,   449,   450,   451,     9,     9,
       9,     9,   109,    14,    14,    14,     9,     9,   222,     4,
       9,   256,     9,     4,    70,   222,    70,   222,    36,     9,
       9,     9,  1096,    48,     9,     9,   650,   651,     9,     9,
     244,     4,     9,     9,    83,     9,    48,   244,     9,    83,
       9,   112,   366,   367,   368,    83,   370,    14,   162,    14,
    1700,    91,   998,    81,   678,   190,   564,    57,   103,  1709,
      83,    48,     0,   198,   116,  1106,    48,  2036,    91,   136,
     137,   162,    38,   199,    70,    38,   199,    14,    83,    84,
     136,   137,   199,   136,   137,   199,    38,   222,   113,   713,
    2059,  1954,   183,  1956,   119,    32,   121,   122,   123,   124,
     125,   126,   127,    70,   132,    70,    70,    50,    51,   244,
      70,   160,   161,  1152,    51,    54,    38,    83,   162,   159,
      83,   166,    54,    70,   259,    70,   269,   262,   180,  2058,
     925,    83,   196,   201,   269,   270,   200,   160,   161,    38,
      70,   200,   826,  2072,   169,   170,   202,   172,   202,    70,
     203,    70,   202,    70,   201,   202,   401,   781,   239,   132,
      38,    83,   994,   204,   201,   460,  1327,   205,  1052,   194,
     200,   263,   167,   183,   388,   267,    70,    70,   203,   201,
     357,   388,   200,   388,    83,   357,   201,    70,   200,  1234,
     201,   201,   201,   201,  1026,  1563,  1667,    70,   201,   201,
     205,  1120,   201,  1122,   201,    83,   202,  1460,  1282,   196,
     176,   201,   201,   201,  1467,   200,  1469,   201,   200,   200,
     193,   201,    83,   358,   201,   201,   200,  1877,   167,   200,
     857,   200,   199,   447,   199,   167,    91,    83,   202,   199,
     447,   184,   447,   162,   387,   869,  1499,   542,    70,  1230,
     182,   875,   387,   388,   176,   202,   516,   202,   393,   394,
     395,   396,   397,   398,   399,   972,  1305,   168,   445,   404,
     202,   162,   202,   445,  1106,    50,    51,   176,   921,   922,
     199,   202,   417,   202,   122,   202,    70,   168,    70,   424,
       8,  1237,  1028,   397,   132,   399,   377,   432,   176,    70,
     181,   202,   107,   108,   159,   386,   199,   200,  1676,   444,
      70,   202,   447,   417,    70,   176,   454,   199,  1149,   202,
    1151,   202,  1153,   947,   405,    57,   199,   462,   463,   202,
     176,   508,    70,  1701,  1702,   416,  1704,    69,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,  1619,   502,    70,   504,
     505,   506,   507,   511,   512,   513,   514,   554,   162,    83,
    1064,   501,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   199,    83,   504,   203,   501,
    1346,    83,   537,   538,   539,   540,   501,    70,   543,   544,
     517,   501,   547,  1218,  1324,   199,   517,   199,   202,     4,
     202,   556,   557,  1002,   162,   707,   103,  1002,   199,   199,
     565,   202,   423,    70,   136,   137,   431,   508,   573,   199,
     575,   183,   202,   199,    19,    20,   202,   447,   107,   108,
    1046,   696,   202,   698,   199,    83,    84,   199,   431,   103,
     785,    83,   676,   534,   202,    70,  1090,    70,    53,   564,
    1236,    56,   176,   136,   137,   984,   984,  1458,  1949,   984,
    1054,   167,   103,   554,   984,   199,   578,   707,    73,   166,
    1117,   626,   817,   564,   176,   162,   567,   199,   793,  1445,
     202,    83,  1541,   123,   504,    83,    84,    14,   195,    91,
    1134,    96,   132,    98,  1138,   202,   183,   517,   103,   104,
    1099,  1145,   166,   199,  1099,   162,    83,  1016,    60,  1018,
    1266,   136,   137,  1269,    91,   202,     4,   159,   160,   161,
     166,   676,   515,   199,   203,   166,   131,   136,   137,   124,
     199,    19,    20,   199,    83,  1198,    88,   162,  1300,    91,
    1302,    83,    91,  1017,  1306,   202,  1020,  1021,   893,    91,
     837,    75,    76,   573,   202,   201,   202,   205,   160,   161,
      83,   906,    87,   718,     4,   830,   831,   911,    91,   913,
      70,   915,   837,   838,   199,   919,   103,   104,  1637,    19,
      20,   739,   183,   160,   161,    70,    31,    83,    75,    76,
     199,    70,   747,  1297,  1298,    91,    87,  1301,   199,   124,
     125,   126,   207,    31,  1308,    50,   208,  1608,    53,  1610,
     159,   160,   161,   183,    70,   716,   199,   159,   160,   161,
     183,   109,    50,  1152,  1152,    53,   183,  1152,   783,   199,
     207,   199,  1152,   124,   125,   126,   199,   160,   161,  1284,
     103,   104,   199,  1468,   239,    78,    79,    80,    81,    32,
    1436,  1295,    53,    54,    55,   199,    57,   812,   263,  1275,
    1249,   166,   267,   112,   160,   161,   271,   162,    69,   199,
    1286,   104,   121,   122,   123,   124,   125,   126,    38,   690,
     134,   135,   121,   122,   123,   124,   125,   126,   183,   844,
     201,   202,   707,   794,   162,   201,   202,   860,   166,   106,
     107,   108,   190,   856,   199,   860,  1462,   202,   141,   142,
     143,   144,   145,  1357,   118,   183,   731,  1361,   201,    53,
      54,    55,  1366,   201,  1725,  1234,  1923,  1924,  1729,   201,
    1374,   832,   201,   834,   222,    69,   169,   170,   731,   172,
     173,   174,  1919,  1920,  1596,   194,   366,   367,   368,   201,
     370,   239,   357,   865,   201,   194,   244,  1582,  1405,  1001,
    1002,   862,   201,   393,   394,   198,   106,   107,   108,   402,
     201,   201,  1476,   406,  1478,   263,  1305,  1305,    70,   267,
    1305,    70,   377,    70,    70,  1305,   113,   114,   115,    70,
     987,   386,    70,   388,    70,    70,   202,   162,   953,   239,
     955,   434,   957,   436,   437,   438,   439,   199,   199,    70,
     405,   162,   166,   968,   366,   367,   368,   369,   370,   201,
      49,    69,   183,   263,   162,   199,   431,   267,   983,   199,
     162,   162,   199,     8,   935,   850,   441,   201,   162,  1483,
     445,   199,    14,  1487,   162,   201,   201,     9,  1464,   454,
    1494,   201,  1677,   202,  1501,    14,   408,   850,  1013,   971,
     132,  1016,   132,  1018,  1620,   200,   183,    14,  1023,  1516,
     103,   200,   200,   971,   200,   200,   971,   971,   199,   202,
     112,     9,  1037,   984,   206,  1040,   199,  1042,   199,   377,
    1891,  1046,   159,   908,  1895,  1053,   987,   200,   386,  1244,
     388,   200,  1247,   508,   509,   510,   511,   512,   513,   514,
    1001,  1002,   200,    95,  1054,   908,   200,   405,     9,   201,
    2015,    14,  1626,   183,  1628,   199,  1630,     9,  1437,   534,
     199,  1635,  1054,  1994,   202,   201,   201,   377,  1999,  1054,
    1095,    83,  2037,   431,  1054,   202,   386,   202,   201,   554,
    1103,  2046,   202,   134,   201,   200,   971,   200,  1155,   447,
     971,   200,   567,  2024,   201,   405,  1440,  1441,  1442,   204,
      78,    79,    80,   578,     9,     9,   204,  1946,   971,   994,
     204,   204,  1951,   204,    92,   996,    70,    32,  1604,  1605,
     135,   431,  1104,   598,  1641,   182,  1016,   162,  1018,   138,
       9,   994,  1649,  1023,  1234,  1234,  1097,   200,  1099,  1110,
    1111,  1026,  1541,  1541,   162,   200,  1541,  1028,  1665,   624,
     625,  1541,  1234,    14,  2085,   121,   122,   123,   124,   125,
     126,  2000,  1736,  1026,     9,   196,     9,   200,   184,   147,
     148,   149,   150,   151,  2045,     9,    14,     9,   200,   200,
     158,  1152,   657,   658,   134,   204,   164,   165,     9,   200,
     200,  2062,    14,   200,   200,   199,   204,  1222,   620,   621,
     178,   204,   203,    50,    51,    52,    53,    54,    55,  1234,
      57,   204,  2048,   162,   200,   193,  2052,   103,   201,   201,
     578,  1106,    69,  1108,  1249,     9,   138,  1252,   194,  1746,
    2066,  2067,   162,     9,   199,  1217,  2075,    70,  1637,  1637,
     200,    70,  1637,  1106,    70,  1108,    70,  1637,   199,  1217,
    1275,   716,  1217,  1217,    70,   199,   731,   202,     9,    14,
    1945,  1286,  1947,   203,   739,   201,   184,     9,   578,   202,
     204,   202,  1994,   176,    14,   200,   196,  1999,   201,  1250,
      32,    70,   199,   199,  1441,    50,    51,    52,    53,    54,
      55,  1324,    14,  1316,    32,    52,    70,   199,   199,  1324,
     199,    70,  2024,    70,    69,   199,    70,    70,  1333,   199,
     121,   122,   123,   124,   125,   126,   162,     9,   200,   138,
     201,   132,   133,  1897,  1898,   201,   199,    14,   184,   794,
     138,   162,  1217,  1437,  1305,     9,  1217,  1319,   200,   176,
    1437,  1312,  1437,    69,  1234,     6,   176,   204,    83,     9,
    2035,   826,   203,   828,  1217,   203,  1327,     9,   716,   203,
     201,   203,   173,  2085,   175,   138,   201,   832,    83,   834,
      14,   199,    83,   731,   199,   850,   138,   200,   189,   199,
     191,     9,  1899,   194,   202,  1266,   200,    48,  1269,   864,
     865,   204,   202,   202,   201,  1420,    92,   862,   159,  1913,
    1914,   202,  1427,    32,    77,   201,   716,  1432,   200,  1434,
     201,  1583,  1437,   184,   138,    32,   204,   200,   200,     9,
       9,   731,   138,   204,   204,  1450,   204,  1577,     9,   204,
     200,   203,   200,   908,     9,   201,   794,    14,  1463,  1464,
     200,   826,   917,   918,   203,  1330,   202,   201,   201,    83,
     199,   201,   113,   201,   199,   201,   200,   204,   119,   200,
     121,   122,   123,   124,   125,   126,   127,  1330,   200,   202,
     935,   199,     9,   948,   832,   200,   834,   200,     9,   138,
     138,     9,    32,   204,   794,   204,   204,   204,   200,   911,
     204,   913,   850,   915,   200,   200,   971,   919,   201,   921,
     922,   923,    81,   200,   862,   138,   201,   865,   169,   170,
     201,   172,   987,   176,  1699,   113,   202,   113,   113,   994,
     113,   171,   832,    83,   834,   104,  1001,  1002,   113,   201,
     167,    83,    14,   194,  1419,    83,   119,   202,  1419,  1424,
     850,   138,   203,  1424,   200,   200,   200,  1437,    14,   202,
     908,  1026,   862,   183,   138,   865,  2073,    14,   201,  1709,
      14,    14,   141,   142,   143,   144,   145,    83,   200,   200,
    1541,  1596,   199,   198,   200,   138,  1601,   935,  1053,  1604,
    1605,  1462,   138,   201,    83,   201,    14,  1062,  1063,  1064,
     169,   170,    14,   172,   173,   174,   201,    14,   908,   121,
     122,   123,   124,   125,   126,     9,  1673,     9,   202,    68,
     132,   133,   203,   971,    81,    14,    83,    84,   183,   198,
     199,   199,  1097,    83,  1099,   935,     9,     9,   202,  1104,
     201,  1106,   116,  1108,   103,   162,   994,   104,   103,   184,
     174,    36,  1670,   200,   199,  1110,  1111,   201,   199,   184,
     180,   184,    83,   175,  1129,     9,   177,   200,  1016,    83,
    1018,   971,   201,    83,  1646,    14,  1637,  1552,  1026,    83,
     202,   200,   194,    83,   141,   142,   143,   144,   145,  1064,
    1155,   200,    14,    83,   994,    14,    83,    14,  2027,  1552,
    1875,    83,  1198,  1047,  1719,   974,   511,   514,  1583,  2042,
     167,  1325,   169,   170,   509,   172,   173,   174,  1745,  1184,
    1513,  1596,  2037,  1576,  1251,   628,  1026,  1732,  1770,  1678,
      81,  1857,  2083,  1572,  1609,  2059,  1565,  1728,  1609,   398,
    1615,   198,  1617,  1596,  1615,   202,  1617,  1150,   205,  1620,
    1301,  1221,  1217,   104,  1146,  1063,  1104,  1297,  1106,  1296,
    1108,  1092,  1110,  1111,  1937,  1640,   445,  1642,  1995,  1640,
    1984,  1557,  1744,  1745,   878,  1650,  1130,  1206,  1184,  1234,
      -1,    -1,    -1,    -1,    -1,    -1,  1198,    -1,    -1,  1642,
     141,   142,   143,   144,   145,  1250,    -1,  1650,    -1,    -1,
      -1,    -1,    -1,    -1,  1104,    -1,  1106,    -1,  1108,    -1,
    1110,  1111,  1869,    -1,    -1,   166,    -1,  1869,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1297,  1298,  1299,  1300,  1301,  1302,    -1,    -1,
      -1,  1306,    -1,  1308,    -1,    -1,  2008,   198,    -1,    -1,
      -1,   202,    -1,    -1,  1319,  1708,    -1,  1312,    -1,  1734,
      -1,    -1,    -1,  1734,    -1,  1330,    -1,  1882,  1743,  1217,
       6,    -1,  1327,    -1,  1749,  1340,    -1,    -1,    -1,    -1,
    1937,  1756,    -1,    -1,    -1,  1937,  1234,    -1,     6,    -1,
    1743,  2031,    -1,    -1,    -1,    -1,  1749,    -1,    -1,    -1,
      -1,    -1,  1250,  1756,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    -1,    -1,    -1,  1217,    -1,    -1,
      -1,    -1,  1297,  1298,  1299,  1300,  1301,  1302,    19,    20,
      48,  1306,    -1,  1308,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1413,    -1,
    1250,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,  1312,    56,    -1,    -1,    -1,    -1,
      -1,  1319,    -1,    -1,    -1,    -1,    -1,   113,    -1,  1327,
      -1,    -1,  1330,   119,    -1,   121,   122,   123,   124,   125,
     126,   127,    -1,    -1,    -1,   113,    59,    60,    -1,    -1,
      -1,   119,    -1,   121,   122,   123,   124,   125,   126,   127,
      -1,  1476,  1312,  1478,    -1,    -1,    -1,    -1,    -1,  1319,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1327,  1903,    -1,
    1330,    -1,    -1,   169,   170,    -1,   172,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    1903,   169,   170,    -1,   172,    -1,    -1,    -1,   194,    -1,
      -1,    -1,    -1,    69,    -1,    81,  2081,   203,    -1,    -1,
      -1,    -1,    -1,   136,   137,  2090,   194,    -1,    -1,    -1,
      -1,    -1,    -1,  2098,  1959,   203,  2101,  1552,   104,  1437,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1563,    -1,
      -1,  1476,    -1,  1478,    -1,    -1,  1959,  1572,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1994,
      -1,    -1,    -1,    -1,  1999,   141,   142,   143,   144,   145,
      -1,  1596,    -1,  2008,    -1,    -1,    -1,   200,    -1,    -1,
      -1,  1994,    -1,    -1,  2015,    -1,  1999,   163,   239,  2024,
     166,    -1,    -1,   169,   170,    -1,   172,   173,   174,    -1,
      -1,  1626,    -1,  1628,    -1,  1630,  2037,    -1,    -1,    -1,
    1635,  2024,    -1,    -1,    -1,  2046,    -1,  1642,    -1,    -1,
      -1,  1646,   198,    -1,    -1,  1650,    -1,   203,    -1,    -1,
      -1,    -1,     6,    -1,    -1,    -1,    -1,   288,    -1,   290,
      -1,    -1,    -1,    -1,  1552,  1670,    -1,    -1,  1673,  2084,
    2085,  1676,    -1,    -1,    -1,     6,    -1,    -1,    -1,    -1,
      -1,  1686,    -1,    -1,    -1,    -1,    -1,    31,  1693,    -1,
      -1,  2084,  2085,    -1,    48,    -1,  1701,  1702,    -1,  1704,
      -1,    -1,    -1,    -1,    -1,  1710,    -1,    -1,  1596,    -1,
      -1,  1626,  1552,  1628,    -1,  1630,    -1,    48,   349,    -1,
    1635,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1736,    -1,    -1,    -1,    -1,    -1,    81,  1743,  1744,
    1745,    -1,    -1,    -1,  1749,    -1,   377,    -1,    92,    -1,
      -1,  1756,    -1,    -1,  1642,   386,  1596,    -1,  1646,   113,
     104,    -1,  1650,    -1,    -1,   119,    -1,   121,   122,   123,
     124,   125,   126,   127,   405,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   113,    -1,    -1,   416,    -1,    -1,   119,    -1,
     121,   122,   123,   124,   125,   126,   127,   141,   142,   143,
     144,   145,  1642,    -1,    -1,    -1,  1646,    -1,    -1,    -1,
    1650,   442,    -1,    -1,   445,   169,   170,    -1,   172,   163,
      -1,  1736,   166,    -1,     6,   169,   170,    -1,   172,   173,
     174,    -1,   176,    -1,    -1,    -1,    -1,    -1,   169,   170,
     194,   172,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   203,
      -1,    -1,    -1,    -1,   198,  1743,  1744,  1745,    -1,    -1,
      -1,  1749,    -1,   194,  1869,    -1,    48,    -1,  1756,    -1,
     501,    -1,   203,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,  1893,    -1,
      -1,    -1,  1897,  1898,    -1,    -1,    -1,    -1,  1903,    -1,
      -1,    -1,    -1,  1743,  1744,  1745,    -1,  1912,    -1,  1749,
      -1,    -1,    -1,    -1,  1919,  1920,  1756,    -1,  1923,  1924,
      59,    60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   113,  1937,    -1,    -1,    -1,    -1,   119,    -1,   121,
     122,   123,   124,   125,   126,   127,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1959,    -1,    -1,    -1,    -1,    -1,
      -1,   592,    -1,   594,    -1,    -1,   597,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,  1897,  1898,    -1,    -1,    -1,   169,   170,  1994,
     172,    -1,    -1,    -1,  1999,    -1,    -1,   136,   137,   630,
      -1,    -1,  2007,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,   194,    59,    60,  1903,    -1,    -1,    -1,  2024,
      -1,   203,    -1,    -1,    -1,  2030,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,   686,   687,    -1,    -1,    -1,
      -1,   200,    -1,  1903,   695,    69,    -1,    -1,    -1,    -1,
      -1,  1959,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2084,
    2085,    -1,    -1,    -1,    -1,   716,    -1,    -1,    -1,    -1,
     136,   137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1994,    -1,    -1,    -1,
      -1,  1999,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1959,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,  2024,    -1,    -1,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,  1994,    -1,    -1,    -1,    -1,  1999,
      -1,    -1,    -1,   794,    -1,    59,    60,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,  2024,    59,    60,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   826,  2084,  2085,    -1,    -1,
     204,   832,    31,   834,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,   862,   863,    -1,    -1,    -1,    -1,    -1,    -1,   870,
      69,    -1,   136,   137,  2084,  2085,   877,   878,   879,   880,
     881,   882,   883,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,   892,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   909,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    31,    57,   935,    -1,   200,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,   949,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    20,    -1,    -1,    -1,    -1,    68,
      -1,    -1,   973,   974,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,   984,    -1,    -1,    -1,    -1,    -1,    -1,
     991,    -1,    -1,    -1,    -1,    -1,    -1,   998,    -1,    -1,
      -1,   200,    -1,  1004,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   112,  1015,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,  1030,
     288,    -1,   290,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,    -1,  1050,
      -1,    -1,    -1,  1054,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    60,  1064,   163,    -1,    -1,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,   201,    -1,   203,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,
      -1,   349,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,
     199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1110,
    1111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1124,    -1,    -1,    -1,  1128,    -1,  1130,
      -1,    -1,  1133,    -1,    -1,    -1,    -1,    -1,   136,   137,
      -1,    -1,    -1,    -1,    -1,  1146,  1147,  1148,  1149,  1150,
    1151,  1152,  1153,    -1,    -1,  1156,  1157,  1158,  1159,  1160,
    1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,   239,   442,    -1,    -1,   445,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1202,   200,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,  1235,    57,  1237,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,  1250,
      -1,    -1,    31,    -1,    -1,    81,    -1,    83,    84,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,  1278,   104,    -1,
    1281,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,  1297,  1298,  1299,  1300,
    1301,  1302,    81,    -1,  1305,  1306,    -1,  1308,    87,    -1,
      -1,  1312,    -1,    -1,    -1,   141,   142,   143,   144,   145,
      -1,   377,    -1,    -1,    -1,   104,  1327,    -1,  1329,    -1,
     386,    -1,    -1,    -1,   592,    -1,   594,    -1,  1339,    -1,
      -1,   167,    -1,   169,   170,  1346,   172,   173,   174,   405,
    1351,    -1,  1353,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     416,   140,   141,   142,   143,   144,   145,   146,    -1,    -1,
      -1,    -1,   198,    -1,    -1,    -1,   202,  1378,    -1,   205,
      -1,   203,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,   176,    -1,    -1,
      -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,
      -1,    -1,    -1,  1414,  1415,    -1,    -1,  1418,    -1,   198,
     199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   686,   687,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   695,    -1,    -1,
      -1,    -1,    -1,    -1,  1445,   501,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1476,    -1,  1478,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
    1521,  1522,    -1,    -1,  1525,    -1,    -1,    -1,    -1,    -1,
    1531,    -1,  1533,    -1,  1535,    -1,    -1,    -1,    -1,  1540,
    1541,   597,    -1,    -1,  1545,    -1,  1547,    -1,    -1,  1550,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,  1563,  1564,    -1,    -1,    -1,    -1,    -1,  1570,
      -1,    -1,    -1,    -1,   630,    -1,  1577,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   870,    -1,    -1,    -1,    69,    -1,    -1,   877,
     878,    -1,    -1,    -1,    -1,  1626,    -1,  1628,    -1,  1630,
      -1,    -1,    -1,   288,  1635,   290,  1637,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1651,   203,    -1,    -1,    -1,  1656,    -1,    -1,    -1,    -1,
     716,    -1,    -1,    -1,    -1,    -1,    -1,  1668,  1669,    -1,
      -1,    -1,    -1,    -1,    -1,  1676,    -1,  1678,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,   349,    -1,    -1,    -1,    -1,    -1,
    1701,  1702,    -1,  1704,    -1,    -1,    -1,    -1,  1709,  1710,
      -1,    -1,    -1,    -1,    -1,   973,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    59,    60,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   991,    -1,  1736,    -1,    -1,   794,    -1,
     998,    -1,    -1,    -1,    -1,    -1,  1004,    -1,    31,    -1,
     203,    -1,  1753,  1754,  1755,    -1,    -1,    -1,    -1,  1760,
      -1,  1762,    -1,    -1,    -1,    -1,    -1,  1768,    -1,  1770,
     826,    -1,    -1,    -1,    -1,    -1,   832,    -1,   834,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,   442,    -1,    -1,
     445,    -1,  1050,    -1,    -1,    -1,    -1,    -1,    81,    -1,
     136,   137,    19,    20,    87,    -1,   862,   863,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,   879,   880,   881,   882,   883,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   892,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   909,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,   146,    -1,  1866,  1124,    -1,    -1,    -1,
    1128,    -1,  1130,  1874,    -1,  1133,    -1,    -1,    -1,   935,
     163,    -1,    -1,   166,   167,    -1,   169,   170,  1889,   172,
     173,   174,  1893,   949,    -1,    -1,  1897,  1898,    -1,    -1,
      81,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,
      -1,  1912,    -1,    -1,    -1,   198,   199,  1918,   974,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,   984,    -1,
      -1,    -1,    -1,  1934,  1935,    -1,    -1,   592,    -1,    -1,
    1941,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,  1015,
     141,   142,   143,   144,   145,    -1,    -1,  1968,    -1,    -1,
      -1,    -1,    -1,    -1,  1030,    -1,    -1,  1235,    -1,  1237,
      -1,    -1,  1983,    -1,    -1,    -1,    59,    60,   169,   170,
      -1,   172,   173,   174,  1995,    -1,  1997,    -1,  1054,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1064,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,   199,    -1,
    1278,    -1,   239,  1281,    -1,  2026,    -1,    -1,    -1,    -1,
      -1,   686,   687,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  2042,    -1,    -1,    -1,    -1,    -1,  2048,    -1,    -1,
      -1,  2052,    -1,    -1,  1110,  1111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   136,   137,  2066,  2067,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1339,    -1,    -1,    -1,    -1,    -1,    -1,  1346,    -1,
    1146,  1147,  1148,  1149,  1150,  1151,  1152,  1153,    -1,    38,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,    -1,    -1,
      -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    -1,    83,    84,  1202,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    -1,    -1,  1414,  1415,    -1,    -1,
     377,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,   386,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   121,    -1,    -1,    -1,    -1,  1445,   405,    -1,
      -1,    -1,    -1,    -1,  1250,    -1,    -1,    -1,    -1,   416,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,    -1,    -1,    31,   158,
      -1,    -1,   877,   878,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,
      -1,  1297,  1298,  1299,  1300,  1301,  1302,    -1,    -1,  1305,
    1306,    -1,  1308,    -1,   193,    68,  1312,    -1,    -1,   198,
     199,    -1,    -1,  1521,  1522,    -1,   205,  1525,    81,    -1,
      -1,  1327,    -1,  1329,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   501,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    31,    -1,    -1,  1351,    -1,  1353,    -1,   112,
      -1,    -1,    -1,    -1,    -1,  1563,    -1,    -1,   121,   122,
     123,   124,   125,   126,    -1,    -1,    -1,    -1,   973,  1577,
      -1,    -1,  1378,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,   146,    -1,    -1,   991,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    81,  1004,
     163,    -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,  1418,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,
     597,   194,    -1,    -1,    -1,   198,   199,    -1,   127,    -1,
      -1,    -1,    -1,  1651,    -1,  1050,    -1,   130,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,   141,   142,
     143,   144,   145,   630,    -1,    -1,    -1,    -1,  1676,    -1,
    1476,    -1,  1478,    -1,   163,    -1,    -1,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,   169,   170,    -1,   172,
     173,   174,    -1,  1701,  1702,    -1,  1704,    -1,    -1,    -1,
      -1,  1709,  1710,    -1,    -1,    -1,    -1,    -1,    -1,   198,
      -1,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,  1124,
      -1,    -1,    -1,  1128,    -1,  1531,    -1,  1533,  1133,  1535,
      -1,    -1,    -1,    -1,  1540,  1541,    -1,    -1,    -1,  1545,
      -1,  1547,    -1,    -1,  1550,    10,    11,    12,    -1,   716,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1564,    -1,
    1768,    -1,    -1,    -1,  1570,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    81,    57,    83,    -1,    85,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
    1626,    -1,  1628,    -1,  1630,    -1,    -1,   794,    -1,  1635,
    1235,  1637,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1656,   141,   142,   143,   144,   145,   597,    -1,    -1,    -1,
      31,    -1,  1668,  1669,    -1,   832,  1874,   834,    -1,    -1,
      -1,    -1,  1678,  1278,    -1,    -1,  1281,    -1,    -1,   169,
     170,  1889,   172,   173,   174,  1893,    -1,    -1,    -1,   630,
      -1,    -1,    -1,    -1,    -1,   862,   863,    68,    -1,    -1,
      -1,    -1,    -1,    -1,  1912,    -1,    -1,    -1,   198,    -1,
      81,    -1,   879,   880,   881,   882,   883,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   892,    -1,    -1,    -1,    -1,
    1736,    -1,    -1,   104,  1339,    -1,    -1,    -1,   203,    -1,
      -1,  1346,    -1,    -1,    -1,    -1,    -1,  1753,  1754,  1755,
      -1,    -1,    -1,    -1,  1760,    -1,  1762,    -1,    -1,    -1,
    1968,    -1,    -1,    -1,  1770,    -1,    -1,    -1,   935,   140,
     141,   142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1995,    -1,  1997,
      -1,    -1,   163,    -1,    -1,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,    -1,    -1,  1414,
    1415,    -1,    -1,    -1,    -1,    -1,   187,   984,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,   199,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    2048,    10,    11,    12,  2052,    -1,    -1,    -1,  1015,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2066,  2067,
    1866,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,  1054,    57,    -1,
      -1,  1897,  1898,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1918,    -1,    -1,    31,  1521,  1522,    -1,    -1,
    1525,    -1,   863,    -1,    -1,    -1,    -1,    -1,  1934,  1935,
      -1,    -1,    -1,    -1,    -1,  1941,    -1,    -1,   879,   880,
     881,   882,   883,  1110,  1111,    -1,    -1,    -1,    -1,    -1,
      -1,   892,    -1,    -1,    -1,    -1,    -1,    -1,  1563,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,  1577,    -1,    -1,    -1,    92,  1983,    -1,  1146,
    1147,  1148,  1149,  1150,  1151,  1152,  1153,    -1,   104,  1156,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,    -1,    -1,    -1,
    2026,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,   203,  1202,  2042,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1651,   163,    -1,    -1,
     166,    -1,    -1,   169,   170,    -1,   172,   173,   174,    -1,
     176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1676,    -1,    -1,  1015,    -1,    -1,    -1,    -1,    81,
      -1,    -1,   198,  1250,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,  1701,  1702,    -1,  1704,
      -1,    -1,   104,    -1,  1709,  1710,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,  1305,   141,
     142,   143,   144,   145,    -1,  1312,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1327,    -1,  1329,  1768,    -1,   167,    -1,   169,   170,   171,
     172,   173,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1351,    -1,  1353,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1146,  1147,    -1,    -1,  1150,
      -1,  1378,    -1,    -1,    -1,  1156,  1157,  1158,  1159,  1160,
    1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,  1202,    -1,    -1,    -1,    92,    10,    11,    12,  1874,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
     203,    -1,    -1,    -1,  1889,    31,    30,    31,  1893,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,   141,   142,   143,   144,   145,    -1,
      -1,    -1,    68,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,   163,    -1,    -1,   166,
      -1,    87,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,  1968,  1531,    -1,  1533,    -1,  1535,    -1,
      -1,   198,    -1,  1540,  1541,    -1,    -1,    -1,  1545,    -1,
    1547,    -1,    -1,  1550,    -1,    -1,    -1,    -1,  1329,    68,
    1995,    -1,  1997,    -1,   140,   141,   142,   143,   144,   145,
     146,    -1,    81,    -1,    83,    -1,    -1,    -1,    -1,    -1,
    1351,    -1,  1353,    -1,    -1,    -1,    -1,   163,    -1,    -1,
     166,   167,    -1,   169,   170,   104,   172,   173,   174,    -1,
     176,    -1,    -1,    -1,    -1,    -1,    -1,  1378,    -1,    -1,
      -1,   187,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,   203,
      -1,   140,   141,   142,   143,   144,   145,   146,    -1,    -1,
    1637,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,  1656,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,     7,    -1,   187,    -1,
      -1,    -1,    13,    -1,    -1,   194,    -1,    -1,    -1,   198,
     199,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,  1753,  1754,  1755,    -1,
    1531,    92,  1533,  1760,  1535,    -1,    -1,    -1,    -1,  1540,
      -1,    -1,  1769,   104,  1545,    -1,  1547,    -1,    -1,  1550,
      -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,   122,   123,   124,   125,   126,    -1,    -1,   129,   130,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,
     181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,
      -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,  1866,
      10,    11,    12,    -1,   205,   206,   207,   208,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1656,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,  1918,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1934,  1935,    -1,
      -1,    -1,    -1,    -1,  1941,    -1,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,  1970,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1753,  1754,  1755,    -1,  1983,    -1,    -1,  1760,
      -1,    48,    -1,    50,    51,    -1,    -1,    -1,   138,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,
      97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,    -1,   109,   110,   111,   112,   113,   114,   115,    -1,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,   129,   130,   131,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,  1866,    -1,    -1,   155,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,   188,   189,    -1,   191,    -1,   193,   194,   195,    -1,
      -1,   198,   199,    -1,   201,   202,    -1,  1918,   205,   206,
     207,   208,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,     7,  1934,  1935,    -1,    -1,    -1,    13,    -1,
    1941,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,  1983,    58,    59,    60,    61,    62,    63,    64,
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
     111,   112,    -1,   114,   115,    -1,   117,   118,    -1,   120,
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
      -1,    92,    93,    94,    95,    96,    97,    -1,    99,    -1,
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
      95,    -1,    97,    -1,    99,    -1,   101,   102,    -1,   104,
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
      73,    74,    -1,    -1,    77,    78,    79,    80,    81,    82,
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
      -1,    92,    93,    94,    95,    -1,    97,    -1,    99,   100,
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
      95,    -1,    97,    98,    99,    -1,   101,    -1,    -1,   104,
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
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,
      -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,
      -1,   202,    -1,    -1,   205,   206,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      54,    55,    57,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    83,    84,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    81,   130,    -1,   132,   133,   134,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,   163,    10,
      11,    12,    13,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,   177,    -1,    -1,   180,    -1,    28,    29,    -1,
      -1,    -1,    -1,   188,   141,   142,   143,   144,   145,    -1,
      -1,    -1,    -1,   198,    -1,    -1,    -1,   202,    -1,    -1,
     205,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,
     167,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    83,    84,    -1,    -1,    87,    -1,    -1,    -1,
      -1,   198,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    -1,   130,
      -1,   132,   133,   134,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,   163,    10,    11,    12,    13,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,   177,    -1,    -1,   180,
      -1,    28,    29,    -1,    31,    -1,    -1,   188,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,    -1,
      -1,   202,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,    -1,   130,    -1,    -1,   133,   134,   135,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,   104,
     177,    -1,    -1,   180,    -1,     3,     4,    -1,     6,     7,
     187,   188,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   198,   199,    -1,    -1,    -1,   203,    -1,    -1,    -1,
      28,    29,    -1,    31,    -1,    -1,   141,   142,   143,   144,
     145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    57,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      68,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,   198,    -1,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    -1,   130,    -1,    -1,   133,   134,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,   175,   104,   177,
      -1,    -1,   180,    -1,     3,     4,    -1,     6,     7,   187,
     188,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
     198,   199,    -1,    -1,    -1,   203,    -1,    -1,    -1,    28,
      29,    -1,    31,    -1,    -1,   141,   142,   143,   144,   145,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    57,    57,
     166,    -1,    -1,   169,   170,    -1,   172,   173,   174,    68,
      -1,    69,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,   198,    -1,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
      -1,   130,    -1,   132,   133,   134,   135,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,   146,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,   104,   177,    -1,
      -1,   180,    -1,     3,     4,    -1,     6,     7,   187,   188,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   198,
     199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,
      -1,    31,    -1,    -1,   141,   142,   143,   144,   145,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    57,    -1,   166,
      -1,    -1,   169,   170,    -1,   172,   173,   174,    68,    -1,
      69,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,
      -1,   198,    -1,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    -1,
     130,    -1,    -1,   133,   134,   135,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,   104,   177,    -1,    -1,
     180,    -1,    -1,    -1,   112,   113,    -1,   187,   188,   189,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,   198,   199,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,    28,    29,
      -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   166,    -1,
      -1,   169,   170,    -1,   172,   173,   174,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
     198,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    -1,
     130,    -1,    -1,   133,   134,   135,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,   104,   177,    -1,    -1,
     180,    -1,     3,     4,     5,     6,     7,   187,   188,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,   198,   199,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,    -1,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    57,    57,    -1,    -1,
      -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,    69,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
     198,    -1,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   163,   164,   165,    -1,    81,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,   177,   178,    -1,   180,
      -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,   189,   104,
     191,    -1,   193,   194,    -1,     3,     4,   198,     6,     7,
      -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    29,    -1,    -1,    -1,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    57,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,   198,    -1,    93,    94,    95,    96,    97,
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
     124,   125,   126,   127,   128,    -1,   130,    -1,   132,   133,
     134,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,   163,
      10,    11,    12,    13,    -1,   169,   170,    -1,   172,   173,
     174,   175,    -1,   177,    -1,    -1,   180,    -1,    28,    29,
      -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    -1,
     130,    -1,    -1,   133,   134,   135,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,   177,    -1,    -1,
     180,    10,    11,    12,    -1,    -1,    -1,    -1,   188,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,   203,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   203,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
     203,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   203,    57,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,   201,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,   201,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,   201,    57,    -1,    10,    11,    12,    -1,
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
      -1,    -1,    -1,    69,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,   201,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    -1,    -1,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,   200,    -1,   163,    -1,
      -1,   166,   167,    -1,   169,   170,    92,   172,   173,   174,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,   200,    -1,    56,    -1,    58,    -1,
      -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,   145,
      70,   147,   148,   149,   150,   151,    -1,    -1,    78,    79,
      80,    81,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    92,   169,   170,    -1,   172,   173,   174,    -1,
      -1,    -1,   178,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,   187,   188,    -1,    -1,    -1,    -1,   193,    -1,    -1,
      -1,    -1,   198,   199,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,    70,   147,   148,   149,
     150,   151,    -1,    -1,    78,    79,    80,    81,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    92,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,
      -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,   198,   199,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,   158,    83,    84,    -1,    -1,   163,
      -1,    -1,   166,   167,    92,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   121,   198,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,    70,    71,    -1,
     178,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,   193,    -1,    -1,    -1,    92,
     198,   199,    -1,    -1,   202,    -1,    -1,   205,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,   139,   140,   141,   142,
     143,   144,   145,    70,   147,   148,   149,   150,   151,    69,
      -1,    78,    79,    80,    81,   158,    83,    84,    -1,    -1,
     163,   164,   165,   166,   167,    92,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     193,    -1,    -1,    -1,   121,   198,   199,    -1,    -1,    -1,
      -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,
      -1,   198,   199,    -1,    -1,    -1,    30,    31,   205,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,    30,    31,    -1,    33,    34,    35,    36,
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
      -1,    69,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,    31,    -1,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
     138,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
     138,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
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
      50,    51,    52,    53,    54,    55,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,    -1,    -1,    -1,    -1,    -1,    -1,   121,
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
      -1,    -1,    -1,    -1,    31,    32,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    12,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    69,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    69,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    69,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
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
     477,   478,   490,   492,   494,   121,   122,   123,   163,   173,
     199,   216,   257,   337,   359,   379,   468,   359,   199,   359,
     359,   359,   359,   109,   359,   359,   454,   455,   359,   359,
     359,   359,    81,    83,    92,   121,   141,   142,   143,   144,
     145,   158,   199,   227,   379,   423,   426,   431,   468,   472,
     468,   359,   359,   359,   359,   359,   359,   359,   359,    38,
     359,   482,   121,   132,   199,   227,   270,   423,   424,   425,
     427,   431,   465,   466,   467,   476,   480,   481,   359,   199,
     358,   428,   199,   358,   370,   348,   359,   238,   358,   199,
     199,   199,   358,   201,   359,   216,   201,   359,     3,     4,
       6,     7,    10,    11,    12,    13,    28,    29,    31,    57,
      68,    71,    72,    73,    74,    75,    76,    77,    87,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   130,   132,   133,   134,   135,   139,
     140,   146,   163,   167,   175,   177,   180,   187,   188,   199,
     216,   217,   218,   229,   495,   516,   517,   520,    27,   201,
     353,   355,   359,   202,   250,   359,   112,   113,   163,   166,
     189,   219,   220,   221,   222,   226,    83,   205,   305,   306,
      83,   307,   123,   132,   122,   132,   199,   199,   199,   199,
     216,   276,   498,   199,   199,    70,    70,    70,    70,    70,
     348,    83,    91,   159,   160,   161,   487,   488,   166,   202,
     226,   226,   216,   277,   498,   167,   199,   199,   498,   498,
      83,   195,   202,   371,    28,   347,   350,   359,   361,   468,
     473,   233,   202,   478,    91,   429,   487,    91,   487,   487,
      32,   166,   183,   499,   199,     9,   201,   199,   346,   360,
     469,   472,   118,    38,   256,   167,   275,   498,   121,   194,
     257,   338,    70,   202,   463,   201,   201,   201,   201,   201,
     201,   201,   201,    10,    11,    12,    30,    31,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    57,    69,   201,    14,    70,    70,   202,   162,   133,
     173,   175,   189,   191,   278,   336,   337,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      59,    60,   136,   137,   458,   463,   463,   199,   199,    70,
     202,   256,   257,    14,    14,   359,   201,   138,    49,   216,
     453,    91,   347,   361,   162,   468,   138,     9,   204,   271,
     347,   361,   468,   499,   162,   199,   430,   458,   463,   200,
     359,    32,   236,     8,   372,     9,   201,   236,   237,   348,
     349,   359,   216,   290,   240,   201,   201,   201,   140,   146,
     520,   520,   183,   519,   199,   112,   520,    14,   162,   140,
     146,   163,   216,   218,   201,   201,   201,   251,   116,   180,
     201,   219,   221,   219,   221,   219,   221,   226,   219,   221,
     202,     9,   439,   201,   103,   166,   202,   468,     9,   201,
      14,     9,   201,   132,   132,   468,   491,   348,   347,   361,
     468,   472,   473,   200,   183,   268,   482,   482,   359,   380,
     381,   348,   404,   404,   380,   404,   201,    70,   458,   159,
     488,    82,   359,   468,    91,   159,   488,   226,   215,   201,
     202,   263,   273,   413,   415,    92,   199,   373,   374,   376,
     422,   426,   475,   477,   492,   404,    14,   103,   493,   367,
     368,   369,   300,   301,   456,   457,   200,   200,   200,   200,
     200,   203,   235,   236,   258,   265,   272,   456,   359,   206,
     207,   208,   216,   500,   501,   520,    38,    87,   176,   303,
     304,   359,   495,   247,   248,   347,   355,   356,   359,   361,
     468,   202,   249,   249,   249,   249,   199,   498,   266,   256,
     359,   479,   359,   359,   359,   359,   359,    32,   359,   359,
     359,   359,   359,   359,   359,   359,   359,   359,   359,   359,
     359,   359,   359,   359,   359,   359,   359,   359,   359,   359,
     359,   359,   427,   359,   350,   355,   359,   479,   479,   359,
     483,   484,   132,   202,   217,   218,   478,   276,   216,   277,
     498,   498,   275,   257,    38,   350,   353,   355,   359,   359,
     359,   359,   359,   359,   359,   359,   359,   359,   359,   359,
     359,   167,   202,   216,   459,   460,   461,   462,   478,   303,
     303,   479,   359,   256,   200,   359,   199,   452,     9,   438,
     200,   200,    38,   359,    38,   359,   430,   200,   200,   200,
     476,   477,   478,   303,   202,   216,   459,   460,   478,   200,
     233,   294,   202,   355,   359,   359,    95,    32,   236,   288,
     201,    27,   103,    14,     9,   200,    32,   202,   291,   520,
      31,    92,   176,   229,   513,   514,   515,   199,     9,    50,
      51,    56,    58,    70,   140,   141,   142,   143,   144,   145,
     187,   188,   199,   227,   387,   390,   393,   396,   399,   402,
     408,   423,   431,   432,   434,   435,   216,   518,   233,   199,
     244,   202,   201,   202,   201,   202,   201,   103,   166,   202,
     201,   112,   113,   166,   222,   223,   224,   225,   226,   222,
     216,   359,   306,   432,    83,     9,   200,   200,   200,   200,
     200,   200,   200,   201,    50,    51,   509,   511,   512,   134,
     281,   200,   200,   138,   204,     9,   438,     9,   438,   204,
     204,   204,   204,    83,    85,   216,   489,   216,    70,   203,
     203,   212,   214,    32,   135,   280,   182,    54,   167,   182,
     202,   417,   361,   138,     9,   438,   200,   162,   200,   520,
     520,    14,   372,   300,   231,   196,     9,   439,    87,   520,
     521,   458,   458,   203,     9,   438,   184,   468,    83,    84,
     302,   359,   200,     9,   439,    14,     9,   200,     9,   200,
     200,   200,   200,    14,   200,   203,   234,   235,   364,   259,
     134,   279,   199,   498,   204,   203,   359,    32,   204,   204,
     138,   203,     9,   438,   359,   499,   199,   269,   264,   274,
      14,   493,   267,   256,    71,   468,   359,   499,   200,   200,
     204,   203,    50,    51,    70,    78,    79,    80,    92,   140,
     141,   142,   143,   144,   145,   158,   187,   188,   216,   388,
     391,   394,   397,   400,   403,   423,   434,   441,   443,   444,
     448,   451,   216,   468,   468,   138,   279,   458,   463,   458,
     200,   359,   295,    75,    76,   296,   231,   358,   233,   349,
     103,    38,   285,   379,   468,   432,   216,    32,   236,   289,
     201,   292,   201,   292,     9,   438,    92,   229,   138,   162,
       9,   438,   200,    87,   502,   503,   520,   521,   500,   432,
     432,   432,   432,   432,   437,   440,   199,    70,    70,    70,
      70,    70,   199,   199,   432,   162,   202,    10,    11,    12,
      31,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    69,   162,   499,   203,   423,   202,   253,
     221,   221,   221,   216,   221,   222,   222,   226,     9,   439,
     203,   203,    14,   468,   201,   184,     9,   438,   216,   282,
     423,   202,   359,   359,   204,   359,   203,   212,   520,   282,
     202,   416,   176,    14,   200,   359,   373,   478,   201,   520,
     196,   203,   232,   235,   245,    32,   507,   457,   521,    38,
      83,   176,   459,   460,   462,   459,   460,   462,   520,    70,
      38,    87,   176,   359,   432,   248,   355,   356,   468,   249,
     248,   249,   249,   203,   235,   300,   199,   423,   280,   365,
     260,   359,   359,   359,   203,   199,   303,   281,    32,   280,
     520,    14,   279,   498,   427,   203,   199,    78,    79,    80,
     216,   442,   442,   444,   446,   447,    52,   199,    70,    70,
      70,    70,    70,    91,   159,   199,   199,   162,     9,   438,
     200,   452,    38,   359,   280,   203,    75,    76,   297,   358,
     236,   203,   201,    96,   201,   285,   468,   138,   284,    14,
     233,   292,   106,   107,   108,   292,   203,   520,   184,   138,
     162,   520,   216,   176,   513,   520,     9,   438,   200,   176,
     438,   138,   204,     9,   438,   437,   382,   383,   432,   405,
     432,   433,   405,   382,   405,   373,   375,   377,   405,   200,
     132,   217,   432,   485,   486,   432,   432,   432,    32,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   518,    83,   254,   203,   203,   203,   203,
     225,   201,   432,   512,   103,   104,   508,   510,     9,   311,
     138,   204,   203,   493,   311,   168,   181,   202,   412,   419,
     359,   168,   202,   418,   138,   201,   507,   199,   248,   346,
     360,   469,   472,   520,   372,    87,   521,    83,    83,   176,
      14,    83,   499,   499,   479,   468,   302,   359,   200,   300,
     202,   300,   199,   138,   199,   303,   200,   202,   520,   202,
     201,   520,   280,   261,   430,   303,   138,   204,     9,   438,
     443,   446,   384,   385,   444,   406,   444,   445,   406,   384,
     406,   159,   373,   449,   450,   406,    81,   444,   468,   202,
     358,    32,    77,   236,   201,   349,   284,   285,   200,   432,
     102,   106,   201,   359,    32,   201,   293,   203,   184,   520,
     216,   138,    87,   520,   521,    32,   200,   432,   432,   200,
     204,     9,   438,   138,   204,     9,   438,   204,   204,   204,
     138,     9,   438,   200,   200,   138,   203,     9,   438,   432,
      32,   200,   233,   201,   201,   201,   201,   216,   520,   520,
     508,   423,     6,   113,   119,   122,   124,   125,   126,   127,
     169,   170,   172,   203,   312,   335,   336,   337,   340,   342,
     343,   344,   345,   456,   359,   203,   202,   203,    54,   359,
     203,   359,   359,   372,   468,   201,   202,   521,    38,    83,
     176,    14,    83,   359,   199,   199,   204,   507,   200,   311,
     200,   300,   359,   303,   200,   311,   493,   311,   201,   202,
     199,   200,   444,   444,   200,   204,     9,   438,   138,   204,
       9,   438,   204,   204,   204,   138,   200,     9,   438,   200,
     311,    32,   233,   201,   200,   200,   241,   201,   201,   293,
     233,   138,   520,   520,   176,   520,   138,   432,   432,   432,
     432,   373,   432,   432,   432,   202,   203,   510,   134,   135,
     189,   217,   496,   520,   283,   423,   113,   345,    31,   127,
     140,   146,   167,   173,   319,   320,   321,   322,   423,   171,
     327,   328,   130,   199,   216,   329,   330,    83,   341,   257,
     520,   113,     9,   201,     9,   201,   201,   493,   336,   337,
     308,   167,   414,   203,   203,   359,    83,    83,   176,    14,
      83,   359,   303,   303,   119,   362,   507,   203,   507,   200,
     200,   203,   202,   203,   311,   300,   138,   444,   444,   444,
     444,   373,   203,   233,   239,   242,    32,   236,   287,   233,
     520,   200,   432,   138,   138,   138,   233,   423,   423,   498,
      14,   217,     9,   201,   202,   496,   493,   322,   183,   202,
       9,   201,     3,     4,     5,     6,     7,    10,    11,    12,
      13,    27,    28,    29,    57,    71,    72,    73,    74,    75,
      76,    77,    87,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   139,   140,   147,   148,   149,
     150,   151,   163,   164,   165,   175,   177,   178,   180,   187,
     189,   191,   193,   194,   216,   420,   421,     9,   201,   167,
     171,   216,   330,   331,   332,   201,    14,     9,   201,   256,
     341,   496,   496,   496,    14,   257,   341,   520,   203,   309,
     310,   496,    14,    83,   359,   200,   200,   199,   507,   198,
     504,   362,   507,   308,   203,   200,   444,   138,   138,    32,
     236,   286,   287,   233,   432,   432,   432,   203,   201,   201,
     432,   423,   315,   520,   323,   324,   431,   320,    14,    32,
      51,   325,   328,     9,    36,   200,    31,    50,    53,   432,
      83,   218,   497,   201,    14,    14,   520,   256,   201,   341,
     201,    14,   359,    38,    83,   411,   202,   505,   506,   520,
     201,   202,   333,   507,   504,   203,   507,   444,   444,   233,
     100,   252,   203,   216,   229,   316,   317,   318,     9,   438,
       9,   438,   203,   432,   421,   421,    68,   326,   331,   331,
      31,    50,    53,    14,   183,   199,   432,   432,   497,   201,
     432,    83,     9,   439,   231,     9,   439,    14,   508,   231,
     202,   333,   333,    98,   201,   116,   243,   162,   103,   520,
     184,   431,   174,   432,   509,   313,   199,    38,    83,   200,
     203,   506,   520,   203,   231,   201,   199,   180,   255,   216,
     336,   337,   184,   184,   298,   299,   457,   314,    83,   203,
     423,   253,   177,   216,   201,   200,     9,   439,    87,   124,
     125,   126,   339,   340,   298,    83,   283,   201,   507,   457,
     521,   521,   200,   200,   201,   504,    87,   339,    83,    38,
      83,   176,   507,   202,   201,   202,   334,   521,   521,    83,
     176,    14,    83,   504,   233,   231,    83,    38,    83,   176,
      14,    83,   359,   334,   203,   203,    83,   176,    14,    83,
     359,    14,    83,   359,   359
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
     379,   379,   379,   380,   380,   381,   381,   382,   382,   383,
     383,   384,   384,   385,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   394,   395,   396,   397,   398,   399,   400,
     401,   402,   403,   404,   404,   405,   405,   406,   406,   407,
     408,   409,   409,   410,   410,   410,   410,   410,   410,   410,
     410,   410,   410,   410,   410,   411,   411,   411,   411,   412,
     413,   413,   414,   414,   415,   415,   415,   416,   416,   417,
     418,   418,   419,   419,   419,   420,   420,   420,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   422,   423,   423,   424,   424,   424,   424,   424,   425,
     425,   426,   426,   426,   426,   427,   427,   427,   428,   428,
     428,   429,   429,   429,   430,   430,   431,   431,   431,   431,
     431,   431,   431,   431,   431,   431,   431,   431,   431,   431,
     431,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   433,   433,   434,
     435,   435,   436,   436,   436,   436,   436,   436,   436,   437,
     437,   438,   438,   439,   439,   440,   440,   440,   440,   441,
     441,   441,   441,   441,   442,   442,   442,   442,   443,   443,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   444,   444,   445,   445,   446,   446,
     447,   447,   447,   447,   448,   448,   449,   449,   450,   450,
     451,   451,   452,   452,   453,   453,   455,   454,   456,   457,
     457,   458,   458,   459,   459,   459,   460,   460,   461,   461,
     462,   462,   463,   463,   464,   464,   464,   465,   465,   466,
     466,   467,   467,   468,   468,   468,   468,   468,   468,   468,
     468,   468,   468,   468,   469,   470,   470,   470,   470,   470,
     470,   470,   470,   471,   471,   471,   471,   471,   471,   471,
     471,   471,   471,   471,   472,   473,   473,   474,   474,   474,
     475,   475,   475,   476,   477,   477,   477,   478,   478,   478,
     478,   479,   479,   480,   480,   480,   480,   480,   480,   481,
     481,   481,   481,   481,   482,   482,   482,   482,   482,   482,
     482,   482,   482,   482,   483,   483,   484,   484,   484,   484,
     485,   485,   486,   486,   486,   486,   487,   487,   487,   487,
     488,   488,   488,   488,   488,   488,   489,   489,   489,   490,
     490,   490,   490,   490,   490,   490,   490,   490,   490,   490,
     491,   491,   492,   492,   493,   493,   494,   494,   494,   494,
     495,   495,   496,   496,   497,   497,   498,   498,   499,   499,
     500,   500,   501,   502,   502,   502,   502,   502,   502,   503,
     503,   503,   503,   504,   504,   505,   505,   506,   506,   507,
     507,   508,   508,   509,   510,   510,   511,   511,   511,   511,
     512,   512,   512,   513,   513,   513,   513,   514,   514,   515,
     515,   515,   515,   516,   517,   518,   518,   519,   519,   520,
     520,   520,   520,   520,   520,   520,   520,   520,   520,   520,
     521,   521
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
       2,     1,     1,     4,     1,     4,     1,     4,     1,     4,
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
       3,     1,     1,     0,     1,     2,     4,     3,     3,     3,
       2,     3,     2,     3,     3,     3,     1,     1,     1,     1,
       1,     3,     3,     3,     4,     6,     3,     3,     3,     3,
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
       4,     3,     4,     2,     0,     5,     3,     2,     0,     5,
       3,     2,     0,     5,     3,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     2,     0,     2,     0,     2,     0,     4,
       4,     4,     4,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     3,     4,     1,     2,     4,
       2,     6,     0,     1,     0,     5,     4,     2,     0,     1,
       1,     3,     1,     3,     1,     1,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     1,     1,     1,     1,     1,     1,     3,     1,
       3,     1,     1,     1,     3,     1,     1,     1,     2,     1,
       0,     0,     1,     1,     3,     0,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       2,     1,     1,     4,     3,     4,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     5,     4,     3,     1,     3,
       3,     1,     1,     1,     1,     1,     3,     3,     3,     2,
       0,     1,     0,     1,     0,     5,     3,     3,     1,     1,
       1,     1,     3,     2,     1,     1,     1,     1,     1,     3,
       1,     1,     1,     3,     1,     2,     2,     4,     3,     4,
       1,     1,     1,     1,     1,     1,     3,     1,     2,     0,
       5,     3,     3,     1,     3,     1,     2,     0,     5,     3,
       2,     0,     3,     0,     4,     2,     0,     3,     3,     1,
       0,     1,     1,     1,     1,     3,     1,     1,     1,     3,
       1,     1,     3,     3,     2,     2,     2,     2,     4,     5,
       5,     5,     5,     1,     1,     1,     1,     1,     1,     3,
       3,     4,     4,     3,     3,     1,     1,     1,     1,     3,
       1,     4,     3,     1,     1,     1,     1,     1,     3,     3,
       1,     1,     4,     4,     3,     1,     1,     7,     9,     9,
       7,     6,     8,     1,     4,     4,     1,     1,     1,     4,
       2,     1,     0,     1,     1,     1,     3,     3,     3,     0,
       1,     1,     3,     3,     4,     3,     2,     1,     5,     6,
       4,     3,     2,     0,     2,     0,     5,     3,     3,     1,
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
#line 821 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
#line 7392 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 824 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 7400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 831 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 7406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 832 "hphp.y" /* yacc.c:1646  */
    { }
#line 7412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 835 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 7418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 836 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 837 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 838 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 839 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 840 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 7448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 841 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 7456 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 844 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 7463 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 846 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 7469 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 847 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7475 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 848 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 7481 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 849 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7487 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 850 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7495 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 854 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7504 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 859 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7513 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 864 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7520 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 867 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 870 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 874 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7543 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 878 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 882 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 886 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7567 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 889 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7574 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 894 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7580 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 895 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7586 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 896 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7592 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 897 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7598 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 898 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7604 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 899 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7610 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 900 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7616 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 901 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 902 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 903 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7634 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 904 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7640 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 905 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7646 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 906 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7652 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 988 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7658 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 990 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7664 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 995 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7670 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 996 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7677 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 1002 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7683 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 1006 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7689 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 1007 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7695 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 1009 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7701 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 1011 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7707 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 1016 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7713 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 1017 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7720 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 1023 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7726 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 1027 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 1029 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 1031 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7747 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 1036 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7753 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7759 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 1041 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7765 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 1044 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7777 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 1049 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7786 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 1056 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1064 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1067 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7809 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1073 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 7815 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1074 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval)); }
#line 7821 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1079 "hphp.y" /* yacc.c:1646  */
    {
                                         _p->onUsing((yyval), (yyvsp[-2]), true, (yyvsp[-1]), nullptr);
                                       }
#line 7829 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7835 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1087 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7841 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1092 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 7847 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1093 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval), (yyval), (yyvsp[0])); }
#line 7854 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1098 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7860 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1099 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7866 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1100 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7872 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1101 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7878 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1104 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7884 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1108 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7890 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1113 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7896 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1114 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7903 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1116 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1120 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7918 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1123 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7926 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1127 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7933 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1129 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1132 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7948 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1134 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7956 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1137 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7962 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1138 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1139 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1140 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7980 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1141 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7986 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1142 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7992 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1143 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7998 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1144 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 8004 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1145 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 8010 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1146 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 8016 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1147 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 8022 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1148 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 8028 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1149 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 8034 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1150 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 8040 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1151 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 8047 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1155 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 8054 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1157 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 8062 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1162 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 8069 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1164 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 8077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1168 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 8085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1177 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 8091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1178 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1181 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false); }
#line 8103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1182 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 8109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1184 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1188 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8125 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1192 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8133 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1196 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1200 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1204 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1209 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8165 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1212 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 8171 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1213 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 8180 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1217 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8186 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1218 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8192 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1219 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8198 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1220 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8204 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1221 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8210 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1222 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8216 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1223 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 8222 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1224 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8228 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1225 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8234 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1226 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 8240 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1227 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8246 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1228 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 8256 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1250 "hphp.y" /* yacc.c:1646  */
    { _p->pushLabelScope(LS::Using);
                                         _p->onNewLabelScope(false);
                                         (yyval) = (yyvsp[-1]); }
#line 8264 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1256 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1; }
#line 8270 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1257 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8276 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1266 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), nullptr, (yyvsp[-2]));
                                         _p->onExprListElem((yyval), &(yyval), (yyvsp[0])); }
#line 8283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1268 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0])); }
#line 8289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1278 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1279 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1283 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false); }
#line 8307 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1284 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8313 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1293 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 8319 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1294 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8325 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1298 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::Finally);}
#line 8332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1300 "hphp.y" /* yacc.c:1646  */
    { _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->popLabelScope();
                                         _p->onCompleteLabelScope(false);}
#line 8340 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1306 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8346 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1307 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8352 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1311 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 8358 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1312 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8364 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1316 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 8370 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1322 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8379 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1329 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8389 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1337 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8398 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1344 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8408 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1352 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8417 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1358 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8427 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1367 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1371 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 8440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1375 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8447 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1379 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 8453 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1385 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1388 "hphp.y" /* yacc.c:1646  */
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
#line 8478 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1403 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1406 "hphp.y" /* yacc.c:1646  */
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
#line 8503 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1420 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8510 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1423 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8518 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1428 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8525 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1431 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8533 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1437 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 8539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1440 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 8545 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1444 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8552 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1447 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8563 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1455 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8570 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1458 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1467 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 8594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1471 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8600 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1474 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8606 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1477 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 8612 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1478 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 8618 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1479 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 8626 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1482 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 8632 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 8638 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1488 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1491 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1492 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1495 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1496 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1499 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8680 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1501 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8686 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1504 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1506 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8698 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1510 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8704 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1511 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8710 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1514 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8716 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1515 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8722 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1516 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[0]), NULL);}
#line 8728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1520 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8734 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1522 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1525 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1527 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1530 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1532 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8764 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1535 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1537 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1541 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1543 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1548 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1549 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8801 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1550 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8807 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1551 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8813 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1556 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8819 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1558 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1559 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8831 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1562 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8837 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1563 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8843 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1568 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8849 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1569 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8855 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1574 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8861 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1575 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8867 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1578 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8873 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1579 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8879 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1582 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8885 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1583 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8891 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1591 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8898 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1597 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1603 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8913 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1607 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1611 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8926 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1616 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8933 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1621 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1624 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1630 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8955 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1635 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8963 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1640 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1646 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8979 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1652 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8987 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1658 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1664 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 9003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1670 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 9011 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1677 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 9019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1684 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 9027 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1693 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 9034 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1698 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 9041 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1703 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 9049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1707 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9055 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1710 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 9062 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1714 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 9069 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1718 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 9077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1721 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9083 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1726 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1730 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1734 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9107 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1739 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1744 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1749 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9131 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1754 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1759 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9147 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1765 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9155 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1771 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1777 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 9169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1778 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 9175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1779 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 9181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1784 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1785 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1788 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,false);}
#line 9200 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1790 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::InOut,false);}
#line 9207 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1792 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::Ref,false);}
#line 9214 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1794 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,true);}
#line 9221 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1797 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::In,false);}
#line 9228 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1800 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::In,true);}
#line 9235 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1803 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::Ref,false);}
#line 9242 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1806 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::InOut,false);}
#line 9249 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1811 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1812 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 9261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1815 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9267 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1816 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 9273 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1817 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 9279 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1821 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9285 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1823 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9291 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1824 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 9297 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1825 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9303 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1830 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9309 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1831 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9315 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1834 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL,NULL);}
#line 9322 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1839 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9328 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1845 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9334 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9340 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1850 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-2]),(yyvsp[-1]),NULL,NULL);}
#line 9347 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1854 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-2]),NULL);}
#line 9354 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1858 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-2]),(yyvsp[-1]),NULL,&(yyvsp[-3]));}
#line 9361 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1863 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-2]),&(yyvsp[-4]));}
#line 9368 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1865 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL,NULL);}
#line 9375 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1868 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL,NULL,true);}
#line 9382 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1870 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9388 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1873 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9396 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1880 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1888 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9414 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1895 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1901 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 9430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1903 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1905 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1907 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 9448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1909 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 9454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1910 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 9461 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1913 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 9467 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1916 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1917 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1918 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 9485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1924 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 9491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1929 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 9498 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1932 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 9506 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1939 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 9512 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1940 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 9519 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1945 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 9526 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1948 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 9532 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1955 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 9539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 9545 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1961 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1966 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9557 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1968 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9563 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1970 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1971 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 9580 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1977 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 9586 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1979 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 9592 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1980 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 9598 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1984 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval),nullptr,(yyvsp[0])); }
#line 9604 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval),&(yyvsp[-2]),(yyvsp[0])); }
#line 9610 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9616 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1995 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 9628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 9634 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 9640 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),&t,0);}
#line 9647 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),&t,0);}
#line 9654 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 2012 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 9661 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 9667 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 9674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 2020 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 9680 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 2024 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 9686 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 2025 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 9692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 2026 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 9698 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 9704 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 2031 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9710 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 2032 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 9716 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 2033 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 9722 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 2034 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 9728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 2035 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 9734 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 2037 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 9740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 2039 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 9746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 2043 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9754 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 2046 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9760 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 2047 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9766 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 2051 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9772 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 2052 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9778 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 2056 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9784 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 2057 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9790 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 2060 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9796 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 2061 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 2064 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 2065 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9814 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 2068 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9820 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 2070 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9826 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 2073 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9832 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 2074 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9838 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 2075 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9844 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 2076 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 2077 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9856 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 2078 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9862 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 2079 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 2083 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9874 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 2084 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9880 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 2087 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9886 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 2088 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 2089 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9898 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 2093 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9904 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 2095 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9910 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 2096 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9916 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 2097 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9922 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 2101 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9928 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 2103 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9934 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 2106 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9940 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 2112 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9946 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 2114 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9952 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 2118 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9960 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 2122 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9967 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 2126 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9973 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 2130 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9979 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 2134 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9985 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 2136 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9991 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 2137 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9997 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 2138 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 10003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 2139 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 2140 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 2143 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 10021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 2144 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 10027 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10033 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 2149 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10039 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 2153 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 10045 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 2154 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 10051 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 2155 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 10057 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 2156 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10063 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 2160 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10069 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 2165 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-2]), &(yyvsp[0]), true);}
#line 10075 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 2169 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 10081 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 2173 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10087 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 2177 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 10093 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 2181 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 2186 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-2]), &(yyvsp[0]), true);}
#line 10105 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 2190 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10111 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 2194 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 2195 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 2196 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 2197 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 2198 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 2202 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10147 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 2207 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-2]), &(yyvsp[0]));}
#line 10153 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 10159 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 2209 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 10165 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 2212 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 10171 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2213 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 10177 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2214 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 10183 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2215 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 10189 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2216 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 10195 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2217 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 10201 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2218 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 10207 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2219 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 10213 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2220 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 10219 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2221 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 10225 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2222 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 10231 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2223 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 10237 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2224 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 10243 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2225 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 10249 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2226 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 10255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2227 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 10261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2228 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 10267 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2229 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 10273 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2230 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 10279 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2231 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 10285 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2232 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 10291 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2233 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 10297 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2234 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 10303 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2235 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 10309 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2236 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 10315 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2237 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 10321 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2238 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 10327 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2239 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 10333 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2240 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 10339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2241 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 10345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2242 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 10351 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2243 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 10357 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2244 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 10363 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2245 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 10369 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2246 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 10375 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2247 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 10381 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2248 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 10387 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2249 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 10393 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2250 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 10399 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2251 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 10405 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2252 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 10411 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2253 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 10417 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2254 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 10423 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2255 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 10429 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2256 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 10436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2258 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 10442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2259 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 10449 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2261 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 10455 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2263 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 10461 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2264 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10467 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2265 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 10473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2266 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 10479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2267 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 10485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2268 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2269 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 10497 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2270 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 10503 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2271 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 10509 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2272 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 10515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2273 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 10521 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2274 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 10527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2275 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 10533 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2276 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 10539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2277 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 10545 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2278 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2279 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10557 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2280 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10563 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2281 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2282 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2283 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2284 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2285 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10593 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2286 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10599 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2287 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 10605 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2288 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 10611 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2289 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10617 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2296 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 10623 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2297 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10629 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2302 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10638 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2308 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2317 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10659 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2323 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10671 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2334 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,NULL,NULL);}
#line 10685 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2343 "hphp.y" /* yacc.c:1646  */
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
#line 10700 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2354 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10710 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2362 "hphp.y" /* yacc.c:1646  */
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
#line 10725 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2373 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2379 "hphp.y" /* yacc.c:1646  */
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
#line 10752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2391 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,NULL,NULL);}
#line 10766 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2400 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10779 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2408 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2416 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2427 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2428 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10814 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2430 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10820 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2434 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10827 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2436 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2443 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0); }
#line 10839 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2446 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0); }
#line 10845 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2453 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0); }
#line 10851 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2456 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0); }
#line 10857 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2462 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10869 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2467 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2468 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10881 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2472 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval), (yyvsp[-1]));}
#line 10887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2476 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10893 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2477 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2478 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2484 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2489 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 10923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2490 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 10929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2495 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2496 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2502 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 10947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2504 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 10953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2509 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2510 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10965 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2516 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 10971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2518 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 10977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2522 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2526 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2530 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2534 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 11001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2538 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 11007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2542 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 11013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2546 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 11019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2550 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 11025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2554 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 11031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2558 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2562 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2566 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2570 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11055 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2574 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11061 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2578 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11067 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2582 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2586 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11079 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2590 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2595 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2596 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2601 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2602 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2607 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2608 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11121 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2613 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2620 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11137 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2627 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11143 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2629 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2633 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11155 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2634 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11161 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2635 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11167 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11173 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2637 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11179 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2638 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11185 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11191 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2640 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11197 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2641 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11203 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 11210 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2644 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11216 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2645 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11222 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2649 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11228 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 11234 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2651 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 11240 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2652 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 11246 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 11252 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2662 "hphp.y" /* yacc.c:1646  */
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
#line 11270 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2677 "hphp.y" /* yacc.c:1646  */
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
#line 11288 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 11294 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2693 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 11300 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2696 "hphp.y" /* yacc.c:1646  */
    { _p->onXhpAttributesStart(); (yyval).reset();}
#line 11306 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2698 "hphp.y" /* yacc.c:1646  */
    { _p->onXhpAttributeSpread((yyval), &(yyvsp[-4]), (yyvsp[-1]));}
#line 11312 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2701 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 11318 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2704 "hphp.y" /* yacc.c:1646  */
    { _p->onOptExprListElem((yyval), &(yyvsp[-1]), (yyvsp[0])); }
#line 11324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2705 "hphp.y" /* yacc.c:1646  */
    {  (yyval).reset();}
#line 11330 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2708 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2715 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11351 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 11363 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11369 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2726 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11375 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11381 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2732 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 11387 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 11393 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11399 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11405 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11411 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11417 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2742 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11423 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2743 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11429 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2744 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11435 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2745 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11441 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2746 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11447 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2747 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11453 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2748 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11459 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2749 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11465 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2750 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11471 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2751 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11477 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2752 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11483 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2753 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11489 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2754 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11495 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2755 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11501 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2756 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11507 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2757 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11513 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2758 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11519 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2759 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11525 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2760 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11531 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2761 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11537 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2762 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11543 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2763 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11549 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2764 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11555 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2765 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11561 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2766 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11567 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2767 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11573 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2768 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11579 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2769 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11585 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2770 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11591 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2771 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11597 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2772 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11603 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2773 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11609 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2774 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11615 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2775 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11621 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2776 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11627 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2777 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11633 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2778 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11639 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2779 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11645 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2780 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11651 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2781 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11657 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2782 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11663 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2783 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11669 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2784 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11675 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2785 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11681 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2786 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11687 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2787 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11693 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2788 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11699 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2789 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11705 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2790 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11711 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2791 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11717 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2792 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11723 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2793 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11729 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2794 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2795 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11741 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2796 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11747 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2797 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11753 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2798 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11759 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2799 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11765 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2800 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2801 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11777 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2802 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11783 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2803 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2804 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2805 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11801 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2806 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11807 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2807 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11813 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2808 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11819 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2809 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2810 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11831 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2811 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11837 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2812 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11843 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2813 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11849 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2814 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11855 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2815 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11861 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2816 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11867 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2817 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11873 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2818 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11879 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2819 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11885 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2820 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11891 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2825 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11897 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2829 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11903 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2830 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11909 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2834 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11915 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2835 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11921 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2836 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11927 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2837 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11934 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2839 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2843 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2852 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2855 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2856 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11966 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2858 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11973 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2868 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11979 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2872 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11985 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2873 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11991 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2874 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11997 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2878 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2879 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2880 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2884 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2885 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 12027 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2886 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12033 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2890 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12039 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2891 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12045 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2895 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12051 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2896 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12057 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2897 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12063 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2898 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 12070 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2900 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 12076 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2901 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 12082 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2902 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 12088 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2903 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 12094 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2904 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 12100 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2905 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 12106 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2906 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 12112 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2907 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 12118 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2908 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 12124 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2911 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12130 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2913 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12136 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2917 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12142 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2918 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12148 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2920 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12154 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2921 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12160 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2923 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1]));}
#line 12166 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2924 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12172 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2925 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12178 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2926 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12184 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2927 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12190 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2928 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12196 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2929 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12202 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2930 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12208 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2931 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12214 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2932 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2934 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 12226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2936 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 12232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2938 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 12238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2940 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 12244 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2942 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 12250 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2943 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 12256 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2944 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 12262 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2945 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 12268 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2946 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 12274 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2947 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 12280 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2948 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 12286 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2949 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 12292 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2950 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 12298 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2951 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 12304 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2952 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 12310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2953 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 12316 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2954 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 12322 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2955 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 12328 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2956 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 12334 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2957 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12340 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2958 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12346 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2960 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 12352 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2962 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 12358 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2964 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 12364 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2966 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 12370 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2967 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 12376 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2969 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 12383 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2971 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 12389 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2974 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 12396 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2978 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 12402 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2981 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 12408 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2982 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 12414 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2986 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12420 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2987 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12426 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2993 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12432 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2999 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12438 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 3000 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12444 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 3004 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12450 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 3005 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12456 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 3006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12462 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 3007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12468 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 3008 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 12474 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 3009 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 12480 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 3011 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 12487 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 3016 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 3021 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 3022 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12511 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 3025 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 12517 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 3026 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12523 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 3032 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 12529 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 3034 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,&(yyvsp[0]),0);}
#line 12535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 3036 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 12541 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 3037 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,&(yyvsp[0]),0);}
#line 12547 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 3041 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12553 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 3042 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 3043 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12565 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 3046 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12571 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 3048 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12577 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 3051 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 12583 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 3052 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 12589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 3053 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 12595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 3054 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12601 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 3058 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 12608 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 3061 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 12616 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 3068 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 3069 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 3072 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 12636 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 3075 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12642 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 3076 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12648 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 3077 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12654 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 3079 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12660 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 3080 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12666 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 3082 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1]));}
#line 12672 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 3083 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12678 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 3084 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12684 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 3085 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12690 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 3086 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12696 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 3087 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12702 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 3088 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12708 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 3093 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12714 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 3094 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12720 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 3099 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12726 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 3100 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12732 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3105 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 12738 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3107 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,&(yyvsp[0]),0);}
#line 12744 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3109 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 12750 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3110 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,&(yyvsp[0]),0);}
#line 12756 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3114 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,&(yyvsp[0]),0);}
#line 12762 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3115 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,&(yyvsp[0]),0);}
#line 12768 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3120 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12774 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3121 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12780 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3126 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0); }
#line 12786 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3129 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0); }
#line 12792 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3134 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12798 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3135 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12804 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3138 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12810 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3139 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 12817 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3146 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 12823 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3148 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 12829 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3151 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 12835 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3153 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12841 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3156 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12847 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3159 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12853 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3160 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12859 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3164 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12865 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3165 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12871 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3169 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12877 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3170 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12883 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3171 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3175 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3180 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12901 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3185 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12907 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3186 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12913 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3190 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3195 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12925 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3200 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12931 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3201 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12937 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3206 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12943 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3207 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12949 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3209 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12955 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3214 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12961 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3216 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12967 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3222 "hphp.y" /* yacc.c:1646  */
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
#line 3233 "hphp.y" /* yacc.c:1646  */
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
#line 3248 "hphp.y" /* yacc.c:1646  */
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
#line 3260 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13023 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3272 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13029 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3273 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13035 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3274 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13041 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3275 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13047 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3276 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13053 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3277 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13059 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3279 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3296 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13079 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3298 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3300 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3301 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3305 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3309 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3310 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3311 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13121 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3312 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3320 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3329 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13147 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3331 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13153 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3335 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13159 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3340 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13165 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3341 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13171 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3342 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13177 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3343 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13183 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3344 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13189 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3345 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13195 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3346 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13201 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3348 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13207 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3349 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13213 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3352 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13219 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3354 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13225 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3358 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13231 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3362 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13237 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3363 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13243 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3369 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13249 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3373 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3377 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3384 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 13267 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3393 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 13273 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3397 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 13279 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3401 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13285 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3410 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13291 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3411 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13297 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3412 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13303 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3416 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13309 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3417 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 13315 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3418 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 13321 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3420 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]) = 1; _p->onIndirectRef((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 13327 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3425 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13333 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3426 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3437 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3438 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13351 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3439 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13357 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3442 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13371 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3453 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13377 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13383 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3458 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13389 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13395 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3462 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13409 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3471 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3476 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),&(yyvsp[0]),1);}
#line 13421 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3477 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 13427 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3478 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,&(yyvsp[0]),1);}
#line 13433 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3479 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,&(yyvsp[0]),0);}
#line 13439 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3482 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 13445 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3484 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),&(yyvsp[0]),1);}
#line 13451 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3485 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,&(yyvsp[0]),1);}
#line 13457 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3486 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,&(yyvsp[0]),0);}
#line 13463 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3490 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),  0,  0,0);}
#line 13469 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3492 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,  0,0);}
#line 13475 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3497 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13481 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3498 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13487 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3502 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 13493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3503 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,&(yyvsp[0]),0);}
#line 13499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3504 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 13505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3505 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,&(yyvsp[0]),0);}
#line 13511 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3510 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13517 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3511 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13523 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3516 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 13529 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3518 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,&(yyvsp[0]),0);}
#line 13535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3520 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 13541 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3521 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,&(yyvsp[0]),0);}
#line 13547 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3525 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 13553 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3527 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 13559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3528 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 13565 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3530 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 13572 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1030:
#line 3535 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13578 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1031:
#line 3537 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13584 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1032:
#line 3539 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 13598 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1033:
#line 3549 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 13604 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1034:
#line 3551 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 13610 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1035:
#line 3552 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13616 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1036:
#line 3555 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 13622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3556 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 13628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1038:
#line 3557 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 13634 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1039:
#line 3561 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 13640 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3562 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 13646 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3563 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13652 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3564 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13658 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3565 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13664 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3566 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13670 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3567 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 13676 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3568 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 13682 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3569 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 13688 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3570 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 13694 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3571 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 13700 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3575 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 13706 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3576 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 13712 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3581 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13718 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3583 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13724 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3597 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13732 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3602 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 13740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3606 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13748 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3611 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 13756 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3617 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13762 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3618 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13768 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3622 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13774 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3623 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13780 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3629 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13786 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3633 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 13792 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3639 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13798 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3643 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 13805 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3650 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13811 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3651 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13817 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3655 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 13825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3658 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 13832 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3664 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13838 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3668 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13846 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3671 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13854 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3674 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-3]); }
#line 13861 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3676 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1077:
#line 3678 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1078:
#line 3680 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13881 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1079:
#line 3685 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]); }
#line 13887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1080:
#line 3686 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13893 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1081:
#line 3687 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1082:
#line 3688 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1089:
#line 3709 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1090:
#line 3710 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1093:
#line 3719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1096:
#line 3730 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1097:
#line 3732 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1098:
#line 3736 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1099:
#line 3739 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1100:
#line 3743 "hphp.y" /* yacc.c:1646  */
    {}
#line 13953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1101:
#line 3744 "hphp.y" /* yacc.c:1646  */
    {}
#line 13959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1102:
#line 3745 "hphp.y" /* yacc.c:1646  */
    {}
#line 13965 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1103:
#line 3751 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13972 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1104:
#line 3756 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13982 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1105:
#line 3765 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13988 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1106:
#line 3771 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13997 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1107:
#line 3779 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 14003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1108:
#line 3780 "hphp.y" /* yacc.c:1646  */
    { }
#line 14009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1109:
#line 3786 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 14015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1110:
#line 3788 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 14021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1111:
#line 3789 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 14031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1112:
#line 3794 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 14038 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1113:
#line 3800 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("HH\\darray"); }
#line 14045 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1114:
#line 3805 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14051 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1115:
#line 3810 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 14059 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1116:
#line 3814 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14065 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1117:
#line 3819 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 14071 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1118:
#line 3821 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 14077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1119:
#line 3827 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 14084 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1120:
#line 3829 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 14092 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1121:
#line 3832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14098 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1122:
#line 3833 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14106 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1123:
#line 3836 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14114 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1124:
#line 3839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14120 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1125:
#line 3842 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 14128 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1126:
#line 3845 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1127:
#line 3847 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 14144 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1128:
#line 3853 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 14153 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1129:
#line 3859 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("HH\\varray");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 14163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1130:
#line 3867 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1131:
#line 3868 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 14175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;


#line 14179 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 3871 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}
