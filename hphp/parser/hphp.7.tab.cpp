// @generated
/* A Bison parser, made by GNU Bison 3.1.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.1"

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
    // return children;
    Token arr;
    if (children.num() == 2) {
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
typedef unsigned short yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short yytype_int16;
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
#  define YYSIZE_T unsigned
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

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
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
#define YYLAST   19899

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  209
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  311
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1128
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  2096

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   434

#define YYTRANSLATE(YYX)                                                \
  ((unsigned) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

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
    1428,  1426,  1438,  1437,  1449,  1447,  1460,  1461,  1465,  1468,
    1471,  1472,  1473,  1476,  1477,  1480,  1482,  1485,  1486,  1489,
    1490,  1493,  1494,  1498,  1499,  1504,  1505,  1508,  1509,  1510,
    1514,  1515,  1519,  1520,  1524,  1525,  1529,  1530,  1535,  1536,
    1542,  1543,  1544,  1545,  1548,  1551,  1553,  1556,  1557,  1561,
    1563,  1566,  1569,  1572,  1573,  1576,  1577,  1581,  1587,  1593,
    1600,  1602,  1607,  1612,  1618,  1622,  1627,  1632,  1637,  1643,
    1649,  1655,  1661,  1667,  1674,  1684,  1689,  1694,  1700,  1702,
    1706,  1710,  1715,  1719,  1723,  1727,  1731,  1736,  1741,  1746,
    1751,  1756,  1762,  1771,  1772,  1773,  1777,  1779,  1782,  1784,
    1786,  1788,  1790,  1793,  1796,  1799,  1805,  1806,  1809,  1810,
    1811,  1815,  1816,  1818,  1819,  1823,  1825,  1828,  1832,  1838,
    1840,  1843,  1846,  1850,  1854,  1859,  1861,  1864,  1867,  1865,
    1882,  1879,  1894,  1896,  1898,  1900,  1902,  1904,  1906,  1910,
    1911,  1912,  1915,  1921,  1925,  1931,  1934,  1939,  1941,  1946,
    1951,  1955,  1956,  1960,  1961,  1963,  1965,  1971,  1972,  1974,
    1978,  1979,  1984,  1988,  1989,  1993,  1994,  1998,  2000,  2006,
    2011,  2012,  2014,  2018,  2019,  2020,  2021,  2025,  2026,  2027,
    2028,  2029,  2030,  2032,  2037,  2040,  2041,  2045,  2046,  2050,
    2051,  2054,  2055,  2058,  2059,  2062,  2063,  2067,  2068,  2069,
    2070,  2071,  2072,  2073,  2077,  2078,  2081,  2082,  2083,  2086,
    2088,  2090,  2091,  2094,  2096,  2099,  2105,  2107,  2111,  2115,
    2119,  2124,  2128,  2129,  2131,  2132,  2133,  2136,  2137,  2141,
    2142,  2146,  2147,  2148,  2149,  2153,  2157,  2162,  2166,  2170,
    2174,  2178,  2183,  2187,  2188,  2189,  2190,  2191,  2195,  2199,
    2201,  2202,  2203,  2206,  2207,  2208,  2209,  2210,  2211,  2212,
    2213,  2214,  2215,  2216,  2217,  2218,  2219,  2220,  2221,  2222,
    2223,  2224,  2225,  2226,  2227,  2228,  2229,  2230,  2231,  2232,
    2233,  2234,  2235,  2236,  2237,  2238,  2239,  2240,  2241,  2242,
    2243,  2244,  2245,  2246,  2247,  2248,  2249,  2251,  2252,  2254,
    2255,  2257,  2258,  2259,  2260,  2261,  2262,  2263,  2264,  2265,
    2266,  2267,  2268,  2269,  2270,  2271,  2272,  2273,  2274,  2275,
    2276,  2277,  2278,  2279,  2280,  2281,  2282,  2286,  2290,  2295,
    2294,  2310,  2308,  2327,  2326,  2347,  2346,  2366,  2365,  2384,
    2384,  2401,  2401,  2420,  2421,  2422,  2427,  2429,  2433,  2437,
    2443,  2447,  2453,  2455,  2459,  2461,  2465,  2469,  2470,  2471,
    2475,  2477,  2481,  2483,  2487,  2489,  2493,  2496,  2501,  2503,
    2507,  2510,  2515,  2519,  2523,  2527,  2531,  2535,  2539,  2543,
    2547,  2551,  2555,  2559,  2563,  2567,  2571,  2575,  2579,  2583,
    2587,  2589,  2593,  2595,  2599,  2601,  2605,  2612,  2619,  2621,
    2626,  2627,  2628,  2629,  2630,  2631,  2632,  2633,  2634,  2635,
    2637,  2638,  2642,  2643,  2644,  2645,  2649,  2655,  2668,  2685,
    2686,  2689,  2690,  2692,  2697,  2698,  2701,  2705,  2708,  2711,
    2718,  2719,  2723,  2724,  2726,  2731,  2732,  2733,  2734,  2735,
    2736,  2737,  2738,  2739,  2740,  2741,  2742,  2743,  2744,  2745,
    2746,  2747,  2748,  2749,  2750,  2751,  2752,  2753,  2754,  2755,
    2756,  2757,  2758,  2759,  2760,  2761,  2762,  2763,  2764,  2765,
    2766,  2767,  2768,  2769,  2770,  2771,  2772,  2773,  2774,  2775,
    2776,  2777,  2778,  2779,  2780,  2781,  2782,  2783,  2784,  2785,
    2786,  2787,  2788,  2789,  2790,  2791,  2792,  2793,  2794,  2795,
    2796,  2797,  2798,  2799,  2800,  2801,  2802,  2803,  2804,  2805,
    2806,  2807,  2808,  2809,  2810,  2811,  2812,  2813,  2817,  2822,
    2823,  2827,  2828,  2829,  2830,  2832,  2836,  2837,  2848,  2849,
    2851,  2853,  2865,  2866,  2867,  2871,  2872,  2873,  2877,  2878,
    2879,  2882,  2884,  2888,  2889,  2890,  2891,  2893,  2894,  2895,
    2896,  2897,  2898,  2899,  2900,  2901,  2902,  2905,  2910,  2911,
    2912,  2914,  2915,  2917,  2918,  2919,  2920,  2921,  2922,  2923,
    2924,  2925,  2926,  2928,  2930,  2932,  2934,  2936,  2937,  2938,
    2939,  2940,  2941,  2942,  2943,  2944,  2945,  2946,  2947,  2948,
    2949,  2950,  2951,  2952,  2954,  2956,  2958,  2960,  2961,  2964,
    2965,  2969,  2973,  2975,  2979,  2980,  2984,  2990,  2993,  2997,
    2998,  2999,  3000,  3001,  3002,  3003,  3008,  3010,  3014,  3015,
    3018,  3019,  3023,  3026,  3028,  3030,  3034,  3035,  3036,  3037,
    3040,  3044,  3045,  3046,  3047,  3051,  3053,  3060,  3061,  3062,
    3063,  3068,  3069,  3070,  3071,  3073,  3074,  3076,  3077,  3078,
    3079,  3080,  3081,  3085,  3087,  3091,  3093,  3096,  3099,  3101,
    3103,  3106,  3108,  3112,  3114,  3117,  3120,  3126,  3128,  3131,
    3132,  3137,  3140,  3144,  3144,  3149,  3152,  3153,  3157,  3158,
    3162,  3163,  3164,  3168,  3173,  3178,  3179,  3183,  3188,  3193,
    3194,  3198,  3200,  3201,  3206,  3208,  3213,  3224,  3238,  3250,
    3265,  3266,  3267,  3268,  3269,  3270,  3271,  3281,  3290,  3292,
    3294,  3298,  3302,  3303,  3304,  3305,  3306,  3322,  3323,  3326,
    3333,  3334,  3335,  3336,  3337,  3338,  3339,  3341,  3342,  3344,
    3346,  3351,  3355,  3356,  3360,  3363,  3367,  3374,  3378,  3387,
    3394,  3402,  3404,  3405,  3409,  3410,  3411,  3413,  3418,  3419,
    3430,  3431,  3432,  3433,  3444,  3447,  3450,  3451,  3452,  3453,
    3464,  3469,  3470,  3471,  3472,  3474,  3476,  3478,  3479,  3483,
    3485,  3489,  3491,  3494,  3496,  3497,  3498,  3502,  3504,  3507,
    3510,  3512,  3514,  3518,  3519,  3521,  3522,  3528,  3529,  3531,
    3541,  3543,  3545,  3548,  3549,  3550,  3554,  3555,  3556,  3557,
    3558,  3559,  3560,  3561,  3562,  3563,  3564,  3568,  3569,  3573,
    3575,  3583,  3585,  3589,  3593,  3598,  3602,  3610,  3611,  3615,
    3616,  3622,  3623,  3632,  3633,  3641,  3644,  3648,  3651,  3656,
    3661,  3664,  3667,  3669,  3671,  3673,  3677,  3679,  3680,  3681,
    3684,  3686,  3692,  3693,  3697,  3698,  3702,  3703,  3707,  3708,
    3711,  3716,  3717,  3721,  3724,  3726,  3730,  3736,  3737,  3738,
    3742,  3746,  3754,  3759,  3771,  3773,  3777,  3780,  3782,  3787,
    3792,  3798,  3801,  3806,  3811,  3813,  3820,  3822,  3825,  3826,
    3829,  3832,  3833,  3838,  3840,  3844,  3850,  3860,  3861
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
  "trait_declaration_statement", "$@23", "$@24", "class_decl_name",
  "interface_decl_name", "trait_decl_name", "class_entry_type",
  "extends_from", "implements_list", "interface_extends_list",
  "interface_list", "trait_list", "foreach_optional_arg",
  "foreach_variable", "for_statement", "foreach_statement",
  "while_statement", "declare_statement", "declare_list",
  "switch_case_list", "case_list", "case_separator", "elseif_list",
  "new_elseif_list", "else_single", "new_else_single",
  "method_parameter_list", "non_empty_method_parameter_list",
  "parameter_list", "non_empty_parameter_list", "inout_variable",
  "function_call_parameter_list", "non_empty_fcall_parameter_list",
  "global_var_list", "global_var", "static_var_list",
  "enum_statement_list", "enum_statement", "enum_constant_declaration",
  "class_statement_list", "class_statement", "$@25", "$@26", "trait_rules",
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
  "closure_expression", "$@27", "$@28", "lambda_expression", "$@29",
  "$@30", "$@31", "$@32", "$@33", "lambda_body", "shape_keyname",
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
  "non_empty_user_attribute_list", "user_attribute_list", "$@34",
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

#define YYPACT_NINF -1828

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1828)))

#define YYTABLE_NINF -1129

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1129)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1828,   227, -1828, -1828,  5383, 14895, 14895,     7, 14895, 14895,
   14895, 14895, 12496, 14895, -1828, 14895, 14895, 14895, 14895, 18222,
   18222, 14895, 14895, 14895, 14895, 14895, 14895, 14895, 14895, 12675,
   18960, 14895,    22,    51, -1828, -1828, -1828,   190, -1828,   307,
   -1828, -1828, -1828,   312, 14895, -1828,    51,   231,   259,   262,
   -1828,    51, 12854, 15627, 13033, -1828, 15913, 11359,   236, 14895,
   19152,    65,    94,    81,   309, -1828, -1828, -1828,   370,   411,
     431,   442, -1828, 15627,   493,   501,   636,   652,   657,   660,
     700, -1828, -1828, -1828, -1828, -1828, 14895,   594,  1704, -1828,
   -1828, 15627, -1828, -1828, -1828, -1828, 15627, -1828, 15627, -1828,
     601,   577,   579, 15627, 15627, -1828,    93, -1828, -1828, 13239,
   -1828, -1828,   422,   606,   628,   628, -1828,   748,   618,   588,
     587, -1828,   107, -1828,   593,   702,   778, -1828, -1828, -1828,
   -1828,  4618,   611, -1828,   163, -1828,   617,   624,   635,   647,
     649,   654,   656,   663, 17133, -1828, -1828, -1828, -1828, -1828,
     125,   753,   791,   798,   802,   815,   817, -1828,   819,   823,
   -1828,   217,   696, -1828,   743,    40, -1828,  1993,   173, -1828,
   -1828,  4123,   163,   163,   718,    90, -1828,   233,   105,   721,
     377, -1828, -1828,   842, -1828,   769, -1828, -1828,   779, -1828,
   14895, -1828,   778,   611, 19440,   139,  4871, 19440, 14895, 19440,
   19440, 16462, 16462,   746, 18395, 19440,   905, 15627,   886,   886,
     168,   886, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828,    95, 14895,   774, -1828, -1828,   796,   765,   486,   766,
     486,   886,   886,   886,   886,   886,   886,   886,   886, 18222,
   18443,    88,   769, 14895,   774,   804, -1828,   806,   770, -1828,
     176, -1828, -1828, -1828,   486,   163, -1828, 13418, -1828, -1828,
   14895,  9926,   963,   115, 19440, 10956, -1828, 14895, 14895, 15627,
   -1828, -1828, 17181,   771, -1828, 17229, -1828, -1828, -1828, -1828,
   -1828, -1828, -1828, -1828, -1828, -1828, 17734, -1828, 17734, -1828,
   -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,   116,   118,
     779, -1828, -1828, -1828, -1828,   776, -1828, 17638,   119, -1828,
   -1828,   811,   964, -1828,   824, 16654, 14895, -1828,   780,   784,
   17278, -1828,    56, 17326,  5232,  5232,  5232, 15627,  5232,   785,
     979,   788, -1828,    76, -1828, 18041,   122, -1828,   976,   126,
     863, -1828,   864, -1828, 18222, 14895, 14895,   797,   818, -1828,
   -1828, 12675, 12675, 14895, 14895, 14895, 14895, 14895,   128,   506,
     648, -1828, 15074, 18222,   622, -1828, 15627, -1828,   252,   618,
   -1828, -1828, -1828, -1828, 19062, 14895,   984,   897, -1828, -1828,
   -1828,   102, 14895,   810,   812, 19440,   829,  2587,   834,  6012,
   14895, -1828,    87,   805,   661,    87,   538,   443, -1828, 15627,
   17734,   807, 11538, 15913, -1828, 13624,   813,   813,   813,   813,
   -1828, -1828,  3203, -1828, -1828, -1828, -1828, -1828,   778, -1828,
   14895, 14895, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828, 14895, 14895, 14895, 14895, 13803, 14895, 14895, 14895, 14895,
   14895, 14895, 14895, 14895, 14895, 14895, 14895, 14895, 14895, 14895,
   14895, 14895, 14895, 14895, 14895, 14895, 14895, 14895, 14895, 18960,
   14895, -1828, 11914, 14895, 14895, 14895, 15247, 15627, 15627, 15627,
   15627, 15627,  4618,   906,   800, 11162, 14895, 14895, 14895, 14895,
   14895, 14895, 14895, 14895, 14895, 14895, 14895, 14895, -1828, -1828,
   -1828, -1828, 19104, -1828, -1828, 11538, 11538, 14895, 14895,   836,
     778, 14895, 13982,  2912, -1828, 14895, -1828,   837,  1029,   882,
     843,   844, 15401,   486, 14161, 14340, -1828,   845,   854,  3213,
   -1828,   195, 11538, -1828, 19138, -1828, -1828, 17374, -1828, -1828,
   12111, -1828, 14895, -1828,   947, 10132,  1046,   855, 19318,  1043,
     152,    78, -1828, -1828, -1828,   876, -1828, -1828, -1828, 17734,
   -1828,  2711,   862,  1051, 17965, 15627, -1828, -1828, -1828, -1828,
   -1828, -1828, -1828, -1828, -1828,   865, -1828, -1828,   867,   870,
     879,   888,   883,   890,   100,   894,   900,  3364, 15295, -1828,
   -1828, 15627, 15627, 14895,   486,    65, -1828, 17965,   983, -1828,
   -1828, -1828,   486,   156,   158,   902,   903,  3311,   260,   908,
     909,   545,   972,   159,   160, 18504,   910,  1098,  1104,   911,
     913,   916,   917, -1828,  4376, 15627, -1828, -1828,  1053,  3982,
      55, -1828, -1828, -1828,   618, -1828, -1828, -1828,  1093,   993,
     950,    82,   967, 14895,   995,  1125,   938, -1828,   978, -1828,
     191, -1828,   943, 17734, 17734,  1130,   963,   102, -1828,   949,
    1139, -1828,  3733,   522, -1828,   539,   359, -1828, -1828, -1828,
   -1828, -1828, -1828, -1828,   708,  4030, -1828, -1828, -1828, -1828,
    1140,   968, -1828, 18222,   689, 14895,   951,  1147, 19440,  1144,
     162,  1150,   960,   971,   973, 19440,   975,  3633,  6218, -1828,
   -1828, -1828, -1828, -1828, -1828,  1034,  3218, 19440,   966,  4156,
   19579, 19670, 16462, 15914, 14895, 19392, 16283,  4758, 17411, 19737,
   15735, 19768, 19799, 19799, 19799, 19799,  4660,  4660,  4660,  4660,
    4660,   970,   970,   861,   861,   861,   168,   168,   168, -1828,
     886, -1828, -1828, 16462,   974,   981, 18552,   969,  1168,    27,
   14895,   250,   774,   339, -1828, -1828, -1828,  1166,   897, -1828,
     778, 18146, -1828, -1828, -1828, 16462, 16462, 16462, 16462, 16462,
   16462, 16462, 16462, 16462, 16462, 16462, 16462, 16462, -1828, 14895,
     251, -1828,   202, -1828,   774,   488,   982,   987,   986,  4275,
     999, -1828, 19440,  4583, -1828, 15627, -1828,   486,    60, 18222,
   19440, 18222, 18613,   465,   486,   206, -1828,   191,  1030,   994,
   14895, -1828,   211, -1828, -1828, -1828,  6424,   642, -1828, -1828,
   19440, 19440,    51, -1828, -1828, -1828, 14895,  1099, 17889, 17965,
   15627, 10338,  1002,  1004, -1828,  1197,  4077,  1069, -1828,  1047,
   -1828,  1201,  1011, 17542, 17734, 17965, 17965, 17965, 17965, 17965,
    1014,  1145,  1146,  1148,  1149,  1152,  1021,  1026, 17965,   471,
   -1828, -1828, -1828, -1828, -1828, -1828, -1828,    32, -1828, 19534,
   -1828, -1828,    39, -1828,  6630,  4281,  1024, 15295, -1828, 15295,
   -1828, 15295, -1828, 15627, 15627, 15295, -1828, 15295, 15295, 15627,
   -1828,  1218,  1025, -1828,   117, -1828, -1828,  4361, -1828, 19534,
    1215, 18222,  1032, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828, -1828,  1050,  1221, 15627,  4281,  1036, -1828, -1828, 14895,
   -1828, 14895, -1828, 14895, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828, -1828,  1027, -1828, 14895, -1828, -1828,  5594, -1828, 17734,
    4281,  1038, -1828, -1828, -1828, -1828,  1060,  1227,  1044, 14895,
   19062, -1828, -1828, 15247, -1828,  1045, -1828, 17734, -1828,  1049,
    6836,  1217,   167, -1828, 17734, -1828,   145, 19104, 19104, -1828,
   17734, -1828, -1828,   486, -1828, -1828,  1177, 19440, -1828, 11717,
   -1828, 17965, 13624,   813, 13624, -1828,   813,   813, -1828, 12317,
   -1828, -1828,  7042, -1828,   106,  1052,  4281,   993, -1828, -1828,
   -1828, -1828, 16283, 14895, -1828, -1828, 14895, -1828, 14895, -1828,
    4575,  1055, 11538,   972,  1220,   993, 17734,  1241,  1034, 15627,
   18960,   486,  4813,  1057,   378,  1061, -1828, -1828,  1478,  1478,
    4583, -1828, -1828, -1828,  1207,  1062,  1193,  1194,  1198,  1200,
    1202,    98,  1068,  1072,   563, -1828, -1828, -1828, -1828, -1828,
   -1828,  1112, -1828, -1828, -1828, -1828,  1269,  1080,   837,   486,
     486, 14519, 19138, -1828, 19138, -1828,  4932,   768,    51, 10956,
   -1828,  7248,  1082,  7454,  1088, 17889, 18222,  1154,  1091,   486,
   19534,  1283, -1828, -1828, -1828, -1828,   634, -1828,    74, 17734,
    1114,  1161,  1138, 17734, 15627,  3643, -1828, -1828, 17734,  1294,
    1105,  1132,  1133,  1140,   907,   907,  1242,  1242,  4868,  1106,
    1305, 17965, 17965, 17965, 17965, 17965, 17965, 19062, 17965,  4458,
   16808, 17965, 17965, 17965, 17965, 17813, 17965, 17965, 17965, 17965,
   17965, 17965, 17965, 17965, 17965, 17965, 17965, 17965, 17965, 17965,
   17965, 17965, 17965, 17965, 17965, 17965, 17965, 17965, 17965, 15627,
   -1828, -1828,  1232, -1828, -1828,  1113,  1115,  1116, -1828,  1117,
   -1828, -1828,   459,  3364, -1828,  1122, -1828, 17965,   486, -1828,
   -1828,   180, -1828,   756,  1308, -1828, -1828, 19440, 18661, -1828,
    2630, -1828,  5806,   897,  1308, -1828,   417, 14895,    43, -1828,
   19440,  1186,  1126, -1828,  1127,  1217, -1828, -1828, -1828, 14716,
   17734,   963, 17686,  1243,    75,  1313,  1247,   212, -1828,   774,
     221, -1828,   774, -1828, 14895, 18222,   689, 14895, 19440, 19534,
   -1828, -1828, -1828,  5058, -1828, -1828, -1828, -1828, -1828, -1828,
    1131,   106, -1828,  1134,   106,  1135, 16283, 19440, 18722,  1141,
   11538,  1151,  1137, 17734,  1142,  1155, 17734,   993, -1828,   770,
     508, 11538, -1828, -1828, -1828, -1828, -1828, -1828,  1203,  1128,
    1336,  1254,  4583,  4583,  4583,  4583,  4583,  4583,  1189, -1828,
   19062,  4583,    92,  4583, -1828, -1828, -1828, 18222, 19440, -1828,
      51,  1321,  1281, 10956, -1828, -1828, -1828,  1158, 14895,  1154,
     486, 17889,  1163, 17965,  7660,   655,  1160, 14895,   110,   475,
   -1828,  1183, -1828, 17734, 15627, -1828,  1230, -1828, -1828, -1828,
   17558, -1828,  1337, -1828,  1170, 17965, -1828, 17965, -1828,  1171,
    1169,  1363, 18770,  1172, 19534,  1368,  1174,  1176,  1179,  1248,
    1372,  1188,  1190, -1828, -1828, -1828, 18830,  1192,  1383, 19626,
    2584, 16093, 17965, 19488, 15248, 19704, 17897, 19044, 18229, 19830,
   19830, 19830, 19830,  2719,  2719,  2719,  2719,  2719,   884,   884,
     907,   907,   907,  1242,  1242,  1242,  1242, -1828,  1196, -1828,
    1199,  1204,  1205,  1206, -1828, -1828, 19534, 15627, 17734, 17734,
   -1828,   756,  4281,  1847, 14895,  1195, -1828,  1191,  1866, -1828,
     239, 14895, -1828, -1828,  5059, -1828, 14895, -1828, 14895, -1828,
     963, 13624,  1209,   326,   813,   326,   382, -1828, -1828, 17734,
     146, -1828,  1385,  1328, 14895, -1828,  1213,  1214,  1210,   486,
    1177, 19440,  1217,  1219, -1828,  1222,   106, 14895, 11538,  1225,
   -1828, -1828,   897, -1828, -1828,  1226,  1228,  1216, -1828,  1229,
    4583, -1828,  4583, -1828, -1828,  1231,  1224,  1411,  1296,  1233,
   -1828,  1423,  1234,  1237,  1238, -1828,  1298,  1244,  1434,  1246,
   -1828, -1828,   486,  1415, -1828,  1249, -1828, -1828,  1251,  1256,
   -1828, -1828, 19534,  1257,  1258, -1828,  5309, -1828, -1828, -1828,
   -1828, -1828, -1828,  1319, 17734, 17734,  1132,  1284, 17734, -1828,
   19534, 18876, -1828, -1828, 17965, -1828, 17965, -1828, 17965, -1828,
   -1828, -1828, -1828, 17965, 19062, -1828, -1828, -1828, 17965, -1828,
   17965, -1828, 16650, 17965,  1259,  7866, -1828, -1828, -1828, -1828,
     756, -1828, -1828, -1828, -1828,   736, 16092,  4281,  1349,  1351,
    1353,  1354, -1828,  2612,  1297,  3078, -1828, -1828,  1386,   906,
   17365,  1357,   131,   132,  1271,   897,   800, 19440, -1828, -1828,
   -1828,  1306,  5129, -1828,  5237, 19440, -1828,  3689, -1828,  6218,
    1391,    97,  1462,  1394, 14895, -1828, 19440, 11538, 11538, -1828,
    1359,  1217,  2179,  1217,  1279, 19440,  1280, -1828,  2194,  1282,
    2289, -1828, -1828,   106, -1828, -1828,  1343, -1828, -1828,  4583,
   -1828,  4583, -1828,  4583, -1828, -1828, -1828, -1828,  4583, -1828,
   19062, -1828, -1828, -1828,  8072, -1828, -1828, -1828, 10544, -1828,
   -1828, -1828,  6424, 17734, -1828, -1828, -1828,  1287, 17965, 18936,
   19534, 19534, 19534,  1344, 19534, 18982, 16650, -1828, -1828,   756,
    4281,  4281, 15627, -1828,  1469, 16962,   103, -1828, 16092,   897,
    3408, -1828,  1307, -1828,   134,  1286,   135, -1828, 16461, -1828,
   -1828, -1828,   138, -1828, -1828,  2250, -1828,  1291, -1828,  1479,
     140,   778,  1386, 16282, 16282, -1828, 16282, -1828, -1828,  1482,
     906, 17461, 15555, -1828, -1828, -1828, -1828,  3088, -1828,  1483,
    1416, 14895, -1828, 19440,  1300,  1301,  1299,  1217,  1304, -1828,
    1359,  1217, -1828, -1828, -1828, -1828,  2355,  1303,  4583,  1366,
   -1828, -1828, -1828,  1367,  6424, 10750, 10544, -1828, -1828, -1828,
    6424, -1828, -1828, 19534, 17965, 17965, 17965,  8278,  1309,  1311,
   -1828, 17965, -1828,  4281, -1828, -1828, -1828, -1828, -1828, 17734,
    2741,  2612, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828, -1828, -1828, -1828, -1828, -1828,   184, -1828,  1297, -1828,
   -1828, -1828, -1828, -1828,   154,   629, -1828, 17965,  1424, -1828,
   16654,   142,  1492,  1499, -1828, 17734,   778,   143,  1386, -1828,
   -1828,  1314,  1500, 14895, -1828, 19440, -1828, -1828,   374,  1315,
   17734,   675,  1217,  1304, 15734, -1828,  1217, -1828,  4583,  4583,
   -1828, -1828, -1828, -1828,  8484, 19534, 19534, 19534, -1828, -1828,
   -1828, 19534, -1828,  2051,  1510,  1511,  1318, -1828, -1828, 17965,
   16461, 16461,  1455, -1828,  2250,  2250,   831, -1828, -1828, -1828,
   19534,  1514,  1342,  1331, -1828, 17965, 17965, -1828, 16654, -1828,
     147, -1828, 17965, 19440,  1449, -1828,  1524, -1828,  1525, -1828,
     592, -1828, -1828, -1828,  1333,   675, -1828,   675, -1828, -1828,
    8690,  1335,  1421, -1828,  1437,  1379, -1828, -1828,  1439, 17734,
    1360,  2741, -1828, -1828, 19534, -1828, -1828,  1371, -1828,  1515,
   -1828, -1828, -1828, -1828, 17965,   545, -1828, 19534, 19534,  1347,
   -1828, 19534, -1828,   516,  1352,  8896, 17734, -1828, 17734, -1828,
    9102, -1828, -1828, -1828,  1346, -1828,  1355,  1373, 15627,   800,
    1376, -1828, -1828, -1828, 19534,  1377,   114, -1828,  1472, -1828,
   -1828, -1828, -1828, -1828, -1828,  9308, -1828,  4281,  1024, -1828,
    1387, 15627,   672, -1828, -1828,  1362,  1557,   633,   114, -1828,
   -1828,  1484, -1828,  4281,  1369, -1828,  1217,   157, -1828, 17734,
   -1828, -1828, -1828, 17734, -1828,  1375,  1378,   148, -1828,  1304,
     713,  1485,   149,  1217,  1370, -1828,   690, 17734, 17734, -1828,
     353,  1559,  1494,  1304, -1828, -1828, -1828, -1828,  1496,   161,
    1566,  1502, 14895, -1828,   690,  9514,  9720, -1828,   368,  1569,
    1504, 14895, -1828, 19440, -1828, -1828, -1828,  1574,  1513, 14895,
   -1828, 19440, 14895, -1828, 19440, 19440
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   204,   471,     0,   913,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1010,
     996,     0,   777,     0,   783,   784,   785,    29,   850,   984,
     985,   171,   172,   786,     0,   152,     0,     0,     0,     0,
      30,     0,     0,     0,     0,   223,     0,     0,     0,     0,
       0,     0,   440,   441,   442,   439,   438,   437,     0,     0,
       0,     0,   250,     0,     0,     0,    37,    38,    40,    41,
      39,   790,   792,   793,   787,   788,     0,     0,     0,   794,
     789,     0,   760,    32,    33,    34,    36,    35,     0,   791,
       0,     0,     0,     0,     0,   795,   443,   581,    31,     0,
     170,   140,     0,   778,     0,     0,     4,   126,   128,   849,
       0,   759,     0,     6,     0,     0,   222,     7,     9,     8,
      10,     0,     0,   435,   967,   485,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   541,   483,   972,   973,   563,
     556,   557,   558,   559,   562,   560,   561,   466,   566,     0,
     465,   941,   761,   768,     0,   852,   555,   434,   944,   945,
     957,   484,     0,     0,     0,   487,   486,   942,   943,   940,
     980,   983,   545,   851,    11,   440,   441,   442,     0,    36,
       0,   126,   222,     0,  1042,   556,   484,  1043,     0,  1045,
    1046,   565,   479,     0,   472,   477,     0,     0,   527,   528,
     529,   530,    29,   984,   786,   763,    37,    38,    40,    41,
      39,     0,     0,  1066,   963,   761,     0,   762,   506,     0,
     508,   546,   547,   548,   549,   550,   551,   552,   554,     0,
    1004,     0,   773,     0,  1066,   772,   766,     0,   782,   762,
     991,   992,   998,   990,   774,     0,   464,     0,   776,   553,
       0,   205,     0,     0,   468,   205,   150,   470,     0,     0,
     156,   158,     0,     0,   160,     0,    75,    76,    82,    83,
      67,    68,    59,    80,    91,    92,     0,    62,     0,    66,
      74,    72,    94,    86,    85,    57,   108,    81,   101,   102,
      58,    97,    55,    98,    56,    99,    54,   103,    90,    95,
     100,    87,    88,    61,    89,    93,    53,    84,    69,   104,
      77,   106,    70,    60,    47,    48,    49,    50,    51,    52,
      71,   107,   105,   110,    64,    45,    46,    73,  1119,  1120,
      65,  1124,    44,    63,    96,     0,    79,     0,   126,   109,
    1057,  1118,     0,  1121,     0,     0,     0,   162,     0,     0,
       0,   213,     0,     0,     0,     0,     0,     0,     0,     0,
     861,     0,   114,   116,   348,     0,     0,   347,   353,     0,
       0,   251,     0,   254,     0,     0,     0,     0,  1063,   238,
     248,  1010,  1010,   601,   631,   631,   601,   631,     0,  1027,
       0,   797,     0,     0,     0,  1025,     0,    16,     0,   130,
     230,   242,   249,   661,   593,   631,     0,  1051,   573,   575,
     577,   917,   471,   485,     0,     0,   483,   484,   486,   205,
       0,   987,   779,     0,   780,     0,     0,     0,   202,     0,
       0,   132,   337,     0,    28,     0,     0,     0,     0,     0,
     203,   221,     0,   247,   234,   246,   440,   443,   222,   436,
     989,     0,   933,   192,   193,   194,   195,   196,   198,   199,
     201,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   996,
       0,   191,     0,   989,   989,  1012,     0,     0,     0,     0,
       0,     0,     0,     0,   433,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   505,   507,
     918,   919,     0,   932,   931,   337,   337,   989,     0,     0,
     222,     0,     0,     0,   164,     0,   915,   910,   859,     0,
     485,   483,     0,  1003,     0,  1009,   598,   485,   483,   484,
     132,     0,   337,   463,     0,   934,   775,     0,   140,   290,
       0,   580,     0,   167,     0,   205,   469,     0,     0,     0,
       0,     0,   159,   190,   161,  1119,  1120,  1116,  1117,     0,
    1123,  1109,     0,     0,     0,     0,    78,    43,    65,    42,
    1058,   197,   200,   163,   140,     0,   180,   189,     0,     0,
       0,     0,     0,     0,   117,     0,     0,     0,   860,   115,
      18,     0,   111,     0,   349,     0,   165,     0,     0,   166,
     252,   253,  1047,     0,     0,   485,   483,   484,   487,   486,
       0,  1099,   260,     0,     0,     0,     0,   859,   859,     0,
       0,     0,     0,   168,     0,     0,   796,  1026,   850,     0,
       0,  1024,   855,  1023,   129,     5,    13,    14,     0,   258,
       0,     0,   586,     0,     0,   859,     0,   770,     0,   769,
     764,   587,     0,     0,     0,     0,     0,   917,   136,     0,
     861,   916,  1128,   462,   474,   488,   950,   971,   147,   139,
     143,   144,   145,   146,   434,     0,   564,   853,   854,   127,
     859,     0,  1067,     0,     0,     0,     0,   861,   338,     0,
       0,     0,   485,   209,   210,   208,   483,   484,   205,   184,
     182,   183,   185,   569,   224,   256,     0,   988,     0,     0,
     511,   513,   512,   524,     0,     0,   544,   509,   510,   514,
     516,   515,   533,   534,   531,   532,   535,   536,   537,   538,
     539,   525,   526,   518,   519,   517,   520,   521,   523,   540,
     522,   476,   481,   489,     0,     0,  1016,     0,   859,  1050,
       0,  1049,  1066,   947,   240,   232,   244,     0,  1051,   236,
     222,     0,   475,   478,   480,   490,   493,   494,   495,   496,
     497,   498,   499,   500,   501,   502,   503,   504,   921,     0,
     920,   923,   946,   927,  1066,   924,     0,     0,     0,     0,
       0,  1044,   473,   908,   912,   858,   914,   461,   765,     0,
    1002,     0,  1008,     0,   765,   995,   994,   980,   983,     0,
       0,   920,   923,   993,   924,   482,   292,   294,   136,   584,
     583,   467,     0,   140,   274,   151,   470,     0,     0,     0,
       0,   205,   286,   286,   157,   859,     0,     0,  1108,     0,
    1105,   859,     0,  1079,     0,     0,     0,     0,     0,   857,
       0,    37,    38,    40,    41,    39,     0,     0,     0,   799,
     803,   804,   805,   808,   806,   807,   810,     0,   798,   134,
     848,   809,  1066,  1122,   205,     0,     0,     0,    21,     0,
      22,     0,    19,     0,   112,     0,    20,     0,     0,     0,
     123,   861,     0,   121,   116,   113,   118,     0,   346,   354,
     351,     0,     0,  1036,  1041,  1038,  1037,  1040,  1039,    12,
    1097,  1098,     0,   859,     0,     0,     0,   599,   597,     0,
     612,   858,   600,   858,   630,   615,   624,   627,   618,  1035,
    1034,  1033,     0,  1029,     0,  1030,  1032,   205,     5,     0,
       0,     0,   656,   657,   666,   665,     0,     0,   483,     0,
     858,   592,   596,     0,   621,     0,  1052,     0,   574,     0,
     205,  1086,   917,   318,  1128,  1127,     0,     0,     0,   986,
     858,  1069,  1065,   340,   334,   335,   339,   341,   758,   860,
     336,     0,     0,     0,     0,   461,     0,     0,   488,     0,
     951,   212,   205,   142,   917,     0,     0,   258,   571,   226,
     929,   930,   543,     0,   638,   639,     0,   636,   858,  1011,
       0,     0,   337,   260,     0,   258,     0,     0,   256,     0,
     996,   491,     0,     0,   948,   949,   981,   982,     0,     0,
     896,   866,   867,   868,   875,     0,    37,    38,    40,    41,
      39,     0,     0,     0,   881,   887,   888,   889,   892,   890,
     891,     0,   879,   877,   878,   902,   859,     0,   910,  1001,
    1007,     0,     0,   935,     0,   781,     0,   296,     0,   205,
     148,   205,     0,   205,     0,     0,     0,   266,   269,   267,
     278,     0,   140,   276,   177,   286,     0,   286,     0,   858,
       0,     0,     0,     0,     0,   858,  1107,  1110,  1075,   859,
       0,  1070,     0,   859,   831,   832,   829,   830,   865,     0,
     859,   857,   605,   633,   633,   605,   633,   595,   633,     0,
       0,  1018,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1113,   214,     0,   217,   181,     0,     0,     0,   119,     0,
     124,   125,   117,   860,   122,     0,   350,     0,  1048,   169,
    1064,  1099,  1090,  1094,   259,   261,   360,   603,     0,  1028,
       0,    17,   205,  1051,   257,   360,     0,     0,     0,   765,
     589,     0,   771,  1053,     0,  1086,   578,   135,   137,     0,
       0,     0,  1128,     0,     0,   323,   321,   923,   936,  1066,
     923,   937,  1066,  1068,   989,     0,     0,     0,   342,   133,
     207,   209,   210,   484,   188,   206,   186,   187,   211,   141,
       0,   917,   255,     0,   917,     0,   542,  1015,  1014,     0,
     337,     0,     0,     0,     0,     0,     0,   258,   228,   782,
     922,   337,   871,   872,   873,   874,   882,   883,   900,     0,
     859,     0,   896,   609,   635,   635,   609,   635,     0,   870,
     904,   635,     0,   858,   907,   909,   911,     0,  1005,   922,
       0,     0,     0,   205,   293,   585,   153,     0,   470,   266,
     268,     0,     0,     0,   205,     0,     0,     0,     0,     0,
     280,     0,  1114,     0,     0,  1100,     0,  1106,  1104,  1071,
     858,  1077,     0,  1078,     0,     0,   801,   858,   856,     0,
       0,   859,     0,     0,   845,   859,     0,     0,     0,     0,
     859,     0,     0,   811,   846,   847,  1022,     0,   859,   814,
     816,   815,     0,     0,   812,   813,   817,   819,   818,   835,
     836,   833,   834,   837,   838,   839,   840,   841,   826,   827,
     821,   822,   820,   823,   824,   825,   828,  1112,     0,   140,
       0,     0,     0,     0,   120,    23,   352,     0,     0,     0,
    1091,  1096,     0,   434,     0,     0,    15,     0,   434,   669,
       0,     0,   671,   664,     0,   667,     0,   663,     0,  1055,
       0,     0,     0,   967,   541,     0,   487,  1087,   582,  1128,
       0,   324,   325,     0,     0,   319,     0,     0,     0,   344,
     345,   343,  1086,     0,   360,     0,   917,     0,   337,     0,
     978,   360,  1051,   360,  1054,     0,     0,     0,   492,     0,
       0,   885,   858,   895,   876,     0,     0,   859,     0,     0,
     894,   859,     0,     0,     0,   869,     0,     0,   859,     0,
     880,   901,  1006,     0,   140,     0,   289,   275,     0,     0,
     265,   173,   279,     0,     0,   282,     0,   287,   288,   140,
     281,  1115,  1101,     0,     0,  1074,  1073,     0,     0,  1126,
     864,   863,   800,   613,   858,   604,     0,   616,   858,   632,
     625,   628,   619,     0,   858,   594,   802,   622,     0,   637,
     858,  1017,   843,     0,     0,   205,    24,    25,    26,    27,
    1093,  1088,  1089,  1092,   262,     0,     0,     0,   441,   439,
     438,   437,   432,     0,     0,     0,   239,   359,     0,     0,
     431,     0,     0,     0,     0,  1051,   434,   602,  1031,   356,
     243,   659,     0,   662,     0,   588,   576,   484,   138,   205,
       0,     0,   328,   317,     0,   320,   327,   337,   337,   333,
     568,  1086,   434,  1086,     0,  1013,     0,   977,   434,     0,
     434,  1056,   360,   917,   974,   899,   898,   884,   614,   858,
     608,     0,   617,   858,   634,   626,   629,   620,     0,   886,
     858,   903,   623,   140,   205,   149,   154,   175,   205,   277,
     283,   140,   285,     0,  1102,  1072,  1076,     0,     0,     0,
     607,   844,   591,     0,  1021,  1020,   842,   140,   218,  1095,
       0,     0,     0,  1059,     0,     0,     0,   263,     0,  1051,
       0,   397,   393,   399,   760,    36,     0,   387,     0,   392,
     396,   409,     0,   407,   412,     0,   411,     0,   410,   451,
       0,   222,     0,     0,     0,   365,     0,   366,   367,     0,
       0,   433,     0,   660,   658,   670,   668,     0,   329,   330,
       0,     0,   315,   326,     0,     0,     0,  1086,  1080,   235,
     568,  1086,   979,   241,   356,   245,   434,     0,     0,     0,
     611,   893,   906,     0,   291,   205,   205,   140,   272,   174,
     284,  1103,  1125,   862,     0,     0,     0,   205,     0,     0,
     460,     0,  1060,     0,   377,   381,   457,   458,   391,     0,
       0,     0,   372,   719,   720,   718,   721,   722,   739,   741,
     740,   710,   682,   680,   681,   700,   715,   716,   676,   687,
     688,   690,   689,   757,   709,   693,   691,   692,   694,   695,
     696,   697,   698,   699,   701,   702,   703,   704,   705,   706,
     708,   707,   677,   678,   679,   683,   684,   686,   756,   724,
     725,   729,   730,   731,   732,   733,   734,   717,   736,   726,
     727,   728,   711,   712,   713,   714,   737,   738,   742,   744,
     743,   745,   746,   723,   748,   747,   750,   752,   751,   685,
     755,   753,   754,   749,   735,   675,   404,   672,     0,   373,
     425,   426,   424,   417,     0,   418,   374,     0,     0,   361,
       0,     0,     0,     0,   456,     0,   222,     0,     0,   231,
     355,     0,     0,     0,   316,   332,   975,   976,     0,     0,
       0,     0,  1086,  1080,     0,   237,  1086,   897,     0,     0,
     140,   270,   155,   176,   205,   606,   590,  1019,   216,   375,
     376,   454,   264,     0,   859,   859,     0,   400,   388,     0,
       0,     0,   406,   408,     0,     0,   413,   420,   421,   419,
     452,   449,  1061,     0,   362,     0,     0,   459,     0,   363,
       0,   357,     0,   331,     0,   654,   861,   136,   861,  1082,
       0,   427,   136,   225,     0,     0,   233,     0,   610,   905,
     205,     0,   178,   378,   126,     0,   379,   380,     0,   858,
       0,   858,   402,   398,   403,   673,   674,     0,   389,   422,
     423,   415,   416,   414,     0,  1099,   368,   455,   453,     0,
     364,   358,   655,   860,     0,   205,   860,  1081,     0,  1085,
     205,   136,   227,   229,     0,   273,     0,   220,     0,   434,
       0,   394,   401,   405,   450,     0,   917,   370,     0,   652,
     567,   570,  1083,  1084,   428,   205,   271,     0,     0,   179,
     385,     0,   433,   395,  1062,     0,   861,   445,   917,   653,
     572,     0,   219,     0,     0,   384,  1086,   917,   300,  1128,
     448,   447,   446,  1128,   444,     0,     0,     0,   383,  1080,
     445,     0,     0,  1086,     0,   382,     0,  1128,  1128,   306,
       0,   305,   303,  1080,   140,   429,   136,   369,     0,     0,
     307,     0,     0,   301,     0,   205,   205,   311,     0,   310,
     299,     0,   302,   309,   371,   215,   430,   312,     0,     0,
     297,   308,     0,   298,   314,   313
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1828, -1828, -1828,  -573, -1828, -1828, -1828,   541,    19,   -42,
     222, -1828,  -245,  -519, -1828, -1828,   401,   282,  1871, -1828,
    3258, -1828,  -787, -1828,  -553, -1828,  -694,    25, -1828, -1828,
   -1828, -1828, -1828, -1828, -1828, -1828,  -923, -1828, -1828,  -903,
    -360, -1828, -1828, -1828,  -421, -1828, -1828,  -164,   362,    38,
   -1828, -1828, -1828, -1828, -1828, -1828,    41, -1828, -1828, -1828,
   -1828,    46, -1828, -1828,  1086,  1095,  1097,   -87,   556,  -927,
     565,   641,  -419,   293, -1003, -1828,  -123, -1828, -1828, -1828,
   -1828,  -770,   109, -1828, -1828, -1828, -1828,  -412, -1828,  -630,
   -1828,   371,  -467, -1828, -1828,  1001, -1828,  -100, -1828, -1828,
   -1108, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,  -134,
   -1828,   -41, -1828, -1828, -1828, -1828, -1828,  -220, -1828,    66,
   -1011, -1828, -1523,  -444, -1828,  -158,    73,  -130,  -418, -1827,
   -1537, -1828, -1828, -1828,    79,   -65,   -77,   -14,  -765,    -6,
   -1828, -1828,    37, -1828,    58,  -349, -1828,     2,    -5,   -64,
     -84,   -86, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828,  -615,  -879, -1828, -1828, -1828, -1828, -1828,    36,  1240,
   -1828,   494, -1828,   342, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
   -1828, -1828, -1828,    31,  -432, -1083, -1828, -1828, -1828, -1828,
   -1828,   425, -1828, -1828, -1828, -1828, -1828, -1828, -1828, -1828,
    -982,  -373,  2812,    28, -1828,   444,  -414, -1828, -1828,   364,
    3889,  1918, -1828,   706, -1828, -1828,   505,   436,  -652, -1828,
   -1828,   586,   363,  -440, -1828,   361, -1828, -1828, -1828, -1828,
   -1828,   568, -1828, -1828, -1828,   130,  -919,  -125,  -456,  -442,
   -1828,   -55,  -129, -1828, -1828,    48,    49,   504,   -62, -1828,
   -1828,   188,   -76, -1828,  -368,    44,  -387,   129,  -420, -1828,
   -1828,   559, -1828, -1828, -1828, -1828,   698,   474, -1828, -1828,
   -1828,  -348,  -706, -1828,  1223, -1291,  -271,   -67,  -174,   786,
   -1828, -1828, -1828, -1778, -1828,  -328, -1037, -1325,  -316,   111,
   -1828,   463,   542, -1828, -1828, -1828, -1828,   489, -1828,  1599,
    -654
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   116,   968,   665,   191,  1663,   782,
     369,   370,   371,   372,   921,   922,   923,   118,   119,   120,
     121,   122,   990,  1227,   429,  1022,   699,   700,   575,   265,
    1735,   581,  1638,  1736,  1997,   906,   124,   125,   720,   721,
     729,   362,   604,  1952,  1184,  1399,  2019,   452,   192,   701,
    1025,  1265,  1467,   128,   668,  1044,   702,   735,  1048,   642,
    1043,   703,   669,  1045,   454,   389,   411,   131,  1027,   971,
     946,  1204,  1666,  1322,  1107,  1892,  1739,   855,  1114,   580,
     864,  1116,  1509,   847,  1097,  1100,  1312,  2025,  2026,   689,
     690,  1006,   716,   717,   376,   377,   379,  1702,  1870,  1871,
    1413,  1567,  2006,  2028,  1903,  1956,  1957,  1958,  1676,  1677,
    1678,  1679,  1905,  1906,  1912,  1968,  1682,  1683,  1687,  1853,
    1854,  1855,  1943,  2067,  1568,  1569,   193,   133,  2043,  1571,
    1690,  1572,  1573,  1574,  1575,   134,   135,   648,   577,   136,
     137,   138,   139,   140,   141,   142,   143,   258,   144,   145,
     146,  1717,   147,  1024,  1264,   148,   686,   687,   688,   262,
     421,   571,   674,   675,  1360,   676,  1361,   149,   195,   646,
     647,  1350,  1351,  1476,  1477,   151,   890,  1075,   152,   891,
    1076,   153,   892,  1077,   154,   893,  1078,   155,   894,  1079,
     156,   895,  1080,   649,  1353,  1479,   157,   896,   158,   159,
    1936,   160,   670,  1704,   671,  1216,   977,  1427,  1423,  1846,
    1847,   161,   162,   163,   247,   164,   248,   259,   433,   563,
     165,  1354,  1355,   900,   901,   166,  1139,   826,   619,  1140,
    1083,  1286,  1084,  1480,  1481,  1289,  1290,  1086,  1487,  1488,
    1087,   824,   548,   206,   207,   704,   692,   532,  1237,  1238,
     813,   814,   462,   168,   250,   169,   170,   196,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   738,   254,
     255,   241,   777,   778,  1367,  1368,   404,   405,   962,   182,
     633,   183,   685,   184,   352,  1872,  1923,   390,   441,   710,
     711,  1129,  1130,  1881,  1938,  1939,  1231,  1410,   942,  1411,
     943,   944,   870,   871,   872,   353,   354,   903,   590,   995,
     996
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     194,   197,   459,   199,   200,   201,   202,   204,   205,   513,
     208,   209,   210,   211,   350,   846,   231,   232,   233,   234,
     235,   236,   237,   238,   240,   426,   256,   680,   539,   123,
     263,   412,   423,   428,  1023,   261,   416,   417,   993,   264,
     150,   677,   127,   533,   534,   129,   679,   272,   266,   275,
     130,   904,   360,   270,   363,   224,   224,   989,   246,   446,
     447,  1101,   448,   459,   455,  1010,   681,  1229,   816,   817,
     560,   988,   398,  1232,   253,   349,   811,   132,   251,   252,
     512,   264,  1047,   774,   775,   769,  1553,   730,   731,   732,
     812,  1104,   967,  1118,   358,   839,   724,   555,   920,   925,
    1263,  1221,  1319,   424,   425,  1945,   426,  1418,   842,  1250,
     862,  1255,  1753,   423,   428,   359,   443,   818,  1274,   609,
     611,   613,   843,   616,   572,   -78,   565,   -43,   -42,   564,
     -78,   625,   -43,   -42,   167,   628,   973,   572,   551,   502,
    1694,  1696,  1507,  -390,  1761,   550,   428,  1848,   374,  1858,
      14,  1858,  1858,   541,    14,  1861,  1858,  1753,  1442,   558,
    -648,   860,    14,  1914,  1867,   931,   557,   572,   555,   555,
     399,  1012,   605,  1490,   837,  -954,   418,   378,  1229,   621,
    1709,  1327,  1328,  1234,  1591,   425,   549,  2060,  1430,  1298,
    1915,   530,   531,   543,  1150,  -640,   530,   531,  1909,  2078,
    2044,  1179,  -762,   913,   380,    14,   198,   229,   229,  -640,
    -110,  1425,  1482,   381,  1484,    14,  1910,   425,  1489,  1132,
     621,   257,   440,  2044,  1364,   499,  -110,     3,  1235,  1592,
     940,   941,  2061,   460,  1151,  1911,   606,   500,   425,  -964,
     514,   431,   622,  -955,  2079,  1426,  -997,   402,   403,   974,
     260,  1443,   567,   576,   401,   567,  -763,  1299,   966,  -966,
    -651,   537,   264,   578,   975,  1664,   914,  -769,  1359,  1194,
     375,  2056,  -956,  1710,   655,   224, -1000,  1330,   213,    40,
     863,  -999,  -938,   439,   976,  2074,   569,  -952,   419,   536,
     574,  -939,   556,  1581,   736,   420,  1228,   150,  -322,   589,
    1103,   150,   636,  -953,  1754,  1755,  -322,  -954,   444,   635,
     639,  1508,   449,   600,  -304,  -579,   573,   -78,  1500,   -43,
     -42,  1236,  1593,   626,  -963,  2062,  1432,   629,  1259,   653,
    -648,  1930,  1695,  1697,  1553,  -390,  1762,  2080,  -963,  1849,
    1233,  1859,   373,  1924,  1929,  1325,  1602,  1329,  1980,  2055,
    1466,   202,   861,  1608,  1916,  1610,   932,  -860,   933,   947,
     948,   726,  1013,  -860,  -858,   461,   126,  -860,   722,   428,
     408,   634,  -961,   409,   349,  -955,   820,  1756,  -997,  -770,
     264,   425,  -649,  1085,   459,   734,   240,   240,   645,   264,
     264,   645,   264,   538,  1260,  1212,   460,   659,  1190,  1191,
     112,   350,  1862,  1863,  -956,  1864,   413,  1228, -1000,  -959,
     264,   224,  1934,  -999,  -938,  1600,  -962,   204,   439,  -952,
     224,  1486,  1992,  -939,  1993,   705,   650,   229,   652,  -951,
     267,   382,  -968,  -109,  -925,  -953,  2070,   718,   361,   224,
     725,   383,   412,   787,   788,   455,   682,   537,  -958,  -109,
    -925,  2087,  -648,   666,   667,   737,   739,  1935,   268,   536,
     937,   269,   349,   226,   226,   150,   740,   741,   742,   743,
     745,   746,   747,   748,   749,   750,   751,   752,   753,   754,
     755,   756,   757,   758,   759,   760,   761,   762,   763,   764,
     765,   766,   767,   768,   458,   770,   771,   773,   737,   737,
     776,  -771,   132,   723,  1726,   213,    40,  1417,   171,   792,
     795,   796,   797,   798,   799,   800,   801,   802,   803,   804,
     805,   806,   807,   228,   230,   781,   399,   246,  1589,  2071,
     718,   718,   737,   819,   661,   460,   773,   795,  1042,  -764,
     822,  -959,  1240,   253,  2088,   117,   513,   251,   252,   830,
     832,   691,   793,  1498,  2008,   540,  1241,   718,  -965,  1324,
     772,  -951,   913,   229,  1718,   850,  1720,   851,   997,   384,
     998,   349,   229,   794,   638,  1271,   654,  -970,  1440,   538,
    -958,   536,  1327,  1328,   728,  1419,   608,   610,   612,   978,
     615,   229,  -461,   680,   273,   940,   941,   348,  1420,  2009,
     854,   530,   531,   402,   403,   836,  1988,   677,  1041,  -650,
     385,   150,   679,   427,   388,  1989,  1438,   512,   927,  1421,
    1288,   399,   530,   531,   430,   439,  1049,   112,   849,   661,
     386,  1453,   681, -1066,  1455,   783,  1279,   410,  1240,   388,
    1053,   387,   530,   531,   388,   388,   373,   373,   373,   614,
     373,   708,  1241,  1254,   440,  1653,  1256,  1257,   530,   531,
    1917,   815,  1185,  1252,  1186,  1252,  1187,   461,   425,  1029,
    1189,  -928,   388, -1066,   920,   530,   531,   399,  1510,  1918,
    1879,   783,  1919,   226,  1883,   400,  1517,  -928,   664,   399,
     838,  -926,   391,   844,   427,  1408,  1409,   432,   402,   403,
     392,  -765,   997,   998,  1093,   399,   393,  -926,  1092,  1094,
    1007,   399,  1356,   661,  1358,   560,  1362,  1098,  1099,   435,
    2039,  -461,   394,    55,  -461,  -126,   427,   395,  1180,  -126,
     396,   399,   456,   186,   187,    65,    66,    67,  -966,  1032,
    1326,  1327,  1328,   553,   399,   707,  -126,   559,   547,   224,
   -1066,  1733,   661,   401,   402,   403,  1609,  2040,  2041,  2042,
     680,  1504,  1327,  1328,   150,   171,   402,   403,   413,   171,
     397,   440,  1004,  1005,   677,  1040,   414,   514,   415,   679,
     438,   662,   402,   403,   439,  1590,   442, -1066,   402,   403,
   -1066,   126,   445,   456,   186,   187,    65,    66,    67,   681,
    2057,   132,   117,  1459,  1052,   457,   117,   656,   402,   403,
     579,   434,   436,   437,  1469,  1586,   451,   691,   463,   226,
     450,   402,   403,  -641,  1448,   464,  1604,   224,   226,   456,
      63,    64,    65,    66,    67,  1096,   465,  2040,  2041,  2042,
      72,   507,   576,  1310,  1311,  1944,  1545,   226,   466,  1947,
     467,   264,  1288,  1478,  1102,   468,  1478,   469,   678,  1408,
    1409,  -642,  1971,  1491,   470,   224,   457,   224,  -643,  1699,
    1660,  1661,  -646,  2035,   657,   790,  1941,  1942,   663,   624,
    1252,  1972,   150,   509,  1973,  -644,  1113,  -645,   632,   503,
     637,  2065,  2066,   504,  1108,  1023,   599,   150,   505,   924,
     924,   229,   457,  1969,  1970,   506,   657,   660,   663,   657,
     663,   663,  -647,   680,   496,   497,   498,   535,   499,   132,
    -960,   456,   186,   187,    65,    66,    67,   677,  1965,  1966,
     500,  -763,   679,   171,  1172,  1173,  1174,  1175,  1176,  1177,
     150,  1634,  1239,  1242,  1207,   406,  1208,   544,   851,   727,
     643,   644,   681,  1178,   546,   500,  1642,   440,   552,  1210,
    1175,  1176,  1177,  1757,  -964,   536,  -761,   224,   561,   562,
     117,   570,   583, -1111,  1220,   591,  1178,   132,   594,   229,
     709,   601,  1278,  1727,   348,   602,   595,   617,   618,   620,
     627,  1606,   123,   388,   457,   630,   631,   640,   683,  2049,
     684,   641,   781,   150,  1248,   127,  -131,   725,   129,   725,
     693,   706,   694,   130,   795,   728,  2063,   229,    55,   229,
     493,   494,   495,   496,   497,   498,   150,   499,  1266,   695,
    1615,  1267,  1616,  1268,   697,   733,   823,   718,   825,   500,
     132,   656,   852,   827,   828,   833,   229,   599,   388,   785,
     388,   388,   388,   388,   834,   572,   856,   859,   150,   589,
     874,   873,  1229,   132,   905,  1446,   930,  1229,  1447,   907,
    1251,   908,  1251,   810,   730,   731,   732,   794,   246,   171,
    1734,   909,   724,   952,   954,   911,  1308,  2027,  1740,   910,
     126,   912,  1229,   599,   253,   132,   915,   167,   251,   252,
    1313,   916,   934,   935,  1747,   841,   945,   951,   938,  2027,
     939,   981,  1222,   953,   950,   955,   117,   956,  2050,   229,
     957,   958,   691,   964,  1314,   969,   815,   815,   970,  -786,
    1714,  1715,   972,   979,   980,   150,   902,   150,   982,   150,
     983,  1108,   224,   984,   987,   991,  1001,   680,   992,  1000,
    1985,  1008,  1002,  1229,   691,  1990,  1009,   226,  1011,  1014,
    1015,   677,   926,   709,  1433,  1434,   679,  1435,  1026,  1365,
    1030,  1016,  1037,  1017,   132,  1018,   132,  1038,  1034,  1729,
    1046,  1730,  1054,  1731,  1894,  1035,   681,  1055,  1732,   924,
    1056,   924,  -767,   924,  1095,   961,   963,   924,  1028,   924,
     924,  1192,  1105,  1115,  2015,  1117,  1119,  1123,   126,  1124,
    1125,  1127,  1424,  1141,  1039,  1142,  1143,  1003,  1144,  1145,
    1147,   844,  1146,   844,   725,  1148,  1183,  1193,  1195,  1197,
    1201,  1209,   171,  1199,  1200,   226,  1217,   123,  1206,   737,
    1215,  1218,  1451,   680,  1219,  1225,  1223,  1244,   150,  1230,
     127,  1261,  1273,   129,  1270,  1276,  1281,   677,   130,  1291,
    -969,  1292,   679,  1293,  1294,   718,   126,  1300,  1295,   117,
    1296,  1301,  1297,   226,  1302,   226,   718,   388,  1303,  2076,
    1305,   224,   681,  1316,  1984,   132,  1987,  1251,  1887,  1318,
    -963,  1228,  1321,   229,   229,  1051,  1228,  1323,  1332,  1333,
    1334,  1120,   226,  1340,   576,  1342,   533,  1126, -1127,  1343,
    1346,  1178,  1493,   264,  1347,  1398,  1400,  1412,  1401,  1402,
    1403,  1228,  1506,  1405,  1428,  1042,  1441,  1444,  1429,   126,
    1445,  1452,  1471,  1089,  1456,  1090,  1454,  1950,  1496,  1461,
    1458,  1470,   167,   224,  1463,  1472,  1064,   426,  1485,   150,
     171,  1460,   126,  1494,   423,   428,  1464,  1108,  1495,  1497,
     150,  1505,  1109,  1501,  1074,   171,  1088,  1511,  1514,  1518,
    1519,  1522,  1524,  1523,  2038,   226,  1527,  1528,  1530,  1202,
    1531,  1534,  1228,  1532,   126,  2051,  1533,   117,  1536,  2052,
    1537,   691,  1540,  1579,   691,  1539,  1544,   132,  1578,  1594,
    1546,  1111,   117,  2068,  2069,  1547,  1548,  1549,   171,  1577,
    1588,  1595,  1597,  1598,  1599,  1613,  1582,  1436,  1700,  1601,
    1619,  1584,  1603,  1585,   678,  1607,   725,  1611,  1618,  1614,
    1612,  1617,  1623,   229,  1621,  1198,  1628,  1622,  1625,  1596,
     459,  1626,  1627,  1630,  1629,   117,  1632,  1633,  1948,  1949,
    1635,  1636,  1605,   718,  1188,   709,  1637,  1643,  1639,  1640,
    1646,  1657,  1668,   126,  -448,   126,  -447,  -446,  1681,  1689,
    1693,   171,  1698,  1703,  1708,   924,  1711,  1712,  1716,  1721,
    1722,  1728,  1745,  1751,  1724,  1203,  1570,  1742,  1760,   723,
    1759,  1570,  1856,  1857,   171,   229,  1865,  1873,  1878,  1874,
    1876,  1877,  1880,  1886,  1888,  1889,  1925,  1921,   117,   229,
    1899,  2075,  1900,  1926,  1932,  1931,  1253,  1937,  1253,  1959,
    1961,  1963,  1304,  1967,   599,  1975,   171,  1860,  1974,  1082,
    1976,   117,  1982,  1983,  1986,  1991,  1995,  1996,   810,   810,
    -386,  1998,  1999,  1576,  2001,  2003,  2007,  2016,  1576,   226,
     226,  1915,  2010,  2018,  2017,  2029,  1282,  1283,  1284,   212,
    2023,  2024,  2036,   117,  2033,  1341,  2037,  2046,  2059,  1344,
    2048,   459,  2064,  2072,   126,  2053,  1348,  2073,  2054,  2077,
    2081,   150,    50,  2089,  1707,  2082,   691,  2090,  2092,  1713,
     388,   678,   718,   718,  1404,  1750,  2093,  2032,   789,  1285,
    1285,  1074,   784,   171,  1277,   171,   786,   171,  1272,  1109,
    1320,  1214,  1499,  1893,  2047,  1641,  2045,  1450,   132,   216,
     217,   218,   219,   220,  1884,   150,   928,  1908,  1913,  1758,
    2084,  1688,  2058,   841,  1882,   841,   651,  1669,  1483,  1357,
     117,  1422,   117,  1468,   117,  1287,  1349,    93,    94,  1701,
      95,   189,    97,  1475,  1474,   355,  1306,  1979,  2012,  2005,
    1133,  1659,   132,  1738,  1407,  1336,   719,  1338,  1397,     0,
     150,     0,     0,     0,   150,  1570,   108,     0,   150,     0,
       0,  1570,     0,  1570,  1752,     0,   126,     0,     0,   226,
       0,   599,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1928,     0,     0,     0,  1875,   132,     0,     0,
       0,     0,     0,     0,     0,   132,   171,     0,     0,     0,
     902,     0,     0,     0,     0,     0,  1473,     0,     0,     0,
       0,     0,  1576,  1253,     0,     0,     0,     0,  1576,     0,
    1576,     0,     0,   691,   678,     0,     0,     0,     0,  1449,
       0,   226,     0,   117,     0,     0,     0,     0,     0,     0,
    1891,  1738,     0,     0,     0,   226,  1082,     0,     0,     0,
     150,   150,   150,     0,     0,     0,   150,     0,     0,     0,
       0,     0,     0,   150,     0,   212,     0,  1525,     0,     0,
       0,  1529,     0,     0,     0,     0,  1535,     0,     0,  1570,
       0,     0,     0,     0,  1541,     0,     0,   132,    50,     0,
       0,  1492,     0,   132,     0,     0,     0,   171,  1922,     0,
     132,     0,     0,     0,     0,  1109,     0,     0,   171,     0,
       0,     0,     0,  1074,  1074,  1074,  1074,  1074,  1074,     0,
       0,  2021,  1074,     0,  1074,   216,   217,   218,   219,   220,
       0,     0,     0,  1555,   117,     0,  1576,     0,     0,     0,
       0,     0,     0,     0,     0,   117,     0,     0,  1933,     0,
     406,     0,  1555,    93,    94,  1513,    95,   189,    97,   349,
       0,     0,     0,     0,     0,   587,  1922,   588,     0,     0,
     223,   223,   459,     0,     0,    14,     0,     0,     0,     0,
       0,   244,   108,     0,     0,     0,   407,   126,     0,     0,
       0,     0,     0,  1620,    14,     0,     0,  1624,     0,     0,
       0,     0,     0,     0,  1631,     0,     0,   244,     0,     0,
     150,  1691,     0,     0,     0,  1587,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   593,   349,  1550,     0,
       0,   126,     0,     0,     0,     0,     0,     0,     0,     0,
    1556,     0,     0,     0,     0,     0,  1557,   132,   456,  1558,
     187,  1559,  1560,  1561,  1562,     0,     0,     0,   678,  1556,
       0,     0,     0,     0,     0,  1557,   150,   456,  1558,   187,
    1559,  1560,  1561,  1562,     0,     0,   126,     0,  1082,  1082,
    1082,  1082,  1082,  1082,   126,     0,     0,  1082,     0,  1082,
       0,  1074,     0,  1074,     0,     0,  1563,  1564,     0,  1565,
       0,   150,     0,   132,     0,     0,   150,     0,     0,     0,
       0,     0,     0,     0,     0,  1563,  1564,     0,  1565,   712,
       0,   457,   355,     0,     0,     0,     0,     0,     0,   171,
    1566,   150,     0,     0,     0,     0,     0,     0,   132,     0,
     457,     0,  1866,   132,     0,     0,     0,  2083,     0,  1580,
       0,     0,  2022,     0,   678,     0,  2091,     0,     0,     0,
       0,     0,     0,     0,  2094,     0,   117,  2095,   132,     0,
       0,     0,     0,   171,     0,     0,   126,   348,     0,     0,
       0,     0,   126,     0,     0,     0,  1686,     0,     0,   126,
     223,   150,   150,     0,   456,    63,    64,    65,    66,    67,
       0,     0,     0,     0,     0,    72,   507,     0,     0,     0,
     117,     0,   212,     0,     0,     0,   691,     0,   171,     0,
       0,     0,   171,     0,     0,     0,   171,     0,   132,   132,
       0,     0,     0,     0,     0,    50,     0,   244,   691,   244,
    1074,     0,  1074,     0,  1074,     0,   508,   691,   509,  1074,
       0,     0,     0,     0,     0,   117,  1082,     0,  1082,   117,
       0,     0,   510,   117,   511,  1555,     0,   457,   865,     0,
       0,     0,   216,   217,   218,   219,   220,     0,     0,     0,
    1555,     0,     0,   388,     0,     0,   599,     0,     0,   348,
       0,     0,     0,     0,   188,     0,     0,    91,   244,  1845,
      93,    94,     0,    95,   189,    97,  1852,    14,     0,     0,
       0,     0,     0,     0,   348,   348,     0,   348,   171,   171,
     171,     0,    14,   348,   171,     0,   223,     0,     0,   108,
       0,   171,     0,     0,  1953,   223,   126,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1074,
       0,     0,     0,     0,   223,   117,   117,   117,     0,     0,
       0,   117,   985,   986,     0,   223,     0,     0,   117,     0,
       0,     0,  1556,     0,     0,  1555,     0,     0,  1557,     0,
     456,  1558,   187,  1559,  1560,  1561,  1562,  1556,     0,     0,
       0,   244,   126,  1557,   244,   456,  1558,   187,  1559,  1560,
    1561,  1562,     0,     0,     0,  1082,     0,  1082,     0,  1082,
       0,   212,     0,     0,  1082,     0,     0,    14,     0,     0,
    1960,  1962,     0,     0,     0,     0,     0,   126,  1563,  1564,
       0,  1565,   126,     0,    50,     0,     0,     0,     0,     0,
       0,  1555,     0,  1563,  1564,     0,  1565,     0,     0,     0,
     244,     0,     0,   457,     0,     0,     0,   126,     0,     0,
       0,     0,  1719,     0,     0,     0,     0,     0,   457,     0,
       0,   216,   217,   218,   219,   220,     0,  1723,   171,     0,
       0,   599,  1556,    14,     0,     0,     0,     0,  1557,     0,
     456,  1558,   187,  1559,  1560,  1561,  1562,  1850,     0,    93,
      94,  1851,    95,   189,    97,   348,     0,     0,     0,  1074,
    1074,     0,     0,     0,  1082,   117,     0,   126,   126,     0,
       0,     0,     0,     0,  1954,     0,     0,     0,   108,  1685,
       0,  1845,  1845,     0,   171,  1852,  1852,     0,  1563,  1564,
     244,  1565,   244,     0,     0,   889,     0,     0,  1556,   599,
       0,     0,  1131,   712,  1557,     0,   456,  1558,   187,  1559,
    1560,  1561,  1562,   457,     0,     0,     0,     0,     0,   171,
       0,   117,  1725,     0,   171,     0,     0,     0,   889,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   899,     0,     0,     0,     0,     0,     0,   171,
       0,     0,     0,     0,  1563,  1564,   117,  1565,     0,     0,
       0,   117,     0,     0,     0,     0,     0,     0,     0,  2020,
       0,     0,     0,     0,     0,   929,     0,     0,     0,   457,
       0,     0,     0,     0,   244,   244,   117,     0,  1885,     0,
       0,     0,  2034,   244,     0,     0,     0,     0,  1213,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   171,
     171,     0,     0,     0,   223,     0,  1224,     0,     0,     0,
       0,     0,     0,     0,  1082,  1082,  1154,     0,     0,  1243,
       0,   542,   516,   517,   518,   519,   520,   521,   522,   523,
     524,   525,   526,   527,     0,  1155,   117,   117,  1156,  1157,
    1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
     471,   472,   473,  1670,     0,  1275,   528,   529,     0,     0,
       0,     0,     0,  1178,     0,     0,     0,     0,     0,     0,
     474,   475,   223,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,     0,   499,     0,     0,
       0,     0,     0,   212,   244,     0,     0,     0,     0,   500,
     223,     0,   223,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,  1331,     0,
       0,     0,  1335,   530,   531,     0,     0,  1339,     0,   223,
     889,     0,     0,     0,     0,     0,     0,   244,     0,  1671,
       0,     0,   866,     0,   244,   244,   889,   889,   889,   889,
     889,     0,  1672,   216,   217,   218,   219,   220,  1673,   889,
       0,     0, -1129, -1129, -1129, -1129, -1129,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,   188,   244,  1110,    91,  1674,
       0,    93,    94,     0,    95,  1675,    97,   696,  1178,     0,
       0,     0,   212,  1134,  1135,  1136,  1137,  1138,     0,     0,
       0,     0,   223,   867,     0,     0,  1149,     0,     0,     0,
     108,     0,     0,     0,     0,    50,   244,     0,     0,    34,
      35,    36,     0,     0,     0,     0,     0,     0,     0,  1437,
       0,   225,   225,   214,  1415,     0,     0,     0,     0,     0,
     244,   244,   245,     0,     0,     0,     0,     0,     0,     0,
       0,   223,   216,   217,   218,   219,   220,     0,   244,     0,
       0,     0,     0,     0,     0,   244,     0,     0,     0,     0,
       0,   244,  1462,     0,   188,  1465,     0,    91,     0,     0,
      93,    94,   889,    95,   189,    97,     0,   868,    81,    82,
      83,    84,    85,     0,     0,     0,     0,   244,     0,   221,
       0,     0,     0,     0,     0,    89,    90,     0,     0,   108,
       0,     0,     0,     0,     0,     0,     0,   244,     0,    99,
       0,   244,   471,   472,   473,     0,     0,     0,     0,  1249,
       0,   244,  1512,     0,   105,     0,     0,     0,     0,  1516,
       0,     0,   474,   475,     0,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,     0,   499,
       0,     0,     0,     0,     0,     0,   223,   223,     0,     0,
       0,   500,     0,     0,     0,     0,     0,     0,     0,     0,
     244,     0,     0,     0,   244,     0,   244,     0,     0,   244,
       0,     0,     0,     0,     0,     0,     0,  1551,  1552,     0,
       0,     0,   889,   889,   889,   889,   889,   889,   223,   889,
       0,     0,   889,   889,   889,   889,   889,   889,   889,   889,
     889,   889,   889,   889,   889,   889,   889,   889,   889,   889,
     889,   889,   889,   889,   889,   889,   889,   889,   889,   889,
       0,   225,     0,     0,     0,     0,     0,     0,     0,  1138,
    1352,     0,     0,  1352,     0,     0,     0,     0,   889,  1366,
    1369,  1370,  1371,  1373,  1374,  1375,  1376,  1377,  1378,  1379,
    1380,  1381,  1382,  1383,  1384,  1385,  1386,  1387,  1388,  1389,
    1390,  1391,  1392,  1393,  1394,  1395,  1396,     0,   471,   472,
     473,   244,     0,   244,     0,     0,     0,     0,     0,     0,
       0,     0,   821,  1644,  1645,  1406,   223,  1647,   474,   475,
       0,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   244,   499,     0,   244,     0,     0,
       0,     0,     0,     0,     0,  1665,     0,   500,     0,   212,
       0,     0,     0,   244,   244,   244,   244,   244,   244,  1692,
       0,   223,   244,     0,   244,     0,     0,     0,   223,     0,
       0,     0,    50,     0,     0,     0,     0,   225,     0,     0,
       0,     0,   223,     0,   889,     0,   225,     0,     0,     0,
       0,     0,     0,     0,   244,     0,     0,     0,  1684,     0,
       0,   244,     0,     0,     0,   225,   889,     0,   889,   216,
     217,   218,   219,   220,     0,     0,   225,   542,   516,   517,
     518,   519,   520,   521,   522,   523,   524,   525,   526,   527,
       0,  1502,  1741,   889,     0,     0,     0,    93,    94,     0,
      95,   189,    97,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1520,     0,  1521,     0,  1665,     0,     0,
       0,     0,   528,   529,     0,     0,   108,  1685,     0,   244,
     244,     0,     0,   244,   212,     0,     0,     0,     0,   501,
    1542,  1031,  1665,  1665,     0,  1665,     0,     0,     0,   212,
    1868,  1665,     0,     0,     0,     0,     0,    50,     0,     0,
     244,   245,     0,     0,   351,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,   542,   516,   517,   518,   519,
     520,   521,   522,   523,   524,   525,   526,   527,     0,     0,
       0,   244,     0,   244,   216,   217,   218,   219,   220,   530,
     531,     0,     0,     0,     0,     0,     0,     0,  1904,   216,
     217,   218,   219,   220,     0,     0,     0,     0,     0,     0,
     528,   529,    93,    94,     0,    95,   189,    97,     0,     0,
       0,     0,     0,     0,     0,   244,   244,    93,    94,   244,
      95,   189,    97,     0,     0,   889,     0,   889,     0,   889,
       0,   108,   733,     0,   889,   223,   897,     0,     0,   889,
       0,   889,     0,   835,   889,     0,   108,  1028,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   244,   244,     0,
       0,     0,     0,     0,   244,     0,     0,     0,     0,   897,
       0,   244,  1649,     0,  1650,   212,  1651,   530,   531,     0,
       0,  1652,     0,     0,     0,     0,  1654,     0,  1655,     0,
       0,  1656,     0,     0,  1927,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,   917,   918,     0,  1940,
       0,     0,     0,  1665,     0,     0,     0,     0,     0,   212,
     244,     0,   244,     0,   244,     0,     0,     0,     0,   244,
       0,   223,     0,     0,     0,   216,   217,   218,   219,   220,
       0,   936,    50,     0,   244,     0,     0,     0,     0,   889,
       0,     0,     0,     0,     0,   225,     0,     0,     0,     0,
     919,   244,   244,    93,    94,  1671,    95,   189,    97,   244,
       0,   244,     0,     0,   351,     0,   351,     0,  1672,   216,
     217,   218,   219,   220,  1673,     0,     0,     0,  2000,     0,
       0,     0,   108,     0,   244,   244,  1743,   244,     0,     0,
       0,   188,   244,   244,    91,    92,     0,    93,    94,     0,
      95,  1675,    97,     0,     0,  1940,     0,  2013,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   244,
       0,     0,     0,   225,     0,   351,   108,     0,     0,     0,
       0,     0,     0,     0,     0,   889,   889,   889,     0,     0,
       0,     0,   889,     0,   244,     0,     0,     0,     0,     0,
     244,     0,   244,     0,     0,  1081,     0,     0,     0,     0,
       0,   225,     0,   225,     0,     0,     0,  1019,   516,   517,
     518,   519,   520,   521,   522,   523,   524,   525,   526,   527,
       0,     0,  1895,  1896,  1897,     0,     0,     0,     0,  1901,
     225,   897,     0,     0,   866,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   897,   897,   897,
     897,   897,   528,   529,     0,     0,     0,     0,   351,     0,
     897,   351,     0,  1019,   516,   517,   518,   519,   520,   521,
     522,   523,   524,   525,   526,   527,     0,  1182,     0,     0,
       0,     0,     0,     0,   212,     0,     0,     0,   889,     0,
       0,     0,     0,     0,     0,   867,   244,     0,     0,     0,
       0,     0,     0,   225,     0,     0,     0,    50,   528,   529,
       0,   244,     0,     0,     0,   244,     0,  1205,     0,   244,
     244,     0,     0,     0,   286,     0,     0,     0,     0,   530,
     531,     0,     0,     0,   244,  1920,     0,     0,     0,     0,
     889,     0,  1205,     0,   216,   217,   218,   219,   220,     0,
       0,     0,   225,     0,     0,     0,   889,   889,     0,     0,
       0,   288,     0,   889,     0,     0,   188,     0,     0,    91,
       0,     0,    93,    94,   212,    95,   189,    97,     0,  1337,
     994,     0,     0,   897,     0,   530,   531,  1964,     0,     0,
     244,     0,     0,  1020,     0,     0,     0,    50,  1262,     0,
       0,   108,     0,  1977,  1978,   889,     0,   351,     0,   869,
    1981,     0,     0,     0,     0,     0,     0,   244,     0,   244,
       0,     0,   245,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1081,   585,   216,   217,   218,   219,   220,   586,
       0,     0,     0,     0,     0,     0,     0,     0,   244,   696,
       0,     0,  2004,     0,     0,     0,   188,     0,     0,    91,
     341,     0,    93,    94,   244,    95,   189,    97,   227,   227,
     244,     0,     0,     0,   244,     0,     0,   225,   225,   249,
     345,     0,     0,     0,     0,     0,     0,     0,   244,   244,
       0,   108,   347,     0,     0,     0,     0,     0,     0,     0,
       0,   351,   351,     0,     0,     0,     0,     0,     0,     0,
     351,     0,     0,   897,   897,   897,   897,   897,   897,   225,
     897,     0,     0,   897,   897,   897,   897,   897,   897,   897,
     897,   897,   897,   897,   897,   897,   897,   897,   897,   897,
     897,   897,   897,   897,   897,   897,   897,   897,   897,   897,
     897,     0,   471,   472,   473,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   897,
       0,     0,   474,   475,     0,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,     0,   499,
     471,   472,   473,     0,     0,     0,     0,     0,     0,     0,
       0,   500,     0,     0,     0,     0,     0,   225,     0,     0,
     474,   475,     0,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,     0,   499,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   500,
       0,     0,     0,     0,  1081,  1081,  1081,  1081,  1081,  1081,
       0,     0,   225,  1081,     0,  1081,     0,     0,     0,   225,
       0,     0,     0,     0,  1122,     0,     0,     0,   227,     0,
       0,   351,   351,   225,     0,   897,     0,   515,   516,   517,
     518,   519,   520,   521,   522,   523,   524,   525,   526,   527,
       0,     0,     0,     0,     0,     0,     0,   897,   212,   897,
       0,     0,     0,     0,     0,     0,   471,   472,   473,  1121,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,   528,   529,   897,   965,   474,   475,     0,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,     0,   499,     0,     0,     0,     0,   216,   217,
     218,   219,   220,     0,  1554,   500,     0,   351,     0,     0,
       0,     0,     0,   999,     0,     0,     0,     0,     0,     0,
     188,     0,     0,    91,     0,   351,    93,    94,     0,    95,
     189,    97,   351,     0,     0,     0,     0,     0,   351,   530,
     531,     0,     0,     0,   227,     0,     0,     0,     0,     0,
       0,     0,     0,   227,     0,   108,     0,     0,     0,     0,
       0,     0,  1081,     0,  1081,   471,   472,   473,     0,     0,
       0,     0,   227,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   249,   351,   474,   475,     0,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,     0,   499,     0,     0,     0,   897,     0,   897,     0,
     897,     0,     0,     0,   500,   897,   225,     0,     0,     0,
     897,     0,   897,     0,     0,   897,     0,     0,     0,  1031,
       0,     0,   212,     0,     0,     0,     0,     0,     0,  1667,
       0,   471,   472,   473,     0,  1680,     0,   351,     0,     0,
       0,   351,     0,   869,     0,    50,   351,     0,   249,     0,
       0,   474,   475,     0,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,     0,   499,     0,
       0,     0,   216,   217,   218,   219,   220,     0,     0,     0,
     500,  1081,     0,  1081,     0,  1081,     0,     0,     0,     0,
    1081,     0,   225,     0,   188,     0,     0,    91,    92,     0,
      93,    94,     0,    95,   189,    97,     0,   212,     0,   959,
     897,   960,     0,     0,     0,     0,     0,     0,  1152,  1153,
    1154,     0,  1748,  1749,     0,     0,     0,     0,  1057,   108,
      50,     0,  1680,   898,     0,     0,     0,     0,   351,  1155,
     351,     0,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,
    1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,     0,     0,   898,   216,   217,   218,
     219,   220,     0,     0,     0,     0,     0,  1178,     0,     0,
       0,   351,     0,     0,   351,     0,     0,     0,     0,     0,
    1081,     0,     0,     0,     0,    93,    94,     0,    95,   189,
      97,     0,     0,     0,     0,     0,   897,   897,   897,     0,
       0,     0,     0,   897,  1196,  1902,     0,     0,     0,     0,
       0,     0,     0,  1680,   108,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   471,   472,   473,     0,     0,
       0,   351,     0,     0,     0,     0,     0,     0,   351,     0,
       0,     0,   227,     0,     0,   474,   475,     0,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,     0,   499,  1058,  1059,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   500,     0,     0,     0,     0,     0,
       0,     0,     0,  1060,     0,     0,     0,     0,  1363,     0,
       0,  1061,  1062,  1063,   212,     0,   351,   351,     0,   897,
       0,     0,     0,     0,     0,  1064,     0,     0,     0,     0,
     227,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   351,     0,   212,
    1081,  1081,     0, -1129, -1129, -1129, -1129, -1129,   491,   492,
     493,   494,   495,   496,   497,   498,     0,   499,   227,     0,
     227,   897,    50,  1065,  1066,  1067,  1068,  1069,  1070,   500,
       0,     0,     0,     0,     0,     0,     0,   897,   897,     0,
       0,  1071,     0,     0,   897,     0,   188,   227,   898,    91,
      92,     0,    93,    94,     0,    95,   189,    97,     0,   216,
     217,   218,   219,   220,   898,   898,   898,   898,   898,     0,
    1072,  1073,   351,   351,     0,     0,   351,   898,  1269,     0,
       0,   108,     0,     0,     0,   453,   897,    93,    94,     0,
      95,   189,    97,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   351,   499,   108,     0,     0,     0,
     227,     0,     0,   471,   472,   473,     0,   500,   351,  2031,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   474,   475,  1667,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   249,
     499,     0,     0,     0,     0,     0,     0,     0,  1152,  1153,
    1154,     0,   500,     0,     0,   542,   516,   517,   518,   519,
     520,   521,   522,   523,   524,   525,   526,   527,     0,  1155,
     898,   351,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,
    1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,     0,     0,   351,     0,     0,     0,
     528,   529,     0,     0,     0,     0,     0,  1178,     0,   249,
       0,     0,   471,   472,   473,     0,     0,     0,     0,     0,
       0,   351,   351,     0,   351,     0,     0,     0,     0,   351,
     351,     0,   474,   475,     0,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,     0,   499,
       0,     0,     0,     0,   227,   227,     0,     0,     0,     0,
       0,   500,     0,     0,     0,     0,  1345,   530,   531,     0,
       0,     0,     0,     0,     0,     0,  1280,   351,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     898,   898,   898,   898,   898,   898,   249,   898,     0,     0,
     898,   898,   898,   898,   898,   898,   898,   898,   898,   898,
     898,   898,   898,   898,   898,   898,   898,   898,   898,   898,
     898,   898,   898,   898,   898,   898,   898,   898,     0,   471,
     472,   473,  1019,   516,   517,   518,   519,   520,   521,   522,
     523,   524,   525,   526,   527,     0,   898,     0,     0,   474,
     475,     0,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,     0,   499,   528,   529,     0,
       0,     0,     0,   351,     0,     0,     0,     0,   500,     0,
       0,     0,     0,     0,   227,  1309,     0,     0,   351,   471,
     472,   473,   351,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   474,
     475,  1955,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,     0,   499,     0,     0,   249,
       0,     0,     0,     0,   530,   531,   227,     0,   500,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     227,     0,   898,     0,     0,     0,     0,   351,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   898,     0,   898,     0,     0,     0,
       0,     0,     0,     0,   351,     0,   351,   471,   472,   473,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   898,  1583,     0,     0,     0,     0,   474,   475,     0,
     476,   477,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,     0,   499,     0,     0,   351,     0,     0,
       0,   351,     0,     0,     0,     0,   500,     0,     0,     0,
       0,     0,     0,   212,     0,   351,   351,     0,     0,   471,
     472,   473,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1705,     0,     0,     0,    50,     0,     0,   474,
     475,  1507,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,     0,   499,     0,     0,     0,
       0,     0,     0,   216,   217,   218,   219,   220,   500,     0,
       0,     0,     0,     0,     0,     0,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,   367,     0,
       0,    93,    94,     0,    95,   189,    97,     0,     0,     0,
      11,    12,    13,   898,     0,   898,     0,   898,     0,     0,
       0,     0,   898,   249,     0,     0,     0,   898,     0,   898,
     108,    14,   898,    15,    16,     0,     0,     0,     0,    17,
    1706,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,    56,    57,    58,     0,
      59,  -205,    60,    61,    62,    63,    64,    65,    66,    67,
    1508,    68,    69,    70,    71,    72,    73,     0,     0,   249,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,   898,    86,     0,
       0,    87,     0,     0,     0,     0,    88,    89,    90,    91,
      92,     0,    93,    94,     0,    95,    96,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,   103,     0,   104,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,     0,     0,   112,   113,
     114,   115,     0,     0,     0,     0,     0,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   898,   898,   898,     0,     0,     0,     0,
     898,     0,    14,     0,    15,    16,     0,     0,     0,  1907,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,     0,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,    56,    57,    58,
       0,    59,     0,    60,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,   898,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,    88,    89,    90,
      91,    92,     0,    93,    94,     0,    95,    96,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,   103,     0,   104,     0,   105,   106,   107,
       0,     0,   108,   109,     0,   110,   111,  1211,   898,   112,
     113,   114,   115,     0,     0,     0,     0,     0,     0,     5,
       6,     7,     8,     9,   898,   898,     0,     0,     0,    10,
       0,   898,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    2002,     0,     0,     0,    14,     0,    15,    16,     0,     0,
       0,     0,    17,   898,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,     0,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,    56,
      57,    58,     0,    59,     0,    60,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,    88,
      89,    90,    91,    92,     0,    93,    94,     0,    95,    96,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,   103,     0,   104,     0,   105,
     106,   107,     0,     0,   108,   109,     0,   110,   111,  1416,
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
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   109,     0,   110,   111,   698,     0,   112,   113,   114,
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
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   109,     0,   110,
     111,  1021,     0,   112,   113,   114,   115,     5,     6,     7,
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
       0,    59,  -205,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,    98,
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
       0,     0,     0,   188,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   189,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
       0,     0,     0,   105,   106,   107,     0,     0,   108,   109,
       0,   110,   111,  1181,     0,   112,   113,   114,   115,     5,
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
       0,    86,     0,     0,    87,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   109,     0,   110,   111,  1226,
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
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   109,     0,   110,   111,  1258,     0,   112,   113,   114,
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
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   109,     0,   110,
     111,  1315,     0,   112,   113,   114,   115,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,     0,    42,     0,     0,     0,    43,    44,    45,    46,
    1317,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,    98,
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
       0,    49,  1503,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   188,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   189,    97,    98,     0,     0,    99,     0,
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
       0,    86,     0,     0,    87,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   109,     0,   110,   111,  1658,
       0,   112,   113,   114,   115,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,  -295,
      34,    35,    36,    37,    38,    39,    40,     0,    41,     0,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,    98,     0,     0,
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
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   109,     0,   110,
     111,  1898,     0,   112,   113,   114,   115,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,     0,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,  1951,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,    98,
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
       0,     0,    43,    44,    45,    46,     0,    47,  1994,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   188,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   189,    97,    98,     0,     0,    99,     0,
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
       0,    86,     0,     0,    87,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   109,     0,   110,   111,  2011,
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
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   109,     0,   110,   111,  2014,     0,   112,   113,   114,
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
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   109,     0,   110,
     111,  2030,     0,   112,   113,   114,   115,     5,     6,     7,
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
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,     0,     0,     0,   105,   106,   107,
       0,     0,   108,   109,     0,   110,   111,  2085,     0,   112,
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
       0,     0,     0,   188,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   189,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
       0,     0,     0,   105,   106,   107,     0,     0,   108,   109,
       0,   110,   111,  2086,     0,   112,   113,   114,   115,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,   568,     0,
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
       0,    86,     0,     0,    87,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   109,     0,   110,   111,     0,
       0,   112,   113,   114,   115,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,   853,     0,     0,     0,     0,     0,
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
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   109,     0,   110,   111,     0,     0,   112,   113,   114,
     115,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
    1112,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   109,     0,   110,
     111,     0,     0,   112,   113,   114,   115,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,  1737,     0,     0,     0,
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
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,     0,     0,     0,   105,   106,   107,
       0,     0,   108,   109,     0,   110,   111,     0,     0,   112,
     113,   114,   115,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,  1890,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,   188,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   189,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
       0,     0,     0,   105,   106,   107,     0,     0,   108,   109,
       0,   110,   111,     0,     0,   112,   113,   114,   115,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
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
       0,    86,     0,     0,    87,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   109,     0,   110,   111,     0,
       0,   112,   113,   114,   115,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   356,
     422,    13,     0,     0,     0,     0,     0,     0,     0,     0,
     791,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,   356,     0,    13,     0,
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
     100,     5,     6,     7,     8,     9,   101,   102,     0,     0,
       0,    10,   105,   106,   107,     0,     0,   108,   190,     0,
     357,     0,     0,     0,   112,   113,   114,   115,     0,     0,
       0,     0,     0,     0,     0,     0,   713,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,   714,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   185,
     186,   187,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,     0,   715,     0,    99,     0,     0,   100,
       5,     6,     7,     8,     9,   101,   102,     0,     0,     0,
      10,   105,   106,   107,     0,     0,   108,   190,     0,     0,
       0,     0,     0,   112,   113,   114,   115,     0,     0,     0,
       0,     0,     0,     0,     0,  1245,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,  1246,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   185,   186,
     187,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     188,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     189,    97,     0,  1247,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   190,     5,     6,     7,
       8,     9,   112,   113,   114,   115,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   356,   422,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,     0,     0,     0,   105,   106,   107,
       0,     0,   108,   109,     5,     6,     7,     8,     9,   112,
     113,   114,   115,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   356,     0,
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
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   188,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   189,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     190,     0,     0,   848,     0,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   356,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   791,     0,     0,     0,     0,
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
     105,   106,   107,     0,     0,   108,   190,     0,     0,     0,
       0,     0,   112,   113,   114,   115,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,   203,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,   185,   186,   187,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
      97,     0,     0,     0,    99,     0,     0,   100,     5,     6,
       7,     8,     9,   101,   102,     0,     0,     0,    10,   105,
     106,   107,     0,     0,   108,   190,     0,     0,     0,     0,
       0,   112,   113,   114,   115,     0,     0,     0,     0,     0,
       0,     0,     0,   239,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   185,   186,   187,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,     0,
       0,     0,    99,     0,     0,   100,     5,     6,     7,     8,
       9,   101,   102,     0,     0,     0,    10,   105,   106,   107,
       0,     0,   108,   190,     0,   271,     0,     0,     0,   112,
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
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   190,     0,   274,     0,     0,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   422,     0,     0,
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
     100,     5,     6,     7,     8,     9,   101,   102,     0,     0,
       0,    10,   105,   106,   107,     0,     0,   108,   109,     0,
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
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   190,   566,     0,
       0,     0,     0,   112,   113,   114,   115,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   356,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,   108,   190,     0,     0,     0,     0,     0,   112,
     113,   114,   115,     0,     0,   744,     0,     0,     0,     0,
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
     791,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     115,     0,     0,     0,     0,     0,     0,     0,     0,   829,
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
       0,     0,     0,     0,     0,     0,     0,     0,   831,     0,
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
       0,     0,     0,     0,     0,     0,     0,  1307,     0,     0,
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
       0,     0,   105,   106,   107,     0,     0,   108,   190,     5,
       6,     7,     8,     9,   112,   113,   114,   115,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   356,     0,     0,     0,     0,     0,     0,
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
      97,     0,     0,     0,    99,     0,     0,   100,     5,     6,
       7,     8,     9,   101,   102,     0,     0,     0,    10,   105,
     106,   107,     0,     0,   108,  1431,     0,     0,     0,     0,
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
       0,     0,    34,    35,    36,    37,   658,    39,    40,     0,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   185,   186,   187,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,     0,
     276,   277,    99,   278,   279,   100,     0,   280,   281,   282,
     283,   101,   102,     0,     0,     0,     0,   105,   106,   107,
       0,     0,   108,   190,     0,   284,   285,     0,     0,   112,
     113,   114,   115,  1157,  1158,  1159,  1160,  1161,  1162,  1163,
    1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,   287,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1178,   289,   290,
     291,   292,   293,   294,   295,     0,     0,     0,   212,     0,
     213,    40,     0,     0,   296,     0,     0,     0,     0,     0,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,    50,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   212,   332,     0,   779,
     334,   335,   336,     0,     0,     0,   337,   596,   216,   217,
     218,   219,   220,   597,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,   276,   277,     0,   278,   279,     0,
     598,   280,   281,   282,   283,     0,    93,    94,     0,    95,
     189,    97,   342,     0,   343,     0,     0,   344,     0,   284,
     285,     0,     0,     0,     0,   346,   216,   217,   218,   219,
     220,     0,     0,     0,     0,   108,     0,     0,     0,   780,
       0,     0,   112,     0,     0,     0,     0,     0,   287,     0,
       0,   919,     0,     0,    93,    94,     0,    95,   189,    97,
       0,     0,   289,   290,   291,   292,   293,   294,   295,     0,
       0,     0,   212,     0,   213,    40,     0,     0,   296,     0,
       0,     0,     0,   108,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,    50,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
       0,   332,     0,   333,   334,   335,   336,     0,     0,     0,
     337,   596,   216,   217,   218,   219,   220,   597,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   276,   277,
       0,   278,   279,     0,   598,   280,   281,   282,   283,     0,
      93,    94,     0,    95,   189,    97,   342,     0,   343,     0,
       0,   344,     0,   284,   285,     0,   286,     0,     0,   346,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   108,
       0,     0,     0,   780,     0,     0,   112,     0,     0,     0,
       0,     0,   287,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   288,     0,     0,   289,   290,   291,   292,
     293,   294,   295,     0,     0,     0,   212,     0,     0,     0,
       0,     0,   296,     0,     0,     0,     0,     0,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,    50,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,     0,   332,     0,     0,   334,   335,
     336,     0,     0,     0,   337,   338,   216,   217,   218,   219,
     220,   339,     0,     0,     0,     0,     0,     0,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   340,     0,
       0,    91,   341,     0,    93,    94,     0,    95,   189,    97,
     342,    50,   343,     0,     0,   344,     0,   276,   277,     0,
     278,   279,   345,   346,   280,   281,   282,   283,     0,     0,
       0,     0,     0,   108,   347,     0,     0,     0,  1869,     0,
       0,     0,   284,   285,     0,   286,     0,     0,   216,   217,
     218,   219,   220,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   287,   499,     0,     0,     0,    93,    94,     0,    95,
     189,    97,   288,     0,   500,   289,   290,   291,   292,   293,
     294,   295,     0,     0,     0,   212,     0,     0,     0,     0,
       0,   296,     0,     0,     0,   108,     0,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,    50,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,     0,   332,     0,     0,   334,   335,   336,
       0,     0,     0,   337,   338,   216,   217,   218,   219,   220,
     339,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   340,     0,     0,
      91,   341,     0,    93,    94,     0,    95,   189,    97,   342,
       0,   343,     0,     0,   344,     0,   276,   277,     0,   278,
     279,   345,   346,   280,   281,   282,   283,     0,     0,     0,
       0,     0,   108,   347,     0,     0,     0,  1946,     0,     0,
       0,   284,   285,     0,   286,   475,     0,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     287,   499,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   288,     0,   500,   289,   290,   291,   292,   293,   294,
     295,     0,     0,     0,   212,     0,     0,     0,     0,     0,
     296,     0,     0,     0,     0,     0,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,    50,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,     0,   332,     0,   333,   334,   335,   336,     0,
       0,     0,   337,   338,   216,   217,   218,   219,   220,   339,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   340,     0,     0,    91,
     341,     0,    93,    94,     0,    95,   189,    97,   342,     0,
     343,     0,     0,   344,     0,   276,   277,     0,   278,   279,
     345,   346,   280,   281,   282,   283,     0,     0,     0,     0,
       0,   108,   347,     0,     0,     0,     0,     0,     0,     0,
     284,   285,     0,   286,  1155,     0,     0,  1156,  1157,  1158,
    1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,   287,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     288,     0,  1178,   289,   290,   291,   292,   293,   294,   295,
       0,     0,     0,   212,     0,     0,     0,     0,     0,   296,
       0,     0,     0,     0,     0,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,    50,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,     0,   332,     0,     0,   334,   335,   336,     0,     0,
       0,   337,   338,   216,   217,   218,   219,   220,   339,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   340,     0,     0,    91,   341,
       0,    93,    94,     0,    95,   189,    97,   342,     0,   343,
       0,     0,   344,     0,     0,     0,     0,     0,     0,   345,
     346,  1662,     0,     0,     0,   276,   277,     0,   278,   279,
     108,   347,   280,   281,   282,   283,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     284,   285,     0,   286,     0,     0,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   287,
     499,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     288,     0,   500,   289,   290,   291,   292,   293,   294,   295,
       0,     0,     0,   212,     0,     0,     0,     0,     0,   296,
       0,     0,     0,     0,     0,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,    50,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,     0,   332,     0,     0,   334,   335,   336,     0,     0,
       0,   337,   338,   216,   217,   218,   219,   220,   339,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   340,     0,     0,    91,   341,
       0,    93,    94,     0,    95,   189,    97,   342,     0,   343,
       0,     0,   344,     0,  1763,  1764,  1765,  1766,  1767,   345,
     346,  1768,  1769,  1770,  1771,     0,     0,     0,     0,     0,
     108,   347,     0,     0,     0,     0,     0,     0,  1772,  1773,
    1774,     0,   474,   475,     0,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,  1775,   499,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   500,  1776,  1777,  1778,  1779,  1780,  1781,  1782,     0,
       0,     0,   212,     0,     0,     0,     0,     0,  1783,     0,
       0,     0,     0,     0,  1784,  1785,  1786,  1787,  1788,  1789,
    1790,  1791,  1792,  1793,  1794,    50,  1795,  1796,  1797,  1798,
    1799,  1800,  1801,  1802,  1803,  1804,  1805,  1806,  1807,  1808,
    1809,  1810,  1811,  1812,  1813,  1814,  1815,  1816,  1817,  1818,
    1819,  1820,  1821,  1822,  1823,  1824,  1825,     0,     0,     0,
    1826,  1827,   216,   217,   218,   219,   220,     0,  1828,  1829,
    1830,  1831,  1832,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1833,  1834,  1835,     0,     0,     0,
      93,    94,     0,    95,   189,    97,  1836,     0,  1837,  1838,
       0,  1839,     0,     0,     0,     0,     0,     0,  1840,     0,
    1841,     0,  1842,     0,  1843,  1844,     0,   276,   277,   108,
     278,   279,     0,     0,   280,   281,   282,   283,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   284,   285,  1156,  1157,  1158,  1159,  1160,  1161,
    1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,     0,     0,     0,     0,
       0,   287,     0,     0,     0,     0,     0,     0,     0,  1178,
       0,     0,     0,     0,     0,   289,   290,   291,   292,   293,
     294,   295,     0,     0,     0,   212,     0,     0,     0,     0,
       0,   296,     0,     0,     0,     0,     0,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,    50,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,     0,   332,     0,   333,   334,   335,   336,
       0,     0,     0,   337,   596,   216,   217,   218,   219,   220,
     597,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   276,   277,     0,   278,   279,     0,   598,   280,   281,
     282,   283,     0,    93,    94,     0,    95,   189,    97,   342,
       0,   343,     0,     0,   344,     0,   284,   285,     0,     0,
       0,     0,   346,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   108,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   287,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   289,
     290,   291,   292,   293,   294,   295,     0,     0,     0,   212,
       0,     0,     0,     0,     0,   296,     0,     0,     0,     0,
       0,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,    50,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,     0,   332,     0,
    1364,   334,   335,   336,     0,     0,     0,   337,   596,   216,
     217,   218,   219,   220,   597,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   276,   277,     0,   278,   279,
       0,   598,   280,   281,   282,   283,     0,    93,    94,     0,
      95,   189,    97,   342,     0,   343,     0,     0,   344,     0,
     284,   285,     0,     0,     0,     0,   346,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   108,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   287,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   289,   290,   291,   292,   293,   294,   295,
       0,     0,     0,   212,     0,     0,     0,     0,     0,   296,
       0,     0,     0,     0,     0,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,    50,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,     0,   332,     0,     0,   334,   335,   336,     0,     0,
       0,   337,   596,   216,   217,   218,   219,   220,   597,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   598,     0,     0,     0,     0,
       0,    93,    94,     0,    95,   189,    97,   342,     0,   343,
       0,     0,   344,   471,   472,   473,     0,     0,     0,     0,
     346,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     108,     0,     0,   474,   475,     0,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,     0,
     499,   471,   472,   473,     0,     0,     0,     0,     0,     0,
       0,     0,   500,     0,     0,     0,     0,     0,     0,     0,
       0,   474,   475,     0,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,     0,   499,   471,
     472,   473,     0,     0,     0,     0,     0,     0,     0,     0,
     500,     0,     0,     0,     0,     0,     0,     0,     0,   474,
     475,     0,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,     0,   499,     0,   471,   472,
     473,     0,     0,     0,     0,     0,     0,     0,   500,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   474,   475,
       0,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   501,   499,   471,   472,   473,     0,
       0,     0,     0,     0,     0,     0,     0,   500,     0,     0,
       0,     0,     0,     0,     0,     0,   474,   475,     0,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   582,   499,   471,   472,   473,     0,     0,     0,
       0,     0,     0,     0,     0,   500,   286,     0,     0,     0,
       0,     0,     0,     0,   474,   475,     0,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     584,   499,     0,   288,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   500,     0,     0,   212,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,     0,   499,    50,
       0,     0,     0,     0,     0,     0,     0,  -433,     0,   603,
     500,     0,     0,     0,     0,     0,   456,   186,   187,    65,
      66,    67,   286,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   585,   216,   217,   218,   219,
     220,   586,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   607,   188,   288,
       0,    91,   341,     0,    93,    94,     0,    95,   189,    97,
       0,     0,   212,     0,  1689,     0,     0,     0,     0,     0,
       0,     0,   345,     0,     0,     0,     0,     0,     0,   457,
       0,     0,     0,   108,   347,    50,     0,     0,     0,     0,
       0,     0,     0,   286,   845,     0,     0,     0,     0,     0,
       0,     0,   456,   186,   187,    65,    66,    67,     0,   286,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   585,   216,   217,   218,   219,   220,   586,     0,     0,
     288,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   212,   188,     0,   288,    91,   341,  1128,
      93,    94,     0,    95,   189,    97,     0,     0,     0,   212,
       0,     0,     0,     0,     0,  1515,    50,     0,   345,     0,
       0,     0,     0,     0,     0,   457,     0,     0,     0,   108,
     347,     0,    50,     0,     0,     0,     0,     0,     0,   286,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   585,   216,   217,   218,   219,   220,   586,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   585,   216,
     217,   218,   219,   220,   586,   188,   288,     0,    91,   341,
       0,    93,    94,     0,    95,   189,    97,   286, -1128,   212,
       0,   188,     0,     0,    91,   341,     0,    93,    94,   345,
      95,   189,    97,     0, -1128,     0,     0,     0,     0,     0,
     108,   347,    50,     0,     0,   345,     0,     0,     0,     0,
     592,     0,     0,     0,   288,     0,   108,   347,     0,     0,
       0,     0,     0,     0,     0,   286,     0,   212,     0,     0,
       0,     0,     0,  1439,     0,     0,     0,     0,   585,   216,
     217,   218,   219,   220,   586,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   188,   288,     0,    91,   341,     0,    93,    94,     0,
      95,   189,    97,     0,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   345,   585,   216,   217,   218,
     219,   220,   586,     0,     0,     0,   108,   347,    50,     0,
       0,     0,     0,     0,     0,  1372,     0,     0,     0,   188,
       0,     0,    91,   341,     0,    93,    94,     0,    95,   189,
      97,     0,     0,   875,   876,     0,     0,     0,     0,   877,
       0,   878,     0,   345,   585,   216,   217,   218,   219,   220,
     586,     0,     0,   879,   108,   347,     0,     0,     0,     0,
       0,    34,    35,    36,   212,     0,     0,   188,     0,     0,
      91,   341,     0,    93,    94,   214,    95,   189,    97,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,   345,     0,     0,     0,     0,     0,  1106,     0,     0,
       0,     0,   108,   347,  1159,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,   880,   881,   882,   883,   884,   885,    29,
      81,    82,    83,    84,    85,     0,  1178,    34,    35,    36,
     212,   221,   213,    40,     0,     0,   188,    89,    90,    91,
      92,   214,    93,    94,     0,    95,   189,    97,     0,     0,
       0,    99,     0,    50,     0,     0,     0,     0,     0,     0,
     886,   887,     0,     0,     0,     0,   105,     0,     0,     0,
     215,   108,   888,     0,     0,   875,   876,     0,     0,     0,
       0,   877,     0,   878,     0,     0,     0,     0,    74,    75,
     216,   217,   218,   219,   220,   879,    81,    82,    83,    84,
      85,     0,     0,    34,    35,    36,   212,   221,     0,     0,
       0,     0,   188,    89,    90,    91,    92,   214,    93,    94,
       0,    95,   189,    97,     0,     0,     0,    99,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   105,     0,     0,     0,     0,   108,   222,     0,
       0,     0,     0,     0,   112,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   880,   881,   882,   883,   884,
     885,    29,    81,    82,    83,    84,    85,     0,     0,    34,
      35,    36,   212,   221,   213,    40,     0,     0,   188,    89,
      90,    91,    92,   214,    93,    94,     0,    95,   189,    97,
       0,     0,     0,    99,     0,    50,     0,     0,     0,     0,
       0,     0,   886,   887,     0,     0,     0,     0,   105,     0,
       0,     0,   215,   108,   888,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,   216,   217,   218,   219,   220,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,   221,
       0,     0,     0,     0,   188,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   189,    97,    29,  1050,     0,    99,
       0,     0,     0,     0,    34,    35,    36,   212,     0,   213,
      40,     0,     0,     0,   105,     0,     0,     0,   214,   108,
     222,     0,     0,   623,     0,     0,   112,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   215,  1161,  1162,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,    74,    75,   216,   217,   218,
     219,   220,    29,    81,    82,    83,    84,    85,  1178,     0,
      34,    35,    36,   212,   221,   213,    40,     0,     0,   188,
      89,    90,    91,    92,   214,    93,    94,     0,    95,   189,
      97,     0,     0,     0,    99,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   105,
       0,     0,     0,   215,   108,   222,     0,     0,     0,     0,
       0,   112,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,   216,   217,   218,   219,   220,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
     221,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,     0,     0,     0,
      99,     0,     0,     0,     0,   471,   472,   473,     0,     0,
       0,     0,     0,     0,     0,   105,     0,     0,     0,     0,
     108,   222,     0,     0,     0,   474,   475,   112,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,     0,   499,   471,   472,   473,     0,     0,     0,     0,
       0,     0,     0,     0,   500,     0,     0,     0,     0,     0,
       0,     0,     0,   474,   475,     0,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,     0,
     499,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   500,     0,   471,   472,   473,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   545,   474,   475,     0,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
       0,   499,   471,   472,   473,     0,     0,     0,     0,     0,
       0,     0,     0,   500,     0,     0,     0,     0,     0,     0,
       0,   554,   474,   475,     0,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,     0,   499,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   500,     0,   471,   472,   473,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   949,   474,   475,     0,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,     0,
     499,   471,   472,   473,     0,     0,     0,     0,     0,     0,
       0,     0,   500,     0,     0,     0,     0,     0,     0,     0,
    1036,   474,   475,     0,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,     0,   499,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     500,     0,   471,   472,   473,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1091,   474,   475,     0,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,     0,   499,
    1152,  1153,  1154,     0,     0,     0,     0,     0,     0,     0,
       0,   500,     0,     0,     0,     0,     0,     0,     0,  1414,
       0,  1155,     0,     0,  1156,  1157,  1158,  1159,  1160,  1161,
    1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1178,
    1152,  1153,  1154,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1457,  1155,     0,     0,  1156,  1157,  1158,  1159,  1160,  1161,
    1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,  1152,  1153,  1154,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1178,
       0,     0,     0,     0,     0,     0,     0,  1155,  1526,     0,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1178,  1152,  1153,  1154,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1155,  1538,     0,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1152,  1153,  1154,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1178,     0,     0,     0,     0,
       0,     0,     0,  1155,  1648,     0,  1156,  1157,  1158,  1159,
    1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,    34,    35,
      36,   212,     0,   213,    40,     0,     0,     0,     0,     0,
       0,  1178,   214,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1744,     0,     0,     0,     0,     0,
       0,   242,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
       0,   216,   217,   218,   219,   220,     0,    81,    82,    83,
      84,    85,     0,  1178,     0,     0,     0,     0,   221,     0,
    1746,     0,     0,   188,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   189,    97,     0,     0,     0,    99,     0,
      34,    35,    36,   212,     0,   213,    40,     0,     0,     0,
       0,     0,     0,   105,   672,     0,     0,     0,   108,   243,
       0,     0,     0,     0,     0,   112,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   215,     0,   212,     0,   213,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   216,   217,   218,   219,   220,    50,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,   212,
     221,   213,    40,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,   212,    95,   189,    97,     0,     0,     0,
      99,     0,    50,     0,     0,   216,   217,   218,   219,   220,
       0,     0,     0,     0,     0,   105,    50,     0,     0,     0,
     108,   673,     0,     0,   364,   365,     0,   112,     0,     0,
       0,   808,     0,    93,    94,     0,    95,   189,    97,   216,
     217,   218,   219,   220,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   216,   217,   218,   219,   220,     0,     0,
       0,     0,   108,     0,     0,   808,   809,    93,    94,   112,
      95,   189,    97,     0,     0,   366,     0,     0,   367,     0,
       0,    93,    94,     0,    95,   189,    97,     0,   471,   472,
     473,     0,     0,     0,     0,     0,   108,     0,     0,     0,
     840,   368,     0,   112,     0,   857,     0,     0,   474,   475,
     108,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,     0,   499,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   500,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   471,   472,   473,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   858,   474,   475,  1033,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,     0,   499,
     471,   472,   473,     0,     0,     0,     0,     0,     0,     0,
       0,   500,     0,     0,     0,     0,     0,     0,     0,     0,
     474,   475,     0,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,     0,   499,  1152,  1153,
    1154,     0,     0,     0,     0,     0,     0,     0,     0,   500,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1155,
    1543,     0,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,
    1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1152,  1153,  1154,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1178,     0,     0,
       0,     0,     0,     0,     0,  1155,     0,     0,  1156,  1157,
    1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
     472,   473,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1178,     0,     0,     0,     0,     0,   474,
     475,     0,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,     0,   499,  1153,  1154,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   500,     0,
       0,     0,     0,     0,     0,     0,     0,  1155,     0,     0,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,   473,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1178,     0,     0,     0,     0,
     474,   475,     0,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,     0,   499,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   500,
    1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1178,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,     0,   499,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   500,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,     0,   499,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   500, -1129, -1129,
   -1129, -1129,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,     0,   499,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   500, -1129,
   -1129, -1129, -1129,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1178
};

static const yytype_int16 yycheck[] =
{
       5,     6,   132,     8,     9,    10,    11,    12,    13,   167,
      15,    16,    17,    18,    56,   568,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   109,    31,   414,   192,     4,
      44,    98,   109,   109,   728,    33,   103,   104,   690,    44,
       4,   414,     4,   172,   173,     4,   414,    52,    46,    54,
       4,   604,    57,    51,    59,    19,    20,   687,    30,   124,
     124,   848,   124,   193,   131,   717,   414,   990,   535,   536,
     244,   686,    86,   992,    30,    56,   532,     4,    30,    30,
     167,    86,   788,   503,   504,   499,  1411,   447,   448,   449,
     532,   856,   665,   863,    57,   562,   445,     9,   617,   618,
    1027,   980,  1105,   109,   109,  1883,   190,  1215,   564,  1012,
      32,  1014,     9,   190,   190,    57,     9,   537,  1045,   364,
     365,   366,   564,   368,     9,     9,   255,     9,     9,   254,
      14,     9,    14,    14,     4,     9,    54,     9,   222,    14,
       9,     9,    32,     9,     9,   222,   222,     9,    83,     9,
      48,     9,     9,    14,    48,  1692,     9,     9,    83,   243,
      70,     9,    48,     9,  1701,     9,   243,     9,     9,     9,
      83,     9,   116,    81,   561,    70,    83,    83,  1101,   103,
      83,   107,   108,    38,    38,   190,    91,    38,  1225,    91,
      36,   136,   137,   198,   162,    70,   136,   137,    14,    38,
    2027,   162,   162,   103,   123,    48,   199,    19,    20,    70,
     183,   168,  1295,   132,  1297,    48,    32,   222,  1301,   873,
     103,   199,   183,  2050,   132,    57,   199,     0,    83,    83,
      50,    51,    83,    70,   202,    51,   180,    69,   243,   199,
     167,   112,   166,    70,    83,   202,    70,   160,   161,   167,
     199,   176,   257,   267,   159,   260,   162,   159,   203,   199,
      70,    70,   267,   268,   182,  1556,   166,   162,  1147,   921,
     205,  2049,    70,   176,   399,   239,    70,   203,    83,    84,
     202,    70,    70,   166,   202,  2063,   261,    70,   195,   199,
     265,    70,   204,    54,   458,   202,   990,   261,   196,   183,
     853,   265,   386,    70,   201,   202,   200,   202,   201,   386,
     386,   201,   124,   355,   200,     8,   201,   201,  1321,   201,
     201,   176,   176,   201,   199,   176,  1229,   201,  1022,   201,
      70,  1868,   201,   201,  1659,   201,   201,   176,   199,   201,
     994,   201,    60,   201,   201,  1115,  1454,  1117,   201,   201,
    1277,   356,   200,  1461,   200,  1463,   200,   200,   200,   200,
     200,   445,   200,   196,   184,   202,     4,   200,   445,   445,
      88,   385,   199,    91,   355,   202,   540,  1668,   202,   162,
     385,   386,    70,   823,   514,   452,   391,   392,   393,   394,
     395,   396,   397,   202,  1024,   968,    70,   402,   917,   918,
     205,   443,  1693,  1694,   202,  1696,   167,  1101,   202,    70,
     415,   375,    38,   202,   202,  1452,   199,   422,   166,   202,
     384,  1300,  1945,   202,  1947,   430,   395,   239,   397,    70,
     199,   122,   199,   183,   183,   202,    83,   442,   202,   403,
     445,   132,   509,   510,   511,   512,   415,    70,    70,   199,
     199,    83,    70,   201,   202,   460,   461,    83,   199,   199,
     200,   199,   443,    19,    20,   429,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   132,   500,   502,   502,   503,   504,
     505,   162,   429,   445,  1612,    83,    84,  1213,     4,   515,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,    19,    20,   506,    83,   499,   202,   176,
     535,   536,   537,   538,    91,    70,   541,   542,   199,   162,
     545,   202,   998,   499,   176,     4,   704,   499,   499,   554,
     555,   421,   515,  1318,    38,   193,   998,   562,   199,  1112,
     502,   202,   103,   375,  1601,   570,  1603,   572,   693,   199,
     695,   552,   384,   515,   386,  1042,    70,   199,  1232,   202,
     202,   199,   107,   108,   202,   168,   364,   365,   366,   673,
     368,   403,    70,   980,    53,    50,    51,    56,   181,    83,
     575,   136,   137,   160,   161,   561,    14,   980,   782,    70,
     199,   575,   980,   109,    73,  1940,  1231,   704,   623,   202,
    1060,    83,   136,   137,   202,   166,   790,   205,   570,    91,
     199,  1261,   980,   162,  1264,   506,  1050,    96,  1094,    98,
     814,   199,   136,   137,   103,   104,   364,   365,   366,   367,
     368,   208,  1094,  1013,   183,  1534,  1016,  1017,   136,   137,
      31,   532,   907,  1012,   909,  1014,   911,   202,   673,   736,
     915,   183,   131,   202,  1193,   136,   137,    83,   203,    50,
    1717,   552,    53,   239,  1721,    91,  1340,   199,   406,    83,
     561,   183,   199,   564,   190,   103,   104,    91,   160,   161,
     199,   162,   827,   828,   833,    83,    70,   199,   833,   834,
     715,    83,  1144,    91,  1146,   889,  1148,    75,    76,    91,
      87,   199,    70,   112,   202,   162,   222,    70,   902,   166,
      70,    83,   121,   122,   123,   124,   125,   126,   199,   744,
     106,   107,   108,   239,    83,   207,   183,   243,   207,   713,
     162,  1630,    91,   159,   160,   161,  1462,   124,   125,   126,
    1147,   106,   107,   108,   728,   261,   160,   161,   167,   265,
      70,   183,    83,    84,  1147,   780,   199,   704,   199,  1147,
      32,   159,   160,   161,   166,  1439,   199,   199,   160,   161,
     202,   429,   199,   121,   122,   123,   124,   125,   126,  1147,
      87,   728,   261,  1270,   809,   194,   265,   159,   160,   161,
     269,   113,   114,   115,  1281,  1430,    38,   687,   201,   375,
     118,   160,   161,    70,  1244,   201,  1456,   791,   384,   121,
     122,   123,   124,   125,   126,   840,   201,   124,   125,   126,
     132,   133,   856,    75,    76,  1882,  1399,   403,   201,  1886,
     201,   856,  1292,  1293,   852,   201,  1296,   201,   414,   103,
     104,    70,    31,  1303,   201,   829,   194,   831,    70,  1575,
     134,   135,    70,   201,   400,   513,   201,   202,   404,   375,
    1229,    50,   846,   175,    53,    70,   861,    70,   384,    70,
     386,   201,   202,    70,   858,  1589,   355,   861,   202,   617,
     618,   713,   194,  1914,  1915,   162,   432,   403,   434,   435,
     436,   437,    70,  1300,    53,    54,    55,   199,    57,   846,
     199,   121,   122,   123,   124,   125,   126,  1300,  1910,  1911,
      69,   162,  1300,   429,    50,    51,    52,    53,    54,    55,
     904,  1494,   997,   998,   949,   166,   951,   201,   953,   445,
     391,   392,  1300,    69,    49,    69,  1509,   183,   162,   964,
      53,    54,    55,  1669,   199,   199,   162,   931,   162,   199,
     429,     8,   201,   162,   979,   199,    69,   904,    14,   791,
     439,   201,  1049,  1613,   443,   201,   162,   202,     9,   201,
      14,  1458,   967,   452,   194,   132,   132,   200,    14,  2036,
     103,   183,   983,   967,  1009,   967,   199,  1012,   967,  1014,
     200,   206,   200,   967,  1019,   202,  2053,   829,   112,   831,
      50,    51,    52,    53,    54,    55,   990,    57,  1033,   200,
    1470,  1036,  1472,  1038,   200,   199,   199,  1042,     9,    69,
     967,   159,    95,   200,   200,   200,   858,   506,   507,   508,
     509,   510,   511,   512,   200,     9,   201,    14,  1022,   183,
       9,   199,  1985,   990,   199,  1239,    83,  1990,  1242,   202,
    1012,   201,  1014,   532,  1434,  1435,  1436,  1019,  1050,   575,
    1633,   202,  1431,   647,   648,   202,  1091,  2006,  1641,   201,
     728,   201,  2015,   552,  1050,  1022,   202,   967,  1050,  1050,
    1098,   201,   200,   200,  1657,   564,   134,     9,   200,  2028,
     201,   675,   983,     9,   204,   204,   575,   204,  2037,   931,
     204,   204,   992,    70,  1099,    32,   997,   998,   135,   162,
    1597,  1598,   182,   138,     9,  1099,   595,  1101,   200,  1103,
     162,  1105,  1106,   200,    14,   196,   710,  1534,     9,     9,
    1937,   200,   184,  2076,  1024,  1942,     9,   713,    14,     9,
     200,  1534,   621,   622,  1229,  1229,  1534,  1229,   134,  1150,
     204,   200,   203,   200,  1101,   200,  1103,     9,   204,  1619,
      14,  1621,   200,  1623,  1737,   204,  1534,   200,  1628,   907,
     204,   909,   162,   911,   200,   654,   655,   915,   199,   917,
     918,   919,   103,   201,  1991,   201,     9,   138,   846,   162,
       9,   200,  1217,   199,   778,    70,    70,   713,    70,    70,
     199,  1092,    70,  1094,  1229,   199,   202,     9,   203,    14,
       9,   204,   728,   201,   184,   791,   176,  1212,   202,  1244,
     202,    14,  1247,  1630,   200,   196,   201,    70,  1212,    32,
    1212,   199,    32,  1212,   199,    14,   199,  1630,  1212,    52,
     199,   199,  1630,    70,    70,  1270,   904,   199,    70,   728,
      70,   199,    70,   829,   162,   831,  1281,   736,     9,  2066,
     200,  1245,  1630,   201,  1936,  1212,  1938,  1229,  1728,   201,
     199,  1985,   138,  1105,  1106,   791,  1990,    14,   184,   138,
     162,   865,   858,     9,  1318,   200,  1435,   871,   176,   176,
     204,    69,  1310,  1318,     9,    83,   203,     9,   203,   203,
     203,  2015,  1327,   201,   138,   199,    83,    14,   201,   967,
      83,   200,   204,   829,   199,   831,   202,  1890,  1313,   202,
     199,   138,  1212,  1307,   202,     9,    92,  1431,   159,  1313,
     846,   200,   990,    32,  1431,  1431,   201,  1321,    77,   201,
    1324,   201,   858,   200,   823,   861,   825,   184,   138,    32,
     200,   200,     9,   204,  2026,   931,   204,     9,   204,   943,
     204,     9,  2076,   204,  1022,  2039,   138,   846,   200,  2043,
     200,  1261,     9,   202,  1264,   203,   200,  1324,   203,    14,
     201,   860,   861,  2057,  2058,   201,   201,   201,   904,  1414,
     201,    83,   199,   199,   204,   199,  1421,  1229,  1576,   200,
       9,  1426,   200,  1428,   980,   200,  1431,   201,   204,   200,
     202,   200,     9,  1245,   138,   931,   138,   204,   204,  1444,
    1570,   204,   204,     9,   200,   904,   200,    32,  1888,  1889,
     201,   200,  1457,  1458,   913,   914,   200,   138,   201,   201,
     176,   202,   113,  1101,   113,  1103,   113,   113,   171,    83,
     113,   967,   201,   167,    83,  1193,    14,    83,   119,   200,
     200,   138,   138,    14,   202,   944,  1413,   200,   202,  1431,
     183,  1418,   201,    14,   990,  1307,    14,    14,   199,    83,
     200,   200,   198,   200,   138,   138,    14,    83,   967,  1321,
     201,  2064,   201,    14,    14,   201,  1012,   202,  1014,     9,
       9,   203,  1086,    68,   983,   183,  1022,  1691,    14,   823,
     199,   990,    83,     9,     9,   202,   201,   116,   997,   998,
     103,   162,   103,  1413,   184,   174,   199,   201,  1418,  1105,
    1106,    36,   200,   180,   199,    83,    78,    79,    80,    81,
     184,   184,   200,  1022,   177,  1129,     9,    83,    83,  1133,
     201,  1701,   202,    14,  1212,   200,  1140,    83,   200,    83,
      14,  1545,   104,    14,  1589,    83,  1456,    83,    14,  1594,
    1049,  1147,  1597,  1598,  1193,  1662,    83,  2018,   512,  1058,
    1059,  1060,   507,  1099,  1048,  1101,   509,  1103,  1043,  1105,
    1106,   970,  1319,  1736,  2033,  1506,  2028,  1246,  1545,   141,
     142,   143,   144,   145,  1724,  1589,   625,  1761,  1848,  1670,
    2074,  1565,  2050,  1092,  1720,  1094,   396,  1558,  1296,  1145,
    1099,  1216,  1101,  1279,  1103,  1059,  1141,   169,   170,  1576,
     172,   173,   174,  1292,  1291,    56,  1088,  1928,  1986,  1975,
     874,  1550,  1589,  1638,  1201,  1124,   443,  1125,  1179,    -1,
    1634,    -1,    -1,    -1,  1638,  1602,   198,    -1,  1642,    -1,
      -1,  1608,    -1,  1610,  1665,    -1,  1324,    -1,    -1,  1245,
      -1,  1150,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1866,    -1,    -1,    -1,  1711,  1634,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1642,  1212,    -1,    -1,    -1,
    1179,    -1,    -1,    -1,    -1,    -1,  1290,    -1,    -1,    -1,
      -1,    -1,  1602,  1229,    -1,    -1,    -1,    -1,  1608,    -1,
    1610,    -1,    -1,  1613,  1300,    -1,    -1,    -1,    -1,  1245,
      -1,  1307,    -1,  1212,    -1,    -1,    -1,    -1,    -1,    -1,
    1735,  1736,    -1,    -1,    -1,  1321,  1060,    -1,    -1,    -1,
    1734,  1735,  1736,    -1,    -1,    -1,  1740,    -1,    -1,    -1,
      -1,    -1,    -1,  1747,    -1,    81,    -1,  1351,    -1,    -1,
      -1,  1355,    -1,    -1,    -1,    -1,  1360,    -1,    -1,  1726,
      -1,    -1,    -1,    -1,  1368,    -1,    -1,  1734,   104,    -1,
      -1,  1307,    -1,  1740,    -1,    -1,    -1,  1313,  1860,    -1,
    1747,    -1,    -1,    -1,    -1,  1321,    -1,    -1,  1324,    -1,
      -1,    -1,    -1,  1292,  1293,  1294,  1295,  1296,  1297,    -1,
      -1,  1999,  1301,    -1,  1303,   141,   142,   143,   144,   145,
      -1,    -1,    -1,     6,  1313,    -1,  1726,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1324,    -1,    -1,  1873,    -1,
     166,    -1,     6,   169,   170,  1334,   172,   173,   174,  1860,
      -1,    -1,    -1,    -1,    -1,   286,  1928,   288,    -1,    -1,
      19,    20,  2022,    -1,    -1,    48,    -1,    -1,    -1,    -1,
      -1,    30,   198,    -1,    -1,    -1,   202,  1545,    -1,    -1,
      -1,    -1,    -1,  1477,    48,    -1,    -1,  1481,    -1,    -1,
      -1,    -1,    -1,    -1,  1488,    -1,    -1,    56,    -1,    -1,
    1894,  1569,    -1,    -1,    -1,  1431,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   347,  1928,  1407,    -1,
      -1,  1589,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,    -1,    -1,    -1,    -1,    -1,   119,  1894,   121,   122,
     123,   124,   125,   126,   127,    -1,    -1,    -1,  1534,   113,
      -1,    -1,    -1,    -1,    -1,   119,  1950,   121,   122,   123,
     124,   125,   126,   127,    -1,    -1,  1634,    -1,  1292,  1293,
    1294,  1295,  1296,  1297,  1642,    -1,    -1,  1301,    -1,  1303,
      -1,  1470,    -1,  1472,    -1,    -1,   169,   170,    -1,   172,
      -1,  1985,    -1,  1950,    -1,    -1,  1990,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   169,   170,    -1,   172,   440,
      -1,   194,   443,    -1,    -1,    -1,    -1,    -1,    -1,  1545,
     203,  2015,    -1,    -1,    -1,    -1,    -1,    -1,  1985,    -1,
     194,    -1,  1700,  1990,    -1,    -1,    -1,  2072,    -1,   203,
      -1,    -1,  1999,    -1,  1630,    -1,  2081,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  2089,    -1,  1545,  2092,  2015,    -1,
      -1,    -1,    -1,  1589,    -1,    -1,  1734,  1556,    -1,    -1,
      -1,    -1,  1740,    -1,    -1,    -1,  1565,    -1,    -1,  1747,
     239,  2075,  2076,    -1,   121,   122,   123,   124,   125,   126,
      -1,    -1,    -1,    -1,    -1,   132,   133,    -1,    -1,    -1,
    1589,    -1,    81,    -1,    -1,    -1,  2006,    -1,  1634,    -1,
      -1,    -1,  1638,    -1,    -1,    -1,  1642,    -1,  2075,  2076,
      -1,    -1,    -1,    -1,    -1,   104,    -1,   286,  2028,   288,
    1619,    -1,  1621,    -1,  1623,    -1,   173,  2037,   175,  1628,
      -1,    -1,    -1,    -1,    -1,  1634,  1470,    -1,  1472,  1638,
      -1,    -1,   189,  1642,   191,     6,    -1,   194,   589,    -1,
      -1,    -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,
       6,    -1,    -1,  1662,    -1,    -1,  1665,    -1,    -1,  1668,
      -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   347,  1678,
     169,   170,    -1,   172,   173,   174,  1685,    48,    -1,    -1,
      -1,    -1,    -1,    -1,  1693,  1694,    -1,  1696,  1734,  1735,
    1736,    -1,    48,  1702,  1740,    -1,   375,    -1,    -1,   198,
      -1,  1747,    -1,    -1,   203,   384,  1894,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1728,
      -1,    -1,    -1,    -1,   403,  1734,  1735,  1736,    -1,    -1,
      -1,  1740,   683,   684,    -1,   414,    -1,    -1,  1747,    -1,
      -1,    -1,   113,    -1,    -1,     6,    -1,    -1,   119,    -1,
     121,   122,   123,   124,   125,   126,   127,   113,    -1,    -1,
      -1,   440,  1950,   119,   443,   121,   122,   123,   124,   125,
     126,   127,    -1,    -1,    -1,  1619,    -1,  1621,    -1,  1623,
      -1,    81,    -1,    -1,  1628,    -1,    -1,    48,    -1,    -1,
    1904,  1905,    -1,    -1,    -1,    -1,    -1,  1985,   169,   170,
      -1,   172,  1990,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,     6,    -1,   169,   170,    -1,   172,    -1,    -1,    -1,
     499,    -1,    -1,   194,    -1,    -1,    -1,  2015,    -1,    -1,
      -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,   194,    -1,
      -1,   141,   142,   143,   144,   145,    -1,   203,  1894,    -1,
      -1,  1860,   113,    48,    -1,    -1,    -1,    -1,   119,    -1,
     121,   122,   123,   124,   125,   126,   127,   167,    -1,   169,
     170,   171,   172,   173,   174,  1884,    -1,    -1,    -1,  1888,
    1889,    -1,    -1,    -1,  1728,  1894,    -1,  2075,  2076,    -1,
      -1,    -1,    -1,    -1,  1903,    -1,    -1,    -1,   198,   199,
      -1,  1910,  1911,    -1,  1950,  1914,  1915,    -1,   169,   170,
     589,   172,   591,    -1,    -1,   594,    -1,    -1,   113,  1928,
      -1,    -1,   873,   874,   119,    -1,   121,   122,   123,   124,
     125,   126,   127,   194,    -1,    -1,    -1,    -1,    -1,  1985,
      -1,  1950,   203,    -1,  1990,    -1,    -1,    -1,   627,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   594,    -1,    -1,    -1,    -1,    -1,    -1,  2015,
      -1,    -1,    -1,    -1,   169,   170,  1985,   172,    -1,    -1,
      -1,  1990,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1998,
      -1,    -1,    -1,    -1,    -1,   627,    -1,    -1,    -1,   194,
      -1,    -1,    -1,    -1,   683,   684,  2015,    -1,   203,    -1,
      -1,    -1,  2021,   692,    -1,    -1,    -1,    -1,   969,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2075,
    2076,    -1,    -1,    -1,   713,    -1,   987,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1888,  1889,    12,    -1,    -1,  1000,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    31,  2075,  2076,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      10,    11,    12,    31,    -1,  1046,    59,    60,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,   791,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    81,   823,    -1,    -1,    -1,    -1,    69,
     829,    -1,   831,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,  1119,    -1,
      -1,    -1,  1123,   136,   137,    -1,    -1,  1128,    -1,   858,
     859,    -1,    -1,    -1,    -1,    -1,    -1,   866,    -1,   127,
      -1,    -1,    31,    -1,   873,   874,   875,   876,   877,   878,
     879,    -1,   140,   141,   142,   143,   144,   145,   146,   888,
      -1,    -1,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,   163,   905,   859,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,   200,    69,    -1,
      -1,    -1,    81,   875,   876,   877,   878,   879,    -1,    -1,
      -1,    -1,   931,    92,    -1,    -1,   888,    -1,    -1,    -1,
     198,    -1,    -1,    -1,    -1,   104,   945,    -1,    -1,    78,
      79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1230,
      -1,    19,    20,    92,   204,    -1,    -1,    -1,    -1,    -1,
     969,   970,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   980,   141,   142,   143,   144,   145,    -1,   987,    -1,
      -1,    -1,    -1,    -1,    -1,   994,    -1,    -1,    -1,    -1,
      -1,  1000,  1273,    -1,   163,  1276,    -1,   166,    -1,    -1,
     169,   170,  1011,   172,   173,   174,    -1,   176,   147,   148,
     149,   150,   151,    -1,    -1,    -1,    -1,  1026,    -1,   158,
      -1,    -1,    -1,    -1,    -1,   164,   165,    -1,    -1,   198,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1046,    -1,   178,
      -1,  1050,    10,    11,    12,    -1,    -1,    -1,    -1,  1011,
      -1,  1060,  1333,    -1,   193,    -1,    -1,    -1,    -1,  1340,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,  1105,  1106,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1119,    -1,    -1,    -1,  1123,    -1,  1125,    -1,    -1,  1128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1408,  1409,    -1,
      -1,    -1,  1141,  1142,  1143,  1144,  1145,  1146,  1147,  1148,
      -1,    -1,  1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,
    1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
      -1,   239,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1141,
    1142,    -1,    -1,  1145,    -1,    -1,    -1,    -1,  1197,  1151,
    1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,
    1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,  1178,    -1,    10,    11,
      12,  1230,    -1,  1232,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   200,  1514,  1515,  1197,  1245,  1518,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,  1273,    57,    -1,  1276,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1556,    -1,    69,    -1,    81,
      -1,    -1,    -1,  1292,  1293,  1294,  1295,  1296,  1297,  1570,
      -1,  1300,  1301,    -1,  1303,    -1,    -1,    -1,  1307,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,   375,    -1,    -1,
      -1,    -1,  1321,    -1,  1323,    -1,   384,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1333,    -1,    -1,    -1,   130,    -1,
      -1,  1340,    -1,    -1,    -1,   403,  1345,    -1,  1347,   141,
     142,   143,   144,   145,    -1,    -1,   414,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,  1323,  1643,  1372,    -1,    -1,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1345,    -1,  1347,    -1,  1668,    -1,    -1,
      -1,    -1,    59,    60,    -1,    -1,   198,   199,    -1,  1408,
    1409,    -1,    -1,  1412,    81,    -1,    -1,    -1,    -1,   201,
    1372,   203,  1693,  1694,    -1,  1696,    -1,    -1,    -1,    81,
    1701,  1702,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
    1439,   499,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,  1470,    -1,  1472,   141,   142,   143,   144,   145,   136,
     137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1759,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,
      59,    60,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1514,  1515,   169,   170,  1518,
     172,   173,   174,    -1,    -1,  1524,    -1,  1526,    -1,  1528,
      -1,   198,   199,    -1,  1533,  1534,   594,    -1,    -1,  1538,
      -1,  1540,    -1,   200,  1543,    -1,   198,   199,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1556,  1557,    -1,
      -1,    -1,    -1,    -1,  1563,    -1,    -1,    -1,    -1,   627,
      -1,  1570,  1524,    -1,  1526,    81,  1528,   136,   137,    -1,
      -1,  1533,    -1,    -1,    -1,    -1,  1538,    -1,  1540,    -1,
      -1,  1543,    -1,    -1,  1865,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,   113,    -1,  1880,
      -1,    -1,    -1,  1884,    -1,    -1,    -1,    -1,    -1,    81,
    1619,    -1,  1621,    -1,  1623,    -1,    -1,    -1,    -1,  1628,
      -1,  1630,    -1,    -1,    -1,   141,   142,   143,   144,   145,
      -1,   200,   104,    -1,  1643,    -1,    -1,    -1,    -1,  1648,
      -1,    -1,    -1,    -1,    -1,   713,    -1,    -1,    -1,    -1,
     166,  1660,  1661,   169,   170,   127,   172,   173,   174,  1668,
      -1,  1670,    -1,    -1,   286,    -1,   288,    -1,   140,   141,
     142,   143,   144,   145,   146,    -1,    -1,    -1,  1959,    -1,
      -1,    -1,   198,    -1,  1693,  1694,  1648,  1696,    -1,    -1,
      -1,   163,  1701,  1702,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,  1986,    -1,  1988,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1728,
      -1,    -1,    -1,   791,    -1,   347,   198,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1744,  1745,  1746,    -1,    -1,
      -1,    -1,  1751,    -1,  1753,    -1,    -1,    -1,    -1,    -1,
    1759,    -1,  1761,    -1,    -1,   823,    -1,    -1,    -1,    -1,
      -1,   829,    -1,   831,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,  1744,  1745,  1746,    -1,    -1,    -1,    -1,  1751,
     858,   859,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   875,   876,   877,
     878,   879,    59,    60,    -1,    -1,    -1,    -1,   440,    -1,
     888,   443,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,   905,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,  1857,    -1,
      -1,    -1,    -1,    -1,    -1,    92,  1865,    -1,    -1,    -1,
      -1,    -1,    -1,   931,    -1,    -1,    -1,   104,    59,    60,
      -1,  1880,    -1,    -1,    -1,  1884,    -1,   945,    -1,  1888,
    1889,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,   136,
     137,    -1,    -1,    -1,  1903,  1857,    -1,    -1,    -1,    -1,
    1909,    -1,   970,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,   980,    -1,    -1,    -1,  1925,  1926,    -1,    -1,
      -1,    68,    -1,  1932,    -1,    -1,   163,    -1,    -1,   166,
      -1,    -1,   169,   170,    81,   172,   173,   174,    -1,   176,
      87,    -1,    -1,  1011,    -1,   136,   137,  1909,    -1,    -1,
    1959,    -1,    -1,   200,    -1,    -1,    -1,   104,  1026,    -1,
      -1,   198,    -1,  1925,  1926,  1974,    -1,   589,    -1,   591,
    1932,    -1,    -1,    -1,    -1,    -1,    -1,  1986,    -1,  1988,
      -1,    -1,  1050,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1060,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2017,   200,
      -1,    -1,  1974,    -1,    -1,    -1,   163,    -1,    -1,   166,
     167,    -1,   169,   170,  2033,   172,   173,   174,    19,    20,
    2039,    -1,    -1,    -1,  2043,    -1,    -1,  1105,  1106,    30,
     187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2057,  2058,
      -1,   198,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   683,   684,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     692,    -1,    -1,  1141,  1142,  1143,  1144,  1145,  1146,  1147,
    1148,    -1,    -1,  1151,  1152,  1153,  1154,  1155,  1156,  1157,
    1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1197,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,  1245,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,  1292,  1293,  1294,  1295,  1296,  1297,
      -1,    -1,  1300,  1301,    -1,  1303,    -1,    -1,    -1,  1307,
      -1,    -1,    -1,    -1,   866,    -1,    -1,    -1,   239,    -1,
      -1,   873,   874,  1321,    -1,  1323,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1345,    81,  1347,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    59,    60,  1372,   203,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,    -1,  1412,    69,    -1,   969,    -1,    -1,
      -1,    -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,
     163,    -1,    -1,   166,    -1,   987,   169,   170,    -1,   172,
     173,   174,   994,    -1,    -1,    -1,    -1,    -1,  1000,   136,
     137,    -1,    -1,    -1,   375,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   384,    -1,   198,    -1,    -1,    -1,    -1,
      -1,    -1,  1470,    -1,  1472,    10,    11,    12,    -1,    -1,
      -1,    -1,   403,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   414,  1046,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,  1524,    -1,  1526,    -1,
    1528,    -1,    -1,    -1,    69,  1533,  1534,    -1,    -1,    -1,
    1538,    -1,  1540,    -1,    -1,  1543,    -1,    -1,    -1,   203,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,  1557,
      -1,    10,    11,    12,    -1,  1563,    -1,  1119,    -1,    -1,
      -1,  1123,    -1,  1125,    -1,   104,  1128,    -1,   499,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,
      69,  1619,    -1,  1621,    -1,  1623,    -1,    -1,    -1,    -1,
    1628,    -1,  1630,    -1,   163,    -1,    -1,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    81,    -1,    83,
    1648,    85,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,  1660,  1661,    -1,    -1,    -1,    -1,   203,   198,
     104,    -1,  1670,   594,    -1,    -1,    -1,    -1,  1230,    31,
    1232,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,   627,   141,   142,   143,
     144,   145,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,  1273,    -1,    -1,  1276,    -1,    -1,    -1,    -1,    -1,
    1728,    -1,    -1,    -1,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,    -1,    -1,  1744,  1745,  1746,    -1,
      -1,    -1,    -1,  1751,   203,  1753,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1761,   198,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,  1333,    -1,    -1,    -1,    -1,    -1,    -1,  1340,    -1,
      -1,    -1,   713,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,   200,    -1,
      -1,    78,    79,    80,    81,    -1,  1408,  1409,    -1,  1857,
      -1,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,    -1,
     791,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1439,    -1,    81,
    1888,  1889,    -1,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,   829,    -1,
     831,  1909,   104,   140,   141,   142,   143,   144,   145,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1925,  1926,    -1,
      -1,   158,    -1,    -1,  1932,    -1,   163,   858,   859,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,    -1,   141,
     142,   143,   144,   145,   875,   876,   877,   878,   879,    -1,
     187,   188,  1514,  1515,    -1,    -1,  1518,   888,   203,    -1,
      -1,   198,    -1,    -1,    -1,   167,  1974,   169,   170,    -1,
     172,   173,   174,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,  1556,    57,   198,    -1,    -1,    -1,
     931,    -1,    -1,    10,    11,    12,    -1,    69,  1570,  2017,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,  2033,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,   980,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    69,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    31,
    1011,  1643,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,  1668,    -1,    -1,    -1,
      59,    60,    -1,    -1,    -1,    -1,    -1,    69,    -1,  1050,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,  1693,  1694,    -1,  1696,    -1,    -1,    -1,    -1,  1701,
    1702,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,  1105,  1106,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,   138,   136,   137,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   203,  1759,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1141,  1142,  1143,  1144,  1145,  1146,  1147,  1148,    -1,    -1,
    1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,
    1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,    -1,    10,
      11,    12,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,  1197,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    59,    60,    -1,
      -1,    -1,    -1,  1865,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,  1245,   203,    -1,    -1,  1880,    10,
      11,    12,  1884,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,  1903,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,  1300,
      -1,    -1,    -1,    -1,   136,   137,  1307,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1321,    -1,  1323,    -1,    -1,    -1,    -1,  1959,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1345,    -1,  1347,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1986,    -1,  1988,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1372,   203,    -1,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,  2039,    -1,    -1,
      -1,  2043,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,  2057,  2058,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   203,    -1,    -1,    -1,   104,    -1,    -1,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,   166,    -1,
      -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
      27,    28,    29,  1524,    -1,  1526,    -1,  1528,    -1,    -1,
      -1,    -1,  1533,  1534,    -1,    -1,    -1,  1538,    -1,  1540,
     198,    48,  1543,    50,    51,    -1,    -1,    -1,    -1,    56,
     203,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,
      97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,    -1,   109,   110,   111,   112,   113,   114,   115,    -1,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     201,   128,   129,   130,   131,   132,   133,    -1,    -1,  1630,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,  1648,   155,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,   188,   189,    -1,   191,    -1,   193,   194,   195,    -1,
      -1,   198,   199,    -1,   201,   202,    -1,    -1,   205,   206,
     207,   208,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1744,  1745,  1746,    -1,    -1,    -1,    -1,
    1751,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,  1760,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,
      -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,
      -1,    -1,    -1,   109,   110,   111,   112,   113,   114,   115,
      -1,   117,    -1,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,   129,   130,   131,   132,   133,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,  1857,    -1,    -1,   155,
      -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,   175,
      -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,
      -1,   187,   188,   189,    -1,   191,    -1,   193,   194,   195,
      -1,    -1,   198,   199,    -1,   201,   202,   203,  1909,   205,
     206,   207,   208,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,     5,     6,     7,  1925,  1926,    -1,    -1,    -1,    13,
      -1,  1932,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1961,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,  1974,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,
      94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,
     104,   105,    -1,    -1,    -1,   109,   110,   111,   112,   113,
     114,   115,    -1,   117,    -1,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,   129,   130,   131,   132,   133,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,    -1,   147,   148,   149,   150,   151,    -1,    -1,
      -1,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,   175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,
      -1,    -1,    -1,   187,   188,   189,    -1,   191,    -1,   193,
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
      -1,   117,   118,    -1,   120,   121,   122,   123,   124,   125,
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
      96,    97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,
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
      -1,   101,   102,    -1,   104,   105,    -1,    -1,    -1,   109,
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
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    77,
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
      -1,    97,    -1,    99,   100,   101,    -1,    -1,   104,   105,
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
      -1,    -1,    92,    93,    94,    95,    -1,    97,    98,    99,
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
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
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
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,
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
     201,    -1,    -1,    -1,   205,   206,   207,   208,    -1,    -1,
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
       3,     4,     5,     6,     7,   187,   188,    -1,    -1,    -1,
      13,   193,   194,   195,    -1,    -1,   198,   199,    -1,    -1,
      -1,    -1,    -1,   205,   206,   207,   208,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    87,    -1,    -1,    -1,    -1,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,
     123,   124,   125,   126,    -1,    -1,   129,   130,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,   176,    -1,   178,    -1,    -1,   181,    -1,
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
      -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,
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
     104,    -1,    -1,    -1,    -1,   109,    -1,    -1,   112,    -1,
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
      -1,    -1,   198,   199,    -1,   201,    -1,    -1,    -1,   205,
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
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,
      -1,   198,   199,    -1,   201,    -1,    -1,    -1,   205,   206,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,
      -1,   193,   194,   195,    -1,    -1,   198,   199,   200,    -1,
      -1,    -1,    -1,   205,   206,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
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
     206,   207,   208,    -1,    -1,    32,    -1,    -1,    -1,    -1,
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
     181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,
      -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,     3,
       4,     5,     6,     7,   205,   206,   207,   208,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   121,   122,   123,   124,   125,
     126,    -1,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,
      -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,    -1,
       3,     4,   178,     6,     7,   181,    -1,    10,    11,    12,
      13,   187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,
      -1,    -1,   198,   199,    -1,    28,    29,    -1,    -1,   205,
     206,   207,   208,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      83,    84,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
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
     145,    -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,   202,
      -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    83,    84,    -1,    -1,    87,    -1,
      -1,    -1,    -1,   198,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
      -1,   130,    -1,   132,   133,   134,   135,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,   146,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,   163,    10,    11,    12,    13,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,   177,    -1,
      -1,   180,    -1,    28,    29,    -1,    31,    -1,    -1,   188,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,
      -1,    -1,    -1,   202,    -1,    -1,   205,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    93,    94,
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
     143,   144,   145,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    57,    57,    -1,    -1,    -1,   169,   170,    -1,   172,
     173,   174,    68,    -1,    69,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    87,    -1,    -1,    -1,   198,    -1,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    -1,   130,    -1,    -1,   133,   134,   135,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
     146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,   175,
      -1,   177,    -1,    -1,   180,    -1,     3,     4,    -1,     6,
       7,   187,   188,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,   198,   199,    -1,    -1,    -1,   203,    -1,    -1,
      -1,    28,    29,    -1,    31,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      57,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    69,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,    -1,   130,    -1,   132,   133,   134,   135,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
     177,    -1,    -1,   180,    -1,     3,     4,    -1,     6,     7,
     187,   188,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   198,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    29,    -1,    31,    31,    -1,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    69,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    -1,   130,    -1,    -1,   133,   134,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,   177,
      -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,    -1,   187,
     188,   189,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
     198,   199,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    29,    -1,    31,    -1,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    57,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    69,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    -1,   130,    -1,    -1,   133,   134,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,   177,
      -1,    -1,   180,    -1,     3,     4,     5,     6,     7,   187,
     188,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
     198,   199,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    57,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   163,   164,   165,    -1,    -1,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,   177,   178,
      -1,   180,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,
     189,    -1,   191,    -1,   193,   194,    -1,     3,     4,   198,
       6,     7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    29,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    87,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    -1,   130,    -1,   132,   133,   134,   135,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
     146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,   163,    10,    11,
      12,    13,    -1,   169,   170,    -1,   172,   173,   174,   175,
      -1,   177,    -1,    -1,   180,    -1,    28,    29,    -1,    -1,
      -1,    -1,   188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
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
     128,    -1,   130,    -1,    -1,   133,   134,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,   177,
      -1,    -1,   180,    10,    11,    12,    -1,    -1,    -1,    -1,
     188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     198,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,   201,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   201,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
     201,    57,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    81,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,   201,
      69,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,   124,
     125,   126,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   201,   163,    68,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    81,    -1,    83,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,    -1,   194,
      -1,    -1,    -1,   198,   199,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,   200,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   121,   122,   123,   124,   125,   126,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    81,   163,    -1,    68,   166,   167,    87,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    87,   104,    -1,   187,    -1,
      -1,    -1,    -1,    -1,    -1,   194,    -1,    -1,    -1,   198,
     199,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,   146,   163,    68,    -1,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,    31,   176,    81,
      -1,   163,    -1,    -1,   166,   167,    -1,   169,   170,   187,
     172,   173,   174,    -1,   176,    -1,    -1,    -1,    -1,    -1,
     198,   199,   104,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     112,    -1,    -1,    -1,    68,    -1,   198,   199,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   163,    68,    -1,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   187,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,   198,   199,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,   163,
      -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    -1,   187,   140,   141,   142,   143,   144,   145,
     146,    -1,    -1,    70,   198,   199,    -1,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    -1,    -1,   163,    -1,    -1,
     166,   167,    -1,   169,   170,    92,   172,   173,   174,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,   187,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,   198,   199,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,   140,   141,   142,   143,   144,   145,    70,
     147,   148,   149,   150,   151,    -1,    69,    78,    79,    80,
      81,   158,    83,    84,    -1,    -1,   163,   164,   165,   166,
     167,    92,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,   178,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
     187,   188,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,
     121,   198,   199,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    70,   147,   148,   149,   150,
     151,    -1,    -1,    78,    79,    80,    81,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    92,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   193,    -1,    -1,    -1,    -1,   198,   199,    -1,
      -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,
     145,    70,   147,   148,   149,   150,   151,    -1,    -1,    78,
      79,    80,    81,   158,    83,    84,    -1,    -1,   163,   164,
     165,   166,   167,    92,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,   178,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,    -1,
      -1,    -1,   121,   198,   199,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    70,    71,    -1,   178,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,    -1,   193,    -1,    -1,    -1,    92,   198,
     199,    -1,    -1,   202,    -1,    -1,   205,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,   139,   140,   141,   142,   143,
     144,   145,    70,   147,   148,   149,   150,   151,    69,    -1,
      78,    79,    80,    81,   158,    83,    84,    -1,    -1,   163,
     164,   165,   166,   167,    92,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,   178,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,
      -1,    -1,    -1,   121,   198,   199,    -1,    -1,    -1,    -1,
      -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,   147,
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
      48,    49,    50,    51,    52,    53,    54,    55,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,    -1,
      -1,   121,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    69,    -1,    -1,    -1,    -1,   158,    -1,
     138,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,   193,    92,    -1,    -1,    -1,   198,   199,
      -1,    -1,    -1,    -1,    -1,   205,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   121,    -1,    81,    -1,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   104,   147,
     148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,    81,
     158,    83,    84,    -1,    -1,   163,   164,   165,   166,   167,
      -1,   169,   170,    81,   172,   173,   174,    -1,    -1,    -1,
     178,    -1,   104,    -1,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,    -1,   193,   104,    -1,    -1,    -1,
     198,   199,    -1,    -1,   112,   113,    -1,   205,    -1,    -1,
      -1,   167,    -1,   169,   170,    -1,   172,   173,   174,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,    -1,    -1,
      -1,    -1,   198,    -1,    -1,   167,   202,   169,   170,   205,
     172,   173,   174,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,   169,   170,    -1,   172,   173,   174,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,
     202,   189,    -1,   205,    -1,    27,    -1,    -1,    30,    31,
     198,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    30,    31,    32,    33,    34,    35,    36,    37,
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
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69
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
     270,   276,   335,   336,   344,   345,   348,   349,   350,   351,
     352,   353,   354,   355,   357,   358,   359,   361,   364,   376,
     377,   384,   387,   390,   393,   396,   399,   405,   407,   408,
     410,   420,   421,   422,   424,   429,   434,   454,   462,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   488,   490,   492,   121,   122,   123,   163,   173,
     199,   216,   257,   335,   357,   377,   466,   357,   199,   357,
     357,   357,   357,   109,   357,   357,   452,   453,   357,   357,
     357,   357,    81,    83,    92,   121,   141,   142,   143,   144,
     145,   158,   199,   227,   377,   421,   424,   429,   466,   470,
     466,   357,   357,   357,   357,   357,   357,   357,   357,    38,
     357,   480,   121,   199,   227,   421,   422,   423,   425,   429,
     463,   464,   465,   474,   478,   479,   357,   199,   356,   426,
     199,   356,   368,   346,   357,   238,   356,   199,   199,   199,
     356,   201,   357,   216,   201,   357,     3,     4,     6,     7,
      10,    11,    12,    13,    28,    29,    31,    57,    68,    71,
      72,    73,    74,    75,    76,    77,    87,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   130,   132,   133,   134,   135,   139,   140,   146,
     163,   167,   175,   177,   180,   187,   188,   199,   216,   217,
     218,   229,   493,   514,   515,   518,    27,   201,   351,   353,
     357,   202,   250,   357,   112,   113,   163,   166,   189,   219,
     220,   221,   222,   226,    83,   205,   303,   304,    83,   305,
     123,   132,   122,   132,   199,   199,   199,   199,   216,   274,
     496,   199,   199,    70,    70,    70,    70,    70,   346,    83,
      91,   159,   160,   161,   485,   486,   166,   202,   226,   226,
     216,   275,   496,   167,   199,   199,   496,   496,    83,   195,
     202,   369,    28,   345,   348,   357,   359,   466,   471,   233,
     202,   476,    91,   427,   485,    91,   485,   485,    32,   166,
     183,   497,   199,     9,   201,   199,   344,   358,   467,   470,
     118,    38,   256,   167,   273,   496,   121,   194,   257,   336,
      70,   202,   461,   201,   201,   201,   201,   201,   201,   201,
     201,    10,    11,    12,    30,    31,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    57,
      69,   201,    14,    70,    70,   202,   162,   133,   173,   175,
     189,   191,   276,   334,   335,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    59,    60,
     136,   137,   456,   461,   461,   199,   199,    70,   202,   256,
     257,    14,    14,   357,   201,   138,    49,   216,   451,    91,
     345,   359,   162,   466,   138,     9,   204,   345,   359,   466,
     497,   162,   199,   428,   456,   461,   200,   357,    32,   236,
       8,   370,     9,   201,   236,   237,   346,   347,   357,   216,
     288,   240,   201,   201,   201,   140,   146,   518,   518,   183,
     517,   199,   112,   518,    14,   162,   140,   146,   163,   216,
     218,   201,   201,   201,   251,   116,   180,   201,   219,   221,
     219,   221,   219,   221,   226,   219,   221,   202,     9,   437,
     201,   103,   166,   202,   466,     9,   201,    14,     9,   201,
     132,   132,   466,   489,   346,   345,   359,   466,   470,   471,
     200,   183,   268,   480,   480,   357,   378,   379,   346,   402,
     402,   378,   402,   201,    70,   456,   159,   486,    82,   357,
     466,    91,   159,   486,   226,   215,   201,   202,   263,   271,
     411,   413,    92,   199,   371,   372,   374,   420,   424,   473,
     475,   490,   402,    14,   103,   491,   365,   366,   367,   298,
     299,   454,   455,   200,   200,   200,   200,   200,   203,   235,
     236,   258,   265,   270,   454,   357,   206,   207,   208,   216,
     498,   499,   518,    38,    87,   176,   301,   302,   357,   493,
     247,   248,   345,   353,   354,   357,   359,   466,   202,   249,
     249,   249,   249,   199,   496,   266,   256,   357,   477,   357,
     357,   357,   357,   357,    32,   357,   357,   357,   357,   357,
     357,   357,   357,   357,   357,   357,   357,   357,   357,   357,
     357,   357,   357,   357,   357,   357,   357,   357,   357,   425,
     357,   348,   353,   357,   477,   477,   357,   481,   482,   132,
     202,   217,   218,   476,   274,   216,   275,   496,   496,   273,
     257,    38,   348,   351,   353,   357,   357,   357,   357,   357,
     357,   357,   357,   357,   357,   357,   357,   357,   167,   202,
     216,   457,   458,   459,   460,   476,   301,   301,   477,   357,
     256,   200,   357,   199,   450,     9,   436,   200,   200,    38,
     357,    38,   357,   200,   200,   200,   474,   475,   476,   301,
     202,   216,   457,   458,   476,   200,   233,   292,   202,   353,
     357,   357,    95,    32,   236,   286,   201,    27,   103,    14,
       9,   200,    32,   202,   289,   518,    31,    92,   176,   229,
     511,   512,   513,   199,     9,    50,    51,    56,    58,    70,
     140,   141,   142,   143,   144,   145,   187,   188,   199,   227,
     385,   388,   391,   394,   397,   400,   406,   421,   429,   430,
     432,   433,   216,   516,   233,   199,   244,   202,   201,   202,
     201,   202,   201,   103,   166,   202,   201,   112,   113,   166,
     222,   223,   224,   225,   226,   222,   216,   357,   304,   430,
      83,     9,   200,   200,   200,   200,   200,   200,   200,   201,
      50,    51,   507,   509,   510,   134,   279,   200,   200,   138,
     204,     9,   436,     9,   436,   204,   204,   204,   204,    83,
      85,   216,   487,   216,    70,   203,   203,   212,   214,    32,
     135,   278,   182,    54,   167,   182,   202,   415,   359,   138,
       9,   436,   200,   162,   200,   518,   518,    14,   370,   298,
     231,   196,     9,   437,    87,   518,   519,   456,   456,   203,
       9,   436,   184,   466,    83,    84,   300,   357,   200,     9,
     437,    14,     9,   200,     9,   200,   200,   200,   200,    14,
     200,   203,   234,   235,   362,   259,   134,   277,   199,   496,
     204,   203,   357,    32,   204,   204,   138,   203,     9,   436,
     357,   497,   199,   269,   264,   272,    14,   491,   267,   256,
      71,   466,   357,   497,   200,   200,   204,   203,    50,    51,
      70,    78,    79,    80,    92,   140,   141,   142,   143,   144,
     145,   158,   187,   188,   216,   386,   389,   392,   395,   398,
     401,   421,   432,   439,   441,   442,   446,   449,   216,   466,
     466,   138,   456,   461,   456,   200,   357,   293,    75,    76,
     294,   231,   356,   233,   347,   103,    38,   283,   377,   466,
     430,   216,    32,   236,   287,   201,   290,   201,   290,     9,
     436,    92,   229,   138,   162,     9,   436,   200,    87,   500,
     501,   518,   519,   498,   430,   430,   430,   430,   430,   435,
     438,   199,    70,    70,    70,    70,    70,   199,   199,   430,
     162,   202,    10,    11,    12,    31,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    69,   162,
     497,   203,   421,   202,   253,   221,   221,   221,   216,   221,
     222,   222,   226,     9,   437,   203,   203,    14,   466,   201,
     184,     9,   436,   216,   280,   421,   202,   357,   357,   204,
     357,   203,   212,   518,   280,   202,   414,   176,    14,   200,
     357,   371,   476,   201,   518,   196,   203,   232,   235,   245,
      32,   505,   455,   519,    38,    83,   176,   457,   458,   460,
     457,   458,   460,   518,    70,    38,    87,   176,   357,   430,
     248,   353,   354,   466,   249,   248,   249,   249,   203,   235,
     298,   199,   421,   278,   363,   260,   357,   357,   357,   203,
     199,   301,   279,    32,   278,   518,    14,   277,   496,   425,
     203,   199,    78,    79,    80,   216,   440,   440,   442,   444,
     445,    52,   199,    70,    70,    70,    70,    70,    91,   159,
     199,   199,   162,     9,   436,   200,   450,    38,   357,   203,
      75,    76,   295,   356,   236,   203,   201,    96,   201,   283,
     466,   138,   282,    14,   233,   290,   106,   107,   108,   290,
     203,   518,   184,   138,   162,   518,   216,   176,   511,   518,
       9,   436,   200,   176,   436,   138,   204,     9,   436,   435,
     380,   381,   430,   403,   430,   431,   403,   380,   403,   371,
     373,   375,   403,   200,   132,   217,   430,   483,   484,   430,
     430,   430,    32,   430,   430,   430,   430,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   430,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   430,   516,    83,   254,
     203,   203,   203,   203,   225,   201,   430,   510,   103,   104,
     506,   508,     9,   309,   138,   204,   203,   491,   309,   168,
     181,   202,   410,   417,   357,   168,   202,   416,   138,   201,
     505,   199,   248,   344,   358,   467,   470,   518,   370,    87,
     519,    83,    83,   176,    14,    83,   497,   497,   477,   466,
     300,   357,   200,   298,   202,   298,   199,   138,   199,   301,
     200,   202,   518,   202,   201,   518,   278,   261,   428,   301,
     138,   204,     9,   436,   441,   444,   382,   383,   442,   404,
     442,   443,   404,   382,   404,   159,   371,   447,   448,   404,
      81,   442,   466,   356,    32,    77,   236,   201,   347,   282,
     283,   200,   430,   102,   106,   201,   357,    32,   201,   291,
     203,   184,   518,   216,   138,    87,   518,   519,    32,   200,
     430,   430,   200,   204,     9,   436,   138,   204,     9,   436,
     204,   204,   204,   138,     9,   436,   200,   200,   138,   203,
       9,   436,   430,    32,   200,   233,   201,   201,   201,   201,
     216,   518,   518,   506,   421,     6,   113,   119,   122,   124,
     125,   126,   127,   169,   170,   172,   203,   310,   333,   334,
     335,   338,   340,   341,   342,   343,   454,   357,   203,   202,
     203,    54,   357,   203,   357,   357,   370,   466,   201,   202,
     519,    38,    83,   176,    14,    83,   357,   199,   199,   204,
     505,   200,   309,   200,   298,   357,   301,   200,   309,   491,
     309,   201,   202,   199,   200,   442,   442,   200,   204,     9,
     436,   138,   204,     9,   436,   204,   204,   204,   138,   200,
       9,   436,   200,    32,   233,   201,   200,   200,   241,   201,
     201,   291,   233,   138,   518,   518,   176,   518,   138,   430,
     430,   430,   430,   371,   430,   430,   430,   202,   203,   508,
     134,   135,   189,   217,   494,   518,   281,   421,   113,   343,
      31,   127,   140,   146,   167,   173,   317,   318,   319,   320,
     421,   171,   325,   326,   130,   199,   216,   327,   328,    83,
     339,   257,   518,   113,     9,   201,     9,   201,   201,   491,
     334,   335,   306,   167,   412,   203,   203,   357,    83,    83,
     176,    14,    83,   357,   301,   301,   119,   360,   505,   203,
     505,   200,   200,   203,   202,   203,   309,   298,   138,   442,
     442,   442,   442,   371,   233,   239,   242,    32,   236,   285,
     233,   518,   200,   430,   138,   138,   138,   233,   421,   421,
     496,    14,   217,     9,   201,   202,   494,   491,   320,   183,
     202,     9,   201,     3,     4,     5,     6,     7,    10,    11,
      12,    13,    27,    28,    29,    57,    71,    72,    73,    74,
      75,    76,    77,    87,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   139,   140,   147,   148,
     149,   150,   151,   163,   164,   165,   175,   177,   178,   180,
     187,   189,   191,   193,   194,   216,   418,   419,     9,   201,
     167,   171,   216,   328,   329,   330,   201,    14,     9,   201,
     256,   339,   494,   494,   494,    14,   257,   339,   518,   203,
     307,   308,   494,    14,    83,   357,   200,   200,   199,   505,
     198,   502,   360,   505,   306,   203,   200,   442,   138,   138,
      32,   236,   284,   285,   233,   430,   430,   430,   203,   201,
     201,   430,   421,   313,   518,   321,   322,   429,   318,    14,
      32,    51,   323,   326,     9,    36,   200,    31,    50,    53,
     430,    83,   218,   495,   201,    14,    14,   518,   256,   201,
     339,   201,    14,   357,    38,    83,   409,   202,   503,   504,
     518,   201,   202,   331,   505,   502,   203,   505,   442,   442,
     233,   100,   252,   203,   216,   229,   314,   315,   316,     9,
     436,     9,   436,   203,   430,   419,   419,    68,   324,   329,
     329,    31,    50,    53,    14,   183,   199,   430,   430,   495,
     201,   430,    83,     9,   437,   231,     9,   437,    14,   506,
     231,   202,   331,   331,    98,   201,   116,   243,   162,   103,
     518,   184,   429,   174,   430,   507,   311,   199,    38,    83,
     200,   203,   504,   518,   203,   231,   201,   199,   180,   255,
     216,   334,   335,   184,   184,   296,   297,   455,   312,    83,
     203,   421,   253,   177,   216,   201,   200,     9,   437,    87,
     124,   125,   126,   337,   338,   296,    83,   281,   201,   505,
     455,   519,   519,   200,   200,   201,   502,    87,   337,    83,
      38,    83,   176,   505,   202,   201,   202,   332,   519,   519,
      83,   176,    14,    83,   502,   233,   231,    83,    38,    83,
     176,    14,    83,   357,   332,   203,   203,    83,   176,    14,
      83,   357,    14,    83,   357,   357
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
     269,   265,   271,   270,   272,   270,   273,   273,   274,   275,
     276,   276,   276,   276,   276,   277,   277,   278,   278,   279,
     279,   280,   280,   281,   281,   282,   282,   283,   283,   283,
     284,   284,   285,   285,   286,   286,   287,   287,   288,   288,
     289,   289,   289,   289,   290,   290,   290,   291,   291,   292,
     292,   293,   293,   294,   294,   295,   295,   296,   296,   296,
     296,   296,   296,   296,   296,   297,   297,   297,   297,   297,
     297,   297,   297,   297,   297,   298,   298,   298,   298,   298,
     298,   298,   298,   299,   299,   299,   299,   299,   299,   299,
     299,   299,   299,   300,   300,   300,   301,   301,   302,   302,
     302,   302,   302,   302,   302,   302,   303,   303,   304,   304,
     304,   305,   305,   305,   305,   306,   306,   307,   308,   309,
     309,   310,   310,   310,   310,   310,   310,   310,   311,   310,
     312,   310,   310,   310,   310,   310,   310,   310,   310,   313,
     313,   313,   314,   315,   315,   316,   316,   317,   317,   318,
     318,   319,   319,   320,   320,   320,   320,   320,   320,   320,
     321,   321,   322,   323,   323,   324,   324,   325,   325,   326,
     327,   327,   327,   328,   328,   328,   328,   329,   329,   329,
     329,   329,   329,   329,   330,   330,   330,   331,   331,   332,
     332,   333,   333,   334,   334,   335,   335,   336,   336,   336,
     336,   336,   336,   336,   337,   337,   338,   338,   338,   339,
     339,   339,   339,   340,   340,   340,   341,   341,   342,   342,
     343,   344,   345,   345,   345,   345,   345,   346,   346,   347,
     347,   348,   348,   348,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   357,   357,   357,   357,   358,   359,
     359,   359,   359,   359,   359,   359,   359,   359,   359,   359,
     359,   359,   359,   359,   359,   359,   359,   359,   359,   359,
     359,   359,   359,   359,   359,   359,   359,   359,   359,   359,
     359,   359,   359,   359,   359,   359,   359,   359,   359,   359,
     359,   359,   359,   359,   359,   359,   359,   359,   359,   359,
     359,   359,   359,   359,   359,   359,   359,   359,   359,   359,
     359,   359,   359,   359,   359,   359,   359,   359,   359,   359,
     359,   359,   359,   359,   359,   359,   359,   360,   360,   362,
     361,   363,   361,   365,   364,   366,   364,   367,   364,   368,
     364,   369,   364,   370,   370,   370,   371,   371,   372,   372,
     373,   373,   374,   374,   375,   375,   376,   377,   377,   377,
     378,   378,   379,   379,   380,   380,   381,   381,   382,   382,
     383,   383,   384,   385,   386,   387,   388,   389,   390,   391,
     392,   393,   394,   395,   396,   397,   398,   399,   400,   401,
     402,   402,   403,   403,   404,   404,   405,   406,   407,   407,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   409,   409,   409,   409,   410,   411,   411,   412,
     412,   413,   413,   413,   414,   414,   415,   416,   416,   417,
     417,   417,   418,   418,   418,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   420,   421,
     421,   422,   422,   422,   422,   422,   423,   423,   424,   424,
     424,   424,   425,   425,   425,   426,   426,   426,   427,   427,
     427,   428,   428,   429,   429,   429,   429,   429,   429,   429,
     429,   429,   429,   429,   429,   429,   429,   429,   430,   430,
     430,   430,   430,   430,   430,   430,   430,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   430,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   430,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   430,   430,   430,   430,
     430,   430,   430,   430,   431,   431,   432,   433,   433,   434,
     434,   434,   434,   434,   434,   434,   435,   435,   436,   436,
     437,   437,   438,   438,   438,   438,   439,   439,   439,   439,
     439,   440,   440,   440,   440,   441,   441,   442,   442,   442,
     442,   442,   442,   442,   442,   442,   442,   442,   442,   442,
     442,   442,   442,   443,   443,   444,   444,   445,   445,   445,
     445,   446,   446,   447,   447,   448,   448,   449,   449,   450,
     450,   451,   451,   453,   452,   454,   455,   455,   456,   456,
     457,   457,   457,   458,   458,   459,   459,   460,   460,   461,
     461,   462,   462,   462,   463,   463,   464,   464,   465,   465,
     466,   466,   466,   466,   466,   466,   466,   466,   466,   466,
     466,   467,   468,   468,   468,   468,   468,   468,   468,   468,
     469,   469,   469,   469,   469,   469,   469,   469,   469,   469,
     469,   470,   471,   471,   472,   472,   472,   473,   473,   473,
     474,   475,   475,   475,   476,   476,   476,   476,   477,   477,
     478,   478,   478,   478,   478,   478,   479,   479,   479,   479,
     479,   480,   480,   480,   480,   480,   480,   480,   480,   480,
     480,   481,   481,   482,   482,   482,   482,   483,   483,   484,
     484,   484,   484,   485,   485,   485,   485,   486,   486,   486,
     486,   486,   486,   487,   487,   487,   488,   488,   488,   488,
     488,   488,   488,   488,   488,   488,   488,   489,   489,   490,
     490,   491,   491,   492,   492,   492,   492,   493,   493,   494,
     494,   495,   495,   496,   496,   497,   497,   498,   498,   499,
     500,   500,   500,   500,   500,   500,   501,   501,   501,   501,
     502,   502,   503,   503,   504,   504,   505,   505,   506,   506,
     507,   508,   508,   509,   509,   509,   509,   510,   510,   510,
     511,   511,   511,   511,   512,   512,   513,   513,   513,   513,
     514,   515,   516,   516,   517,   517,   518,   518,   518,   518,
     518,   518,   518,   518,   518,   518,   518,   519,   519
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
       0,     8,     0,     7,     0,     8,     1,     1,     1,     1,
       1,     2,     3,     3,     2,     2,     0,     2,     0,     2,
       0,     1,     3,     1,     3,     2,     0,     1,     2,     1,
       1,     4,     1,     4,     1,     4,     1,     4,     3,     5,
       3,     4,     4,     5,     5,     4,     0,     1,     1,     4,
       0,     5,     0,     2,     0,     3,     0,     7,     8,     6,
       2,     5,     6,     4,     0,     4,     4,     5,     7,     6,
       6,     6,     7,     9,     8,     6,     7,     5,     2,     4,
       5,     3,     0,     3,     4,     4,     6,     5,     5,     6,
       6,     8,     7,     4,     1,     1,     2,     0,     1,     2,
       2,     2,     3,     4,     4,     4,     3,     1,     1,     2,
       4,     3,     5,     1,     3,     2,     0,     2,     3,     2,
       0,     3,     4,     4,     5,     2,     2,     2,     0,    11,
       0,    12,     3,     3,     3,     4,     4,     3,     5,     2,
       2,     0,     6,     5,     4,     3,     1,     1,     3,     4,
       1,     2,     1,     1,     5,     6,     1,     1,     4,     1,
       1,     3,     2,     2,     0,     2,     0,     1,     3,     1,
       1,     1,     1,     3,     4,     4,     4,     1,     1,     2,
       2,     2,     3,     3,     1,     1,     1,     1,     3,     1,
       3,     1,     1,     1,     0,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     1,     1,     1,     3,
       5,     1,     3,     5,     4,     5,     3,     3,     3,     4,
       3,     3,     3,     3,     2,     1,     1,     3,     1,     1,
       0,     1,     2,     4,     3,     3,     3,     2,     3,     2,
       3,     3,     3,     1,     1,     1,     1,     1,     3,     3,
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
       5,     3,     2,     0,     2,     0,     4,     4,     3,     4,
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
       3,     4,     3,     2,     1,     5,     6,     4,     3,     2,
       0,     2,     0,     5,     3,     3,     1,     2,     0,     5,
       3,     3,     1,     2,     2,     1,     2,     1,     4,     3,
       3,     6,     3,     1,     1,     1,     4,     4,     4,     4,
       4,     4,     2,     2,     4,     2,     2,     1,     3,     3,
       3,     0,     2,     5,     6,     6,     7,     1,     2,     1,
       2,     1,     4,     1,     4,     3,     0,     1,     3,     2,
       1,     2,     4,     3,     3,     1,     4,     2,     2,     0,
       0,     3,     1,     3,     3,     2,     0,     2,     2,     2,
       2,     1,     2,     4,     2,     5,     3,     1,     1,     0,
       3,     4,     5,     6,     3,     1,     3,     2,     1,     0,
       4,     1,     3,     2,     4,     5,     2,     2,     1,     1,
       1,     1,     3,     2,     1,     8,     6,     1,     0
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
  unsigned long yylno = yyrline[yyrule];
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
    default: /* Avoid compiler warnings. */
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
                  (unsigned long) yystacksize));

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

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 821 "hphp.y" /* yacc.c:1651  */
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
#line 7326 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 3:
#line 824 "hphp.y" /* yacc.c:1651  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 7334 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 4:
#line 831 "hphp.y" /* yacc.c:1651  */
    { _p->addTopStatement((yyvsp[0]));}
