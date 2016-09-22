// @generated

/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 1

/* Substitute the variable and function names.  */
#define yyparse         Compiler7parse
#define yylex           Compiler7lex
#define yyerror         Compiler7error
#define yylval          Compiler7lval
#define yychar          Compiler7char
#define yydebug         Compiler7debug
#define yynerrs         Compiler7nerrs
#define yylloc          Compiler7lloc

/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "hphp.y"


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
    Token fname;   fname.setText("__xhpAttributeDeclaration");
    Token param1;  _p->onCall(param1, 0, fname, dummy, &cls);
    Token params1; _p->onCallParam(params1, NULL, param1, false, false);

    for (unsigned int i = 0; i < classes.size(); i++) {
      Token parent;  parent.set(T_STRING, classes[i]);
      Token cls;     _p->onName(cls, parent, Parser::StringName);
      Token fname;   fname.setText("__xhpAttributeDeclaration");
      Token param;   _p->onCall(param, 0, fname, dummy, &cls);

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


/* Line 189 of yacc.c  */
#line 666 "hphp.7.tab.cpp"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_REQUIRE_ONCE = 258,
     T_REQUIRE = 259,
     T_EVAL = 260,
     T_INCLUDE_ONCE = 261,
     T_INCLUDE = 262,
     T_LAMBDA_ARROW = 263,
     T_LOGICAL_OR = 264,
     T_LOGICAL_XOR = 265,
     T_LOGICAL_AND = 266,
     T_PRINT = 267,
     T_POW_EQUAL = 268,
     T_SR_EQUAL = 269,
     T_SL_EQUAL = 270,
     T_XOR_EQUAL = 271,
     T_OR_EQUAL = 272,
     T_AND_EQUAL = 273,
     T_MOD_EQUAL = 274,
     T_CONCAT_EQUAL = 275,
     T_DIV_EQUAL = 276,
     T_MUL_EQUAL = 277,
     T_MINUS_EQUAL = 278,
     T_PLUS_EQUAL = 279,
     T_YIELD = 280,
     T_AWAIT = 281,
     T_YIELD_FROM = 282,
     T_PIPE = 283,
     T_COALESCE = 284,
     T_BOOLEAN_OR = 285,
     T_BOOLEAN_AND = 286,
     T_IS_NOT_IDENTICAL = 287,
     T_IS_IDENTICAL = 288,
     T_IS_NOT_EQUAL = 289,
     T_IS_EQUAL = 290,
     T_SPACESHIP = 291,
     T_IS_GREATER_OR_EQUAL = 292,
     T_IS_SMALLER_OR_EQUAL = 293,
     T_SR = 294,
     T_SL = 295,
     T_INSTANCEOF = 296,
     T_UNSET_CAST = 297,
     T_BOOL_CAST = 298,
     T_OBJECT_CAST = 299,
     T_ARRAY_CAST = 300,
     T_STRING_CAST = 301,
     T_DOUBLE_CAST = 302,
     T_INT_CAST = 303,
     T_DEC = 304,
     T_INC = 305,
     T_POW = 306,
     T_CLONE = 307,
     T_NEW = 308,
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
     T_PUBLIC = 355,
     T_PROTECTED = 356,
     T_PRIVATE = 357,
     T_FINAL = 358,
     T_ABSTRACT = 359,
     T_STATIC = 360,
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
     T_KEYSET = 377,
     T_CALLABLE = 378,
     T_CLASS_C = 379,
     T_METHOD_C = 380,
     T_FUNC_C = 381,
     T_LINE = 382,
     T_FILE = 383,
     T_COMMENT = 384,
     T_DOC_COMMENT = 385,
     T_OPEN_TAG = 386,
     T_OPEN_TAG_WITH_ECHO = 387,
     T_CLOSE_TAG = 388,
     T_WHITESPACE = 389,
     T_START_HEREDOC = 390,
     T_END_HEREDOC = 391,
     T_DOLLAR_OPEN_CURLY_BRACES = 392,
     T_CURLY_OPEN = 393,
     T_DOUBLE_COLON = 394,
     T_NAMESPACE = 395,
     T_NS_C = 396,
     T_DIR = 397,
     T_NS_SEPARATOR = 398,
     T_XHP_LABEL = 399,
     T_XHP_TEXT = 400,
     T_XHP_ATTRIBUTE = 401,
     T_XHP_CATEGORY = 402,
     T_XHP_CATEGORY_LABEL = 403,
     T_XHP_CHILDREN = 404,
     T_ENUM = 405,
     T_XHP_REQUIRED = 406,
     T_TRAIT = 407,
     T_ELLIPSIS = 408,
     T_INSTEADOF = 409,
     T_TRAIT_C = 410,
     T_HH_ERROR = 411,
     T_FINALLY = 412,
     T_XHP_TAG_LT = 413,
     T_XHP_TAG_GT = 414,
     T_TYPELIST_LT = 415,
     T_TYPELIST_GT = 416,
     T_UNRESOLVED_LT = 417,
     T_COLLECTION = 418,
     T_SHAPE = 419,
     T_TYPE = 420,
     T_UNRESOLVED_TYPE = 421,
     T_NEWTYPE = 422,
     T_UNRESOLVED_NEWTYPE = 423,
     T_COMPILER_HALT_OFFSET = 424,
     T_ASYNC = 425,
     T_LAMBDA_OP = 426,
     T_LAMBDA_CP = 427,
     T_UNRESOLVED_OP = 428,
     T_WHERE = 429
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int line0;
  int char0;
  int line1;
  int char1;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 895 "hphp.7.tab.cpp"

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
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
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
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
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
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
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
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   18494

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  204
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  294
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1055
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1938

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   429

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,   202,     2,   199,    55,    38,   203,
     194,   195,    53,    50,     9,    51,    52,    54,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    32,   196,
      43,    14,    44,    31,    59,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    70,     2,   201,    37,     2,   200,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   197,    36,   198,    58,     2,     2,     2,
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
      42,    45,    46,    47,    48,    49,    57,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    71,    72,    73,
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
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    11,    13,    15,    17,
      19,    21,    23,    28,    32,    33,    40,    41,    47,    51,
      56,    61,    68,    76,    84,    87,    89,    91,    93,    95,
      97,    99,   101,   103,   105,   107,   109,   111,   113,   115,
     117,   119,   121,   123,   125,   127,   129,   131,   133,   135,
     137,   139,   141,   143,   145,   147,   149,   151,   153,   155,
     157,   159,   161,   163,   165,   167,   169,   171,   173,   175,
     177,   179,   181,   183,   185,   187,   189,   191,   193,   195,
     197,   199,   201,   203,   205,   207,   209,   211,   213,   215,
     217,   219,   221,   223,   225,   227,   229,   231,   233,   235,
     237,   239,   241,   244,   248,   252,   254,   257,   259,   262,
     266,   271,   275,   277,   280,   282,   285,   288,   290,   294,
     296,   300,   303,   306,   309,   315,   320,   323,   324,   326,
     328,   330,   332,   336,   342,   351,   352,   357,   358,   365,
     366,   377,   378,   383,   386,   390,   393,   397,   400,   404,
     408,   412,   416,   420,   424,   430,   432,   434,   436,   437,
     447,   448,   459,   465,   466,   480,   481,   487,   491,   495,
     498,   501,   504,   507,   510,   513,   517,   520,   523,   527,
     530,   533,   534,   539,   549,   550,   551,   556,   559,   560,
     562,   563,   565,   566,   576,   577,   588,   589,   601,   602,
     612,   613,   624,   625,   634,   635,   645,   646,   654,   655,
     664,   665,   674,   675,   683,   684,   693,   695,   697,   699,
     701,   703,   706,   710,   714,   717,   720,   721,   724,   725,
     728,   729,   731,   735,   737,   741,   744,   745,   747,   750,
     755,   757,   762,   764,   769,   771,   776,   778,   783,   787,
     793,   797,   802,   807,   813,   819,   824,   825,   827,   829,
     834,   835,   841,   842,   845,   846,   850,   851,   859,   868,
     875,   878,   884,   891,   896,   897,   902,   908,   916,   923,
     930,   938,   948,   957,   964,   972,   978,   981,   986,   992,
     996,   997,  1001,  1006,  1013,  1019,  1025,  1032,  1041,  1049,
    1052,  1053,  1055,  1058,  1061,  1065,  1070,  1075,  1079,  1081,
    1083,  1086,  1091,  1095,  1101,  1103,  1107,  1110,  1111,  1114,
    1118,  1121,  1122,  1123,  1128,  1129,  1135,  1138,  1141,  1144,
    1145,  1157,  1158,  1171,  1175,  1179,  1183,  1188,  1193,  1197,
    1203,  1206,  1209,  1210,  1217,  1223,  1228,  1232,  1234,  1236,
    1240,  1245,  1247,  1250,  1252,  1254,  1260,  1267,  1269,  1271,
    1276,  1278,  1280,  1284,  1287,  1290,  1291,  1294,  1295,  1297,
    1301,  1303,  1305,  1307,  1309,  1313,  1318,  1323,  1328,  1330,
    1332,  1335,  1338,  1341,  1345,  1349,  1351,  1353,  1355,  1357,
    1361,  1363,  1367,  1369,  1371,  1373,  1374,  1376,  1379,  1381,
    1383,  1385,  1387,  1389,  1391,  1393,  1395,  1396,  1398,  1400,
    1402,  1406,  1412,  1414,  1418,  1424,  1429,  1433,  1437,  1441,
    1446,  1450,  1454,  1458,  1461,  1464,  1466,  1468,  1472,  1476,
    1478,  1480,  1481,  1483,  1486,  1491,  1495,  1499,  1506,  1509,
    1513,  1516,  1520,  1527,  1529,  1531,  1533,  1535,  1537,  1544,
    1548,  1553,  1560,  1564,  1568,  1572,  1576,  1580,  1584,  1588,
    1592,  1596,  1600,  1604,  1608,  1611,  1614,  1617,  1620,  1624,
    1628,  1632,  1636,  1640,  1644,  1648,  1652,  1656,  1660,  1664,
    1668,  1672,  1676,  1680,  1684,  1688,  1692,  1695,  1698,  1701,
    1704,  1708,  1712,  1716,  1720,  1724,  1728,  1732,  1736,  1740,
    1744,  1748,  1754,  1759,  1763,  1765,  1768,  1771,  1774,  1777,
    1780,  1783,  1786,  1789,  1792,  1794,  1796,  1798,  1800,  1802,
    1804,  1808,  1811,  1813,  1819,  1820,  1821,  1834,  1835,  1849,
    1850,  1855,  1856,  1864,  1865,  1871,  1872,  1876,  1877,  1884,
    1887,  1890,  1895,  1897,  1899,  1905,  1909,  1915,  1919,  1922,
    1923,  1926,  1927,  1932,  1937,  1941,  1944,  1945,  1951,  1955,
    1962,  1967,  1970,  1971,  1977,  1981,  1984,  1985,  1991,  1995,
    2000,  2005,  2010,  2015,  2020,  2025,  2030,  2035,  2040,  2043,
    2044,  2047,  2048,  2051,  2052,  2057,  2062,  2067,  2072,  2074,
    2076,  2078,  2080,  2082,  2084,  2086,  2090,  2092,  2096,  2101,
    2103,  2106,  2111,  2114,  2121,  2122,  2124,  2129,  2130,  2133,
    2134,  2136,  2138,  2142,  2144,  2148,  2150,  2152,  2156,  2160,
    2162,  2164,  2166,  2168,  2170,  2172,  2174,  2176,  2178,  2180,
    2182,  2184,  2186,  2188,  2190,  2192,  2194,  2196,  2198,  2200,
    2202,  2204,  2206,  2208,  2210,  2212,  2214,  2216,  2218,  2220,
    2222,  2224,  2226,  2228,  2230,  2232,  2234,  2236,  2238,  2240,
    2242,  2244,  2246,  2248,  2250,  2252,  2254,  2256,  2258,  2260,
    2262,  2264,  2266,  2268,  2270,  2272,  2274,  2276,  2278,  2280,
    2282,  2284,  2286,  2288,  2290,  2292,  2294,  2296,  2298,  2300,
    2302,  2304,  2306,  2308,  2310,  2312,  2314,  2316,  2318,  2320,
    2322,  2327,  2329,  2331,  2333,  2335,  2337,  2339,  2343,  2345,
    2349,  2351,  2353,  2357,  2359,  2361,  2363,  2366,  2368,  2369,
    2370,  2372,  2374,  2378,  2379,  2381,  2383,  2385,  2387,  2389,
    2391,  2393,  2395,  2397,  2399,  2401,  2403,  2405,  2409,  2412,
    2414,  2416,  2421,  2425,  2430,  2432,  2434,  2436,  2438,  2440,
    2444,  2448,  2452,  2456,  2460,  2464,  2468,  2472,  2476,  2480,
    2484,  2488,  2492,  2496,  2500,  2504,  2508,  2512,  2515,  2518,
    2521,  2524,  2528,  2532,  2536,  2540,  2544,  2548,  2552,  2556,
    2560,  2566,  2571,  2575,  2577,  2581,  2585,  2589,  2593,  2595,
    2597,  2599,  2601,  2605,  2609,  2613,  2616,  2617,  2619,  2620,
    2622,  2623,  2629,  2633,  2637,  2639,  2641,  2643,  2645,  2649,
    2652,  2654,  2656,  2658,  2660,  2662,  2666,  2668,  2670,  2672,
    2675,  2678,  2683,  2687,  2692,  2694,  2696,  2698,  2702,  2704,
    2707,  2708,  2714,  2718,  2722,  2724,  2728,  2730,  2733,  2734,
    2740,  2744,  2747,  2748,  2752,  2753,  2758,  2761,  2762,  2766,
    2770,  2772,  2773,  2775,  2777,  2779,  2781,  2785,  2787,  2789,
    2791,  2795,  2797,  2799,  2803,  2807,  2810,  2815,  2818,  2823,
    2829,  2835,  2841,  2847,  2849,  2851,  2853,  2855,  2857,  2859,
    2863,  2867,  2872,  2877,  2881,  2883,  2885,  2887,  2889,  2893,
    2895,  2900,  2904,  2908,  2910,  2912,  2914,  2916,  2918,  2922,
    2926,  2931,  2936,  2940,  2942,  2944,  2952,  2962,  2970,  2977,
    2986,  2988,  2993,  2998,  3000,  3002,  3004,  3009,  3012,  3014,
    3015,  3017,  3019,  3021,  3025,  3029,  3033,  3034,  3036,  3038,
    3042,  3046,  3049,  3053,  3060,  3061,  3063,  3068,  3071,  3072,
    3078,  3082,  3086,  3088,  3095,  3100,  3105,  3108,  3111,  3112,
    3118,  3122,  3126,  3128,  3131,  3132,  3138,  3142,  3146,  3148,
    3151,  3154,  3156,  3159,  3161,  3166,  3170,  3174,  3181,  3185,
    3187,  3189,  3191,  3196,  3201,  3206,  3211,  3216,  3221,  3224,
    3227,  3232,  3235,  3238,  3240,  3244,  3248,  3252,  3253,  3256,
    3262,  3269,  3276,  3284,  3286,  3289,  3291,  3294,  3296,  3301,
    3303,  3308,  3312,  3313,  3315,  3319,  3322,  3326,  3328,  3330,
    3331,  3332,  3336,  3338,  3342,  3346,  3349,  3350,  3353,  3356,
    3359,  3362,  3364,  3367,  3372,  3375,  3381,  3385,  3387,  3389,
    3390,  3394,  3399,  3405,  3409,  3411,  3414,  3415,  3420,  3422,
    3426,  3429,  3434,  3440,  3443,  3446,  3448,  3450,  3452,  3454,
    3458,  3461,  3463,  3472,  3479,  3481
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     205,     0,    -1,    -1,   206,   207,    -1,   207,   208,    -1,
      -1,   228,    -1,   245,    -1,   252,    -1,   249,    -1,   259,
      -1,   471,    -1,   129,   194,   195,   196,    -1,   159,   221,
     196,    -1,    -1,   159,   221,   197,   209,   207,   198,    -1,
      -1,   159,   197,   210,   207,   198,    -1,   117,   216,   196,
      -1,   117,   111,   216,   196,    -1,   117,   112,   216,   196,
      -1,   117,   214,   197,   219,   198,   196,    -1,   117,   111,
     214,   197,   216,   198,   196,    -1,   117,   112,   214,   197,
     216,   198,   196,    -1,   225,   196,    -1,    81,    -1,   103,
      -1,   193,    -1,   165,    -1,   166,    -1,   168,    -1,   170,
      -1,   169,    -1,   139,    -1,   140,    -1,   141,    -1,   211,
      -1,   142,    -1,   171,    -1,   132,    -1,   133,    -1,   124,
      -1,   123,    -1,   122,    -1,   121,    -1,   120,    -1,   119,
      -1,   112,    -1,   101,    -1,    97,    -1,    99,    -1,    77,
      -1,    95,    -1,    12,    -1,   118,    -1,   109,    -1,    57,
      -1,   173,    -1,   131,    -1,   159,    -1,    72,    -1,    10,
      -1,    11,    -1,   114,    -1,   117,    -1,   125,    -1,    73,
      -1,   137,    -1,    71,    -1,     7,    -1,     6,    -1,   116,
      -1,   138,    -1,    13,    -1,    92,    -1,     4,    -1,     3,
      -1,   113,    -1,    76,    -1,    75,    -1,   107,    -1,   108,
      -1,   110,    -1,   104,    -1,    27,    -1,    29,    -1,   111,
      -1,    74,    -1,   105,    -1,   176,    -1,    96,    -1,    98,
      -1,   100,    -1,   106,    -1,    93,    -1,    94,    -1,   102,
      -1,   115,    -1,   128,    -1,   126,    -1,   212,    -1,   130,
      -1,   221,   162,    -1,   162,   221,   162,    -1,   215,     9,
     217,    -1,   217,    -1,   215,   415,    -1,   221,    -1,   162,
     221,    -1,   221,   102,   211,    -1,   162,   221,   102,   211,
      -1,   218,     9,   220,    -1,   220,    -1,   218,   415,    -1,
     217,    -1,   111,   217,    -1,   112,   217,    -1,   211,    -1,
     221,   162,   211,    -1,   221,    -1,   159,   162,   221,    -1,
     162,   221,    -1,   222,   476,    -1,   222,   476,    -1,   225,
       9,   472,    14,   409,    -1,   112,   472,    14,   409,    -1,
     226,   227,    -1,    -1,   228,    -1,   245,    -1,   252,    -1,
     259,    -1,   197,   226,   198,    -1,    74,   335,   228,   281,
     283,    -1,    74,   335,    32,   226,   282,   284,    77,   196,
      -1,    -1,    94,   335,   229,   275,    -1,    -1,    93,   230,
     228,    94,   335,   196,    -1,    -1,    96,   194,   337,   196,
     337,   196,   337,   195,   231,   273,    -1,    -1,   104,   335,
     232,   278,    -1,   108,   196,    -1,   108,   346,   196,    -1,
     110,   196,    -1,   110,   346,   196,    -1,   113,   196,    -1,
     113,   346,   196,    -1,    27,   108,   196,    -1,   118,   291,
     196,    -1,   124,   293,   196,    -1,    92,   336,   196,    -1,
     151,   336,   196,    -1,   126,   194,   468,   195,   196,    -1,
     196,    -1,    86,    -1,    87,    -1,    -1,    98,   194,   346,
     102,   272,   271,   195,   233,   274,    -1,    -1,    98,   194,
     346,    28,   102,   272,   271,   195,   234,   274,    -1,   100,
     194,   277,   195,   276,    -1,    -1,   114,   237,   115,   194,
     400,    83,   195,   197,   226,   198,   239,   235,   242,    -1,
      -1,   114,   237,   176,   236,   240,    -1,   116,   346,   196,
      -1,   109,   211,   196,    -1,   346,   196,    -1,   338,   196,
      -1,   339,   196,    -1,   340,   196,    -1,   341,   196,    -1,
     342,   196,    -1,   113,   341,   196,    -1,   343,   196,    -1,
     344,   196,    -1,   113,   343,   196,    -1,   345,   196,    -1,
     211,    32,    -1,    -1,   197,   238,   226,   198,    -1,   239,
     115,   194,   400,    83,   195,   197,   226,   198,    -1,    -1,
      -1,   197,   241,   226,   198,    -1,   176,   240,    -1,    -1,
      38,    -1,    -1,   111,    -1,    -1,   244,   243,   475,   246,
     194,   287,   195,   483,   321,    -1,    -1,   325,   244,   243,
     475,   247,   194,   287,   195,   483,   321,    -1,    -1,   432,
     324,   244,   243,   475,   248,   194,   287,   195,   483,   321,
      -1,    -1,   169,   211,   250,    32,   496,   470,   197,   294,
     198,    -1,    -1,   432,   169,   211,   251,    32,   496,   470,
     197,   294,   198,    -1,    -1,   265,   262,   253,   266,   267,
     197,   297,   198,    -1,    -1,   432,   265,   262,   254,   266,
     267,   197,   297,   198,    -1,    -1,   131,   263,   255,   268,
     197,   297,   198,    -1,    -1,   432,   131,   263,   256,   268,
     197,   297,   198,    -1,    -1,   130,   258,   407,   266,   267,
     197,   297,   198,    -1,    -1,   171,   264,   260,   267,   197,
     297,   198,    -1,    -1,   432,   171,   264,   261,   267,   197,
     297,   198,    -1,   475,    -1,   163,    -1,   475,    -1,   475,
      -1,   130,    -1,   123,   130,    -1,   123,   122,   130,    -1,
     122,   123,   130,    -1,   122,   130,    -1,   132,   400,    -1,
      -1,   133,   269,    -1,    -1,   132,   269,    -1,    -1,   400,
      -1,   269,     9,   400,    -1,   400,    -1,   270,     9,   400,
      -1,   136,   272,    -1,    -1,   444,    -1,    38,   444,    -1,
     137,   194,   457,   195,    -1,   228,    -1,    32,   226,    97,
     196,    -1,   228,    -1,    32,   226,    99,   196,    -1,   228,
      -1,    32,   226,    95,   196,    -1,   228,    -1,    32,   226,
     101,   196,    -1,   211,    14,   409,    -1,   277,     9,   211,
      14,   409,    -1,   197,   279,   198,    -1,   197,   196,   279,
     198,    -1,    32,   279,   105,   196,    -1,    32,   196,   279,
     105,   196,    -1,   279,   106,   346,   280,   226,    -1,   279,
     107,   280,   226,    -1,    -1,    32,    -1,   196,    -1,   281,
      75,   335,   228,    -1,    -1,   282,    75,   335,    32,   226,
      -1,    -1,    76,   228,    -1,    -1,    76,    32,   226,    -1,
      -1,   286,     9,   433,   327,   497,   172,    83,    -1,   286,
       9,   433,   327,   497,    38,   172,    83,    -1,   286,     9,
     433,   327,   497,   172,    -1,   286,   415,    -1,   433,   327,
     497,   172,    83,    -1,   433,   327,   497,    38,   172,    83,
      -1,   433,   327,   497,   172,    -1,    -1,   433,   327,   497,
      83,    -1,   433,   327,   497,    38,    83,    -1,   433,   327,
     497,    38,    83,    14,   346,    -1,   433,   327,   497,    83,
      14,   346,    -1,   286,     9,   433,   327,   497,    83,    -1,
     286,     9,   433,   327,   497,    38,    83,    -1,   286,     9,
     433,   327,   497,    38,    83,    14,   346,    -1,   286,     9,
     433,   327,   497,    83,    14,   346,    -1,   288,     9,   433,
     497,   172,    83,    -1,   288,     9,   433,   497,    38,   172,
      83,    -1,   288,     9,   433,   497,   172,    -1,   288,   415,
      -1,   433,   497,   172,    83,    -1,   433,   497,    38,   172,
      83,    -1,   433,   497,   172,    -1,    -1,   433,   497,    83,
      -1,   433,   497,    38,    83,    -1,   433,   497,    38,    83,
      14,   346,    -1,   433,   497,    83,    14,   346,    -1,   288,
       9,   433,   497,    83,    -1,   288,     9,   433,   497,    38,
      83,    -1,   288,     9,   433,   497,    38,    83,    14,   346,
      -1,   288,     9,   433,   497,    83,    14,   346,    -1,   290,
     415,    -1,    -1,   346,    -1,    38,   444,    -1,   172,   346,
      -1,   290,     9,   346,    -1,   290,     9,   172,   346,    -1,
     290,     9,    38,   444,    -1,   291,     9,   292,    -1,   292,
      -1,    83,    -1,   199,   444,    -1,   199,   197,   346,   198,
      -1,   293,     9,    83,    -1,   293,     9,    83,    14,   409,
      -1,    83,    -1,    83,    14,   409,    -1,   294,   295,    -1,
      -1,   296,   196,    -1,   473,    14,   409,    -1,   297,   298,
      -1,    -1,    -1,   323,   299,   329,   196,    -1,    -1,   325,
     496,   300,   329,   196,    -1,   330,   196,    -1,   331,   196,
      -1,   332,   196,    -1,    -1,   324,   244,   243,   474,   194,
     301,   285,   195,   483,   480,   322,    -1,    -1,   432,   324,
     244,   243,   475,   194,   302,   285,   195,   483,   480,   322,
      -1,   165,   307,   196,    -1,   166,   315,   196,    -1,   168,
     317,   196,    -1,     4,   132,   400,   196,    -1,     4,   133,
     400,   196,    -1,   117,   270,   196,    -1,   117,   270,   197,
     303,   198,    -1,   303,   304,    -1,   303,   305,    -1,    -1,
     224,   158,   211,   173,   270,   196,    -1,   306,   102,   324,
     211,   196,    -1,   306,   102,   325,   196,    -1,   224,   158,
     211,    -1,   211,    -1,   308,    -1,   307,     9,   308,    -1,
     309,   397,   313,   314,    -1,   163,    -1,    31,   310,    -1,
     310,    -1,   138,    -1,   138,   179,   496,   414,   180,    -1,
     138,   179,   496,     9,   496,   180,    -1,   400,    -1,   125,
      -1,   169,   197,   312,   198,    -1,   142,    -1,   408,    -1,
     311,     9,   408,    -1,   311,   414,    -1,    14,   409,    -1,
      -1,    59,   170,    -1,    -1,   316,    -1,   315,     9,   316,
      -1,   167,    -1,   318,    -1,   211,    -1,   128,    -1,   194,
     319,   195,    -1,   194,   319,   195,    53,    -1,   194,   319,
     195,    31,    -1,   194,   319,   195,    50,    -1,   318,    -1,
     320,    -1,   320,    53,    -1,   320,    31,    -1,   320,    50,
      -1,   319,     9,   319,    -1,   319,    36,   319,    -1,   211,
      -1,   163,    -1,   167,    -1,   196,    -1,   197,   226,   198,
      -1,   196,    -1,   197,   226,   198,    -1,   325,    -1,   125,
      -1,   325,    -1,    -1,   326,    -1,   325,   326,    -1,   119,
      -1,   120,    -1,   121,    -1,   124,    -1,   123,    -1,   122,
      -1,   189,    -1,   328,    -1,    -1,   119,    -1,   120,    -1,
     121,    -1,   329,     9,    83,    -1,   329,     9,    83,    14,
     409,    -1,    83,    -1,    83,    14,   409,    -1,   330,     9,
     473,    14,   409,    -1,   112,   473,    14,   409,    -1,   331,
       9,   473,    -1,   123,   112,   473,    -1,   123,   333,   470,
      -1,   333,   470,    14,   496,    -1,   112,   184,   475,    -1,
     194,   334,   195,    -1,    72,   404,   407,    -1,    72,   257,
      -1,    71,   346,    -1,   389,    -1,   384,    -1,   194,   346,
     195,    -1,   336,     9,   346,    -1,   346,    -1,   336,    -1,
      -1,    27,    -1,    27,   346,    -1,    27,   346,   136,   346,
      -1,   194,   338,   195,    -1,   444,    14,   338,    -1,   137,
     194,   457,   195,    14,   338,    -1,    29,   346,    -1,   444,
      14,   341,    -1,    28,   346,    -1,   444,    14,   343,    -1,
     137,   194,   457,   195,    14,   343,    -1,   347,    -1,   444,
      -1,   334,    -1,   448,    -1,   447,    -1,   137,   194,   457,
     195,    14,   346,    -1,   444,    14,   346,    -1,   444,    14,
      38,   444,    -1,   444,    14,    38,    72,   404,   407,    -1,
     444,    26,   346,    -1,   444,    25,   346,    -1,   444,    24,
     346,    -1,   444,    23,   346,    -1,   444,    22,   346,    -1,
     444,    21,   346,    -1,   444,    20,   346,    -1,   444,    19,
     346,    -1,   444,    18,   346,    -1,   444,    17,   346,    -1,
     444,    16,   346,    -1,   444,    15,   346,    -1,   444,    68,
      -1,    68,   444,    -1,   444,    67,    -1,    67,   444,    -1,
     346,    34,   346,    -1,   346,    35,   346,    -1,   346,    10,
     346,    -1,   346,    12,   346,    -1,   346,    11,   346,    -1,
     346,    36,   346,    -1,   346,    38,   346,    -1,   346,    37,
     346,    -1,   346,    52,   346,    -1,   346,    50,   346,    -1,
     346,    51,   346,    -1,   346,    53,   346,    -1,   346,    54,
     346,    -1,   346,    69,   346,    -1,   346,    55,   346,    -1,
     346,    30,   346,    -1,   346,    49,   346,    -1,   346,    48,
     346,    -1,    50,   346,    -1,    51,   346,    -1,    56,   346,
      -1,    58,   346,    -1,   346,    40,   346,    -1,   346,    39,
     346,    -1,   346,    42,   346,    -1,   346,    41,   346,    -1,
     346,    43,   346,    -1,   346,    47,   346,    -1,   346,    44,
     346,    -1,   346,    46,   346,    -1,   346,    45,   346,    -1,
     346,    57,   404,    -1,   194,   347,   195,    -1,   346,    31,
     346,    32,   346,    -1,   346,    31,    32,   346,    -1,   346,
      33,   346,    -1,   467,    -1,    66,   346,    -1,    65,   346,
      -1,    64,   346,    -1,    63,   346,    -1,    62,   346,    -1,
      61,   346,    -1,    60,   346,    -1,    73,   405,    -1,    59,
     346,    -1,   412,    -1,   365,    -1,   372,    -1,   375,    -1,
     378,    -1,   364,    -1,   200,   406,   200,    -1,    13,   346,
      -1,   386,    -1,   117,   194,   388,   415,   195,    -1,    -1,
      -1,   244,   243,   194,   350,   287,   195,   483,   348,   483,
     197,   226,   198,    -1,    -1,   325,   244,   243,   194,   351,
     287,   195,   483,   348,   483,   197,   226,   198,    -1,    -1,
     189,    83,   353,   358,    -1,    -1,   189,   190,   354,   287,
     191,   483,   358,    -1,    -1,   189,   197,   355,   226,   198,
      -1,    -1,    83,   356,   358,    -1,    -1,   190,   357,   287,
     191,   483,   358,    -1,     8,   346,    -1,     8,   343,    -1,
       8,   197,   226,   198,    -1,    91,    -1,   469,    -1,   360,
       9,   359,   136,   346,    -1,   359,   136,   346,    -1,   361,
       9,   359,   136,   409,    -1,   359,   136,   409,    -1,   360,
     414,    -1,    -1,   361,   414,    -1,    -1,   183,   194,   362,
     195,    -1,   138,   194,   458,   195,    -1,    70,   458,   201,
      -1,   367,   414,    -1,    -1,   367,     9,   346,   136,   346,
      -1,   346,   136,   346,    -1,   367,     9,   346,   136,    38,
     444,    -1,   346,   136,    38,   444,    -1,   369,   414,    -1,
      -1,   369,     9,   409,   136,   409,    -1,   409,   136,   409,
      -1,   371,   414,    -1,    -1,   371,     9,   420,   136,   420,
      -1,   420,   136,   420,    -1,   139,    70,   366,   201,    -1,
     139,    70,   368,   201,    -1,   139,    70,   370,   201,    -1,
     140,    70,   381,   201,    -1,   140,    70,   382,   201,    -1,
     140,    70,   383,   201,    -1,   141,    70,   381,   201,    -1,
     141,    70,   382,   201,    -1,   141,    70,   383,   201,    -1,
     336,   414,    -1,    -1,   410,   414,    -1,    -1,   421,   414,
      -1,    -1,   400,   197,   460,   198,    -1,   400,   197,   462,
     198,    -1,   386,    70,   454,   201,    -1,   387,    70,   454,
     201,    -1,   365,    -1,   372,    -1,   375,    -1,   378,    -1,
     469,    -1,   447,    -1,    91,    -1,   194,   347,   195,    -1,
      81,    -1,   388,     9,    83,    -1,   388,     9,    38,    83,
      -1,    83,    -1,    38,    83,    -1,   177,   163,   390,   178,
      -1,   392,    54,    -1,   392,   178,   393,   177,    54,   391,
      -1,    -1,   163,    -1,   392,   394,    14,   395,    -1,    -1,
     393,   396,    -1,    -1,   163,    -1,   164,    -1,   197,   346,
     198,    -1,   164,    -1,   197,   346,   198,    -1,   389,    -1,
     398,    -1,   397,    32,   398,    -1,   397,    51,   398,    -1,
     211,    -1,    73,    -1,   111,    -1,   112,    -1,   113,    -1,
      27,    -1,    29,    -1,    28,    -1,   114,    -1,   115,    -1,
     176,    -1,   116,    -1,    74,    -1,    75,    -1,    77,    -1,
      76,    -1,    94,    -1,    95,    -1,    93,    -1,    96,    -1,
      97,    -1,    98,    -1,    99,    -1,   100,    -1,   101,    -1,
      57,    -1,   102,    -1,   104,    -1,   105,    -1,   106,    -1,
     107,    -1,   108,    -1,   110,    -1,   109,    -1,    92,    -1,
      13,    -1,   130,    -1,   131,    -1,   132,    -1,   133,    -1,
      72,    -1,    71,    -1,   125,    -1,     5,    -1,     7,    -1,
       6,    -1,     4,    -1,     3,    -1,   159,    -1,   117,    -1,
     118,    -1,   127,    -1,   128,    -1,   129,    -1,   124,    -1,
     123,    -1,   122,    -1,   121,    -1,   120,    -1,   119,    -1,
     189,    -1,   126,    -1,   137,    -1,   138,    -1,    10,    -1,
      12,    -1,    11,    -1,   143,    -1,   145,    -1,   144,    -1,
     146,    -1,   147,    -1,   161,    -1,   160,    -1,   188,    -1,
     171,    -1,   174,    -1,   173,    -1,   184,    -1,   186,    -1,
     183,    -1,   223,   194,   289,   195,    -1,   224,    -1,   163,
      -1,   400,    -1,   408,    -1,   124,    -1,   452,    -1,   194,
     347,   195,    -1,   401,    -1,   402,   158,   453,    -1,   401,
      -1,   450,    -1,   403,   158,   453,    -1,   400,    -1,   124,
      -1,   455,    -1,   194,   195,    -1,   335,    -1,    -1,    -1,
      90,    -1,   464,    -1,   194,   289,   195,    -1,    -1,    78,
      -1,    79,    -1,    80,    -1,    91,    -1,   146,    -1,   147,
      -1,   161,    -1,   143,    -1,   174,    -1,   144,    -1,   145,
      -1,   160,    -1,   188,    -1,   154,    90,   155,    -1,   154,
     155,    -1,   408,    -1,   222,    -1,   138,   194,   413,   195,
      -1,    70,   413,   201,    -1,   183,   194,   363,   195,    -1,
     373,    -1,   376,    -1,   379,    -1,   411,    -1,   385,    -1,
     194,   409,   195,    -1,   409,    34,   409,    -1,   409,    35,
     409,    -1,   409,    10,   409,    -1,   409,    12,   409,    -1,
     409,    11,   409,    -1,   409,    36,   409,    -1,   409,    38,
     409,    -1,   409,    37,   409,    -1,   409,    52,   409,    -1,
     409,    50,   409,    -1,   409,    51,   409,    -1,   409,    53,
     409,    -1,   409,    54,   409,    -1,   409,    55,   409,    -1,
     409,    49,   409,    -1,   409,    48,   409,    -1,   409,    69,
     409,    -1,    56,   409,    -1,    58,   409,    -1,    50,   409,
      -1,    51,   409,    -1,   409,    40,   409,    -1,   409,    39,
     409,    -1,   409,    42,   409,    -1,   409,    41,   409,    -1,
     409,    43,   409,    -1,   409,    47,   409,    -1,   409,    44,
     409,    -1,   409,    46,   409,    -1,   409,    45,   409,    -1,
     409,    31,   409,    32,   409,    -1,   409,    31,    32,   409,
      -1,   410,     9,   409,    -1,   409,    -1,   224,   158,   212,
      -1,   163,   158,   212,    -1,   163,   158,   130,    -1,   224,
     158,   130,    -1,   222,    -1,    82,    -1,   469,    -1,   408,
      -1,   202,   464,   202,    -1,   203,   464,   203,    -1,   154,
     464,   155,    -1,   416,   414,    -1,    -1,     9,    -1,    -1,
       9,    -1,    -1,   416,     9,   409,   136,   409,    -1,   416,
       9,   409,    -1,   409,   136,   409,    -1,   409,    -1,    78,
      -1,    79,    -1,    80,    -1,   154,    90,   155,    -1,   154,
     155,    -1,    78,    -1,    79,    -1,    80,    -1,   211,    -1,
      91,    -1,    91,    52,   419,    -1,   417,    -1,   419,    -1,
     211,    -1,    50,   418,    -1,    51,   418,    -1,   138,   194,
     422,   195,    -1,    70,   422,   201,    -1,   183,   194,   425,
     195,    -1,   374,    -1,   377,    -1,   380,    -1,   421,     9,
     420,    -1,   420,    -1,   423,   414,    -1,    -1,   423,     9,
     420,   136,   420,    -1,   423,     9,   420,    -1,   420,   136,
     420,    -1,   420,    -1,   424,     9,   420,    -1,   420,    -1,
     426,   414,    -1,    -1,   426,     9,   359,   136,   420,    -1,
     359,   136,   420,    -1,   424,   414,    -1,    -1,   194,   427,
     195,    -1,    -1,   429,     9,   211,   428,    -1,   211,   428,
      -1,    -1,   431,   429,   414,    -1,    49,   430,    48,    -1,
     432,    -1,    -1,   134,    -1,   135,    -1,   211,    -1,   163,
      -1,   197,   346,   198,    -1,   435,    -1,   453,    -1,   211,
      -1,   197,   346,   198,    -1,   437,    -1,   453,    -1,    70,
     454,   201,    -1,   197,   346,   198,    -1,   445,   439,    -1,
     194,   334,   195,   439,    -1,   456,   439,    -1,   194,   334,
     195,   439,    -1,   194,   334,   195,   434,   436,    -1,   194,
     347,   195,   434,   436,    -1,   194,   334,   195,   434,   435,
      -1,   194,   347,   195,   434,   435,    -1,   451,    -1,   399,
      -1,   449,    -1,   450,    -1,   440,    -1,   442,    -1,   444,
     434,   436,    -1,   403,   158,   453,    -1,   446,   194,   289,
     195,    -1,   447,   194,   289,   195,    -1,   194,   444,   195,
      -1,   399,    -1,   449,    -1,   450,    -1,   440,    -1,   444,
     434,   436,    -1,   443,    -1,   446,   194,   289,   195,    -1,
     194,   444,   195,    -1,   403,   158,   453,    -1,   451,    -1,
     440,    -1,   399,    -1,   365,    -1,   408,    -1,   194,   444,
     195,    -1,   194,   347,   195,    -1,   447,   194,   289,   195,
      -1,   446,   194,   289,   195,    -1,   194,   448,   195,    -1,
     349,    -1,   352,    -1,   444,   434,   438,   476,   194,   289,
     195,    -1,   194,   334,   195,   434,   438,   476,   194,   289,
     195,    -1,   403,   158,   213,   476,   194,   289,   195,    -1,
     403,   158,   453,   194,   289,   195,    -1,   403,   158,   197,
     346,   198,   194,   289,   195,    -1,   452,    -1,   452,    70,
     454,   201,    -1,   452,   197,   346,   198,    -1,   453,    -1,
      83,    -1,    84,    -1,   199,   197,   346,   198,    -1,   199,
     453,    -1,   346,    -1,    -1,   451,    -1,   441,    -1,   442,
      -1,   455,   434,   436,    -1,   402,   158,   451,    -1,   194,
     444,   195,    -1,    -1,   441,    -1,   443,    -1,   455,   434,
     435,    -1,   194,   444,   195,    -1,   457,     9,    -1,   457,
       9,   444,    -1,   457,     9,   137,   194,   457,   195,    -1,
      -1,   444,    -1,   137,   194,   457,   195,    -1,   459,   414,
      -1,    -1,   459,     9,   346,   136,   346,    -1,   459,     9,
     346,    -1,   346,   136,   346,    -1,   346,    -1,   459,     9,
     346,   136,    38,   444,    -1,   459,     9,    38,   444,    -1,
     346,   136,    38,   444,    -1,    38,   444,    -1,   461,   414,
      -1,    -1,   461,     9,   346,   136,   346,    -1,   461,     9,
     346,    -1,   346,   136,   346,    -1,   346,    -1,   463,   414,
      -1,    -1,   463,     9,   409,   136,   409,    -1,   463,     9,
     409,    -1,   409,   136,   409,    -1,   409,    -1,   464,   465,
      -1,   464,    90,    -1,   465,    -1,    90,   465,    -1,    83,
      -1,    83,    70,   466,   201,    -1,    83,   434,   211,    -1,
     156,   346,   198,    -1,   156,    82,    70,   346,   201,   198,
      -1,   157,   444,   198,    -1,   211,    -1,    85,    -1,    83,
      -1,   127,   194,   336,   195,    -1,   128,   194,   444,   195,
      -1,   128,   194,   347,   195,    -1,   128,   194,   448,   195,
      -1,   128,   194,   447,   195,    -1,   128,   194,   334,   195,
      -1,     7,   346,    -1,     6,   346,    -1,     5,   194,   346,
     195,    -1,     4,   346,    -1,     3,   346,    -1,   444,    -1,
     468,     9,   444,    -1,   403,   158,   212,    -1,   403,   158,
     130,    -1,    -1,   102,   496,    -1,   184,   475,    14,   496,
     196,    -1,   432,   184,   475,    14,   496,   196,    -1,   186,
     475,   470,    14,   496,   196,    -1,   432,   186,   475,   470,
      14,   496,   196,    -1,   213,    -1,   496,   213,    -1,   212,
      -1,   496,   212,    -1,   213,    -1,   213,   179,   485,   180,
      -1,   211,    -1,   211,   179,   485,   180,    -1,   179,   478,
     180,    -1,    -1,   496,    -1,   477,     9,   496,    -1,   477,
     414,    -1,   477,     9,   172,    -1,   478,    -1,   172,    -1,
      -1,    -1,   193,   481,   415,    -1,   482,    -1,   481,     9,
     482,    -1,   496,    14,   496,    -1,   496,   484,    -1,    -1,
      32,   496,    -1,   102,   496,    -1,   103,   496,    -1,   487,
     414,    -1,   484,    -1,   486,   484,    -1,   487,     9,   488,
     211,    -1,   488,   211,    -1,   487,     9,   488,   211,   486,
      -1,   488,   211,   486,    -1,    50,    -1,    51,    -1,    -1,
      91,   136,   496,    -1,    31,    91,   136,   496,    -1,   224,
     158,   211,   136,   496,    -1,   490,     9,   489,    -1,   489,
      -1,   490,   414,    -1,    -1,   183,   194,   491,   195,    -1,
     224,    -1,   211,   158,   494,    -1,   211,   476,    -1,   179,
     496,   414,   180,    -1,   179,   496,     9,   496,   180,    -1,
      31,   496,    -1,    59,   496,    -1,   224,    -1,   138,    -1,
     142,    -1,   492,    -1,   493,   158,   494,    -1,   138,   495,
      -1,   163,    -1,   194,   111,   194,   479,   195,    32,   496,
     195,    -1,   194,   496,     9,   477,   414,   195,    -1,   496,
      -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   749,   749,   749,   758,   760,   763,   764,   765,   766,
     767,   768,   769,   772,   774,   774,   776,   776,   778,   780,
     783,   786,   790,   794,   798,   803,   804,   805,   806,   807,
     808,   809,   810,   811,   812,   813,   817,   818,   819,   820,
     821,   822,   823,   824,   825,   826,   827,   828,   829,   830,
     831,   832,   833,   834,   835,   836,   837,   838,   839,   840,
     841,   842,   843,   844,   845,   846,   847,   848,   849,   850,
     851,   852,   853,   854,   855,   856,   857,   858,   859,   860,
     861,   862,   863,   864,   865,   866,   867,   868,   869,   870,
     871,   872,   873,   874,   875,   876,   877,   878,   879,   883,
     887,   888,   892,   893,   898,   900,   905,   910,   911,   912,
     914,   919,   921,   926,   931,   933,   935,   940,   941,   945,
     946,   948,   952,   959,   966,   970,   976,   978,   981,   982,
     983,   984,   987,   988,   992,   997,   997,  1003,  1003,  1010,
    1009,  1015,  1015,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1038,  1036,
    1045,  1043,  1050,  1060,  1054,  1064,  1062,  1066,  1067,  1071,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1090,  1090,  1095,  1101,  1105,  1105,  1113,  1114,  1118,
    1119,  1123,  1129,  1127,  1142,  1139,  1155,  1152,  1169,  1168,
    1177,  1175,  1187,  1186,  1205,  1203,  1222,  1221,  1230,  1228,
    1239,  1239,  1246,  1245,  1257,  1255,  1268,  1269,  1273,  1276,
    1279,  1280,  1281,  1284,  1285,  1288,  1290,  1293,  1294,  1297,
    1298,  1301,  1302,  1306,  1307,  1312,  1313,  1316,  1317,  1318,
    1322,  1323,  1327,  1328,  1332,  1333,  1337,  1338,  1343,  1344,
    1350,  1351,  1352,  1353,  1356,  1359,  1361,  1364,  1365,  1369,
    1371,  1374,  1377,  1380,  1381,  1384,  1385,  1389,  1395,  1401,
    1408,  1410,  1415,  1420,  1426,  1430,  1434,  1438,  1443,  1448,
    1453,  1458,  1464,  1473,  1478,  1483,  1489,  1491,  1495,  1499,
    1504,  1508,  1511,  1514,  1518,  1522,  1526,  1530,  1535,  1543,
    1545,  1548,  1549,  1550,  1551,  1553,  1555,  1560,  1561,  1564,
    1565,  1566,  1570,  1571,  1573,  1574,  1578,  1580,  1583,  1587,
    1593,  1595,  1598,  1598,  1602,  1601,  1605,  1607,  1610,  1613,
    1611,  1628,  1624,  1639,  1641,  1643,  1645,  1647,  1649,  1651,
    1655,  1656,  1657,  1660,  1666,  1670,  1676,  1679,  1684,  1686,
    1691,  1696,  1700,  1701,  1705,  1706,  1708,  1710,  1716,  1717,
    1719,  1723,  1724,  1729,  1733,  1734,  1738,  1739,  1743,  1745,
    1751,  1756,  1757,  1759,  1763,  1764,  1765,  1766,  1770,  1771,
    1772,  1773,  1774,  1775,  1777,  1782,  1785,  1786,  1790,  1791,
    1795,  1796,  1799,  1800,  1803,  1804,  1807,  1808,  1812,  1813,
    1814,  1815,  1816,  1817,  1818,  1822,  1823,  1826,  1827,  1828,
    1831,  1833,  1835,  1836,  1839,  1841,  1845,  1847,  1851,  1855,
    1859,  1864,  1865,  1867,  1868,  1869,  1870,  1873,  1877,  1878,
    1882,  1883,  1887,  1888,  1889,  1890,  1894,  1898,  1903,  1907,
    1911,  1915,  1919,  1924,  1925,  1926,  1927,  1928,  1932,  1934,
    1935,  1936,  1939,  1940,  1941,  1942,  1943,  1944,  1945,  1946,
    1947,  1948,  1949,  1950,  1951,  1952,  1953,  1954,  1955,  1956,
    1957,  1958,  1959,  1960,  1961,  1962,  1963,  1964,  1965,  1966,
    1967,  1968,  1969,  1970,  1971,  1972,  1973,  1974,  1975,  1976,
    1977,  1978,  1979,  1980,  1981,  1982,  1984,  1985,  1987,  1988,
    1990,  1991,  1992,  1993,  1994,  1995,  1996,  1997,  1998,  1999,
    2000,  2001,  2002,  2003,  2004,  2005,  2006,  2007,  2008,  2009,
    2010,  2011,  2012,  2016,  2020,  2025,  2024,  2039,  2037,  2055,
    2054,  2073,  2072,  2091,  2090,  2108,  2108,  2123,  2123,  2141,
    2142,  2143,  2148,  2150,  2154,  2158,  2164,  2168,  2174,  2176,
    2180,  2182,  2186,  2190,  2191,  2195,  2197,  2201,  2203,  2204,
    2207,  2211,  2213,  2217,  2220,  2225,  2227,  2231,  2234,  2239,
    2243,  2247,  2251,  2255,  2259,  2263,  2267,  2271,  2275,  2277,
    2281,  2283,  2287,  2289,  2293,  2300,  2307,  2309,  2314,  2315,
    2316,  2317,  2318,  2319,  2320,  2322,  2323,  2327,  2328,  2329,
    2330,  2334,  2340,  2349,  2362,  2363,  2366,  2369,  2372,  2373,
    2376,  2380,  2383,  2386,  2393,  2394,  2398,  2399,  2401,  2406,
    2407,  2408,  2409,  2410,  2411,  2412,  2413,  2414,  2415,  2416,
    2417,  2418,  2419,  2420,  2421,  2422,  2423,  2424,  2425,  2426,
    2427,  2428,  2429,  2430,  2431,  2432,  2433,  2434,  2435,  2436,
    2437,  2438,  2439,  2440,  2441,  2442,  2443,  2444,  2445,  2446,
    2447,  2448,  2449,  2450,  2451,  2452,  2453,  2454,  2455,  2456,
    2457,  2458,  2459,  2460,  2461,  2462,  2463,  2464,  2465,  2466,
    2467,  2468,  2469,  2470,  2471,  2472,  2473,  2474,  2475,  2476,
    2477,  2478,  2479,  2480,  2481,  2482,  2483,  2484,  2485,  2486,
    2490,  2495,  2496,  2500,  2501,  2502,  2503,  2505,  2509,  2510,
    2521,  2522,  2524,  2536,  2537,  2538,  2542,  2543,  2544,  2548,
    2549,  2550,  2553,  2555,  2559,  2560,  2561,  2562,  2564,  2565,
    2566,  2567,  2568,  2569,  2570,  2571,  2572,  2573,  2576,  2581,
    2582,  2583,  2585,  2586,  2588,  2589,  2590,  2591,  2592,  2593,
    2594,  2596,  2598,  2600,  2602,  2604,  2605,  2606,  2607,  2608,
    2609,  2610,  2611,  2612,  2613,  2614,  2615,  2616,  2617,  2618,
    2619,  2620,  2622,  2624,  2626,  2628,  2629,  2632,  2633,  2637,
    2641,  2643,  2647,  2648,  2652,  2655,  2658,  2661,  2667,  2668,
    2669,  2670,  2671,  2672,  2673,  2678,  2680,  2684,  2685,  2688,
    2689,  2693,  2696,  2698,  2700,  2704,  2705,  2706,  2707,  2710,
    2714,  2715,  2716,  2717,  2721,  2723,  2730,  2731,  2732,  2733,
    2734,  2735,  2737,  2738,  2740,  2741,  2742,  2746,  2748,  2752,
    2754,  2757,  2760,  2762,  2764,  2767,  2769,  2773,  2775,  2778,
    2781,  2787,  2789,  2792,  2793,  2798,  2801,  2805,  2805,  2810,
    2813,  2814,  2818,  2819,  2823,  2824,  2825,  2829,  2834,  2839,
    2840,  2844,  2849,  2854,  2855,  2859,  2860,  2865,  2867,  2872,
    2883,  2897,  2909,  2924,  2925,  2926,  2927,  2928,  2929,  2930,
    2940,  2949,  2951,  2953,  2957,  2958,  2959,  2960,  2961,  2977,
    2978,  2980,  2982,  2989,  2990,  2991,  2992,  2993,  2994,  2995,
    2996,  2998,  3003,  3007,  3008,  3012,  3015,  3022,  3026,  3035,
    3042,  3050,  3052,  3053,  3057,  3058,  3059,  3061,  3066,  3067,
    3078,  3079,  3080,  3081,  3092,  3095,  3098,  3099,  3100,  3101,
    3112,  3116,  3117,  3118,  3120,  3121,  3122,  3126,  3128,  3131,
    3133,  3134,  3135,  3136,  3139,  3141,  3142,  3146,  3148,  3151,
    3153,  3154,  3155,  3159,  3161,  3164,  3167,  3169,  3171,  3175,
    3176,  3178,  3179,  3185,  3186,  3188,  3198,  3200,  3202,  3205,
    3206,  3207,  3211,  3212,  3213,  3214,  3215,  3216,  3217,  3218,
    3219,  3220,  3221,  3225,  3226,  3230,  3232,  3240,  3242,  3246,
    3250,  3255,  3259,  3267,  3268,  3272,  3273,  3279,  3280,  3289,
    3290,  3298,  3301,  3305,  3308,  3313,  3318,  3320,  3321,  3322,
    3325,  3327,  3333,  3334,  3338,  3339,  3343,  3344,  3348,  3349,
    3352,  3357,  3358,  3362,  3365,  3367,  3371,  3377,  3378,  3379,
    3383,  3387,  3397,  3405,  3407,  3411,  3413,  3418,  3424,  3427,
    3432,  3437,  3439,  3446,  3449,  3452,  3453,  3456,  3459,  3460,
    3465,  3467,  3471,  3477,  3487,  3488
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_REQUIRE_ONCE", "T_REQUIRE", "T_EVAL",
  "T_INCLUDE_ONCE", "T_INCLUDE", "T_LAMBDA_ARROW", "','", "T_LOGICAL_OR",
  "T_LOGICAL_XOR", "T_LOGICAL_AND", "T_PRINT", "'='", "T_POW_EQUAL",
  "T_SR_EQUAL", "T_SL_EQUAL", "T_XOR_EQUAL", "T_OR_EQUAL", "T_AND_EQUAL",
  "T_MOD_EQUAL", "T_CONCAT_EQUAL", "T_DIV_EQUAL", "T_MUL_EQUAL",
  "T_MINUS_EQUAL", "T_PLUS_EQUAL", "T_YIELD", "T_AWAIT", "T_YIELD_FROM",
  "T_PIPE", "'?'", "':'", "\"??\"", "T_BOOLEAN_OR", "T_BOOLEAN_AND", "'|'",
  "'^'", "'&'", "T_IS_NOT_IDENTICAL", "T_IS_IDENTICAL", "T_IS_NOT_EQUAL",
  "T_IS_EQUAL", "'<'", "'>'", "T_SPACESHIP", "T_IS_GREATER_OR_EQUAL",
  "T_IS_SMALLER_OR_EQUAL", "T_SR", "T_SL", "'+'", "'-'", "'.'", "'*'",
  "'/'", "'%'", "'!'", "T_INSTANCEOF", "'~'", "'@'", "T_UNSET_CAST",
  "T_BOOL_CAST", "T_OBJECT_CAST", "T_ARRAY_CAST", "T_STRING_CAST",
  "T_DOUBLE_CAST", "T_INT_CAST", "T_DEC", "T_INC", "T_POW", "'['",
  "T_CLONE", "T_NEW", "T_EXIT", "T_IF", "T_ELSEIF", "T_ELSE", "T_ENDIF",
  "T_LNUMBER", "T_DNUMBER", "T_ONUMBER", "T_STRING", "T_STRING_VARNAME",
  "T_VARIABLE", "T_PIPE_VAR", "T_NUM_STRING", "T_INLINE_HTML",
  "T_HASHBANG", "T_CHARACTER", "T_BAD_CHARACTER",
  "T_ENCAPSED_AND_WHITESPACE", "T_CONSTANT_ENCAPSED_STRING", "T_ECHO",
  "T_DO", "T_WHILE", "T_ENDWHILE", "T_FOR", "T_ENDFOR", "T_FOREACH",
  "T_ENDFOREACH", "T_DECLARE", "T_ENDDECLARE", "T_AS", "T_SUPER",
  "T_SWITCH", "T_ENDSWITCH", "T_CASE", "T_DEFAULT", "T_BREAK", "T_GOTO",
  "T_CONTINUE", "T_FUNCTION", "T_CONST", "T_RETURN", "T_TRY", "T_CATCH",
  "T_THROW", "T_USE", "T_GLOBAL", "T_PUBLIC", "T_PROTECTED", "T_PRIVATE",
  "T_FINAL", "T_ABSTRACT", "T_STATIC", "T_VAR", "T_UNSET", "T_ISSET",
  "T_EMPTY", "T_HALT_COMPILER", "T_CLASS", "T_INTERFACE", "T_EXTENDS",
  "T_IMPLEMENTS", "T_OBJECT_OPERATOR", "T_NULLSAFE_OBJECT_OPERATOR",
  "T_DOUBLE_ARROW", "T_LIST", "T_ARRAY", "T_DICT", "T_VEC", "T_KEYSET",
  "T_CALLABLE", "T_CLASS_C", "T_METHOD_C", "T_FUNC_C", "T_LINE", "T_FILE",
  "T_COMMENT", "T_DOC_COMMENT", "T_OPEN_TAG", "T_OPEN_TAG_WITH_ECHO",
  "T_CLOSE_TAG", "T_WHITESPACE", "T_START_HEREDOC", "T_END_HEREDOC",
  "T_DOLLAR_OPEN_CURLY_BRACES", "T_CURLY_OPEN", "T_DOUBLE_COLON",
  "T_NAMESPACE", "T_NS_C", "T_DIR", "T_NS_SEPARATOR", "T_XHP_LABEL",
  "T_XHP_TEXT", "T_XHP_ATTRIBUTE", "T_XHP_CATEGORY",
  "T_XHP_CATEGORY_LABEL", "T_XHP_CHILDREN", "T_ENUM", "T_XHP_REQUIRED",
  "T_TRAIT", "\"...\"", "T_INSTEADOF", "T_TRAIT_C", "T_HH_ERROR",
  "T_FINALLY", "T_XHP_TAG_LT", "T_XHP_TAG_GT", "T_TYPELIST_LT",
  "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION", "T_SHAPE", "T_TYPE",
  "T_UNRESOLVED_TYPE", "T_NEWTYPE", "T_UNRESOLVED_NEWTYPE",
  "T_COMPILER_HALT_OFFSET", "T_ASYNC", "T_LAMBDA_OP", "T_LAMBDA_CP",
  "T_UNRESOLVED_OP", "T_WHERE", "'('", "')'", "';'", "'{'", "'}'", "'$'",
  "'`'", "']'", "'\"'", "'\\''", "$accept", "start", "$@1",
  "top_statement_list", "top_statement", "$@2", "$@3",
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
  "static_keyset_literal", "static_keyset_literal_ae", "vec_ks_expr_list",
  "static_vec_ks_expr_list", "static_vec_ks_expr_list_ae",
  "collection_literal", "static_collection_literal", "dim_expr",
  "dim_expr_base", "lexical_var_list", "xhp_tag", "xhp_tag_body",
  "xhp_opt_end_label", "xhp_attributes", "xhp_children",
  "xhp_attribute_name", "xhp_attribute_value", "xhp_child", "xhp_label_ws",
  "xhp_bareword", "simple_function_call", "fully_qualified_class_name",
  "static_class_name_base", "static_class_name_no_calls",
  "static_class_name", "class_name_reference", "exit_expr",
  "backticks_expr", "ctor_arguments", "common_scalar", "static_expr",
  "static_expr_list", "static_class_constant", "scalar",
  "static_array_pair_list", "possible_comma", "hh_possible_comma",
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
  "variable_no_calls", "dimmable_variable_no_calls", "assignment_list",
  "array_pair_list", "non_empty_array_pair_list", "collection_init",
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
  "hh_access_type", "array_typelist", "hh_type", "hh_type_opt", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,    44,
     264,   265,   266,   267,    61,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,    63,    58,   284,   285,   286,   124,    94,    38,   287,
     288,   289,   290,    60,    62,   291,   292,   293,   294,   295,
      43,    45,    46,    42,    47,    37,    33,   296,   126,    64,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
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
     426,   427,   428,   429,    40,    41,    59,   123,   125,    36,
      96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   204,   206,   205,   207,   207,   208,   208,   208,   208,
     208,   208,   208,   208,   209,   208,   210,   208,   208,   208,
     208,   208,   208,   208,   208,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   212,   212,   212,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   212,
     213,   213,   214,   214,   215,   215,   216,   217,   217,   217,
     217,   218,   218,   219,   220,   220,   220,   221,   221,   222,
     222,   222,   223,   224,   225,   225,   226,   226,   227,   227,
     227,   227,   228,   228,   228,   229,   228,   230,   228,   231,
     228,   232,   228,   228,   228,   228,   228,   228,   228,   228,
     228,   228,   228,   228,   228,   228,   228,   228,   233,   228,
     234,   228,   228,   235,   228,   236,   228,   228,   228,   228,
     228,   228,   228,   228,   228,   228,   228,   228,   228,   228,
     228,   238,   237,   239,   239,   241,   240,   242,   242,   243,
     243,   244,   246,   245,   247,   245,   248,   245,   250,   249,
     251,   249,   253,   252,   254,   252,   255,   252,   256,   252,
     258,   257,   260,   259,   261,   259,   262,   262,   263,   264,
     265,   265,   265,   265,   265,   266,   266,   267,   267,   268,
     268,   269,   269,   270,   270,   271,   271,   272,   272,   272,
     273,   273,   274,   274,   275,   275,   276,   276,   277,   277,
     278,   278,   278,   278,   279,   279,   279,   280,   280,   281,
     281,   282,   282,   283,   283,   284,   284,   285,   285,   285,
     285,   285,   285,   285,   285,   286,   286,   286,   286,   286,
     286,   286,   286,   287,   287,   287,   287,   287,   287,   287,
     287,   288,   288,   288,   288,   288,   288,   288,   288,   289,
     289,   290,   290,   290,   290,   290,   290,   291,   291,   292,
     292,   292,   293,   293,   293,   293,   294,   294,   295,   296,
     297,   297,   299,   298,   300,   298,   298,   298,   298,   301,
     298,   302,   298,   298,   298,   298,   298,   298,   298,   298,
     303,   303,   303,   304,   305,   305,   306,   306,   307,   307,
     308,   308,   309,   309,   310,   310,   310,   310,   310,   310,
     310,   311,   311,   312,   313,   313,   314,   314,   315,   315,
     316,   317,   317,   317,   318,   318,   318,   318,   319,   319,
     319,   319,   319,   319,   319,   320,   320,   320,   321,   321,
     322,   322,   323,   323,   324,   324,   325,   325,   326,   326,
     326,   326,   326,   326,   326,   327,   327,   328,   328,   328,
     329,   329,   329,   329,   330,   330,   331,   331,   332,   332,
     333,   334,   334,   334,   334,   334,   334,   335,   336,   336,
     337,   337,   338,   338,   338,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   346,   346,   346,   346,   347,   347,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   347,   348,   348,   350,   349,   351,   349,   353,
     352,   354,   352,   355,   352,   356,   352,   357,   352,   358,
     358,   358,   359,   359,   360,   360,   361,   361,   362,   362,
     363,   363,   364,   365,   365,   366,   366,   367,   367,   367,
     367,   368,   368,   369,   369,   370,   370,   371,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   381,
     382,   382,   383,   383,   384,   385,   386,   386,   387,   387,
     387,   387,   387,   387,   387,   387,   387,   388,   388,   388,
     388,   389,   390,   390,   391,   391,   392,   392,   393,   393,
     394,   395,   395,   396,   396,   396,   397,   397,   397,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     399,   400,   400,   401,   401,   401,   401,   401,   402,   402,
     403,   403,   403,   404,   404,   404,   405,   405,   405,   406,
     406,   406,   407,   407,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   410,   410,   411,   411,   411,   411,   412,   412,
     412,   412,   412,   412,   412,   413,   413,   414,   414,   415,
     415,   416,   416,   416,   416,   417,   417,   417,   417,   417,
     418,   418,   418,   418,   419,   419,   420,   420,   420,   420,
     420,   420,   420,   420,   420,   420,   420,   421,   421,   422,
     422,   423,   423,   423,   423,   424,   424,   425,   425,   426,
     426,   427,   427,   428,   428,   429,   429,   431,   430,   432,
     433,   433,   434,   434,   435,   435,   435,   436,   436,   437,
     437,   438,   438,   439,   439,   440,   440,   441,   441,   442,
     442,   443,   443,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   445,   445,   445,   445,   445,   445,
     445,   445,   445,   446,   446,   446,   446,   446,   446,   446,
     446,   446,   447,   448,   448,   449,   449,   450,   450,   450,
     451,   452,   452,   452,   453,   453,   453,   453,   454,   454,
     455,   455,   455,   455,   455,   455,   456,   456,   456,   456,
     456,   457,   457,   457,   457,   457,   457,   458,   458,   459,
     459,   459,   459,   459,   459,   459,   459,   460,   460,   461,
     461,   461,   461,   462,   462,   463,   463,   463,   463,   464,
     464,   464,   464,   465,   465,   465,   465,   465,   465,   466,
     466,   466,   467,   467,   467,   467,   467,   467,   467,   467,
     467,   467,   467,   468,   468,   469,   469,   470,   470,   471,
     471,   471,   471,   472,   472,   473,   473,   474,   474,   475,
     475,   476,   476,   477,   477,   478,   479,   479,   479,   479,
     480,   480,   481,   481,   482,   482,   483,   483,   484,   484,
     485,   486,   486,   487,   487,   487,   487,   488,   488,   488,
     489,   489,   489,   490,   490,   491,   491,   492,   493,   494,
     494,   495,   495,   496,   496,   496,   496,   496,   496,   496,
     496,   496,   496,   496,   497,   497
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
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
       1,     1,     2,     3,     3,     1,     2,     1,     2,     3,
       4,     3,     1,     2,     1,     2,     2,     1,     3,     1,
       3,     2,     2,     2,     5,     4,     2,     0,     1,     1,
       1,     1,     3,     5,     8,     0,     4,     0,     6,     0,
      10,     0,     4,     2,     3,     2,     3,     2,     3,     3,
       3,     3,     3,     3,     5,     1,     1,     1,     0,     9,
       0,    10,     5,     0,    13,     0,     5,     3,     3,     2,
       2,     2,     2,     2,     2,     3,     2,     2,     3,     2,
       2,     0,     4,     9,     0,     0,     4,     2,     0,     1,
       0,     1,     0,     9,     0,    10,     0,    11,     0,     9,
       0,    10,     0,     8,     0,     9,     0,     7,     0,     8,
       0,     8,     0,     7,     0,     8,     1,     1,     1,     1,
       1,     2,     3,     3,     2,     2,     0,     2,     0,     2,
       0,     1,     3,     1,     3,     2,     0,     1,     2,     4,
       1,     4,     1,     4,     1,     4,     1,     4,     3,     5,
       3,     4,     4,     5,     5,     4,     0,     1,     1,     4,
       0,     5,     0,     2,     0,     3,     0,     7,     8,     6,
       2,     5,     6,     4,     0,     4,     5,     7,     6,     6,
       7,     9,     8,     6,     7,     5,     2,     4,     5,     3,
       0,     3,     4,     6,     5,     5,     6,     8,     7,     2,
       0,     1,     2,     2,     3,     4,     4,     3,     1,     1,
       2,     4,     3,     5,     1,     3,     2,     0,     2,     3,
       2,     0,     0,     4,     0,     5,     2,     2,     2,     0,
      11,     0,    12,     3,     3,     3,     4,     4,     3,     5,
       2,     2,     0,     6,     5,     4,     3,     1,     1,     3,
       4,     1,     2,     1,     1,     5,     6,     1,     1,     4,
       1,     1,     3,     2,     2,     0,     2,     0,     1,     3,
       1,     1,     1,     1,     3,     4,     4,     4,     1,     1,
       2,     2,     2,     3,     3,     1,     1,     1,     1,     3,
       1,     3,     1,     1,     1,     0,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     0,     1,     1,     1,
       3,     5,     1,     3,     5,     4,     3,     3,     3,     4,
       3,     3,     3,     2,     2,     1,     1,     3,     3,     1,
       1,     0,     1,     2,     4,     3,     3,     6,     2,     3,
       2,     3,     6,     1,     1,     1,     1,     1,     6,     3,
       4,     6,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     5,     4,     3,     1,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     1,     1,     1,     1,     1,     1,
       3,     2,     1,     5,     0,     0,    12,     0,    13,     0,
       4,     0,     7,     0,     5,     0,     3,     0,     6,     2,
       2,     4,     1,     1,     5,     3,     5,     3,     2,     0,
       2,     0,     4,     4,     3,     2,     0,     5,     3,     6,
       4,     2,     0,     5,     3,     2,     0,     5,     3,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     2,     0,
       2,     0,     2,     0,     4,     4,     4,     4,     1,     1,
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
       1,     1,     3,     1,     1,     1,     2,     1,     0,     0,
       1,     1,     3,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     2,     1,
       1,     4,     3,     4,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       5,     4,     3,     1,     3,     3,     3,     3,     1,     1,
       1,     1,     3,     3,     3,     2,     0,     1,     0,     1,
       0,     5,     3,     3,     1,     1,     1,     1,     3,     2,
       1,     1,     1,     1,     1,     3,     1,     1,     1,     2,
       2,     4,     3,     4,     1,     1,     1,     3,     1,     2,
       0,     5,     3,     3,     1,     3,     1,     2,     0,     5,
       3,     2,     0,     3,     0,     4,     2,     0,     3,     3,
       1,     0,     1,     1,     1,     1,     3,     1,     1,     1,
       3,     1,     1,     3,     3,     2,     4,     2,     4,     5,
       5,     5,     5,     1,     1,     1,     1,     1,     1,     3,
       3,     4,     4,     3,     1,     1,     1,     1,     3,     1,
       4,     3,     3,     1,     1,     1,     1,     1,     3,     3,
       4,     4,     3,     1,     1,     7,     9,     7,     6,     8,
       1,     4,     4,     1,     1,     1,     4,     2,     1,     0,
       1,     1,     1,     3,     3,     3,     0,     1,     1,     3,
       3,     2,     3,     6,     0,     1,     4,     2,     0,     5,
       3,     3,     1,     6,     4,     4,     2,     2,     0,     5,
       3,     3,     1,     2,     0,     5,     3,     3,     1,     2,
       2,     1,     2,     1,     4,     3,     3,     6,     3,     1,
       1,     1,     4,     4,     4,     4,     4,     4,     2,     2,
       4,     2,     2,     1,     3,     3,     3,     0,     2,     5,
       6,     6,     7,     1,     2,     1,     2,     1,     4,     1,
       4,     3,     0,     1,     3,     2,     3,     1,     1,     0,
       0,     3,     1,     3,     3,     2,     0,     2,     2,     2,
       2,     1,     2,     4,     2,     5,     3,     1,     1,     0,
       3,     4,     5,     3,     1,     2,     0,     4,     1,     3,
       2,     4,     5,     2,     2,     1,     1,     1,     1,     3,
       2,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   432,     0,     0,   847,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   938,
       0,   926,   718,     0,   724,   725,   726,    25,   789,   914,
     915,   156,   157,   727,     0,   137,     0,     0,     0,     0,
      26,     0,     0,     0,     0,   191,     0,     0,     0,     0,
       0,     0,   398,   399,   400,   403,   402,   401,     0,     0,
       0,     0,   220,     0,     0,     0,    33,    34,    35,   731,
     733,   734,   728,   729,     0,     0,     0,   735,   730,     0,
     702,    28,    29,    30,    32,    31,     0,   732,     0,     0,
       0,     0,   736,   404,   537,    27,     0,   155,   127,     0,
     719,     0,     0,     4,   117,   119,   788,     0,   701,     0,
       6,   190,     7,     9,     8,    10,     0,     0,   396,   445,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   443,
     903,   904,   519,   515,   516,   517,   518,   426,   522,     0,
     425,   874,   703,   710,     0,   791,   514,   395,   877,   878,
     889,   444,     0,     0,   447,   446,   875,   876,   873,   910,
     913,   504,   790,    11,   403,   402,   401,     0,     0,    32,
       0,   117,   190,     0,   982,   444,   981,     0,   979,   978,
     521,     0,   433,   440,   438,     0,     0,   486,   487,   488,
     489,   513,   511,   510,   509,   508,   507,   506,   505,    25,
     914,   727,   705,    33,    34,    35,     0,     0,  1002,   896,
     703,     0,   704,   467,     0,   465,     0,   942,     0,   798,
     424,   714,   210,     0,  1002,   423,   713,   708,     0,   723,
     704,   921,   922,   928,   920,   715,     0,     0,   717,   512,
       0,     0,     0,     0,   429,     0,   135,   431,     0,     0,
     141,   143,     0,     0,   145,     0,    76,    75,    70,    69,
      61,    62,    53,    73,    84,    85,     0,    56,     0,    68,
      60,    66,    87,    79,    78,    51,    74,    94,    95,    52,
      90,    49,    91,    50,    92,    48,    96,    83,    88,    93,
      80,    81,    55,    82,    86,    47,    77,    63,    97,    71,
      64,    54,    46,    45,    44,    43,    42,    41,    65,    99,
      98,   101,    58,    39,    40,    67,  1046,  1047,    59,  1051,
      38,    57,    89,     0,     0,   117,   100,   993,  1045,     0,
    1048,     0,     0,   147,     0,     0,     0,   181,     0,     0,
       0,     0,     0,     0,   800,     0,   105,   107,   309,     0,
       0,   308,     0,   224,     0,   221,   314,     0,     0,     0,
       0,     0,   999,   206,   218,   934,   938,   556,   579,   579,
       0,   963,     0,   738,     0,     0,     0,   961,     0,    16,
       0,   121,   198,   212,   219,   607,   549,     0,   987,   529,
     531,   533,   851,   432,   445,     0,     0,   443,   444,   446,
       0,     0,   917,   720,     0,   721,     0,     0,     0,   180,
       0,     0,   123,   300,     0,    24,   189,     0,   217,   202,
     216,   401,   404,   190,   397,   170,   171,   172,   173,   174,
     176,   177,   179,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   926,     0,   169,   919,   919,   948,     0,     0,     0,
       0,     0,     0,     0,     0,   394,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   466,
     464,   852,   853,     0,   919,     0,   865,   300,   300,   919,
       0,   934,     0,   190,     0,     0,   149,     0,   849,   844,
     798,     0,   445,   443,     0,   946,     0,   554,   797,   937,
     723,   445,   443,   444,   123,     0,   300,   422,     0,   867,
     716,     0,   127,   260,     0,   536,     0,   152,     0,     0,
     430,     0,     0,     0,     0,     0,   144,   168,   146,  1046,
    1047,  1043,  1044,     0,  1050,  1036,     0,     0,     0,     0,
      72,    37,    59,    36,   994,   175,   178,   148,   127,     0,
     165,   167,     0,     0,     0,     0,   108,     0,   799,   106,
      18,     0,   102,     0,   310,     0,   150,   223,   222,     0,
       0,   151,   983,     0,     0,   445,   443,   444,   447,   446,
       0,  1029,   230,     0,   935,     0,     0,     0,     0,   798,
     798,     0,     0,   153,     0,     0,   737,   962,   789,     0,
       0,   960,   794,   959,   120,     5,    13,    14,     0,   228,
       0,     0,   542,     0,     0,   798,     0,     0,   711,   706,
     543,     0,     0,     0,     0,   851,   127,     0,   800,   850,
    1055,   421,   435,   500,   883,   902,   132,   126,   128,   129,
     130,   131,   395,     0,   520,   792,   793,   118,   798,     0,
    1003,     0,     0,     0,   800,   301,     0,   525,   192,   226,
       0,   470,   472,   471,   483,     0,     0,   503,   468,   469,
     473,   475,   474,   491,   490,   493,   492,   494,   496,   498,
     497,   495,   485,   484,   477,   478,   476,   479,   480,   482,
     499,   481,   918,     0,     0,   952,     0,   798,   986,     0,
     985,  1002,   880,   208,   200,   214,     0,   987,   204,   190,
       0,   436,   439,   441,   449,   463,   462,   461,   460,   459,
     458,   457,   456,   455,   454,   453,   452,   855,     0,   854,
     857,   879,   861,  1002,   858,     0,     0,     0,     0,     0,
       0,     0,     0,   980,   434,   842,   846,   797,   848,     0,
     707,     0,   941,     0,   940,   226,     0,   707,   925,   924,
     910,   913,     0,     0,   854,   857,   923,   858,   427,   262,
     264,   127,   540,   539,   428,     0,   127,   244,   136,   431,
       0,     0,     0,     0,     0,   256,   256,   142,   798,     0,
       0,     0,  1034,   798,     0,  1009,     0,     0,     0,     0,
       0,   796,     0,    33,    34,    35,   702,     0,     0,   740,
     701,   744,   745,   746,   748,     0,   739,   125,   747,  1002,
    1049,     0,     0,     0,     0,    19,     0,    20,     0,   103,
       0,     0,     0,   114,   800,     0,   112,   107,   104,   109,
       0,   307,   315,   312,     0,     0,   972,   977,   974,   973,
     976,   975,    12,  1027,  1028,     0,   798,     0,     0,     0,
     934,   931,     0,   553,     0,   569,   797,   555,   797,   578,
     572,   575,   971,   970,   969,     0,   965,     0,   966,   968,
       0,     5,     0,     0,     0,   601,   602,   610,   609,     0,
     443,     0,   797,   548,   552,     0,     0,   988,     0,   530,
       0,     0,  1016,   851,   286,  1054,     0,     0,   866,     0,
     916,   797,  1005,  1001,   302,   303,   700,   799,   299,     0,
     851,     0,     0,   228,   527,   194,   502,     0,   586,   587,
       0,   584,   797,   947,     0,     0,   300,   230,     0,   228,
       0,     0,   226,     0,   926,   450,     0,     0,   863,   864,
     881,   882,   911,   912,     0,     0,     0,   830,   805,   806,
     807,   814,     0,    33,    34,    35,     0,     0,   818,   824,
     825,   826,   816,   817,   836,   798,     0,   844,   945,   944,
       0,   228,     0,   868,   722,     0,   266,     0,     0,   133,
       0,     0,     0,     0,     0,     0,     0,   236,   237,   248,
       0,   127,   246,   162,   256,     0,   256,     0,   797,     0,
       0,     0,     0,   797,  1035,  1037,  1008,   798,  1007,     0,
     798,   769,   770,   767,   768,   804,     0,   798,   796,   562,
     581,   581,     0,   551,     0,     0,   954,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1040,   182,     0,   185,   166,
       0,     0,   110,   115,   116,   108,   799,   113,     0,   311,
       0,   984,   154,  1000,  1029,  1020,  1024,   229,   231,   321,
       0,     0,   932,     0,     0,   558,     0,   964,     0,    17,
       0,   987,   227,   321,     0,     0,   707,   545,     0,   712,
     989,     0,  1016,   534,     0,     0,  1055,     0,   291,   289,
     857,   869,  1002,   857,   870,  1004,     0,     0,   304,   124,
       0,   851,   225,     0,   851,     0,   501,   951,   950,     0,
     300,     0,     0,     0,     0,     0,     0,   228,   196,   723,
     856,   300,     0,   810,   811,   812,   813,   819,   820,   834,
       0,   798,     0,   830,   566,   583,   583,     0,   809,   838,
     797,   841,   843,   845,     0,   939,     0,   856,     0,     0,
       0,     0,   263,   541,   138,     0,   431,   236,   238,   934,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   250,
       0,  1041,     0,  1030,     0,  1033,   797,     0,     0,     0,
     742,   797,   795,     0,     0,   798,     0,     0,   783,   798,
       0,   786,   785,     0,   798,     0,   749,   787,   784,   958,
       0,   798,   752,   754,   753,     0,     0,   750,   751,   755,
     757,   756,   772,   771,   774,   773,   775,   777,   779,   778,
     776,   765,   764,   759,   760,   758,   761,   762,   763,   766,
    1039,     0,   127,     0,     0,   111,    21,   313,     0,     0,
       0,  1021,  1026,     0,   395,   936,   934,   437,   442,   448,
     560,     0,     0,    15,     0,   395,   613,     0,     0,   615,
     608,   611,     0,   606,     0,   991,     0,  1017,   538,     0,
     292,     0,     0,   287,     0,   306,   305,  1016,     0,   321,
       0,   851,     0,   300,     0,   908,   321,   987,   321,   990,
       0,     0,     0,   451,     0,     0,   822,   797,   829,   815,
       0,     0,   798,     0,     0,   828,   798,     0,   808,     0,
       0,   798,   835,   943,   321,     0,   127,     0,   259,   245,
       0,     0,     0,   235,   158,   249,     0,     0,   252,     0,
     257,   258,   127,   251,  1042,  1031,     0,  1006,     0,  1053,
     803,   802,   741,   570,   797,   561,     0,   573,   797,   580,
     576,     0,   797,   550,   743,     0,   585,   797,   953,   781,
       0,     0,     0,    22,    23,  1023,  1018,  1019,  1022,   232,
       0,     0,     0,   402,   393,     0,     0,     0,   207,   320,
     322,     0,   392,     0,     0,     0,   987,   395,     0,     0,
     557,   967,   317,   213,   604,     0,     0,   544,   532,     0,
     295,   285,     0,   288,   294,   300,   524,  1016,   395,  1016,
       0,   949,     0,   907,   395,     0,   395,   992,   321,   851,
     905,   833,   832,   821,   571,   797,   565,     0,   574,   797,
     582,   577,     0,   823,   797,   837,   395,   127,   265,   134,
     139,   160,   239,     0,   247,   253,   127,   255,  1032,     0,
       0,     0,   564,   782,   547,     0,   957,   956,   780,   127,
     186,  1025,     0,     0,     0,   995,     0,     0,     0,   233,
       0,   987,     0,   358,   354,   360,   702,    32,     0,   348,
       0,   353,   357,   370,     0,   368,   373,     0,   372,     0,
     371,     0,   190,   324,     0,   326,     0,   327,   328,     0,
       0,   933,   559,     0,   605,   603,   614,   612,   296,     0,
       0,   283,   293,     0,     0,  1016,     0,   203,   524,  1016,
     909,   209,   317,   215,   395,     0,     0,     0,   568,   827,
     840,     0,   211,   261,     0,     0,   127,   242,   159,   254,
    1052,   801,     0,     0,     0,     0,     0,     0,   420,     0,
     996,     0,   338,   342,   417,   418,   352,     0,     0,     0,
     333,   666,   665,   662,   664,   663,   683,   685,   684,   654,
     624,   626,   625,   644,   660,   659,   620,   631,   632,   634,
     633,   653,   637,   635,   636,   638,   639,   640,   641,   642,
     643,   645,   646,   647,   648,   649,   650,   652,   651,   621,
     622,   623,   627,   628,   630,   668,   669,   678,   677,   676,
     675,   674,   673,   661,   680,   670,   671,   672,   655,   656,
     657,   658,   681,   682,   686,   688,   687,   689,   690,   667,
     692,   691,   694,   696,   695,   629,   699,   697,   698,   693,
     679,   619,   365,   616,     0,   334,   386,   387,   385,   378,
       0,   379,   335,   412,     0,     0,     0,     0,   416,     0,
     190,   199,   316,     0,     0,     0,   284,   298,   906,     0,
       0,   388,   127,   193,  1016,     0,     0,   205,  1016,   831,
       0,     0,   127,   240,   140,   161,     0,   563,   546,   955,
     184,   336,   337,   415,   234,     0,   798,   798,     0,   361,
     349,     0,     0,     0,   367,   369,     0,     0,   374,   381,
     382,   380,     0,     0,   323,   997,     0,     0,     0,   419,
       0,   318,     0,   297,     0,   599,   800,   127,     0,     0,
     195,   201,     0,   567,   839,     0,     0,   163,   339,   117,
       0,   340,   341,     0,   797,     0,   797,   363,   359,   364,
     617,   618,     0,   350,   383,   384,   376,   377,   375,   413,
     410,  1029,   329,   325,   414,     0,   319,   600,   799,     0,
       0,   389,   127,   197,     0,   243,     0,   188,     0,   395,
       0,   355,   362,   366,     0,     0,   851,   331,     0,   597,
     523,   526,     0,   241,     0,     0,   164,   346,     0,   394,
     356,   411,   998,     0,   800,   406,   851,   598,   528,     0,
     187,     0,     0,   345,  1016,   851,   270,   407,   408,   409,
    1055,   405,     0,     0,     0,   344,  1010,   406,     0,  1016,
       0,   343,     0,     0,  1055,     0,   275,   273,  1010,   127,
     800,  1012,     0,   390,   127,   330,     0,   276,     0,     0,
     271,     0,     0,   799,  1011,     0,  1015,     0,     0,   279,
     269,     0,   272,   278,   332,   183,  1013,  1014,   391,   280,
       0,     0,   267,   277,     0,   268,   282,   281
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   113,   911,   635,   181,  1525,   731,
     353,   354,   355,   356,   864,   865,   866,   115,   116,   117,
     118,   119,   410,   667,   668,   549,   255,  1594,   555,  1503,
    1595,  1837,   853,   348,   578,  1797,  1099,  1292,  1856,   427,
     182,   669,   951,  1165,  1352,   123,   638,   968,   670,   689,
     972,   612,   967,   235,   530,   671,   639,   969,   429,   373,
     393,   126,   953,   914,   889,  1117,  1528,  1221,  1027,  1744,
    1598,   808,  1033,   554,   817,  1035,  1392,   800,  1016,  1019,
    1210,  1863,  1864,   657,   658,   683,   684,   360,   361,   367,
    1563,  1722,  1723,  1304,  1439,  1551,  1716,  1846,  1866,  1755,
    1801,  1802,  1803,  1538,  1539,  1540,  1541,  1757,  1758,  1764,
    1813,  1544,  1545,  1549,  1709,  1710,  1711,  1733,  1905,  1440,
    1441,   183,   128,  1880,  1881,  1714,  1443,  1444,  1445,  1446,
     129,   248,   550,   551,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,  1575,   140,   950,  1164,   141,   654,
     655,   656,   252,   402,   545,   644,   645,  1254,   646,  1255,
     142,   143,   618,   619,  1244,  1245,  1361,  1362,   144,   841,
     999,   145,   842,  1000,   146,   843,  1001,   621,  1247,  1364,
     147,   844,   148,   149,  1786,   150,   640,  1565,   641,  1134,
     919,  1323,  1320,  1702,  1703,   151,   152,   153,   238,   154,
     239,   249,   414,   537,   155,  1055,  1249,   848,   156,  1056,
     942,   589,  1057,  1002,  1187,  1003,  1189,  1366,  1190,  1191,
    1005,  1370,  1371,  1006,   776,   520,   195,   196,   672,   660,
     503,  1150,  1151,   762,   763,   938,   158,   241,   159,   160,
     185,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     723,   245,   246,   615,   228,   229,   726,   727,  1260,  1261,
     386,   387,   905,   171,   603,   172,   653,   173,   339,  1724,
    1776,   374,   422,   678,   679,  1049,  1893,  1900,  1901,  1145,
    1301,   885,  1302,   886,   887,   822,   823,   824,   340,   341,
     850,   564,  1527,   936
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1622
static const yytype_int16 yypact[] =
{
   -1622,   181, -1622, -1622,  6050, 14291, 14291,    -7, 14291, 14291,
   14291, 11678, 14291, 14291, -1622, 14291, 14291, 14291, 14291, 14291,
   14291, 14291, 14291, 14291, 14291, 14291, 14291, 17215, 17215, 11879,
   14291,  4179,    -3,    -1, -1622, -1622, -1622,   227, -1622,   196,
   -1622, -1622, -1622,   298, 14291, -1622,    -1,   194,   208,   210,
   -1622,    -1, 12080,  3241, 12281, -1622, 15304, 10673,    28, 14291,
    2937,    44, -1622, -1622, -1622,    54,    73,    55,   222,   312,
     341,   359, -1622,  3241,   363,   369,   326,   361,   504, -1622,
   -1622, -1622, -1622, -1622, 14291,    65,  1744, -1622, -1622,  3241,
   -1622, -1622, -1622, -1622,  3241, -1622,  3241, -1622,   444,   393,
    3241,  3241, -1622,   311, -1622, -1622, 12482, -1622, -1622,   349,
     550,   578,   578, -1622,   567,   451,   618,   438, -1622,    90,
   -1622,   596, -1622, -1622, -1622, -1622,  2988,   671, -1622, -1622,
     440,   443,   445,   461,   463,   466,   468,   477,  3654, -1622,
   -1622, -1622, -1622,   144,   573,   606,   616, -1622,   628,   630,
   -1622,   136,   505, -1622,   546,   164, -1622,  1509,   169, -1622,
   -1622,  2287,   110,   535,   157, -1622,   156,   202,   557,   233,
   -1622, -1622,   684, -1622, -1622, -1622,   610,   583,   636, -1622,
   14291, -1622,   596,   671, 18243,  3007, 18243, 14291, 18243, 18243,
   15828,   604, 17382, 15828, 18243,   758,  3241,   740,   740,   321,
     740,   740,   740,   740,   740,   740,   740,   740,   740, -1622,
   -1622, -1622, -1622, -1622, -1622, -1622,    33, 14291,   632, -1622,
   -1622,   660,   626,   588,   635,   588, 17215, 17430,   644,   839,
   -1622,   610, -1622, 14291,   632, -1622,   700, -1622,   708,   676,
   -1622,   179, -1622, -1622, -1622,   588,   110, 12683, -1622, -1622,
   14291,  9467,   867,    91, 18243, 10472, -1622, 14291, 14291,  3241,
   -1622, -1622, 11662,   692, -1622, 12064, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622, -1622, -1622,  2956, -1622,  2956, -1622,
   -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622, -1622, -1622,    96,    94,   636, -1622,
   -1622, -1622, -1622,   686,   486,    97, -1622, -1622,   734,   885,
   -1622,   752, 16015, -1622,   719,   723, 12667, -1622,    74, 14275,
    3753,  3753,  3241,   731,   927,   747, -1622,    49, -1622,  5469,
      98, -1622,   819, -1622,   826, -1622,   937,   104, 17215, 14291,
   14291,   763,   781, -1622, -1622, 16912, 11879, 14291, 14291, 14291,
     106,    82,   600, -1622, 14492, 17215,   570, -1622,  3241, -1622,
     230,   451, -1622, -1622, -1622, -1622, 17927,   958,   871, -1622,
   -1622, -1622,    70, 14291,   784,   785, 18243,   790,  1943,   791,
    6251, 14291, -1622,   447,   787,   609,   447,   539,   525, -1622,
    3241,  2956,   805, 10874, 15304, -1622, -1622,  1961, -1622, -1622,
   -1622, -1622, -1622,   596, -1622, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, 14291, 14291, 14291, 14291, 12884, 14291, 14291,
   14291, 14291, 14291, 14291, 14291, 14291, 14291, 14291, 14291, 14291,
   14291, 14291, 14291, 14291, 14291, 14291, 14291, 14291, 14291, 14291,
   14291, 18025, 14291, -1622, 14291, 14291, 14291, 14661,  3241,  3241,
    3241,  3241,  3241,  2988,   873,   651,  5329, 14291, 14291, 14291,
   14291, 14291, 14291, 14291, 14291, 14291, 14291, 14291, 14291, -1622,
   -1622, -1622, -1622,  2077, 14291, 14291, -1622, 10874, 10874, 14291,
   14291, 16912,   808,   596, 13085, 16617, -1622, 14291, -1622,   810,
     984,   851,   812,   813, 14806,   588, 13286, -1622, 13487, -1622,
     676,   815,   822,  2073, -1622,   467, 10874, -1622,  3206, -1622,
   -1622, 16665, -1622, -1622, 11075, -1622, 14291, -1622,   920,  9668,
    1012,   827, 14476,  1010,    81,    59, -1622, -1622, -1622,   849,
   -1622, -1622, -1622,  2956, -1622,  1266,   842,  1029, 16811,  3241,
   -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622,   847,
   -1622, -1622,   845,   848,   852,   854,   260,  3255,  3846, -1622,
   -1622,  3241,  3241, 14291,   588,    44, -1622, -1622, -1622, 16811,
     962, -1622,   588,   100,   130,   858,   862,  2211,   214,   869,
     866,   729,   914,   872,   588,   133,   870, 17486,   876,  1058,
    1059,   878,   880, -1622,  3459,  3241, -1622, -1622,  1000,  2890,
     221, -1622, -1622, -1622,   451, -1622, -1622, -1622,  1039,   939,
     895,   237,   924, 14291,   947,  1075,   890,   929, -1622,   282,
   -1622,  2956,  2956,  1076,   867,    70, -1622,   898,  1083, -1622,
    2956,   424, -1622,   484,   331, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622,  1710,  3163, -1622, -1622, -1622, -1622,  1084,   916,
   -1622, 17215, 14291,   902,  1089, 18243,  1086, -1622, -1622,   970,
    2600, 12266, 18381, 15828, 15129, 14291, 18195, 15303, 11858, 13264,
    3761,  4120, 13663, 13864, 13864, 13864, 13864,  3565,  3565,  3565,
    3565,  3565,  1074,  1074,   661,   661,   661,   321,   321,   321,
   -1622,   740, 18243,   904,   905, 17534,   911,  1094,   -26, 14291,
       0,   632,   189, -1622, -1622, -1622,  1096,   871, -1622,   596,
   17013, -1622, -1622, -1622, 15828, 15828, 15828, 15828, 15828, 15828,
   15828, 15828, 15828, 15828, 15828, 15828, 15828, -1622, 14291,   515,
   -1622,   310, -1622,   632,   623,   912,  3538,   921,   922,   918,
    3728,   134,   917, -1622, 18243,  3185, -1622,  3241, -1622,   424,
     471, 17215, 18243, 17215, 17590,   970,   424,   588,   319, -1622,
     282,   957,   925, 14291, -1622,   323, -1622, -1622, -1622,  9266,
     760, -1622, -1622, 18243, 18243,    -1, -1622, -1622, -1622, 14291,
    1019,  3926, 16811,  3241,  9869,   934,   940, -1622,  1124,  1046,
    1008,   987, -1622,  1137,   952,  2387,  2956, 16811, 16811, 16811,
   16811, 16811,   954,  1080,  1081,  1087,   995,   960, 16811,   488,
     998, -1622, -1622, -1622, -1622,   963, -1622, 18337, -1622,    39,
   -1622,  6452,  2270,   966,  3846, -1622,  3846, -1622,  3241,  3241,
    3846,  3846,  3241, -1622,  1155,   968, -1622,   267, -1622, -1622,
    3834, -1622, 18337,  1156, 17215,   975, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622, -1622,   992,  1164,  3241,  2270,   978,
   16912, 17114,  1165, -1622, 13688, -1622, 14291, -1622, 14291, -1622,
   -1622, -1622, -1622, -1622, -1622,   977, -1622, 14291, -1622, -1622,
    5648, -1622,  2956,  2270,   983, -1622, -1622, -1622, -1622,  1168,
     988, 14291, 17927, -1622, -1622, 14661,   989, -1622,  2956, -1622,
     996,  6653,  1154,    53, -1622, -1622,    58,  2077, -1622,  3206,
   -1622,  2956, -1622, -1622,   588, 18243, -1622, 11276, -1622, 16811,
      43,   999,  2270,   939, -1622, -1622, 15303, 14291, -1622, -1622,
   14291, -1622, 14291, -1622,  4394,  1001, 10874,   914,  1160,   939,
    2956,  1180,   970,  3241, 18025,   588,  4478,  1003, -1622, -1622,
     350,  1004, -1622, -1622,  1185,  1650,  1650,  3185, -1622, -1622,
   -1622,  1148,  1007,  1133,  1134,  1135,    60,  1013, -1622, -1622,
   -1622, -1622, -1622, -1622, -1622,  1199,  1014,   810,   588,   588,
   13889,   939,  3206, -1622, -1622,  4765,   837,    -1, 10472, -1622,
    6854,  1017,  7055,  1018,  3926, 17215,  1016,  1079,   588, 18337,
    1202, -1622, -1622, -1622, -1622,   772, -1622,    56,  2956,  1037,
    1091,  2956,  3241,  1266, -1622, -1622, -1622,  1210, -1622,  1027,
    1084,   769,   769,  1159,  1159, 16884,  1022,  1216, 16811, 16811,
   16811, 16811, 16160, 17927,  2284, 16305, 16811, 16811, 16811, 16811,
   16689, 16811, 16811, 16811, 16811, 16811, 16811, 16811, 16811, 16811,
   16811, 16811, 16811, 16811, 16811, 16811, 16811, 16811, 16811, 16811,
   16811, 16811, 16811, 16811,  3241, -1622, -1622,  1146, -1622, -1622,
    1033,  1034, -1622, -1622, -1622,   436,  3255, -1622,  1038, -1622,
   16811,   588, -1622, -1622,    71, -1622,   821,  1227, -1622, -1622,
     147,  1043,   588, 11477, 17215, 18243, 17638, -1622,  2841, -1622,
    5849,   871,  1227, -1622,     8,    12, -1622, 18243,  1102,  1047,
   -1622,  1049,  1154, -1622,  2956,   867,  2956,    75,  1228,  1163,
     325, -1622,   632,   327, -1622, -1622, 17215, 14291, 18243, 18337,
    1048,    43, -1622,  1050,    43,  1054, 15303, 18243, 17694,  1055,
   10874,  1057,  1053,  2956,  1056,  1060,  2956,   939, -1622,   676,
     637, 10874, 14291, -1622, -1622, -1622, -1622, -1622, -1622,  1119,
    1061,  1248,  1167,  3185,  3185,  3185,  3185,  1104, -1622, 17927,
    3185, -1622, -1622, -1622, 17215, 18243,  1063, -1622,    -1,  1229,
    1186, 10472, -1622, -1622, -1622,  1068, 14291,  1079,   588, 16912,
    3926,  1070, 16811,  7256,   789,  1073, 14291,    61,   495, -1622,
    1090, -1622,  2956, -1622,  1136, -1622,  2646,  1241,  1082, 16811,
   -1622, 16811, -1622,  1085,  1077,  1270, 17742,  1093, 18337,  1272,
    1095, -1622, -1622,  1139,  1273,  1088, -1622, -1622, -1622, 17797,
    1092,  1276, 16012, 18425, 10854, 16811, 18291, 13064,  4575, 13464,
   12860,  4906, 14065, 14065, 14065, 14065,  4652,  4652,  4652,  4652,
    4652,   832,   832,   769,   769,   769,  1159,  1159,  1159,  1159,
   -1622,  1100, -1622,  1108,  1111, -1622, -1622, 18337,  3241,  2956,
    2956, -1622,   821,  2270,   112, -1622, 16912, -1622, -1622, 15828,
     588, 14090,  1110, -1622,  1101,   143, -1622,   153, 14291, -1622,
   -1622, -1622, 14291, -1622, 14291, -1622,   867, -1622, -1622,    86,
    1274,  1203, 14291, -1622,  1097,   588, 18243,  1154,  1105, -1622,
    1114,    43, 14291, 10874,  1116, -1622, -1622,   871, -1622, -1622,
    1118,  1121,  1122, -1622,  1120,  3185, -1622,  3185, -1622, -1622,
    1125,  1127,  1278,  1189,  1129, -1622,  1312,  1130, -1622,  1190,
    1141,  1313, -1622,   588, -1622,  1300, -1622,  1143, -1622, -1622,
    1145,  1147,   151, -1622, -1622, 18337,  1149,  1152, -1622,  5243,
   -1622, -1622, -1622, -1622, -1622, -1622,  2956, -1622,  2956, -1622,
   18337, 17845, -1622, -1622, 16811, -1622, 16811, -1622, 16811, -1622,
   -1622, 16811, 17927, -1622, -1622, 16811, -1622, 16811, -1622, 11256,
   16811,  1144,  7457, -1622, -1622,   821, -1622, -1622, -1622, -1622,
     793, 15478,  2270,  1232, -1622,   850,  1182,  2582, -1622, -1622,
   -1622,   873,  2399,   109,   117,  1157,   871,   651,   155, 17215,
   18243, -1622, -1622, -1622,  1191,  5053,  5147, 18243, -1622,   342,
    1341,  1275, 14291, -1622, 18243, 10874,  1242,  1154,   732,  1154,
    1169, 18243,  1170, -1622,   910,  1171,  1346, -1622, -1622,    43,
   -1622, -1622,  1224, -1622, -1622,  3185, -1622,  3185, -1622,  3185,
   -1622, -1622,  3185, -1622, 17927, -1622,  1481, -1622,  9266, -1622,
   -1622, -1622, -1622, 10070, -1622, -1622, -1622,  9266, -1622,  1172,
   16811, 17900, 18337, 18337, 18337,  1226, 18337, 17948, 11256, -1622,
   -1622,   821,  2270,  2270,  3241, -1622,  1357, 16450,    85, -1622,
   15478,   871,  3142, -1622,  1193, -1622,   119,  1176,   121, -1622,
   15827, -1622, -1622, -1622,   122, -1622, -1622,  1358, -1622,  1181,
   -1622,  1293,   596, -1622, 15653, -1622, 15653, -1622, -1622,  1364,
     873, -1622,   588, 14956, -1622, -1622, -1622, -1622,  1365,  1297,
   14291, -1622, 18243,  1187,  1192,  1154,   751, -1622,  1242,  1154,
   -1622, -1622, -1622, -1622,  1615,  1188,  3185,  1249, -1622, -1622,
   -1622,  1252, -1622,  9266, 10271, 10070, -1622, -1622, -1622,  9266,
   -1622, 18337, 16811, 16811, 16811,  7658,  1194,  1195, -1622, 16811,
   -1622,  2270, -1622, -1622, -1622, -1622, -1622,  2956,  3378,   850,
   -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622,   579, -1622,  1182, -1622, -1622, -1622, -1622, -1622,
     131,   690, -1622,  1370,   123, 16015,  1293,  1375, -1622,  2956,
     596, -1622, -1622,  1196,  1379, 14291, -1622, 18243, -1622,   116,
    1197, -1622, -1622, -1622,  1154,   751, 15130, -1622,  1154, -1622,
    3185,  3185, -1622, -1622, -1622, -1622,  7859, 18337, 18337, 18337,
   -1622, -1622, -1622, 18337, -1622,  2521,  1387,  1388,  1201, -1622,
   -1622, 16811, 15827, 15827,  1342, -1622,  1358,  1358,   736, -1622,
   -1622, -1622, 16811,  1317, -1622,  1223,  1214,   124, 16811, -1622,
    3241, -1622, 16811, 18243,  1321, -1622,  1400, -1622,  8060,  1213,
   -1622, -1622,   751, -1622, -1622,  8261,  1217,  1299, -1622,  1310,
    1257, -1622, -1622,  1314,  2956,  1237,  3378, -1622, -1622, 18337,
   -1622, -1622,  1251, -1622,  1382, -1622, -1622, -1622, -1622, 18337,
    1408,   729, -1622, -1622, 18337,  1230, 18337, -1622,   533,  1234,
    8462, -1622, -1622, -1622,  1244, -1622,  1236,  1247,  3241,   651,
    1253, -1622, -1622, -1622, 16811,  1258,    46, -1622,  1354, -1622,
   -1622, -1622,  8663, -1622,  2270,   966, -1622,  1271,  3241,   625,
   -1622, 18337, -1622,  1254,  1438,   722,    46, -1622, -1622,  1367,
   -1622,  2270,  1255, -1622,  1154,    76, -1622, -1622, -1622, -1622,
    2956, -1622,  1260,  1269,   125, -1622,  1259,   722,   127,  1154,
    1256, -1622,  2956,   767,  2956,   354,  1434,  1389,  1259, -1622,
    1464, -1622,   371, -1622, -1622, -1622,   163,  1465,  1395, 14291,
   -1622,   767,  8864,  2956, -1622,  2956, -1622,  9065,   472,  1470,
    1403, 14291, -1622, 18243, -1622, -1622, -1622, -1622, -1622,  1473,
    1406, 14291, -1622, 18243, 14291, -1622, 18243, 18243
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1622, -1622, -1622,  -571, -1622, -1622, -1622,   281,    42,   -54,
     582, -1622,  -270,  -516, -1622, -1622,   384,    84,  1560, -1622,
    2421, -1622,  -458, -1622,    19, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622, -1622, -1622,  -364, -1622, -1622,  -154,
     204,    25, -1622, -1622, -1622, -1622, -1622, -1622,    29, -1622,
   -1622, -1622, -1622, -1622, -1622,    31, -1622, -1622,  1011,  1015,
    1020,   -81,  -649,  -865,   528,   589,  -375,   284,  -937, -1622,
     -92, -1622, -1622, -1622, -1622,  -730,   118, -1622, -1622, -1622,
   -1622,  -361, -1622,  -589, -1622,  -439, -1622, -1622,   913, -1622,
     -76, -1622, -1622, -1050, -1622, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622,  -110, -1622,   -22, -1622, -1622, -1622,
   -1622, -1622,  -189, -1622,    79,  -801, -1622, -1621,  -394, -1622,
    -135,    66,  -118,  -369, -1622,  -197, -1622, -1622, -1622,    99,
     -31,    -6,    45,  -736,   -69, -1622, -1622,    10, -1622,   -11,
   -1622, -1622,    -5,   -43,   -47, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622,  -593,  -863, -1622, -1622, -1622, -1622,
   -1622,  1392, -1622, -1622, -1622, -1622, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622, -1622, -1622, -1622,  1158,   480,   337,
   -1622, -1622, -1622, -1622, -1622,   404, -1622, -1622, -1622, -1622,
   -1622, -1622, -1622, -1622,  -793, -1622,  3161,    24, -1622,  1161,
    -409, -1622, -1622,  -482,  3820,  3911, -1622, -1622, -1622,   485,
     -10,  -624, -1622, -1622,   559,   355,  -618, -1622,   356, -1622,
   -1622, -1622, -1622, -1622,   541, -1622, -1622, -1622,   141,  -892,
     -67,  -426,  -421, -1622,   613,  -111, -1622, -1622,    26,    27,
     552, -1622, -1622,   799,   -21, -1622,  -360,     7,  -352,    57,
     108, -1622, -1622,  -469,  1177, -1622, -1622, -1622, -1622, -1622,
     797,   446, -1622, -1622, -1622,  -353,  -663, -1622,  1131,  -867,
   -1622,   -70,  -202,   152,   733, -1622,  -344, -1622,  -357, -1064,
   -1252,  -262,   135, -1622,   449,   518, -1622, -1622, -1622, -1622,
     473, -1622,  1832, -1093
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1039
static const yytype_int16 yytable[] =
{
     184,   186,   337,   188,   189,   190,   192,   193,   194,   434,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   484,   120,   227,   230,   394,   251,   512,   122,
     397,   398,   534,   124,   934,   125,   648,   405,   244,   254,
     256,  1146,   771,   650,   649,   260,   345,   262,   785,   265,
    1428,   506,   346,  1329,   349,   237,   430,   242,   243,  1138,
     948,   929,   720,   407,   910,   434,   930,   344,   767,   768,
     127,   863,   868,  1023,   971,   404,   483,   760,  1326,   254,
     583,   585,   761,  1315,   799,   409,  1037,  1217,  1163,   253,
     813,   815,    14,  1390,  1611,    14,  1147,   792,   336,   424,
     546,   406,    14,   -37,  1174,   -72,   -36,   595,   -37,   874,
     -72,   -36,   795,   600,  1790,   546,  1430,   796,  1554,    14,
     851,   883,   884,   521,  1459,    14,  1556,   358,  -351,   380,
    1619,  1704,  1773,  1773,  1611,   539,  1011,   407,   366,   546,
    1766,  1148,   891,   891,   357,   157,  1206,  1430,   381,   404,
    1197,   591,   624,  -101,  1784,   382,   891,  1004,  1330,   409,
     891,    14,  1226,  1227,   891,  1895,   412,  1767,  -101,  1460,
     390,  1833,  1316,   391,   523,   406,  1321,   362,   538,  -100,
     504,     3,   515,   790,   363,  1317,   522,   187,   383,   579,
     532,   247,    14,   250,  -100,   364,   409,  1094,   931,  1785,
    1253,  1918,   531,   365,  -535,  1318,  -884,  1454,   121,  1322,
    1896,   592,   406,  -705,  -588,  1198,   501,   502,   421,   529,
     383,   384,   385,   485,  1431,   347,  -885,  -593,   406,  1432,
    1149,    62,    63,    64,   174,  1433,   431,  1434,  -290,  -887,
    1107,  -274,   541,   359,  -799,   541,  1919,  1331,  -799,  -927,
     580,  -797,   254,   552,  1229,  1431,   816,  1391,  1461,  -892,
    1432,  -290,    62,    63,    64,   174,  1433,   431,  1434,  1428,
     543,  -799,  -886,  1466,   548,   563,   814,  1435,  1436,   690,
    1437,  1612,  1613,  1383,  -593,   114,   425,   547,   574,  1468,
     -37,   916,   -72,   -36,   596,   875,  1474,  -596,  1476,  1897,
     601,   432,   623,   509,  1224,  1555,  1228,   505,  1435,  1436,
    1438,  1437,  1351,  1557,   625,  -351,   395,  1620,  1705,  1774,
    1823,  1891,  -704,  1177,  1496,   876,  1768,   606,   892,   984,
    -895,   433,   432,  -884,   263,  1920,  1369,   335,  -896,   605,
    1130,  1453,  1305,  1020,  1103,  1104,  1502,  -712,  1022,   609,
    1561,   508,   509,  -885,   372,   501,   502,   688,  -897,   772,
    -711,  1160,   858,  -894,   254,   406,  -887,   434,  -594,   591,
     337,   227,   617,   254,   254,   392,  -927,   372,   471,   629,
    -888,   372,   372,   966,   336,  1915,  -892,   513,   257,  -930,
     472,  -706,   420,  -929,   399,  -871,   377,  -872,   192,  -886,
     917,  -891,   258,  1576,   259,  1578,   673,   372,   508,   880,
     394,   736,   737,   430,   604,   918,   368,   741,   685,   909,
    -890,  1120,   859,   620,   620,  1568,   636,   637,  1584,   420,
     510,   378,   210,    40,   357,   357,   586,  1907,   691,   692,
     693,   694,   696,   697,   698,   699,   700,   701,   702,   703,
     704,   705,   706,   707,   708,   709,   710,   711,   712,   713,
     714,   715,   716,   717,   718,   719,   336,   721,  1314,   722,
     722,   725,   634,  1299,  1300,   743,   127,   519,   244,   510,
    1380,   744,   745,   746,   747,   748,   749,   750,   751,   752,
     753,   754,   755,   756,   504,   237,   742,   242,   243,   722,
     766,   400,   685,   685,   722,   770,   369,  -888,   401,   744,
     778,  1730,   774,  1153,  1569,  1735,  -930,   276,  1154,   730,
    -929,   782,  -871,   784,  -872,  -898,  1908,  1171,  -891,   965,
     381,   685,   114,   802,   732,   370,   114,   484,   858,   803,
     553,   804,   789,   659,  -901,   278,   411,  -890,   109,  1515,
     210,    40,  1328,   371,  -595,  1929,   161,   375,   501,   502,
     764,   977,   648,   376,  1526,  1179,   336,   209,   807,   650,
     649,  1848,  1338,  1223,   379,  1340,  1363,  1365,  1365,   223,
     225,   732,  1372,   724,  1100,   973,  1101,   396,   870,    50,
     863,   483,   791,  1761,   937,   797,   939,   566,   420,   419,
     920,  1226,  1227,   384,   385,   501,   502,   395,   381,   897,
     899,  1762,   765,   420,   121,   631,  1849,   769,   501,   502,
     955,   505,   381,   573,   559,   213,   214,   215,   560,   631,
    1763,  1591,   423,   381,   426,   923,   435,   534,   406,   436,
     413,   437,  -707,  -589,  1930,   178, -1002,  1095,    89,   329,
    1916,    91,    92,   381,    93,   179,    95,   438,   408,   439,
     631,   381,   440,  1614,   441,  -899,   109,   421,   416,   333,
    1789,   867,   867,   442,  1792,  1013,  -590,   945,  -899,   105,
     334,   384,   385,   381,  1475, -1002,  -591,  1717,   739,  1718,
     956,   114,   381,  1393,  -859,   384,   385,  1353,   474,   631,
     475,   677,   476,   648,   477,   335,   384,   385,   372,  -859,
     650,   649,   937,   939,   468,   469,   470,   963,   471,  1012,
     939,  1769,   501,   502,   964,   632,   384,   385,   676,   507,
     472,  1344,   408,  1458,   384,   385,  1430,  1481,   485,  1482,
    1770,   675,  1354,  1771,    62,    63,    64,   174,   175,   431,
    1382,  -893,  1470,   976,  -592,   626,   384,   385,   573,   372,
     734,   372,   372,   372,   372,   384,   385,  1816,  -705,   408,
      62,    63,    64,   174,   175,   431, -1002,   511,   525,   883,
     884,    14,    55,  1559,   759,   533,  1817,  1888,  1015,  1818,
      62,    63,    64,   174,   175,   431,   659,   421,   388,  1021,
     516,  1906,  -862,   161,   254,   573,   518,   161,  1039,   472,
    1886,   421, -1002,  1044,   432, -1002,  -860,  -862,   524,   794,
    -897,  1873,  1090,  1091,  1092,  1898,   224,   224,   627,   508,
     114,  -860,   633,  1032,  1422,  1017,  1018,  1448,  1093,   648,
     432,  1877,  1878,  1879,  1431,   527,   650,   649,   528,  1432,
     849,    62,    63,    64,   174,  1433,   431,  1434,  -703,   627,
     432,   633,   627,   633,   633,   127,   535,  1587,  1615,  1588,
     536,  1589,   869,   677,  1590,   544,  1115,  1225,  1226,  1227,
     565,  1532,  1087,  1088,  1089,  1090,  1091,  1092,   557,  1125,
    1585,  1126, -1038,   804,  1387,  1226,  1227,  1435,  1436,   568,
    1437,  1093,  1128,  1178,  1472,   904,   906,   415,   417,   418,
     569,   594,  1208,  1209,  1430,   575,  1137,   127,  1498,   576,
     602,   432,   607,  1299,  1300,  1522,  1523,   614,   587,   120,
    1577,   209,   582,   584,  1507,   122,   588,   630,   867,   124,
     867,   125,  1158,   590,   867,   867,  1105,  1731,  1732,   597,
    1334,   599,  1166,    50,  1865,  1167,   598,  1168,   610,    14,
     611,   685,   161,  1903,  1904,  1814,  1815,   730,  1739,  1810,
    1811,   372,   651,   652,  1865,  1533,   127,  1047,  1050,   661,
     662,   244,  1139,  1887,    55,   663,   665,   674,  1534,   213,
     214,   215,  1535,   777,   764,  1201,   797,   127,   237,  -122,
     242,   243,   687,   121,   775,  1205,   626,   779,   780,   178,
     786,  1211,    89,  1536,   805,    91,    92,   787,    93,  1537,
      95,   546,  1431,   809,   812,   224,  1573,  1432,   563,    62,
      63,    64,   174,  1433,   431,  1434,   825,  1212,   826,  1593,
    1238,   852,   854,   105,   855,   873,   888,  1242,  1599,   856,
     857,   157,   648,   877,  1307,   121,   998,   878,  1007,   650,
     649,  1605,   882,   614,   881,   893,   890,   896,   898,   797,
     907,   912,   913,   915,   659,  1435,  1436,   895,  1437,   900,
     114,   901,  -727,   921,   922,   924,   127,   925,   127,   932,
     928,   659,   933,   941,  1030,   114,   943,   946,   947,   432,
     949,   161,   952,   962,  1252,   958,   959,  1258,  1581,   961,
     970,   954,  1308,   978,   121,  -709,   980,   981,  1309,   982,
    1014,  1024,  1793,  1794,   465,   466,   467,   468,   469,   470,
    1034,   471,   114,  1038,   648,   121,  1036,  1040,  1746,  1102,
     677,   650,   649,   472,  1041,  1042,  1043,  1045,  1058,   120,
    1059,  1060,  1336,  1062,  1063,   122,  1065,  1061,   224,   124,
    1066,   125,  1829,  1098,  1106,   685,  1108,   224,  1116,   608,
    1110,  1112,  1113,  1114,   224,  1119,   685,  1309,  1127,  1123,
    1133,  1358,  1135,  1136,   224,  1140,  1144,  1142,   221,   221,
     867,   114,  1173,  1161,  1176,  1170,   127,  1181,  -900,  1182,
    1192,  1193,  1375,  1194,  1195,  1196,   573,  1199,  1200,  1202,
    1219,   254,   114,  1214,  1216,  1220,  1222,  1231,   759,  1236,
     794,  1389,  1237,  1240,   121,  1241,   121,  1232,  1093,  1291,
    1378,  1293,  1294,   944,  1296,  1405,  1303,  1306,  1324,  1409,
    1876,   966,  1332,  1337,  1413,  1325,  1333,  1339,  1341,  1343,
    1346,  1418,  1345,  1348,   372,  1355,  1349,  1357,   991,  1368,
    1374,  1376,  1356,  1377,  1379,  1384,  1186,  1186,   998,  1388,
    1394,   157,  1396,  1398,  1788,  1411,  1914,  1399,  1403,  1404,
    1402,  1408,  1412,  1414,  1795,  1417,  1463,  1485,  1462,   127,
    1416,  1465,   975,   794,  1407,  1421,  1410,   819,  1452,   114,
    1467,   114,   659,   114,  1423,   659,  1450,  1424,  1451,  1469,
     224,  1473,  1560,  1455,  1477,  1480,  1479,  1456,  1478,  1457,
    1483,  1489,  1494,  1234,   434,  1487,  1492,  1464,  1484,  1830,
    1488,  1491,  1497,  1008,   121,  1009,  1493,  1471,   685,  1499,
    1500,  1519,  1501,   573,  1530,  1504,   573,   209,  1505,  1543,
    1430,   161,  1486,  1558,  1564,  1570,  1490,   820,  1571,  1574,
    1586,  1495,  1603,  1028,  1579,  1580,   161,  1600,  1582,    50,
    1442,  1609,  1617,  1618,  1852,   849,  1713,  1712,  1719,  1725,
    1726,  1442,  1728,  1738,  1772,  1740,  1729,   221,  1741,  1778,
    1751,  1752,  1781,  1782,  1787,    14,  1804,  1806,  1715,  1808,
    1820,  1812,  1821,   161,  1827,   213,   214,   215,  1822,  1828,
    1832,   114,  -347,  1835,  1836,  1838,  1839,  1841,  1767,   219,
     219,  1843,  1844,  1855,  1847,   178,  1111,   121,    89,  1850,
    1854,    91,    92,  1860,    93,   179,    95,  1867,  1862,   209,
    1853,  1912,   614,  1122,  1871,  1447,  1917,  1875,  1909,  1874,
    1883,  1885,  1892,  1899,  1608,  1889,  1447,  1572,  1431,   105,
     685,    50,   161,  1432,  1890,    62,    63,    64,   174,  1433,
     431,  1434,  1910,  1913,   998,   998,   998,   998,  1922,  1921,
     224,   998,   659,   161,  1931,  1430,  1932,  1934,   127,  1935,
    1295,  1870,   114,   733,   738,  1172,  1884,   213,   214,   215,
     735,  1381,  1132,  1745,   114,  1882,  1736,  1506,   871,  1760,
    1616,  1435,  1436,   485,  1437,  1765,  1550,  1924,  1894,  1777,
     221,  1706,  1597,    91,    92,  1707,    93,   179,    95,   221,
      14,  1734,  1531,  1367,  1442,   432,   221,   622,  1319,   224,
    1442,  1250,  1442,  1243,  1583,  1188,   221,  1359,  1203,  1360,
    1152,   105,  1547,   616,  1911,   686,  1926,   647,  1048,  1845,
    1521,  1235,  1442,  1298,   127,  1727,  1780,  1290,     0,  1610,
     161,     0,   161,   127,   161,     0,  1028,  1218,     0,  1425,
     224,     0,   224,     0,     0,     0,     0,   218,   218,     0,
       0,   234,     0,  1431,     0,     0,     0,     0,  1432,     0,
      62,    63,    64,   174,  1433,   431,  1434,     0,     0,  1447,
     224,     0,     0,  1743,  1597,  1447,   234,  1447,   219,  1430,
     659,     0,     0,     0,     0,     0,   121,     0,    62,    63,
      64,    65,    66,   431,     0,     0,   998,  1447,   998,    72,
     478,     0,     0,     0,     0,  1552,  1435,  1436,     0,  1437,
    1442,     0,     0,     0,     0,     0,     0,     0,     0,   127,
       0,  1775,     0,     0,    14,   127,     0,     0,     0,     0,
     432,   127,   221,   224,     0,     0,  1310,     0,   479,  1592,
     480,     0,   161,     0,     0,     0,     0,     0,     0,   224,
     224,     0,     0,   481,     0,   482,     0,     0,   432,     0,
       0,     0,   121,   114,  1858,     0,     0,     0,  1335,     0,
    1825,   121,   335,     0,     0,     0,     0,     0,  1548,     0,
    1783,     0,     0,     0,     0,  1447,     0,  1431,  1183,  1184,
    1185,   209,  1432,     0,    62,    63,    64,   174,  1433,   431,
    1434,   434,     0,     0,     0,     0,  1805,  1807,     0,     0,
       0,   219,     0,    50,     0,     0,  1373,   336,     0,     0,
     219,     0,     0,   161,  1720,     0,   998,   219,   998,     0,
     998,   614,  1028,   998,     0,   161,     0,   219,     0,   114,
    1435,  1436,     0,  1437,   114,     0,   218,     0,   114,   213,
     214,   215,     0,     0,     0,     0,     0,   121,     0,     0,
       0,     0,     0,   121,   432,   372,     0,     0,   573,   121,
       0,   335,   127,  1737,     0,    91,    92,     0,    93,   179,
      95,  1701,     0,   224,   224,   209,     0,     0,  1708,    62,
      63,    64,    65,    66,   431,   335,   234,   335,   234,     0,
      72,   478,   221,   105,   335,     0,     0,    50,     0,     0,
       0,     0,     0,     0,   127,     0,     0,     0,   614,     0,
       0,   127,     0,     0,     0,     0,     0,   998,     0,     0,
       0,     0,     0,     0,   114,   114,   114,     0,     0,     0,
     114,   480,     0,   213,   214,   215,   114,     0,   342,     0,
       0,     0,     0,     0,   234,     0,   127,     0,     0,   432,
       0,   221,     0,   219,  1923,  1859,   388,     0,     0,    91,
      92,     0,    93,   179,    95,     0,  1933,     0,   127,   218,
       0,     0,     0,   224,     0,     0,  1936,     0,   218,  1937,
       0,     0,     0,     0,     0,   218,     0,   105,     0,     0,
       0,   389,   221,     0,   221,   218,     0,     0,     0,     0,
     121,     0,     0,     0,     0,   224,   234,   514,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
       0,     0,   221,     0,   161,     0,     0,     0,   127,     0,
       0,   234,     0,   127,   234,     0,     0,   659,     0,     0,
       0,     0,   121,     0,     0,     0,   573,     0,     0,   121,
       0,  1562,     0,   224,     0,     0,     0,   659,     0,     0,
     499,   500,     0,     0,     0,     0,   659,   335,   224,   224,
       0,   998,   998,     0,     0,     0,     0,   114,     0,     0,
       0,   234,     0,     0,   121,   221,  1799,     0,     0,     0,
       0,     0,   209,  1701,  1701,     0,     0,  1708,  1708,     0,
     161,   221,   221,     0,     0,   161,   121,     0,     0,   161,
       0,   372,     0,     0,    50,     0,     0,     0,     0,   114,
       0,   218,     0,   219,     0,     0,   114,   501,   502,     0,
       0,     0,     0,   647,     0,     0,     0,   514,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     213,   214,   215,     0,     0,   224,     0,     0,   561,     0,
     562,   114,     0,     0,     0,     0,   121,     0,     0,  1857,
       0,   121,     0,   234,     0,   234,    91,    92,   839,    93,
     179,    95,   219,   114,     0,     0,     0,     0,   664,  1872,
     499,   500,     0,     0,     0,   161,   161,   161,     0,     0,
       0,   161,     0,     0,   105,   687,     0,   161,   209,   839,
     210,    40,     0,     0,     0,     0,   567,     0,     0,     0,
       0,     0,     0,   219,     0,   219,     0,     0,     0,     0,
      50,     0,     0,     0,     0,   221,   221,     0,     0,     0,
       0,     0,     0,   114,     0,     0,     0,     0,   114,     0,
       0,     0,     0,   219,     0,     0,     0,   501,   502,     0,
       0,   234,   234,     0,     0,     0,   213,   214,   215,     0,
     234,     0,     0,     0,   647,   514,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,     0,     0,
     757,   218,    91,    92,     0,    93,   179,    95,   224,     0,
       0,     0,     0,   680,     0,     0,   342,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   219,     0,   788,     0,
     105,     0,     0,     0,   758,     0,   109,     0,   499,   500,
       0,     0,   219,   219,     0,   221,     0,     0,     0,     0,
       0,     0,     0,     0,  1067,  1068,  1069,     0,   161,     0,
     218,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,     0,  1070,     0,   221,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
     161,   218,     0,   218,     0,   501,   502,   161,     0,     0,
       0,   209,     0,  1093,   499,   500,     0,     0,     0,     0,
     647,     0,     0,     0,     0,   221,     0,     0,     0,     0,
       0,   218,   839,    50,     0,     0,     0,     0,     0,     0,
     221,   221,   161,     0,     0,   234,   234,   839,   839,   839,
     839,   839,     0,     0,     0,   818,     0,     0,   839,     0,
       0,     0,     0,     0,   161,     0,   879,     0,     0,   213,
     214,   215,   234,     0,     0,     0,   219,   219,   276,     0,
       0,   501,   502,     0,     0,     0,     0,     0,     0,   178,
     276,     0,    89,    90,   218,    91,    92,     0,    93,   179,
      95,     0,     0,     0,     0,     0,   278,     0,   234,     0,
     218,   218,     0,     0,     0,     0,     0,     0,   278,     0,
       0,     0,     0,   105,   161,     0,     0,   221,   209,   161,
       0,     0,   234,   234,     0,     0,     0,   338,     0,  1256,
     209,     0,   234,   926,   927,     0,     0,     0,   234,     0,
      50,     0,   935,     0,     0,     0,     0,     0,     0,     0,
       0,   234,    50,     0,     0,     0,     0,     0,     0,   839,
    -394,     0,   234,     0,     0,     0,   219,     0,    62,    63,
      64,   174,   175,   431,     0,   559,   213,   214,   215,   560,
     234,     0,     0,     0,   234,     0,     0,   559,   213,   214,
     215,   560,     0,     0,     0,     0,   178,     0,   219,    89,
     329,     0,    91,    92,     0,    93,   179,    95,   178,  1046,
       0,    89,   329,     0,    91,    92,     0,    93,   179,    95,
     333,     0,     0,   647,     0,     0,     0,     0,     0,     0,
     105,   334,   333,     0,   218,   218,     0,     0,   432,     0,
       0,     0,   105,   334,     0,     0,   219,     0,   234,     0,
       0,   234,   209,   234,     0,     0,     0,     0,     0,     0,
     221,   219,   219,     0,     0,     0,     0,     0,   839,   839,
     839,   839,     0,   234,    50,     0,   839,   839,   839,   839,
     839,   839,   839,   839,   839,   839,   839,   839,   839,   839,
     839,   839,   839,   839,   839,   839,   839,   839,   839,   839,
     839,   839,   839,   839,     0,   647,     0,   680,   680,     0,
     213,   214,   215,   209,     0,     0,     0,     0,     0,     0,
     839,     0,     0,     0,     0,     0,     0,   276,     0,     0,
     178,   209,     0,    89,   218,    50,    91,    92,     0,    93,
     179,    95,     0,     0,     0,     0,     0,   338,   219,   338,
       0,     0,     0,    50,   234,   278,   234,     0,     0,     0,
    1546,     0,     0,     0,   105,     0,   218,     0,     0,  1798,
       0,   213,   214,   215,     0,     0,     0,   209,     0,     0,
       0,     0,     0,   234,     0,     0,   234,     0,     0,   213,
     214,   215,     0,     0,  1131,     0,     0,    91,    92,    50,
      93,   179,    95,     0,     0,   338,     0,     0,     0,   234,
    1141,     0,     0,     0,   218,    91,    92,     0,    93,   179,
      95,     0,     0,  1155,     0,   105,  1547,     0,     0,   218,
     218,     0,   839,     0,   559,   213,   214,   215,   560,     0,
       0,     0,   234,   105,   954,     0,   234,     0,     0,   839,
       0,   839,  1175,     0,     0,   178,     0,     0,    89,   329,
       0,    91,    92,     0,    93,   179,    95,     0,  1397,     0,
       0,     0,     0,     0,     0,   839,     0,     0,     0,   333,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   105,
     334,   219,   338,     0,     0,   338,     0,     0,     0,     0,
       0,   443,   444,   445,     0,     0,     0,     0,     0,   234,
     234,     0,     0,   234,     0,     0,   218,     0,     0,     0,
    1230,   446,   447,  1233,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,     0,   471,     0,
     443,   444,   445,     0,     0,     0,     0,     0,     0,     0,
     472,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     446,   447,     0,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,     0,   471,     0,     0,
       0,     0,     0,     0,     0,     0,   234,     0,   234,   472,
       0,     0,     0,     0,   839,     0,   839,     0,   839,     0,
       0,   839,   234,     0,     0,   839,  1327,   839,   935,     0,
     839,     0,     0,     0,   338,     0,   821,   276,     0,   840,
       0,   234,   234,     0,     0,   234,     0,     0,     0,     0,
       0,     0,   234,     0,     0,  1347,     0,     0,  1350,   218,
       0,     0,     0,     0,     0,   278,     0,     0,   209,     0,
     840,   514,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,     0,     0,     0,   209,     0,     0,
      50,     0,  1312,     0,     0,     0,     0,     0,   350,   351,
       0,     0,     0,     0,   234,     0,     0,     0,     0,    50,
       0,     0,     0,     0,  1395,     0,     0,     0,  1155,   209,
     839,     0,   338,   338,   499,   500,   213,   214,   215,     0,
       0,   338,   234,   234,     0,     0,     0,     0,   908,     0,
     234,    50,   234,     0,   559,   213,   214,   215,   560,   352,
       0,     0,    91,    92,     0,    93,   179,    95,     0,     0,
       0,     0,     0,     0,   234,   178,   234,     0,    89,   329,
       0,    91,    92,   234,    93,   179,    95,   213,   214,   215,
     105,  1426,  1427,     0,     0,     0,     0,     0,     0,   333,
       0,   501,   502,     0,     0,     0,     0,     0,     0,   105,
     334,   428,     0,    91,    92,     0,    93,   179,    95,     0,
       0,     0,   839,   839,   839,     0,     0,     0,     0,   839,
       0,   234,     0,   443,   444,   445,     0,   234,     0,   234,
       0,   105,     0,     0,     0,     0,     0,     0,   220,   220,
       0,     0,   236,   446,   447,     0,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,     0,
     471,     0,     0,   209,     0,     0,     0,     0,  1508,     0,
    1509,     0,   472,   840,     0,   985,   986,     0,     0,     0,
       0,     0,     0,     0,     0,    50,   338,   338,   840,   840,
     840,   840,   840,     0,     0,   987,     0,     0,     0,   840,
       0,     0,     0,   988,   989,   990,   209,  1533,     0,     0,
       0,     0,     0,     0,  1553,     0,   991,     0,     0,   234,
    1534,   213,   214,   215,  1535,     0,     0,   209,    50,   210,
      40,     0,     0,     0,     0,     0,   234,     0,     0,     0,
       0,   178,     0,     0,    89,    90,     0,    91,    92,    50,
      93,  1537,    95,     0,     0,   234,     0,     0,     0,     0,
       0,   839,   209,   992,   993,   994,   995,     0,     0,     0,
       0,     0,   839,   338,     0,   105,   209,     0,   839,   996,
       0,     0,   839,     0,    50,   213,   214,   215,     0,   338,
      91,    92,     0,    93,   179,    95,     0,     0,    50,     0,
       0,   940,   338,     0,   234,     0,   860,   861,   997,   757,
     840,    91,    92,     0,    93,   179,    95,     0,   105,     0,
     213,   214,   215,     0,     0,     0,     0,   220,     0,     0,
       0,   338,     0,     0,   213,   214,   215,     0,     0,   105,
       0,     0,     0,   793,   839,   109,    91,    92,     0,    93,
     179,    95,     0,     0,   234,     0,     0,   862,     0,     0,
      91,    92,     0,    93,   179,    95,     0,     0,     0,     0,
       0,   234,     0,     0,   105,     0,     0,     0,     0,     0,
     234,     0,     0,     0,     0,     0,     0,     0,   105,  1756,
       0,     0,   234,     0,   234,     0,    34,    35,    36,   338,
       0,     0,   338,     0,   821,     0,     0,     0,     0,   211,
       0,     0,     0,   234,     0,   234,     0,     0,     0,   840,
     840,   840,   840,     0,     0,     0,     0,   840,   840,   840,
     840,   840,   840,   840,   840,   840,   840,   840,   840,   840,
     840,   840,   840,   840,   840,   840,   840,   840,   840,   840,
     840,   840,   840,   840,   840,     0,     0,     0,     0,     0,
     220,    79,    80,    81,    82,    83,     0,     0,     0,   220,
       0,   840,   216,     0,     0,     0,   220,     0,    87,    88,
     209,     0,   902,     0,   903,     0,   220,     0,   443,   444,
     445,  1779,    97,     0,     0,     0,     0,   220,     0,     0,
       0,     0,    50,     0,     0,   338,   102,   338,   446,   447,
       0,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   338,   471,     0,   338,   213,   214,
     215,     0,     0,     0,     0,     0,     0,   472, -1039, -1039,
   -1039, -1039, -1039,   463,   464,   465,   466,   467,   468,   469,
     470,     0,   471,     0,    91,    92,     0,    93,   179,    95,
       0,     0,   236,     0,   472,     0,  1840,     0,     0,     0,
       0,     0,     0,   840,     0,     0,     0,     0,     0,     0,
       0,     0,   105,   338,     0,     0,     0,   338,     0,     0,
     840,     0,   840,     0,   443,   444,   445,     0,     0,     0,
       0,     0,   220,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   446,   447,   840,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
       0,   471,   935,     0,     0,     0,     0,     0,     0,     0,
     338,   338,     0,   472,  1902,     0,   935,     0,     0,   845,
       0,     0,     0,     0,     0,     0,   979,     0,   443,   444,
     445,     0,     0,     0,     0,  1902,     0,  1927,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   446,   447,
     845,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,     0,   471,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   472,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   338,   471,   338,
       0,     0,     0,     0,     0,   840,     0,   840,     0,   840,
     472,     0,   840,     0,   209,     0,   840,     0,   840,     0,
       0,   840,   220,     0,   443,   444,   445,   222,   222,     0,
     473,   240,   338,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,   338,   446,   447,     0,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
       0,   471,   213,   214,   215,     0,     0,     0,     0,     0,
       0,   220,     0,   472,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   352,     0,     0,    91,    92,
       0,    93,   179,    95,     0,     0,   983,   209,     0,     0,
       0,   840,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   220,     0,   220,     0,   105,     0,     0,    50,
       0,   338,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1025,     0,     0,     0,     0,     0,
       0,     0,   220,   845,     0,   338,     0,   338,     0,     0,
       0,     0,     0,     0,   338,   213,   214,   215,   845,   845,
     845,   845,   845,     0,     0,     0,    29,     0,     0,   845,
       0,     0,     0,     0,    34,    35,    36,   209,   862,   210,
      40,    91,    92,  1097,    93,   179,    95,   211,     0,     0,
       0,     0,     0,   840,   840,   840,     0,     0,     0,    50,
     840,     0,  1109,     0,     0,   220,     0,     0,   338,   105,
       0,     0,     0,     0,     0,     0,   222,     0,     0,  1118,
     212,   220,   220,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1026,    75,   213,   214,   215,     0,    79,
      80,    81,    82,    83,  1118,     0,     0,     0,     0,     0,
     216,     0,     0,   220,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,     0,     0,     0,
      97,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     845,     0,     0,  1162,   102,     0,     0,     0,     0,   105,
     217,     0,     0,     0,     0,   109,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   236,     0,     0,     0,     0,
     338,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   338,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,  1800,   471,     0,   222,
       0,     0,   840,     0,     0,   220,   220,     0,   222,   472,
       0,     0,     0,   840,     0,   222,     0,     0,     0,   840,
       0,     0,     0,   840,     0,   222,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   240,     0,     0,   845,
     845,   845,   845,     0,   220,   338,     0,   845,   845,   845,
     845,   845,   845,   845,   845,   845,   845,   845,   845,   845,
     845,   845,   845,   845,   845,   845,   845,   845,   845,   845,
     845,   845,   845,   845,   845,     0,     0,    34,    35,    36,
     209,     0,   210,    40,     0,   840,     0,     0,     0,     0,
     211,   845,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,   220,     0,     0,     0,     0,
       0,   240,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   338,     0,   231,     0,     0,     0,     0,     0,   232,
       0,     0,     0,   338,     0,   338,     0,   220,   213,   214,
     215,     0,    79,    80,    81,    82,    83,     0,     0,     0,
       0,   222,     0,   216,   338,     0,   338,     0,   178,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   179,    95,
       0,     0,     0,    97,     0,     0,     0,     0,     0,     0,
     220,     0,     0,     0,     0,   220,     0,   102,     0,     0,
       0,     0,   105,   233,     0,     0,     0,     0,   109,     0,
     220,   220,     0,   845,     0,     0,     0,     0,   846,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     845,     0,   845,     0,   443,   444,   445,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   846,
       0,     0,     0,     0,   446,   447,   845,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
       0,   471,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   472,  1429,     0,     0,   220,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   847,
       0,     0,     0,     0,     0,     0,     0,     0,   443,   444,
     445,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   222,     0,     0,     0,     0,     0,     0,   446,   447,
     872,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,     0,   471,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   472,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     222,     0,     0,     0,     0,   845,     0,   845,     0,   845,
       0,     0,   845,   220,     0,     0,   845,     0,   845,     0,
       0,   845,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1169,  1529,     0,     0,  1542,     0,     0,     0,
       0,   222,     0,   222,     0,     0,     0,     0,     0,     0,
     220,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,   222,   846,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1093,     0,     0,   846,   846,   846,
     846,   846,     0,     0,     0,   220,     0,     0,   846,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   845,     0,     0,     0,     0,  1180,     0,     0,     0,
       0,     0,     0,  1606,  1607,     0,     0,     0,     0,     0,
       0,     0,     0,  1542,   222, -1039, -1039, -1039, -1039, -1039,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,     0,
     222,   222,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1093,     0,  1029,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1051,  1052,
    1053,  1054,   240,     0,     0,     0,     0,     0,     0,  1064,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   845,   845,   845,     0,     0,     0,   846,
     845,     0,  1754,     0,     0,   443,   444,   445,     0,     0,
    1542,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   240,   446,   447,     0,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,     0,   471,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   472,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   222,   222,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1159,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   846,   846,
     846,   846,     0,   240,     0,     0,   846,   846,   846,   846,
     846,   846,   846,   846,   846,   846,   846,   846,   846,   846,
     846,   846,   846,   846,   846,   846,   846,   846,   846,   846,
     846,   846,   846,   846,     0,     0,     0,     0,     0,     0,
       0,     0,   845,     0,     0,     0,     0,     0,     0,     0,
     846,     0,     0,   845,     0,     0,     0,     0,     0,   845,
       0,     0,     0,   845,   222,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,     0,  1207,     0,     0,     0,     0,     0,     0,
    1246,  1248,  1248,     0,     0,  1093,   222,  1259,  1262,  1263,
    1264,  1266,  1267,  1268,  1269,  1270,  1271,  1272,  1273,  1274,
    1275,  1276,  1277,  1278,  1279,  1280,  1281,  1282,  1283,  1284,
    1285,  1286,  1287,  1288,  1289,   845,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1869,     0,     0,     0,   240,
       0,  1297,     0,     0,   222,     0,     0,     0,     0,     0,
       0,     0,  1529,     0,     0,     0,     0,     0,     0,   222,
     222,     0,   846,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   846,
       0,   846,     0,   443,   444,   445,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   446,   447,   846,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,     0,
     471,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   472,     0,     0,     0,   222,     0,     0,     0,
       0,     0,     0,  1385,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1400,     0,  1401,     0,     0,     0,     0,   443,   444,   445,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1419,   446,   447,     0,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,     0,   471,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   472,     0,     0,     0,
       0,     0,     0,     0,   846,     0,   846,     0,   846,     0,
       0,   846,   240,     0,     0,   846,     0,   846,     0,     0,
     846,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1566,     0,   443,   444,   445,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   222,
       0,     0,     0,   446,   447,  1390,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,     0,
     471,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   472,     0,   240,  1511,     0,  1512,     0,  1513,
       0,     0,  1514,     0,     0,     0,  1516,     0,  1517,     0,
     846,  1518,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,  1567,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   403,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,   740,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,  1601,   846,   846,   846,     0,     0,     0,     0,   846,
       0,     0,    50,     0,     0,     0,     0,     0,  1759,  1391,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   174,   175,   176,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   177,    75,    76,    77,
      78,     0,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,    85,     0,     0,     0,     0,   178,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   179,    95,
       0,     0,     0,    97,     0,     0,    98,     0,     0,     0,
       0,     0,    99,  1747,  1748,  1749,     0,   102,   103,   104,
    1753,     0,   105,   106,     0,     0,     0,     0,   109,   110,
       0,   111,   112,     0,     0,     0,     0,     0,     0,    29,
       0,     0,     0,     0,     0,     0,     0,    34,    35,    36,
     209,     0,   210,    40,     0,     0,     0,     0,     0,     0,
     211,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,   846,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   846,   212,     0,     0,     0,     0,   846,     0,
       0,     0,   846,     0,     0,     0,     0,    75,   213,   214,
     215,     0,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,   216,     0,     0,  1842,     0,   178,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   179,    95,
       0,     0,     0,    97,     0,     0,     0,     0,     0,     0,
       0,     5,     6,     7,     8,     9,     0,   102,     0,     0,
       0,    10,   105,   217,   846,     0,   593,     0,   109,     0,
       0,     0,  1809,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,  1819,     0,     0,     0,     0,     0,  1824,
       0,     0,     0,  1826,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,  1861,    52,    53,    54,    55,
      56,    57,    58,     0,    59,    60,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
       0,    79,    80,    81,    82,    83,     0,     0,     0,    84,
       0,     0,    85,     0,     0,     0,     0,    86,    87,    88,
      89,    90,     0,    91,    92,     0,    93,    94,    95,    96,
       0,     0,    97,     0,     0,    98,     0,     0,     0,     0,
       0,    99,   100,     0,   101,     0,   102,   103,   104,     0,
       0,   105,   106,     0,   107,   108,  1129,   109,   110,     0,
     111,   112,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,    56,    57,    58,     0,    59,    60,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,     0,    79,    80,    81,    82,    83,     0,     0,     0,
      84,     0,     0,    85,     0,     0,     0,     0,    86,    87,
      88,    89,    90,     0,    91,    92,     0,    93,    94,    95,
      96,     0,     0,    97,     0,     0,    98,     0,     0,     0,
       0,     0,    99,   100,     0,   101,     0,   102,   103,   104,
       0,     0,   105,   106,     0,   107,   108,  1313,   109,   110,
       0,   111,   112,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,    56,    57,    58,     0,    59,    60,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,     0,    79,    80,    81,    82,    83,     0,     0,
       0,    84,     0,     0,    85,     0,     0,     0,     0,    86,
      87,    88,    89,    90,     0,    91,    92,     0,    93,    94,
      95,    96,     0,     0,    97,     0,     0,    98,     0,     0,
       0,     0,     0,    99,   100,     0,   101,     0,   102,   103,
     104,     0,     0,   105,   106,     0,   107,   108,     0,   109,
     110,     0,   111,   112,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,     0,    79,    80,    81,    82,    83,     0,
       0,     0,    84,     0,     0,    85,     0,     0,     0,     0,
     178,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     179,    95,    96,     0,     0,    97,     0,     0,    98,     0,
       0,     0,     0,     0,    99,     0,     0,     0,     0,   102,
     103,   104,     0,     0,   105,   106,     0,   107,   108,   666,
     109,   110,     0,   111,   112,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   178,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   179,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,   106,     0,   107,   108,
    1096,   109,   110,     0,   111,   112,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,     0,    79,    80,    81,    82,
      83,     0,     0,     0,    84,     0,     0,    85,     0,     0,
       0,     0,   178,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   179,    95,    96,     0,     0,    97,     0,     0,
      98,     0,     0,     0,     0,     0,    99,     0,     0,     0,
       0,   102,   103,   104,     0,     0,   105,   106,     0,   107,
     108,  1143,   109,   110,     0,   111,   112,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,     0,    79,    80,    81,
      82,    83,     0,     0,     0,    84,     0,     0,    85,     0,
       0,     0,     0,   178,    87,    88,    89,    90,     0,    91,
      92,     0,    93,   179,    95,    96,     0,     0,    97,     0,
       0,    98,     0,     0,     0,     0,     0,    99,     0,     0,
       0,     0,   102,   103,   104,     0,     0,   105,   106,     0,
     107,   108,  1213,   109,   110,     0,   111,   112,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
    1215,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,     0,    79,    80,
      81,    82,    83,     0,     0,     0,    84,     0,     0,    85,
       0,     0,     0,     0,   178,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   179,    95,    96,     0,     0,    97,
       0,     0,    98,     0,     0,     0,     0,     0,    99,     0,
       0,     0,     0,   102,   103,   104,     0,     0,   105,   106,
       0,   107,   108,     0,   109,   110,     0,   111,   112,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,  1386,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,    96,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
     106,     0,   107,   108,     0,   109,   110,     0,   111,   112,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,     0,
      79,    80,    81,    82,    83,     0,     0,     0,    84,     0,
       0,    85,     0,     0,     0,     0,   178,    87,    88,    89,
      90,     0,    91,    92,     0,    93,   179,    95,    96,     0,
       0,    97,     0,     0,    98,     0,     0,     0,     0,     0,
      99,     0,     0,     0,     0,   102,   103,   104,     0,     0,
     105,   106,     0,   107,   108,  1520,   109,   110,     0,   111,
     112,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
       0,    79,    80,    81,    82,    83,     0,     0,     0,    84,
       0,     0,    85,     0,     0,     0,     0,   178,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   179,    95,    96,
       0,     0,    97,     0,     0,    98,     0,     0,     0,     0,
       0,    99,     0,     0,     0,     0,   102,   103,   104,     0,
       0,   105,   106,     0,   107,   108,  1750,   109,   110,     0,
     111,   112,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,  1796,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,     0,    79,    80,    81,    82,    83,     0,     0,     0,
      84,     0,     0,    85,     0,     0,     0,     0,   178,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   179,    95,
      96,     0,     0,    97,     0,     0,    98,     0,     0,     0,
       0,     0,    99,     0,     0,     0,     0,   102,   103,   104,
       0,     0,   105,   106,     0,   107,   108,     0,   109,   110,
       0,   111,   112,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,     0,    79,    80,    81,    82,    83,     0,     0,
       0,    84,     0,     0,    85,     0,     0,     0,     0,   178,
      87,    88,    89,    90,     0,    91,    92,     0,    93,   179,
      95,    96,     0,     0,    97,     0,     0,    98,     0,     0,
       0,     0,     0,    99,     0,     0,     0,     0,   102,   103,
     104,     0,     0,   105,   106,     0,   107,   108,  1831,   109,
     110,     0,   111,   112,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,  1834,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,     0,    79,    80,    81,    82,    83,     0,
       0,     0,    84,     0,     0,    85,     0,     0,     0,     0,
     178,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     179,    95,    96,     0,     0,    97,     0,     0,    98,     0,
       0,     0,     0,     0,    99,     0,     0,     0,     0,   102,
     103,   104,     0,     0,   105,   106,     0,   107,   108,     0,
     109,   110,     0,   111,   112,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   178,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   179,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,   106,     0,   107,   108,
    1851,   109,   110,     0,   111,   112,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,     0,    79,    80,    81,    82,
      83,     0,     0,     0,    84,     0,     0,    85,     0,     0,
       0,     0,   178,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   179,    95,    96,     0,     0,    97,     0,     0,
      98,     0,     0,     0,     0,     0,    99,     0,     0,     0,
       0,   102,   103,   104,     0,     0,   105,   106,     0,   107,
     108,  1868,   109,   110,     0,   111,   112,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,     0,    79,    80,    81,
      82,    83,     0,     0,     0,    84,     0,     0,    85,     0,
       0,     0,     0,   178,    87,    88,    89,    90,     0,    91,
      92,     0,    93,   179,    95,    96,     0,     0,    97,     0,
       0,    98,     0,     0,     0,     0,     0,    99,     0,     0,
       0,     0,   102,   103,   104,     0,     0,   105,   106,     0,
     107,   108,  1925,   109,   110,     0,   111,   112,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,     0,    79,    80,
      81,    82,    83,     0,     0,     0,    84,     0,     0,    85,
       0,     0,     0,     0,   178,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   179,    95,    96,     0,     0,    97,
       0,     0,    98,     0,     0,     0,     0,     0,    99,     0,
       0,     0,     0,   102,   103,   104,     0,     0,   105,   106,
       0,   107,   108,  1928,   109,   110,     0,   111,   112,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,    96,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
     106,     0,   107,   108,     0,   109,   110,     0,   111,   112,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,   542,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,    63,    64,   174,
     175,    67,     0,    68,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,     0,
      79,    80,    81,    82,    83,     0,     0,     0,    84,     0,
       0,    85,     0,     0,     0,     0,   178,    87,    88,    89,
      90,     0,    91,    92,     0,    93,   179,    95,     0,     0,
       0,    97,     0,     0,    98,     0,     0,     0,     0,     0,
      99,     0,     0,     0,     0,   102,   103,   104,     0,     0,
     105,   106,     0,   107,   108,     0,   109,   110,     0,   111,
     112,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
     806,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
     174,   175,    67,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
       0,    79,    80,    81,    82,    83,     0,     0,     0,    84,
       0,     0,    85,     0,     0,     0,     0,   178,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   179,    95,     0,
       0,     0,    97,     0,     0,    98,     0,     0,     0,     0,
       0,    99,     0,     0,     0,     0,   102,   103,   104,     0,
       0,   105,   106,     0,   107,   108,     0,   109,   110,     0,
     111,   112,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,  1031,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,   174,   175,    67,     0,    68,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,     0,    79,    80,    81,    82,    83,     0,     0,     0,
      84,     0,     0,    85,     0,     0,     0,     0,   178,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   179,    95,
       0,     0,     0,    97,     0,     0,    98,     0,     0,     0,
       0,     0,    99,     0,     0,     0,     0,   102,   103,   104,
       0,     0,   105,   106,     0,   107,   108,     0,   109,   110,
       0,   111,   112,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,  1596,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,    61,    62,
      63,    64,   174,   175,    67,     0,    68,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,     0,    79,    80,    81,    82,    83,     0,     0,
       0,    84,     0,     0,    85,     0,     0,     0,     0,   178,
      87,    88,    89,    90,     0,    91,    92,     0,    93,   179,
      95,     0,     0,     0,    97,     0,     0,    98,     0,     0,
       0,     0,     0,    99,     0,     0,     0,     0,   102,   103,
     104,     0,     0,   105,   106,     0,   107,   108,     0,   109,
     110,     0,   111,   112,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,  1742,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,   174,   175,    67,     0,    68,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,     0,    79,    80,    81,    82,    83,     0,
       0,     0,    84,     0,     0,    85,     0,     0,     0,     0,
     178,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     179,    95,     0,     0,     0,    97,     0,     0,    98,     0,
       0,     0,     0,     0,    99,     0,     0,     0,     0,   102,
     103,   104,     0,     0,   105,   106,     0,   107,   108,     0,
     109,   110,     0,   111,   112,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,   174,   175,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   178,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   179,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,   106,     0,   107,   108,
       0,   109,   110,     0,   111,   112,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   174,   175,   176,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     177,    75,    76,    77,    78,     0,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,    85,     0,     0,
       0,     0,   178,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   179,    95,     0,     0,     0,    97,     0,     0,
      98,     0,     0,     0,     0,     0,    99,     0,     0,     0,
       0,   102,   103,   104,     0,     0,   105,   180,     0,   343,
       0,     0,   109,   110,     0,   111,   112,     5,     6,     7,
       8,     9,     0,     0,     0,  1070,     0,    10,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
       0,     0,   681,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1093,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   174,   175,   176,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   177,    75,    76,    77,    78,     0,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,    85,     0,
       0,     0,     0,   178,    87,    88,    89,    90,     0,    91,
      92,     0,    93,   179,    95,     0,   682,     0,    97,     0,
       0,    98,     0,     0,     0,     0,     0,    99,     0,     0,
       0,     0,   102,   103,   104,     0,     0,   105,   180,     0,
       0,     0,     0,   109,   110,     0,   111,   112,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   174,   175,   176,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   177,    75,    76,    77,    78,     0,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,    85,
       0,     0,     0,     0,   178,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   179,    95,     0,     0,     0,    97,
       0,     0,    98,     0,     0,     0,     0,     0,    99,     0,
       0,     0,     0,   102,   103,   104,     0,     0,   105,   180,
       0,     0,   801,     0,   109,   110,     0,   111,   112,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,     0,     0,  1156,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1093,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   174,   175,
     176,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   177,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
      85,     0,     0,     0,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,     0,  1157,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
     180,     0,     0,     0,     0,   109,   110,     0,   111,   112,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   403,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   174,
     175,   176,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   177,    75,    76,    77,    78,     0,
      79,    80,    81,    82,    83,     0,     0,     0,     0,     0,
       0,    85,     0,     0,     0,     0,   178,    87,    88,    89,
      90,     0,    91,    92,     0,    93,   179,    95,     0,     0,
       0,    97,     0,     0,    98,     0,     0,     0,     0,     0,
      99,     0,     0,     0,     0,   102,   103,   104,     0,     0,
     105,   106,   443,   444,   445,     0,   109,   110,     0,   111,
     112,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   446,   447,     0,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,     0,   471,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,   472,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,   191,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     174,   175,   176,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   177,    75,    76,    77,    78,
       0,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,    85,     0,     0,     0,     0,   178,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   179,    95,     0,
       0,     0,    97,     0,     0,    98,     0,     0,   556,     0,
       0,    99,     0,     0,     0,     0,   102,   103,   104,     0,
       0,   105,   180,     0,     0,     0,     0,   109,   110,     0,
     111,   112,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,     0,   471,     0,   226,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   472,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   174,   175,   176,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   177,    75,    76,    77,
      78,     0,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,    85,     0,     0,     0,     0,   178,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   179,    95,
       0,     0,     0,    97,     0,     0,    98,     0,     0,     0,
       0,     0,    99,     0,     0,     0,     0,   102,   103,   104,
       0,     0,   105,   180,   443,   444,   445,     0,   109,   110,
       0,   111,   112,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   446,   447,     0,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
       0,   471,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,   472,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   174,   175,   176,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   177,    75,    76,
      77,    78,     0,    79,    80,    81,    82,    83,     0,     0,
       0,     0,     0,     0,    85,     0,     0,     0,     0,   178,
      87,    88,    89,    90,     0,    91,    92,     0,    93,   179,
      95,     0,     0,     0,    97,     0,     0,    98,     0,     0,
     558,     0,     0,    99,     0,     0,     0,     0,   102,   103,
     104,     0,     0,   105,   180,     0,   261,   444,   445,   109,
     110,     0,   111,   112,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,   446,   447,     0,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,     0,   471,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,   472,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   174,   175,   176,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   177,    75,
      76,    77,    78,     0,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,    85,     0,     0,     0,     0,
     178,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     179,    95,     0,     0,     0,    97,     0,     0,    98,     0,
       0,     0,     0,     0,    99,     0,     0,     0,     0,   102,
     103,   104,     0,     0,   105,   180,     0,   264,     0,     0,
     109,   110,     0,   111,   112,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   403,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   174,   175,   176,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   177,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   178,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   179,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,   106,   443,   444,   445,
       0,   109,   110,     0,   111,   112,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   446,   447,     0,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,     0,   471,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,   472,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   174,   175,   176,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     177,    75,    76,    77,    78,     0,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,    85,     0,     0,
       0,     0,   178,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   179,    95,     0,     0,     0,    97,     0,     0,
      98,     0,     0,   577,     0,     0,    99,     0,     0,     0,
       0,   102,   103,   104,     0,     0,   105,   180,   540,     0,
       0,     0,   109,   110,     0,   111,   112,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,  1092,   695,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1093,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   174,   175,   176,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   177,    75,    76,    77,    78,     0,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,    85,     0,
       0,     0,     0,   178,    87,    88,    89,    90,     0,    91,
      92,     0,    93,   179,    95,     0,     0,     0,    97,     0,
       0,    98,     0,     0,     0,     0,     0,    99,     0,     0,
       0,     0,   102,   103,   104,     0,     0,   105,   180,     0,
       0,     0,     0,   109,   110,     0,   111,   112,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
       0,     0,     0,   740,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1093,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   174,   175,   176,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   177,    75,    76,    77,    78,     0,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,    85,
       0,     0,     0,     0,   178,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   179,    95,     0,     0,     0,    97,
       0,     0,    98,     0,     0,     0,     0,     0,    99,     0,
       0,     0,     0,   102,   103,   104,     0,     0,   105,   180,
       0,     0,     0,     0,   109,   110,     0,   111,   112,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
       0,   471,     0,     0,   781,     0,     0,     0,     0,     0,
       0,     0,     0,   472,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   174,   175,
     176,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   177,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
      85,     0,     0,     0,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,     0,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
     180,     0,     0,     0,     0,   109,   110,     0,   111,   112,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
       0,     0,     0,     0,     0,   783,     0,     0,     0,     0,
       0,     0,     0,  1093,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   174,
     175,   176,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   177,    75,    76,    77,    78,     0,
      79,    80,    81,    82,    83,     0,     0,     0,     0,     0,
       0,    85,     0,     0,     0,     0,   178,    87,    88,    89,
      90,     0,    91,    92,     0,    93,   179,    95,     0,     0,
       0,    97,     0,     0,    98,     0,     0,     0,     0,     0,
      99,     0,     0,     0,     0,   102,   103,   104,     0,     0,
     105,   180,     0,     0,     0,     0,   109,   110,     0,   111,
     112,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,     0,
     471,     0,     0,     0,     0,     0,  1124,     0,     0,     0,
       0,     0,   472,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     174,   175,   176,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   177,    75,    76,    77,    78,
       0,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,    85,     0,     0,     0,     0,   178,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   179,    95,     0,
       0,     0,    97,     0,     0,    98,     0,     0,     0,     0,
       0,    99,     0,     0,     0,     0,   102,   103,   104,     0,
       0,   105,   180,     0,     0,     0,     0,   109,   110,     0,
     111,   112,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10, -1039, -1039, -1039, -1039,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
       0,   471,     0,     0,     0,     0,     0,  1204,     0,     0,
       0,     0,     0,   472,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   174,   175,   176,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   177,    75,    76,    77,
      78,     0,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,    85,     0,     0,     0,     0,   178,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   179,    95,
       0,     0,     0,    97,     0,     0,    98,     0,     0,     0,
       0,     0,    99,     0,     0,     0,     0,   102,   103,   104,
       0,     0,   105,   180,     0,     0,     0,     0,   109,   110,
       0,   111,   112,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10, -1039, -1039, -1039, -1039,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,     0,     0,     0,     0,     0,     0,     0,  1449,     0,
       0,     0,     0,     0,  1093,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   174,   175,   176,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   177,    75,    76,
      77,    78,     0,    79,    80,    81,    82,    83,     0,     0,
       0,     0,     0,     0,    85,     0,     0,     0,     0,   178,
      87,    88,    89,    90,     0,    91,    92,     0,    93,   179,
      95,     0,     0,     0,    97,     0,     0,    98,     0,     0,
       0,     0,     0,    99,     0,     0,     0,     0,   102,   103,
     104,     0,     0,   105,   180,   443,   444,   445,     0,   109,
     110,     0,   111,   112,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   446,   447,     0,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,     0,   471,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,   472,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   174,   175,   176,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   177,    75,
      76,    77,    78,     0,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,    85,     0,     0,     0,     0,
     178,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     179,    95,     0,     0,     0,    97,     0,     0,    98,     0,
       0,   581,     0,     0,    99,     0,     0,     0,     0,   102,
     103,   104,     0,     0,   105,   180,   443,   444,   445,     0,
     109,   110,     0,   111,   112,     5,     6,     7,     8,     9,
       0,     0,     0,     0,   810,    10,   446,   447,     0,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,     0,   471,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   472,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,   628,    39,    40,     0,   811,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   174,   175,   176,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   177,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   178,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   179,    95,     0,   266,   267,    97,   268,   269,    98,
       0,   270,   271,   272,   273,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,   180,     0,   274,     0,
     275,   109,   110,     0,   111,   112,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   277,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   279,   280,   281,   282,   283,   284,   285,     0,
       0,     0,   209,     0,   210,    40,     0,     0,     0,     0,
       0,     0,     0,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,    50,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,     0,   320,
       0,   728,   322,   323,   324,     0,     0,     0,   325,   570,
     213,   214,   215,   571,     0,     0,     0,     0,     0,   266,
     267,     0,   268,   269,     0,     0,   270,   271,   272,   273,
     572,     0,     0,     0,     0,     0,    91,    92,     0,    93,
     179,    95,   330,   274,   331,   275,     0,   332,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   105,     0,     0,     0,   729,     0,
     109,     0,     0,   277,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   279,   280,   281,
     282,   283,   284,   285,     0,     0,     0,   209,     0,   210,
      40,     0,     0,     0,     0,     0,     0,     0,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,    50,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,     0,   320,     0,   321,   322,   323,   324,
       0,     0,     0,   325,   570,   213,   214,   215,   571,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   266,
     267,     0,   268,   269,     0,   572,   270,   271,   272,   273,
       0,    91,    92,     0,    93,   179,    95,   330,     0,   331,
       0,     0,   332,   274,     0,   275,     0,   276,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   105,
       0,     0,     0,   729,     0,   109,     0,     0,     0,     0,
       0,     0,     0,   277,     0,   278,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   279,   280,   281,
     282,   283,   284,   285,     0,     0,     0,   209,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,    50,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,     0,   320,     0,     0,   322,   323,   324,
       0,     0,     0,   325,   326,   213,   214,   215,   327,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   328,     0,     0,    89,   329,
       0,    91,    92,     0,    93,   179,    95,   330,     0,   331,
       0,     0,   332,   266,   267,     0,   268,   269,     0,   333,
     270,   271,   272,   273,     0,     0,     0,     0,     0,   105,
     334,     0,     0,     0,  1721,     0,     0,   274,     0,   275,
     447,   276,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,     0,   471,   277,     0,   278,
       0,     0,     0,     0,     0,     0,     0,     0,   472,     0,
       0,   279,   280,   281,   282,   283,   284,   285,     0,     0,
       0,   209,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,    50,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,     0,   320,     0,
       0,   322,   323,   324,     0,     0,     0,   325,   326,   213,
     214,   215,   327,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   328,
       0,     0,    89,   329,     0,    91,    92,     0,    93,   179,
      95,   330,     0,   331,     0,     0,   332,   266,   267,     0,
     268,   269,     0,   333,   270,   271,   272,   273,     0,     0,
       0,     0,     0,   105,   334,     0,     0,     0,  1791,     0,
       0,   274,     0,   275,     0,   276,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,     0,
     471,   277,     0,   278,     0,     0,     0,     0,     0,     0,
       0,     0,   472,     0,     0,   279,   280,   281,   282,   283,
     284,   285,     0,     0,     0,   209,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,    50,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,     0,   320,     0,   321,   322,   323,   324,     0,     0,
       0,   325,   326,   213,   214,   215,   327,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   328,     0,     0,    89,   329,     0,    91,
      92,     0,    93,   179,    95,   330,     0,   331,     0,     0,
     332,   266,   267,     0,   268,   269,     0,   333,   270,   271,
     272,   273,     0,     0,     0,     0,     0,   105,   334,     0,
       0,     0,     0,     0,     0,   274,     0,   275,     0,   276,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   277,     0,   278,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   279,
     280,   281,   282,   283,   284,   285,     0,     0,     0,   209,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,    50,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,     0,   320,     0,     0,   322,
     323,   324,     0,     0,     0,   325,   326,   213,   214,   215,
     327,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   328,     0,     0,
      89,   329,     0,    91,    92,     0,    93,   179,    95,   330,
       0,   331,     0,     0,   332,     0,   266,   267,     0,   268,
     269,   333,  1524,   270,   271,   272,   273,     0,     0,     0,
       0,   105,   334,     0,     0,     0,     0,     0,     0,     0,
     274,     0,   275,     0,   276,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     277,     0,   278,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   279,   280,   281,   282,   283,   284,
     285,     0,     0,     0,   209,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,    50,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
       0,   320,     0,     0,   322,   323,   324,     0,     0,     0,
     325,   326,   213,   214,   215,   327,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   328,     0,     0,    89,   329,     0,    91,    92,
       0,    93,   179,    95,   330,     0,   331,     0,     0,   332,
    1621,  1622,  1623,  1624,  1625,     0,   333,  1626,  1627,  1628,
    1629,     0,     0,     0,     0,     0,   105,   334,     0,     0,
       0,     0,     0,     0,  1630,  1631,  1632,     0,   446,   447,
       0,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,  1633,   471,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   472,  1634,  1635,
    1636,  1637,  1638,  1639,  1640,     0,     0,     0,   209,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1641,
    1642,  1643,  1644,  1645,  1646,  1647,  1648,  1649,  1650,  1651,
      50,  1652,  1653,  1654,  1655,  1656,  1657,  1658,  1659,  1660,
    1661,  1662,  1663,  1664,  1665,  1666,  1667,  1668,  1669,  1670,
    1671,  1672,  1673,  1674,  1675,  1676,  1677,  1678,  1679,  1680,
    1681,     0,     0,     0,  1682,  1683,   213,   214,   215,     0,
    1684,  1685,  1686,  1687,  1688,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1689,  1690,  1691,     0,
       0,     0,    91,    92,     0,    93,   179,    95,  1692,     0,
    1693,  1694,     0,  1695,     0,     0,     0,     0,     0,     0,
    1696,  1697,     0,  1698,     0,  1699,  1700,     0,   266,   267,
     105,   268,   269,  1068,  1069,   270,   271,   272,   273,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   274,  1070,   275,     0,  1071,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,     0,
       0,     0,   277,     0,     0,     0,     0,     0,     0,     0,
       0,  1093,     0,     0,     0,     0,   279,   280,   281,   282,
     283,   284,   285,     0,     0,     0,   209,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,    50,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,     0,   320,     0,   321,   322,   323,   324,     0,
       0,     0,   325,   570,   213,   214,   215,   571,     0,     0,
       0,     0,     0,   266,   267,     0,   268,   269,     0,     0,
     270,   271,   272,   273,   572,     0,     0,     0,     0,     0,
      91,    92,     0,    93,   179,    95,   330,   274,   331,   275,
       0,   332,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   105,     0,
       0,     0,     0,     0,     0,     0,     0,   277,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   279,   280,   281,   282,   283,   284,   285,     0,     0,
       0,   209,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,    50,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,     0,   320,     0,
    1251,   322,   323,   324,     0,     0,     0,   325,   570,   213,
     214,   215,   571,     0,     0,     0,     0,     0,   266,   267,
       0,   268,   269,     0,     0,   270,   271,   272,   273,   572,
       0,     0,     0,     0,     0,    91,    92,     0,    93,   179,
      95,   330,   274,   331,   275,     0,   332,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   105,     0,     0,     0,     0,     0,     0,
       0,     0,   277,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   279,   280,   281,   282,
     283,   284,   285,     0,     0,     0,   209,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,    50,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,     0,   320,     0,  1257,   322,   323,   324,     0,
       0,     0,   325,   570,   213,   214,   215,   571,     0,     0,
       0,     0,     0,   266,   267,     0,   268,   269,     0,     0,
     270,   271,   272,   273,   572,     0,     0,     0,     0,     0,
      91,    92,     0,    93,   179,    95,   330,   274,   331,   275,
       0,   332,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   105,     0,
       0,     0,     0,     0,     0,     0,     0,   277,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   279,   280,   281,   282,   283,   284,   285,     0,     0,
       0,   209,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,    50,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,     0,   320,     0,
       0,   322,   323,   324,     0,     0,     0,   325,   570,   213,
     214,   215,   571,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   572,
       0,     0,     0,     0,     0,    91,    92,     0,    93,   179,
      95,   330,     0,   331,     0,     0,   332,   443,   444,   445,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   105,     0,     0,     0,   446,   447,     0,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,     0,   471,   443,   444,   445,     0,     0,
       0,     0,     0,     0,     0,     0,   472,     0,     0,     0,
       0,     0,     0,     0,     0,   446,   447,     0,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,  1265,   471,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   472,     0,     0,     0,     0,   827,
     828,     0,     0,     0,     0,   829,     0,   830,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   831,
       0,     0,     0,     0,     0,     0,     0,    34,    35,    36,
     209,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     211,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   773,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   832,   833,   834,
     835,     0,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,   216,     0,     0,     0,     0,   178,    87,
      88,    89,   836,     0,    91,    92,     0,    93,   179,    95,
     798,   827,   828,    97,     0,     0,     0,   829,     0,   830,
       0,     0,   837,     0,     0,     0,     0,   102,     0,     0,
       0,   831,   105,   838,     0,     0,     0,     0,     0,    34,
      35,    36,   209,     0,  1067,  1068,  1069,     0,     0,     0,
       0,     0,   211,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,  1070,     0,     0,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   832,
     833,   834,   835,  1093,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,   216,     0,     0,     0,     0,
     178,    87,    88,    89,   836,     0,    91,    92,     0,    93,
     179,    95,    29,     0,     0,    97,     0,     0,     0,     0,
      34,    35,    36,   209,   837,   210,    40,     0,     0,   102,
       0,     0,     0,   211,   105,   838,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
    1239,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   613,
      75,   213,   214,   215,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,   216,     0,     0,     0,
       0,   178,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   179,    95,    29,     0,   974,    97,     0,     0,     0,
       0,    34,    35,    36,   209,     0,   210,    40,     0,     0,
     102,     0,     0,     0,   211,   105,   217,     0,     0,     0,
       0,   109,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   212,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,   213,   214,   215,     0,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,   216,     0,     0,
       0,     0,   178,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   179,    95,    29,     0,     0,    97,     0,     0,
       0,     0,    34,    35,    36,   209,     0,   210,    40,     0,
       0,   102,     0,     0,     0,   211,   105,   217,     0,     0,
       0,     0,   109,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1121,    75,   213,   214,   215,     0,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,   216,     0,
       0,     0,     0,   178,    87,    88,    89,    90,     0,    91,
      92,     0,    93,   179,    95,    29,     0,     0,    97,     0,
       0,     0,     0,    34,    35,    36,   209,     0,   210,    40,
       0,     0,   102,     0,     0,     0,   211,   105,   217,     0,
       0,     0,     0,   109,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   212,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,   213,   214,   215,     0,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,   216,
       0,     0,     0,     0,   178,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   179,    95,     0,     0,     0,    97,
       0,     0,   443,   444,   445,     0,     0,     0,     0,     0,
       0,     0,     0,   102,     0,     0,     0,     0,   105,   217,
       0,     0,   446,   447,   109,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,     0,   471,
     443,   444,   445,     0,     0,     0,     0,     0,     0,     0,
       0,   472,     0,     0,     0,     0,     0,     0,     0,     0,
     446,   447,     0,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,     0,   471,     0,     0,
       0,     0,     0,     0,     0,     0,   443,   444,   445,   472,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   446,   447,   517,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,     0,   471,   443,   444,   445,     0,     0,     0,
       0,     0,     0,     0,     0,   472,     0,     0,     0,     0,
       0,     0,     0,     0,   446,   447,   526,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
       0,   471,     0,     0,     0,     0,     0,     0,     0,     0,
     443,   444,   445,   472,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     446,   447,   894,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,     0,   471,   443,   444,
     445,     0,     0,     0,     0,     0,     0,     0,     0,   472,
       0,     0,     0,     0,     0,     0,     0,     0,   446,   447,
     960,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,     0,   471,     0,     0,     0,     0,
       0,     0,     0,     0,   443,   444,   445,   472,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   446,   447,  1010,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
       0,   471,  1067,  1068,  1069,     0,     0,     0,     0,     0,
       0,     0,     0,   472,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1070,  1311,     0,  1071,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1067,  1068,  1069,
       0,  1093,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1070,     0,
    1342,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,     0,     0,  1067,  1068,  1069,     0,     0,
       0,     0,     0,     0,     0,     0,  1093,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1070,     0,  1406,  1071,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1067,  1068,  1069,     0,  1093,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1070,     0,  1415,  1071,  1072,  1073,  1074,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,  1092,     0,     0,  1067,  1068,
    1069,     0,     0,     0,     0,     0,     0,     0,     0,  1093,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1070,
       0,  1510,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,     0,    34,    35,    36,   209,     0,
     210,    40,     0,     0,     0,     0,     0,  1093,   642,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,  1602,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   212,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   213,   214,   215,     0,
      79,    80,    81,    82,    83,     0,     0,     0,     0,     0,
       0,   216,     0,     0,  1604,     0,   178,    87,    88,    89,
      90,     0,    91,    92,     0,    93,   179,    95,     0,     0,
       0,    97,     0,    34,    35,    36,   209,     0,   210,    40,
       0,     0,     0,     0,     0,   102,   211,     0,     0,     0,
     105,   643,     0,     0,     0,     0,   109,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   231,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   213,   214,   215,     0,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,   216,
       0,     0,     0,     0,   178,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   179,    95,     0,     0,     0,    97,
       0,     0,     0,     0,     0,   443,   444,   445,     0,     0,
       0,     0,     0,   102,     0,     0,     0,     0,   105,   233,
       0,     0,     0,     0,   109,   446,   447,   957,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,     0,   471,   443,   444,   445,     0,     0,     0,     0,
       0,     0,     0,     0,   472,     0,     0,     0,     0,     0,
       0,     0,     0,   446,   447,     0,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,     0,
     471,  1067,  1068,  1069,     0,     0,     0,     0,     0,     0,
       0,     0,   472,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1070,  1420,     0,  1071,  1072,  1073,  1074,  1075,
    1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1092,  1067,  1068,  1069,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1093,     0,     0,     0,     0,     0,     0,     0,  1070,     0,
       0,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,   445,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1093,     0,     0,     0,
       0,   446,   447,     0,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,  1069,   471,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     472,     0,     0,     0,     0,     0,  1070,     0,     0,  1071,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1093
};

static const yytype_int16 yycheck[] =
{
       5,     6,    56,     8,     9,    10,    11,    12,    13,   127,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   157,     4,    29,    30,    96,    33,   182,     4,
     100,   101,   234,     4,   658,     4,   396,   106,    31,    44,
      46,   933,   511,   396,   396,    51,    57,    52,   530,    54,
    1302,   162,    57,  1146,    59,    31,   126,    31,    31,   922,
     684,   654,   471,   106,   635,   183,   655,    57,   507,   508,
       4,   587,   588,   809,   737,   106,   157,   503,  1142,    84,
     350,   351,   503,  1133,   542,   106,   816,  1024,   953,    44,
       9,    32,    49,    32,     9,    49,    38,   536,    56,     9,
       9,   106,    49,     9,   969,     9,     9,     9,    14,     9,
      14,    14,   538,     9,  1735,     9,     4,   538,     9,    49,
     578,    50,    51,    90,    38,    49,     9,    83,     9,    84,
       9,     9,     9,     9,     9,   246,   785,   180,    83,     9,
       9,    83,     9,     9,    60,     4,  1011,     4,    83,   180,
      90,   102,    70,   179,    38,    90,     9,   775,    83,   180,
       9,    49,   106,   107,     9,    38,   109,    36,   194,    83,
      86,  1792,   164,    89,   217,   180,   164,   123,   245,   179,
      70,     0,   187,   535,   130,   177,   217,   194,   155,   115,
     233,   194,    49,   194,   194,   122,   217,   158,   656,    83,
    1063,    38,   233,   130,     8,   197,    70,    54,     4,   197,
      83,   162,   217,   158,    70,   155,   134,   135,   179,   229,
     155,   156,   157,   157,   112,   197,    70,    70,   233,   117,
     172,   119,   120,   121,   122,   123,   124,   125,   195,    70,
     864,   195,   247,   199,   191,   250,    83,   172,   195,    70,
     176,   180,   257,   258,   198,   112,   197,   196,   172,    70,
     117,   191,   119,   120,   121,   122,   123,   124,   125,  1521,
     251,   195,    70,  1337,   255,   179,   195,   165,   166,   433,
     168,   196,   197,  1220,    70,     4,   196,   196,   342,  1339,
     196,    54,   196,   196,   196,   195,  1346,    70,  1348,   172,
     196,   189,   196,    70,  1034,   196,  1036,   197,   165,   166,
     198,   168,  1177,   196,   381,   196,   163,   196,   196,   196,
     196,   196,   158,   972,  1374,   195,   195,   370,   195,   195,
     194,   127,   189,   197,    53,   172,  1199,    56,   194,   370,
     911,   198,   195,   801,   860,   861,   195,   158,   806,   370,
     195,   194,    70,   197,    73,   134,   135,   427,   194,   513,
     158,   950,   102,   194,   369,   370,   197,   485,    70,   102,
     424,   376,   377,   378,   379,    94,   197,    96,    57,   384,
      70,   100,   101,   194,   342,    14,   197,   183,   194,    70,
      69,   158,   162,    70,    83,    70,    70,    70,   403,   197,
     163,    70,   194,  1467,   194,  1469,   411,   126,   194,   195,
     480,   481,   482,   483,   369,   178,   194,   486,   423,   198,
      70,   890,   162,   378,   379,    83,   196,   197,  1478,   162,
     197,    70,    83,    84,   350,   351,   352,    83,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   424,   472,  1131,   474,
     475,   476,   388,   102,   103,   486,   410,   196,   471,   197,
    1216,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,    70,   471,   486,   471,   471,   504,
     505,   190,   507,   508,   509,   510,   194,   197,   197,   514,
     520,  1575,   517,   939,   172,  1579,   197,    31,   939,   477,
     197,   526,   197,   528,   197,   194,   172,   966,   197,   731,
      83,   536,   251,   544,   477,   194,   255,   672,   102,   544,
     259,   546,   535,   402,   194,    59,   197,   197,   199,  1412,
      83,    84,  1145,   194,    70,    83,     4,   194,   134,   135,
     503,   763,   922,   194,  1431,   974,   524,    81,   549,   922,
     922,    38,  1161,  1031,    70,  1164,  1194,  1195,  1196,    27,
      28,   524,  1200,   475,   854,   739,   856,   194,   593,   103,
    1106,   672,   535,    14,   661,   538,   663,   111,   162,    32,
     643,   106,   107,   156,   157,   134,   135,   163,    83,   619,
     620,    32,   504,   162,   410,    90,    83,   509,   134,   135,
     690,   197,    83,   342,   138,   139,   140,   141,   142,    90,
      51,  1494,   194,    83,    38,   645,   196,   839,   643,   196,
      90,   196,   158,    70,   172,   159,   158,   849,   162,   163,
    1902,   165,   166,    83,   168,   169,   170,   196,   106,   196,
      90,    83,   196,  1530,   196,   194,   199,   179,    90,   183,
    1734,   587,   588,   196,  1738,   786,    70,   682,   194,   193,
     194,   156,   157,    83,  1347,   197,    70,  1554,   484,  1556,
     695,   410,    83,   198,   179,   156,   157,  1179,    70,    90,
      70,   420,   197,  1063,   158,   424,   156,   157,   427,   194,
    1063,  1063,   779,   780,    53,    54,    55,   727,    57,   786,
     787,    31,   134,   135,   729,   155,   156,   157,   203,   194,
      69,  1170,   180,  1326,   156,   157,     4,  1355,   672,  1357,
      50,   202,  1181,    53,   119,   120,   121,   122,   123,   124,
    1219,   194,  1341,   758,    70,   155,   156,   157,   477,   478,
     479,   480,   481,   482,   483,   156,   157,    31,   158,   217,
     119,   120,   121,   122,   123,   124,   158,   194,   226,    50,
      51,    49,   111,  1446,   503,   233,    50,  1880,   793,    53,
     119,   120,   121,   122,   123,   124,   655,   179,   162,   805,
     196,  1894,   179,   251,   809,   524,    48,   255,   818,    69,
    1874,   179,   194,   823,   189,   197,   179,   194,   158,   538,
     194,   196,    53,    54,    55,  1889,    27,    28,   382,   194,
     549,   194,   386,   814,  1292,    75,    76,  1306,    69,  1199,
     189,   119,   120,   121,   112,   201,  1199,  1199,     9,   117,
     569,   119,   120,   121,   122,   123,   124,   125,   158,   413,
     189,   415,   416,   417,   418,   799,   158,  1485,  1531,  1487,
     194,  1489,   591,   592,  1492,     8,   886,   105,   106,   107,
     194,    31,    50,    51,    52,    53,    54,    55,   196,   894,
    1479,   896,   158,   898,   105,   106,   107,   165,   166,    14,
     168,    69,   907,   973,  1343,   624,   625,   110,   111,   112,
     158,   359,    75,    76,     4,   196,   921,   851,  1376,   196,
     368,   189,   370,   102,   103,   132,   133,   375,   197,   910,
     198,    81,   350,   351,  1392,   910,     9,   385,   854,   910,
     856,   910,   947,   196,   860,   861,   862,   196,   197,   130,
    1152,    14,   957,   103,  1846,   960,   130,   962,   195,    49,
     179,   966,   410,   196,   197,  1766,  1767,   925,  1586,  1762,
    1763,   690,    14,   102,  1866,   125,   910,   825,   826,   195,
     195,   974,   925,  1875,   111,   195,   195,   200,   138,   139,
     140,   141,   142,     9,   937,  1005,   939,   931,   974,   194,
     974,   974,   194,   799,   194,  1010,   155,   195,   195,   159,
     195,  1017,   162,   163,    94,   165,   166,   195,   168,   169,
     170,     9,   112,   196,    14,   226,  1465,   117,   179,   119,
     120,   121,   122,   123,   124,   125,   194,  1018,     9,  1497,
    1050,   194,   197,   193,   196,    83,   132,  1057,  1506,   197,
     196,   910,  1412,   195,  1123,   851,   775,   195,   777,  1412,
    1412,  1519,   196,   511,   195,   195,   194,     9,     9,  1012,
      70,    32,   133,   178,   933,   165,   166,   201,   168,   201,
     799,   201,   158,   136,     9,   195,  1020,   158,  1022,   191,
      14,   950,     9,     9,   813,   814,   180,   195,     9,   189,
      14,   549,   132,     9,  1062,   201,   201,  1065,   198,   198,
      14,   194,  1123,   201,   910,   158,   195,   195,  1123,   201,
     195,   102,  1740,  1741,    50,    51,    52,    53,    54,    55,
     196,    57,   851,     9,  1494,   931,   196,    91,  1596,   858,
     859,  1494,  1494,    69,   136,   158,     9,   195,   194,  1130,
      70,    70,  1157,   158,   194,  1130,   158,    70,   359,  1130,
     197,  1130,  1786,   197,     9,  1170,   198,   368,   887,   370,
      14,   196,   180,     9,   375,   197,  1181,  1182,   201,    14,
     197,  1191,    14,   195,   385,   196,    32,   191,    27,    28,
    1106,   910,    32,   194,    14,   194,  1130,   194,   194,    14,
      52,   194,  1208,    70,    70,    70,   925,   194,     9,   195,
     194,  1216,   931,   196,   196,   136,    14,   180,   937,     9,
     939,  1226,   195,   201,  1020,     9,  1022,   136,    69,    83,
    1211,   198,   198,   681,   196,  1245,     9,   194,   136,  1249,
    1864,   194,    14,   195,  1254,   196,    83,   197,   194,   194,
     197,  1261,   195,   197,   973,   136,   196,     9,    91,   155,
     197,    32,   201,    77,   196,   195,   985,   986,   987,   196,
     180,  1130,   136,    32,  1732,   136,  1900,   195,   201,     9,
     195,     9,     9,   195,  1742,     9,    83,     9,    14,  1223,
     198,   194,   740,  1012,   201,   195,   201,    31,   197,  1018,
     195,  1020,  1161,  1022,   196,  1164,  1311,   196,   198,   195,
     511,   195,  1447,  1318,   196,   195,   194,  1322,   197,  1324,
     195,     9,     9,  1042,  1442,   136,   136,  1332,   201,  1787,
     201,   201,    32,   781,  1130,   783,   195,  1342,  1343,   196,
     195,   197,   195,  1062,   112,   196,  1065,    81,   196,   167,
       4,   799,  1362,   196,   163,    14,  1366,    91,    83,   117,
     136,  1371,   136,   811,   195,   195,   814,   195,   197,   103,
    1304,    14,   179,   197,  1832,  1094,    83,   196,    14,    14,
      83,  1315,   195,   195,    14,   136,   194,   226,   136,    14,
     196,   196,   196,    14,   197,    49,     9,     9,  1552,   198,
      83,    59,   179,   851,    83,   139,   140,   141,   194,     9,
     197,  1130,   102,   196,   115,   158,   102,   180,    36,    27,
      28,   170,    14,   176,   194,   159,   874,  1223,   162,   195,
     194,   165,   166,   180,   168,   169,   170,    83,   180,    81,
     196,  1899,   890,   891,   173,  1304,  1904,     9,    14,   195,
      83,   196,   193,   197,  1524,   195,  1315,  1462,   112,   193,
    1465,   103,   910,   117,   195,   119,   120,   121,   122,   123,
     124,   125,    83,     9,  1193,  1194,  1195,  1196,    83,    14,
     681,  1200,  1341,   931,    14,     4,    83,    14,  1422,    83,
    1106,  1855,  1211,   478,   483,   967,  1871,   139,   140,   141,
     480,  1217,   913,  1595,  1223,  1866,  1582,  1389,   595,  1619,
    1532,   165,   166,  1447,   168,  1704,  1437,  1911,  1887,  1716,
     359,   163,  1503,   165,   166,   167,   168,   169,   170,   368,
      49,  1578,  1433,  1196,  1468,   189,   375,   379,  1134,   740,
    1474,  1061,  1476,  1058,   198,   986,   385,  1192,  1007,  1193,
     937,   193,   194,   376,  1898,   424,  1913,   396,   825,  1821,
    1425,  1043,  1496,  1114,  1498,  1570,  1720,  1094,    -1,  1527,
    1018,    -1,  1020,  1507,  1022,    -1,  1024,  1025,    -1,  1298,
     781,    -1,   783,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    31,    -1,   112,    -1,    -1,    -1,    -1,   117,    -1,
     119,   120,   121,   122,   123,   124,   125,    -1,    -1,  1468,
     811,    -1,    -1,  1594,  1595,  1474,    56,  1476,   226,     4,
    1479,    -1,    -1,    -1,    -1,    -1,  1422,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,  1355,  1496,  1357,   130,
     131,    -1,    -1,    -1,    -1,  1441,   165,   166,    -1,   168,
    1584,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1593,
      -1,  1715,    -1,    -1,    49,  1599,    -1,    -1,    -1,    -1,
     189,  1605,   511,   874,    -1,    -1,  1124,    -1,   169,   198,
     171,    -1,  1130,    -1,    -1,    -1,    -1,    -1,    -1,   890,
     891,    -1,    -1,   184,    -1,   186,    -1,    -1,   189,    -1,
      -1,    -1,  1498,  1422,  1839,    -1,    -1,    -1,  1156,    -1,
    1780,  1507,  1431,    -1,    -1,    -1,    -1,    -1,  1437,    -1,
    1725,    -1,    -1,    -1,    -1,  1584,    -1,   112,    78,    79,
      80,    81,   117,    -1,   119,   120,   121,   122,   123,   124,
     125,  1859,    -1,    -1,    -1,    -1,  1756,  1757,    -1,    -1,
      -1,   359,    -1,   103,    -1,    -1,  1204,  1715,    -1,    -1,
     368,    -1,    -1,  1211,  1560,    -1,  1485,   375,  1487,    -1,
    1489,  1219,  1220,  1492,    -1,  1223,    -1,   385,    -1,  1498,
     165,   166,    -1,   168,  1503,    -1,   226,    -1,  1507,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,  1593,    -1,    -1,
      -1,    -1,    -1,  1599,   189,  1524,    -1,    -1,  1527,  1605,
      -1,  1530,  1746,   198,    -1,   165,   166,    -1,   168,   169,
     170,  1540,    -1,  1024,  1025,    81,    -1,    -1,  1547,   119,
     120,   121,   122,   123,   124,  1554,   276,  1556,   278,    -1,
     130,   131,   681,   193,  1563,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,  1788,    -1,    -1,    -1,  1306,    -1,
      -1,  1795,    -1,    -1,    -1,    -1,    -1,  1586,    -1,    -1,
      -1,    -1,    -1,    -1,  1593,  1594,  1595,    -1,    -1,    -1,
    1599,   171,    -1,   139,   140,   141,  1605,    -1,    56,    -1,
      -1,    -1,    -1,    -1,   334,    -1,  1830,    -1,    -1,   189,
      -1,   740,    -1,   511,  1909,  1839,   162,    -1,    -1,   165,
     166,    -1,   168,   169,   170,    -1,  1921,    -1,  1852,   359,
      -1,    -1,    -1,  1124,    -1,    -1,  1931,    -1,   368,  1934,
      -1,    -1,    -1,    -1,    -1,   375,    -1,   193,    -1,    -1,
      -1,   197,   781,    -1,   783,   385,    -1,    -1,    -1,    -1,
    1746,    -1,    -1,    -1,    -1,  1156,   396,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,   811,    -1,  1422,    -1,    -1,    -1,  1912,    -1,
      -1,   421,    -1,  1917,   424,    -1,    -1,  1846,    -1,    -1,
      -1,    -1,  1788,    -1,    -1,    -1,  1715,    -1,    -1,  1795,
      -1,  1449,    -1,  1204,    -1,    -1,    -1,  1866,    -1,    -1,
      67,    68,    -1,    -1,    -1,    -1,  1875,  1736,  1219,  1220,
      -1,  1740,  1741,    -1,    -1,    -1,    -1,  1746,    -1,    -1,
      -1,   471,    -1,    -1,  1830,   874,  1755,    -1,    -1,    -1,
      -1,    -1,    81,  1762,  1763,    -1,    -1,  1766,  1767,    -1,
    1498,   890,   891,    -1,    -1,  1503,  1852,    -1,    -1,  1507,
      -1,  1780,    -1,    -1,   103,    -1,    -1,    -1,    -1,  1788,
      -1,   511,    -1,   681,    -1,    -1,  1795,   134,   135,    -1,
      -1,    -1,    -1,   922,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     139,   140,   141,    -1,    -1,  1306,    -1,    -1,   276,    -1,
     278,  1830,    -1,    -1,    -1,    -1,  1912,    -1,    -1,  1838,
      -1,  1917,    -1,   563,    -1,   565,   165,   166,   568,   168,
     169,   170,   740,  1852,    -1,    -1,    -1,    -1,   195,  1858,
      67,    68,    -1,    -1,    -1,  1593,  1594,  1595,    -1,    -1,
      -1,  1599,    -1,    -1,   193,   194,    -1,  1605,    81,   599,
      83,    84,    -1,    -1,    -1,    -1,   334,    -1,    -1,    -1,
      -1,    -1,    -1,   781,    -1,   783,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,  1024,  1025,    -1,    -1,    -1,
      -1,    -1,    -1,  1912,    -1,    -1,    -1,    -1,  1917,    -1,
      -1,    -1,    -1,   811,    -1,    -1,    -1,   134,   135,    -1,
      -1,   651,   652,    -1,    -1,    -1,   139,   140,   141,    -1,
     660,    -1,    -1,    -1,  1063,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
     163,   681,   165,   166,    -1,   168,   169,   170,  1449,    -1,
      -1,    -1,    -1,   421,    -1,    -1,   424,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   874,    -1,   195,    -1,
     193,    -1,    -1,    -1,   197,    -1,   199,    -1,    67,    68,
      -1,    -1,   890,   891,    -1,  1124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,  1746,    -1,
     740,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    31,    -1,  1156,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    1788,   781,    -1,   783,    -1,   134,   135,  1795,    -1,    -1,
      -1,    81,    -1,    69,    67,    68,    -1,    -1,    -1,    -1,
    1199,    -1,    -1,    -1,    -1,  1204,    -1,    -1,    -1,    -1,
      -1,   811,   812,   103,    -1,    -1,    -1,    -1,    -1,    -1,
    1219,  1220,  1830,    -1,    -1,   825,   826,   827,   828,   829,
     830,   831,    -1,    -1,    -1,   563,    -1,    -1,   838,    -1,
      -1,    -1,    -1,    -1,  1852,    -1,   195,    -1,    -1,   139,
     140,   141,   852,    -1,    -1,    -1,  1024,  1025,    31,    -1,
      -1,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,   159,
      31,    -1,   162,   163,   874,   165,   166,    -1,   168,   169,
     170,    -1,    -1,    -1,    -1,    -1,    59,    -1,   888,    -1,
     890,   891,    -1,    -1,    -1,    -1,    -1,    -1,    59,    -1,
      -1,    -1,    -1,   193,  1912,    -1,    -1,  1306,    81,  1917,
      -1,    -1,   912,   913,    -1,    -1,    -1,    56,    -1,   195,
      81,    -1,   922,   651,   652,    -1,    -1,    -1,   928,    -1,
     103,    -1,   660,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   941,   103,    -1,    -1,    -1,    -1,    -1,    -1,   949,
     111,    -1,   952,    -1,    -1,    -1,  1124,    -1,   119,   120,
     121,   122,   123,   124,    -1,   138,   139,   140,   141,   142,
     970,    -1,    -1,    -1,   974,    -1,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,   159,    -1,  1156,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,   159,   172,
      -1,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
     183,    -1,    -1,  1412,    -1,    -1,    -1,    -1,    -1,    -1,
     193,   194,   183,    -1,  1024,  1025,    -1,    -1,   189,    -1,
      -1,    -1,   193,   194,    -1,    -1,  1204,    -1,  1038,    -1,
      -1,  1041,    81,  1043,    -1,    -1,    -1,    -1,    -1,    -1,
    1449,  1219,  1220,    -1,    -1,    -1,    -1,    -1,  1058,  1059,
    1060,  1061,    -1,  1063,   103,    -1,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,  1093,    -1,  1494,    -1,   825,   826,    -1,
     139,   140,   141,    81,    -1,    -1,    -1,    -1,    -1,    -1,
    1110,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
     159,    81,    -1,   162,  1124,   103,   165,   166,    -1,   168,
     169,   170,    -1,    -1,    -1,    -1,    -1,   276,  1306,   278,
      -1,    -1,    -1,   103,  1144,    59,  1146,    -1,    -1,    -1,
     128,    -1,    -1,    -1,   193,    -1,  1156,    -1,    -1,   198,
      -1,   139,   140,   141,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,  1173,    -1,    -1,  1176,    -1,    -1,   139,
     140,   141,    -1,    -1,   912,    -1,    -1,   165,   166,   103,
     168,   169,   170,    -1,    -1,   334,    -1,    -1,    -1,  1199,
     928,    -1,    -1,    -1,  1204,   165,   166,    -1,   168,   169,
     170,    -1,    -1,   941,    -1,   193,   194,    -1,    -1,  1219,
    1220,    -1,  1222,    -1,   138,   139,   140,   141,   142,    -1,
      -1,    -1,  1232,   193,   194,    -1,  1236,    -1,    -1,  1239,
      -1,  1241,   970,    -1,    -1,   159,    -1,    -1,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,   172,    -1,
      -1,    -1,    -1,    -1,    -1,  1265,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,
     194,  1449,   421,    -1,    -1,   424,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,  1299,
    1300,    -1,    -1,  1303,    -1,    -1,  1306,    -1,    -1,    -1,
    1038,    30,    31,  1041,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1396,    -1,  1398,    69,
      -1,    -1,    -1,    -1,  1404,    -1,  1406,    -1,  1408,    -1,
      -1,  1411,  1412,    -1,    -1,  1415,  1144,  1417,  1146,    -1,
    1420,    -1,    -1,    -1,   563,    -1,   565,    31,    -1,   568,
      -1,  1431,  1432,    -1,    -1,  1435,    -1,    -1,    -1,    -1,
      -1,    -1,  1442,    -1,    -1,  1173,    -1,    -1,  1176,  1449,
      -1,    -1,    -1,    -1,    -1,    59,    -1,    -1,    81,    -1,
     599,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    81,    -1,    -1,
     103,    -1,   201,    -1,    -1,    -1,    -1,    -1,   111,   112,
      -1,    -1,    -1,    -1,  1494,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,  1232,    -1,    -1,    -1,  1236,    81,
    1510,    -1,   651,   652,    67,    68,   139,   140,   141,    -1,
      -1,   660,  1522,  1523,    -1,    -1,    -1,    -1,   198,    -1,
    1530,   103,  1532,    -1,   138,   139,   140,   141,   142,   162,
      -1,    -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,
      -1,    -1,    -1,    -1,  1554,   159,  1556,    -1,   162,   163,
      -1,   165,   166,  1563,   168,   169,   170,   139,   140,   141,
     193,  1299,  1300,    -1,    -1,    -1,    -1,    -1,    -1,   183,
      -1,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,   193,
     194,   163,    -1,   165,   166,    -1,   168,   169,   170,    -1,
      -1,    -1,  1602,  1603,  1604,    -1,    -1,    -1,    -1,  1609,
      -1,  1611,    -1,    10,    11,    12,    -1,  1617,    -1,  1619,
      -1,   193,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    31,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    81,    -1,    -1,    -1,    -1,  1396,    -1,
    1398,    -1,    69,   812,    -1,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,   825,   826,   827,   828,
     829,   830,   831,    -1,    -1,    70,    -1,    -1,    -1,   838,
      -1,    -1,    -1,    78,    79,    80,    81,   125,    -1,    -1,
      -1,    -1,    -1,    -1,  1442,    -1,    91,    -1,    -1,  1719,
     138,   139,   140,   141,   142,    -1,    -1,    81,   103,    83,
      84,    -1,    -1,    -1,    -1,    -1,  1736,    -1,    -1,    -1,
      -1,   159,    -1,    -1,   162,   163,    -1,   165,   166,   103,
     168,   169,   170,    -1,    -1,  1755,    -1,    -1,    -1,    -1,
      -1,  1761,    81,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,  1772,   912,    -1,   193,    81,    -1,  1778,   154,
      -1,    -1,  1782,    -1,   103,   139,   140,   141,    -1,   928,
     165,   166,    -1,   168,   169,   170,    -1,    -1,   103,    -1,
      -1,   198,   941,    -1,  1804,    -1,   111,   112,   183,   163,
     949,   165,   166,    -1,   168,   169,   170,    -1,   193,    -1,
     139,   140,   141,    -1,    -1,    -1,    -1,   226,    -1,    -1,
      -1,   970,    -1,    -1,   139,   140,   141,    -1,    -1,   193,
      -1,    -1,    -1,   197,  1844,   199,   165,   166,    -1,   168,
     169,   170,    -1,    -1,  1854,    -1,    -1,   162,    -1,    -1,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,    -1,
      -1,  1871,    -1,    -1,   193,    -1,    -1,    -1,    -1,    -1,
    1880,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,  1617,
      -1,    -1,  1892,    -1,  1894,    -1,    78,    79,    80,  1038,
      -1,    -1,  1041,    -1,  1043,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,  1913,    -1,  1915,    -1,    -1,    -1,  1058,
    1059,  1060,  1061,    -1,    -1,    -1,    -1,  1066,  1067,  1068,
    1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,  1093,    -1,    -1,    -1,    -1,    -1,
     359,   143,   144,   145,   146,   147,    -1,    -1,    -1,   368,
      -1,  1110,   154,    -1,    -1,    -1,   375,    -1,   160,   161,
      81,    -1,    83,    -1,    85,    -1,   385,    -1,    10,    11,
      12,  1719,   174,    -1,    -1,    -1,    -1,   396,    -1,    -1,
      -1,    -1,   103,    -1,    -1,  1144,   188,  1146,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,  1173,    57,    -1,  1176,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,    69,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,   471,    -1,    69,    -1,  1804,    -1,    -1,    -1,
      -1,    -1,    -1,  1222,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   193,  1232,    -1,    -1,    -1,  1236,    -1,    -1,
    1239,    -1,  1241,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,   511,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,  1265,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,  1880,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1299,  1300,    -1,    69,  1892,    -1,  1894,    -1,    -1,   568,
      -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,  1913,    -1,  1915,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     599,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,  1396,    57,  1398,
      -1,    -1,    -1,    -1,    -1,  1404,    -1,  1406,    -1,  1408,
      69,    -1,  1411,    -1,    81,    -1,  1415,    -1,  1417,    -1,
      -1,  1420,   681,    -1,    10,    11,    12,    27,    28,    -1,
     196,    31,  1431,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,  1442,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,
      -1,   740,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   162,    -1,    -1,   165,   166,
      -1,   168,   169,   170,    -1,    -1,   198,    81,    -1,    -1,
      -1,  1510,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   781,    -1,   783,    -1,   193,    -1,    -1,   103,
      -1,  1530,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   811,   812,    -1,  1554,    -1,  1556,    -1,    -1,
      -1,    -1,    -1,    -1,  1563,   139,   140,   141,   827,   828,
     829,   830,   831,    -1,    -1,    -1,    70,    -1,    -1,   838,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,   162,    83,
      84,   165,   166,   852,   168,   169,   170,    91,    -1,    -1,
      -1,    -1,    -1,  1602,  1603,  1604,    -1,    -1,    -1,   103,
    1609,    -1,   198,    -1,    -1,   874,    -1,    -1,  1617,   193,
      -1,    -1,    -1,    -1,    -1,    -1,   226,    -1,    -1,   888,
     124,   890,   891,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,   913,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,   922,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,
     174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     949,    -1,    -1,   952,   188,    -1,    -1,    -1,    -1,   193,
     194,    -1,    -1,    -1,    -1,   199,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   974,    -1,    -1,    -1,    -1,
    1719,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1736,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,  1755,    57,    -1,   359,
      -1,    -1,  1761,    -1,    -1,  1024,  1025,    -1,   368,    69,
      -1,    -1,    -1,  1772,    -1,   375,    -1,    -1,    -1,  1778,
      -1,    -1,    -1,  1782,    -1,   385,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   396,    -1,    -1,  1058,
    1059,  1060,  1061,    -1,  1063,  1804,    -1,  1066,  1067,  1068,
    1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,  1093,    -1,    -1,    78,    79,    80,
      81,    -1,    83,    84,    -1,  1844,    -1,    -1,    -1,    -1,
      91,  1110,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,  1124,    -1,    -1,    -1,    -1,
      -1,   471,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1880,    -1,   124,    -1,    -1,    -1,    -1,    -1,   130,
      -1,    -1,    -1,  1892,    -1,  1894,    -1,  1156,   139,   140,
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
      -1,   511,    -1,   154,  1913,    -1,  1915,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,    -1,
    1199,    -1,    -1,    -1,    -1,  1204,    -1,   188,    -1,    -1,
      -1,    -1,   193,   194,    -1,    -1,    -1,    -1,   199,    -1,
    1219,  1220,    -1,  1222,    -1,    -1,    -1,    -1,   568,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1239,    -1,  1241,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   599,
      -1,    -1,    -1,    -1,    30,    31,  1265,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,  1303,    -1,    -1,  1306,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   568,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   681,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     599,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     740,    -1,    -1,    -1,    -1,  1404,    -1,  1406,    -1,  1408,
      -1,    -1,  1411,  1412,    -1,    -1,  1415,    -1,  1417,    -1,
      -1,  1420,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   198,  1432,    -1,    -1,  1435,    -1,    -1,    -1,
      -1,   781,    -1,   783,    -1,    -1,    -1,    -1,    -1,    -1,
    1449,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,   811,   812,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,   827,   828,   829,
     830,   831,    -1,    -1,    -1,  1494,    -1,    -1,   838,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1510,    -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,
      -1,    -1,    -1,  1522,  1523,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1532,   874,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
     890,   891,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,   812,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   827,   828,
     829,   830,   922,    -1,    -1,    -1,    -1,    -1,    -1,   838,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1602,  1603,  1604,    -1,    -1,    -1,   949,
    1609,    -1,  1611,    -1,    -1,    10,    11,    12,    -1,    -1,
    1619,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   974,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1024,  1025,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     949,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1058,  1059,
    1060,  1061,    -1,  1063,    -1,    -1,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,  1093,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1761,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1110,    -1,    -1,  1772,    -1,    -1,    -1,    -1,    -1,  1778,
      -1,    -1,    -1,  1782,  1124,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,   198,    -1,    -1,    -1,    -1,    -1,    -1,
    1059,  1060,  1061,    -1,    -1,    69,  1156,  1066,  1067,  1068,
    1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,  1093,  1844,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1854,    -1,    -1,    -1,  1199,
      -1,  1110,    -1,    -1,  1204,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1871,    -1,    -1,    -1,    -1,    -1,    -1,  1219,
    1220,    -1,  1222,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1239,
      -1,  1241,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,  1265,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,  1306,    -1,    -1,    -1,
      -1,    -1,    -1,  1222,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1239,    -1,  1241,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1265,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1404,    -1,  1406,    -1,  1408,    -1,
      -1,  1411,  1412,    -1,    -1,  1415,    -1,  1417,    -1,    -1,
    1420,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   198,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1449,
      -1,    -1,    -1,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,  1494,  1404,    -1,  1406,    -1,  1408,
      -1,    -1,  1411,    -1,    -1,    -1,  1415,    -1,  1417,    -1,
    1510,  1420,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,   198,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,  1510,  1602,  1603,  1604,    -1,    -1,    -1,    -1,  1609,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,  1618,   196,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,
      -1,    -1,   183,  1602,  1603,  1604,    -1,   188,   189,   190,
    1609,    -1,   193,   194,    -1,    -1,    -1,    -1,   199,   200,
      -1,   202,   203,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1761,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1772,   124,    -1,    -1,    -1,    -1,  1778,    -1,
      -1,    -1,  1782,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    -1,  1806,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,     7,    -1,   188,    -1,    -1,
      -1,    13,   193,   194,  1844,    -1,   197,    -1,   199,    -1,
      -1,    -1,  1761,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,  1772,    -1,    -1,    -1,    -1,    -1,  1778,
      -1,    -1,    -1,  1782,    -1,    -1,    -1,    49,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,
      -1,   103,   104,    -1,    -1,  1844,   108,   109,   110,   111,
     112,   113,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,   123,   124,    -1,   126,   127,   128,   129,   130,   131,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,   151,
      -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,   171,
      -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,    -1,
      -1,   183,   184,    -1,   186,    -1,   188,   189,   190,    -1,
      -1,   193,   194,    -1,   196,   197,   198,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    -1,    96,    -1,    98,    -1,   100,
      -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,   114,    -1,   116,   117,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,   127,   128,   129,   130,
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
     151,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
     171,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,
      -1,    -1,   183,   184,    -1,   186,    -1,   188,   189,   190,
      -1,    -1,   193,   194,    -1,   196,   197,   198,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,   114,    -1,   116,   117,   118,   119,
     120,   121,   122,   123,   124,    -1,   126,   127,   128,   129,
     130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,   143,   144,   145,   146,   147,    -1,    -1,
      -1,   151,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,
     160,   161,   162,   163,    -1,   165,   166,    -1,   168,   169,
     170,   171,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,
      -1,    -1,    -1,   183,   184,    -1,   186,    -1,   188,   189,
     190,    -1,    -1,   193,   194,    -1,   196,   197,    -1,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      49,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,    98,
      -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
     109,   110,   111,    -1,   113,   114,    -1,   116,    -1,   118,
     119,   120,   121,   122,   123,   124,    -1,   126,   127,   128,
      -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,    -1,   143,   144,   145,   146,   147,    -1,
      -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,    -1,
     159,   160,   161,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,    -1,
      -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,   188,
     189,   190,    -1,    -1,   193,   194,    -1,   196,   197,   198,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    49,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,
      98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,    -1,   113,   114,    -1,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,   194,    -1,   196,   197,
     198,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,
      -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,    -1,   113,   114,    -1,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,    -1,   143,   144,   145,   146,
     147,    -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,
     177,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,
      -1,   188,   189,   190,    -1,    -1,   193,   194,    -1,   196,
     197,   198,   199,   200,    -1,   202,   203,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,    -1,   108,   109,   110,   111,    -1,   113,   114,    -1,
     116,    -1,   118,   119,   120,   121,   122,   123,   124,    -1,
     126,   127,   128,    -1,   130,   131,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,    -1,   143,   144,   145,
     146,   147,    -1,    -1,    -1,   151,    -1,    -1,   154,    -1,
      -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,   171,    -1,    -1,   174,    -1,
      -1,   177,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,
      -1,    -1,   188,   189,   190,    -1,    -1,   193,   194,    -1,
     196,   197,   198,   199,   200,    -1,   202,   203,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,    94,
      95,    96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,   114,
      -1,   116,    -1,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,   171,    -1,    -1,   174,
      -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,    -1,
      -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,   194,
      -1,   196,   197,    -1,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,
      94,    -1,    96,    -1,    98,    -1,   100,   101,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,
     114,    -1,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,   171,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
     194,    -1,   196,   197,    -1,   199,   200,    -1,   202,   203,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,
     113,   114,    -1,   116,    -1,   118,   119,   120,   121,   122,
     123,   124,    -1,   126,   127,   128,    -1,   130,   131,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,
     143,   144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,
      -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,   171,    -1,
      -1,   174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,
     183,    -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,
     193,   194,    -1,   196,   197,   198,   199,   200,    -1,   202,
     203,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,
      -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
      -1,   113,   114,    -1,   116,    -1,   118,   119,   120,   121,
     122,   123,   124,    -1,   126,   127,   128,    -1,   130,   131,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,   151,
      -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,   171,
      -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,    -1,
      -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,    -1,
      -1,   193,   194,    -1,   196,   197,   198,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    -1,    96,    -1,    98,    99,   100,
      -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,    -1,   113,   114,    -1,   116,    -1,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,   127,   128,    -1,   130,
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
     151,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
     171,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,
      -1,    -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,
      -1,    -1,   193,   194,    -1,   196,   197,    -1,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,    -1,   113,   114,    -1,   116,    -1,   118,   119,
     120,   121,   122,   123,   124,    -1,   126,   127,   128,    -1,
     130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,   143,   144,   145,   146,   147,    -1,    -1,
      -1,   151,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,
     160,   161,   162,   163,    -1,   165,   166,    -1,   168,   169,
     170,   171,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,
      -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,   188,   189,
     190,    -1,    -1,   193,   194,    -1,   196,   197,   198,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      49,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    -1,    96,    97,    98,
      -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
     109,   110,   111,    -1,   113,   114,    -1,   116,    -1,   118,
     119,   120,   121,   122,   123,   124,    -1,   126,   127,   128,
      -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,    -1,   143,   144,   145,   146,   147,    -1,
      -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,    -1,
     159,   160,   161,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,    -1,
      -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,   188,
     189,   190,    -1,    -1,   193,   194,    -1,   196,   197,    -1,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    49,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,
      98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,    -1,   113,   114,    -1,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,   194,    -1,   196,   197,
     198,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,
      -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,    -1,   113,   114,    -1,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,    -1,   143,   144,   145,   146,
     147,    -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,
     177,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,
      -1,   188,   189,   190,    -1,    -1,   193,   194,    -1,   196,
     197,   198,   199,   200,    -1,   202,   203,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,    -1,   108,   109,   110,   111,    -1,   113,   114,    -1,
     116,    -1,   118,   119,   120,   121,   122,   123,   124,    -1,
     126,   127,   128,    -1,   130,   131,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,    -1,   143,   144,   145,
     146,   147,    -1,    -1,    -1,   151,    -1,    -1,   154,    -1,
      -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,   171,    -1,    -1,   174,    -1,
      -1,   177,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,
      -1,    -1,   188,   189,   190,    -1,    -1,   193,   194,    -1,
     196,   197,   198,   199,   200,    -1,   202,   203,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,    94,
      -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,   114,
      -1,   116,    -1,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,   171,    -1,    -1,   174,
      -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,    -1,
      -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,   194,
      -1,   196,   197,   198,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,
      94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,
     114,    -1,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,   171,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
     194,    -1,   196,   197,    -1,   199,   200,    -1,   202,   203,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,
     113,   114,    -1,   116,    -1,   118,   119,   120,   121,   122,
     123,   124,    -1,   126,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,
     143,   144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,
      -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,
      -1,   174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,
     183,    -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,
     193,   194,    -1,   196,   197,    -1,   199,   200,    -1,   202,
     203,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,   151,
      -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,    -1,
      -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,    -1,
      -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,    -1,
      -1,   193,   194,    -1,   196,   197,    -1,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
     151,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,
      -1,    -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,
      -1,    -1,   193,   194,    -1,   196,   197,    -1,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,    -1,   113,   114,    -1,   116,    -1,   118,   119,
     120,   121,   122,   123,   124,    -1,   126,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,   143,   144,   145,   146,   147,    -1,    -1,
      -1,   151,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,
     160,   161,   162,   163,    -1,   165,   166,    -1,   168,   169,
     170,    -1,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,
      -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,   188,   189,
     190,    -1,    -1,   193,   194,    -1,   196,   197,    -1,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,
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
     139,   140,   141,    -1,   143,   144,   145,   146,   147,    -1,
      -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,    -1,
     159,   160,   161,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,    -1,
      -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,   188,
     189,   190,    -1,    -1,   193,   194,    -1,   196,   197,    -1,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,   194,    -1,   196,   197,
      -1,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,    -1,   143,   144,   145,   146,
     147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,
     177,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,
      -1,   188,   189,   190,    -1,    -1,   193,   194,    -1,   196,
      -1,    -1,   199,   200,    -1,   202,   203,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    31,    -1,    13,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,    -1,   143,   144,   145,
     146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,
      -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,    -1,   172,    -1,   174,    -1,
      -1,   177,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,
      -1,    -1,   188,   189,   190,    -1,    -1,   193,   194,    -1,
      -1,    -1,    -1,   199,   200,    -1,   202,   203,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,   174,
      -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,    -1,
      -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,   194,
      -1,    -1,   197,    -1,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,
     124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,   172,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
     194,    -1,    -1,    -1,    -1,   199,   200,    -1,   202,   203,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,
     143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,
      -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,
      -1,   174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,
     183,    -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,
     193,   194,    10,    11,    12,    -1,   199,   200,    -1,   202,
     203,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    69,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,   108,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,    -1,
      -1,    -1,   174,    -1,    -1,   177,    -1,    -1,   196,    -1,
      -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,    -1,
      -1,   193,   194,    -1,    -1,    -1,    -1,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,
      -1,    -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,
      -1,    -1,   193,   194,    10,    11,    12,    -1,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    69,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,   143,   144,   145,   146,   147,    -1,    -1,
      -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,
     160,   161,   162,   163,    -1,   165,   166,    -1,   168,   169,
     170,    -1,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,
     196,    -1,    -1,   183,    -1,    -1,    -1,    -1,   188,   189,
     190,    -1,    -1,   193,   194,    -1,   196,    11,    12,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    69,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,    -1,   143,   144,   145,   146,   147,    -1,
      -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,
     159,   160,   161,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,    -1,
      -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,   188,
     189,   190,    -1,    -1,   193,   194,    -1,   196,    -1,    -1,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,   194,    10,    11,    12,
      -1,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    69,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,    -1,   143,   144,   145,   146,
     147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,
     177,    -1,    -1,   196,    -1,    -1,   183,    -1,    -1,    -1,
      -1,   188,   189,   190,    -1,    -1,   193,   194,   195,    -1,
      -1,    -1,   199,   200,    -1,   202,   203,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,    -1,   143,   144,   145,
     146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,
      -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,    -1,    -1,    -1,   174,    -1,
      -1,   177,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,
      -1,    -1,   188,   189,   190,    -1,    -1,   193,   194,    -1,
      -1,    -1,    -1,   199,   200,    -1,   202,   203,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,   174,
      -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,    -1,
      -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,   194,
      -1,    -1,    -1,    -1,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,
     124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
     194,    -1,    -1,    -1,    -1,   199,   200,    -1,   202,   203,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,
     143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,
      -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,
      -1,   174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,
     183,    -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,
     193,   194,    -1,    -1,    -1,    -1,   199,   200,    -1,   202,
     203,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,    -1,
      -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,    -1,
      -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,    -1,
      -1,   193,   194,    -1,    -1,    -1,    -1,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,
      -1,    -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,
      -1,    -1,   193,   194,    -1,    -1,    -1,    -1,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,   143,   144,   145,   146,   147,    -1,    -1,
      -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,
     160,   161,   162,   163,    -1,   165,   166,    -1,   168,   169,
     170,    -1,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,
      -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,   188,   189,
     190,    -1,    -1,   193,   194,    10,    11,    12,    -1,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    69,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,    -1,   143,   144,   145,   146,   147,    -1,
      -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,
     159,   160,   161,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,    -1,
      -1,   196,    -1,    -1,   183,    -1,    -1,    -1,    -1,   188,
     189,   190,    -1,    -1,   193,   194,    10,    11,    12,    -1,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    28,    13,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    69,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,   102,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,     3,     4,   174,     6,     7,   177,
      -1,    10,    11,    12,    13,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,   194,    -1,    27,    -1,
      29,   199,   200,    -1,   202,   203,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
      -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,    -1,    -1,    10,    11,    12,    13,
     159,    -1,    -1,    -1,    -1,    -1,   165,   166,    -1,   168,
     169,   170,   171,    27,   173,    29,    -1,   176,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,   197,    -1,
     199,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,    -1,   130,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,    -1,   159,    10,    11,    12,    13,
      -1,   165,   166,    -1,   168,   169,   170,   171,    -1,   173,
      -1,    -1,   176,    27,    -1,    29,    -1,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,
      -1,    -1,    -1,   197,    -1,   199,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    57,    -1,    59,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,    -1,    -1,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,   171,    -1,   173,
      -1,    -1,   176,     3,     4,    -1,     6,     7,    -1,   183,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   193,
     194,    -1,    -1,    -1,   198,    -1,    -1,    27,    -1,    29,
      31,    31,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    57,    -1,    59,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,    -1,
      -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,
      -1,    -1,   162,   163,    -1,   165,   166,    -1,   168,   169,
     170,   171,    -1,   173,    -1,    -1,   176,     3,     4,    -1,
       6,     7,    -1,   183,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,   193,   194,    -1,    -1,    -1,   198,    -1,
      -1,    27,    -1,    29,    -1,    31,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    57,    -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,    -1,   130,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   159,    -1,    -1,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,   171,    -1,   173,    -1,    -1,
     176,     3,     4,    -1,     6,     7,    -1,   183,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,   193,   194,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,    -1,    -1,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,   171,
      -1,   173,    -1,    -1,   176,    -1,     3,     4,    -1,     6,
       7,   183,   184,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,   193,   194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    29,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   159,    -1,    -1,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,   171,    -1,   173,    -1,    -1,   176,
       3,     4,     5,     6,     7,    -1,   183,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,   193,   194,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    57,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,
     143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   159,   160,   161,    -1,
      -1,    -1,   165,   166,    -1,   168,   169,   170,   171,    -1,
     173,   174,    -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,
     183,   184,    -1,   186,    -1,   188,   189,    -1,     3,     4,
     193,     6,     7,    11,    12,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    31,    29,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,    -1,   130,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,
      10,    11,    12,    13,   159,    -1,    -1,    -1,    -1,    -1,
     165,   166,    -1,   168,   169,   170,   171,    27,   173,    29,
      -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,    -1,
     130,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,    -1,    10,    11,    12,    13,   159,
      -1,    -1,    -1,    -1,    -1,   165,   166,    -1,   168,   169,
     170,   171,    27,   173,    29,    -1,   176,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,    -1,   130,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,
      10,    11,    12,    13,   159,    -1,    -1,    -1,    -1,    -1,
     165,   166,    -1,   168,   169,   170,   171,    27,   173,    29,
      -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,    -1,
      -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,
      -1,    -1,    -1,    -1,    -1,   165,   166,    -1,   168,   169,
     170,   171,    -1,   173,    -1,    -1,   176,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   193,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    32,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
     195,    50,    51,   174,    -1,    -1,    -1,    56,    -1,    58,
      -1,    -1,   183,    -1,    -1,    -1,    -1,   188,    -1,    -1,
      -1,    70,   193,   194,    -1,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    31,    -1,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,    69,   143,   144,   145,   146,   147,    -1,
      -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,
     159,   160,   161,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,    70,    -1,    -1,   174,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,   183,    83,    84,    -1,    -1,   188,
      -1,    -1,    -1,    91,   193,   194,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    70,    -1,    72,   174,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
     188,    -1,    -1,    -1,    91,   193,   194,    -1,    -1,    -1,
      -1,   199,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,    -1,   143,   144,   145,   146,
     147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,    70,    -1,    -1,   174,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,   188,    -1,    -1,    -1,    91,   193,   194,    -1,    -1,
      -1,    -1,   199,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,    -1,   143,   144,   145,
     146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,
      -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,    70,    -1,    -1,   174,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,   188,    -1,    -1,    -1,    91,   193,   194,    -1,
      -1,    -1,    -1,   199,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,   174,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,   193,   194,
      -1,    -1,    30,    31,   199,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   136,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   136,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,   136,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     136,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   136,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,   136,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
     136,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   136,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,   136,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,   136,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    69,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,    -1,
     143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,
      -1,   154,    -1,    -1,   136,    -1,   159,   160,   161,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,
      -1,   174,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    -1,   188,    91,    -1,    -1,    -1,
     193,   194,    -1,    -1,    -1,    -1,   199,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,   174,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,   193,   194,
      -1,    -1,    -1,    -1,   199,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    32,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    12,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   205,   206,     0,   207,     3,     4,     5,     6,     7,
      13,    27,    28,    29,    49,    50,    51,    56,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    70,
      71,    72,    73,    74,    78,    79,    80,    81,    82,    83,
      84,    86,    87,    91,    92,    93,    94,    96,    98,   100,
     103,   104,   108,   109,   110,   111,   112,   113,   114,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   126,   127,
     128,   129,   130,   131,   137,   138,   139,   140,   141,   143,
     144,   145,   146,   147,   151,   154,   159,   160,   161,   162,
     163,   165,   166,   168,   169,   170,   171,   174,   177,   183,
     184,   186,   188,   189,   190,   193,   194,   196,   197,   199,
     200,   202,   203,   208,   211,   221,   222,   223,   224,   225,
     228,   244,   245,   249,   252,   259,   265,   325,   326,   334,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     349,   352,   364,   365,   372,   375,   378,   384,   386,   387,
     389,   399,   400,   401,   403,   408,   412,   432,   440,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   467,   469,   471,   122,   123,   124,   137,   159,   169,
     194,   211,   244,   325,   346,   444,   346,   194,   346,   346,
     346,   108,   346,   346,   346,   430,   431,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,    81,
      83,    91,   124,   139,   140,   141,   154,   194,   222,   365,
     400,   403,   408,   444,   447,   444,    38,   346,   458,   459,
     346,   124,   130,   194,   222,   257,   400,   401,   402,   404,
     408,   441,   442,   443,   451,   455,   456,   194,   335,   405,
     194,   335,   356,   336,   346,   230,   335,   194,   194,   194,
     335,   196,   346,   211,   196,   346,     3,     4,     6,     7,
      10,    11,    12,    13,    27,    29,    31,    57,    59,    71,
      72,    73,    74,    75,    76,    77,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     128,   130,   131,   132,   133,   137,   138,   142,   159,   163,
     171,   173,   176,   183,   194,   211,   212,   213,   224,   472,
     492,   493,   496,   196,   341,   343,   346,   197,   237,   346,
     111,   112,   162,   214,   215,   216,   217,   221,    83,   199,
     291,   292,   123,   130,   122,   130,    83,   293,   194,   194,
     194,   194,   211,   263,   475,   194,   194,    70,    70,    70,
     336,    83,    90,   155,   156,   157,   464,   465,   162,   197,
     221,   221,   211,   264,   475,   163,   194,   475,   475,    83,
     190,   197,   357,    27,   334,   338,   346,   347,   444,   448,
     226,   197,   453,    90,   406,   464,    90,   464,   464,    32,
     162,   179,   476,   194,     9,   196,    38,   243,   163,   262,
     475,   124,   189,   244,   326,   196,   196,   196,   196,   196,
     196,   196,   196,    10,    11,    12,    30,    31,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    57,    69,   196,    70,    70,   197,   158,   131,   169,
     171,   184,   186,   265,   324,   325,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    67,
      68,   134,   135,   434,    70,   197,   439,   194,   194,    70,
     197,   194,   243,   244,    14,   346,   196,   136,    48,   211,
     429,    90,   334,   347,   158,   444,   136,   201,     9,   414,
     258,   334,   347,   444,   476,   158,   194,   407,   434,   439,
     195,   346,    32,   228,     8,   358,     9,   196,   228,   229,
     336,   337,   346,   211,   277,   232,   196,   196,   196,   138,
     142,   496,   496,   179,   495,   194,   111,   496,    14,   158,
     138,   142,   159,   211,   213,   196,   196,   196,   238,   115,
     176,   196,   214,   216,   214,   216,   221,   197,     9,   415,
     196,   102,   162,   197,   444,     9,   196,   130,   130,    14,
       9,   196,   444,   468,   336,   334,   347,   444,   447,   448,
     195,   179,   255,   137,   444,   457,   458,   346,   366,   367,
     336,   381,   381,   196,    70,   434,   155,   465,    82,   346,
     444,    90,   155,   465,   221,   210,   196,   197,   250,   260,
     390,   392,    91,   194,   359,   360,   362,   403,   450,   452,
     469,    14,   102,   470,   353,   354,   355,   287,   288,   432,
     433,   195,   195,   195,   195,   195,   198,   227,   228,   245,
     252,   259,   432,   346,   200,   202,   203,   211,   477,   478,
     496,    38,   172,   289,   290,   346,   472,   194,   475,   253,
     243,   346,   346,   346,   346,    32,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     404,   346,   346,   454,   454,   346,   460,   461,   130,   197,
     212,   213,   453,   263,   211,   264,   475,   475,   262,   244,
      38,   338,   341,   343,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   163,   197,   211,
     435,   436,   437,   438,   453,   454,   346,   289,   289,   454,
     346,   457,   243,   195,   346,   194,   428,     9,   414,   195,
     195,    38,   346,    38,   346,   407,   195,   195,   195,   451,
     452,   453,   289,   197,   211,   435,   436,   453,   195,   226,
     281,   197,   343,   346,   346,    94,    32,   228,   275,   196,
      28,   102,    14,     9,   195,    32,   197,   278,   496,    31,
      91,   224,   489,   490,   491,   194,     9,    50,    51,    56,
      58,    70,   138,   139,   140,   141,   163,   183,   194,   222,
     224,   373,   376,   379,   385,   400,   408,   409,   411,   211,
     494,   226,   194,   236,   197,   196,   197,   196,   102,   162,
     111,   112,   162,   217,   218,   219,   220,   221,   217,   211,
     346,   292,   409,    83,     9,   195,   195,   195,   195,   195,
     195,   195,   196,    50,    51,   485,   487,   488,   132,   268,
     194,     9,   195,   195,   136,   201,     9,   414,     9,   414,
     201,   201,    83,    85,   211,   466,   211,    70,   198,   198,
     207,   209,    32,   133,   267,   178,    54,   163,   178,   394,
     347,   136,     9,   414,   195,   158,   496,   496,    14,   358,
     287,   226,   191,     9,   415,   496,   497,   434,   439,   434,
     198,     9,   414,   180,   444,   346,   195,     9,   415,    14,
     350,   246,   132,   266,   194,   475,   346,    32,   201,   201,
     136,   198,     9,   414,   346,   476,   194,   256,   251,   261,
      14,   470,   254,   243,    72,   444,   346,   476,   201,   198,
     195,   195,   201,   198,   195,    50,    51,    70,    78,    79,
      80,    91,   138,   139,   140,   141,   154,   183,   211,   374,
     377,   380,   417,   419,   420,   424,   427,   211,   444,   444,
     136,   266,   434,   439,   195,   346,   282,    75,    76,   283,
     226,   335,   226,   337,   102,    38,   137,   272,   444,   409,
     211,    32,   228,   276,   196,   279,   196,   279,     9,   414,
      91,   136,   158,     9,   414,   195,   172,   477,   478,   479,
     477,   409,   409,   409,   409,   409,   413,   416,   194,    70,
      70,    70,   158,   194,   409,   158,   197,    10,    11,    12,
      31,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    69,   158,   476,   198,   400,   197,   240,
     216,   216,   211,   217,   217,   221,     9,   415,   198,   198,
      14,   444,   196,   180,     9,   414,   211,   269,   400,   197,
     457,   137,   444,    14,    38,   346,   346,   201,   346,   198,
     207,   496,   269,   197,   393,    14,   195,   346,   359,   453,
     196,   496,   191,   198,    32,   483,   433,    38,    83,   172,
     435,   436,   438,   435,   436,   496,    38,   172,   346,   409,
     287,   194,   400,   267,   351,   247,   346,   346,   346,   198,
     194,   289,   268,    32,   267,   496,    14,   266,   475,   404,
     198,   194,    14,    78,    79,    80,   211,   418,   418,   420,
     422,   423,    52,   194,    70,    70,    70,    90,   155,   194,
       9,   414,   195,   428,    38,   346,   267,   198,    75,    76,
     284,   335,   228,   198,   196,    95,   196,   272,   444,   194,
     136,   271,    14,   226,   279,   105,   106,   107,   279,   198,
     496,   180,   136,   496,   211,   489,     9,   195,   414,   136,
     201,     9,   414,   413,   368,   369,   409,   382,   409,   410,
     382,   130,   212,   359,   361,   363,   195,   130,   212,   409,
     462,   463,   409,   409,   409,    32,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     494,    83,   241,   198,   198,   220,   196,   409,   488,   102,
     103,   484,   486,     9,   297,   195,   194,   338,   343,   346,
     444,   136,   201,   198,   470,   297,   164,   177,   197,   389,
     396,   164,   197,   395,   136,   196,   483,   496,   358,   497,
      83,   172,    14,    83,   476,   444,   346,   195,   287,   197,
     287,   194,   136,   194,   289,   195,   197,   496,   197,   196,
     496,   267,   248,   407,   289,   136,   201,     9,   414,   419,
     422,   370,   371,   420,   383,   420,   421,   383,   155,   359,
     425,   426,   420,   444,   197,   335,    32,    77,   228,   196,
     337,   271,   457,   272,   195,   409,   101,   105,   196,   346,
      32,   196,   280,   198,   180,   496,   136,   172,    32,   195,
     409,   409,   195,   201,     9,   414,   136,   201,     9,   414,
     201,   136,     9,   414,   195,   136,   198,     9,   414,   409,
      32,   195,   226,   196,   196,   211,   496,   496,   484,   400,
       4,   112,   117,   123,   125,   165,   166,   168,   198,   298,
     323,   324,   325,   330,   331,   332,   333,   432,   457,    38,
     346,   198,   197,   198,    54,   346,   346,   346,   358,    38,
      83,   172,    14,    83,   346,   194,   483,   195,   297,   195,
     287,   346,   289,   195,   297,   470,   297,   196,   197,   194,
     195,   420,   420,   195,   201,     9,   414,   136,   201,     9,
     414,   201,   136,   195,     9,   414,   297,    32,   226,   196,
     195,   195,   195,   233,   196,   196,   280,   226,   496,   496,
     136,   409,   409,   409,   409,   359,   409,   409,   409,   197,
     198,   486,   132,   133,   184,   212,   473,   496,   270,   400,
     112,   333,    31,   125,   138,   142,   163,   169,   307,   308,
     309,   310,   400,   167,   315,   316,   128,   194,   211,   317,
     318,   299,   244,   496,     9,   196,     9,   196,   196,   470,
     324,   195,   444,   294,   163,   391,   198,   198,    83,   172,
      14,    83,   346,   289,   117,   348,   483,   198,   483,   195,
     195,   198,   197,   198,   297,   287,   136,   420,   420,   420,
     420,   359,   198,   226,   231,   234,    32,   228,   274,   226,
     195,   409,   136,   136,   136,   226,   400,   400,   475,    14,
     212,     9,   196,   197,   473,   470,   310,   179,   197,     9,
     196,     3,     4,     5,     6,     7,    10,    11,    12,    13,
      27,    28,    29,    57,    71,    72,    73,    74,    75,    76,
      77,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   137,   138,   143,   144,   145,   146,   147,   159,
     160,   161,   171,   173,   174,   176,   183,   184,   186,   188,
     189,   211,   397,   398,     9,   196,   163,   167,   211,   318,
     319,   320,   196,    83,   329,   243,   300,   473,   473,    14,
     244,   198,   295,   296,   473,    14,    83,   346,   195,   194,
     483,   196,   197,   321,   348,   483,   294,   198,   195,   420,
     136,   136,    32,   228,   273,   274,   226,   409,   409,   409,
     198,   196,   196,   409,   400,   303,   496,   311,   312,   408,
     308,    14,    32,    51,   313,   316,     9,    36,   195,    31,
      50,    53,    14,     9,   196,   213,   474,   329,    14,   496,
     243,   196,    14,   346,    38,    83,   388,   197,   226,   483,
     321,   198,   483,   420,   420,   226,    99,   239,   198,   211,
     224,   304,   305,   306,     9,   414,     9,   414,   198,   409,
     398,   398,    59,   314,   319,   319,    31,    50,    53,   409,
      83,   179,   194,   196,   409,   475,   409,    83,     9,   415,
     226,   198,   197,   321,    97,   196,   115,   235,   158,   102,
     496,   180,   408,   170,    14,   485,   301,   194,    38,    83,
     195,   198,   226,   196,   194,   176,   242,   211,   324,   325,
     180,   409,   180,   285,   286,   433,   302,    83,   198,   400,
     240,   173,   211,   196,   195,     9,   415,   119,   120,   121,
     327,   328,   285,    83,   270,   196,   483,   433,   497,   195,
     195,   196,   193,   480,   327,    38,    83,   172,   483,   197,
     481,   482,   496,   196,   197,   322,   497,    83,   172,    14,
      83,   480,   226,     9,   415,    14,   484,   226,    38,    83,
     172,    14,    83,   346,   322,   198,   482,   496,   198,    83,
     172,    14,    83,   346,    14,    83,   346,   346
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (&yylloc, _p, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).line0   = YYRHSLOC (Rhs, 1).line0;	\
	  (Current).char0 = YYRHSLOC (Rhs, 1).char0;	\
	  (Current).line1    = YYRHSLOC (Rhs, N).line1;		\
	  (Current).char1  = YYRHSLOC (Rhs, N).char1;	\
	}								\
      else								\
	{								\
	  (Current).line0   = (Current).line1   =		\
	    YYRHSLOC (Rhs, 0).line1;				\
	  (Current).char0 = (Current).char1 =		\
	    YYRHSLOC (Rhs, 0).char1;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).line0, (Loc).char0,	\
	      (Loc).line1,  (Loc).char1)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, &yylloc, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, &yylloc, _p)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location, _p); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, HPHP::HPHP_PARSER_NS::Parser *_p)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, _p)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
    HPHP::HPHP_PARSER_NS::Parser *_p;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
  YYUSE (_p);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, HPHP::HPHP_PARSER_NS::Parser *_p)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp, _p)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
    HPHP::HPHP_PARSER_NS::Parser *_p;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, _p);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, HPHP::HPHP_PARSER_NS::Parser *_p)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule, _p)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
    HPHP::HPHP_PARSER_NS::Parser *_p;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       , _p);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule, _p); \
} while (YYID (0))

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
#ifndef	YYINITDEPTH
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
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

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, HPHP::HPHP_PARSER_NS::Parser *_p)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp, _p)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
    HPHP::HPHP_PARSER_NS::Parser *_p;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (_p);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (HPHP::HPHP_PARSER_NS::Parser *_p);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */





/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (HPHP::HPHP_PARSER_NS::Parser *_p)
#else
int
yyparse (_p)
    HPHP::HPHP_PARSER_NS::Parser *_p;
#endif
#endif
{
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Location data for the lookahead symbol.  */
YYLTYPE yylloc;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.
       `yyls': related to locations.

       Refer to the stacks thru separate pointers, to allow yyoverflow
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
    YYLTYPE yyerror_range[2];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
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

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;

#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.line0   = yylloc.line1   = 1;
  yylloc.char0 = yylloc.char1 = 1;
#endif

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
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
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
      if (yyn == 0 || yyn == YYTABLE_NINF)
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
  *++yyvsp = yylval;
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
     `$$ = $1'.

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

/* Line 1455 of yacc.c  */
#line 749 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 752 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 759 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 763 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 764 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 765 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 766 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 767 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 769 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 772 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 774 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 775 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 776 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 777 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 778 "hphp.y"
    { _p->onUse((yyvsp[(2) - (3)]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 781 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 784 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 787 "hphp.y"
    { _p->onGroupUse((yyvsp[(2) - (6)]).text(), (yyvsp[(4) - (6)]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 791 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 795 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 798 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 803 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 892 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 894 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 899 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 900 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 906 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 910 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 911 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 913 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 915 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 920 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 921 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(1) - (1)]),
                                           &Parser::useClass);;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useFunction);;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useConst);;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 940 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 942 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 945 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 947 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 948 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 953 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 960 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 968 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 981 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 982 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 983 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 984 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 987 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 991 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 997 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 999 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1006 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1010 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1015 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1017 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1020 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1021 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1022 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1023 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1024 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1025 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1026 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1027 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1028 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1029 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1030 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1031 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1032 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1033 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1034 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1038 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1040 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1045 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1047 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1051 "hphp.y"
    { _p->onDeclare((yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                         (yyval) = (yyvsp[(3) - (5)]);
                                         (yyval) = T_DECLARE;;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1060 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1061 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1064 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1065 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1066 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1067 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1071 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1072 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1073 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1074 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1075 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1076 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1077 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1078 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1079 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1080 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1081 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1082 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1090 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1091 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1100 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1101 "hphp.y"
    { (yyval).reset();;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1105 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1107 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1113 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1114 "hphp.y"
    { (yyval).reset();;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1118 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1119 "hphp.y"
    { (yyval).reset();;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1123 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1129 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1135 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1142 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1148 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1155 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1161 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1169 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1173 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1177 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1181 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1187 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1190 "hphp.y"
    { Token stmts;
                                         if (_p->peekClass()) {
                                           xhp_collect_attributes(_p,stmts,(yyvsp[(7) - (8)]));
                                         } else {
                                           stmts = (yyvsp[(7) - (8)]);
                                         }
                                         _p->onClass((yyval),(yyvsp[(1) - (8)]).num(),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),
                                                     stmts,0,nullptr);
                                         if (_p->peekClass()) {
                                           _p->xhpResetAttributes();
                                         }
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1205 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1208 "hphp.y"
    { Token stmts;
                                         if (_p->peekClass()) {
                                           xhp_collect_attributes(_p,stmts,(yyvsp[(8) - (9)]));
                                         } else {
                                           stmts = (yyvsp[(8) - (9)]);
                                         }
                                         _p->onClass((yyval),(yyvsp[(2) - (9)]).num(),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),
                                                     stmts,&(yyvsp[(1) - (9)]),nullptr);
                                         if (_p->peekClass()) {
                                           _p->xhpResetAttributes();
                                         }
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1222 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1225 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1230 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1233 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1239 "hphp.y"
    { _p->onClassExpressionStart(); ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1242 "hphp.y"
    { _p->onClassExpression((yyval), (yyvsp[(3) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(5) - (8)]), (yyvsp[(7) - (8)])); ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1249 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1269 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1276 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1279 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1280 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1281 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1284 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1285 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1289 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1290 "hphp.y"
    { (yyval).reset();;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1293 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1294 "hphp.y"
    { (yyval).reset();;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1297 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1298 "hphp.y"
    { (yyval).reset();;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1301 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1303 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1308 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1312 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1313 "hphp.y"
    { (yyval).reset();;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1317 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1318 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1324 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1327 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1332 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1334 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1337 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1339 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (4)]));;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1343 "hphp.y"
    {_p->onDeclareList((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1345 "hphp.y"
    {_p->onDeclareList((yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                           (yyval) = (yyvsp[(1) - (5)]);;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1350 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1351 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1352 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1353 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1358 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1360 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1361 "hphp.y"
    { (yyval).reset();;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1364 "hphp.y"
    { (yyval).reset();;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1365 "hphp.y"
    { (yyval).reset();;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1370 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1371 "hphp.y"
    { (yyval).reset();;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1376 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1377 "hphp.y"
    { (yyval).reset();;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1380 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1381 "hphp.y"
    { (yyval).reset();;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1385 "hphp.y"
    { (yyval).reset();;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1393 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1399 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1405 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1409 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1413 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1418 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1423 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1426 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1432 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1436 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1441 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1446 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1451 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1468 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1476 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1481 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1486 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1490 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1493 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1497 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1501 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1504 "hphp.y"
    { (yyval).reset();;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1512 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1533 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1538 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1545 "hphp.y"
    { (yyval).reset();;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { (yyval).reset();;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1594 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { (yyval).reset();;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1620 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (11)]),(yyvsp[(9) - (11)]),(yyvsp[(3) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(7) - (11)]),(yyvsp[(11) - (11)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1628 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (12)]),(yyvsp[(10) - (12)]),(yyvsp[(4) - (12)]),(yyvsp[(5) - (12)]),(yyvsp[(8) - (12)]),(yyvsp[(12) - (12)]),&(yyvsp[(1) - (12)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1642 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1644 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1649 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1655 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1657 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1668 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1671 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1684 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1694 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1696 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1723 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1730 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1751 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyval).reset();;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval).reset();;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { (yyval).reset();;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { (yyval).reset();;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { (yyval).reset();;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1867 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1878 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { (yyval).reset();;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1888 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1890 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { _p->onYieldFrom((yyval),&(yyvsp[(2) - (2)]));;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PIPE);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1975 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1982 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1985 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1987 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1989 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1991 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1992 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { _p->onNullCoalesce((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1994 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1996 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1997 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1998 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2000 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2003 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2004 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2005 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2006 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2009 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2011 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2012 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2020 "hphp.y"
    { (yyval).reset();;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2025 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2031 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[(2) - (12)]),(yyvsp[(5) - (12)]),(yyvsp[(8) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(7) - (12)]),&(yyvsp[(9) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2039 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2045 "hphp.y"
    { _p->finishStatement((yyvsp[(12) - (13)]), (yyvsp[(12) - (13)])); (yyvsp[(12) - (13)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[(1) - (13)]),
                                           (yyvsp[(3) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(12) - (13)]),(yyvsp[(8) - (13)]),&(yyvsp[(10) - (13)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2055 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2063 "hphp.y"
    { Token v; Token w; Token x;
                                         (yyvsp[(1) - (4)]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[(1) - (4)]), nullptr, (yyvsp[(1) - (4)]));
                                         _p->finishStatement((yyvsp[(4) - (4)]), (yyvsp[(4) - (4)])); (yyvsp[(4) - (4)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[(1) - (4)]),
                                                            v,(yyvsp[(2) - (4)]),w,(yyvsp[(4) - (4)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2073 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { Token u; Token v;
                                         (yyvsp[(1) - (7)]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[(1) - (7)]), nullptr, (yyvsp[(1) - (7)]));
                                         _p->finishStatement((yyvsp[(7) - (7)]), (yyvsp[(7) - (7)])); (yyvsp[(7) - (7)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[(1) - (7)]),
                                                            u,(yyvsp[(4) - (7)]),v,(yyvsp[(7) - (7)]),(yyvsp[(6) - (7)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2097 "hphp.y"
    { Token u; Token v; Token w; Token x;
                                         Token y;
                                         (yyvsp[(1) - (5)]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[(1) - (5)]), nullptr, (yyvsp[(1) - (5)]));
                                         _p->finishStatement((yyvsp[(4) - (5)]), (yyvsp[(4) - (5)])); (yyvsp[(4) - (5)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[(1) - (5)]),
                                                            u,v,w,(yyvsp[(4) - (5)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);
                                         _p->onCall((yyval),1,(yyval),y,NULL);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2116 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2141 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2148 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2160 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2182 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2190 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2191 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { (yyval).reset();;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval).reset();;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { (yyval).reset();;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { _p->onKeyset((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { _p->onKeyset((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { _p->onKeyset((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval).reset();;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval).reset();;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval).reset();;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { Token t1; _p->onArray(t1,(yyvsp[(1) - (2)]));
                                         Token t2; _p->onArray(t2,(yyvsp[(2) - (2)]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[(1) - (2)]),NULL,t1,0,0);
                                         _p->onCallParam((yyval), &(yyvsp[(1) - (2)]),t2,0,0);
                                         _p->onCallParam((yyvsp[(1) - (2)]), &(yyvsp[(1) - (2)]),file,0,0);
                                         _p->onCallParam((yyvsp[(1) - (2)]), &(yyvsp[(1) - (2)]),line,0,0);
                                         (yyval).setText("");;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray((yyvsp[(4) - (6)]),(yyvsp[(1) - (6)]));
                                         _p->onArray((yyvsp[(5) - (6)]),(yyvsp[(3) - (6)]));
                                         _p->onCallParam((yyvsp[(2) - (6)]),NULL,(yyvsp[(4) - (6)]),0,0);
                                         _p->onCallParam((yyval), &(yyvsp[(2) - (6)]),(yyvsp[(5) - (6)]),0,0);
                                         _p->onCallParam((yyvsp[(2) - (6)]), &(yyvsp[(2) - (6)]),file,0,0);
                                         _p->onCallParam((yyvsp[(2) - (6)]), &(yyvsp[(2) - (6)]),line,0,0);
                                         (yyval).setText((yyvsp[(6) - (6)]).text());;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval).reset();;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { (yyval).reset();;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2485 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2503 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2522 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { (yyval).reset();;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { (yyval).reset();;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { (yyval).reset();;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { (yyval).reset();;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2565 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2569 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2577 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2588 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2599 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2603 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2608 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2610 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2613 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2615 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2635 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2639 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2648 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2654 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2663 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2669 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2672 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2680 "hphp.y"
    { (yyval).reset();;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2684 "hphp.y"
    { (yyval).reset();;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2685 "hphp.y"
    { (yyval).reset();;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2689 "hphp.y"
    { (yyval).reset();;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2700 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2705 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2709 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2715 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2716 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2717 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2721 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2724 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2731 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2732 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2737 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2739 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2740 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2742 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2748 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2753 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { (yyval).reset();;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2759 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2761 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2763 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2764 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2769 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2780 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { (yyval).reset();;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2792 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2793 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2802 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2805 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2810 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
    { (yyval).reset();;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2819 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2823 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2824 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2829 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2854 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2855 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2859 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2861 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2868 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2874 "hphp.y"
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[(2) - (5)]),
                                        !(yyvsp[(4) - (5)]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[(5) - (5)])
                                      );
                                    ;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2885 "hphp.y"
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[(2) - (5)]),
                                        !(yyvsp[(4) - (5)]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[(5) - (5)])
                                      );
                                    ;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2900 "hphp.y"
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[(2) - (5)]),
                                        !(yyvsp[(4) - (5)]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[(5) - (5)])
                                      );
                                    ;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2912 "hphp.y"
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[(2) - (5)]),
                                        !(yyvsp[(4) - (5)]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[(5) - (5)])
                                      );
                                    ;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2926 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2927 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2929 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2931 "hphp.y"
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[(1) - (3)]),
                                        !(yyvsp[(2) - (3)]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[(3) - (3)])
                                      );
                                    ;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2948 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2950 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2952 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2953 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2957 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2959 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[(1) - (3)]),
                                        !(yyvsp[(2) - (3)]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[(3) - (3)])
                                      );
                                    ;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2977 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2980 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2989 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2990 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2991 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2994 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2995 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2999 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3003 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3007 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3018 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3025 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3034 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3038 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3042 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3051 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3052 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3053 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3057 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3058 "hphp.y"
    { _p->onPipeVariable((yyval));;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3059 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3067 "hphp.y"
    { (yyval).reset();;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3078 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3079 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3080 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3083 "hphp.y"
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[(1) - (3)]),
                                        !(yyvsp[(2) - (3)]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[(3) - (3)])
                                      );
                                    ;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3094 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3095 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3099 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3100 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3103 "hphp.y"
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[(1) - (3)]),
                                        !(yyvsp[(2) - (3)]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[(3) - (3)])
                                      );
                                    ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3112 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3117 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3119 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3122 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3128 "hphp.y"
    { (yyval).reset();;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3133 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3135 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3138 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3140 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3141 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3142 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3147 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3148 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3152 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3153 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3154 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3155 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3160 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3161 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3166 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3168 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3170 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3171 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3175 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3177 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3178 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3180 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3185 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3187 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3189 "hphp.y"
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[(1) - (3)]),
                                           !(yyvsp[(2) - (3)]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[(3) - (3)])
                                         );
                                       ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3199 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3201 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3202 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3205 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3206 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3207 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3211 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3212 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3213 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3214 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3215 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3216 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3217 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3218 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3219 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3220 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3221 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3225 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3226 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3231 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3233 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3247 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3252 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3256 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3261 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3268 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 995:

/* Line 1455 of yacc.c  */
#line 3272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 996:

/* Line 1455 of yacc.c  */
#line 3273 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3279 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3283 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3289 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3293 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3300 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3301 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3305 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1004:

/* Line 1455 of yacc.c  */
#line 3308 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1005:

/* Line 1455 of yacc.c  */
#line 3314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1006:

/* Line 1455 of yacc.c  */
#line 3319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3320 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3321 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3322 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1016:

/* Line 1455 of yacc.c  */
#line 3343 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3344 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1; ;}
    break;

  case 1020:

/* Line 1455 of yacc.c  */
#line 3353 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1023:

/* Line 1455 of yacc.c  */
#line 3364 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 1024:

/* Line 1455 of yacc.c  */
#line 3366 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 1025:

/* Line 1455 of yacc.c  */
#line 3370 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 1026:

/* Line 1455 of yacc.c  */
#line 3373 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 1027:

/* Line 1455 of yacc.c  */
#line 3377 "hphp.y"
    {;}
    break;

  case 1028:

/* Line 1455 of yacc.c  */
#line 3378 "hphp.y"
    {;}
    break;

  case 1029:

/* Line 1455 of yacc.c  */
#line 3379 "hphp.y"
    {;}
    break;

  case 1030:

/* Line 1455 of yacc.c  */
#line 3385 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 1031:

/* Line 1455 of yacc.c  */
#line 3390 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 1032:

/* Line 1455 of yacc.c  */
#line 3401 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 1033:

/* Line 1455 of yacc.c  */
#line 3406 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1034:

/* Line 1455 of yacc.c  */
#line 3407 "hphp.y"
    { ;}
    break;

  case 1035:

/* Line 1455 of yacc.c  */
#line 3412 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 1036:

/* Line 1455 of yacc.c  */
#line 3413 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 1037:

/* Line 1455 of yacc.c  */
#line 3419 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1038:

/* Line 1455 of yacc.c  */
#line 3424 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1039:

/* Line 1455 of yacc.c  */
#line 3429 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1040:

/* Line 1455 of yacc.c  */
#line 3433 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1041:

/* Line 1455 of yacc.c  */
#line 3438 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 1042:

/* Line 1455 of yacc.c  */
#line 3440 "hphp.y"
    { _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)])); (yyval) = (yyvsp[(2) - (5)]);;}
    break;

  case 1043:

/* Line 1455 of yacc.c  */
#line 3446 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1044:

/* Line 1455 of yacc.c  */
#line 3449 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1045:

/* Line 1455 of yacc.c  */
#line 3452 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1046:

/* Line 1455 of yacc.c  */
#line 3453 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1047:

/* Line 1455 of yacc.c  */
#line 3456 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1048:

/* Line 1455 of yacc.c  */
#line 3459 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1049:

/* Line 1455 of yacc.c  */
#line 3462 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1050:

/* Line 1455 of yacc.c  */
#line 3465 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1051:

/* Line 1455 of yacc.c  */
#line 3467 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1052:

/* Line 1455 of yacc.c  */
#line 3473 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1053:

/* Line 1455 of yacc.c  */
#line 3479 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1054:

/* Line 1455 of yacc.c  */
#line 3487 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1055:

/* Line 1455 of yacc.c  */
#line 3488 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14642 "hphp.7.tab.cpp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, _p, YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (&yylloc, _p, yymsg);
	  }
	else
	  {
	    yyerror (&yylloc, _p, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[0] = yylloc;

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

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
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
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
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

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp, _p);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
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

#if !defined(yyoverflow) || YYERROR_VERBOSE
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
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc, _p);
  /* Do not reclaim the symbols of the rule which action triggered
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
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 3491 "hphp.y"

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