#line 7340 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 5:
#line 832 "hphp.y" /* yacc.c:1651  */
    { }
#line 7346 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 6:
#line 835 "hphp.y" /* yacc.c:1651  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 7352 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 7:
#line 836 "hphp.y" /* yacc.c:1651  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7358 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 8:
#line 837 "hphp.y" /* yacc.c:1651  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7364 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 9:
#line 838 "hphp.y" /* yacc.c:1651  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7370 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 10:
#line 839 "hphp.y" /* yacc.c:1651  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7376 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 11:
#line 840 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 7382 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 12:
#line 841 "hphp.y" /* yacc.c:1651  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 7390 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 13:
#line 844 "hphp.y" /* yacc.c:1651  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 7397 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 14:
#line 846 "hphp.y" /* yacc.c:1651  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 7403 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 15:
#line 847 "hphp.y" /* yacc.c:1651  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7409 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 16:
#line 848 "hphp.y" /* yacc.c:1651  */
    { _p->onNamespaceStart("");}
#line 7415 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 17:
#line 849 "hphp.y" /* yacc.c:1651  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7421 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 18:
#line 850 "hphp.y" /* yacc.c:1651  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7429 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 19:
#line 854 "hphp.y" /* yacc.c:1651  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7438 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 20:
#line 859 "hphp.y" /* yacc.c:1651  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7447 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 21:
#line 864 "hphp.y" /* yacc.c:1651  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7454 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 22:
#line 867 "hphp.y" /* yacc.c:1651  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7461 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 23:
#line 870 "hphp.y" /* yacc.c:1651  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7469 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 24:
#line 874 "hphp.y" /* yacc.c:1651  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7477 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 25:
#line 878 "hphp.y" /* yacc.c:1651  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7485 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 26:
#line 882 "hphp.y" /* yacc.c:1651  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7493 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 27:
#line 886 "hphp.y" /* yacc.c:1651  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7501 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 28:
#line 889 "hphp.y" /* yacc.c:1651  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7508 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 29:
#line 894 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7514 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 30:
#line 895 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7520 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 31:
#line 896 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7526 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 32:
#line 897 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7532 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 33:
#line 898 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7538 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 34:
#line 899 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7544 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 35:
#line 900 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7550 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 36:
#line 901 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7556 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 37:
#line 902 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7562 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 38:
#line 903 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7568 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 39:
#line 904 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7574 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 40:
#line 905 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7580 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 41:
#line 906 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7586 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 111:
#line 988 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 7592 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 112:
#line 990 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 7598 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 113:
#line 995 "hphp.y" /* yacc.c:1651  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7604 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 114:
#line 996 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7611 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 115:
#line 1002 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 7617 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 116:
#line 1006 "hphp.y" /* yacc.c:1651  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7623 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 117:
#line 1007 "hphp.y" /* yacc.c:1651  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7629 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 118:
#line 1009 "hphp.y" /* yacc.c:1651  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7635 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 119:
#line 1011 "hphp.y" /* yacc.c:1651  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7641 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 120:
#line 1016 "hphp.y" /* yacc.c:1651  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7647 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 121:
#line 1017 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7654 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 122:
#line 1023 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 7660 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 123:
#line 1027 "hphp.y" /* yacc.c:1651  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7667 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 124:
#line 1029 "hphp.y" /* yacc.c:1651  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7674 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 125:
#line 1031 "hphp.y" /* yacc.c:1651  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7681 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 126:
#line 1036 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7687 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 127:
#line 1038 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7693 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 128:
#line 1041 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7699 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 129:
#line 1043 "hphp.y" /* yacc.c:1651  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7705 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 130:
#line 1044 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7711 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 131:
#line 1049 "hphp.y" /* yacc.c:1651  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7720 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 132:
#line 1056 "hphp.y" /* yacc.c:1651  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7729 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 133:
#line 1064 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7736 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 134:
#line 1067 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7743 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 135:
#line 1073 "hphp.y" /* yacc.c:1651  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 7749 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 136:
#line 1074 "hphp.y" /* yacc.c:1651  */
    { _p->onStatementListStart((yyval)); }
#line 7755 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 138:
#line 1079 "hphp.y" /* yacc.c:1651  */
    {
                                         _p->onUsing((yyval), (yyvsp[-2]), true, (yyvsp[-1]), nullptr);
                                       }
#line 7763 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 139:
#line 1086 "hphp.y" /* yacc.c:1651  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7769 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 140:
#line 1087 "hphp.y" /* yacc.c:1651  */
    { _p->onStatementListStart((yyval));}
#line 7775 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 141:
#line 1092 "hphp.y" /* yacc.c:1651  */
    { _p->addStatement((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 7781 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 142:
#line 1093 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();
                                         _p->addStatement((yyval), (yyval), (yyvsp[0])); }
#line 7788 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 143:
#line 1098 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7794 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 144:
#line 1099 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7800 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 145:
#line 1100 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7806 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 146:
#line 1101 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 7812 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 147:
#line 1104 "hphp.y" /* yacc.c:1651  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7818 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 148:
#line 1108 "hphp.y" /* yacc.c:1651  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7824 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 149:
#line 1113 "hphp.y" /* yacc.c:1651  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7830 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 150:
#line 1114 "hphp.y" /* yacc.c:1651  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7837 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 151:
#line 1116 "hphp.y" /* yacc.c:1651  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7845 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 152:
#line 1120 "hphp.y" /* yacc.c:1651  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7852 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 153:
#line 1123 "hphp.y" /* yacc.c:1651  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7860 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 154:
#line 1127 "hphp.y" /* yacc.c:1651  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7867 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 155:
#line 1129 "hphp.y" /* yacc.c:1651  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7875 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 156:
#line 1132 "hphp.y" /* yacc.c:1651  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7882 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 157:
#line 1134 "hphp.y" /* yacc.c:1651  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7890 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 158:
#line 1137 "hphp.y" /* yacc.c:1651  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7896 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 159:
#line 1138 "hphp.y" /* yacc.c:1651  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7902 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 160:
#line 1139 "hphp.y" /* yacc.c:1651  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7908 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 161:
#line 1140 "hphp.y" /* yacc.c:1651  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7914 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 162:
#line 1141 "hphp.y" /* yacc.c:1651  */
    { _p->onReturn((yyval), NULL);}
#line 7920 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 163:
#line 1142 "hphp.y" /* yacc.c:1651  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7926 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 164:
#line 1143 "hphp.y" /* yacc.c:1651  */
    { _p->onYieldBreak((yyval));}
#line 7932 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 165:
#line 1144 "hphp.y" /* yacc.c:1651  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7938 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 166:
#line 1145 "hphp.y" /* yacc.c:1651  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7944 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 167:
#line 1146 "hphp.y" /* yacc.c:1651  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7950 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 168:
#line 1147 "hphp.y" /* yacc.c:1651  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7956 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 169:
#line 1148 "hphp.y" /* yacc.c:1651  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7962 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 170:
#line 1149 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset(); (yyval) = ';';}
#line 7968 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 171:
#line 1150 "hphp.y" /* yacc.c:1651  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7974 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 172:
#line 1151 "hphp.y" /* yacc.c:1651  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7981 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 173:
#line 1155 "hphp.y" /* yacc.c:1651  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7988 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 174:
#line 1157 "hphp.y" /* yacc.c:1651  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7996 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 175:
#line 1162 "hphp.y" /* yacc.c:1651  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 8003 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 176:
#line 1164 "hphp.y" /* yacc.c:1651  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 8011 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 177:
#line 1168 "hphp.y" /* yacc.c:1651  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 8019 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 178:
#line 1177 "hphp.y" /* yacc.c:1651  */
    { _p->onCompleteLabelScope(false);}
#line 8025 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 179:
#line 1178 "hphp.y" /* yacc.c:1651  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8031 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 180:
#line 1181 "hphp.y" /* yacc.c:1651  */
    { _p->onCompleteLabelScope(false); }
#line 8037 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 181:
#line 1182 "hphp.y" /* yacc.c:1651  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 8043 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 182:
#line 1184 "hphp.y" /* yacc.c:1651  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8051 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 183:
#line 1188 "hphp.y" /* yacc.c:1651  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8059 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 184:
#line 1192 "hphp.y" /* yacc.c:1651  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8067 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 185:
#line 1196 "hphp.y" /* yacc.c:1651  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8075 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 186:
#line 1200 "hphp.y" /* yacc.c:1651  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8083 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 187:
#line 1204 "hphp.y" /* yacc.c:1651  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8091 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 188:
#line 1209 "hphp.y" /* yacc.c:1651  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8099 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 189:
#line 1212 "hphp.y" /* yacc.c:1651  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 8105 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 190:
#line 1213 "hphp.y" /* yacc.c:1651  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 8114 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 191:
#line 1217 "hphp.y" /* yacc.c:1651  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8120 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 192:
#line 1218 "hphp.y" /* yacc.c:1651  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8126 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 193:
#line 1219 "hphp.y" /* yacc.c:1651  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8132 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 194:
#line 1220 "hphp.y" /* yacc.c:1651  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8138 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 195:
#line 1221 "hphp.y" /* yacc.c:1651  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8144 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 196:
#line 1222 "hphp.y" /* yacc.c:1651  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8150 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 197:
#line 1223 "hphp.y" /* yacc.c:1651  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 8156 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 198:
#line 1224 "hphp.y" /* yacc.c:1651  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8162 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 199:
#line 1225 "hphp.y" /* yacc.c:1651  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8168 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 200:
#line 1226 "hphp.y" /* yacc.c:1651  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 8174 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 201:
#line 1227 "hphp.y" /* yacc.c:1651  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8180 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 202:
#line 1228 "hphp.y" /* yacc.c:1651  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 8190 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 203:
#line 1250 "hphp.y" /* yacc.c:1651  */
    { _p->pushLabelScope(LS::Using);
                                         _p->onNewLabelScope(false);
                                         (yyval) = (yyvsp[-1]); }
#line 8198 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 204:
#line 1256 "hphp.y" /* yacc.c:1651  */
    { (yyval) = 1; }
#line 8204 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 205:
#line 1257 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset(); }
#line 8210 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 206:
#line 1266 "hphp.y" /* yacc.c:1651  */
    { _p->onExprListElem((yyval), nullptr, (yyvsp[-2]));
                                         _p->onExprListElem((yyval), &(yyval), (yyvsp[0])); }
#line 8217 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 207:
#line 1268 "hphp.y" /* yacc.c:1651  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0])); }
#line 8223 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 211:
#line 1278 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); }
#line 8229 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 212:
#line 1279 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset(); }
#line 8235 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 213:
#line 1283 "hphp.y" /* yacc.c:1651  */
    { _p->onNewLabelScope(false); }
#line 8241 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 214:
#line 1284 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); }
#line 8247 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 215:
#line 1293 "hphp.y" /* yacc.c:1651  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 8253 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 216:
#line 1294 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 8259 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 217:
#line 1298 "hphp.y" /* yacc.c:1651  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::Finally);}
#line 8266 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 218:
#line 1300 "hphp.y" /* yacc.c:1651  */
    { _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->popLabelScope();
                                         _p->onCompleteLabelScope(false);}
#line 8274 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 219:
#line 1306 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 8280 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 220:
#line 1307 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 8286 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 221:
#line 1311 "hphp.y" /* yacc.c:1651  */
    { (yyval) = 1;}
#line 8292 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 222:
#line 1312 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 8298 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 223:
#line 1316 "hphp.y" /* yacc.c:1651  */
    { _p->pushFuncLocation(); }
#line 8304 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 224:
#line 1322 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8313 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 225:
#line 1329 "hphp.y" /* yacc.c:1651  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8323 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 226:
#line 1337 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8332 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 227:
#line 1344 "hphp.y" /* yacc.c:1651  */
    { _p->onFunction((yyval),&(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8342 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 228:
#line 1352 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8351 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 229:
#line 1358 "hphp.y" /* yacc.c:1651  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8361 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 230:
#line 1367 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8368 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 231:
#line 1371 "hphp.y" /* yacc.c:1651  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 8374 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 232:
#line 1375 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8381 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 233:
#line 1379 "hphp.y" /* yacc.c:1651  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 8387 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 234:
#line 1385 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8394 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 235:
#line 1388 "hphp.y" /* yacc.c:1651  */
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
#line 8412 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 236:
#line 1403 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8419 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 237:
#line 1406 "hphp.y" /* yacc.c:1651  */
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
#line 8437 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 238:
#line 1420 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8444 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 239:
#line 1423 "hphp.y" /* yacc.c:1651  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8452 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 240:
#line 1428 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8459 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 241:
#line 1431 "hphp.y" /* yacc.c:1651  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8467 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 242:
#line 1438 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8474 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 243:
#line 1441 "hphp.y" /* yacc.c:1651  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8485 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 244:
#line 1449 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8492 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 245:
#line 1452 "hphp.y" /* yacc.c:1651  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8503 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 246:
#line 1460 "hphp.y" /* yacc.c:1651  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8509 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 247:
#line 1461 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 8516 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 248:
#line 1465 "hphp.y" /* yacc.c:1651  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8522 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 249:
#line 1468 "hphp.y" /* yacc.c:1651  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8528 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 250:
#line 1471 "hphp.y" /* yacc.c:1651  */
    { (yyval) = T_CLASS;}
#line 8534 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 251:
#line 1472 "hphp.y" /* yacc.c:1651  */
    { (yyval) = T_ABSTRACT; }
#line 8540 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 252:
#line 1473 "hphp.y" /* yacc.c:1651  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 8548 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 253:
#line 1476 "hphp.y" /* yacc.c:1651  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 8554 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 254:
#line 1477 "hphp.y" /* yacc.c:1651  */
    { (yyval) = T_FINAL;}
#line 8560 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 255:
#line 1481 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 8566 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 256:
#line 1482 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 8572 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 257:
#line 1485 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 8578 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 258:
#line 1486 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 8584 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 259:
#line 1489 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 8590 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 260:
#line 1490 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 8596 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 261:
#line 1493 "hphp.y" /* yacc.c:1651  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8602 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 262:
#line 1495 "hphp.y" /* yacc.c:1651  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8608 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 263:
#line 1498 "hphp.y" /* yacc.c:1651  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8614 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 264:
#line 1500 "hphp.y" /* yacc.c:1651  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8620 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 265:
#line 1504 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 8626 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 266:
#line 1505 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 8632 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 267:
#line 1508 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8638 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 268:
#line 1509 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8644 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 269:
#line 1510 "hphp.y" /* yacc.c:1651  */
    { _p->onListAssignment((yyval), (yyvsp[0]), NULL);}
#line 8650 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 270:
#line 1514 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 8656 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 271:
#line 1516 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-2]);}
#line 8662 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 272:
#line 1519 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 8668 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 273:
#line 1521 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-2]);}
#line 8674 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 274:
#line 1524 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 8680 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 275:
#line 1526 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-2]);}
#line 8686 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 276:
#line 1529 "hphp.y" /* yacc.c:1651  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8692 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 277:
#line 1531 "hphp.y" /* yacc.c:1651  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8698 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 278:
#line 1535 "hphp.y" /* yacc.c:1651  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8704 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 279:
#line 1537 "hphp.y" /* yacc.c:1651  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8711 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 280:
#line 1542 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 8717 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 281:
#line 1543 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 8723 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 282:
#line 1544 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-2]);}
#line 8729 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 283:
#line 1545 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-2]);}
#line 8735 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 284:
#line 1550 "hphp.y" /* yacc.c:1651  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8741 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 285:
#line 1552 "hphp.y" /* yacc.c:1651  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8747 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 286:
#line 1553 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 8753 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 287:
#line 1556 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 8759 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 288:
#line 1557 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 8765 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 289:
#line 1562 "hphp.y" /* yacc.c:1651  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8771 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 290:
#line 1563 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 8777 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 291:
#line 1568 "hphp.y" /* yacc.c:1651  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8783 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 292:
#line 1569 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 8789 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 293:
#line 1572 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 8795 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 294:
#line 1573 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 8801 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 295:
#line 1576 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 8807 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 296:
#line 1577 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 8813 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 297:
#line 1585 "hphp.y" /* yacc.c:1651  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8820 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 298:
#line 1591 "hphp.y" /* yacc.c:1651  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8827 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 299:
#line 1597 "hphp.y" /* yacc.c:1651  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8835 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 300:
#line 1601 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 8841 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 301:
#line 1605 "hphp.y" /* yacc.c:1651  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8848 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 302:
#line 1610 "hphp.y" /* yacc.c:1651  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8855 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 303:
#line 1615 "hphp.y" /* yacc.c:1651  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8863 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 304:
#line 1618 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset(); }
#line 8869 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 305:
#line 1624 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8877 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 306:
#line 1629 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8885 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 307:
#line 1634 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8893 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 308:
#line 1640 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8901 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 309:
#line 1646 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8909 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 310:
#line 1652 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8917 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 311:
#line 1658 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8925 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 312:
#line 1664 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8933 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 313:
#line 1671 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8941 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 314:
#line 1678 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8949 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 315:
#line 1687 "hphp.y" /* yacc.c:1651  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8956 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 316:
#line 1692 "hphp.y" /* yacc.c:1651  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8963 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 317:
#line 1697 "hphp.y" /* yacc.c:1651  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8971 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 318:
#line 1701 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 8977 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 319:
#line 1704 "hphp.y" /* yacc.c:1651  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8984 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 320:
#line 1708 "hphp.y" /* yacc.c:1651  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 8991 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 321:
#line 1712 "hphp.y" /* yacc.c:1651  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 8999 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 322:
#line 1715 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 9005 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 323:
#line 1720 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9013 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 324:
#line 1724 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9021 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 325:
#line 1728 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9029 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 326:
#line 1733 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9037 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 327:
#line 1738 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9045 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 328:
#line 1743 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9053 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 329:
#line 1748 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9061 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 330:
#line 1753 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9069 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 331:
#line 1759 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9077 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 332:
#line 1765 "hphp.y" /* yacc.c:1651  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9085 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 333:
#line 1771 "hphp.y" /* yacc.c:1651  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 9091 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 334:
#line 1772 "hphp.y" /* yacc.c:1651  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 9097 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 335:
#line 1773 "hphp.y" /* yacc.c:1651  */
    { _p->onPipeVariable((yyval));}
#line 9103 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 336:
#line 1778 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 9109 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 337:
#line 1779 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 9115 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 338:
#line 1782 "hphp.y" /* yacc.c:1651  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,false);}
#line 9122 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 339:
#line 1784 "hphp.y" /* yacc.c:1651  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::InOut,false);}
#line 9129 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 340:
#line 1786 "hphp.y" /* yacc.c:1651  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::Ref,false);}
#line 9136 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 341:
#line 1788 "hphp.y" /* yacc.c:1651  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,true);}
#line 9143 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 342:
#line 1791 "hphp.y" /* yacc.c:1651  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::In,false);}
#line 9150 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 343:
#line 1794 "hphp.y" /* yacc.c:1651  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::In,true);}
#line 9157 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 344:
#line 1797 "hphp.y" /* yacc.c:1651  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::Ref,false);}
#line 9164 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 345:
#line 1800 "hphp.y" /* yacc.c:1651  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::InOut,false);}
#line 9171 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 346:
#line 1805 "hphp.y" /* yacc.c:1651  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9177 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 347:
#line 1806 "hphp.y" /* yacc.c:1651  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 9183 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 348:
#line 1809 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 9189 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 349:
#line 1810 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 9195 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 350:
#line 1811 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 9201 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 351:
#line 1815 "hphp.y" /* yacc.c:1651  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9207 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 352:
#line 1817 "hphp.y" /* yacc.c:1651  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9213 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 353:
#line 1818 "hphp.y" /* yacc.c:1651  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 9219 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 354:
#line 1819 "hphp.y" /* yacc.c:1651  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9225 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 355:
#line 1824 "hphp.y" /* yacc.c:1651  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9231 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 356:
#line 1825 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 9237 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 357:
#line 1828 "hphp.y" /* yacc.c:1651  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL,NULL);}
#line 9244 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 358:
#line 1833 "hphp.y" /* yacc.c:1651  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9250 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 359:
#line 1839 "hphp.y" /* yacc.c:1651  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9256 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 360:
#line 1840 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 9262 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 361:
#line 1844 "hphp.y" /* yacc.c:1651  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-2]),(yyvsp[-1]),NULL,NULL);}
#line 9269 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 362:
#line 1848 "hphp.y" /* yacc.c:1651  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-2]),NULL);}
#line 9276 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 363:
#line 1852 "hphp.y" /* yacc.c:1651  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-2]),(yyvsp[-1]),NULL,&(yyvsp[-3]));}
#line 9283 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 364:
#line 1857 "hphp.y" /* yacc.c:1651  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-2]),&(yyvsp[-4]));}
#line 9290 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 365:
#line 1859 "hphp.y" /* yacc.c:1651  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL,NULL);}
#line 9297 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 366:
#line 1862 "hphp.y" /* yacc.c:1651  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL,NULL,true);}
#line 9304 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 367:
#line 1864 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); }
#line 9310 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 368:
#line 1867 "hphp.y" /* yacc.c:1651  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9318 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 369:
#line 1874 "hphp.y" /* yacc.c:1651  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9328 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 370:
#line 1882 "hphp.y" /* yacc.c:1651  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9336 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 371:
#line 1889 "hphp.y" /* yacc.c:1651  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9346 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 372:
#line 1895 "hphp.y" /* yacc.c:1651  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 9352 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 373:
#line 1897 "hphp.y" /* yacc.c:1651  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9358 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 374:
#line 1899 "hphp.y" /* yacc.c:1651  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9364 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 375:
#line 1901 "hphp.y" /* yacc.c:1651  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 9370 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 376:
#line 1903 "hphp.y" /* yacc.c:1651  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 9376 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 377:
#line 1904 "hphp.y" /* yacc.c:1651  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 9383 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 378:
#line 1907 "hphp.y" /* yacc.c:1651  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 9389 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 379:
#line 1910 "hphp.y" /* yacc.c:1651  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9395 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 380:
#line 1911 "hphp.y" /* yacc.c:1651  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9401 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 381:
#line 1912 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset(); }
#line 9407 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 382:
#line 1918 "hphp.y" /* yacc.c:1651  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 9413 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 383:
#line 1923 "hphp.y" /* yacc.c:1651  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 9420 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 384:
#line 1926 "hphp.y" /* yacc.c:1651  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 9428 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 385:
#line 1933 "hphp.y" /* yacc.c:1651  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 9434 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 386:
#line 1934 "hphp.y" /* yacc.c:1651  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 9441 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 387:
#line 1939 "hphp.y" /* yacc.c:1651  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 9448 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 388:
#line 1942 "hphp.y" /* yacc.c:1651  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 9454 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 389:
#line 1949 "hphp.y" /* yacc.c:1651  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 9461 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 390:
#line 1951 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 9467 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 391:
#line 1955 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 9473 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 393:
#line 1960 "hphp.y" /* yacc.c:1651  */
    { (yyval) = 4;}
#line 9479 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 394:
#line 1962 "hphp.y" /* yacc.c:1651  */
    { (yyval) = 4;}
#line 9485 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 395:
#line 1964 "hphp.y" /* yacc.c:1651  */
    { (yyval) = 4;}
#line 9491 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 396:
#line 1965 "hphp.y" /* yacc.c:1651  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 9502 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 397:
#line 1971 "hphp.y" /* yacc.c:1651  */
    { (yyval) = 6;}
#line 9508 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 398:
#line 1973 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 9514 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 399:
#line 1974 "hphp.y" /* yacc.c:1651  */
    { (yyval) = 9; }
#line 9520 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 400:
#line 1978 "hphp.y" /* yacc.c:1651  */
    { _p->onExprListElem((yyval),nullptr,(yyvsp[0])); }
#line 9526 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 401:
#line 1980 "hphp.y" /* yacc.c:1651  */
    { _p->onExprListElem((yyval),&(yyvsp[-2]),(yyvsp[0])); }
#line 9532 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 402:
#line 1985 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 9538 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 403:
#line 1988 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 9544 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 404:
#line 1989 "hphp.y" /* yacc.c:1651  */
    { scalar_null(_p, (yyval));}
#line 9550 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 405:
#line 1993 "hphp.y" /* yacc.c:1651  */
    { scalar_num(_p, (yyval), "1");}
#line 9556 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 406:
#line 1994 "hphp.y" /* yacc.c:1651  */
    { scalar_num(_p, (yyval), "0");}
#line 9562 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 407:
#line 1998 "hphp.y" /* yacc.c:1651  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),&t,0);}
#line 9569 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 408:
#line 2001 "hphp.y" /* yacc.c:1651  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),&t,0);}
#line 9576 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 409:
#line 2006 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 9583 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 410:
#line 2011 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 9589 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 411:
#line 2012 "hphp.y" /* yacc.c:1651  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 9596 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 412:
#line 2014 "hphp.y" /* yacc.c:1651  */
    { (yyval) = 0;}
#line 9602 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 413:
#line 2018 "hphp.y" /* yacc.c:1651  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 9608 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 414:
#line 2019 "hphp.y" /* yacc.c:1651  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 9614 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 415:
#line 2020 "hphp.y" /* yacc.c:1651  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 9620 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 416:
#line 2021 "hphp.y" /* yacc.c:1651  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 9626 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 417:
#line 2025 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 9632 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 418:
#line 2026 "hphp.y" /* yacc.c:1651  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 9638 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 419:
#line 2027 "hphp.y" /* yacc.c:1651  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 9644 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 420:
#line 2028 "hphp.y" /* yacc.c:1651  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 9650 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 421:
#line 2029 "hphp.y" /* yacc.c:1651  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 9656 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 422:
#line 2031 "hphp.y" /* yacc.c:1651  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 9662 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 423:
#line 2033 "hphp.y" /* yacc.c:1651  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 9668 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 424:
#line 2037 "hphp.y" /* yacc.c:1651  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9676 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 425:
#line 2040 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9682 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 426:
#line 2041 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9688 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 427:
#line 2045 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 9694 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 428:
#line 2046 "hphp.y" /* yacc.c:1651  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9700 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 429:
#line 2050 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 9706 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 430:
#line 2051 "hphp.y" /* yacc.c:1651  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9712 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 431:
#line 2054 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 9718 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 432:
#line 2055 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 9724 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 433:
#line 2058 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 9730 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 434:
#line 2059 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 9736 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 435:
#line 2062 "hphp.y" /* yacc.c:1651  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9742 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 436:
#line 2064 "hphp.y" /* yacc.c:1651  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9748 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 437:
#line 2067 "hphp.y" /* yacc.c:1651  */
    { (yyval) = T_PUBLIC;}
#line 9754 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 438:
#line 2068 "hphp.y" /* yacc.c:1651  */
    { (yyval) = T_PROTECTED;}
#line 9760 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 439:
#line 2069 "hphp.y" /* yacc.c:1651  */
    { (yyval) = T_PRIVATE;}
#line 9766 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 440:
#line 2070 "hphp.y" /* yacc.c:1651  */
    { (yyval) = T_STATIC;}
#line 9772 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 441:
#line 2071 "hphp.y" /* yacc.c:1651  */
    { (yyval) = T_ABSTRACT;}
#line 9778 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 442:
#line 2072 "hphp.y" /* yacc.c:1651  */
    { (yyval) = T_FINAL;}
#line 9784 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 443:
#line 2073 "hphp.y" /* yacc.c:1651  */
    { (yyval) = T_ASYNC;}
#line 9790 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 444:
#line 2077 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 9796 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 445:
#line 2078 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 9802 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 446:
#line 2081 "hphp.y" /* yacc.c:1651  */
    { (yyval) = T_PUBLIC;}
#line 9808 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 447:
#line 2082 "hphp.y" /* yacc.c:1651  */
    { (yyval) = T_PROTECTED;}
#line 9814 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 448:
#line 2083 "hphp.y" /* yacc.c:1651  */
    { (yyval) = T_PRIVATE;}
#line 9820 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 449:
#line 2087 "hphp.y" /* yacc.c:1651  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9826 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 450:
#line 2089 "hphp.y" /* yacc.c:1651  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9832 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 451:
#line 2090 "hphp.y" /* yacc.c:1651  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9838 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 452:
#line 2091 "hphp.y" /* yacc.c:1651  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9844 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 453:
#line 2095 "hphp.y" /* yacc.c:1651  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9850 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 454:
#line 2097 "hphp.y" /* yacc.c:1651  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9856 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 455:
#line 2100 "hphp.y" /* yacc.c:1651  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9862 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 456:
#line 2106 "hphp.y" /* yacc.c:1651  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9868 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 457:
#line 2108 "hphp.y" /* yacc.c:1651  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9874 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 458:
#line 2112 "hphp.y" /* yacc.c:1651  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9882 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 459:
#line 2116 "hphp.y" /* yacc.c:1651  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9889 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 460:
#line 2120 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 9895 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 461:
#line 2124 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); }
#line 9901 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 462:
#line 2128 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 9907 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 463:
#line 2130 "hphp.y" /* yacc.c:1651  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9913 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 464:
#line 2131 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9919 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 465:
#line 2132 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 9925 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 466:
#line 2133 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 9931 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 467:
#line 2136 "hphp.y" /* yacc.c:1651  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9937 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 468:
#line 2137 "hphp.y" /* yacc.c:1651  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9943 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 469:
#line 2141 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 9949 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 470:
#line 2142 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 9955 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 471:
#line 2146 "hphp.y" /* yacc.c:1651  */
    { _p->onYield((yyval), NULL);}
#line 9961 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 472:
#line 2147 "hphp.y" /* yacc.c:1651  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9967 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 473:
#line 2148 "hphp.y" /* yacc.c:1651  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9973 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 474:
#line 2149 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); }
#line 9979 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 475:
#line 2153 "hphp.y" /* yacc.c:1651  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9985 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 476:
#line 2158 "hphp.y" /* yacc.c:1651  */
    { _p->onListAssignment((yyval), (yyvsp[-2]), &(yyvsp[0]), true);}
#line 9991 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 477:
#line 2162 "hphp.y" /* yacc.c:1651  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 9997 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 478:
#line 2166 "hphp.y" /* yacc.c:1651  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10003 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 479:
#line 2170 "hphp.y" /* yacc.c:1651  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 10009 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 480:
#line 2174 "hphp.y" /* yacc.c:1651  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10015 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 481:
#line 2179 "hphp.y" /* yacc.c:1651  */
    { _p->onListAssignment((yyval), (yyvsp[-2]), &(yyvsp[0]), true);}
#line 10021 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 482:
#line 2183 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 10027 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 483:
#line 2187 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 10033 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 484:
#line 2188 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 10039 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 485:
#line 2189 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 10045 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 486:
#line 2190 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 10051 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 487:
#line 2191 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 10057 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 488:
#line 2195 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); }
#line 10063 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 489:
#line 2200 "hphp.y" /* yacc.c:1651  */
    { _p->onListAssignment((yyval), (yyvsp[-2]), &(yyvsp[0]));}
#line 10069 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 490:
#line 2201 "hphp.y" /* yacc.c:1651  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 10075 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 491:
#line 2202 "hphp.y" /* yacc.c:1651  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 10081 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 492:
#line 2205 "hphp.y" /* yacc.c:1651  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 10087 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 493:
#line 2206 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 10093 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 494:
#line 2207 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 10099 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 495:
#line 2208 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 10105 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 496:
#line 2209 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 10111 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 497:
#line 2210 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 10117 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 498:
#line 2211 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 10123 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 499:
#line 2212 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 10129 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 500:
#line 2213 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 10135 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 501:
#line 2214 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 10141 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 502:
#line 2215 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 10147 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 503:
#line 2216 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 10153 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 504:
#line 2217 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 10159 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 505:
#line 2218 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 10165 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 506:
#line 2219 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 10171 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 507:
#line 2220 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 10177 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 508:
#line 2221 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 10183 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 509:
#line 2222 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 10189 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 510:
#line 2223 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 10195 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 511:
#line 2224 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 10201 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 512:
#line 2225 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 10207 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 513:
#line 2226 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 10213 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 514:
#line 2227 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 10219 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 515:
#line 2228 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 10225 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 516:
#line 2229 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 10231 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 517:
#line 2230 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 10237 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 518:
#line 2231 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 10243 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 519:
#line 2232 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 10249 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 520:
#line 2233 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 10255 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 521:
#line 2234 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 10261 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 522:
#line 2235 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 10267 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 523:
#line 2236 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 10273 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 524:
#line 2237 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 10279 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 525:
#line 2238 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 10285 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 526:
#line 2239 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 10291 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 527:
#line 2240 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 10297 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 528:
#line 2241 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 10303 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 529:
#line 2242 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 10309 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 530:
#line 2243 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 10315 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 531:
#line 2244 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 10321 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 532:
#line 2245 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 10327 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 533:
#line 2246 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 10333 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 534:
#line 2247 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 10339 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 535:
#line 2248 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 10345 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 536:
#line 2249 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 10352 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 537:
#line 2251 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 10358 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 538:
#line 2252 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 10365 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 539:
#line 2254 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 10371 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 540:
#line 2256 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 10377 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 541:
#line 2257 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 10383 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 542:
#line 2258 "hphp.y" /* yacc.c:1651  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 10389 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 543:
#line 2259 "hphp.y" /* yacc.c:1651  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 10395 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 544:
#line 2260 "hphp.y" /* yacc.c:1651  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 10401 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 545:
#line 2261 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 10407 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 546:
#line 2262 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 10413 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 547:
#line 2263 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 10419 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 548:
#line 2264 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 10425 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 549:
#line 2265 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 10431 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 550:
#line 2266 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 10437 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 551:
#line 2267 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 10443 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 552:
#line 2268 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 10449 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 553:
#line 2269 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 10455 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 554:
#line 2270 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 10461 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 555:
#line 2271 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 10467 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 556:
#line 2272 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 10473 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 557:
#line 2273 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 10479 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 558:
#line 2274 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 10485 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 559:
#line 2275 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 10491 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 560:
#line 2276 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 10497 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 561:
#line 2277 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 10503 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 562:
#line 2278 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 10509 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 563:
#line 2279 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 10515 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 564:
#line 2280 "hphp.y" /* yacc.c:1651  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 10521 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 565:
#line 2281 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 10527 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 566:
#line 2282 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 10533 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 567:
#line 2289 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-2]);}
#line 10539 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 568:
#line 2290 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 10545 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 569:
#line 2295 "hphp.y" /* yacc.c:1651  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10554 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 570:
#line 2301 "hphp.y" /* yacc.c:1651  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10566 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 571:
#line 2310 "hphp.y" /* yacc.c:1651  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10575 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 572:
#line 2316 "hphp.y" /* yacc.c:1651  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10587 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 573:
#line 2327 "hphp.y" /* yacc.c:1651  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,NULL,NULL);}
#line 10601 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 574:
#line 2336 "hphp.y" /* yacc.c:1651  */
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
#line 10616 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 575:
#line 2347 "hphp.y" /* yacc.c:1651  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10626 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 576:
#line 2355 "hphp.y" /* yacc.c:1651  */
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
#line 10641 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 577:
#line 2366 "hphp.y" /* yacc.c:1651  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10651 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 578:
#line 2372 "hphp.y" /* yacc.c:1651  */
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
#line 10668 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 579:
#line 2384 "hphp.y" /* yacc.c:1651  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,NULL,NULL);}
#line 10682 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 580:
#line 2393 "hphp.y" /* yacc.c:1651  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10695 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 581:
#line 2401 "hphp.y" /* yacc.c:1651  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10705 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 582:
#line 2409 "hphp.y" /* yacc.c:1651  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10718 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 583:
#line 2420 "hphp.y" /* yacc.c:1651  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10724 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 584:
#line 2421 "hphp.y" /* yacc.c:1651  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10730 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 585:
#line 2423 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); }
#line 10736 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 586:
#line 2427 "hphp.y" /* yacc.c:1651  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10743 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 587:
#line 2429 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 10749 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 588:
#line 2436 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0); }
#line 10755 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 589:
#line 2439 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0); }
#line 10761 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 590:
#line 2446 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0); }
#line 10767 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 591:
#line 2449 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0); }
#line 10773 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 592:
#line 2454 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); }
#line 10779 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 593:
#line 2455 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset(); }
#line 10785 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 594:
#line 2460 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); }
#line 10791 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 595:
#line 2461 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset(); }
#line 10797 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 596:
#line 2465 "hphp.y" /* yacc.c:1651  */
    { _p->onDArray((yyval), (yyvsp[-1]));}
#line 10803 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 597:
#line 2469 "hphp.y" /* yacc.c:1651  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10809 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 598:
#line 2470 "hphp.y" /* yacc.c:1651  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10815 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 599:
#line 2471 "hphp.y" /* yacc.c:1651  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10821 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 600:
#line 2476 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 10827 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 601:
#line 2477 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 10833 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 602:
#line 2482 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 10839 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 603:
#line 2483 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 10845 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 604:
#line 2488 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 10851 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 605:
#line 2489 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 10857 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 606:
#line 2495 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 10863 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 607:
#line 2497 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 10869 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 608:
#line 2502 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 10875 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 609:
#line 2503 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 10881 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 610:
#line 2509 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 10887 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 611:
#line 2511 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 10893 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 612:
#line 2515 "hphp.y" /* yacc.c:1651  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10899 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 613:
#line 2519 "hphp.y" /* yacc.c:1651  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10905 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 614:
#line 2523 "hphp.y" /* yacc.c:1651  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10911 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 615:
#line 2527 "hphp.y" /* yacc.c:1651  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10917 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 616:
#line 2531 "hphp.y" /* yacc.c:1651  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10923 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 617:
#line 2535 "hphp.y" /* yacc.c:1651  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10929 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 618:
#line 2539 "hphp.y" /* yacc.c:1651  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10935 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 619:
#line 2543 "hphp.y" /* yacc.c:1651  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10941 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 620:
#line 2547 "hphp.y" /* yacc.c:1651  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10947 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 621:
#line 2551 "hphp.y" /* yacc.c:1651  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10953 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 622:
#line 2555 "hphp.y" /* yacc.c:1651  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10959 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 623:
#line 2559 "hphp.y" /* yacc.c:1651  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10965 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 624:
#line 2563 "hphp.y" /* yacc.c:1651  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10971 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 625:
#line 2567 "hphp.y" /* yacc.c:1651  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10977 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 626:
#line 2571 "hphp.y" /* yacc.c:1651  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10983 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 627:
#line 2575 "hphp.y" /* yacc.c:1651  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10989 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 628:
#line 2579 "hphp.y" /* yacc.c:1651  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10995 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 629:
#line 2583 "hphp.y" /* yacc.c:1651  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11001 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 630:
#line 2588 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 11007 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 631:
#line 2589 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 11013 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 632:
#line 2594 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 11019 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 633:
#line 2595 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 11025 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 634:
#line 2600 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 11031 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 635:
#line 2601 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 11037 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 636:
#line 2606 "hphp.y" /* yacc.c:1651  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11045 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 637:
#line 2613 "hphp.y" /* yacc.c:1651  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11053 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 638:
#line 2620 "hphp.y" /* yacc.c:1651  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11059 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 639:
#line 2622 "hphp.y" /* yacc.c:1651  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11065 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 640:
#line 2626 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11071 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 641:
#line 2627 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11077 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 642:
#line 2628 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11083 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 643:
#line 2629 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11089 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 644:
#line 2630 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11095 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 645:
#line 2631 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11101 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 646:
#line 2632 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11107 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 647:
#line 2633 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11113 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 648:
#line 2634 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11119 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 649:
#line 2635 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 11126 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 650:
#line 2637 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 11132 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 651:
#line 2638 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11138 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 652:
#line 2642 "hphp.y" /* yacc.c:1651  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11144 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 653:
#line 2643 "hphp.y" /* yacc.c:1651  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 11150 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 654:
#line 2644 "hphp.y" /* yacc.c:1651  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 11156 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 655:
#line 2645 "hphp.y" /* yacc.c:1651  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 11162 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 656:
#line 2652 "hphp.y" /* yacc.c:1651  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 11168 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 657:
#line 2655 "hphp.y" /* yacc.c:1651  */
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
#line 11186 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 658:
#line 2670 "hphp.y" /* yacc.c:1651  */
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
#line 11204 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 659:
#line 2685 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset(); (yyval).setText("");}
#line 11210 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 660:
#line 2686 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 11216 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 661:
#line 2689 "hphp.y" /* yacc.c:1651  */
    { _p->onXhpAttributesStart(); (yyval).reset();}
#line 11222 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 662:
#line 2691 "hphp.y" /* yacc.c:1651  */
    { _p->onXhpAttributeSpread((yyval), &(yyvsp[-4]), (yyvsp[-1]));}
#line 11228 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 663:
#line 2694 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 11234 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 664:
#line 2697 "hphp.y" /* yacc.c:1651  */
    { _p->onOptExprListElem((yyval), &(yyvsp[-1]), (yyvsp[0])); }
#line 11240 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 665:
#line 2698 "hphp.y" /* yacc.c:1651  */
    {  (yyval).reset();}
#line 11246 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 666:
#line 2701 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11253 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 667:
#line 2705 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11261 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 668:
#line 2708 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 11267 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 669:
#line 2711 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 11279 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 670:
#line 2718 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); }
#line 11285 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 671:
#line 2719 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 11291 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 672:
#line 2723 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11297 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 673:
#line 2725 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 11303 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 674:
#line 2727 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 11309 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 675:
#line 2731 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11315 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 676:
#line 2732 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11321 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 677:
#line 2733 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11327 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 678:
#line 2734 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11333 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 679:
#line 2735 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11339 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 680:
#line 2736 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11345 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 681:
#line 2737 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11351 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 682:
#line 2738 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11357 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 683:
#line 2739 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11363 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 684:
#line 2740 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11369 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 685:
#line 2741 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11375 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 686:
#line 2742 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11381 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 687:
#line 2743 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11387 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 688:
#line 2744 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11393 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 689:
#line 2745 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11399 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 690:
#line 2746 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11405 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 691:
#line 2747 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11411 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 692:
#line 2748 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11417 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 693:
#line 2749 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11423 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 694:
#line 2750 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11429 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 695:
#line 2751 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11435 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 696:
#line 2752 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11441 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 697:
#line 2753 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11447 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 698:
#line 2754 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11453 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 699:
#line 2755 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11459 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 700:
#line 2756 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11465 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 701:
#line 2757 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11471 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 702:
#line 2758 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11477 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 703:
#line 2759 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11483 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 704:
#line 2760 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11489 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 705:
#line 2761 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11495 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 706:
#line 2762 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11501 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 707:
#line 2763 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11507 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 708:
#line 2764 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11513 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 709:
#line 2765 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11519 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 710:
#line 2766 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11525 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 711:
#line 2767 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11531 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 712:
#line 2768 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11537 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 713:
#line 2769 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11543 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 714:
#line 2770 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11549 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 715:
#line 2771 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11555 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 716:
#line 2772 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11561 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 717:
#line 2773 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11567 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 718:
#line 2774 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11573 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 719:
#line 2775 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11579 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 720:
#line 2776 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11585 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 721:
#line 2777 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11591 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 722:
#line 2778 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11597 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 723:
#line 2779 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11603 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 724:
#line 2780 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11609 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 725:
#line 2781 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11615 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 726:
#line 2782 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11621 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 727:
#line 2783 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11627 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 728:
#line 2784 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11633 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 729:
#line 2785 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11639 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 730:
#line 2786 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11645 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 731:
#line 2787 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11651 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 732:
#line 2788 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11657 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 733:
#line 2789 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11663 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 734:
#line 2790 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11669 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 735:
#line 2791 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11675 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 736:
#line 2792 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11681 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 737:
#line 2793 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11687 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 738:
#line 2794 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11693 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 739:
#line 2795 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11699 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 740:
#line 2796 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11705 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 741:
#line 2797 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11711 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 742:
#line 2798 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11717 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 743:
#line 2799 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11723 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 744:
#line 2800 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11729 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 745:
#line 2801 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11735 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 746:
#line 2802 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11741 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 747:
#line 2803 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11747 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 748:
#line 2804 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11753 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 749:
#line 2805 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11759 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 750:
#line 2806 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11765 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 751:
#line 2807 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11771 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 752:
#line 2808 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11777 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 753:
#line 2809 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11783 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 754:
#line 2810 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11789 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 755:
#line 2811 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11795 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 756:
#line 2812 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11801 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 757:
#line 2813 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11807 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 758:
#line 2818 "hphp.y" /* yacc.c:1651  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11813 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 759:
#line 2822 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11819 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 760:
#line 2823 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11825 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 761:
#line 2827 "hphp.y" /* yacc.c:1651  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11831 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 762:
#line 2828 "hphp.y" /* yacc.c:1651  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11837 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 763:
#line 2829 "hphp.y" /* yacc.c:1651  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11843 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 764:
#line 2830 "hphp.y" /* yacc.c:1651  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11850 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 765:
#line 2832 "hphp.y" /* yacc.c:1651  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11857 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 766:
#line 2836 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 11863 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 767:
#line 2845 "hphp.y" /* yacc.c:1651  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11869 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 768:
#line 2848 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 11875 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 769:
#line 2849 "hphp.y" /* yacc.c:1651  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11882 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 770:
#line 2851 "hphp.y" /* yacc.c:1651  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11889 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 771:
#line 2861 "hphp.y" /* yacc.c:1651  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11895 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 772:
#line 2865 "hphp.y" /* yacc.c:1651  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11901 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 773:
#line 2866 "hphp.y" /* yacc.c:1651  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11907 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 774:
#line 2867 "hphp.y" /* yacc.c:1651  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11913 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 775:
#line 2871 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 11919 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 776:
#line 2872 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11925 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 777:
#line 2873 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 11931 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 778:
#line 2877 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 11937 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 779:
#line 2878 "hphp.y" /* yacc.c:1651  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11943 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 780:
#line 2879 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 11949 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 781:
#line 2883 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 11955 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 782:
#line 2884 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 11961 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 783:
#line 2888 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11967 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 784:
#line 2889 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11973 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 785:
#line 2890 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11979 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 786:
#line 2891 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11986 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 787:
#line 2893 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11992 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 788:
#line 2894 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11998 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 789:
#line 2895 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 12004 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 790:
#line 2896 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 12010 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 791:
#line 2897 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 12016 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 792:
#line 2898 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 12022 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 793:
#line 2899 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 12028 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 794:
#line 2900 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 12034 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 795:
#line 2901 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 12040 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 796:
#line 2904 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12046 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 797:
#line 2906 "hphp.y" /* yacc.c:1651  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12052 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 798:
#line 2910 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12058 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 799:
#line 2911 "hphp.y" /* yacc.c:1651  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12064 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 800:
#line 2913 "hphp.y" /* yacc.c:1651  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12070 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 801:
#line 2914 "hphp.y" /* yacc.c:1651  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12076 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 802:
#line 2916 "hphp.y" /* yacc.c:1651  */
    { _p->onDArray((yyval),(yyvsp[-1]));}
#line 12082 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 803:
#line 2917 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12088 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 804:
#line 2918 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12094 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 805:
#line 2919 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12100 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 806:
#line 2920 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12106 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 807:
#line 2921 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12112 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 808:
#line 2922 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12118 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 809:
#line 2923 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12124 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 810:
#line 2924 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12130 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 811:
#line 2925 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 12136 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 812:
#line 2927 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 12142 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 813:
#line 2929 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 12148 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 814:
#line 2931 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 12154 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 815:
#line 2933 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 12160 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 816:
#line 2935 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 12166 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 817:
#line 2936 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 12172 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 818:
#line 2937 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 12178 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 819:
#line 2938 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 12184 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 820:
#line 2939 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 12190 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 821:
#line 2940 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 12196 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 822:
#line 2941 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 12202 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 823:
#line 2942 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 12208 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 824:
#line 2943 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 12214 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 825:
#line 2944 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 12220 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 826:
#line 2945 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 12226 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 827:
#line 2946 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 12232 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 828:
#line 2947 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 12238 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 829:
#line 2948 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 12244 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 830:
#line 2949 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 12250 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 831:
#line 2950 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12256 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 832:
#line 2951 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12262 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 833:
#line 2953 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 12268 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 834:
#line 2955 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 12274 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 835:
#line 2957 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 12280 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 836:
#line 2959 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 12286 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 837:
#line 2960 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 12292 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 838:
#line 2962 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 12299 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 839:
#line 2964 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 12305 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 840:
#line 2967 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 12312 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 841:
#line 2971 "hphp.y" /* yacc.c:1651  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 12318 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 842:
#line 2974 "hphp.y" /* yacc.c:1651  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 12324 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 843:
#line 2975 "hphp.y" /* yacc.c:1651  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 12330 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 844:
#line 2979 "hphp.y" /* yacc.c:1651  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12336 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 845:
#line 2980 "hphp.y" /* yacc.c:1651  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12342 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 846:
#line 2986 "hphp.y" /* yacc.c:1651  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12348 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 847:
#line 2992 "hphp.y" /* yacc.c:1651  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12354 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 848:
#line 2993 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12360 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 849:
#line 2997 "hphp.y" /* yacc.c:1651  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12366 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 850:
#line 2998 "hphp.y" /* yacc.c:1651  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12372 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 851:
#line 2999 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12378 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 852:
#line 3000 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12384 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 853:
#line 3001 "hphp.y" /* yacc.c:1651  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 12390 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 854:
#line 3002 "hphp.y" /* yacc.c:1651  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 12396 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 855:
#line 3004 "hphp.y" /* yacc.c:1651  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 12403 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 856:
#line 3009 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 12409 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 857:
#line 3010 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 12415 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 858:
#line 3014 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 12421 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 859:
#line 3015 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 12427 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 860:
#line 3018 "hphp.y" /* yacc.c:1651  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 12433 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 861:
#line 3019 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 12439 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 862:
#line 3025 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 12445 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 863:
#line 3027 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,&(yyvsp[0]),0);}
#line 12451 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 864:
#line 3029 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 12457 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 865:
#line 3030 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,  0,&(yyvsp[0]),0);}
#line 12463 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 866:
#line 3034 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12469 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 867:
#line 3035 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12475 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 868:
#line 3036 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12481 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 869:
#line 3039 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12487 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 870:
#line 3041 "hphp.y" /* yacc.c:1651  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12493 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 871:
#line 3044 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 12499 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 872:
#line 3045 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 12505 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 873:
#line 3046 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 12511 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 874:
#line 3047 "hphp.y" /* yacc.c:1651  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12517 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 875:
#line 3051 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 12524 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 876:
#line 3054 "hphp.y" /* yacc.c:1651  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 12532 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 878:
#line 3061 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12538 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 879:
#line 3062 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12544 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 880:
#line 3065 "hphp.y" /* yacc.c:1651  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 12552 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 881:
#line 3068 "hphp.y" /* yacc.c:1651  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12558 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 882:
#line 3069 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12564 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 883:
#line 3070 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12570 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 884:
#line 3072 "hphp.y" /* yacc.c:1651  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12576 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 885:
#line 3073 "hphp.y" /* yacc.c:1651  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12582 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 886:
#line 3075 "hphp.y" /* yacc.c:1651  */
    { _p->onDArray((yyval),(yyvsp[-1]));}
#line 12588 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 887:
#line 3076 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12594 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 888:
#line 3077 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12600 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 889:
#line 3078 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12606 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 890:
#line 3079 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12612 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 891:
#line 3080 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12618 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 892:
#line 3081 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12624 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 893:
#line 3086 "hphp.y" /* yacc.c:1651  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12630 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 894:
#line 3087 "hphp.y" /* yacc.c:1651  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12636 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 895:
#line 3092 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 12642 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 896:
#line 3093 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 12648 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 897:
#line 3098 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 12654 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 898:
#line 3100 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,&(yyvsp[0]),0);}
#line 12660 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 899:
#line 3102 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 12666 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 900:
#line 3103 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,  0,&(yyvsp[0]),0);}
#line 12672 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 901:
#line 3107 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,&(yyvsp[0]),0);}
#line 12678 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 902:
#line 3108 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,  0,&(yyvsp[0]),0);}
#line 12684 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 903:
#line 3113 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); }
#line 12690 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 904:
#line 3114 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset(); }
#line 12696 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 905:
#line 3119 "hphp.y" /* yacc.c:1651  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0); }
#line 12702 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 906:
#line 3122 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0); }
#line 12708 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 907:
#line 3127 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 12714 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 908:
#line 3128 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 12720 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 909:
#line 3131 "hphp.y" /* yacc.c:1651  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12726 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 910:
#line 3132 "hphp.y" /* yacc.c:1651  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 12733 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 911:
#line 3139 "hphp.y" /* yacc.c:1651  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 12739 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 912:
#line 3141 "hphp.y" /* yacc.c:1651  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 12745 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 913:
#line 3144 "hphp.y" /* yacc.c:1651  */
    { only_in_hh_syntax(_p);}
#line 12751 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 914:
#line 3146 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 12757 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 915:
#line 3149 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 12763 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 916:
#line 3152 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12769 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 917:
#line 3153 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 12775 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 918:
#line 3157 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12781 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 919:
#line 3158 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12787 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 920:
#line 3162 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12793 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 921:
#line 3163 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12799 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 922:
#line 3164 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12805 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 923:
#line 3168 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12811 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 924:
#line 3173 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12817 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 925:
#line 3178 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12823 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 926:
#line 3179 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 12829 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 927:
#line 3183 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12835 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 928:
#line 3188 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12841 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 929:
#line 3193 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 12847 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 930:
#line 3194 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 12853 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 931:
#line 3199 "hphp.y" /* yacc.c:1651  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12859 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 932:
#line 3200 "hphp.y" /* yacc.c:1651  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12865 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 933:
#line 3202 "hphp.y" /* yacc.c:1651  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12871 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 934:
#line 3207 "hphp.y" /* yacc.c:1651  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12877 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 935:
#line 3209 "hphp.y" /* yacc.c:1651  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12883 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 936:
#line 3215 "hphp.y" /* yacc.c:1651  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12897 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 937:
#line 3226 "hphp.y" /* yacc.c:1651  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12911 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 938:
#line 3241 "hphp.y" /* yacc.c:1651  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12925 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 939:
#line 3253 "hphp.y" /* yacc.c:1651  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12939 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 940:
#line 3265 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12945 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 941:
#line 3266 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12951 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 942:
#line 3267 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12957 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 943:
#line 3268 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12963 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 944:
#line 3269 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12969 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 945:
#line 3270 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 12975 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 946:
#line 3272 "hphp.y" /* yacc.c:1651  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12989 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 947:
#line 3289 "hphp.y" /* yacc.c:1651  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12995 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 948:
#line 3291 "hphp.y" /* yacc.c:1651  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13001 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 949:
#line 3293 "hphp.y" /* yacc.c:1651  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13007 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 950:
#line 3294 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 13013 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 951:
#line 3298 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); }
#line 13019 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 952:
#line 3302 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13025 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 953:
#line 3303 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13031 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 954:
#line 3304 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13037 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 955:
#line 3305 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13043 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 956:
#line 3313 "hphp.y" /* yacc.c:1651  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13057 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 957:
#line 3322 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13063 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 958:
#line 3324 "hphp.y" /* yacc.c:1651  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13069 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 959:
#line 3328 "hphp.y" /* yacc.c:1651  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13075 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 960:
#line 3333 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13081 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 961:
#line 3334 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13087 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 962:
#line 3335 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13093 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 963:
#line 3336 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13099 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 964:
#line 3337 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13105 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 965:
#line 3338 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 13111 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 966:
#line 3339 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 13117 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 967:
#line 3341 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13123 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 968:
#line 3342 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13129 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 969:
#line 3345 "hphp.y" /* yacc.c:1651  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13135 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 970:
#line 3347 "hphp.y" /* yacc.c:1651  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13141 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 971:
#line 3351 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 13147 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 972:
#line 3355 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13153 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 973:
#line 3356 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13159 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 974:
#line 3362 "hphp.y" /* yacc.c:1651  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13165 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 975:
#line 3366 "hphp.y" /* yacc.c:1651  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13171 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 976:
#line 3370 "hphp.y" /* yacc.c:1651  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13177 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 977:
#line 3377 "hphp.y" /* yacc.c:1651  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 13183 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 978:
#line 3386 "hphp.y" /* yacc.c:1651  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 13189 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 979:
#line 3390 "hphp.y" /* yacc.c:1651  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 13195 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 980:
#line 3394 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13201 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 981:
#line 3403 "hphp.y" /* yacc.c:1651  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13207 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 982:
#line 3404 "hphp.y" /* yacc.c:1651  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13213 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 983:
#line 3405 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13219 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 984:
#line 3409 "hphp.y" /* yacc.c:1651  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13225 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 985:
#line 3410 "hphp.y" /* yacc.c:1651  */
    { _p->onPipeVariable((yyval));}
#line 13231 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 986:
#line 3411 "hphp.y" /* yacc.c:1651  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 13237 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 987:
#line 3413 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[-1]) = 1; _p->onIndirectRef((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 13243 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 988:
#line 3418 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13249 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 989:
#line 3419 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset();}
#line 13255 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 990:
#line 3430 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13261 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 991:
#line 3431 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13267 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 992:
#line 3432 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13273 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 993:
#line 3435 "hphp.y" /* yacc.c:1651  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13287 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 994:
#line 3446 "hphp.y" /* yacc.c:1651  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13293 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 995:
#line 3447 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 13299 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 997:
#line 3451 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13305 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 998:
#line 3452 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]);}
#line 13311 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 999:
#line 3455 "hphp.y" /* yacc.c:1651  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13325 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1000:
#line 3464 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 13331 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1001:
#line 3469 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),&(yyvsp[0]),1);}
#line 13337 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1002:
#line 3470 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 13343 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1003:
#line 3471 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,  0,&(yyvsp[0]),1);}
#line 13349 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1004:
#line 3472 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,  0,&(yyvsp[0]),0);}
#line 13355 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1005:
#line 3475 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 13361 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1006:
#line 3477 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),&(yyvsp[0]),1);}
#line 13367 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1007:
#line 3478 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,&(yyvsp[0]),1);}
#line 13373 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1008:
#line 3479 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,&(yyvsp[0]),0);}
#line 13379 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1009:
#line 3483 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),  0,  0,0);}
#line 13385 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1010:
#line 3485 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,  0,  0,0);}
#line 13391 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1011:
#line 3490 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 13397 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1012:
#line 3491 "hphp.y" /* yacc.c:1651  */
    { _p->onEmptyCollection((yyval));}
#line 13403 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1013:
#line 3495 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 13409 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1014:
#line 3496 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,&(yyvsp[0]),0);}
#line 13415 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1015:
#line 3497 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 13421 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1016:
#line 3498 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,  0,&(yyvsp[0]),0);}
#line 13427 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1017:
#line 3503 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 13433 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1018:
#line 3504 "hphp.y" /* yacc.c:1651  */
    { _p->onEmptyCollection((yyval));}
#line 13439 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1019:
#line 3509 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 13445 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1020:
#line 3511 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,&(yyvsp[0]),0);}
#line 13451 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1021:
#line 3513 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),&(yyvsp[0]),0);}
#line 13457 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1022:
#line 3514 "hphp.y" /* yacc.c:1651  */
    { _p->onArrayPair((yyval),  0,  0,&(yyvsp[0]),0);}
#line 13463 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1023:
#line 3518 "hphp.y" /* yacc.c:1651  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 13469 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1024:
#line 3520 "hphp.y" /* yacc.c:1651  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 13475 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1025:
#line 3521 "hphp.y" /* yacc.c:1651  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 13481 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1026:
#line 3523 "hphp.y" /* yacc.c:1651  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 13488 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1027:
#line 3528 "hphp.y" /* yacc.c:1651  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13494 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1028:
#line 3530 "hphp.y" /* yacc.c:1651  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13500 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1029:
#line 3532 "hphp.y" /* yacc.c:1651  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 13514 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1030:
#line 3542 "hphp.y" /* yacc.c:1651  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 13520 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1031:
#line 3544 "hphp.y" /* yacc.c:1651  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 13526 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1032:
#line 3545 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);}
#line 13532 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1033:
#line 3548 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 13538 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1034:
#line 3549 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 13544 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1035:
#line 3550 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 13550 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1036:
#line 3554 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 13556 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1037:
#line 3555 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 13562 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1038:
#line 3556 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13568 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1039:
#line 3557 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13574 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1040:
#line 3558 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13580 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1041:
#line 3559 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13586 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1042:
#line 3560 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 13592 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1043:
#line 3561 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 13598 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1044:
#line 3562 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 13604 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1045:
#line 3563 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 13610 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1046:
#line 3564 "hphp.y" /* yacc.c:1651  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 13616 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1047:
#line 3568 "hphp.y" /* yacc.c:1651  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 13622 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1048:
#line 3569 "hphp.y" /* yacc.c:1651  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 13628 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1049:
#line 3574 "hphp.y" /* yacc.c:1651  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13634 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1050:
#line 3576 "hphp.y" /* yacc.c:1651  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13640 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1053:
#line 3590 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13648 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1054:
#line 3595 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 13656 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1055:
#line 3599 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13664 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1056:
#line 3604 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 13672 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1057:
#line 3610 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 13678 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1058:
#line 3611 "hphp.y" /* yacc.c:1651  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13684 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1059:
#line 3615 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 13690 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1060:
#line 3616 "hphp.y" /* yacc.c:1651  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13696 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1061:
#line 3622 "hphp.y" /* yacc.c:1651  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13702 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1062:
#line 3626 "hphp.y" /* yacc.c:1651  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 13708 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1063:
#line 3632 "hphp.y" /* yacc.c:1651  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13714 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1064:
#line 3636 "hphp.y" /* yacc.c:1651  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 13721 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1065:
#line 3643 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); }
#line 13727 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1066:
#line 3644 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset(); }
#line 13733 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1067:
#line 3648 "hphp.y" /* yacc.c:1651  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 13741 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1068:
#line 3651 "hphp.y" /* yacc.c:1651  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 13748 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1069:
#line 3657 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); }
#line 13754 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1070:
#line 3661 "hphp.y" /* yacc.c:1651  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13762 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1071:
#line 3664 "hphp.y" /* yacc.c:1651  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13770 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1072:
#line 3667 "hphp.y" /* yacc.c:1651  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-3]); }
#line 13777 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1073:
#line 3669 "hphp.y" /* yacc.c:1651  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13784 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1074:
#line 3671 "hphp.y" /* yacc.c:1651  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13791 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1075:
#line 3673 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset(); }
#line 13797 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1076:
#line 3678 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-3]); }
#line 13803 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1077:
#line 3679 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); }
#line 13809 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1078:
#line 3680 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset(); }
#line 13815 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1079:
#line 3681 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset(); }
#line 13821 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1086:
#line 3702 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset(); }
#line 13827 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1087:
#line 3703 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13833 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1090:
#line 3712 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]); }
#line 13839 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1093:
#line 3723 "hphp.y" /* yacc.c:1651  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13845 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1094:
#line 3725 "hphp.y" /* yacc.c:1651  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13851 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1095:
#line 3729 "hphp.y" /* yacc.c:1651  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13857 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1096:
#line 3732 "hphp.y" /* yacc.c:1651  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13863 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1097:
#line 3736 "hphp.y" /* yacc.c:1651  */
    {}
#line 13869 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1098:
#line 3737 "hphp.y" /* yacc.c:1651  */
    {}
#line 13875 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1099:
#line 3738 "hphp.y" /* yacc.c:1651  */
    {}
#line 13881 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1100:
#line 3744 "hphp.y" /* yacc.c:1651  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13888 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1101:
#line 3749 "hphp.y" /* yacc.c:1651  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13898 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1102:
#line 3758 "hphp.y" /* yacc.c:1651  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13904 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1103:
#line 3764 "hphp.y" /* yacc.c:1651  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13913 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1104:
#line 3772 "hphp.y" /* yacc.c:1651  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13919 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1105:
#line 3773 "hphp.y" /* yacc.c:1651  */
    { }
#line 13925 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1106:
#line 3779 "hphp.y" /* yacc.c:1651  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13931 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1107:
#line 3781 "hphp.y" /* yacc.c:1651  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13937 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1108:
#line 3782 "hphp.y" /* yacc.c:1651  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13947 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1109:
#line 3787 "hphp.y" /* yacc.c:1651  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13954 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1110:
#line 3793 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("HH\\darray"); }
#line 13961 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1111:
#line 3798 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 13967 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1112:
#line 3803 "hphp.y" /* yacc.c:1651  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13975 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1113:
#line 3807 "hphp.y" /* yacc.c:1651  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13981 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1114:
#line 3812 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[-2]);}
#line 13987 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1115:
#line 3814 "hphp.y" /* yacc.c:1651  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 13993 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1116:
#line 3820 "hphp.y" /* yacc.c:1651  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 14000 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1117:
#line 3822 "hphp.y" /* yacc.c:1651  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 14008 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1118:
#line 3825 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 14014 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1119:
#line 3826 "hphp.y" /* yacc.c:1651  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14022 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1120:
#line 3829 "hphp.y" /* yacc.c:1651  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14030 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1121:
#line 3832 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 14036 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1122:
#line 3835 "hphp.y" /* yacc.c:1651  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 14044 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1123:
#line 3838 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14051 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1124:
#line 3840 "hphp.y" /* yacc.c:1651  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 14060 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1125:
#line 3846 "hphp.y" /* yacc.c:1651  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 14069 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1126:
#line 3852 "hphp.y" /* yacc.c:1651  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("HH\\varray");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 14079 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1127:
#line 3860 "hphp.y" /* yacc.c:1651  */
    { (yyval) = (yyvsp[0]); }
#line 14085 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;

  case 1128:
#line 3861 "hphp.y" /* yacc.c:1651  */
    { (yyval).reset(); }
#line 14091 "hphp.7.tab.cpp" /* yacc.c:1651  */
    break;


#line 14095 "hphp.7.tab.cpp" /* yacc.c:1651  */
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
#line 3864 "hphp.y" /* yacc.c:1910  */

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}
