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
    Token fname2;   fname2.setText("__xhpAttributeDeclaration");
    Token param1;  _p->onCall(param1, 0, fname2, dummy, &cls);
    Token params1; _p->onCallParam(params1, NULL, param1, false, false);

    for (unsigned int i = 0; i < classes.size(); i++) {
      Token parent2;  parent2.set(T_STRING, classes[i]);
      Token cls2;     _p->onName(cls2, parent2, Parser::StringName);
      Token fname3;   fname3.setText("__xhpAttributeDeclaration");
      Token param;   _p->onCall(param, 0, fname3, dummy, &cls2);

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
     T_VARRAY = 377,
     T_DARRAY = 378,
     T_KEYSET = 379,
     T_CALLABLE = 380,
     T_CLASS_C = 381,
     T_METHOD_C = 382,
     T_FUNC_C = 383,
     T_LINE = 384,
     T_FILE = 385,
     T_COMMENT = 386,
     T_DOC_COMMENT = 387,
     T_OPEN_TAG = 388,
     T_OPEN_TAG_WITH_ECHO = 389,
     T_CLOSE_TAG = 390,
     T_WHITESPACE = 391,
     T_START_HEREDOC = 392,
     T_END_HEREDOC = 393,
     T_DOLLAR_OPEN_CURLY_BRACES = 394,
     T_CURLY_OPEN = 395,
     T_DOUBLE_COLON = 396,
     T_NAMESPACE = 397,
     T_NS_C = 398,
     T_DIR = 399,
     T_NS_SEPARATOR = 400,
     T_XHP_LABEL = 401,
     T_XHP_TEXT = 402,
     T_XHP_ATTRIBUTE = 403,
     T_XHP_CATEGORY = 404,
     T_XHP_CATEGORY_LABEL = 405,
     T_XHP_CHILDREN = 406,
     T_ENUM = 407,
     T_XHP_REQUIRED = 408,
     T_TRAIT = 409,
     T_ELLIPSIS = 410,
     T_INSTEADOF = 411,
     T_TRAIT_C = 412,
     T_HH_ERROR = 413,
     T_FINALLY = 414,
     T_XHP_TAG_LT = 415,
     T_XHP_TAG_GT = 416,
     T_TYPELIST_LT = 417,
     T_TYPELIST_GT = 418,
     T_UNRESOLVED_LT = 419,
     T_COLLECTION = 420,
     T_SHAPE = 421,
     T_TYPE = 422,
     T_UNRESOLVED_TYPE = 423,
     T_NEWTYPE = 424,
     T_UNRESOLVED_NEWTYPE = 425,
     T_COMPILER_HALT_OFFSET = 426,
     T_ASYNC = 427,
     T_LAMBDA_OP = 428,
     T_LAMBDA_CP = 429,
     T_UNRESOLVED_OP = 430,
     T_WHERE = 431
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
#line 897 "hphp.7.tab.cpp"

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
#define YYLAST   18303

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  206
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  301
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1074
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1974

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   431

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,   204,     2,   201,    55,    38,   205,
     196,   197,    53,    50,     9,    51,    52,    54,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    32,   198,
      43,    14,    44,    31,    59,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    70,     2,   203,    37,     2,   202,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   199,    36,   200,    58,     2,     2,     2,
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
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195
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
     237,   239,   241,   243,   245,   248,   252,   256,   258,   261,
     263,   266,   270,   275,   279,   281,   284,   286,   289,   292,
     294,   298,   300,   304,   307,   310,   313,   319,   324,   327,
     328,   330,   332,   334,   336,   340,   346,   355,   356,   361,
     362,   369,   370,   381,   382,   387,   390,   394,   397,   401,
     404,   408,   412,   416,   420,   424,   428,   434,   436,   438,
     440,   441,   451,   452,   463,   469,   470,   484,   485,   491,
     495,   499,   502,   505,   508,   511,   514,   517,   521,   524,
     527,   531,   534,   537,   538,   543,   553,   554,   555,   560,
     563,   564,   566,   567,   569,   570,   580,   581,   592,   593,
     605,   606,   616,   617,   628,   629,   638,   639,   649,   650,
     658,   659,   668,   669,   678,   679,   687,   688,   697,   699,
     701,   703,   705,   707,   710,   714,   718,   721,   724,   725,
     728,   729,   732,   733,   735,   739,   741,   745,   748,   749,
     751,   754,   759,   761,   766,   768,   773,   775,   780,   782,
     787,   791,   797,   801,   806,   811,   817,   823,   828,   829,
     831,   833,   838,   839,   845,   846,   849,   850,   854,   855,
     863,   872,   879,   882,   888,   895,   900,   901,   906,   912,
     920,   927,   934,   942,   952,   961,   968,   976,   982,   985,
     990,   996,  1000,  1001,  1005,  1010,  1017,  1023,  1029,  1036,
    1045,  1053,  1056,  1057,  1059,  1062,  1065,  1069,  1074,  1079,
    1083,  1085,  1087,  1090,  1095,  1099,  1105,  1107,  1111,  1114,
    1115,  1118,  1122,  1125,  1126,  1127,  1132,  1133,  1139,  1142,
    1145,  1148,  1149,  1161,  1162,  1175,  1179,  1183,  1187,  1192,
    1197,  1201,  1207,  1210,  1213,  1214,  1221,  1227,  1232,  1236,
    1238,  1240,  1244,  1249,  1251,  1254,  1256,  1258,  1264,  1271,
    1273,  1275,  1280,  1282,  1284,  1288,  1291,  1294,  1295,  1298,
    1299,  1301,  1305,  1307,  1309,  1311,  1313,  1317,  1322,  1327,
    1332,  1334,  1336,  1339,  1342,  1345,  1349,  1353,  1355,  1357,
    1359,  1361,  1365,  1367,  1371,  1373,  1375,  1377,  1378,  1380,
    1383,  1385,  1387,  1389,  1391,  1393,  1395,  1397,  1399,  1400,
    1402,  1404,  1406,  1410,  1416,  1418,  1422,  1428,  1433,  1437,
    1441,  1445,  1450,  1454,  1458,  1462,  1465,  1468,  1470,  1472,
    1476,  1480,  1482,  1484,  1485,  1487,  1490,  1495,  1499,  1503,
    1510,  1513,  1517,  1520,  1524,  1531,  1533,  1535,  1537,  1539,
    1541,  1548,  1552,  1557,  1564,  1568,  1572,  1576,  1580,  1584,
    1588,  1592,  1596,  1600,  1604,  1608,  1612,  1615,  1618,  1621,
    1624,  1628,  1632,  1636,  1640,  1644,  1648,  1652,  1656,  1660,
    1664,  1668,  1672,  1676,  1680,  1684,  1688,  1692,  1696,  1699,
    1702,  1705,  1708,  1712,  1716,  1720,  1724,  1728,  1732,  1736,
    1740,  1744,  1748,  1752,  1758,  1763,  1767,  1769,  1772,  1775,
    1778,  1781,  1784,  1787,  1790,  1793,  1796,  1798,  1800,  1802,
    1804,  1806,  1808,  1810,  1812,  1816,  1819,  1821,  1827,  1828,
    1829,  1842,  1843,  1857,  1858,  1863,  1864,  1872,  1873,  1879,
    1880,  1884,  1885,  1892,  1895,  1898,  1903,  1905,  1907,  1913,
    1917,  1923,  1927,  1930,  1931,  1934,  1935,  1940,  1945,  1949,
    1952,  1953,  1959,  1963,  1966,  1967,  1973,  1977,  1980,  1981,
    1987,  1991,  1996,  2001,  2006,  2011,  2016,  2021,  2026,  2031,
    2036,  2041,  2046,  2051,  2056,  2061,  2066,  2069,  2070,  2073,
    2074,  2077,  2078,  2083,  2088,  2093,  2098,  2100,  2102,  2104,
    2106,  2108,  2110,  2112,  2114,  2116,  2120,  2122,  2126,  2131,
    2133,  2136,  2141,  2144,  2151,  2152,  2154,  2159,  2160,  2163,
    2164,  2166,  2168,  2172,  2174,  2178,  2180,  2182,  2186,  2190,
    2192,  2194,  2196,  2198,  2200,  2202,  2204,  2206,  2208,  2210,
    2212,  2214,  2216,  2218,  2220,  2222,  2224,  2226,  2228,  2230,
    2232,  2234,  2236,  2238,  2240,  2242,  2244,  2246,  2248,  2250,
    2252,  2254,  2256,  2258,  2260,  2262,  2264,  2266,  2268,  2270,
    2272,  2274,  2276,  2278,  2280,  2282,  2284,  2286,  2288,  2290,
    2292,  2294,  2296,  2298,  2300,  2302,  2304,  2306,  2308,  2310,
    2312,  2314,  2316,  2318,  2320,  2322,  2324,  2326,  2328,  2330,
    2332,  2334,  2336,  2338,  2340,  2342,  2344,  2346,  2348,  2350,
    2352,  2357,  2359,  2361,  2363,  2365,  2367,  2369,  2373,  2375,
    2379,  2381,  2383,  2385,  2389,  2391,  2393,  2395,  2398,  2400,
    2401,  2402,  2404,  2406,  2410,  2411,  2413,  2415,  2417,  2419,
    2421,  2423,  2425,  2427,  2429,  2431,  2433,  2435,  2437,  2441,
    2444,  2446,  2448,  2453,  2457,  2462,  2464,  2466,  2468,  2470,
    2472,  2474,  2476,  2480,  2484,  2488,  2492,  2496,  2500,  2504,
    2508,  2512,  2516,  2520,  2524,  2528,  2532,  2536,  2540,  2544,
    2548,  2551,  2554,  2557,  2560,  2564,  2568,  2572,  2576,  2580,
    2584,  2588,  2592,  2596,  2602,  2607,  2611,  2613,  2617,  2621,
    2623,  2625,  2627,  2629,  2631,  2635,  2639,  2643,  2646,  2647,
    2649,  2650,  2652,  2653,  2659,  2663,  2667,  2669,  2671,  2673,
    2675,  2679,  2682,  2684,  2686,  2688,  2690,  2692,  2696,  2698,
    2700,  2702,  2706,  2708,  2711,  2714,  2719,  2723,  2728,  2730,
    2732,  2734,  2736,  2738,  2742,  2744,  2747,  2748,  2754,  2758,
    2762,  2764,  2768,  2770,  2773,  2774,  2780,  2784,  2787,  2788,
    2792,  2793,  2798,  2801,  2802,  2806,  2810,  2812,  2813,  2815,
    2817,  2819,  2821,  2825,  2827,  2829,  2831,  2835,  2837,  2839,
    2843,  2847,  2850,  2855,  2858,  2863,  2869,  2875,  2881,  2887,
    2889,  2891,  2893,  2895,  2897,  2899,  2903,  2907,  2912,  2917,
    2921,  2923,  2925,  2927,  2929,  2933,  2935,  2940,  2944,  2948,
    2950,  2952,  2954,  2956,  2958,  2962,  2966,  2971,  2976,  2980,
    2982,  2984,  2992,  3002,  3010,  3017,  3026,  3028,  3033,  3038,
    3040,  3042,  3044,  3049,  3052,  3054,  3055,  3057,  3059,  3061,
    3065,  3069,  3073,  3074,  3076,  3078,  3082,  3086,  3089,  3093,
    3100,  3101,  3103,  3108,  3111,  3112,  3118,  3122,  3126,  3128,
    3135,  3140,  3145,  3148,  3151,  3152,  3158,  3162,  3166,  3168,
    3171,  3172,  3178,  3182,  3186,  3188,  3191,  3194,  3196,  3199,
    3201,  3206,  3210,  3214,  3221,  3225,  3227,  3229,  3231,  3236,
    3241,  3246,  3251,  3256,  3261,  3264,  3267,  3272,  3275,  3278,
    3280,  3284,  3288,  3292,  3293,  3296,  3302,  3309,  3316,  3324,
    3326,  3329,  3331,  3334,  3336,  3341,  3343,  3348,  3352,  3353,
    3355,  3359,  3362,  3366,  3368,  3370,  3371,  3372,  3376,  3378,
    3382,  3386,  3389,  3390,  3393,  3396,  3399,  3402,  3404,  3407,
    3412,  3415,  3421,  3425,  3427,  3429,  3430,  3434,  3439,  3445,
    3452,  3456,  3458,  3462,  3465,  3467,  3468,  3473,  3475,  3479,
    3482,  3487,  3493,  3496,  3499,  3501,  3503,  3505,  3507,  3511,
    3514,  3516,  3525,  3532,  3534
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     207,     0,    -1,    -1,   208,   209,    -1,   209,   210,    -1,
      -1,   230,    -1,   247,    -1,   254,    -1,   251,    -1,   261,
      -1,   480,    -1,   129,   196,   197,   198,    -1,   161,   223,
     198,    -1,    -1,   161,   223,   199,   211,   209,   200,    -1,
      -1,   161,   199,   212,   209,   200,    -1,   117,   218,   198,
      -1,   117,   111,   218,   198,    -1,   117,   112,   218,   198,
      -1,   117,   216,   199,   221,   200,   198,    -1,   117,   111,
     216,   199,   218,   200,   198,    -1,   117,   112,   216,   199,
     218,   200,   198,    -1,   227,   198,    -1,    81,    -1,   103,
      -1,   195,    -1,   167,    -1,   168,    -1,   170,    -1,   172,
      -1,   171,    -1,   139,    -1,   140,    -1,   143,    -1,   141,
      -1,   142,    -1,   213,    -1,   144,    -1,   173,    -1,   132,
      -1,   133,    -1,   124,    -1,   123,    -1,   122,    -1,   121,
      -1,   120,    -1,   119,    -1,   112,    -1,   101,    -1,    97,
      -1,    99,    -1,    77,    -1,    95,    -1,    12,    -1,   118,
      -1,   109,    -1,    57,    -1,   175,    -1,   131,    -1,   161,
      -1,    72,    -1,    10,    -1,    11,    -1,   114,    -1,   117,
      -1,   125,    -1,    73,    -1,   137,    -1,    71,    -1,     7,
      -1,     6,    -1,   116,    -1,   138,    -1,    13,    -1,    92,
      -1,     4,    -1,     3,    -1,   113,    -1,    76,    -1,    75,
      -1,   107,    -1,   108,    -1,   110,    -1,   104,    -1,    27,
      -1,    29,    -1,   111,    -1,    74,    -1,   105,    -1,   178,
      -1,    96,    -1,    98,    -1,   100,    -1,   106,    -1,    93,
      -1,    94,    -1,   102,    -1,   115,    -1,   128,    -1,   126,
      -1,   214,    -1,   130,    -1,   223,   164,    -1,   164,   223,
     164,    -1,   217,     9,   219,    -1,   219,    -1,   217,   424,
      -1,   223,    -1,   164,   223,    -1,   223,   102,   213,    -1,
     164,   223,   102,   213,    -1,   220,     9,   222,    -1,   222,
      -1,   220,   424,    -1,   219,    -1,   111,   219,    -1,   112,
     219,    -1,   213,    -1,   223,   164,   213,    -1,   223,    -1,
     161,   164,   223,    -1,   164,   223,    -1,   224,   485,    -1,
     224,   485,    -1,   227,     9,   481,    14,   417,    -1,   112,
     481,    14,   417,    -1,   228,   229,    -1,    -1,   230,    -1,
     247,    -1,   254,    -1,   261,    -1,   199,   228,   200,    -1,
      74,   337,   230,   283,   285,    -1,    74,   337,    32,   228,
     284,   286,    77,   198,    -1,    -1,    94,   337,   231,   277,
      -1,    -1,    93,   232,   230,    94,   337,   198,    -1,    -1,
      96,   196,   339,   198,   339,   198,   339,   197,   233,   275,
      -1,    -1,   104,   337,   234,   280,    -1,   108,   198,    -1,
     108,   348,   198,    -1,   110,   198,    -1,   110,   348,   198,
      -1,   113,   198,    -1,   113,   348,   198,    -1,    27,   108,
     198,    -1,   118,   293,   198,    -1,   124,   295,   198,    -1,
      92,   338,   198,    -1,   153,   338,   198,    -1,   126,   196,
     477,   197,   198,    -1,   198,    -1,    86,    -1,    87,    -1,
      -1,    98,   196,   348,   102,   274,   273,   197,   235,   276,
      -1,    -1,    98,   196,   348,    28,   102,   274,   273,   197,
     236,   276,    -1,   100,   196,   279,   197,   278,    -1,    -1,
     114,   239,   115,   196,   408,    83,   197,   199,   228,   200,
     241,   237,   244,    -1,    -1,   114,   239,   178,   238,   242,
      -1,   116,   348,   198,    -1,   109,   213,   198,    -1,   348,
     198,    -1,   340,   198,    -1,   341,   198,    -1,   342,   198,
      -1,   343,   198,    -1,   344,   198,    -1,   113,   343,   198,
      -1,   345,   198,    -1,   346,   198,    -1,   113,   345,   198,
      -1,   347,   198,    -1,   213,    32,    -1,    -1,   199,   240,
     228,   200,    -1,   241,   115,   196,   408,    83,   197,   199,
     228,   200,    -1,    -1,    -1,   199,   243,   228,   200,    -1,
     178,   242,    -1,    -1,    38,    -1,    -1,   111,    -1,    -1,
     246,   245,   484,   248,   196,   289,   197,   492,   323,    -1,
      -1,   327,   246,   245,   484,   249,   196,   289,   197,   492,
     323,    -1,    -1,   441,   326,   246,   245,   484,   250,   196,
     289,   197,   492,   323,    -1,    -1,   171,   213,   252,    32,
     505,   479,   199,   296,   200,    -1,    -1,   441,   171,   213,
     253,    32,   505,   479,   199,   296,   200,    -1,    -1,   267,
     264,   255,   268,   269,   199,   299,   200,    -1,    -1,   441,
     267,   264,   256,   268,   269,   199,   299,   200,    -1,    -1,
     131,   265,   257,   270,   199,   299,   200,    -1,    -1,   441,
     131,   265,   258,   270,   199,   299,   200,    -1,    -1,   130,
     260,   415,   268,   269,   199,   299,   200,    -1,    -1,   173,
     266,   262,   269,   199,   299,   200,    -1,    -1,   441,   173,
     266,   263,   269,   199,   299,   200,    -1,   484,    -1,   165,
      -1,   484,    -1,   484,    -1,   130,    -1,   123,   130,    -1,
     123,   122,   130,    -1,   122,   123,   130,    -1,   122,   130,
      -1,   132,   408,    -1,    -1,   133,   271,    -1,    -1,   132,
     271,    -1,    -1,   408,    -1,   271,     9,   408,    -1,   408,
      -1,   272,     9,   408,    -1,   136,   274,    -1,    -1,   453,
      -1,    38,   453,    -1,   137,   196,   466,   197,    -1,   230,
      -1,    32,   228,    97,   198,    -1,   230,    -1,    32,   228,
      99,   198,    -1,   230,    -1,    32,   228,    95,   198,    -1,
     230,    -1,    32,   228,   101,   198,    -1,   213,    14,   417,
      -1,   279,     9,   213,    14,   417,    -1,   199,   281,   200,
      -1,   199,   198,   281,   200,    -1,    32,   281,   105,   198,
      -1,    32,   198,   281,   105,   198,    -1,   281,   106,   348,
     282,   228,    -1,   281,   107,   282,   228,    -1,    -1,    32,
      -1,   198,    -1,   283,    75,   337,   230,    -1,    -1,   284,
      75,   337,    32,   228,    -1,    -1,    76,   230,    -1,    -1,
      76,    32,   228,    -1,    -1,   288,     9,   442,   329,   506,
     174,    83,    -1,   288,     9,   442,   329,   506,    38,   174,
      83,    -1,   288,     9,   442,   329,   506,   174,    -1,   288,
     424,    -1,   442,   329,   506,   174,    83,    -1,   442,   329,
     506,    38,   174,    83,    -1,   442,   329,   506,   174,    -1,
      -1,   442,   329,   506,    83,    -1,   442,   329,   506,    38,
      83,    -1,   442,   329,   506,    38,    83,    14,   348,    -1,
     442,   329,   506,    83,    14,   348,    -1,   288,     9,   442,
     329,   506,    83,    -1,   288,     9,   442,   329,   506,    38,
      83,    -1,   288,     9,   442,   329,   506,    38,    83,    14,
     348,    -1,   288,     9,   442,   329,   506,    83,    14,   348,
      -1,   290,     9,   442,   506,   174,    83,    -1,   290,     9,
     442,   506,    38,   174,    83,    -1,   290,     9,   442,   506,
     174,    -1,   290,   424,    -1,   442,   506,   174,    83,    -1,
     442,   506,    38,   174,    83,    -1,   442,   506,   174,    -1,
      -1,   442,   506,    83,    -1,   442,   506,    38,    83,    -1,
     442,   506,    38,    83,    14,   348,    -1,   442,   506,    83,
      14,   348,    -1,   290,     9,   442,   506,    83,    -1,   290,
       9,   442,   506,    38,    83,    -1,   290,     9,   442,   506,
      38,    83,    14,   348,    -1,   290,     9,   442,   506,    83,
      14,   348,    -1,   292,   424,    -1,    -1,   348,    -1,    38,
     453,    -1,   174,   348,    -1,   292,     9,   348,    -1,   292,
       9,   174,   348,    -1,   292,     9,    38,   453,    -1,   293,
       9,   294,    -1,   294,    -1,    83,    -1,   201,   453,    -1,
     201,   199,   348,   200,    -1,   295,     9,    83,    -1,   295,
       9,    83,    14,   417,    -1,    83,    -1,    83,    14,   417,
      -1,   296,   297,    -1,    -1,   298,   198,    -1,   482,    14,
     417,    -1,   299,   300,    -1,    -1,    -1,   325,   301,   331,
     198,    -1,    -1,   327,   505,   302,   331,   198,    -1,   332,
     198,    -1,   333,   198,    -1,   334,   198,    -1,    -1,   326,
     246,   245,   483,   196,   303,   287,   197,   492,   489,   324,
      -1,    -1,   441,   326,   246,   245,   484,   196,   304,   287,
     197,   492,   489,   324,    -1,   167,   309,   198,    -1,   168,
     317,   198,    -1,   170,   319,   198,    -1,     4,   132,   408,
     198,    -1,     4,   133,   408,   198,    -1,   117,   272,   198,
      -1,   117,   272,   199,   305,   200,    -1,   305,   306,    -1,
     305,   307,    -1,    -1,   226,   160,   213,   175,   272,   198,
      -1,   308,   102,   326,   213,   198,    -1,   308,   102,   327,
     198,    -1,   226,   160,   213,    -1,   213,    -1,   310,    -1,
     309,     9,   310,    -1,   311,   405,   315,   316,    -1,   165,
      -1,    31,   312,    -1,   312,    -1,   138,    -1,   138,   181,
     505,   423,   182,    -1,   138,   181,   505,     9,   505,   182,
      -1,   408,    -1,   125,    -1,   171,   199,   314,   200,    -1,
     144,    -1,   416,    -1,   313,     9,   416,    -1,   313,   423,
      -1,    14,   417,    -1,    -1,    59,   172,    -1,    -1,   318,
      -1,   317,     9,   318,    -1,   169,    -1,   320,    -1,   213,
      -1,   128,    -1,   196,   321,   197,    -1,   196,   321,   197,
      53,    -1,   196,   321,   197,    31,    -1,   196,   321,   197,
      50,    -1,   320,    -1,   322,    -1,   322,    53,    -1,   322,
      31,    -1,   322,    50,    -1,   321,     9,   321,    -1,   321,
      36,   321,    -1,   213,    -1,   165,    -1,   169,    -1,   198,
      -1,   199,   228,   200,    -1,   198,    -1,   199,   228,   200,
      -1,   327,    -1,   125,    -1,   327,    -1,    -1,   328,    -1,
     327,   328,    -1,   119,    -1,   120,    -1,   121,    -1,   124,
      -1,   123,    -1,   122,    -1,   191,    -1,   330,    -1,    -1,
     119,    -1,   120,    -1,   121,    -1,   331,     9,    83,    -1,
     331,     9,    83,    14,   417,    -1,    83,    -1,    83,    14,
     417,    -1,   332,     9,   482,    14,   417,    -1,   112,   482,
      14,   417,    -1,   333,     9,   482,    -1,   123,   112,   482,
      -1,   123,   335,   479,    -1,   335,   479,    14,   505,    -1,
     112,   186,   484,    -1,   196,   336,   197,    -1,    72,   412,
     415,    -1,    72,   259,    -1,    71,   348,    -1,   397,    -1,
     392,    -1,   196,   348,   197,    -1,   338,     9,   348,    -1,
     348,    -1,   338,    -1,    -1,    27,    -1,    27,   348,    -1,
      27,   348,   136,   348,    -1,   196,   340,   197,    -1,   453,
      14,   340,    -1,   137,   196,   466,   197,    14,   340,    -1,
      29,   348,    -1,   453,    14,   343,    -1,    28,   348,    -1,
     453,    14,   345,    -1,   137,   196,   466,   197,    14,   345,
      -1,   349,    -1,   453,    -1,   336,    -1,   457,    -1,   456,
      -1,   137,   196,   466,   197,    14,   348,    -1,   453,    14,
     348,    -1,   453,    14,    38,   453,    -1,   453,    14,    38,
      72,   412,   415,    -1,   453,    26,   348,    -1,   453,    25,
     348,    -1,   453,    24,   348,    -1,   453,    23,   348,    -1,
     453,    22,   348,    -1,   453,    21,   348,    -1,   453,    20,
     348,    -1,   453,    19,   348,    -1,   453,    18,   348,    -1,
     453,    17,   348,    -1,   453,    16,   348,    -1,   453,    15,
     348,    -1,   453,    68,    -1,    68,   453,    -1,   453,    67,
      -1,    67,   453,    -1,   348,    34,   348,    -1,   348,    35,
     348,    -1,   348,    10,   348,    -1,   348,    12,   348,    -1,
     348,    11,   348,    -1,   348,    36,   348,    -1,   348,    38,
     348,    -1,   348,    37,   348,    -1,   348,    52,   348,    -1,
     348,    50,   348,    -1,   348,    51,   348,    -1,   348,    53,
     348,    -1,   348,    54,   348,    -1,   348,    69,   348,    -1,
     348,    55,   348,    -1,   348,    30,   348,    -1,   348,    49,
     348,    -1,   348,    48,   348,    -1,    50,   348,    -1,    51,
     348,    -1,    56,   348,    -1,    58,   348,    -1,   348,    40,
     348,    -1,   348,    39,   348,    -1,   348,    42,   348,    -1,
     348,    41,   348,    -1,   348,    43,   348,    -1,   348,    47,
     348,    -1,   348,    44,   348,    -1,   348,    46,   348,    -1,
     348,    45,   348,    -1,   348,    57,   412,    -1,   196,   349,
     197,    -1,   348,    31,   348,    32,   348,    -1,   348,    31,
      32,   348,    -1,   348,    33,   348,    -1,   476,    -1,    66,
     348,    -1,    65,   348,    -1,    64,   348,    -1,    63,   348,
      -1,    62,   348,    -1,    61,   348,    -1,    60,   348,    -1,
      73,   413,    -1,    59,   348,    -1,   421,    -1,   367,    -1,
     374,    -1,   377,    -1,   380,    -1,   383,    -1,   386,    -1,
     366,    -1,   202,   414,   202,    -1,    13,   348,    -1,   394,
      -1,   117,   196,   396,   424,   197,    -1,    -1,    -1,   246,
     245,   196,   352,   289,   197,   492,   350,   492,   199,   228,
     200,    -1,    -1,   327,   246,   245,   196,   353,   289,   197,
     492,   350,   492,   199,   228,   200,    -1,    -1,   191,    83,
     355,   360,    -1,    -1,   191,   192,   356,   289,   193,   492,
     360,    -1,    -1,   191,   199,   357,   228,   200,    -1,    -1,
      83,   358,   360,    -1,    -1,   192,   359,   289,   193,   492,
     360,    -1,     8,   348,    -1,     8,   345,    -1,     8,   199,
     228,   200,    -1,    91,    -1,   478,    -1,   362,     9,   361,
     136,   348,    -1,   361,   136,   348,    -1,   363,     9,   361,
     136,   417,    -1,   361,   136,   417,    -1,   362,   423,    -1,
      -1,   363,   423,    -1,    -1,   185,   196,   364,   197,    -1,
     138,   196,   467,   197,    -1,    70,   467,   203,    -1,   369,
     423,    -1,    -1,   369,     9,   348,   136,   348,    -1,   348,
     136,   348,    -1,   371,   423,    -1,    -1,   371,     9,   417,
     136,   417,    -1,   417,   136,   417,    -1,   373,   423,    -1,
      -1,   373,     9,   429,   136,   429,    -1,   429,   136,   429,
      -1,   139,    70,   368,   203,    -1,   139,    70,   370,   203,
      -1,   139,    70,   372,   203,    -1,   140,    70,   389,   203,
      -1,   140,    70,   390,   203,    -1,   140,    70,   391,   203,
      -1,   143,    70,   389,   203,    -1,   143,    70,   390,   203,
      -1,   143,    70,   391,   203,    -1,   141,    70,   389,   203,
      -1,   141,    70,   390,   203,    -1,   141,    70,   391,   203,
      -1,   142,    70,   368,   203,    -1,   142,    70,   370,   203,
      -1,   142,    70,   372,   203,    -1,   338,   423,    -1,    -1,
     418,   423,    -1,    -1,   430,   423,    -1,    -1,   408,   199,
     469,   200,    -1,   408,   199,   471,   200,    -1,   394,    70,
     463,   203,    -1,   395,    70,   463,   203,    -1,   367,    -1,
     374,    -1,   377,    -1,   380,    -1,   383,    -1,   386,    -1,
     478,    -1,   456,    -1,    91,    -1,   196,   349,   197,    -1,
      81,    -1,   396,     9,    83,    -1,   396,     9,    38,    83,
      -1,    83,    -1,    38,    83,    -1,   179,   165,   398,   180,
      -1,   400,    54,    -1,   400,   180,   401,   179,    54,   399,
      -1,    -1,   165,    -1,   400,   402,    14,   403,    -1,    -1,
     401,   404,    -1,    -1,   165,    -1,   166,    -1,   199,   348,
     200,    -1,   166,    -1,   199,   348,   200,    -1,   397,    -1,
     406,    -1,   405,    32,   406,    -1,   405,    51,   406,    -1,
     213,    -1,    73,    -1,   111,    -1,   112,    -1,   113,    -1,
      27,    -1,    29,    -1,    28,    -1,   114,    -1,   115,    -1,
     178,    -1,   116,    -1,    74,    -1,    75,    -1,    77,    -1,
      76,    -1,    94,    -1,    95,    -1,    93,    -1,    96,    -1,
      97,    -1,    98,    -1,    99,    -1,   100,    -1,   101,    -1,
      57,    -1,   102,    -1,   104,    -1,   105,    -1,   106,    -1,
     107,    -1,   108,    -1,   110,    -1,   109,    -1,    92,    -1,
      13,    -1,   130,    -1,   131,    -1,   132,    -1,   133,    -1,
      72,    -1,    71,    -1,   125,    -1,     5,    -1,     7,    -1,
       6,    -1,     4,    -1,     3,    -1,   161,    -1,   117,    -1,
     118,    -1,   127,    -1,   128,    -1,   129,    -1,   124,    -1,
     123,    -1,   122,    -1,   121,    -1,   120,    -1,   119,    -1,
     191,    -1,   126,    -1,   137,    -1,   138,    -1,    10,    -1,
      12,    -1,    11,    -1,   145,    -1,   147,    -1,   146,    -1,
     148,    -1,   149,    -1,   163,    -1,   162,    -1,   190,    -1,
     173,    -1,   176,    -1,   175,    -1,   186,    -1,   188,    -1,
     185,    -1,   225,   196,   291,   197,    -1,   226,    -1,   165,
      -1,   408,    -1,   416,    -1,   124,    -1,   461,    -1,   196,
     349,   197,    -1,   409,    -1,   410,   160,   462,    -1,   409,
      -1,   459,    -1,   407,    -1,   411,   160,   462,    -1,   408,
      -1,   124,    -1,   464,    -1,   196,   197,    -1,   337,    -1,
      -1,    -1,    90,    -1,   473,    -1,   196,   291,   197,    -1,
      -1,    78,    -1,    79,    -1,    80,    -1,    91,    -1,   148,
      -1,   149,    -1,   163,    -1,   145,    -1,   176,    -1,   146,
      -1,   147,    -1,   162,    -1,   190,    -1,   156,    90,   157,
      -1,   156,   157,    -1,   416,    -1,   224,    -1,   138,   196,
     422,   197,    -1,    70,   422,   203,    -1,   185,   196,   365,
     197,    -1,   375,    -1,   378,    -1,   381,    -1,   384,    -1,
     387,    -1,   420,    -1,   393,    -1,   196,   417,   197,    -1,
     417,    34,   417,    -1,   417,    35,   417,    -1,   417,    10,
     417,    -1,   417,    12,   417,    -1,   417,    11,   417,    -1,
     417,    36,   417,    -1,   417,    38,   417,    -1,   417,    37,
     417,    -1,   417,    52,   417,    -1,   417,    50,   417,    -1,
     417,    51,   417,    -1,   417,    53,   417,    -1,   417,    54,
     417,    -1,   417,    55,   417,    -1,   417,    49,   417,    -1,
     417,    48,   417,    -1,   417,    69,   417,    -1,    56,   417,
      -1,    58,   417,    -1,    50,   417,    -1,    51,   417,    -1,
     417,    40,   417,    -1,   417,    39,   417,    -1,   417,    42,
     417,    -1,   417,    41,   417,    -1,   417,    43,   417,    -1,
     417,    47,   417,    -1,   417,    44,   417,    -1,   417,    46,
     417,    -1,   417,    45,   417,    -1,   417,    31,   417,    32,
     417,    -1,   417,    31,    32,   417,    -1,   418,     9,   417,
      -1,   417,    -1,   408,   160,   130,    -1,   408,   160,   214,
      -1,   419,    -1,   224,    -1,    82,    -1,   478,    -1,   416,
      -1,   204,   473,   204,    -1,   205,   473,   205,    -1,   156,
     473,   157,    -1,   425,   423,    -1,    -1,     9,    -1,    -1,
       9,    -1,    -1,   425,     9,   417,   136,   417,    -1,   425,
       9,   417,    -1,   417,   136,   417,    -1,   417,    -1,    78,
      -1,    79,    -1,    80,    -1,   156,    90,   157,    -1,   156,
     157,    -1,    78,    -1,    79,    -1,    80,    -1,   213,    -1,
      91,    -1,    91,    52,   428,    -1,   426,    -1,   428,    -1,
     419,    -1,   408,   160,    81,    -1,   213,    -1,    50,   427,
      -1,    51,   427,    -1,   138,   196,   431,   197,    -1,    70,
     431,   203,    -1,   185,   196,   434,   197,    -1,   376,    -1,
     379,    -1,   382,    -1,   385,    -1,   388,    -1,   430,     9,
     429,    -1,   429,    -1,   432,   423,    -1,    -1,   432,     9,
     429,   136,   429,    -1,   432,     9,   429,    -1,   429,   136,
     429,    -1,   429,    -1,   433,     9,   429,    -1,   429,    -1,
     435,   423,    -1,    -1,   435,     9,   361,   136,   429,    -1,
     361,   136,   429,    -1,   433,   423,    -1,    -1,   196,   436,
     197,    -1,    -1,   438,     9,   213,   437,    -1,   213,   437,
      -1,    -1,   440,   438,   423,    -1,    49,   439,    48,    -1,
     441,    -1,    -1,   134,    -1,   135,    -1,   213,    -1,   165,
      -1,   199,   348,   200,    -1,   444,    -1,   462,    -1,   213,
      -1,   199,   348,   200,    -1,   446,    -1,   462,    -1,    70,
     463,   203,    -1,   199,   348,   200,    -1,   454,   448,    -1,
     196,   336,   197,   448,    -1,   465,   448,    -1,   196,   336,
     197,   448,    -1,   196,   336,   197,   443,   445,    -1,   196,
     349,   197,   443,   445,    -1,   196,   336,   197,   443,   444,
      -1,   196,   349,   197,   443,   444,    -1,   460,    -1,   407,
      -1,   458,    -1,   459,    -1,   449,    -1,   451,    -1,   453,
     443,   445,    -1,   411,   160,   462,    -1,   455,   196,   291,
     197,    -1,   456,   196,   291,   197,    -1,   196,   453,   197,
      -1,   407,    -1,   458,    -1,   459,    -1,   449,    -1,   453,
     443,   445,    -1,   452,    -1,   455,   196,   291,   197,    -1,
     196,   453,   197,    -1,   411,   160,   462,    -1,   460,    -1,
     449,    -1,   407,    -1,   367,    -1,   416,    -1,   196,   453,
     197,    -1,   196,   349,   197,    -1,   456,   196,   291,   197,
      -1,   455,   196,   291,   197,    -1,   196,   457,   197,    -1,
     351,    -1,   354,    -1,   453,   443,   447,   485,   196,   291,
     197,    -1,   196,   336,   197,   443,   447,   485,   196,   291,
     197,    -1,   411,   160,   215,   485,   196,   291,   197,    -1,
     411,   160,   462,   196,   291,   197,    -1,   411,   160,   199,
     348,   200,   196,   291,   197,    -1,   461,    -1,   461,    70,
     463,   203,    -1,   461,   199,   348,   200,    -1,   462,    -1,
      83,    -1,    84,    -1,   201,   199,   348,   200,    -1,   201,
     462,    -1,   348,    -1,    -1,   460,    -1,   450,    -1,   451,
      -1,   464,   443,   445,    -1,   410,   160,   460,    -1,   196,
     453,   197,    -1,    -1,   450,    -1,   452,    -1,   464,   443,
     444,    -1,   196,   453,   197,    -1,   466,     9,    -1,   466,
       9,   453,    -1,   466,     9,   137,   196,   466,   197,    -1,
      -1,   453,    -1,   137,   196,   466,   197,    -1,   468,   423,
      -1,    -1,   468,     9,   348,   136,   348,    -1,   468,     9,
     348,    -1,   348,   136,   348,    -1,   348,    -1,   468,     9,
     348,   136,    38,   453,    -1,   468,     9,    38,   453,    -1,
     348,   136,    38,   453,    -1,    38,   453,    -1,   470,   423,
      -1,    -1,   470,     9,   348,   136,   348,    -1,   470,     9,
     348,    -1,   348,   136,   348,    -1,   348,    -1,   472,   423,
      -1,    -1,   472,     9,   417,   136,   417,    -1,   472,     9,
     417,    -1,   417,   136,   417,    -1,   417,    -1,   473,   474,
      -1,   473,    90,    -1,   474,    -1,    90,   474,    -1,    83,
      -1,    83,    70,   475,   203,    -1,    83,   443,   213,    -1,
     158,   348,   200,    -1,   158,    82,    70,   348,   203,   200,
      -1,   159,   453,   200,    -1,   213,    -1,    85,    -1,    83,
      -1,   127,   196,   338,   197,    -1,   128,   196,   453,   197,
      -1,   128,   196,   349,   197,    -1,   128,   196,   457,   197,
      -1,   128,   196,   456,   197,    -1,   128,   196,   336,   197,
      -1,     7,   348,    -1,     6,   348,    -1,     5,   196,   348,
     197,    -1,     4,   348,    -1,     3,   348,    -1,   453,    -1,
     477,     9,   453,    -1,   411,   160,   214,    -1,   411,   160,
     130,    -1,    -1,   102,   505,    -1,   186,   484,    14,   505,
     198,    -1,   441,   186,   484,    14,   505,   198,    -1,   188,
     484,   479,    14,   505,   198,    -1,   441,   188,   484,   479,
      14,   505,   198,    -1,   215,    -1,   505,   215,    -1,   214,
      -1,   505,   214,    -1,   215,    -1,   215,   181,   494,   182,
      -1,   213,    -1,   213,   181,   494,   182,    -1,   181,   487,
     182,    -1,    -1,   505,    -1,   486,     9,   505,    -1,   486,
     423,    -1,   486,     9,   174,    -1,   487,    -1,   174,    -1,
      -1,    -1,   195,   490,   424,    -1,   491,    -1,   490,     9,
     491,    -1,   505,    14,   505,    -1,   505,   493,    -1,    -1,
      32,   505,    -1,   102,   505,    -1,   103,   505,    -1,   496,
     423,    -1,   493,    -1,   495,   493,    -1,   496,     9,   497,
     213,    -1,   497,   213,    -1,   496,     9,   497,   213,   495,
      -1,   497,   213,   495,    -1,    50,    -1,    51,    -1,    -1,
      91,   136,   505,    -1,    31,    91,   136,   505,    -1,   226,
     160,   213,   136,   505,    -1,    31,   226,   160,   213,   136,
     505,    -1,   499,     9,   498,    -1,   498,    -1,   499,     9,
     174,    -1,   499,   423,    -1,   174,    -1,    -1,   185,   196,
     500,   197,    -1,   226,    -1,   213,   160,   503,    -1,   213,
     485,    -1,   181,   505,   423,   182,    -1,   181,   505,     9,
     505,   182,    -1,    31,   505,    -1,    59,   505,    -1,   226,
      -1,   138,    -1,   144,    -1,   501,    -1,   502,   160,   503,
      -1,   138,   504,    -1,   165,    -1,   196,   111,   196,   488,
     197,    32,   505,   197,    -1,   196,   505,     9,   486,   423,
     197,    -1,   505,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   751,   751,   751,   760,   762,   765,   766,   767,   768,
     769,   770,   771,   774,   776,   776,   778,   778,   780,   782,
     785,   788,   792,   796,   800,   805,   806,   807,   808,   809,
     810,   811,   812,   813,   814,   815,   816,   817,   821,   822,
     823,   824,   825,   826,   827,   828,   829,   830,   831,   832,
     833,   834,   835,   836,   837,   838,   839,   840,   841,   842,
     843,   844,   845,   846,   847,   848,   849,   850,   851,   852,
     853,   854,   855,   856,   857,   858,   859,   860,   861,   862,
     863,   864,   865,   866,   867,   868,   869,   870,   871,   872,
     873,   874,   875,   876,   877,   878,   879,   880,   881,   882,
     883,   887,   891,   892,   896,   897,   902,   904,   909,   914,
     915,   916,   918,   923,   925,   930,   935,   937,   939,   944,
     945,   949,   950,   952,   956,   963,   970,   974,   980,   982,
     985,   986,   987,   988,   991,   992,   996,  1001,  1001,  1007,
    1007,  1014,  1013,  1019,  1019,  1024,  1025,  1026,  1027,  1028,
    1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,
    1042,  1040,  1049,  1047,  1054,  1064,  1058,  1068,  1066,  1070,
    1071,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1094,  1094,  1099,  1105,  1109,  1109,  1117,
    1118,  1122,  1123,  1127,  1133,  1131,  1146,  1143,  1159,  1156,
    1173,  1172,  1181,  1179,  1191,  1190,  1209,  1207,  1226,  1225,
    1234,  1232,  1243,  1243,  1250,  1249,  1261,  1259,  1272,  1273,
    1277,  1280,  1283,  1284,  1285,  1288,  1289,  1292,  1294,  1297,
    1298,  1301,  1302,  1305,  1306,  1310,  1311,  1316,  1317,  1320,
    1321,  1322,  1326,  1327,  1331,  1332,  1336,  1337,  1341,  1342,
    1347,  1348,  1354,  1355,  1356,  1357,  1360,  1363,  1365,  1368,
    1369,  1373,  1375,  1378,  1381,  1384,  1385,  1388,  1389,  1393,
    1399,  1405,  1412,  1414,  1419,  1424,  1430,  1434,  1438,  1442,
    1447,  1452,  1457,  1462,  1468,  1477,  1482,  1487,  1493,  1495,
    1499,  1503,  1508,  1512,  1515,  1518,  1522,  1526,  1530,  1534,
    1539,  1547,  1549,  1552,  1553,  1554,  1555,  1557,  1559,  1564,
    1565,  1568,  1569,  1570,  1574,  1575,  1577,  1578,  1582,  1584,
    1587,  1591,  1597,  1599,  1602,  1602,  1606,  1605,  1609,  1611,
    1614,  1617,  1615,  1632,  1628,  1643,  1645,  1647,  1649,  1651,
    1653,  1655,  1659,  1660,  1661,  1664,  1670,  1674,  1680,  1683,
    1688,  1690,  1695,  1700,  1704,  1705,  1709,  1710,  1712,  1714,
    1720,  1721,  1723,  1727,  1728,  1733,  1737,  1738,  1742,  1743,
    1747,  1749,  1755,  1760,  1761,  1763,  1767,  1768,  1769,  1770,
    1774,  1775,  1776,  1777,  1778,  1779,  1781,  1786,  1789,  1790,
    1794,  1795,  1799,  1800,  1803,  1804,  1807,  1808,  1811,  1812,
    1816,  1817,  1818,  1819,  1820,  1821,  1822,  1826,  1827,  1830,
    1831,  1832,  1835,  1837,  1839,  1840,  1843,  1845,  1849,  1851,
    1855,  1859,  1863,  1868,  1869,  1871,  1872,  1873,  1874,  1877,
    1881,  1882,  1886,  1887,  1891,  1892,  1893,  1894,  1898,  1902,
    1907,  1911,  1915,  1919,  1923,  1928,  1929,  1930,  1931,  1932,
    1936,  1938,  1939,  1940,  1943,  1944,  1945,  1946,  1947,  1948,
    1949,  1950,  1951,  1952,  1953,  1954,  1955,  1956,  1957,  1958,
    1959,  1960,  1961,  1962,  1963,  1964,  1965,  1966,  1967,  1968,
    1969,  1970,  1971,  1972,  1973,  1974,  1975,  1976,  1977,  1978,
    1979,  1980,  1981,  1982,  1983,  1984,  1985,  1986,  1988,  1989,
    1991,  1992,  1994,  1995,  1996,  1997,  1998,  1999,  2000,  2001,
    2002,  2003,  2004,  2005,  2006,  2007,  2008,  2009,  2010,  2011,
    2012,  2013,  2014,  2015,  2016,  2017,  2018,  2022,  2026,  2031,
    2030,  2045,  2043,  2061,  2060,  2079,  2078,  2097,  2096,  2114,
    2114,  2129,  2129,  2147,  2148,  2149,  2154,  2156,  2160,  2164,
    2170,  2174,  2180,  2182,  2186,  2188,  2192,  2196,  2197,  2201,
    2203,  2207,  2209,  2213,  2215,  2219,  2222,  2227,  2229,  2233,
    2236,  2241,  2245,  2249,  2253,  2257,  2261,  2265,  2269,  2273,
    2277,  2281,  2285,  2289,  2293,  2297,  2301,  2303,  2307,  2309,
    2313,  2315,  2319,  2326,  2333,  2335,  2340,  2341,  2342,  2343,
    2344,  2345,  2346,  2347,  2348,  2350,  2351,  2355,  2356,  2357,
    2358,  2362,  2368,  2377,  2390,  2391,  2394,  2397,  2400,  2401,
    2404,  2408,  2411,  2414,  2421,  2422,  2426,  2427,  2429,  2434,
    2435,  2436,  2437,  2438,  2439,  2440,  2441,  2442,  2443,  2444,
    2445,  2446,  2447,  2448,  2449,  2450,  2451,  2452,  2453,  2454,
    2455,  2456,  2457,  2458,  2459,  2460,  2461,  2462,  2463,  2464,
    2465,  2466,  2467,  2468,  2469,  2470,  2471,  2472,  2473,  2474,
    2475,  2476,  2477,  2478,  2479,  2480,  2481,  2482,  2483,  2484,
    2485,  2486,  2487,  2488,  2489,  2490,  2491,  2492,  2493,  2494,
    2495,  2496,  2497,  2498,  2499,  2500,  2501,  2502,  2503,  2504,
    2505,  2506,  2507,  2508,  2509,  2510,  2511,  2512,  2513,  2514,
    2518,  2523,  2524,  2528,  2529,  2530,  2531,  2533,  2537,  2538,
    2549,  2550,  2552,  2554,  2566,  2567,  2568,  2572,  2573,  2574,
    2578,  2579,  2580,  2583,  2585,  2589,  2590,  2591,  2592,  2594,
    2595,  2596,  2597,  2598,  2599,  2600,  2601,  2602,  2603,  2606,
    2611,  2612,  2613,  2615,  2616,  2618,  2619,  2620,  2621,  2622,
    2623,  2624,  2625,  2626,  2628,  2630,  2632,  2634,  2636,  2637,
    2638,  2639,  2640,  2641,  2642,  2643,  2644,  2645,  2646,  2647,
    2648,  2649,  2650,  2651,  2652,  2654,  2656,  2658,  2660,  2661,
    2664,  2665,  2669,  2673,  2675,  2679,  2680,  2684,  2690,  2693,
    2697,  2698,  2699,  2700,  2701,  2702,  2703,  2708,  2710,  2714,
    2715,  2718,  2719,  2723,  2726,  2728,  2730,  2734,  2735,  2736,
    2737,  2740,  2744,  2745,  2746,  2747,  2751,  2753,  2760,  2761,
    2762,  2763,  2768,  2769,  2770,  2771,  2773,  2774,  2776,  2777,
    2778,  2779,  2780,  2784,  2786,  2790,  2792,  2795,  2798,  2800,
    2802,  2805,  2807,  2811,  2813,  2816,  2819,  2825,  2827,  2830,
    2831,  2836,  2839,  2843,  2843,  2848,  2851,  2852,  2856,  2857,
    2861,  2862,  2863,  2867,  2872,  2877,  2878,  2882,  2887,  2892,
    2893,  2897,  2898,  2903,  2905,  2910,  2921,  2935,  2947,  2962,
    2963,  2964,  2965,  2966,  2967,  2968,  2978,  2987,  2989,  2991,
    2995,  2996,  2997,  2998,  2999,  3015,  3016,  3018,  3020,  3027,
    3028,  3029,  3030,  3031,  3032,  3033,  3034,  3036,  3041,  3045,
    3046,  3050,  3053,  3060,  3064,  3073,  3080,  3088,  3090,  3091,
    3095,  3096,  3097,  3099,  3104,  3105,  3116,  3117,  3118,  3119,
    3130,  3133,  3136,  3137,  3138,  3139,  3150,  3154,  3155,  3156,
    3158,  3159,  3160,  3164,  3166,  3169,  3171,  3172,  3173,  3174,
    3177,  3179,  3180,  3184,  3186,  3189,  3191,  3192,  3193,  3197,
    3199,  3202,  3205,  3207,  3209,  3213,  3214,  3216,  3217,  3223,
    3224,  3226,  3236,  3238,  3240,  3243,  3244,  3245,  3249,  3250,
    3251,  3252,  3253,  3254,  3255,  3256,  3257,  3258,  3259,  3263,
    3264,  3268,  3270,  3278,  3280,  3284,  3288,  3293,  3297,  3305,
    3306,  3310,  3311,  3317,  3318,  3327,  3328,  3336,  3339,  3343,
    3346,  3351,  3356,  3358,  3359,  3360,  3363,  3365,  3371,  3372,
    3376,  3377,  3381,  3382,  3386,  3387,  3390,  3395,  3396,  3400,
    3403,  3405,  3409,  3415,  3416,  3417,  3421,  3425,  3433,  3438,
    3450,  3452,  3456,  3459,  3461,  3466,  3471,  3477,  3480,  3485,
    3490,  3492,  3499,  3502,  3505,  3506,  3509,  3512,  3513,  3518,
    3520,  3524,  3530,  3540,  3541
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
  "T_DOUBLE_ARROW", "T_LIST", "T_ARRAY", "T_DICT", "T_VEC", "T_VARRAY",
  "T_DARRAY", "T_KEYSET", "T_CALLABLE", "T_CLASS_C", "T_METHOD_C",
  "T_FUNC_C", "T_LINE", "T_FILE", "T_COMMENT", "T_DOC_COMMENT",
  "T_OPEN_TAG", "T_OPEN_TAG_WITH_ECHO", "T_CLOSE_TAG", "T_WHITESPACE",
  "T_START_HEREDOC", "T_END_HEREDOC", "T_DOLLAR_OPEN_CURLY_BRACES",
  "T_CURLY_OPEN", "T_DOUBLE_COLON", "T_NAMESPACE", "T_NS_C", "T_DIR",
  "T_NS_SEPARATOR", "T_XHP_LABEL", "T_XHP_TEXT", "T_XHP_ATTRIBUTE",
  "T_XHP_CATEGORY", "T_XHP_CATEGORY_LABEL", "T_XHP_CHILDREN", "T_ENUM",
  "T_XHP_REQUIRED", "T_TRAIT", "\"...\"", "T_INSTEADOF", "T_TRAIT_C",
  "T_HH_ERROR", "T_FINALLY", "T_XHP_TAG_LT", "T_XHP_TAG_GT",
  "T_TYPELIST_LT", "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION",
  "T_SHAPE", "T_TYPE", "T_UNRESOLVED_TYPE", "T_NEWTYPE",
  "T_UNRESOLVED_NEWTYPE", "T_COMPILER_HALT_OFFSET", "T_ASYNC",
  "T_LAMBDA_OP", "T_LAMBDA_CP", "T_UNRESOLVED_OP", "T_WHERE", "'('", "')'",
  "';'", "'{'", "'}'", "'$'", "'`'", "']'", "'\"'", "'\\''", "$accept",
  "start", "$@1", "top_statement_list", "top_statement", "$@2", "$@3",
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
  "static_keyset_literal", "static_keyset_literal_ae", "varray_literal",
  "static_varray_literal", "static_varray_literal_ae", "darray_literal",
  "static_darray_literal", "static_darray_literal_ae", "vec_ks_expr_list",
  "static_vec_ks_expr_list", "static_vec_ks_expr_list_ae",
  "collection_literal", "static_collection_literal", "dim_expr",
  "dim_expr_base", "lexical_var_list", "xhp_tag", "xhp_tag_body",
  "xhp_opt_end_label", "xhp_attributes", "xhp_children",
  "xhp_attribute_name", "xhp_attribute_value", "xhp_child", "xhp_label_ws",
  "xhp_bareword", "simple_function_call", "fully_qualified_class_name",
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
     426,   427,   428,   429,   430,   431,    40,    41,    59,   123,
     125,    36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   206,   208,   207,   209,   209,   210,   210,   210,   210,
     210,   210,   210,   210,   211,   210,   212,   210,   210,   210,
     210,   210,   210,   210,   210,   213,   213,   213,   213,   213,
     213,   213,   213,   213,   213,   213,   213,   213,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   215,   215,   216,   216,   217,   217,   218,   219,
     219,   219,   219,   220,   220,   221,   222,   222,   222,   223,
     223,   224,   224,   224,   225,   226,   227,   227,   228,   228,
     229,   229,   229,   229,   230,   230,   230,   231,   230,   232,
     230,   233,   230,   234,   230,   230,   230,   230,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     235,   230,   236,   230,   230,   237,   230,   238,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   230,   230,   240,   239,   241,   241,   243,   242,   244,
     244,   245,   245,   246,   248,   247,   249,   247,   250,   247,
     252,   251,   253,   251,   255,   254,   256,   254,   257,   254,
     258,   254,   260,   259,   262,   261,   263,   261,   264,   264,
     265,   266,   267,   267,   267,   267,   267,   268,   268,   269,
     269,   270,   270,   271,   271,   272,   272,   273,   273,   274,
     274,   274,   275,   275,   276,   276,   277,   277,   278,   278,
     279,   279,   280,   280,   280,   280,   281,   281,   281,   282,
     282,   283,   283,   284,   284,   285,   285,   286,   286,   287,
     287,   287,   287,   287,   287,   287,   287,   288,   288,   288,
     288,   288,   288,   288,   288,   289,   289,   289,   289,   289,
     289,   289,   289,   290,   290,   290,   290,   290,   290,   290,
     290,   291,   291,   292,   292,   292,   292,   292,   292,   293,
     293,   294,   294,   294,   295,   295,   295,   295,   296,   296,
     297,   298,   299,   299,   301,   300,   302,   300,   300,   300,
     300,   303,   300,   304,   300,   300,   300,   300,   300,   300,
     300,   300,   305,   305,   305,   306,   307,   307,   308,   308,
     309,   309,   310,   310,   311,   311,   312,   312,   312,   312,
     312,   312,   312,   313,   313,   314,   315,   315,   316,   316,
     317,   317,   318,   319,   319,   319,   320,   320,   320,   320,
     321,   321,   321,   321,   321,   321,   321,   322,   322,   322,
     323,   323,   324,   324,   325,   325,   326,   326,   327,   327,
     328,   328,   328,   328,   328,   328,   328,   329,   329,   330,
     330,   330,   331,   331,   331,   331,   332,   332,   333,   333,
     334,   334,   335,   336,   336,   336,   336,   336,   336,   337,
     338,   338,   339,   339,   340,   340,   340,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   348,   348,   348,   348,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   350,   350,   352,
     351,   353,   351,   355,   354,   356,   354,   357,   354,   358,
     354,   359,   354,   360,   360,   360,   361,   361,   362,   362,
     363,   363,   364,   364,   365,   365,   366,   367,   367,   368,
     368,   369,   369,   370,   370,   371,   371,   372,   372,   373,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,   389,   389,   390,   390,
     391,   391,   392,   393,   394,   394,   395,   395,   395,   395,
     395,   395,   395,   395,   395,   395,   395,   396,   396,   396,
     396,   397,   398,   398,   399,   399,   400,   400,   401,   401,
     402,   403,   403,   404,   404,   404,   405,   405,   405,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     407,   408,   408,   409,   409,   409,   409,   409,   410,   410,
     411,   411,   411,   411,   412,   412,   412,   413,   413,   413,
     414,   414,   414,   415,   415,   416,   416,   416,   416,   416,
     416,   416,   416,   416,   416,   416,   416,   416,   416,   416,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   418,   418,   419,   420,   420,
     421,   421,   421,   421,   421,   421,   421,   422,   422,   423,
     423,   424,   424,   425,   425,   425,   425,   426,   426,   426,
     426,   426,   427,   427,   427,   427,   428,   428,   429,   429,
     429,   429,   429,   429,   429,   429,   429,   429,   429,   429,
     429,   429,   429,   430,   430,   431,   431,   432,   432,   432,
     432,   433,   433,   434,   434,   435,   435,   436,   436,   437,
     437,   438,   438,   440,   439,   441,   442,   442,   443,   443,
     444,   444,   444,   445,   445,   446,   446,   447,   447,   448,
     448,   449,   449,   450,   450,   451,   451,   452,   452,   453,
     453,   453,   453,   453,   453,   453,   453,   453,   453,   453,
     454,   454,   454,   454,   454,   454,   454,   454,   454,   455,
     455,   455,   455,   455,   455,   455,   455,   455,   456,   457,
     457,   458,   458,   459,   459,   459,   460,   461,   461,   461,
     462,   462,   462,   462,   463,   463,   464,   464,   464,   464,
     464,   464,   465,   465,   465,   465,   465,   466,   466,   466,
     466,   466,   466,   467,   467,   468,   468,   468,   468,   468,
     468,   468,   468,   469,   469,   470,   470,   470,   470,   471,
     471,   472,   472,   472,   472,   473,   473,   473,   473,   474,
     474,   474,   474,   474,   474,   475,   475,   475,   476,   476,
     476,   476,   476,   476,   476,   476,   476,   476,   476,   477,
     477,   478,   478,   479,   479,   480,   480,   480,   480,   481,
     481,   482,   482,   483,   483,   484,   484,   485,   485,   486,
     486,   487,   488,   488,   488,   488,   489,   489,   490,   490,
     491,   491,   492,   492,   493,   493,   494,   495,   495,   496,
     496,   496,   496,   497,   497,   497,   498,   498,   498,   498,
     499,   499,   500,   500,   500,   500,   501,   502,   503,   503,
     504,   504,   505,   505,   505,   505,   505,   505,   505,   505,
     505,   505,   505,   506,   506
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
       1,     1,     1,     1,     2,     3,     3,     1,     2,     1,
       2,     3,     4,     3,     1,     2,     1,     2,     2,     1,
       3,     1,     3,     2,     2,     2,     5,     4,     2,     0,
       1,     1,     1,     1,     3,     5,     8,     0,     4,     0,
       6,     0,    10,     0,     4,     2,     3,     2,     3,     2,
       3,     3,     3,     3,     3,     3,     5,     1,     1,     1,
       0,     9,     0,    10,     5,     0,    13,     0,     5,     3,
       3,     2,     2,     2,     2,     2,     2,     3,     2,     2,
       3,     2,     2,     0,     4,     9,     0,     0,     4,     2,
       0,     1,     0,     1,     0,     9,     0,    10,     0,    11,
       0,     9,     0,    10,     0,     8,     0,     9,     0,     7,
       0,     8,     0,     8,     0,     7,     0,     8,     1,     1,
       1,     1,     1,     2,     3,     3,     2,     2,     0,     2,
       0,     2,     0,     1,     3,     1,     3,     2,     0,     1,
       2,     4,     1,     4,     1,     4,     1,     4,     1,     4,
       3,     5,     3,     4,     4,     5,     5,     4,     0,     1,
       1,     4,     0,     5,     0,     2,     0,     3,     0,     7,
       8,     6,     2,     5,     6,     4,     0,     4,     5,     7,
       6,     6,     7,     9,     8,     6,     7,     5,     2,     4,
       5,     3,     0,     3,     4,     6,     5,     5,     6,     8,
       7,     2,     0,     1,     2,     2,     3,     4,     4,     3,
       1,     1,     2,     4,     3,     5,     1,     3,     2,     0,
       2,     3,     2,     0,     0,     4,     0,     5,     2,     2,
       2,     0,    11,     0,    12,     3,     3,     3,     4,     4,
       3,     5,     2,     2,     0,     6,     5,     4,     3,     1,
       1,     3,     4,     1,     2,     1,     1,     5,     6,     1,
       1,     4,     1,     1,     3,     2,     2,     0,     2,     0,
       1,     3,     1,     1,     1,     1,     3,     4,     4,     4,
       1,     1,     2,     2,     2,     3,     3,     1,     1,     1,
       1,     3,     1,     3,     1,     1,     1,     0,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       1,     1,     3,     5,     1,     3,     5,     4,     3,     3,
       3,     4,     3,     3,     3,     2,     2,     1,     1,     3,
       3,     1,     1,     0,     1,     2,     4,     3,     3,     6,
       2,     3,     2,     3,     6,     1,     1,     1,     1,     1,
       6,     3,     4,     6,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     4,     3,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     1,     5,     0,     0,
      12,     0,    13,     0,     4,     0,     7,     0,     5,     0,
       3,     0,     6,     2,     2,     4,     1,     1,     5,     3,
       5,     3,     2,     0,     2,     0,     4,     4,     3,     2,
       0,     5,     3,     2,     0,     5,     3,     2,     0,     5,
       3,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     2,     0,     2,     0,
       2,     0,     4,     4,     4,     4,     1,     1,     1,     1,
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
       1,     1,     1,     3,     1,     1,     1,     2,     1,     0,
       0,     1,     1,     3,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     2,
       1,     1,     4,     3,     4,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     4,     3,     1,     3,     3,     1,
       1,     1,     1,     1,     3,     3,     3,     2,     0,     1,
       0,     1,     0,     5,     3,     3,     1,     1,     1,     1,
       3,     2,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     3,     1,     2,     2,     4,     3,     4,     1,     1,
       1,     1,     1,     3,     1,     2,     0,     5,     3,     3,
       1,     3,     1,     2,     0,     5,     3,     2,     0,     3,
       0,     4,     2,     0,     3,     3,     1,     0,     1,     1,
       1,     1,     3,     1,     1,     1,     3,     1,     1,     3,
       3,     2,     4,     2,     4,     5,     5,     5,     5,     1,
       1,     1,     1,     1,     1,     3,     3,     4,     4,     3,
       1,     1,     1,     1,     3,     1,     4,     3,     3,     1,
       1,     1,     1,     1,     3,     3,     4,     4,     3,     1,
       1,     7,     9,     7,     6,     8,     1,     4,     4,     1,
       1,     1,     4,     2,     1,     0,     1,     1,     1,     3,
       3,     3,     0,     1,     1,     3,     3,     2,     3,     6,
       0,     1,     4,     2,     0,     5,     3,     3,     1,     6,
       4,     4,     2,     2,     0,     5,     3,     3,     1,     2,
       0,     5,     3,     3,     1,     2,     2,     1,     2,     1,
       4,     3,     3,     6,     3,     1,     1,     1,     4,     4,
       4,     4,     4,     4,     2,     2,     4,     2,     2,     1,
       3,     3,     3,     0,     2,     5,     6,     6,     7,     1,
       2,     1,     2,     1,     4,     1,     4,     3,     0,     1,
       3,     2,     3,     1,     1,     0,     0,     3,     1,     3,
       3,     2,     0,     2,     2,     2,     2,     1,     2,     4,
       2,     5,     3,     1,     1,     0,     3,     4,     5,     6,
       3,     1,     3,     2,     1,     0,     4,     1,     3,     2,
       4,     5,     2,     2,     1,     1,     1,     1,     3,     2,
       1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   434,     0,     0,   863,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   954,
       0,   942,   729,     0,   735,   736,   737,    25,   801,   930,
     931,   158,   159,   738,     0,   139,     0,     0,     0,     0,
      26,     0,     0,     0,     0,   193,     0,     0,     0,     0,
       0,     0,   400,   401,   402,   405,   404,   403,     0,     0,
       0,     0,   222,     0,     0,     0,    33,    34,    36,    37,
      35,   742,   744,   745,   739,   740,     0,     0,     0,   746,
     741,     0,   712,    28,    29,    30,    32,    31,     0,   743,
       0,     0,     0,     0,   747,   406,   541,    27,     0,   157,
     129,     0,   730,     0,     0,     4,   119,   121,   800,     0,
     711,     0,     6,   192,     7,     9,     8,    10,     0,     0,
     398,   447,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   445,   919,   920,   523,   517,   518,   519,   520,   521,
     522,   428,   526,     0,   427,   890,   713,   720,     0,   803,
     516,   397,   893,   894,   905,   446,     0,     0,   449,   448,
     891,   892,   889,   926,   929,   506,   802,    11,   405,   404,
     403,     0,     0,    32,     0,   119,   192,     0,   998,   446,
     997,     0,   995,   994,   525,     0,   435,   442,   440,     0,
       0,   488,   489,   490,   491,   515,   513,   512,   511,   510,
     509,   508,   507,    25,   930,   738,   715,    33,    34,    36,
      37,    35,     0,     0,  1018,   912,   713,     0,   714,   469,
       0,   467,     0,   958,     0,   810,   426,   725,   212,     0,
    1018,   425,   724,   718,     0,   734,   714,   937,   938,   944,
     936,   726,     0,     0,   728,   514,     0,     0,     0,     0,
     431,     0,   137,   433,     0,     0,   143,   145,     0,     0,
     147,     0,    78,    77,    72,    71,    63,    64,    55,    75,
      86,    87,     0,    58,     0,    70,    62,    68,    89,    81,
      80,    53,    76,    96,    97,    54,    92,    51,    93,    52,
      94,    50,    98,    85,    90,    95,    82,    83,    57,    84,
      88,    49,    79,    65,    99,    73,    66,    56,    48,    47,
      46,    45,    44,    43,    67,   101,   100,   103,    60,    41,
      42,    69,  1065,  1066,    61,  1070,    40,    59,    91,     0,
       0,   119,   102,  1009,  1064,     0,  1067,     0,     0,   149,
       0,     0,     0,   183,     0,     0,     0,     0,     0,     0,
     812,     0,   107,   109,   311,     0,     0,   310,     0,   226,
       0,   223,   316,     0,     0,     0,     0,     0,  1015,   208,
     220,   950,   954,   560,   587,   587,   560,   587,     0,   979,
       0,   749,     0,     0,     0,   977,     0,    16,     0,   123,
     200,   214,   221,   617,   553,     0,  1003,   533,   535,   537,
     867,   434,   447,     0,     0,   445,   446,   448,     0,     0,
     933,   731,     0,   732,     0,     0,     0,   182,     0,     0,
     125,   302,     0,    24,   191,     0,   219,   204,   218,   403,
     406,   192,   399,   172,   173,   174,   175,   176,   178,   179,
     181,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   942,
       0,   171,   935,   935,   964,     0,     0,     0,     0,     0,
       0,     0,     0,   396,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   468,   466,   868,
     869,     0,   935,     0,   881,   302,   302,   935,     0,   950,
       0,   192,     0,     0,   151,     0,   865,   860,   810,     0,
     447,   445,     0,   962,     0,   558,   809,   953,   734,   447,
     445,   446,   125,     0,   302,   424,     0,   883,   727,     0,
     129,   262,     0,   540,     0,   154,     0,     0,   432,     0,
       0,     0,     0,     0,   146,   170,   148,  1065,  1066,  1062,
    1063,     0,  1069,  1055,     0,     0,     0,     0,    74,    39,
      61,    38,  1010,   177,   180,   150,   129,     0,   167,   169,
       0,     0,     0,     0,   110,     0,   811,   108,    18,     0,
     104,     0,   312,     0,   152,   225,   224,     0,     0,   153,
     999,     0,     0,   447,   445,   446,   449,   448,     0,  1045,
     232,     0,   951,     0,     0,     0,     0,   810,   810,     0,
       0,     0,     0,   155,     0,     0,   748,   978,   801,     0,
       0,   976,   806,   975,   122,     5,    13,    14,     0,   230,
       0,     0,   546,     0,     0,   810,     0,   722,     0,   721,
     716,   547,     0,     0,     0,     0,   867,   129,     0,   812,
     866,  1074,   423,   437,   502,   899,   918,   134,   128,   130,
     131,   132,   133,   397,     0,   524,   804,   805,   120,   810,
       0,  1019,     0,     0,     0,   812,   303,     0,   529,   194,
     228,     0,   472,   474,   473,   485,     0,     0,   505,   470,
     471,   475,   477,   476,   493,   492,   495,   494,   496,   498,
     500,   499,   497,   487,   486,   479,   480,   478,   481,   482,
     484,   501,   483,   934,     0,     0,   968,     0,   810,  1002,
       0,  1001,  1018,   896,   210,   202,   216,     0,  1003,   206,
     192,     0,   438,   441,   443,   451,   465,   464,   463,   462,
     461,   460,   459,   458,   457,   456,   455,   454,   871,     0,
     870,   873,   895,   877,  1018,   874,     0,     0,     0,     0,
       0,     0,     0,     0,   996,   436,   858,   862,   809,   864,
       0,   717,     0,   957,     0,   956,   228,     0,   717,   941,
     940,   926,   929,     0,     0,   870,   873,   939,   874,   429,
     264,   266,   129,   544,   543,   430,     0,   129,   246,   138,
     433,     0,     0,     0,     0,     0,   258,   258,   144,   810,
       0,     0,  1054,     0,  1051,   810,     0,  1025,     0,     0,
       0,     0,     0,   808,     0,    33,    34,    36,    37,    35,
       0,     0,   751,   755,   756,   757,   758,   759,   761,     0,
     750,   127,   799,   760,  1018,  1068,     0,     0,     0,     0,
      19,     0,    20,     0,   105,     0,     0,     0,   116,   812,
       0,   114,   109,   106,   111,     0,   309,   317,   314,     0,
       0,   988,   993,   990,   989,   992,   991,    12,  1043,  1044,
       0,   810,     0,     0,     0,   950,   947,     0,   557,     0,
     571,   809,   559,   809,   586,   574,   580,   583,   577,   987,
     986,   985,     0,   981,     0,   982,   984,     0,     5,     0,
       0,     0,   611,   612,   620,   619,     0,   445,     0,   809,
     552,   556,     0,     0,  1004,     0,   534,     0,     0,  1032,
     867,   288,  1073,     0,     0,   882,     0,   932,   809,  1021,
    1017,   304,   305,   710,   811,   301,     0,   867,     0,     0,
     230,   531,   196,   504,     0,   594,   595,     0,   592,   809,
     963,     0,     0,   302,   232,     0,   230,     0,     0,   228,
       0,   942,   452,     0,     0,   879,   880,   897,   898,   927,
     928,     0,     0,     0,   846,   817,   818,   819,   826,     0,
      33,    34,    36,    37,    35,     0,     0,   832,   838,   839,
     840,   841,   842,     0,   830,   828,   829,   852,   810,     0,
     860,   961,   960,     0,   230,     0,   884,   733,     0,   268,
       0,     0,   135,     0,     0,     0,     0,     0,     0,     0,
     238,   239,   250,     0,   129,   248,   164,   258,     0,   258,
       0,   809,     0,     0,     0,     0,     0,   809,  1053,  1056,
    1024,   810,  1023,     0,   810,   782,   783,   780,   781,   816,
       0,   810,   808,   564,   589,   589,   564,   589,   555,     0,
       0,   970,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1059,   184,     0,   187,   168,     0,     0,   112,   117,   118,
     110,   811,   115,     0,   313,     0,  1000,   156,  1016,  1045,
    1036,  1040,   231,   233,   323,     0,     0,   948,     0,   562,
       0,   980,     0,    17,     0,  1003,   229,   323,     0,     0,
     717,   549,     0,   723,  1005,     0,  1032,   538,     0,     0,
    1074,     0,   293,   291,   873,   885,  1018,   873,   886,  1020,
       0,     0,   306,   126,     0,   867,   227,     0,   867,     0,
     503,   967,   966,     0,   302,     0,     0,     0,     0,     0,
       0,   230,   198,   734,   872,   302,     0,   822,   823,   824,
     825,   833,   834,   850,     0,   810,     0,   846,   568,   591,
     591,   568,   591,     0,   821,   854,     0,   809,   857,   859,
     861,     0,   955,     0,   872,     0,     0,     0,     0,   265,
     545,   140,     0,   433,   238,   240,   950,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   252,     0,  1060,     0,
       0,  1046,     0,  1052,  1050,   809,     0,     0,     0,   753,
     809,   807,     0,     0,   810,     0,     0,   796,   810,     0,
       0,     0,     0,   810,     0,   762,   797,   798,   974,     0,
     810,   765,   767,   766,     0,     0,   763,   764,   768,   770,
     769,   785,   784,   787,   786,   788,   790,   792,   791,   789,
     778,   777,   772,   773,   771,   774,   775,   776,   779,  1058,
       0,   129,     0,     0,   113,    21,   315,     0,     0,     0,
    1037,  1042,     0,   397,   952,   950,   439,   444,   450,     0,
       0,    15,     0,   397,   623,     0,     0,   625,   618,   621,
       0,   616,     0,  1007,     0,  1033,   542,     0,   294,     0,
       0,   289,     0,   308,   307,  1032,     0,   323,     0,   867,
       0,   302,     0,   924,   323,  1003,   323,  1006,     0,     0,
       0,   453,     0,     0,   836,   809,   845,   827,     0,     0,
     810,     0,     0,   844,   810,     0,     0,     0,   820,     0,
       0,   810,   831,   851,   959,   323,     0,   129,     0,   261,
     247,     0,     0,     0,   237,   160,   251,     0,     0,   254,
       0,   259,   260,   129,   253,  1061,  1047,     0,     0,  1022,
       0,  1072,   815,   814,   752,   572,   809,   563,     0,   575,
     809,   588,   581,   584,   578,     0,   809,   554,   754,     0,
     593,   809,   969,   794,     0,     0,     0,    22,    23,  1039,
    1034,  1035,  1038,   234,     0,     0,     0,   404,   395,     0,
       0,     0,   209,   322,   324,     0,   394,     0,     0,     0,
    1003,   397,     0,   561,   983,   319,   215,   614,     0,     0,
     548,   536,     0,   297,   287,     0,   290,   296,   302,   528,
    1032,   397,  1032,     0,   965,     0,   923,   397,     0,   397,
    1008,   323,   867,   921,   849,   848,   835,   573,   809,   567,
       0,   576,   809,   590,   582,   585,   579,     0,   837,   809,
     853,   397,   129,   267,   136,   141,   162,   241,     0,   249,
     255,   129,   257,     0,  1048,     0,     0,     0,   566,   795,
     551,     0,   973,   972,   793,   129,   188,  1041,     0,     0,
       0,  1011,     0,     0,     0,   235,     0,  1003,     0,   360,
     356,   362,   712,    32,     0,   350,     0,   355,   359,   372,
       0,   370,   375,     0,   374,     0,   373,     0,   192,   326,
       0,   328,     0,   329,   330,     0,     0,   949,     0,   615,
     613,   624,   622,   298,     0,     0,   285,   295,     0,     0,
    1032,     0,   205,   528,  1032,   925,   211,   319,   217,   397,
       0,     0,     0,   570,   843,   856,     0,   213,   263,     0,
       0,   129,   244,   161,   256,  1049,  1071,   813,     0,     0,
       0,     0,     0,     0,   422,     0,  1012,     0,   340,   344,
     419,   420,   354,     0,     0,     0,   335,   676,   675,   672,
     674,   673,   693,   695,   694,   664,   634,   636,   635,   654,
     670,   669,   630,   641,   642,   644,   643,   663,   647,   645,
     646,   648,   649,   650,   651,   652,   653,   655,   656,   657,
     658,   659,   660,   662,   661,   631,   632,   633,   637,   638,
     640,   678,   679,   688,   687,   686,   685,   684,   683,   671,
     690,   680,   681,   682,   665,   666,   667,   668,   691,   692,
     696,   698,   697,   699,   700,   677,   702,   701,   704,   706,
     705,   639,   709,   707,   708,   703,   689,   629,   367,   626,
       0,   336,   388,   389,   387,   380,     0,   381,   337,   414,
       0,     0,     0,     0,   418,     0,   192,   201,   318,     0,
       0,     0,   286,   300,   922,     0,     0,   390,   129,   195,
    1032,     0,     0,   207,  1032,   847,     0,     0,   129,   242,
     142,   163,     0,   565,   550,   971,   186,   338,   339,   417,
     236,     0,   810,   810,     0,   363,   351,     0,     0,     0,
     369,   371,     0,     0,   376,   383,   384,   382,     0,     0,
     325,  1013,     0,     0,     0,   421,     0,   320,     0,   299,
       0,   609,   812,   129,     0,     0,   197,   203,     0,   569,
     855,     0,     0,   165,   341,   119,     0,   342,   343,     0,
     809,     0,   809,   365,   361,   366,   627,   628,     0,   352,
     385,   386,   378,   379,   377,   415,   412,  1045,   331,   327,
     416,     0,   321,   610,   811,     0,     0,   391,   129,   199,
       0,   245,     0,   190,     0,   397,     0,   357,   364,   368,
       0,     0,   867,   333,     0,   607,   527,   530,     0,   243,
       0,     0,   166,   348,     0,   396,   358,   413,  1014,     0,
     812,   408,   867,   608,   532,     0,   189,     0,     0,   347,
    1032,   867,   272,   409,   410,   411,  1074,   407,     0,     0,
       0,   346,  1026,   408,     0,  1032,     0,   345,     0,     0,
    1074,     0,   277,   275,  1026,   129,   812,  1028,     0,   392,
     129,   332,     0,   278,     0,     0,   273,     0,     0,   811,
    1027,     0,  1031,     0,     0,   281,   271,     0,   274,   280,
     334,   185,  1029,  1030,   393,   282,     0,     0,   269,   279,
       0,   270,   284,   283
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   115,   928,   645,   185,  1561,   742,
     359,   360,   361,   362,   879,   880,   881,   117,   118,   119,
     120,   121,   418,   678,   679,   557,   261,  1629,   563,  1538,
    1630,  1873,   868,   354,   586,  1833,  1124,  1321,  1892,   435,
     186,   680,   968,  1189,  1380,   125,   648,   985,   681,   700,
     989,   620,   984,   241,   538,   682,   649,   986,   437,   379,
     401,   128,   970,   931,   904,  1142,  1564,  1248,  1050,  1780,
    1633,   819,  1056,   562,   828,  1058,  1423,   811,  1039,  1042,
    1237,  1899,  1900,   668,   669,   694,   695,   366,   367,   373,
    1598,  1758,  1759,  1333,  1473,  1587,  1752,  1882,  1902,  1791,
    1837,  1838,  1839,  1574,  1575,  1576,  1577,  1793,  1794,  1800,
    1849,  1580,  1581,  1585,  1745,  1746,  1747,  1769,  1941,  1474,
    1475,   187,   130,  1916,  1917,  1750,  1477,  1478,  1479,  1480,
     131,   254,   558,   559,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,  1610,   142,   967,  1188,   143,   665,
     666,   667,   258,   410,   553,   654,   655,  1283,   656,  1284,
     144,   145,   626,   627,  1273,  1274,  1389,  1390,   146,   853,
    1018,   147,   854,  1019,   148,   855,  1020,   149,   856,  1021,
     150,   857,  1022,   629,  1276,  1392,   151,   858,   152,   153,
    1822,   154,   650,  1600,   651,  1158,   936,  1351,  1348,  1738,
    1739,   155,   156,   157,   244,   158,   245,   255,   422,   545,
     159,  1277,  1278,   862,   863,   160,  1080,   959,   597,  1081,
    1025,  1211,  1026,  1393,  1394,  1214,  1215,  1028,  1400,  1401,
    1029,   787,   528,   199,   200,   683,   671,   511,  1174,  1175,
     773,   774,   955,   162,   247,   163,   164,   189,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   734,   251,   252,
     623,   234,   235,   737,   738,  1289,  1290,   394,   395,   922,
     175,   611,   176,   664,   177,   345,  1760,  1812,   380,   430,
     689,   690,  1073,  1929,  1936,  1937,  1169,  1330,   900,  1331,
     901,   902,   834,   835,   836,   346,   347,   865,   572,  1563,
     953
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1384
static const yytype_int16 yypact[] =
{
   -1384,   154, -1384, -1384,  5688, 13605, 13605,    -8, 13605, 13605,
   13605, 11372, 13605, 13605, -1384, 13605, 13605, 13605, 13605, 13605,
   13605, 13605, 13605, 13605, 13605, 13605, 13605, 16930, 16930, 11575,
   13605, 17595,     9,   217, -1384, -1384, -1384,   368, -1384,   281,
   -1384, -1384, -1384,   374, 13605, -1384,   217,   221,   309,   322,
   -1384,   217, 11778,  4150, 11981, -1384, 14630, 10560,   283, 13605,
    4282,    52, -1384, -1384, -1384,   448,   286,    68,   349,   372,
     381,   387, -1384,  4150,   404,   411,   547,   553,   566,   568,
     571, -1384, -1384, -1384, -1384, -1384, 13605,    72,  1931, -1384,
   -1384,  4150, -1384, -1384, -1384, -1384,  4150, -1384,  4150, -1384,
     449,   465,  4150,  4150, -1384,   338, -1384, -1384, 12184, -1384,
   -1384,   326,   328,   535,   535, -1384,   641,   525,   234,   489,
   -1384,    84, -1384,   653, -1384, -1384, -1384, -1384, 14174,   548,
   -1384, -1384,   531,   536,   540,   552,   559,   563,   577,   579,
   15812, -1384, -1384, -1384, -1384,   157,   675,   725,   733,   739,
     750, -1384,   752,   770, -1384,   169,   524, -1384,   624,   231,
   -1384,   714,   142, -1384, -1384,  2973,   150,   621,   180, -1384,
     152,    86,   665,   185, -1384, -1384,   779, -1384, -1384, -1384,
     702,   667,   700, -1384, 13605, -1384,   653,   548, 18015,  3038,
   18015, 13605, 18015, 18015, 18234,   668, 16247, 18234, 18015,   819,
    4150,   800,   800,   335,   800,   800,   800,   800,   800,   800,
     800,   800,   800, -1384, -1384, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384,    75, 13605,   690, -1384, -1384,   712,   681,    58,
     682,    58, 16930, 16490,   677,   870, -1384,   702, -1384, 13605,
     690, -1384,   721, -1384,   722,   687, -1384,   163, -1384, -1384,
   -1384,    58,   150, 12387, -1384, -1384, 13605,  9139,   876,    92,
   18015, 10154, -1384, 13605, 13605,  4150, -1384, -1384, 15860,   688,
   -1384, 15908, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384,  4299, -1384,  4299, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384,    99,    93,   700, -1384, -1384, -1384, -1384,   696,
    3493,   100, -1384, -1384,   728,   881, -1384,   730, 15349, -1384,
     699,   701, 15978, -1384,    28, 16026, 14877, 14877,  4150,   704,
     892,   709, -1384,    73, -1384, 16518,   101, -1384,   780, -1384,
     781, -1384,   895,   107, 16930, 13605, 13605,   715,   737, -1384,
   -1384, 16621, 11575, 13605, 13605, 13605, 13605, 13605,   109,   468,
     562, -1384, 13808, 16930,   473, -1384,  4150, -1384,   482,   525,
   -1384, -1384, -1384, -1384, 17695,   900,   823, -1384, -1384, -1384,
      77, 13605,   729,   738, 18015,   740,  1704,   741,  5891, 13605,
   -1384,    83,   732,   573,    83,   475,   461, -1384,  4150,  4299,
     735,  4845, 14630, -1384, -1384,  3018, -1384, -1384, -1384, -1384,
   -1384,   653, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384,
   -1384, 13605, 13605, 13605, 13605, 12590, 13605, 13605, 13605, 13605,
   13605, 13605, 13605, 13605, 13605, 13605, 13605, 13605, 13605, 13605,
   13605, 13605, 13605, 13605, 13605, 13605, 13605, 13605, 13605, 17795,
   13605, -1384, 13605, 13605, 13605, 13979,  4150,  4150,  4150,  4150,
    4150, 14174,   825,   669, 10357, 13605, 13605, 13605, 13605, 13605,
   13605, 13605, 13605, 13605, 13605, 13605, 13605, -1384, -1384, -1384,
   -1384,  3589, 13605, 13605, -1384,  4845,  4845, 13605, 13605, 16621,
     743,   653, 12793, 16074, -1384, 13605, -1384,   744,   921,   784,
     748,   751, 14126,    58, 12996, -1384, 13199, -1384,   687,   753,
     755,  2435, -1384,   132,  4845, -1384,  3991, -1384, -1384, 16144,
   -1384, -1384, 10763, -1384, 13605, -1384,   850,  9342,   940,   756,
   13792,   941,    82,    85, -1384, -1384, -1384,   785, -1384, -1384,
   -1384,  4299, -1384,  1445,   764,   952, 16415,  4150, -1384, -1384,
   -1384, -1384, -1384, -1384, -1384, -1384, -1384,   766, -1384, -1384,
     768,   772,   769,   775,    88,  4604, 15243, -1384, -1384,  4150,
    4150, 13605,    58,    52, -1384, -1384, -1384, 16415,   888, -1384,
      58,   106,   128,   782,   783,  2651,   206,   786,   789,   602,
     845,   788,    58,   129,   798, 17099,   778,   973,   987,   794,
     795,   796,   803, -1384,  2326,  4150, -1384, -1384,   931,  3313,
      49, -1384, -1384, -1384,   525, -1384, -1384, -1384,   971,   877,
     829,   240,   852, 13605,   878,  1004,   818, -1384,   857, -1384,
     168, -1384,  4299,  4299,  1011,   876,    77, -1384,   834,  1021,
   -1384,  4299,    64, -1384,   446,   143, -1384, -1384, -1384, -1384,
   -1384, -1384, -1384,   936,  3969, -1384, -1384, -1384, -1384,  1023,
     854, -1384, 16930, 13605,   840,  1031, 18015,  1027, -1384, -1384,
     910,  3544, 11966, 18153, 18234,  4530, 13605, 17967, 14453, 12772,
   13177, 13977,  5166, 14799, 14982, 14982, 14982, 14982,  3538,  3538,
    3538,  3538,  3538,   969,   969,   747,   747,   747,   335,   335,
     335, -1384,   800, 18015,   842,   844, 17147,   849,  1045,     1,
   13605,     4,   690,   190, -1384, -1384, -1384,  1048,   823, -1384,
     653, 16724, -1384, -1384, -1384, 18234, 18234, 18234, 18234, 18234,
   18234, 18234, 18234, 18234, 18234, 18234, 18234, 18234, -1384, 13605,
      13, -1384,   174, -1384,   690,   307,   861,  4169,   868,   871,
     866,  4774,   130,   874, -1384, 18015,  3979, -1384,  4150, -1384,
      64,    29, 16930, 18015, 16930, 17203,   910,    64,    58,   186,
   -1384,   168,   912,   879, 13605, -1384,   191, -1384, -1384, -1384,
    8936,   551, -1384, -1384, 18015, 18015,   217, -1384, -1384, -1384,
   13605,   972, 16291, 16415,  4150,  9545,   880,   883, -1384,  1068,
    4963,   943, -1384,   922, -1384,  1078,   891,  3666,  4299, 16415,
   16415, 16415, 16415, 16415,   893,  1024,  1025,  1028,  1029,  1030,
     901, 16415,   479, -1384, -1384, -1384, -1384, -1384, -1384,   -10,
   -1384, 18109, -1384, -1384,    14, -1384,  6094, 15054,   903, 15243,
   -1384, 15243, -1384,  4150,  4150, 15243, 15243,  4150, -1384,  1082,
     904, -1384,   310, -1384, -1384,  5041, -1384, 18109,  1091, 16930,
     909, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384,
     928,  1102,  4150, 15054,   913, 16621, 16827,  1099, -1384, 13605,
   -1384, 13605, -1384, 13605, -1384, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384,   911, -1384, 13605, -1384, -1384,  5282, -1384,  4299,
   15054,   916, -1384, -1384, -1384, -1384,  1103,   919, 13605, 17695,
   -1384, -1384, 13979,   920, -1384,  4299, -1384,   926,  6297,  1088,
      50, -1384, -1384,    90,  3589, -1384,  3991, -1384,  4299, -1384,
   -1384,    58, 18015, -1384, 10966, -1384, 16415,    76,   927, 15054,
     877, -1384, -1384, 14453, 13605, -1384, -1384, 13605, -1384, 13605,
   -1384,  5134,   929,  4845,   845,  1090,   877,  4299,  1112,   910,
    4150, 17795,    58,  5214,   932, -1384, -1384,   144,   933, -1384,
   -1384,  1117,  2404,  2404,  3979, -1384, -1384, -1384,  1084,   938,
    1071,  1072,  1075,  1079,  1080,   282,   942,   598, -1384, -1384,
   -1384, -1384, -1384,   993, -1384, -1384, -1384, -1384,  1146,   960,
     744,    58,    58, 13402,   877,  3991, -1384, -1384, 11356,   627,
     217, 10154, -1384,  6500,   962,  6703,   964, 16291, 16930,   974,
    1022,    58, 18109,  1151, -1384, -1384, -1384, -1384,   620, -1384,
      54,  4299,   992,  1041,  1026,  4299,  4150,  3311, -1384, -1384,
   -1384,  1173, -1384,   986,  1023,   694,   694,  1116,  1116, 17355,
     984,  1181, 16415, 16415, 16415, 16415, 16415, 16415, 17695,  3101,
   15496, 16415, 16415, 16415, 16415, 16172, 16415, 16415, 16415, 16415,
   16415, 16415, 16415, 16415, 16415, 16415, 16415, 16415, 16415, 16415,
   16415, 16415, 16415, 16415, 16415, 16415, 16415, 16415, 16415,  4150,
   -1384, -1384,  1109, -1384, -1384,   994,   995, -1384, -1384, -1384,
     441,  4604, -1384,   998, -1384, 16415,    58, -1384, -1384,   119,
   -1384,   609,  1188, -1384, -1384,   137,  1002,    58, 11169, 18015,
   17251, -1384,  2916, -1384,  5485,   823,  1188, -1384,   445,   208,
   -1384, 18015,  1074,  1003, -1384,  1009,  1088, -1384,  4299,   876,
    4299,    47,  1194,  1129,   197, -1384,   690,   201, -1384, -1384,
   16930, 13605, 18015, 18109,  1016,    76, -1384,  1017,    76,  1019,
   14453, 18015, 17307,  1032,  4845,  1035,  1020,  4299,  1037,  1042,
    4299,   877, -1384,   687,   365,  4845, 13605, -1384, -1384, -1384,
   -1384, -1384, -1384,  1089,  1015,  1211,  1135,  3979,  3979,  3979,
    3979,  3979,  3979,  1085, -1384, 17695,   312,  3979, -1384, -1384,
   -1384, 16930, 18015,  1040, -1384,   217,  1209,  1167, 10154, -1384,
   -1384, -1384,  1047, 13605,  1022,    58, 16621, 16291,  1049, 16415,
    6906,   676,  1051, 13605,    79,    96, -1384,  1065, -1384,  4299,
    4150, -1384,  1114, -1384, -1384,  3714,  1219,  1055, 16415, -1384,
   16415, -1384,  1056,  1052,  1247, 17410,  1054, 18109,  1250,  1058,
    1062,  1064,  1132,  1260,  1073, -1384, -1384, -1384, 17458,  1076,
    1262,  3880, 18197, 10946, 16415, 18063, 12975, 13380, 14625, 12566,
   15348, 16356, 16356, 16356, 16356,  5068,  5068,  5068,  5068,  5068,
     773,   773,   694,   694,   694,  1116,  1116,  1116,  1116, -1384,
    1083, -1384,  1077,  1081, -1384, -1384, 18109,  4150,  4299,  4299,
   -1384,   609, 15054,  1307, -1384, 16621, -1384, -1384, 18234, 13605,
    1086, -1384,  1092,  1861, -1384,   165, 13605, -1384, -1384, -1384,
   13605, -1384, 13605, -1384,   876, -1384, -1384,    94,  1258,  1190,
   13605, -1384,  1087,    58, 18015,  1088,  1095, -1384,  1100,    76,
   13605,  4845,  1104, -1384, -1384,   823, -1384, -1384,  1101,  1108,
    1093, -1384,  1106,  3979, -1384,  3979, -1384, -1384,  1111,  1107,
    1268,  1145,  1113, -1384,  1273,  1115,  1118,  1119, -1384,  1148,
    1120,  1278, -1384, -1384,    58, -1384,  1282, -1384,  1122, -1384,
   -1384,  1126,  1127,   138, -1384, -1384, 18109,  1130,  1131, -1384,
   13589, -1384, -1384, -1384, -1384, -1384, -1384,  1179,  4299, -1384,
    4299, -1384, 18109, 17513, -1384, -1384, 16415, -1384, 16415, -1384,
   16415, -1384, -1384, -1384, -1384, 16415, 17695, -1384, -1384, 16415,
   -1384, 16415, -1384, 11555, 16415,  1128,  7109, -1384, -1384,   609,
   -1384, -1384, -1384, -1384,   584, 14806, 15054,  1213, -1384,  2484,
    1161,  4144, -1384, -1384, -1384,   825,  3377,   110,   111,  1133,
     823,   669,   139, 18015, -1384, -1384, -1384,  1171, 11762, 12371,
   18015, -1384,    61,  1318,  1254, 13605, -1384, 18015,  4845,  1222,
    1088,  2063,  1088,  1143, 18015,  1147, -1384,  2078,  1149,  2165,
   -1384, -1384,    76, -1384, -1384,  1214, -1384, -1384,  3979, -1384,
    3979, -1384,  3979, -1384, -1384, -1384, -1384,  3979, -1384, 17695,
   -1384,  2185, -1384,  8936, -1384, -1384, -1384, -1384,  9748, -1384,
   -1384, -1384,  8936,  4299, -1384,  1154, 16415, 17561, 18109, 18109,
   18109,  1216, 18109, 17616, 11555, -1384, -1384,   609, 15054, 15054,
    4150, -1384,  1339, 15643,    89, -1384, 14806,   823,  4559, -1384,
    1176, -1384,   112,  1159,   113, -1384, 15159, -1384, -1384, -1384,
     114, -1384, -1384,  2636, -1384,  1169, -1384,  1285,   653, -1384,
   14983, -1384, 14983, -1384, -1384,  1356,   825, -1384, 14278, -1384,
   -1384, -1384, -1384,  1358,  1292, 13605, -1384, 18015,  1182,  1184,
    1088,   544, -1384,  1222,  1088, -1384, -1384, -1384, -1384,  2202,
    1185,  3979,  1245, -1384, -1384, -1384,  1248, -1384,  8936,  9951,
    9748, -1384, -1384, -1384,  8936, -1384, -1384, 18109, 16415, 16415,
   16415,  7312,  1187,  1189, -1384, 16415, -1384, 15054, -1384, -1384,
   -1384, -1384, -1384,  4299,  1215,  2484, -1384, -1384, -1384, -1384,
   -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384,   521, -1384,
    1161, -1384, -1384, -1384, -1384, -1384,   122,   634, -1384,  1369,
     115, 15349,  1285,  1375, -1384,  4299,   653, -1384, -1384,  1192,
    1379, 13605, -1384, 18015, -1384,   314,  1195, -1384, -1384, -1384,
    1088,   544, 14454, -1384,  1088, -1384,  3979,  3979, -1384, -1384,
   -1384, -1384,  7515, 18109, 18109, 18109, -1384, -1384, -1384, 18109,
   -1384,  1063,  1388,  1389,  1199, -1384, -1384, 16415, 15159, 15159,
    1341, -1384,  2636,  2636,   683, -1384, -1384, -1384, 16415,  1319,
   -1384,  1220,  1207,   118, 16415, -1384,  4150, -1384, 16415, 18015,
    1325, -1384,  1400, -1384,  7718,  1212, -1384, -1384,   544, -1384,
   -1384,  7921,  1217,  1295, -1384,  1311,  1257, -1384, -1384,  1312,
    4299,  1236,  1215, -1384, -1384, 18109, -1384, -1384,  1249, -1384,
    1384, -1384, -1384, -1384, -1384, 18109,  1408,   602, -1384, -1384,
   18109,  1237, 18109, -1384,   486,  1238,  8124, -1384, -1384, -1384,
    1244, -1384,  1251,  1256,  4150,   669,  1261, -1384, -1384, -1384,
   16415,  1263,   127, -1384,  1361, -1384, -1384, -1384,  8327, -1384,
   15054,   903, -1384,  1274,  4150,   576, -1384, 18109, -1384,  1253,
    1443,   692,   127, -1384, -1384,  1370, -1384, 15054,  1264, -1384,
    1088,   131, -1384, -1384, -1384, -1384,  4299, -1384,  1259,  1266,
     120, -1384,  1269,   692,    95,  1088,  1255, -1384,  4299,   554,
    4299,    62,  1444,  1376,  1269, -1384,  1452, -1384,   332, -1384,
   -1384, -1384,   121,  1451,  1383, 13605, -1384,   554,  8530,  4299,
   -1384,  4299, -1384,  8733,    66,  1453,  1385, 13605, -1384, 18015,
   -1384, -1384, -1384, -1384, -1384,  1456,  1390, 13605, -1384, 18015,
   13605, -1384, 18015, 18015
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1384, -1384, -1384,  -567, -1384, -1384, -1384,   219,     0,   -34,
     450, -1384,  -272,  -516, -1384, -1384,   341,    -1,  1649, -1384,
    1851, -1384,  -480, -1384,     5, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384, -1384, -1384, -1384, -1384,  -413, -1384, -1384,  -163,
     136,    24, -1384, -1384, -1384, -1384, -1384, -1384,    30, -1384,
   -1384, -1384, -1384, -1384, -1384,    37, -1384, -1384,   990,   996,
     997,   -98,  -708,  -881,   499,   556,  -423,   245,  -961, -1384,
    -139, -1384, -1384, -1384, -1384,  -753,    80, -1384, -1384, -1384,
   -1384,  -410, -1384,  -584, -1384,  -450, -1384, -1384,   894, -1384,
    -122, -1384, -1384, -1074, -1384, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384, -1384, -1384,  -159, -1384,   -69, -1384, -1384, -1384,
   -1384, -1384,  -239, -1384,    31, -1005, -1384, -1383,  -444, -1384,
    -135,    63,  -127,  -419, -1384,  -244, -1384, -1384, -1384,    45,
     -13,    -6,    56,  -728,   -65, -1384, -1384,    15, -1384,   -11,
   -1384, -1384,    -5,   -32,  -102, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384, -1384, -1384,  -614,  -871, -1384, -1384, -1384, -1384,
   -1384,   961,  1134, -1384,   427, -1384,   293, -1384, -1384, -1384,
   -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384,
   -1384, -1384, -1384,   149,  -545,  -565, -1384, -1384, -1384, -1384,
   -1384,   357, -1384, -1384, -1384, -1384, -1384, -1384, -1384, -1384,
    -967,  -373,  2859,    40, -1384,  1478,  -415, -1384, -1384,  -485,
    3509,  3425, -1384,  -629, -1384, -1384,   434,    22,  -634, -1384,
   -1384,   514,   302,  -605, -1384,   304, -1384, -1384, -1384, -1384,
   -1384,   492, -1384, -1384, -1384,    26,  -917,  -115,  -442,  -434,
   -1384,   569,  -111, -1384, -1384,    42,    44,   582, -1384, -1384,
     759,   -12, -1384,  -366,    27,  -375,    97,  -316, -1384, -1384,
    -477,  1142, -1384, -1384, -1384, -1384, -1384,   734,   347, -1384,
   -1384, -1384,  -360,  -651, -1384,  1096, -1064, -1384,   -66,  -190,
      18,   695, -1384,  -409, -1384,  -420,  -606, -1274,  -326,    78,
   -1384,   395,   472, -1384, -1384, -1384, -1384,   421, -1384,  1869,
   -1122
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1058
static const yytype_int16 yytable[] =
{
     188,   190,   442,   192,   193,   194,   196,   197,   198,   122,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   343,   520,   233,   236,   492,   257,   124,   660,
     161,   657,   402,  1170,   126,   951,   405,   406,   659,   260,
     262,   127,   782,   413,   661,   266,   351,   268,  1357,   271,
     542,   946,   352,   796,   355,   514,   342,  1462,   250,   363,
     442,   965,   438,   491,   731,   778,   779,   129,  1162,   771,
     810,   243,   350,   248,  1060,   249,   415,   772,   927,   878,
     883,   260,   947,  1343,   591,   593,  1244,   398,  1034,  1187,
     399,   824,  1046,   432,   803,   412,   417,   988,  1647,    14,
     259,   554,   -39,   414,   806,  1198,   866,   -39,   -74,   -38,
     603,  1421,   807,   -74,   -38,   889,   608,   826,   554,  1590,
    1592,  -353,  1655,  1740,  1809,    14,    14,  1809,  1171,  1647,
    1358,  1802,  1492,  1931,   512,   364,   546,   554,   906,   906,
     123,   547,   388,   587,  1603,  1943,   906,   906,   906,  1965,
    1090,   372,   415,  1233,     3,   389,  -902,  1024,  1803,  1954,
    1253,  1254,   390,   509,   510,   529,   389,   735,   801,   898,
     899,   412,   417,  1172,  1119,   599,    14,  1493,  1932,   414,
      14,  1027,  -103,   509,   510,  -102,   523,   948,   191,  1091,
     873,   531,   509,   510,  -875,   429,   776,  -103,   509,   510,
    -102,   780,  1253,  1254,  1955,   253,   588,   540,   420,  -875,
     530,   417,  -903,  -907,  -906,   214,    40,  1282,   414,  1487,
     512,  1359,  -901,   116,   493,  -915,   539,  -596,  -715,   391,
     392,   393,   391,  -943,   414,  1604,  1944,   600,   517,  -900,
    1966,   392,   393,  -811,  -904,  1132,  -721,  -811,   549,   926,
    -603,   549,   874,   365,  1256,   517,  -946,   537,   260,   560,
    -908,  -945,   551,   513,  1173,   441,   556,  -887,  1494,  1933,
    -292,  -888,   269,  -292,   635,   341,  -603,  1422,   701,   825,
     571,  1201,   433,  1462,   827,  -902,  1414,  1648,  1649,  -539,
     555,   -39,   378,  1501,   933,  1956,  1424,   -74,   -38,   604,
    1507,  -809,  1509,   890,  1251,   609,  1255,   633,  1591,  1593,
    -353,  1656,  1741,  1810,   582,   400,  1859,   378,  1927,  1804,
    1379,   378,   378,   521,  -276,   891,   907,  1001,  -811,  -722,
     403,  1531,  1043,   111,  1334,  1537,  1597,  1045,  -910,  -914,
    -917,  -903,  -907,  -906,   614,  -716,  1951,   378,   342,   513,
    -723,  -901,  1820,  -912,  1399,   363,   363,   594,   783,  1128,
    1129,  1154,  -943,   613,   617,  -911,   442,   518,  -900,   699,
     260,   414,  1223,  -904,  1349,  1024,   516,   233,   625,   260,
     260,   625,   260,  1184,   518,  -946,   983,   639,  1826,  -908,
    -945,  -714,   479,  1402, -1018,   644,  -887,  1821,   343,  1213,
    -888,  1562,   516,   895,   480,   934,   196,  1350,   370,   214,
      40,   389,   599,   256,   684,   429,   371,   263,   421,   527,
     935,   407,   402,   747,   748,   438,   696,  -913,  1145,   752,
   -1018,   612,   342, -1018,  1328,  1329,   670,  1619,  -606,  1224,
     628,   628,  1286,   628,  -604,  1869,   702,   703,   704,   705,
     707,   708,   709,   710,   711,   712,   713,   714,   715,   716,
     717,   718,   719,   720,   721,   722,   723,   724,   725,   726,
     727,   728,   729,   730,   428,   732,   116,   733,   733,   736,
     116,   129,   353,   754,   561,   741,   392,   393,  -878,   755,
     756,   757,   758,   759,   760,   761,   762,   763,   764,   765,
     766,   767,  1650,  -878,  1342,   264,   250,   733,   777,   753,
     696,   696,   733,   781,  1177,  1411,  -605,   755,   265,   243,
     785,   248,  1178,   249,  1884,   419,  1753,   111,  1754,   793,
     408,   795,   342,  1195,   630,  1797,   632,   409,   634,   696,
    1279,   813,  1281,   873,   389,   374,  -876,   814,   492,   815,
     789,   641,   982,  1798,   123,  1356,   389,   954,   389,   956,
    1354,  -876,   818,   641,   660,   641,   657,   581,   375,  1885,
     800,   368,  1799,   659,  1250,  1551,  1203,   376,   369,   661,
     509,   510,   743,   377,   994,   491,   165,   990,  1024,  1024,
    1024,  1024,  1024,  1024,   882,   882,   885,  1125,  1024,  1126,
     381,  1366,   509,   510,  1368,   428,  -717,   382,   775,   229,
     231,  1344,  1213,  1391,   403,   878,  1391,   383,   389,   392,
     393,   937,  1403,   384,  1345,   424,  1040,  1041,   750,   743,
     642,   392,   393,   392,   393,   972,   385,   116,   386, -1018,
     802,   387,  -915,   808,  1346,   389,   428,   688,   414,   912,
     914,   341,   898,   899,   378,  1395,   389,  1397,  1626,    55,
     429,   404,   542,   641,  1952,  1805,   687,    62,    63,    64,
     178,   179,   439,   427,  1120,   954,   956,   940, -1018,   686,
     646,   647,  1035,   956,  1806,   431,  1036,  1807,   962,   428,
     416,   434,   670,   392,   393,    62,    63,    64,   178,   179,
     439,   973,  1235,  1236,   581,   378,   745,   378,   378,   378,
     378,  1328,  1329,   660,  1852,   657,  1558,  1559,  1381,   636,
     392,   393,   659,   484,  1508,  1252,  1253,  1254,   661,   443,
     770,   392,   393,  1853,   444,   981,  1854,   637,   445,   440,
    1491,   643,  1767,  1768,  1372,  -597,   493,  1115,  1116,  1117,
     446,   581,  1939,  1940,  1024,  1382,  1024,   447,  -119,  1499,
     980,   448,  -119,  1118,   993,   805,   416,   440,   637,  1413,
     643,   637,   643,   643,  1909,   449,   116,   450,  1514,  -119,
    1515,  1418,  1253,  1254,   485,  1503,   230,   230,    62,    63,
      64,   178,   179,   439,  1924,  -598,   864,  1850,  1851,  1038,
     476,   477,   478,  -599,   479,   416,   590,   592,  1942,  -600,
    1044,  1913,  1914,  1915,   533,   260,   480,   515,   884,   688,
    -601,   541,   482,  1112,  1113,  1114,  1115,  1116,  1117,  1595,
    1055,  1846,  1847,    62,    63,    64,    65,    66,   439,   165,
     483,  1456,  1118,   165,    72,   486,   423,   425,   426,  -602,
     660,  1062,   657,   921,   923,  1071,  1074,  1068,  1482,   659,
     440,  -909,  -715,   519,   396,   661,   524,   526,   882,   480,
     882,   429,   532,   129,   882,   882,  1130,  -913,   516,   536,
     535,  -713,   543,   544,   552,   487,   565,   488, -1057,  1024,
     577,  1024,   573,  1024,  1611,   576,  1613,   583,  1024,   584,
     489,   596,   490,   595,  1149,   440,  1150,   598,   815,   607,
     605,   606,   618,  1622,   662,  1623,  1651,  1624,   619,  1152,
     378,  1505,  1625,  1140,  1202,   663,   672,  1533,  1620,   129,
     788,  -124,   122,  1161,   685,   673,    55,   674,   676,   698,
     786,   636,   741,  1542,   816,   790,   123,   602,   791,   554,
     797,   124,   798,   161,   820,   823,   610,   126,   615,  1182,
     837,   838,   867,   622,   127,  1901,   571,   869,   871,  1190,
     870,   888,  1191,   872,  1192,   640,   670,   903,   696,   892,
     893,   910,   911,   896,   905,  1901,  1362,   897,   225,   225,
     129,   230,  1024,   670,  1923,   908,   913,   915,   916,   917,
     165,   924,   123,   929,  1766,  1017,   918,  1030,  1771,   932,
     930,   129,  -738,   939,   938,   941,  1775,   942,   250,   473,
     474,   475,   476,   477,   478,   945,   479,   949,  1232,   116,
     950,   243,   958,   248,  1238,   249,   960,   963,   480,  1163,
     964,   966,   969,  1053,   116,   975,  1239,   976,  1608,   978,
    1228,   775,  1628,   808,   979,    62,    63,    64,    65,    66,
     439,  1634,   987,   123,   995,   997,    72,   486,   998,   999,
     971,   660,  -719,   657,  1047,  1641,  1037,  1061,  1057,  1065,
     659,  1059,  1066,  1336,   123,   116,   661,  1067,  1069,  1082,
    1287,  1131,  1127,   688,  1083,  1084,  1267,  1088,  1085,  1086,
    1087,   622,  1123,  1271,  1133,  1135,   129,  1137,   129,   488,
    1138,  1139,  1144,  1148,  1151,  1157,  1160,  1159,  1164,  1166,
    1168,  1141,  1197,  1185,   230,  1194,  1200,   440,  1205,  -916,
     882,  1206,   808,   230,  1217,   616,  1216,  1337,  1225,   165,
     230,  1218,  1219,  1338,   213,  1220,   116,  1024,  1024,  1221,
    1222,  1782,   230,  1226,   660,  1227,   657,  1229,  1247,   122,
    1241,   581,  1243,   659,  1825,  1249,    50,   116,  1828,   661,
    1246,  1829,  1830,   770,  1258,   805,  1364,  1259,   124,   123,
     161,   123,  1265,  1266,   126,  1118,  1260,  1269,  1865,   696,
    1270,   127,  1320,   225,  1322,  1323,  1325,  1332,  1335,   983,
     696,  1338,   217,   218,   219,   220,   221,  1353,  1360,   378,
    1352,   670,  1361,  1365,   670,  1369,  1367,   129,  1384,  1374,
    1385,  1210,  1210,  1017,   182,  1383,  1008,    91,  1371,  1406,
      93,    94,  1373,    95,   183,    97,  1376,  1386,   260,  1405,
    1377,  1407,  1398,  1409,  1408,  1410,  1415,  1425,  1420,  1419,
    1428,  1430,  1431,  1434,   805,  1435,  1436,  1439,   107,  1440,
     116,  1442,   116,  1834,   116,  1443,  1912,  1444,  1445,  1446,
    1448,  1451,  1495,  1496,   961,  1457,  1450,  1518,   230,  1458,
    1455,  1520,  1522,  1498,  1527,  1262,  1484,  1529,  1824,  1512,
     123,  1485,  1500,    34,    35,    36,  1437,  1502,  1831,  1510,
    1441,  1506,  1950,  1513,  1922,  1447,   215,  1511,  1516,   581,
    1517,  1464,  1452,   129,  1532,  1543,  1521,  1528,  1524,  1934,
    1534,  1525,  1526,  1535,  1536,  1566,   225,  1555,  1539,  1540,
    1579,  1594,  1605,   992,  1483,   225,  1599,  1606,   864,  1609,
    1614,  1488,   225,  1866,  1615,  1489,  1596,  1490,  1617,   442,
    1621,  1636,  1639,  1645,   225,  1497,    14,  1653,  1654,  1481,
      81,    82,    83,    84,    85,  1504,   696,  1748,  1749,  1481,
    1755,   222,  1761,   116,  1031,  1762,  1032,    89,    90,  1764,
    1765,  1776,  1774,  1808,  1777,  1787,   123,  1788,  1888,  1814,
    1817,    99,   165,  1818,  1823,   670,  1476,  1840,  1842,  1844,
    1848,  1857,  1856,  1858,  1051,   104,  1476,   165,  1863,  1864,
    1872,  1868,  1519,  -349,  1875,  1871,  1523,  1874,  1877,  1465,
    1803,  1879,  1880,  1530,  1466,  1751,    62,    63,    64,   178,
    1467,   439,  1468,  1883,  1891,  1886,  1017,  1017,  1017,  1017,
    1017,  1017,  1889,  1896,  1903,  1898,  1017,  1890,   165,  1907,
    1910,   230,  1911,  1919,  1935,  1948,  1925,   116,  1945,  1946,
    1953,  1949,  1921,  1926,  1928,  1957,  1958,  1967,  1968,   116,
    1970,  1136,  1324,  1971,  1469,  1470,   830,  1471,  1906,  1427,
     225,   749,   744,  1196,  1920,   746,  1156,   622,  1147,  1412,
    1607,  1781,  1918,   696,  1644,  1772,  1796,   886,   440,  1652,
    1541,  1801,  1586,  1960,  1930,   227,   227,  1472,  1813,   165,
     230,  1770,  1567,  1280,  1396,  1347,  1272,  1212,  1387,   129,
     631,  1388,  1230,  1176,   624,  1947,   213,  1481,   697,  1962,
     165,  1881,  1072,  1481,  1327,  1481,   831,  1557,   670,  1264,
    1319,     0,     0,  1632,   493,     0,  1459,     0,    50,     0,
       0,   230,     0,   230,     0,     0,     0,  1481,     0,     0,
       0,     0,     0,  1646,  1476,     0,     0,     0,     0,     0,
    1476,     0,  1476,     0,     0,     0,     0,     0,     0,     0,
       0,   230,     0,     0,   217,   218,   219,   220,   221,     0,
       0,     0,   123,  1816,  1476,     0,   129,     0,     0,     0,
    1763,     0,  1017,     0,  1017,   129,   182,     0,     0,    91,
       0,  1588,    93,    94,     0,    95,   183,    97,     0,   832,
       0,     0,     0,   165,     0,   165,     0,   165,     0,  1051,
    1245,     0,     0,     0,  1779,  1632,     0,     0,     0,     0,
     107,     0,     0,     0,     0,  1481,     0,     0,   230,     0,
       0,     0,     0,   225,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   230,   230,     0,     0,     0,   123,
       0,     0,     0,     0,     0,   116,   224,   224,   123,     0,
     240,     0,  1476,     0,   341,     0,     0,     0,     0,     0,
    1584,   129,     0,     0,     0,     0,     0,   129,     0,     0,
       0,     0,     0,     0,   129,   240,     0,     0,     0,     0,
     227,     0,   225,     0,     0,     0,     0,  1811,   522,   495,
     496,   497,   498,   499,   500,   501,   502,   503,   504,   505,
     506,     0,  1756,     0,     0,     0,   165,  1017,     0,  1017,
    1894,  1017,     0,     0,     0,     0,  1017,     0,     0,     0,
    1861,   342,   116,   225,     0,   225,  1819,   116,     0,     0,
       0,   116,  1363,     0,   123,     0,     0,     0,   442,     0,
     123,   507,   508,     0,     0,     0,     0,   123,     0,   378,
       0,     0,   581,   225,     0,   341,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1737,     0,     0,     0,     0,
       0,     0,  1744,     0,     0,     0,   230,   230,     0,   341,
       0,   341,     0,  1404,  1841,  1843,     0,   341,     0,     0,
     165,     0,     0,     0,     0,     0,     0,     0,   622,  1051,
       0,     0,   165,     0,     0,     0,     0,     0,   509,   510,
    1017,     0,     0,   227,     0,   129,     0,   116,   116,   116,
     225,     0,   227,   116,     0,     0,     0,     0,     0,   227,
     116,     0,     0,     0,     0,  1464,   225,   225,     0,     0,
       0,   227,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   224,   658,     0,     0,     0,     0,   129,     0,     0,
       0,     0,     0,     0,   129,     0,     0,     0,     0,     0,
       0,   675,     0,     0,     0,     0,     0,   344,   670,     0,
      14,     0,     0,     0,     0,     0,     0,   622,   123,     0,
       0,     0,     0,     0,     0,   348,     0,     0,   670,   129,
       0,   240,     0,   240,     0,     0,     0,   670,  1895,   230,
    1959,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   129,  1969,     0,     0,     0,     0,     0,     0,     0,
     123,     0,  1972,     0,     0,  1973,     0,   123,     0,     0,
     581,     0,     0,  1465,     0,     0,     0,     0,  1466,     0,
      62,    63,    64,   178,  1467,   439,  1468,     0,     0,   240,
     230,   341,     0,     0,     0,  1017,  1017,   227,     0,     0,
       0,   116,   123,     0,     0,   230,   230,     0,   225,   225,
    1835,   129,   213,     0,   224,     0,   129,  1737,  1737,     0,
       0,  1744,  1744,   224,   123,     0,     0,     0,  1469,  1470,
     224,  1471,     0,     0,    50,   378,     0,     0,   165,     0,
       0,     0,   224,   116,     0,     0,     0,     0,     0,     0,
     116,     0,   440,   224,     0,     0,     0,     0,     0,     0,
       0,  1486,     0,     0,     0,     0,     0,  1464,     0,     0,
     217,   218,   219,   220,   221,     0,     0,     0,   240,     0,
       0,   240,  1464,     0,   123,   116,     0,     0,     0,   123,
       0,     0,     0,  1893,   230,   396,     0,     0,    93,    94,
       0,    95,   183,    97,     0,     0,     0,   116,     0,     0,
       0,     0,    14,  1908,     0,   165,     0,     0,     0,     0,
     165,     0,     0,     0,   165,     0,   107,    14,   240,     0,
     397,     0,     0,   344,     0,   344,     0,     0,     0,     0,
       0,   225,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   569,     0,   570,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   116,   224,  1464,
     227,     0,   116,     0,     0,  1465,     0,     0,     0,     0,
    1466,     0,    62,    63,    64,   178,  1467,   439,  1468,  1464,
    1465,   344,   225,     0,     0,  1466,     0,    62,    63,    64,
     178,  1467,   439,  1468,     0,     0,  1464,   225,   225,   575,
     165,   165,   165,     0,    14,     0,   165,     0,     0,     0,
     240,     0,   240,   165,     0,   852,     0,     0,     0,   227,
    1469,  1470,     0,  1471,    14,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1469,  1470,     0,  1471,     0,
       0,    14,     0,     0,   440,     0,   852,     0,     0,     0,
       0,     0,     0,  1612,     0,     0,     0,     0,     0,   440,
     227,     0,   227,     0,     0,     0,     0,  1465,  1616,     0,
     344,     0,  1466,   344,    62,    63,    64,   178,  1467,   439,
    1468,     0,     0,     0,     0,     0,   225,  1465,   691,     0,
     227,   348,  1466,     0,    62,    63,    64,   178,  1467,   439,
    1468,   240,   240,     0,  1465,     0,     0,     0,     0,  1466,
     240,    62,    63,    64,   178,  1467,   439,  1468,     0,     0,
       0,     0,  1469,  1470,     0,  1471,     0,     0,     0,     0,
       0,   224,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1469,  1470,     0,  1471,   440,     0,     0,     0,
       0,     0,     0,     0,   165,  1618,     0,   227,     0,  1469,
    1470,     0,  1471,     0,     0,     0,   440,     0,     0,     0,
       0,     0,     0,   227,   227,  1627,     0,     0,     0,     0,
       0,     0,     0,   440,     0,     0,     0,     0,     0,     0,
     224,     0,  1773,     0,     0,     0,   165,   213,     0,   919,
       0,   920,     0,   165,     0,     0,     0,   658,     0,     0,
       0,     0,   344,     0,   833,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,   240,     0,     0,     0,     0,
     829,   224,     0,   224,     0,     0,     0,     0,   165,   522,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,     0,     0,     0,   217,   218,   219,   220,   221,
     165,   224,   852,     0,     0,     0,     0,     0,     0,   240,
       0,     0,  1207,  1208,  1209,   213,   240,   240,   852,   852,
     852,   852,   852,    93,    94,     0,    95,   183,    97,     0,
     852,     0,   507,   508,     0,     0,     0,    50,     0,     0,
       0,     0,     0,   344,   344,  1568,   240,     0,     0,     0,
       0,   107,   344,     0,     0,   227,   227,     0,     0,     0,
     165,   943,   944,     0,     0,   165,     0,     0,   224,     0,
     952,     0,     0,   217,   218,   219,   220,   221,     0,     0,
       0,     0,   240,     0,   224,   224,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   213,   658,     0,     0,   509,
     510,    93,    94,     0,    95,   183,    97,     0,   240,   240,
       0,     0,     0,     0,     0,     0,     0,    50,   224,     0,
       0,     0,     0,     0,   240,     0,     0,     0,     0,   107,
       0,     0,     0,     0,     0,     0,     0,   240,     0,  1569,
       0,     0,     0,     0,     0,   852,     0,     0,   240,     0,
       0,     0,  1570,   217,   218,   219,   220,   221,  1571,     0,
       0,     0,   799,     0,     0,     0,   240,     0,     0,     0,
     240,     0,     0,     0,     0,   182,     0,     0,    91,  1572,
       0,    93,    94,   240,    95,  1573,    97,     0,   227,     0,
       0,     0,     0,     0,     0,   522,   495,   496,   497,   498,
     499,   500,   501,   502,   503,   504,   505,   506,     0,   107,
       0,  1064,     0,     0,     0,     0,     0,     0,   344,   344,
       0,     0,     0,     0,     0,     0,   224,   224,     0,     0,
       0,     0,     0,   658,     0,     0,   691,   691,     0,   227,
     240,     0,     0,     0,   240,     0,   240,   213,   507,   508,
       0,     0,     0,     0,   227,   227,     0,     0,     0,     0,
       0,   852,   852,   852,   852,   852,   852,   224,     0,    50,
     852,   852,   852,   852,   852,   852,   852,   852,   852,   852,
     852,   852,   852,   852,   852,   852,   852,   852,   852,   852,
     852,   852,   852,   852,   852,   852,   852,   852,     0,     0,
       0,     0,     0,     0,     0,   217,   218,   219,   220,   221,
     344,     0,     0,     0,   852,   509,   510,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   344,     0,  1155,     0,
       0,  1742,     0,    93,    94,  1743,    95,   183,    97,   344,
       0,     0,     0,   227,  1165,     0,     0,   240,     0,   240,
       0,     0,     0,     0,     0,     0,     0,  1179,     0,   224,
       0,   107,  1583,     0,     0,     0,     0,     0,   344,     0,
       0,     0,     0,     0,     0,     0,   240,     0,   894,   240,
       0,     0,     0,     0,     0,     0,  1199,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   240,   240,   240,   240,
     240,   240,     0,     0,   224,     0,   240,     0,     0,     0,
     224,     0,     0,     0,     0,     0,   226,   226,     0,     0,
     242,     0,     0,     0,     0,   224,   224,     0,   852,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   240,     0,
       0,     0,   344,     0,   240,     0,   344,   852,   833,   852,
       0,     0,     0,     0,   658,     0,   451,   452,   453,     0,
    1257,     0,     0,     0,  1261,     0,     0,     0,     0,     0,
       0,     0,     0,   852,     0,     0,   454,   455,     0,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,     0,   479,     0,     0,     0,   240,   240,     0,
       0,   240,     0,     0,   224,   480,     0,   494,   495,   496,
     497,   498,   499,   500,   501,   502,   503,   504,   505,   506,
       0,     0,     0,     0,     0,     0,     0,   658,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   344,
       0,   344,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   240,     0,   240,     0,     0,  1355,     0,   952,
     507,   508,     0,     0,     0,     0,     0,     0,   344,     0,
       0,   344,   522,   495,   496,   497,   498,   499,   500,   501,
     502,   503,   504,   505,   506,     0,  1375,     0,     0,  1378,
       0,     0,     0,     0,     0,     0,     0,   240,     0,   240,
       0,     0,     0,     0,     0,   852,     0,   852,     0,   852,
       0,   226,     0,     0,   852,   224,     0,     0,   852,   213,
     852,     0,     0,   852,     0,   507,   508,   509,   510,     0,
     344,  1092,  1093,  1094,   240,   240,   344,     0,   240,  1340,
       0,    50,     0,     0,     0,   240,     0,     0,  1426,     0,
       0,     0,  1095,     0,  1179,  1096,  1097,  1098,  1099,  1100,
    1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,   217,   218,   219,
     220,   221,     0,     0,     0,     0,     0,   240,     0,   240,
    1118,   240,   509,   510,     0,     0,   240,     0,   224,   344,
     344,     0,     0,     0,     0,    93,    94,     0,    95,   183,
      97,     0,   240,     0,     0,   852,     0,  1460,  1461,     0,
       0,     0,     0,     0,     0,     0,     0,   240,   240,     0,
       0,     0,     0,   107,   698,   240,     0,   240,     0,     0,
       0,     0,     0,     0,   226,     0,     0,     0,     0,     0,
       0,     0,     0,   226,     0,     0,     0,     0,     0,   240,
     226,   240,     0,     0,     0,     0,     0,   240,     0,     0,
       0,     0,   226,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   226,     0,     0,     0,     0,     0,     0,
     240,     0,     0,     0,     0,     0,     0,     0,     0,   344,
       0,   344,     0,     0,     0,     0,     0,   852,   852,   852,
       0,     0,     0,     0,   852,     0,   240,  1544,  1285,  1545,
       0,     0,   240,     0,   240,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   344,     0,     0,     0,
       0,     0,     0,   451,   452,   453,     0,   344,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   242,     0,
       0,     0,   830,   454,   455,  1589,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,     0,
     479,     0,     0,     0,     0,     0,     0,     0,   226,     0,
       0,     0,   480,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   213,     0,   344,     0,     0,     0,     0,     0,
       0,     0,   831,     0,   240,     0,     0,     0,   282,     0,
       0,     0,  1635,     0,    50,     0,     0,   344,     0,     0,
       0,   240,     0,     0,     0,   240,   240,     0,     0,     0,
       0,     0,     0,     0,     0,   859,   284,     0,     0,     0,
     240,   344,     0,   344,     0,     0,   852,     0,     0,   344,
     217,   218,   219,   220,   221,     0,     0,   852,   213,     0,
       0,     0,     0,   852,     0,     0,   859,   852,     0,     0,
       0,     0,   182,     0,     0,    91,     0,     0,    93,    94,
      50,    95,   183,    97,     0,  1263,     0,     0,  -396,   240,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   178,
     179,   439,     0,     0,   344,     0,   107,     0,     0,     0,
       0,     0,     0,   925,     0,   567,   217,   218,   219,   220,
     221,   568,  1792,     0,   282,     0,     0,     0,     0,   852,
       0,     0,     0,     0,     0,     0,   228,   228,   182,   240,
     246,    91,   335,     0,    93,    94,     0,    95,   183,    97,
       0,   226,   284,     0,     0,     0,   240,     0,     0,     0,
       0,     0,   339,     0,     0,   240,     0,     0,   440,     0,
       0,     0,   107,   340,   213,     0,     0,   240,     0,   240,
       0, -1058, -1058, -1058, -1058, -1058,   471,   472,   473,   474,
     475,   476,   477,   478,     0,   479,    50,     0,   240,     0,
     240,     0,     0,     0,   574,     0,   344,   480,     0,     0,
     226,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   344,  1815,   213,     0,     0,     0,     0,
       0,   567,   217,   218,   219,   220,   221,   568,     0,     0,
       0,     0,  1836,     0,     0,  1023,     0,    50,     0,     0,
       0,   226,     0,   226,   182,     0,     0,    91,   335,     0,
      93,    94,     0,    95,   183,    97,     0,     0,     0,     0,
     213,     0,   214,    40,     0,     0,     0,     0,   339,     0,
       0,   226,   859,   217,   218,   219,   220,   221,   107,   340,
       0,   344,    50,     0,     0,     0,     0,   282,   859,   859,
     859,   859,   859,     0,     0,     0,     0,     0,     0,  1876,
     859,    93,    94,     0,    95,   183,    97,     0,     0,     0,
       0,     0,     0,     0,     0,   284,  1122,     0,   217,   218,
     219,   220,   221,     0,     0,     0,     0,     0,     0,   107,
     971,   228,     0,     0,     0,   282,     0,   213,   226,     0,
       0,     0,     0,     0,   768,     0,    93,    94,     0,    95,
     183,    97,  1143,     0,   226,   226,     0,   344,     0,    50,
       0,     0,     0,   284,     0,     0,     0,     0,     0,   344,
       0,   344,     0,     0,   107,   952,     0,     0,   769,  1143,
     111,     0,     0,     0,     0,   213,     0,  1938,   226,   952,
     344,     0,   344,     0,   567,   217,   218,   219,   220,   221,
     568,     0,     0,     0,     0,     0,     0,    50,  1938,     0,
    1963,     0,     0,     0,     0,   859,     0,   182,  1186,     0,
      91,   335,     0,    93,    94,     0,    95,   183,    97,     0,
    1070,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     242,   339,   567,   217,   218,   219,   220,   221,   568,     0,
       0,   107,   340,  1023,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   228,   182,     0,     0,    91,   335,
       0,    93,    94,   228,    95,   183,    97,     0,  1429,     0,
     228,  1093,  1094,     0,     0,     0,     0,     0,     0,   339,
       0,     0,   228,     0,     0,     0,   226,   226,     0,   107,
     340,  1095,     0,   246,  1096,  1097,  1098,  1099,  1100,  1101,
    1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,     0,     0,     0,     0,
       0,   859,   859,   859,   859,   859,   859,   226,     0,  1118,
     859,   859,   859,   859,   859,   859,   859,   859,   859,   859,
     859,   859,   859,   859,   859,   859,   859,   859,   859,   859,
     859,   859,   859,   859,   859,   859,   859,   859,     0,   451,
     452,   453,     0,     0,     0,     0,     0,     0,   246,     0,
       0,     0,     0,     0,   859,     0,     0,     0,     0,   454,
     455,   861,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,     0,   479,     0,   228,  1002,
    1003,     0,   887,     0,     0,     0,     0,     0,   480,   226,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1004,
       0,     0,     0,     0,     0,     0,     0,  1005,  1006,  1007,
     213,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1008,     0,   213,     0,   214,    40,  1023,  1023,  1023,  1023,
    1023,  1023,    50,     0,   226,   860,  1023,     0,     0,     0,
     226,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   226,   226,     0,   859,     0,
       0,     0,     0,     0,     0,     0,   860,  1009,  1010,  1011,
    1012,  1013,  1014,     0,     0,     0,     0,   859,     0,   859,
     217,   218,   219,   220,   221,  1015,     0,     0,     0,     0,
     182,     0,     0,    91,    92,     0,    93,    94,     0,    95,
     183,    97,     0,   859,     0,     0,   768,     0,    93,    94,
       0,    95,   183,    97,  1016,     0,     0,     0,     0,   957,
       0,     0,     0,     0,   107,     0,     0,     0,     0,   451,
     452,   453,     0,     0,     0,     0,   107,     0,     0,     0,
     804,  1463,   111,     0,   226,     0,     0,     0,     0,   454,
     455,   228,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   213,   479,     0,     0,     0,
       0,   213,     0,     0,     0,     0,     0,     0,   480,     0,
       0,     0,  1023,     0,  1023,     0,     0,    50,  1052,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
     228,     0,     0,     0,  1075,  1076,  1077,  1078,  1079,     0,
       0,     0,  1582,     0,     0,     0,  1089,     0,     0,     0,
       0,     0,     0,   217,   218,   219,   220,   221,     0,   217,
     218,   219,   220,   221,     0,   859,     0,   859,     0,   859,
       0,   228,     0,   228,   859,   226,     0,     0,   859,     0,
     859,    93,    94,   859,    95,   183,    97,    93,    94,     0,
      95,   183,    97,     0,     0,  1565,     0,     0,  1578,     0,
     282,   228,   860,     0,     0,     0,     0,     0,     0,   107,
    1583,     0,     0,     0,     0,   107,     0,     0,   860,   860,
     860,   860,   860,     0,     0,     0,     0,     0,   284,     0,
     860,     0,     0,   213,     0,     0,     0,     0,     0,   996,
       0,     0,     0,     0,     0,     0,     0,  1023,     0,  1023,
     213,  1023,     0,     0,     0,    50,  1023,     0,   226,     0,
       0,  1183,     0,   356,   357,     0,     0,     0,   228,     0,
       0,     0,    50,     0,     0,   859,     0,     0,     0,     0,
       0,     0,     0,     0,   228,   228,     0,  1642,  1643,     0,
       0,   217,   218,   219,   220,   221,     0,  1578,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   567,   217,   218,
     219,   220,   221,   568,     0,     0,   358,     0,   246,    93,
      94,     0,    95,   183,    97,     0,     0,     0,     0,     0,
     182,     0,     0,    91,   335,     0,    93,    94,     0,    95,
     183,    97,     0,     0,     0,   860,     0,   107,     0,     0,
    1023,     0,     0,     0,   339,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   107,   340,     0,   859,   859,   859,
     246,     0,     0,     0,   859,     0,  1790,  1079,  1275,     0,
       0,  1275,     0,     0,  1578,     0,  1288,  1291,  1292,  1293,
    1295,  1296,  1297,  1298,  1299,  1300,  1301,  1302,  1303,  1304,
    1305,  1306,  1307,  1308,  1309,  1310,  1311,  1312,  1313,  1314,
    1315,  1316,  1317,  1318,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   228,   228,     0,     0,
    1326,   455,     0,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,     0,   479,     0,     0,
       0,   860,   860,   860,   860,   860,   860,   246,     0,   480,
     860,   860,   860,   860,   860,   860,   860,   860,   860,   860,
     860,   860,   860,   860,   860,   860,   860,   860,   860,   860,
     860,   860,   860,   860,   860,   860,   860,   860,     0,     0,
       0,     0,     0,     0,     0,  1023,  1023,     0,     0,     0,
     213,     0,     0,     0,   860,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   859,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,   859,     0,     0,
       0,     0,     0,   859,  1416,     0,     0,   859,     0,     0,
       0,     0,     0,     0,  1569,   213,     0,     0,     0,   228,
       0,     0,     0,  1432,     0,  1433,     0,  1570,   217,   218,
     219,   220,   221,  1571,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,   875,   876,     0,     0,  1453,
     182,     0,     0,    91,    92,     0,    93,    94,     0,    95,
    1573,    97,     0,     0,   246,     0,     0,     0,     0,   859,
     228,     0,     0,   217,   218,   219,   220,   221,     0,  1905,
       0,     0,     0,     0,   107,   228,   228,     0,   860,     0,
       0,     0,     0,     0,     0,     0,  1565,     0,   877,     0,
       0,    93,    94,     0,    95,   183,    97,   860,     0,   860,
       0,     0,     0,     0,   451,   452,   453,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   107,
       0,     0,     0,   860,   454,   455,     0,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   480,   228,     0,     0,     0,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,  1547,     0,  1548,     0,  1549,     0,     0,     0,     0,
    1550,     0,     0,     0,  1552,     0,  1553,     0,     0,  1554,
       0,     0,     0,   692,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,   860,     0,   860,    50,   860,
       0,     0,     0,     0,   860,   246,    55,     0,   860,     0,
     860,     0,     0,   860,    62,    63,    64,   178,   179,   180,
       0,  1637,    69,    70,  1000,     0,     0,     0,     0,     0,
       0,     0,   181,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,     0,   693,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,   246,     0,
     107,   184,     0,     0,   213,     0,   111,   112,     0,   113,
     114,   451,   452,   453,  1063,   860,     0,     0,     0,     0,
       0,     0,     0,  1783,  1784,  1785,    50,     0,     0,     0,
    1789,   454,   455,     0,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,     0,   479,     0,
       0,     0,   217,   218,   219,   220,   221,     0,     0,     0,
     480, -1058, -1058, -1058, -1058, -1058,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,   182,     0,     0,    91,     0,     0,
      93,    94,     0,    95,   183,    97,     0,  1118,     0,     0,
       0,     0,     0,     0,   451,   452,   453,   860,   860,   860,
       0,     0,     0,     0,   860,     0,     0,     0,   107,     0,
       0,     0,     0,  1795,   454,   455,     0,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   480,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,  1845,   479,   451,   452,   453,     0,     0,     0,
       0,     0,     0,  1855,     0,   480,     0,     0,     0,  1860,
       0,  1134,     0,  1862,   454,   455,     0,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   480,     0,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1897,   860,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,   860,     0,     0,
       0,     0,     0,   860,     0,     0,     0,   860,     0,     0,
       0,    14,    15,    16,  1193,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,  1878,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,   860,
      52,    53,    54,    55,    56,    57,    58,     0,    59,    60,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,    71,    72,    73,  1204,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,    88,    89,    90,    91,    92,     0,    93,
      94,     0,    95,    96,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
     103,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  1153,   111,   112,     0,   113,   114,     5,     6,
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
       0,     0,     0,    52,    53,    54,    55,    56,    57,    58,
       0,    59,    60,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,    88,    89,    90,    91,
      92,     0,    93,    94,     0,    95,    96,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,   103,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  1341,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
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
      56,    57,    58,     0,    59,    60,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,    88,
      89,    90,    91,    92,     0,    93,    94,     0,    95,    96,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,   103,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   109,   110,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
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
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   182,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   183,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   108,     0,   109,
     110,   677,   111,   112,     0,   113,   114,     5,     6,     7,
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
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   182,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   183,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,  1121,   111,   112,     0,   113,   114,
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
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   182,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   183,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,  1167,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
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
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   182,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   183,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    1240,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,  1242,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   182,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   183,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   108,
       0,   109,   110,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,  1417,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   182,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   183,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
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
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     182,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     183,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1556,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
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
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  1786,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,  1832,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
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
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   182,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   183,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   109,   110,  1867,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,  1870,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   182,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   183,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   108,     0,   109,
     110,     0,   111,   112,     0,   113,   114,     5,     6,     7,
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
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   182,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   183,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,  1887,   111,   112,     0,   113,   114,
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
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   182,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   183,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,  1904,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
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
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   182,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   183,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    1961,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   182,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   183,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   108,
       0,   109,   110,  1964,   111,   112,     0,   113,   114,     5,
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
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   182,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   183,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,   550,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,   178,   179,    67,     0,    68,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     182,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     183,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,   817,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,   178,   179,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,  1054,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,    61,    62,    63,    64,   178,   179,    67,
       0,    68,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
    1631,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
     178,   179,    67,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   182,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   183,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   109,   110,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,  1778,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,   178,   179,    67,     0,    68,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   182,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   183,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   108,     0,   109,
     110,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,    61,    62,    63,    64,   178,   179,    67,     0,
      68,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   182,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   183,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   411,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,   751,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   178,
     179,   180,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   181,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   182,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   183,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,     0,     0,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   178,   179,   180,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   181,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   182,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   183,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   184,     0,   349,     0,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   178,   179,   180,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     181,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   182,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   183,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   184,
       0,     0,   812,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,  1095,     0,    10,
    1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,     0,     0,  1180,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1118,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   178,   179,
     180,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   181,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   182,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   183,    97,     0,
    1181,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   184,     0,     0,     0,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   411,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   178,   179,   180,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   181,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     182,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     183,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   108,   451,   452,   453,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   454,   455,     0,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,     0,   479,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   480,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
     195,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   178,   179,   180,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   181,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,  1234,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   184,     0,
       0,     0,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,  1096,
    1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,     0,     0,   232,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1118,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   178,   179,   180,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   181,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   184,   451,   452,   453,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   454,   455,     0,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,     0,   479,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,   480,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     178,   179,   180,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   181,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   182,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   183,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,  1601,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   184,     0,   267,   452,   453,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,   454,   455,     0,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,     0,   479,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,   480,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   178,   179,   180,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   181,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   182,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   183,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   184,     0,   270,
       0,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   411,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   178,   179,   180,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   181,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   182,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   183,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,   451,   452,   453,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   454,   455,     0,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,     0,   479,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
     480,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   178,
     179,   180,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   181,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   182,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   183,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,  1602,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   184,   548,     0,     0,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,  1100,  1101,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,   706,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1118,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   178,   179,   180,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   181,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   182,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   183,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   184,     0,     0,     0,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,     0,   479,
       0,   751,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   480,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   178,   179,   180,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     181,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   182,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   183,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   184,
       0,     0,     0,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,     0,     0,     0,   792,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1118,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   178,   179,
     180,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   181,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   182,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   183,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   184,     0,     0,     0,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,     0,     0,   794,     0,     0,
       0,     0,     0,     0,     0,     0,   480,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   178,   179,   180,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   181,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     182,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     183,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   184,     0,     0,     0,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,  1098,  1099,  1100,  1101,
    1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,     0,     0,     0,     0,
    1231,     0,     0,     0,     0,     0,     0,     0,     0,  1118,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   178,   179,   180,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   181,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   184,   451,
     452,   453,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   454,
     455,  1421,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,     0,   479,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,   480,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   178,   179,   180,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   181,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,  1422,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   184,   451,   452,   453,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
     821,    10,   454,   455,     0,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,     0,   479,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,   480,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
     638,    39,    40,     0,   822,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     178,   179,   180,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   181,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   182,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   183,
      97,     0,   272,   273,    99,   274,   275,   100,     0,   276,
     277,   278,   279,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   184,     0,   280,     0,   281,   111,
     112,     0,   113,   114,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,     0,   283,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   480,     0,     0,     0,
     285,   286,   287,   288,   289,   290,   291,     0,     0,     0,
     213,     0,   214,    40,     0,     0,     0,     0,     0,     0,
       0,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,    50,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,     0,   326,     0,   739,
     328,   329,   330,     0,     0,     0,   331,   578,   217,   218,
     219,   220,   221,   579,     0,     0,     0,     0,     0,   272,
     273,     0,   274,   275,     0,     0,   276,   277,   278,   279,
     580,     0,     0,     0,     0,     0,    93,    94,     0,    95,
     183,    97,   336,   280,   337,   281,     0,   338,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   107,     0,     0,     0,   740,     0,
     111,     0,     0,   283,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   285,   286,   287,
     288,   289,   290,   291,     0,     0,     0,   213,     0,   214,
      40,     0,     0,     0,     0,     0,     0,     0,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,    50,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,     0,   326,   213,   327,   328,   329,   330,
       0,     0,     0,   331,   578,   217,   218,   219,   220,   221,
     579,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,   272,   273,     0,   274,   275,     0,   580,   276,   277,
     278,   279,     0,    93,    94,     0,    95,   183,    97,   336,
       0,   337,     0,     0,   338,   280,     0,   281,     0,   282,
       0,     0,     0,   217,   218,   219,   220,   221,     0,     0,
       0,   107,     0,     0,     0,   740,     0,   111,     0,     0,
       0,     0,     0,     0,     0,   283,     0,   284,     0,   436,
       0,    93,    94,     0,    95,   183,    97,     0,     0,   285,
     286,   287,   288,   289,   290,   291,     0,     0,     0,   213,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   107,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,    50,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,     0,   326,     0,     0,   328,
     329,   330,     0,     0,     0,   331,   332,   217,   218,   219,
     220,   221,   333,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   334,
       0,     0,    91,   335,     0,    93,    94,     0,    95,   183,
      97,   336,     0,   337,     0,     0,   338,   272,   273,     0,
     274,   275,     0,   339,   276,   277,   278,   279,     0,     0,
       0,     0,     0,   107,   340,     0,     0,     0,  1757,     0,
       0,   280,     0,   281,     0,   282,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,     0,
     479,   283,     0,   284,     0,     0,     0,     0,     0,     0,
       0,     0,   480,     0,     0,   285,   286,   287,   288,   289,
     290,   291,     0,     0,     0,   213,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,    50,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,     0,   326,     0,     0,   328,   329,   330,     0,     0,
       0,   331,   332,   217,   218,   219,   220,   221,   333,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   334,     0,     0,    91,   335,
       0,    93,    94,     0,    95,   183,    97,   336,     0,   337,
       0,     0,   338,   272,   273,     0,   274,   275,     0,   339,
     276,   277,   278,   279,     0,     0,     0,     0,     0,   107,
     340,     0,     0,     0,  1827,     0,     0,   280,     0,   281,
       0,   282,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,     0,     0,     0,     0,     0,     0,   283,     0,   284,
       0,     0,     0,     0,  1118,     0,     0,     0,     0,     0,
       0,   285,   286,   287,   288,   289,   290,   291,     0,     0,
       0,   213,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,    50,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,     0,   326,     0,
     327,   328,   329,   330,     0,     0,     0,   331,   332,   217,
     218,   219,   220,   221,   333,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   334,     0,     0,    91,   335,     0,    93,    94,     0,
      95,   183,    97,   336,     0,   337,     0,     0,   338,   272,
     273,     0,   274,   275,     0,   339,   276,   277,   278,   279,
       0,     0,     0,     0,     0,   107,   340,     0,     0,     0,
       0,     0,     0,   280,     0,   281,     0,   282,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,     0,   479,     0,     0,     0,
       0,     0,     0,   283,     0,   284,     0,     0,   480,     0,
       0,     0,     0,     0,     0,     0,     0,   285,   286,   287,
     288,   289,   290,   291,     0,     0,     0,   213,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,    50,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,     0,   326,     0,     0,   328,   329,   330,
       0,     0,     0,   331,   332,   217,   218,   219,   220,   221,
     333,     0,     0,     0,     0,     0,     0,     0,   213,     0,
       0,     0,     0,     0,     0,     0,     0,   334,     0,     0,
      91,   335,     0,    93,    94,     0,    95,   183,    97,   336,
      50,   337,     0,     0,   338,     0,   272,   273,     0,   274,
     275,   339,  1560,   276,   277,   278,   279,     0,     0,     0,
       0,   107,   340,     0,     0,     0,     0,     0,     0,     0,
     280,     0,   281,     0,   282,     0,   217,   218,   219,   220,
     221, -1058, -1058, -1058, -1058,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,     0,   479,
     283,   358,   284,     0,    93,    94,     0,    95,   183,    97,
       0,   480,     0,     0,   285,   286,   287,   288,   289,   290,
     291,     0,     0,     0,   213,     0,     0,     0,     0,     0,
       0,     0,   107,     0,     0,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,    50,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
       0,   326,     0,     0,   328,   329,   330,     0,     0,     0,
     331,   332,   217,   218,   219,   220,   221,   333,     0,     0,
       0,     0,     0,     0,     0,   213,     0,     0,     0,     0,
       0,     0,     0,     0,   334,     0,     0,    91,   335,     0,
      93,    94,     0,    95,   183,    97,   336,    50,   337,     0,
       0,   338,  1657,  1658,  1659,  1660,  1661,     0,   339,  1662,
    1663,  1664,  1665,     0,     0,     0,     0,     0,   107,   340,
       0,     0,     0,     0,     0,     0,  1666,  1667,  1668,     0,
       0,     0,     0,   217,   218,   219,   220,   221,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   182,  1669,     0,    91,    92,
       0,    93,    94,     0,    95,   183,    97,     0,     0,     0,
    1670,  1671,  1672,  1673,  1674,  1675,  1676,     0,     0,     0,
     213,     0,     0,     0,     0,     0,     0,     0,     0,   107,
       0,  1677,  1678,  1679,  1680,  1681,  1682,  1683,  1684,  1685,
    1686,  1687,    50,  1688,  1689,  1690,  1691,  1692,  1693,  1694,
    1695,  1696,  1697,  1698,  1699,  1700,  1701,  1702,  1703,  1704,
    1705,  1706,  1707,  1708,  1709,  1710,  1711,  1712,  1713,  1714,
    1715,  1716,  1717,     0,     0,     0,  1718,  1719,   217,   218,
     219,   220,   221,     0,  1720,  1721,  1722,  1723,  1724,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1725,  1726,  1727,     0,   213,     0,    93,    94,     0,    95,
     183,    97,  1728,     0,  1729,  1730,     0,  1731,     0,     0,
       0,     0,     0,     0,  1732,  1733,    50,  1734,     0,  1735,
    1736,     0,   272,   273,   107,   274,   275,     0,     0,   276,
     277,   278,   279,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   280,     0,   281,     0,
       0,     0,   217,   218,   219,   220,   221,  1101,  1102,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,     0,     0,   283,   877,     0,     0,
      93,    94,     0,    95,   183,    97,     0,  1118,     0,     0,
     285,   286,   287,   288,   289,   290,   291,     0,     0,     0,
     213,     0,     0,     0,     0,     0,     0,     0,   107,     0,
       0,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,    50,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,     0,   326,     0,   327,
     328,   329,   330,     0,     0,     0,   331,   578,   217,   218,
     219,   220,   221,   579,     0,     0,     0,     0,     0,   272,
     273,     0,   274,   275,     0,     0,   276,   277,   278,   279,
     580,     0,     0,     0,     0,     0,    93,    94,     0,    95,
     183,    97,   336,   280,   337,   281,     0,   338,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   107,     0,     0,     0,     0,     0,
       0,     0,     0,   283,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   285,   286,   287,
     288,   289,   290,   291,     0,     0,     0,   213,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,    50,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,     0,   326,     0,  1286,   328,   329,   330,
       0,     0,     0,   331,   578,   217,   218,   219,   220,   221,
     579,     0,     0,     0,     0,     0,   272,   273,     0,   274,
     275,     0,     0,   276,   277,   278,   279,   580,     0,     0,
       0,     0,     0,    93,    94,     0,    95,   183,    97,   336,
     280,   337,   281,     0,   338,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   107,     0,     0,     0,     0,     0,     0,     0,     0,
     283,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   285,   286,   287,   288,   289,   290,
     291,     0,     0,     0,   213,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,    50,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
       0,   326,     0,     0,   328,   329,   330,     0,     0,     0,
     331,   578,   217,   218,   219,   220,   221,   579,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   580,     0,     0,     0,     0,     0,
      93,    94,     0,    95,   183,    97,   336,     0,   337,     0,
       0,   338,   451,   452,   453,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   107,     0,
       0,     0,   454,   455,     0,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,     0,   479,
     451,   452,   453,     0,     0,     0,     0,     0,     0,     0,
       0,   480,     0,     0,     0,     0,     0,     0,     0,     0,
     454,   455,     0,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,     0,   479,   451,   452,
     453,     0,     0,     0,     0,     0,     0,     0,     0,   480,
       0,     0,     0,     0,     0,     0,     0,     0,   454,   455,
       0,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,     0,   479,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   480,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   451,   452,
     453,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   454,   455,
     481,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,     0,   479,   451,   452,   453,     0,
       0,     0,     0,     0,     0,     0,     0,   480,     0,     0,
       0,     0,     0,     0,     0,     0,   454,   455,   564,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,     0,   479,   451,   452,   453,     0,     0,     0,
       0,     0,     0,     0,     0,   480,     0,     0,     0,     0,
       0,     0,     0,     0,   454,   455,   566,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   480,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   451,   452,   453,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   454,   455,   585,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,     0,     0,  1294,     0,     0,     0,     0,     0,
       0,     0,     0,   480,     0,     0,     0,     0,     0,     0,
       0,     0,   839,   840,   589,     0,     0,     0,   841,     0,
     842,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   843,     0,     0,     0,     0,     0,     0,     0,
      34,    35,    36,   213,     0,     0,     0,   451,   452,   453,
       0,     0,     0,   215,     0,     0,     0,     0,     0,     0,
       0,   784,     0,     0,     0,    50,     0,   454,   455,     0,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,     0,     0,     0,     0,     0,
     844,   845,   846,   847,   848,   849,   480,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   222,  1048,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,   809,    95,   183,    97,     0,     0,     0,    99,     0,
       0,     0,     0,     0,     0,     0,     0,   850,     0,     0,
       0,    29,   104,     0,     0,     0,     0,   107,   851,    34,
      35,    36,   213,     0,   214,    40,     0,     0,     0,     0,
       0,     0,   215,   525,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50, -1058, -1058, -1058, -1058,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,     0,     0,     0,   216,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1118,     0,     0,  1049,    75,
     217,   218,   219,   220,   221,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,   222,     0,     0,
       0,     0,   182,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   183,    97,     0,   839,   840,    99,     0,     0,
       0,   841,     0,   842,     0,     0,     0,     0,     0,     0,
       0,   104,     0,     0,     0,   843,   107,   223,     0,     0,
       0,     0,   111,    34,    35,    36,   213,     0,     0,     0,
     451,   452,   453,     0,     0,     0,   215,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
     454,   455,     0,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,     0,   479,     0,     0,
       0,     0,     0,   844,   845,   846,   847,   848,   849,   480,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,   222,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,    29,     0,
       0,    99,     0,     0,     0,     0,    34,    35,    36,   213,
     850,   214,    40,     0,     0,   104,     0,     0,     0,   215,
     107,   851,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,   534,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   216,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    75,   217,   218,   219,
     220,   221,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,   222,     0,     0,     0,     0,   182,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   183,
      97,    29,     0,     0,    99,     0,     0,     0,     0,    34,
      35,    36,   213,     0,   214,    40,     0,     0,   104,     0,
       0,     0,   215,   107,   223,     0,     0,   601,     0,   111,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   216,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   621,    75,
     217,   218,   219,   220,   221,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,   222,     0,     0,
       0,     0,   182,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   183,    97,    29,     0,   991,    99,     0,     0,
       0,     0,    34,    35,    36,   213,     0,   214,    40,     0,
       0,   104,     0,     0,     0,   215,   107,   223,     0,     0,
       0,     0,   111,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   216,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,   217,   218,   219,   220,   221,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
     222,     0,     0,     0,     0,   182,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   183,    97,    29,     0,     0,
      99,     0,     0,     0,     0,    34,    35,    36,   213,     0,
     214,    40,     0,     0,   104,     0,     0,     0,   215,   107,
     223,     0,     0,     0,     0,   111,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   216,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1146,    75,   217,   218,   219,   220,
     221,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,   222,     0,     0,     0,     0,   182,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   183,    97,
      29,     0,     0,    99,     0,     0,     0,     0,    34,    35,
      36,   213,     0,   214,    40,     0,     0,   104,     0,     0,
       0,   215,   107,   223,     0,     0,     0,     0,   111,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   216,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,   217,
     218,   219,   220,   221,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   222,     0,     0,     0,
       0,   182,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   183,    97,     0,     0,     0,    99,     0,     0,   451,
     452,   453,     0,     0,     0,     0,     0,     0,     0,     0,
     104,     0,     0,     0,     0,   107,   223,     0,     0,   454,
     455,   111,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,     0,   479,   451,   452,   453,
       0,     0,     0,     0,     0,     0,     0,     0,   480,     0,
       0,     0,     0,     0,     0,     0,     0,   454,   455,     0,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,     0,     0,     0,     0,     0,
       0,     0,     0,   451,   452,   453,   480,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   454,   455,   909,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,     0,
     479,   451,   452,   453,     0,     0,     0,     0,     0,     0,
       0,     0,   480,     0,     0,     0,     0,     0,     0,     0,
       0,   454,   455,   977,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,     0,   479,     0,
       0,     0,     0,     0,     0,     0,     0,   451,   452,   453,
     480,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   454,   455,  1033,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,  1092,  1093,  1094,     0,     0,
       0,     0,     0,     0,     0,     0,   480,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1095,  1339,     0,  1096,
    1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1092,  1093,  1094,     0,  1118,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1095,     0,  1370,  1096,  1097,  1098,  1099,  1100,  1101,
    1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,     0,     0,  1092,  1093,
    1094,     0,     0,     0,     0,     0,     0,     0,     0,  1118,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1095,
       0,  1268,  1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1092,  1093,  1094,     0,  1118,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1095,     0,  1438,  1096,  1097,  1098,
    1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,     0,
       0,  1092,  1093,  1094,     0,     0,     0,     0,     0,     0,
       0,     0,  1118,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1095,     0,  1449,  1096,  1097,  1098,  1099,  1100,
    1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1092,  1093,  1094,     0,
    1118,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1095,     0,  1546,
    1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,     0,    34,    35,    36,   213,     0,   214,    40,
       0,     0,     0,     0,     0,  1118,   215,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1638,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   237,
       0,     0,     0,     0,     0,   238,     0,     0,     0,     0,
       0,     0,     0,     0,   217,   218,   219,   220,   221,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,   222,  1640,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,     0,     0,
       0,    99,     0,    34,    35,    36,   213,     0,   214,    40,
       0,     0,     0,     0,     0,   104,   652,     0,     0,     0,
     107,   239,     0,     0,     0,     0,   111,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   216,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   217,   218,   219,   220,   221,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,   222,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,     0,     0,
       0,    99,     0,    34,    35,    36,   213,     0,   214,    40,
       0,     0,     0,     0,     0,   104,   215,     0,     0,     0,
     107,   653,     0,     0,     0,     0,   111,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   237,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   217,   218,   219,   220,   221,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,   222,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,     0,     0,
       0,    99,     0,     0,     0,     0,     0,   451,   452,   453,
       0,     0,     0,     0,     0,   104,     0,     0,     0,     0,
     107,   239,     0,     0,     0,     0,   111,   454,   455,   974,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,   451,   452,   453,     0,     0,
       0,     0,     0,     0,     0,     0,   480,     0,     0,     0,
       0,     0,     0,     0,     0,   454,   455,     0,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,     0,   479,  1092,  1093,  1094,     0,     0,     0,     0,
       0,     0,     0,     0,   480,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1095,  1454,     0,  1096,  1097,  1098,
    1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1092,
    1093,  1094,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1118,     0,     0,     0,     0,     0,     0,     0,
    1095,     0,     0,  1096,  1097,  1098,  1099,  1100,  1101,  1102,
    1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,   453,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1118,     0,
       0,     0,     0,   454,   455,     0,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,  1094,
     479,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   480,     0,     0,     0,     0,     0,  1095,     0,
       0,  1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   454,   455,  1118,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   480
};

static const yytype_int16 yycheck[] =
{
       5,     6,   129,     8,     9,    10,    11,    12,    13,     4,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    56,   186,    29,    30,   161,    33,     4,   404,
       4,   404,    98,   950,     4,   669,   102,   103,   404,    44,
      46,     4,   519,   108,   404,    51,    57,    52,  1170,    54,
     240,   665,    57,   538,    59,   166,    56,  1331,    31,    60,
     187,   695,   128,   161,   479,   515,   516,     4,   939,   511,
     550,    31,    57,    31,   827,    31,   108,   511,   645,   595,
     596,    86,   666,  1157,   356,   357,  1047,    88,   796,   970,
      91,     9,   820,     9,   544,   108,   108,   748,     9,    49,
      44,     9,     9,   108,   546,   986,   586,    14,     9,     9,
       9,    32,   546,    14,    14,     9,     9,    32,     9,     9,
       9,     9,     9,     9,     9,    49,    49,     9,    38,     9,
      83,     9,    38,    38,    70,    83,   251,     9,     9,     9,
       4,   252,    86,   115,    83,    83,     9,     9,     9,    83,
     160,    83,   184,  1034,     0,    83,    70,   786,    36,    38,
     106,   107,    90,   134,   135,    90,    83,   483,   543,    50,
      51,   184,   184,    83,   160,   102,    49,    83,    83,   184,
      49,   786,   181,   134,   135,   181,   191,   667,   196,   199,
     102,   223,   134,   135,   181,   181,   512,   196,   134,   135,
     196,   517,   106,   107,    83,   196,   178,   239,   111,   196,
     223,   223,    70,    70,    70,    83,    84,  1088,   223,    54,
      70,   174,    70,     4,   161,   196,   239,    70,   160,   157,
     158,   159,   157,    70,   239,   174,   174,   164,    70,    70,
     174,   158,   159,   193,    70,   879,   160,   197,   253,   200,
      70,   256,   164,   201,   200,    70,    70,   235,   263,   264,
      70,    70,   257,   199,   174,   129,   261,    70,   174,   174,
     193,    70,    53,   197,   389,    56,    70,   198,   441,   197,
     181,   989,   198,  1557,   199,   199,  1247,   198,   199,     8,
     198,   198,    73,  1367,    54,   174,   200,   198,   198,   198,
    1374,   182,  1376,   197,  1057,   198,  1059,   198,   198,   198,
     198,   198,   198,   198,   348,    96,   198,    98,   198,   197,
    1201,   102,   103,   187,   197,   197,   197,   197,   197,   160,
     165,  1405,   812,   201,   197,   197,   197,   817,   196,   196,
     196,   199,   199,   199,   376,   160,    14,   128,   348,   199,
     160,   199,    38,   196,  1225,   356,   357,   358,   521,   875,
     876,   928,   199,   376,   376,   196,   493,   199,   199,   435,
     375,   376,    90,   199,   166,  1004,   196,   382,   383,   384,
     385,   386,   387,   967,   199,   199,   196,   392,  1771,   199,
     199,   160,    57,    81,   160,   396,   199,    83,   432,  1004,
     199,  1465,   196,   197,    69,   165,   411,   199,   122,    83,
      84,    83,   102,   196,   419,   181,   130,   196,    90,   200,
     180,    83,   488,   489,   490,   491,   431,   196,   905,   494,
     196,   375,   432,   199,   102,   103,   410,  1511,    70,   157,
     384,   385,   130,   387,    70,  1828,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   164,   480,   257,   482,   483,   484,
     261,   418,   199,   494,   265,   485,   158,   159,   181,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,  1566,   196,  1155,   196,   479,   512,   513,   494,
     515,   516,   517,   518,   956,  1243,    70,   522,   196,   479,
     525,   479,   956,   479,    38,   199,  1590,   201,  1592,   534,
     192,   536,   532,   983,   385,    14,   387,   199,    70,   544,
    1085,   552,  1087,   102,    83,   196,   181,   552,   683,   554,
     528,    90,   742,    32,   418,  1169,    83,   672,    83,   674,
    1166,   196,   557,    90,   939,    90,   939,   348,   196,    83,
     543,   123,    51,   939,  1054,  1446,   991,   196,   130,   939,
     134,   135,   485,   196,   774,   683,     4,   750,  1217,  1218,
    1219,  1220,  1221,  1222,   595,   596,   601,   869,  1227,   871,
     196,  1185,   134,   135,  1188,   164,   160,   196,   511,    27,
      28,   166,  1217,  1218,   165,  1131,  1221,    70,    83,   158,
     159,   653,  1227,    70,   179,    90,    75,    76,   492,   532,
     157,   158,   159,   158,   159,   701,    70,   418,    70,   160,
     543,    70,   196,   546,   199,    83,   164,   428,   653,   627,
     628,   432,    50,    51,   435,  1220,    83,  1222,  1529,   111,
     181,   196,   852,    90,  1938,    31,   205,   119,   120,   121,
     122,   123,   124,    32,   864,   790,   791,   655,   199,   204,
     198,   199,   797,   798,    50,   196,   797,    53,   693,   164,
     108,    38,   666,   158,   159,   119,   120,   121,   122,   123,
     124,   706,    75,    76,   485,   486,   487,   488,   489,   490,
     491,   102,   103,  1088,    31,  1088,   132,   133,  1203,   157,
     158,   159,  1088,   199,  1375,   105,   106,   107,  1088,   198,
     511,   158,   159,    50,   198,   740,    53,   390,   198,   191,
    1354,   394,   198,   199,  1194,    70,   683,    53,    54,    55,
     198,   532,   198,   199,  1383,  1205,  1385,   198,   160,  1365,
     738,   198,   164,    69,   769,   546,   184,   191,   421,  1246,
     423,   424,   425,   426,   198,   198,   557,   198,  1383,   181,
    1385,   105,   106,   107,   160,  1369,    27,    28,   119,   120,
     121,   122,   123,   124,  1916,    70,   577,  1802,  1803,   804,
      53,    54,    55,    70,    57,   223,   356,   357,  1930,    70,
     816,   119,   120,   121,   232,   820,    69,   196,   599,   600,
      70,   239,    70,    50,    51,    52,    53,    54,    55,  1480,
     825,  1798,  1799,   119,   120,   121,   122,   123,   124,   257,
      70,  1321,    69,   261,   130,   131,   112,   113,   114,    70,
    1225,   829,  1225,   634,   635,   837,   838,   835,  1335,  1225,
     191,   196,   160,   196,   164,  1225,   198,    48,   869,    69,
     871,   181,   160,   810,   875,   876,   877,   196,   196,     9,
     203,   160,   160,   196,     8,   171,   198,   173,   160,  1518,
     160,  1520,   196,  1522,  1500,    14,  1502,   198,  1527,   198,
     186,     9,   188,   199,   909,   191,   911,   198,   913,    14,
     130,   130,   197,  1518,    14,  1520,  1567,  1522,   181,   924,
     701,  1371,  1527,   901,   990,   102,   197,  1407,  1512,   866,
       9,   196,   927,   938,   202,   197,   111,   197,   197,   196,
     196,   157,   942,  1423,    94,   197,   810,   365,   197,     9,
     197,   927,   197,   927,   198,    14,   374,   927,   376,   964,
     196,     9,   196,   381,   927,  1882,   181,   199,   199,   974,
     198,    83,   977,   198,   979,   393,   950,   132,   983,   197,
     197,   203,     9,   197,   196,  1902,  1176,   198,    27,    28,
     927,   232,  1621,   967,  1911,   197,     9,   203,   203,   203,
     418,    70,   866,    32,  1610,   786,   203,   788,  1614,   180,
     133,   948,   160,     9,   136,   197,  1621,   160,   991,    50,
      51,    52,    53,    54,    55,    14,    57,   193,  1033,   810,
       9,   991,     9,   991,  1040,   991,   182,   197,    69,   942,
       9,    14,   132,   824,   825,   203,  1041,   203,  1498,   200,
    1028,   954,  1532,   956,     9,   119,   120,   121,   122,   123,
     124,  1541,    14,   927,   203,   197,   130,   131,   197,   203,
     196,  1446,   160,  1446,   102,  1555,   197,     9,   198,   136,
    1446,   198,   160,  1148,   948,   866,  1446,     9,   197,   196,
    1090,     9,   873,   874,    70,    70,  1074,   196,    70,    70,
      70,   519,   199,  1081,   200,    14,  1043,   198,  1045,   173,
     182,     9,   199,    14,   203,   199,   197,    14,   198,   193,
      32,   902,    32,   196,   365,   196,    14,   191,   196,   196,
    1131,    14,  1035,   374,   196,   376,    52,  1148,   196,   557,
     381,    70,    70,  1148,    81,    70,   927,  1776,  1777,    70,
      70,  1631,   393,   160,  1529,     9,  1529,   197,   136,  1154,
     198,   942,   198,  1529,  1770,    14,   103,   948,  1774,  1529,
     196,  1776,  1777,   954,   182,   956,  1181,   136,  1154,  1043,
    1154,  1045,     9,   197,  1154,    69,   160,   203,  1822,  1194,
       9,  1154,    83,   232,   200,   200,   198,     9,   196,   196,
    1205,  1206,   139,   140,   141,   142,   143,   198,    14,   990,
     136,  1185,    83,   197,  1188,   196,   199,  1154,   203,   199,
       9,  1002,  1003,  1004,   161,   136,    91,   164,   196,  1235,
     167,   168,   197,   170,   171,   172,   199,  1215,  1243,   199,
     198,    32,   157,  1238,    77,   198,   197,   182,  1253,   198,
     136,    32,   197,   197,  1035,   203,     9,   203,   195,     9,
    1041,   203,  1043,   200,  1045,   203,  1900,   203,   136,     9,
     197,     9,    14,    83,   692,   198,   200,     9,   519,   198,
     197,   136,     9,   196,   136,  1066,   200,     9,  1768,   196,
    1154,   199,   197,    78,    79,    80,  1274,   197,  1778,   198,
    1278,   197,  1936,   197,  1910,  1283,    91,   199,   197,  1090,
     203,     4,  1290,  1250,    32,   136,   203,   197,   203,  1925,
     198,   203,   203,   197,   197,   112,   365,   199,   198,   198,
     169,   198,    14,   751,  1339,   374,   165,    83,  1119,   117,
     197,  1346,   381,  1823,   197,  1350,  1481,  1352,   199,  1476,
     136,   197,   136,    14,   393,  1360,    49,   181,   199,  1333,
     145,   146,   147,   148,   149,  1370,  1371,   198,    83,  1343,
      14,   156,    14,  1154,   792,    83,   794,   162,   163,   197,
     196,   136,   197,    14,   136,   198,  1250,   198,  1868,    14,
     198,   176,   810,    14,   199,  1369,  1333,     9,     9,   200,
      59,   181,    83,   196,   822,   190,  1343,   825,    83,     9,
     115,   199,  1390,   102,   102,   198,  1394,   160,   182,   112,
      36,   172,    14,  1401,   117,  1588,   119,   120,   121,   122,
     123,   124,   125,   196,   178,   197,  1217,  1218,  1219,  1220,
    1221,  1222,   198,   182,    83,   182,  1227,   196,   866,   175,
     197,   692,     9,    83,   199,  1935,   197,  1238,    14,    83,
    1940,     9,   198,   197,   195,    14,    83,    14,    83,  1250,
      14,   889,  1131,    83,   167,   168,    31,   170,  1891,  1260,
     519,   491,   486,   984,  1907,   488,   930,   905,   906,  1244,
    1495,  1630,  1902,  1498,  1560,  1617,  1655,   603,   191,  1568,
    1420,  1740,  1471,  1947,  1923,    27,    28,   200,  1752,   927,
     751,  1613,  1467,  1086,  1221,  1158,  1082,  1003,  1216,  1456,
     386,  1217,  1030,   954,   382,  1934,    81,  1501,   432,  1949,
     948,  1857,   837,  1507,  1139,  1509,    91,  1459,  1512,  1067,
    1119,    -1,    -1,  1538,  1481,    -1,  1327,    -1,   103,    -1,
      -1,   792,    -1,   794,    -1,    -1,    -1,  1531,    -1,    -1,
      -1,    -1,    -1,  1563,  1501,    -1,    -1,    -1,    -1,    -1,
    1507,    -1,  1509,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   822,    -1,    -1,   139,   140,   141,   142,   143,    -1,
      -1,    -1,  1456,  1756,  1531,    -1,  1533,    -1,    -1,    -1,
    1605,    -1,  1383,    -1,  1385,  1542,   161,    -1,    -1,   164,
      -1,  1475,   167,   168,    -1,   170,   171,   172,    -1,   174,
      -1,    -1,    -1,  1041,    -1,  1043,    -1,  1045,    -1,  1047,
    1048,    -1,    -1,    -1,  1629,  1630,    -1,    -1,    -1,    -1,
     195,    -1,    -1,    -1,    -1,  1619,    -1,    -1,   889,    -1,
      -1,    -1,    -1,   692,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   905,   906,    -1,    -1,    -1,  1533,
      -1,    -1,    -1,    -1,    -1,  1456,    27,    28,  1542,    -1,
      31,    -1,  1619,    -1,  1465,    -1,    -1,    -1,    -1,    -1,
    1471,  1628,    -1,    -1,    -1,    -1,    -1,  1634,    -1,    -1,
      -1,    -1,    -1,    -1,  1641,    56,    -1,    -1,    -1,    -1,
     232,    -1,   751,    -1,    -1,    -1,    -1,  1751,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,  1596,    -1,    -1,    -1,  1154,  1518,    -1,  1520,
    1875,  1522,    -1,    -1,    -1,    -1,  1527,    -1,    -1,    -1,
    1816,  1751,  1533,   792,    -1,   794,  1761,  1538,    -1,    -1,
      -1,  1542,  1180,    -1,  1628,    -1,    -1,    -1,  1895,    -1,
    1634,    67,    68,    -1,    -1,    -1,    -1,  1641,    -1,  1560,
      -1,    -1,  1563,   822,    -1,  1566,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1576,    -1,    -1,    -1,    -1,
      -1,    -1,  1583,    -1,    -1,    -1,  1047,  1048,    -1,  1590,
      -1,  1592,    -1,  1231,  1792,  1793,    -1,  1598,    -1,    -1,
    1238,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1246,  1247,
      -1,    -1,  1250,    -1,    -1,    -1,    -1,    -1,   134,   135,
    1621,    -1,    -1,   365,    -1,  1782,    -1,  1628,  1629,  1630,
     889,    -1,   374,  1634,    -1,    -1,    -1,    -1,    -1,   381,
    1641,    -1,    -1,    -1,    -1,     4,   905,   906,    -1,    -1,
      -1,   393,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   232,   404,    -1,    -1,    -1,    -1,  1824,    -1,    -1,
      -1,    -1,    -1,    -1,  1831,    -1,    -1,    -1,    -1,    -1,
      -1,   197,    -1,    -1,    -1,    -1,    -1,    56,  1882,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    -1,  1335,  1782,    -1,
      -1,    -1,    -1,    -1,    -1,    56,    -1,    -1,  1902,  1866,
      -1,   282,    -1,   284,    -1,    -1,    -1,  1911,  1875,  1180,
    1945,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1888,  1957,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1824,    -1,  1967,    -1,    -1,  1970,    -1,  1831,    -1,    -1,
    1751,    -1,    -1,   112,    -1,    -1,    -1,    -1,   117,    -1,
     119,   120,   121,   122,   123,   124,   125,    -1,    -1,   340,
    1231,  1772,    -1,    -1,    -1,  1776,  1777,   519,    -1,    -1,
      -1,  1782,  1866,    -1,    -1,  1246,  1247,    -1,  1047,  1048,
    1791,  1948,    81,    -1,   365,    -1,  1953,  1798,  1799,    -1,
      -1,  1802,  1803,   374,  1888,    -1,    -1,    -1,   167,   168,
     381,   170,    -1,    -1,   103,  1816,    -1,    -1,  1456,    -1,
      -1,    -1,   393,  1824,    -1,    -1,    -1,    -1,    -1,    -1,
    1831,    -1,   191,   404,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   200,    -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,
     139,   140,   141,   142,   143,    -1,    -1,    -1,   429,    -1,
      -1,   432,     4,    -1,  1948,  1866,    -1,    -1,    -1,  1953,
      -1,    -1,    -1,  1874,  1335,   164,    -1,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,  1888,    -1,    -1,
      -1,    -1,    49,  1894,    -1,  1533,    -1,    -1,    -1,    -1,
    1538,    -1,    -1,    -1,  1542,    -1,   195,    49,   479,    -1,
     199,    -1,    -1,   282,    -1,   284,    -1,    -1,    -1,    -1,
      -1,  1180,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   282,    -1,   284,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1948,   519,     4,
     692,    -1,  1953,    -1,    -1,   112,    -1,    -1,    -1,    -1,
     117,    -1,   119,   120,   121,   122,   123,   124,   125,     4,
     112,   340,  1231,    -1,    -1,   117,    -1,   119,   120,   121,
     122,   123,   124,   125,    -1,    -1,     4,  1246,  1247,   340,
    1628,  1629,  1630,    -1,    49,    -1,  1634,    -1,    -1,    -1,
     571,    -1,   573,  1641,    -1,   576,    -1,    -1,    -1,   751,
     167,   168,    -1,   170,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   167,   168,    -1,   170,    -1,
      -1,    49,    -1,    -1,   191,    -1,   607,    -1,    -1,    -1,
      -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,   191,
     792,    -1,   794,    -1,    -1,    -1,    -1,   112,   200,    -1,
     429,    -1,   117,   432,   119,   120,   121,   122,   123,   124,
     125,    -1,    -1,    -1,    -1,    -1,  1335,   112,   429,    -1,
     822,   432,   117,    -1,   119,   120,   121,   122,   123,   124,
     125,   662,   663,    -1,   112,    -1,    -1,    -1,    -1,   117,
     671,   119,   120,   121,   122,   123,   124,   125,    -1,    -1,
      -1,    -1,   167,   168,    -1,   170,    -1,    -1,    -1,    -1,
      -1,   692,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   167,   168,    -1,   170,   191,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1782,   200,    -1,   889,    -1,   167,
     168,    -1,   170,    -1,    -1,    -1,   191,    -1,    -1,    -1,
      -1,    -1,    -1,   905,   906,   200,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   191,    -1,    -1,    -1,    -1,    -1,    -1,
     751,    -1,   200,    -1,    -1,    -1,  1824,    81,    -1,    83,
      -1,    85,    -1,  1831,    -1,    -1,    -1,   939,    -1,    -1,
      -1,    -1,   571,    -1,   573,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,   786,    -1,    -1,    -1,    -1,
     571,   792,    -1,   794,    -1,    -1,    -1,    -1,  1866,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,   139,   140,   141,   142,   143,
    1888,   822,   823,    -1,    -1,    -1,    -1,    -1,    -1,   830,
      -1,    -1,    78,    79,    80,    81,   837,   838,   839,   840,
     841,   842,   843,   167,   168,    -1,   170,   171,   172,    -1,
     851,    -1,    67,    68,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,   662,   663,    31,   867,    -1,    -1,    -1,
      -1,   195,   671,    -1,    -1,  1047,  1048,    -1,    -1,    -1,
    1948,   662,   663,    -1,    -1,  1953,    -1,    -1,   889,    -1,
     671,    -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,   903,    -1,   905,   906,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,  1088,    -1,    -1,   134,
     135,   167,   168,    -1,   170,   171,   172,    -1,   929,   930,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,   939,    -1,
      -1,    -1,    -1,    -1,   945,    -1,    -1,    -1,    -1,   195,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   958,    -1,   125,
      -1,    -1,    -1,    -1,    -1,   966,    -1,    -1,   969,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,   144,    -1,
      -1,    -1,   197,    -1,    -1,    -1,   987,    -1,    -1,    -1,
     991,    -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,
      -1,   167,   168,  1004,   170,   171,   172,    -1,  1180,    -1,
      -1,    -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,   195,
      -1,   830,    -1,    -1,    -1,    -1,    -1,    -1,   837,   838,
      -1,    -1,    -1,    -1,    -1,    -1,  1047,  1048,    -1,    -1,
      -1,    -1,    -1,  1225,    -1,    -1,   837,   838,    -1,  1231,
    1061,    -1,    -1,    -1,  1065,    -1,  1067,    81,    67,    68,
      -1,    -1,    -1,    -1,  1246,  1247,    -1,    -1,    -1,    -1,
      -1,  1082,  1083,  1084,  1085,  1086,  1087,  1088,    -1,   103,
    1091,  1092,  1093,  1094,  1095,  1096,  1097,  1098,  1099,  1100,
    1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     929,    -1,    -1,    -1,  1135,   134,   135,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   945,    -1,   929,    -1,
      -1,   165,    -1,   167,   168,   169,   170,   171,   172,   958,
      -1,    -1,    -1,  1335,   945,    -1,    -1,  1168,    -1,  1170,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   958,    -1,  1180,
      -1,   195,   196,    -1,    -1,    -1,    -1,    -1,   987,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1197,    -1,   197,  1200,
      -1,    -1,    -1,    -1,    -1,    -1,   987,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1217,  1218,  1219,  1220,
    1221,  1222,    -1,    -1,  1225,    -1,  1227,    -1,    -1,    -1,
    1231,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      31,    -1,    -1,    -1,    -1,  1246,  1247,    -1,  1249,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1259,    -1,
      -1,    -1,  1061,    -1,  1265,    -1,  1065,  1268,  1067,  1270,
      -1,    -1,    -1,    -1,  1446,    -1,    10,    11,    12,    -1,
    1061,    -1,    -1,    -1,  1065,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1294,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,  1328,  1329,    -1,
      -1,  1332,    -1,    -1,  1335,    69,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1529,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1168,
      -1,  1170,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1383,    -1,  1385,    -1,    -1,  1168,    -1,  1170,
      67,    68,    -1,    -1,    -1,    -1,    -1,    -1,  1197,    -1,
      -1,  1200,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,  1197,    -1,    -1,  1200,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1428,    -1,  1430,
      -1,    -1,    -1,    -1,    -1,  1436,    -1,  1438,    -1,  1440,
      -1,   232,    -1,    -1,  1445,  1446,    -1,    -1,  1449,    81,
    1451,    -1,    -1,  1454,    -1,    67,    68,   134,   135,    -1,
    1259,    10,    11,    12,  1465,  1466,  1265,    -1,  1469,   203,
      -1,   103,    -1,    -1,    -1,  1476,    -1,    -1,  1259,    -1,
      -1,    -1,    31,    -1,  1265,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,  1518,    -1,  1520,
      69,  1522,   134,   135,    -1,    -1,  1527,    -1,  1529,  1328,
    1329,    -1,    -1,    -1,    -1,   167,   168,    -1,   170,   171,
     172,    -1,  1543,    -1,    -1,  1546,    -1,  1328,  1329,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1558,  1559,    -1,
      -1,    -1,    -1,   195,   196,  1566,    -1,  1568,    -1,    -1,
      -1,    -1,    -1,    -1,   365,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   374,    -1,    -1,    -1,    -1,    -1,  1590,
     381,  1592,    -1,    -1,    -1,    -1,    -1,  1598,    -1,    -1,
      -1,    -1,   393,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   404,    -1,    -1,    -1,    -1,    -1,    -1,
    1621,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1428,
      -1,  1430,    -1,    -1,    -1,    -1,    -1,  1638,  1639,  1640,
      -1,    -1,    -1,    -1,  1645,    -1,  1647,  1428,   197,  1430,
      -1,    -1,  1653,    -1,  1655,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1465,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,  1476,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   479,    -1,
      -1,    -1,    31,    30,    31,  1476,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   519,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,  1543,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,  1755,    -1,    -1,    -1,    31,    -1,
      -1,    -1,  1543,    -1,   103,    -1,    -1,  1566,    -1,    -1,
      -1,  1772,    -1,    -1,    -1,  1776,  1777,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   576,    59,    -1,    -1,    -1,
    1791,  1590,    -1,  1592,    -1,    -1,  1797,    -1,    -1,  1598,
     139,   140,   141,   142,   143,    -1,    -1,  1808,    81,    -1,
      -1,    -1,    -1,  1814,    -1,    -1,   607,  1818,    -1,    -1,
      -1,    -1,   161,    -1,    -1,   164,    -1,    -1,   167,   168,
     103,   170,   171,   172,    -1,   174,    -1,    -1,   111,  1840,
      -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,  1653,    -1,   195,    -1,    -1,    -1,
      -1,    -1,    -1,   200,    -1,   138,   139,   140,   141,   142,
     143,   144,  1653,    -1,    31,    -1,    -1,    -1,    -1,  1880,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,   161,  1890,
      31,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,   692,    59,    -1,    -1,    -1,  1907,    -1,    -1,    -1,
      -1,    -1,   185,    -1,    -1,  1916,    -1,    -1,   191,    -1,
      -1,    -1,   195,   196,    81,    -1,    -1,  1928,    -1,  1930,
      -1,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,   103,    -1,  1949,    -1,
    1951,    -1,    -1,    -1,   111,    -1,  1755,    69,    -1,    -1,
     751,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1772,  1755,    81,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,  1791,    -1,    -1,   786,    -1,   103,    -1,    -1,
      -1,   792,    -1,   794,   161,    -1,    -1,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,    -1,
      81,    -1,    83,    84,    -1,    -1,    -1,    -1,   185,    -1,
      -1,   822,   823,   139,   140,   141,   142,   143,   195,   196,
      -1,  1840,   103,    -1,    -1,    -1,    -1,    31,   839,   840,
     841,   842,   843,    -1,    -1,    -1,    -1,    -1,    -1,  1840,
     851,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    59,   867,    -1,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   195,
     196,   232,    -1,    -1,    -1,    31,    -1,    81,   889,    -1,
      -1,    -1,    -1,    -1,   165,    -1,   167,   168,    -1,   170,
     171,   172,   903,    -1,   905,   906,    -1,  1916,    -1,   103,
      -1,    -1,    -1,    59,    -1,    -1,    -1,    -1,    -1,  1928,
      -1,  1930,    -1,    -1,   195,  1916,    -1,    -1,   199,   930,
     201,    -1,    -1,    -1,    -1,    81,    -1,  1928,   939,  1930,
    1949,    -1,  1951,    -1,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,   103,  1949,    -1,
    1951,    -1,    -1,    -1,    -1,   966,    -1,   161,   969,    -1,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
     174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     991,   185,   138,   139,   140,   141,   142,   143,   144,    -1,
      -1,   195,   196,  1004,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   365,   161,    -1,    -1,   164,   165,
      -1,   167,   168,   374,   170,   171,   172,    -1,   174,    -1,
     381,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,   185,
      -1,    -1,   393,    -1,    -1,    -1,  1047,  1048,    -1,   195,
     196,    31,    -1,   404,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,  1082,  1083,  1084,  1085,  1086,  1087,  1088,    -1,    69,
    1091,  1092,  1093,  1094,  1095,  1096,  1097,  1098,  1099,  1100,
    1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,   479,    -1,
      -1,    -1,    -1,    -1,  1135,    -1,    -1,    -1,    -1,    30,
      31,   576,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,   519,    50,
      51,    -1,   607,    -1,    -1,    -1,    -1,    -1,    69,  1180,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    81,    -1,    83,    84,  1217,  1218,  1219,  1220,
    1221,  1222,   103,    -1,  1225,   576,  1227,    -1,    -1,    -1,
    1231,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1246,  1247,    -1,  1249,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   607,   138,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,  1268,    -1,  1270,
     139,   140,   141,   142,   143,   156,    -1,    -1,    -1,    -1,
     161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,  1294,    -1,    -1,   165,    -1,   167,   168,
      -1,   170,   171,   172,   185,    -1,    -1,    -1,    -1,   200,
      -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,
     199,  1332,   201,    -1,  1335,    -1,    -1,    -1,    -1,    30,
      31,   692,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    81,    57,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,  1383,    -1,  1385,    -1,    -1,   103,   823,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
     751,    -1,    -1,    -1,   839,   840,   841,   842,   843,    -1,
      -1,    -1,   128,    -1,    -1,    -1,   851,    -1,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,   139,
     140,   141,   142,   143,    -1,  1436,    -1,  1438,    -1,  1440,
      -1,   792,    -1,   794,  1445,  1446,    -1,    -1,  1449,    -1,
    1451,   167,   168,  1454,   170,   171,   172,   167,   168,    -1,
     170,   171,   172,    -1,    -1,  1466,    -1,    -1,  1469,    -1,
      31,   822,   823,    -1,    -1,    -1,    -1,    -1,    -1,   195,
     196,    -1,    -1,    -1,    -1,   195,    -1,    -1,   839,   840,
     841,   842,   843,    -1,    -1,    -1,    -1,    -1,    59,    -1,
     851,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,   200,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1518,    -1,  1520,
      81,  1522,    -1,    -1,    -1,   103,  1527,    -1,  1529,    -1,
      -1,   966,    -1,   111,   112,    -1,    -1,    -1,   889,    -1,
      -1,    -1,   103,    -1,    -1,  1546,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   905,   906,    -1,  1558,  1559,    -1,
      -1,   139,   140,   141,   142,   143,    -1,  1568,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,   164,    -1,   939,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,    -1,    -1,
     161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,   966,    -1,   195,    -1,    -1,
    1621,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   195,   196,    -1,  1638,  1639,  1640,
     991,    -1,    -1,    -1,  1645,    -1,  1647,  1082,  1083,    -1,
      -1,  1086,    -1,    -1,  1655,    -1,  1091,  1092,  1093,  1094,
    1095,  1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1047,  1048,    -1,    -1,
    1135,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,  1082,  1083,  1084,  1085,  1086,  1087,  1088,    -1,    69,
    1091,  1092,  1093,  1094,  1095,  1096,  1097,  1098,  1099,  1100,
    1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1776,  1777,    -1,    -1,    -1,
      81,    -1,    -1,    -1,  1135,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1797,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,  1808,    -1,    -1,
      -1,    -1,    -1,  1814,  1249,    -1,    -1,  1818,    -1,    -1,
      -1,    -1,    -1,    -1,   125,    81,    -1,    -1,    -1,  1180,
      -1,    -1,    -1,  1268,    -1,  1270,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,   112,    -1,    -1,  1294,
     161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,  1225,    -1,    -1,    -1,    -1,  1880,
    1231,    -1,    -1,   139,   140,   141,   142,   143,    -1,  1890,
      -1,    -1,    -1,    -1,   195,  1246,  1247,    -1,  1249,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1907,    -1,   164,    -1,
      -1,   167,   168,    -1,   170,   171,   172,  1268,    -1,  1270,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
      -1,    -1,    -1,  1294,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,  1335,    -1,    -1,    -1,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,  1436,    -1,  1438,    -1,  1440,    -1,    -1,    -1,    -1,
    1445,    -1,    -1,    -1,  1449,    -1,  1451,    -1,    -1,  1454,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1436,    -1,  1438,   103,  1440,
      -1,    -1,    -1,    -1,  1445,  1446,   111,    -1,  1449,    -1,
    1451,    -1,    -1,  1454,   119,   120,   121,   122,   123,   124,
      -1,  1546,   127,   128,   200,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,   174,
      -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
     185,    -1,    -1,    -1,    -1,   190,   191,   192,  1529,    -1,
     195,   196,    -1,    -1,    81,    -1,   201,   202,    -1,   204,
     205,    10,    11,    12,    91,  1546,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1638,  1639,  1640,   103,    -1,    -1,    -1,
    1645,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      69,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,   161,    -1,    -1,   164,    -1,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,  1638,  1639,  1640,
      -1,    -1,    -1,    -1,  1645,    -1,    -1,    -1,   195,    -1,
      -1,    -1,    -1,  1654,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,  1797,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,  1808,    -1,    69,    -1,    -1,    -1,  1814,
      -1,   200,    -1,  1818,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1880,  1797,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,  1808,    -1,    -1,
      -1,    -1,    -1,  1814,    -1,    -1,    -1,  1818,    -1,    -1,
      -1,    49,    50,    51,   200,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,  1842,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,
      98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,  1880,
     108,   109,   110,   111,   112,   113,   114,    -1,   116,   117,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,   129,   130,   131,   200,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,   153,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,   173,    -1,    -1,   176,    -1,
      -1,   179,    -1,    -1,    -1,    -1,    -1,   185,   186,    -1,
     188,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
     198,   199,   200,   201,   202,    -1,   204,   205,     3,     4,
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
      -1,    -1,    -1,   108,   109,   110,   111,   112,   113,   114,
      -1,   116,   117,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,   129,   130,   131,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,   153,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,   173,    -1,
      -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
     185,   186,    -1,   188,    -1,   190,   191,   192,    -1,    -1,
     195,   196,    -1,   198,   199,   200,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
     112,   113,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,   123,   124,    -1,   126,   127,   128,   129,   130,   131,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,   153,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,   173,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,
      -1,    -1,    -1,   185,   186,    -1,   188,    -1,   190,   191,
     192,    -1,    -1,   195,   196,    -1,   198,   199,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
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
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,   153,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,   173,    -1,    -1,   176,    -1,    -1,
     179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,   198,
     199,   200,   201,   202,    -1,   204,   205,     3,     4,     5,
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
      -1,   137,   138,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,   153,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,   173,    -1,    -1,
     176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,
      -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,
     196,    -1,   198,   199,   200,   201,   202,    -1,   204,   205,
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
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
     153,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
     173,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,
      -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,
      -1,    -1,   195,   196,    -1,   198,   199,   200,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
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
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,   153,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,   173,    -1,    -1,   176,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,   198,   199,
     200,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      87,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,    -1,   113,   114,    -1,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,   153,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,   173,    -1,    -1,   176,
      -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,
      -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,
      -1,   198,   199,    -1,   201,   202,    -1,   204,   205,     3,
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
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,   153,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,   173,
      -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,
      -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     111,    -1,   113,   114,    -1,   116,    -1,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,   127,   128,    -1,   130,
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,   153,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,   173,    -1,    -1,   176,    -1,    -1,   179,    -1,
      -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,
     191,   192,    -1,    -1,   195,   196,    -1,   198,   199,   200,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,   153,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,   173,    -1,    -1,   176,    -1,
      -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
     198,   199,   200,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,    94,
      -1,    96,    -1,    98,    99,   100,    -1,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,   114,
      -1,   116,    -1,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,   153,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,   173,    -1,
      -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
     185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,
     195,   196,    -1,   198,   199,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,   153,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,   173,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,
      -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,
     192,    -1,    -1,   195,   196,    -1,   198,   199,   200,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
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
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,   153,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,   173,    -1,    -1,   176,    -1,    -1,
     179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,   198,
     199,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
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
      -1,   137,   138,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,   153,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,   173,    -1,    -1,
     176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,
      -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,
     196,    -1,   198,   199,   200,   201,   202,    -1,   204,   205,
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
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
     153,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
     173,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,
      -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,
      -1,    -1,   195,   196,    -1,   198,   199,   200,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
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
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,   153,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,   173,    -1,    -1,   176,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,   198,   199,
     200,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
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
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,   153,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,   173,    -1,    -1,   176,
      -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,
      -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,
      -1,   198,   199,   200,   201,   202,    -1,   204,   205,     3,
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
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,   153,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,   173,
      -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,
      -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,   153,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,    -1,
      -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,
     191,   192,    -1,    -1,   195,   196,    -1,   198,   199,    -1,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,
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
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,   153,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
     198,   199,    -1,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,    94,
      -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,   114,
      -1,   116,    -1,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,   153,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
     185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,
     195,   196,    -1,   198,   199,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,   153,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,
      -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,
     192,    -1,    -1,   195,   196,    -1,   198,   199,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
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
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,   153,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,
     179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,   198,
     199,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,    -1,   108,   109,   110,   111,    -1,   113,   114,    -1,
     116,    -1,   118,   119,   120,   121,   122,   123,   124,    -1,
     126,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,   153,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
     176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,
      -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,
     196,    -1,   198,   199,    -1,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,
      -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,
      -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,   198,    -1,
      -1,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,
      -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,
      -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,
      -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    31,    -1,    13,
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
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
     174,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,
      -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,
      -1,   195,   196,    -1,    -1,    -1,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,    -1,
      -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,
     191,   192,    -1,    -1,   195,   196,    10,    11,    12,    -1,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    69,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
     108,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      -1,   179,    -1,    -1,    -1,    -1,   200,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
      -1,    -1,    -1,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
     185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,
     195,   196,    10,    11,    12,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    69,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,
      -1,    -1,   200,   185,    -1,    -1,    -1,    -1,   190,   191,
     192,    -1,    -1,   195,   196,    -1,   198,    11,    12,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
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
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,
     179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,   198,
      -1,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
     176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,
      -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,
     196,    10,    11,    12,    -1,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      69,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,
      -1,   200,   185,    -1,    -1,    -1,    -1,   190,   191,   192,
      -1,    -1,   195,   196,   197,    -1,    -1,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,    -1,    -1,
      -1,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,
      -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,
      -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,
      -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,
     124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
      -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,
      -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,
      -1,   195,   196,    -1,    -1,    -1,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,    -1,
      -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,
     191,   192,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    10,
      11,    12,    -1,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    69,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,   176,    -1,    -1,   179,    -1,    -1,   198,    -1,    -1,
     185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,
     195,   196,    10,    11,    12,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      28,    13,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    69,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,   102,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,     3,     4,   176,     6,     7,   179,    -1,    10,
      11,    12,    13,   185,    -1,    -1,    -1,    -1,   190,   191,
     192,    -1,    -1,   195,   196,    -1,    27,    -1,    29,   201,
     202,    -1,   204,   205,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,    -1,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,    -1,    -1,    10,    11,    12,    13,
     161,    -1,    -1,    -1,    -1,    -1,   167,   168,    -1,   170,
     171,   172,   173,    27,   175,    29,    -1,   178,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,   199,    -1,
     201,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,    81,   130,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,   161,    10,    11,
      12,    13,    -1,   167,   168,    -1,   170,   171,   172,   173,
      -1,   175,    -1,    -1,   178,    27,    -1,    29,    -1,    31,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,   195,    -1,    -1,    -1,   199,    -1,   201,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    59,    -1,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,    -1,    -1,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,
      -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,   173,    -1,   175,    -1,    -1,   178,     3,     4,    -1,
       6,     7,    -1,   185,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,   195,   196,    -1,    -1,    -1,   200,    -1,
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
     126,    -1,   128,    -1,    -1,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,   173,    -1,   175,
      -1,    -1,   178,     3,     4,    -1,     6,     7,    -1,   185,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   195,
     196,    -1,    -1,    -1,   200,    -1,    -1,    27,    -1,    29,
      -1,    31,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    59,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,    -1,
     130,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,   173,    -1,   175,    -1,    -1,   178,     3,
       4,    -1,     6,     7,    -1,   185,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    29,    -1,    31,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    57,    -1,    59,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,    -1,    -1,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,   173,
     103,   175,    -1,    -1,   178,    -1,     3,     4,    -1,     6,
       7,   185,   186,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    29,    -1,    31,    -1,   139,   140,   141,   142,
     143,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      57,   164,    59,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    69,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,   173,   103,   175,    -1,
      -1,   178,     3,     4,     5,     6,     7,    -1,   185,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,   195,   196,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   161,    57,    -1,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     161,   162,   163,    -1,    81,    -1,   167,   168,    -1,   170,
     171,   172,   173,    -1,   175,   176,    -1,   178,    -1,    -1,
      -1,    -1,    -1,    -1,   185,   186,   103,   188,    -1,   190,
     191,    -1,     3,     4,   195,     6,     7,    -1,    -1,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,
      -1,    -1,   139,   140,   141,   142,   143,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    57,   164,    -1,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    69,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,    -1,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,    -1,    -1,    10,    11,    12,    13,
     161,    -1,    -1,    -1,    -1,    -1,   167,   168,    -1,   170,
     171,   172,   173,    27,   175,    29,    -1,   178,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,    -1,   130,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,    -1,    10,    11,    12,    13,   161,    -1,    -1,
      -1,    -1,    -1,   167,   168,    -1,   170,   171,   172,   173,
      27,   175,    29,    -1,   178,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   161,    -1,    -1,    -1,    -1,    -1,
     167,   168,    -1,   170,   171,   172,   173,    -1,   175,    -1,
      -1,   178,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,
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
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     198,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   198,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   198,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   198,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,   198,    -1,    -1,    -1,    56,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   197,    -1,    -1,    -1,   103,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,    69,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    38,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,   197,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    70,   190,    -1,    -1,    -1,    -1,   195,   196,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,   136,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    50,    51,   176,    -1,    -1,
      -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   190,    -1,    -1,    -1,    70,   195,   196,    -1,    -1,
      -1,    -1,   201,    78,    79,    80,    81,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,    69,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    70,    -1,
      -1,   176,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
     185,    83,    84,    -1,    -1,   190,    -1,    -1,    -1,    91,
     195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    70,    -1,    -1,   176,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,   190,    -1,
      -1,    -1,    91,   195,   196,    -1,    -1,   199,    -1,   201,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    70,    -1,    72,   176,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,   190,    -1,    -1,    -1,    91,   195,   196,    -1,    -1,
      -1,    -1,   201,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    70,    -1,    -1,
     176,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,   190,    -1,    -1,    -1,    91,   195,
     196,    -1,    -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      70,    -1,    -1,   176,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,   190,    -1,    -1,
      -1,    91,   195,   196,    -1,    -1,    -1,    -1,   201,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     190,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    30,
      31,   201,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,   136,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,   136,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   136,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,   136,    -1,    34,
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
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,   136,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,   136,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   136,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    -1,    69,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
      -1,    -1,    -1,    -1,    -1,   130,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,   136,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,   176,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    -1,   190,    91,    -1,    -1,    -1,
     195,   196,    -1,    -1,    -1,    -1,   201,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,   176,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    -1,   190,    91,    -1,    -1,    -1,
     195,   196,    -1,    -1,    -1,    -1,   201,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,   176,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,   190,    -1,    -1,    -1,    -1,
     195,   196,    -1,    -1,    -1,    -1,   201,    30,    31,    32,
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
      51,    52,    53,    54,    55,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    12,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    69,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   207,   208,     0,   209,     3,     4,     5,     6,     7,
      13,    27,    28,    29,    49,    50,    51,    56,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    70,
      71,    72,    73,    74,    78,    79,    80,    81,    82,    83,
      84,    86,    87,    91,    92,    93,    94,    96,    98,   100,
     103,   104,   108,   109,   110,   111,   112,   113,   114,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   126,   127,
     128,   129,   130,   131,   137,   138,   139,   140,   141,   142,
     143,   145,   146,   147,   148,   149,   153,   156,   161,   162,
     163,   164,   165,   167,   168,   170,   171,   172,   173,   176,
     179,   185,   186,   188,   190,   191,   192,   195,   196,   198,
     199,   201,   202,   204,   205,   210,   213,   223,   224,   225,
     226,   227,   230,   246,   247,   251,   254,   261,   267,   327,
     328,   336,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   351,   354,   366,   367,   374,   377,   380,   383,
     386,   392,   394,   395,   397,   407,   408,   409,   411,   416,
     421,   441,   449,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   476,   478,   480,   122,   123,
     124,   137,   161,   171,   196,   213,   246,   327,   348,   453,
     348,   196,   348,   348,   348,   108,   348,   348,   348,   439,
     440,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,    81,    83,    91,   124,   139,   140,   141,
     142,   143,   156,   196,   224,   367,   408,   411,   416,   453,
     456,   453,    38,   348,   467,   468,   348,   124,   130,   196,
     224,   259,   408,   409,   410,   412,   416,   450,   451,   452,
     460,   464,   465,   196,   337,   413,   196,   337,   358,   338,
     348,   232,   337,   196,   196,   196,   337,   198,   348,   213,
     198,   348,     3,     4,     6,     7,    10,    11,    12,    13,
      27,    29,    31,    57,    59,    71,    72,    73,    74,    75,
      76,    77,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   128,   130,   131,   132,
     133,   137,   138,   144,   161,   165,   173,   175,   178,   185,
     196,   213,   214,   215,   226,   481,   501,   502,   505,   198,
     343,   345,   348,   199,   239,   348,   111,   112,   164,   216,
     217,   218,   219,   223,    83,   201,   293,   294,   123,   130,
     122,   130,    83,   295,   196,   196,   196,   196,   213,   265,
     484,   196,   196,    70,    70,    70,    70,    70,   338,    83,
      90,   157,   158,   159,   473,   474,   164,   199,   223,   223,
     213,   266,   484,   165,   196,   484,   484,    83,   192,   199,
     359,    27,   336,   340,   348,   349,   453,   457,   228,   199,
     462,    90,   414,   473,    90,   473,   473,    32,   164,   181,
     485,   196,     9,   198,    38,   245,   165,   264,   484,   124,
     191,   246,   328,   198,   198,   198,   198,   198,   198,   198,
     198,    10,    11,    12,    30,    31,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    57,
      69,   198,    70,    70,   199,   160,   131,   171,   173,   186,
     188,   267,   326,   327,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    67,    68,   134,
     135,   443,    70,   199,   448,   196,   196,    70,   199,   196,
     245,   246,    14,   348,   198,   136,    48,   213,   438,    90,
     336,   349,   160,   453,   136,   203,     9,   423,   260,   336,
     349,   453,   485,   160,   196,   415,   443,   448,   197,   348,
      32,   230,     8,   360,     9,   198,   230,   231,   338,   339,
     348,   213,   279,   234,   198,   198,   198,   138,   144,   505,
     505,   181,   504,   196,   111,   505,    14,   160,   138,   144,
     161,   213,   215,   198,   198,   198,   240,   115,   178,   198,
     216,   218,   216,   218,   223,   199,     9,   424,   198,   102,
     164,   199,   453,     9,   198,   130,   130,    14,     9,   198,
     453,   477,   338,   336,   349,   453,   456,   457,   197,   181,
     257,   137,   453,   466,   467,   348,   368,   369,   338,   389,
     389,   368,   389,   198,    70,   443,   157,   474,    82,   348,
     453,    90,   157,   474,   223,   212,   198,   199,   252,   262,
     398,   400,    91,   196,   361,   362,   364,   407,   411,   459,
     461,   478,    14,   102,   479,   355,   356,   357,   289,   290,
     441,   442,   197,   197,   197,   197,   197,   200,   229,   230,
     247,   254,   261,   441,   348,   202,   204,   205,   213,   486,
     487,   505,    38,   174,   291,   292,   348,   481,   196,   484,
     255,   245,   348,   348,   348,   348,    32,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   412,   348,   348,   463,   463,   348,   469,   470,   130,
     199,   214,   215,   462,   265,   213,   266,   484,   484,   264,
     246,    38,   340,   343,   345,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   165,   199,
     213,   444,   445,   446,   447,   462,   463,   348,   291,   291,
     463,   348,   466,   245,   197,   348,   196,   437,     9,   423,
     197,   197,    38,   348,    38,   348,   415,   197,   197,   197,
     460,   461,   462,   291,   199,   213,   444,   445,   462,   197,
     228,   283,   199,   345,   348,   348,    94,    32,   230,   277,
     198,    28,   102,    14,     9,   197,    32,   199,   280,   505,
      31,    91,   174,   226,   498,   499,   500,   196,     9,    50,
      51,    56,    58,    70,   138,   139,   140,   141,   142,   143,
     185,   196,   224,   375,   378,   381,   384,   387,   393,   408,
     416,   417,   419,   420,   213,   503,   228,   196,   238,   199,
     198,   199,   198,   102,   164,   111,   112,   164,   219,   220,
     221,   222,   223,   219,   213,   348,   294,   417,    83,     9,
     197,   197,   197,   197,   197,   197,   197,   198,    50,    51,
     494,   496,   497,   132,   270,   196,     9,   197,   197,   136,
     203,     9,   423,     9,   423,   203,   203,   203,   203,    83,
      85,   213,   475,   213,    70,   200,   200,   209,   211,    32,
     133,   269,   180,    54,   165,   180,   402,   349,   136,     9,
     423,   197,   160,   505,   505,    14,   360,   289,   228,   193,
       9,   424,   505,   506,   443,   448,   443,   200,     9,   423,
     182,   453,   348,   197,     9,   424,    14,   352,   248,   132,
     268,   196,   484,   348,    32,   203,   203,   136,   200,     9,
     423,   348,   485,   196,   258,   253,   263,    14,   479,   256,
     245,    72,   453,   348,   485,   203,   200,   197,   197,   203,
     200,   197,    50,    51,    70,    78,    79,    80,    91,   138,
     139,   140,   141,   142,   143,   156,   185,   213,   376,   379,
     382,   385,   388,   408,   419,   426,   428,   429,   433,   436,
     213,   453,   453,   136,   268,   443,   448,   197,   348,   284,
      75,    76,   285,   228,   337,   228,   339,   102,    38,   137,
     274,   453,   417,   213,    32,   230,   278,   198,   281,   198,
     281,     9,   423,    91,   226,   136,   160,     9,   423,   197,
     174,   486,   487,   488,   486,   417,   417,   417,   417,   417,
     422,   425,   196,    70,    70,    70,    70,    70,   196,   417,
     160,   199,    10,    11,    12,    31,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    69,   160,
     485,   200,   408,   199,   242,   218,   218,   213,   219,   219,
     223,     9,   424,   200,   200,    14,   453,   198,   182,     9,
     423,   213,   271,   408,   199,   466,   137,   453,    14,   348,
     348,   203,   348,   200,   209,   505,   271,   199,   401,    14,
     197,   348,   361,   462,   198,   505,   193,   200,    32,   492,
     442,    38,    83,   174,   444,   445,   447,   444,   445,   505,
      38,   174,   348,   417,   289,   196,   408,   269,   353,   249,
     348,   348,   348,   200,   196,   291,   270,    32,   269,   505,
      14,   268,   484,   412,   200,   196,    14,    78,    79,    80,
     213,   427,   427,   429,   431,   432,    52,   196,    70,    70,
      70,    70,    70,    90,   157,   196,   160,     9,   423,   197,
     437,    38,   348,   269,   200,    75,    76,   286,   337,   230,
     200,   198,    95,   198,   274,   453,   196,   136,   273,    14,
     228,   281,   105,   106,   107,   281,   200,   505,   182,   136,
     160,   505,   213,   174,   498,     9,   197,   423,   136,   203,
       9,   423,   422,   370,   371,   417,   390,   417,   418,   390,
     370,   390,   361,   363,   365,   197,   130,   214,   417,   471,
     472,   417,   417,   417,    32,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   503,
      83,   243,   200,   200,   222,   198,   417,   497,   102,   103,
     493,   495,     9,   299,   197,   196,   340,   345,   348,   136,
     203,   200,   479,   299,   166,   179,   199,   397,   404,   166,
     199,   403,   136,   198,   492,   505,   360,   506,    83,   174,
      14,    83,   485,   453,   348,   197,   289,   199,   289,   196,
     136,   196,   291,   197,   199,   505,   199,   198,   505,   269,
     250,   415,   291,   136,   203,     9,   423,   428,   431,   372,
     373,   429,   391,   429,   430,   391,   372,   391,   157,   361,
     434,   435,    81,   429,   453,   199,   337,    32,    77,   230,
     198,   339,   273,   466,   274,   197,   417,   101,   105,   198,
     348,    32,   198,   282,   200,   182,   505,   213,   136,   174,
      32,   197,   417,   417,   197,   203,     9,   423,   136,   203,
       9,   423,   203,   203,   203,   136,     9,   423,   197,   136,
     200,     9,   423,   417,    32,   197,   228,   198,   198,   213,
     505,   505,   493,   408,     4,   112,   117,   123,   125,   167,
     168,   170,   200,   300,   325,   326,   327,   332,   333,   334,
     335,   441,   466,   348,   200,   199,   200,    54,   348,   348,
     348,   360,    38,    83,   174,    14,    83,   348,   196,   492,
     197,   299,   197,   289,   348,   291,   197,   299,   479,   299,
     198,   199,   196,   197,   429,   429,   197,   203,     9,   423,
     136,   203,     9,   423,   203,   203,   203,   136,   197,     9,
     423,   299,    32,   228,   198,   197,   197,   197,   235,   198,
     198,   282,   228,   136,   505,   505,   136,   417,   417,   417,
     417,   361,   417,   417,   417,   199,   200,   495,   132,   133,
     186,   214,   482,   505,   272,   408,   112,   335,    31,   125,
     138,   144,   165,   171,   309,   310,   311,   312,   408,   169,
     317,   318,   128,   196,   213,   319,   320,   301,   246,   505,
       9,   198,     9,   198,   198,   479,   326,   197,   296,   165,
     399,   200,   200,    83,   174,    14,    83,   348,   291,   117,
     350,   492,   200,   492,   197,   197,   200,   199,   200,   299,
     289,   136,   429,   429,   429,   429,   361,   200,   228,   233,
     236,    32,   230,   276,   228,   505,   197,   417,   136,   136,
     136,   228,   408,   408,   484,    14,   214,     9,   198,   199,
     482,   479,   312,   181,   199,     9,   198,     3,     4,     5,
       6,     7,    10,    11,    12,    13,    27,    28,    29,    57,
      71,    72,    73,    74,    75,    76,    77,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   137,   138,
     145,   146,   147,   148,   149,   161,   162,   163,   173,   175,
     176,   178,   185,   186,   188,   190,   191,   213,   405,   406,
       9,   198,   165,   169,   213,   320,   321,   322,   198,    83,
     331,   245,   302,   482,   482,    14,   246,   200,   297,   298,
     482,    14,    83,   348,   197,   196,   492,   198,   199,   323,
     350,   492,   296,   200,   197,   429,   136,   136,    32,   230,
     275,   276,   228,   417,   417,   417,   200,   198,   198,   417,
     408,   305,   505,   313,   314,   416,   310,    14,    32,    51,
     315,   318,     9,    36,   197,    31,    50,    53,    14,     9,
     198,   215,   483,   331,    14,   505,   245,   198,    14,   348,
      38,    83,   396,   199,   228,   492,   323,   200,   492,   429,
     429,   228,    99,   241,   200,   213,   226,   306,   307,   308,
       9,   423,     9,   423,   200,   417,   406,   406,    59,   316,
     321,   321,    31,    50,    53,   417,    83,   181,   196,   198,
     417,   484,   417,    83,     9,   424,   228,   200,   199,   323,
      97,   198,   115,   237,   160,   102,   505,   182,   416,   172,
      14,   494,   303,   196,    38,    83,   197,   200,   228,   198,
     196,   178,   244,   213,   326,   327,   182,   417,   182,   287,
     288,   442,   304,    83,   200,   408,   242,   175,   213,   198,
     197,     9,   424,   119,   120,   121,   329,   330,   287,    83,
     272,   198,   492,   442,   506,   197,   197,   198,   195,   489,
     329,    38,    83,   174,   492,   199,   490,   491,   505,   198,
     199,   324,   506,    83,   174,    14,    83,   489,   228,     9,
     424,    14,   493,   228,    38,    83,   174,    14,    83,   348,
     324,   200,   491,   505,   200,    83,   174,    14,    83,   348,
      14,    83,   348,   348
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
#line 751 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 761 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 762 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 765 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 766 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 767 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 768 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 769 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 771 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 774 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 776 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 777 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 778 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 779 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 780 "hphp.y"
    { _p->onUse((yyvsp[(2) - (3)]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 783 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 786 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 789 "hphp.y"
    { _p->onGroupUse((yyvsp[(2) - (6)]).text(), (yyvsp[(4) - (6)]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 793 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 797 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 800 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 814 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 815 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 816 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 817 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 896 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 898 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 903 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 904 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 910 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 914 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 915 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 917 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(1) - (1)]),
                                           &Parser::useClass);;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 937 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useFunction);;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 939 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useConst);;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 944 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 946 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 951 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 952 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 957 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 964 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 972 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 981 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 982 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 985 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 986 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 987 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 988 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 991 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 995 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1000 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1010 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1016 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1019 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1021 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1024 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1025 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1026 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1027 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1028 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1029 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1030 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1031 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1032 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1033 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1034 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1035 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1036 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1037 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1038 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1042 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1044 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1049 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1051 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1055 "hphp.y"
    { _p->onDeclare((yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                         (yyval) = (yyvsp[(3) - (5)]);
                                         (yyval) = T_DECLARE;;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1064 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1065 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1068 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1069 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1070 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1071 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1075 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1076 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1077 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1078 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1079 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1080 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1081 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1082 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1083 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1084 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1085 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1086 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1094 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1095 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1104 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1105 "hphp.y"
    { (yyval).reset();;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1109 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1111 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1117 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1118 "hphp.y"
    { (yyval).reset();;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1122 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1123 "hphp.y"
    { (yyval).reset();;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1127 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1133 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1139 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1146 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1152 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1159 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1173 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1177 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1181 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1185 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1191 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1194 "hphp.y"
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

  case 206:

/* Line 1455 of yacc.c  */
#line 1209 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1212 "hphp.y"
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

  case 208:

/* Line 1455 of yacc.c  */
#line 1226 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1229 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1234 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1237 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1243 "hphp.y"
    { _p->onClassExpressionStart(); ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { _p->onClassExpression((yyval), (yyvsp[(3) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(5) - (8)]), (yyvsp[(7) - (8)])); ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1250 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1253 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1277 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1280 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1283 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1284 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1285 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1288 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1289 "hphp.y"
    { (yyval) = T_FINAL;;}
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
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1302 "hphp.y"
    { (yyval).reset();;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1305 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1307 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1310 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1312 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1316 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1317 "hphp.y"
    { (yyval).reset();;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1320 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1321 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1322 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1326 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1328 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1333 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1336 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1338 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1341 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1343 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (4)]));;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1347 "hphp.y"
    {_p->onDeclareList((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1349 "hphp.y"
    {_p->onDeclareList((yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                           (yyval) = (yyvsp[(1) - (5)]);;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1354 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1355 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1356 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1357 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1362 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1364 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1365 "hphp.y"
    { (yyval).reset();;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1368 "hphp.y"
    { (yyval).reset();;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1369 "hphp.y"
    { (yyval).reset();;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1374 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1375 "hphp.y"
    { (yyval).reset();;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1380 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1381 "hphp.y"
    { (yyval).reset();;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1385 "hphp.y"
    { (yyval).reset();;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1388 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1389 "hphp.y"
    { (yyval).reset();;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1397 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1403 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1409 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1413 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1417 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1422 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1427 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1430 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1436 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1445 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1450 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1455 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1460 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1466 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1472 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1480 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1485 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1490 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1497 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1501 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1505 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1508 "hphp.y"
    { (yyval).reset();;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1513 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { (yyval).reset();;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1584 "hphp.y"
    { (yyval).reset();;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1592 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { (yyval).reset();;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (11)]),(yyvsp[(9) - (11)]),(yyvsp[(3) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(7) - (11)]),(yyvsp[(11) - (11)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1632 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (12)]),(yyvsp[(10) - (12)]),(yyvsp[(4) - (12)]),(yyvsp[(5) - (12)]),(yyvsp[(8) - (12)]),(yyvsp[(12) - (12)]),&(yyvsp[(1) - (12)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1644 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1660 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1661 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1667 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1672 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1675 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1683 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1688 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1698 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1723 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1727 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { (yyval).reset();;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval).reset();;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
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
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { (yyval).reset();;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { (yyval).reset();;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { (yyval).reset();;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { _p->onYieldFrom((yyval),&(yyvsp[(2) - (2)]));;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PIPE);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1975 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1982 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1983 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1985 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1988 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1989 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1991 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1994 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1996 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1997 "hphp.y"
    { _p->onNullCoalesce((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1998 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2000 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2003 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2004 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2005 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2006 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2009 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2011 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2012 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2013 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2014 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2015 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2016 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2017 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2018 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2025 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2026 "hphp.y"
    { (yyval).reset();;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2031 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2037 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[(2) - (12)]),(yyvsp[(5) - (12)]),(yyvsp[(8) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(7) - (12)]),&(yyvsp[(9) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2045 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2051 "hphp.y"
    { _p->finishStatement((yyvsp[(12) - (13)]), (yyvsp[(12) - (13)])); (yyvsp[(12) - (13)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[(1) - (13)]),
                                           (yyvsp[(3) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(12) - (13)]),(yyvsp[(8) - (13)]),&(yyvsp[(10) - (13)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2061 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2069 "hphp.y"
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

  case 535:

/* Line 1455 of yacc.c  */
#line 2079 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2087 "hphp.y"
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

  case 537:

/* Line 1455 of yacc.c  */
#line 2097 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
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

  case 539:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2137 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2148 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2154 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2156 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2182 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2187 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { (yyval).reset();;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { (yyval).reset();;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { (yyval).reset();;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { _p->onKeyset((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { _p->onKeyset((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { _p->onKeyset((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { _p->onVArray((yyval),(yyvsp[(3) - (4)])); ;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { _p->onVArray((yyval),(yyvsp[(3) - (4)])); ;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { _p->onVArray((yyval),(yyvsp[(3) - (4)])); ;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { _p->onDArray((yyval),(yyvsp[(3) - (4)])); ;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { _p->onDArray((yyval),(yyvsp[(3) - (4)])); ;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { _p->onDArray((yyval),(yyvsp[(3) - (4)])); ;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval).reset();;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval).reset();;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval).reset();;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
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

  case 613:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
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

  case 614:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { (yyval).reset();;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { (yyval).reset();;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2485 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2493 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2498 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2503 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2512 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2519 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2523 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { _p->onName((yyval), (yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { (yyval).reset();;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2574 "hphp.y"
    { (yyval).reset();;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2578 "hphp.y"
    { (yyval).reset();;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2579 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2580 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { (yyval).reset();;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2596 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2599 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2615 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2624 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2629 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2633 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2635 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2639 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2640 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2644 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2645 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2648 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2660 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2662 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2664 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2675 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2680 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2686 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2692 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2693 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2698 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2700 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2701 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2702 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2709 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2710 "hphp.y"
    { (yyval).reset();;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { (yyval).reset();;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2715 "hphp.y"
    { (yyval).reset();;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2718 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { (yyval).reset();;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2725 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2727 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2729 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2730 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2739 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2746 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2751 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2761 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2769 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2772 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2778 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2785 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2786 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2792 "hphp.y"
    { (yyval).reset();;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2797 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2802 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2806 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2813 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2818 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2821 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2827 "hphp.y"
    { (yyval).reset();;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2830 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2831 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2848 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2852 "hphp.y"
    { (yyval).reset();;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2857 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2861 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2863 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2867 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2872 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2877 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2878 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2892 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2893 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2897 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2899 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2904 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2906 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 885:

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

  case 886:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
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

  case 887:

/* Line 1455 of yacc.c  */
#line 2938 "hphp.y"
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

  case 888:

/* Line 1455 of yacc.c  */
#line 2950 "hphp.y"
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

  case 889:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2965 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2966 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2967 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2969 "hphp.y"
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

  case 896:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2990 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2991 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2995 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2996 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2998 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3006 "hphp.y"
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

  case 905:

/* Line 1455 of yacc.c  */
#line 3015 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3017 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3018 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3027 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3028 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3030 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3031 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3032 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3037 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3045 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3052 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3056 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3063 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3072 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3076 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3080 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3089 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3090 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3091 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3095 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3096 "hphp.y"
    { _p->onPipeVariable((yyval));;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3097 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3099 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3104 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3105 "hphp.y"
    { (yyval).reset();;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3117 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3118 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
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

  case 940:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3133 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3138 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3141 "hphp.y"
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

  case 946:

/* Line 1455 of yacc.c  */
#line 3150 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3154 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3155 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3157 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3158 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3159 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3160 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3165 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3166 "hphp.y"
    { (yyval).reset();;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3170 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3171 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3172 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3173 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3176 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3178 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3179 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3180 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3185 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3186 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3190 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3191 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3192 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3193 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3198 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3199 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3204 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3206 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3208 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3209 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3213 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3215 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3216 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3218 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3223 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3225 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3227 "hphp.y"
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

  case 982:

/* Line 1455 of yacc.c  */
#line 3237 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3239 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3240 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3243 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3244 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3249 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3250 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3251 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3252 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3253 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3254 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3255 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 995:

/* Line 1455 of yacc.c  */
#line 3256 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 996:

/* Line 1455 of yacc.c  */
#line 3257 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3258 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3259 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3263 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3264 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3269 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3271 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 1005:

/* Line 1455 of yacc.c  */
#line 3285 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 1006:

/* Line 1455 of yacc.c  */
#line 3290 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3294 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3299 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1010:

/* Line 1455 of yacc.c  */
#line 3306 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1011:

/* Line 1455 of yacc.c  */
#line 3310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1012:

/* Line 1455 of yacc.c  */
#line 3311 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1013:

/* Line 1455 of yacc.c  */
#line 3317 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1014:

/* Line 1455 of yacc.c  */
#line 3321 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 1015:

/* Line 1455 of yacc.c  */
#line 3327 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1016:

/* Line 1455 of yacc.c  */
#line 3331 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3338 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 1018:

/* Line 1455 of yacc.c  */
#line 3339 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1019:

/* Line 1455 of yacc.c  */
#line 3343 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1020:

/* Line 1455 of yacc.c  */
#line 3346 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1021:

/* Line 1455 of yacc.c  */
#line 3352 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1022:

/* Line 1455 of yacc.c  */
#line 3357 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1023:

/* Line 1455 of yacc.c  */
#line 3358 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1024:

/* Line 1455 of yacc.c  */
#line 3359 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1025:

/* Line 1455 of yacc.c  */
#line 3360 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1032:

/* Line 1455 of yacc.c  */
#line 3381 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1033:

/* Line 1455 of yacc.c  */
#line 3382 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1; ;}
    break;

  case 1036:

/* Line 1455 of yacc.c  */
#line 3391 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1039:

/* Line 1455 of yacc.c  */
#line 3402 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 1040:

/* Line 1455 of yacc.c  */
#line 3404 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 1041:

/* Line 1455 of yacc.c  */
#line 3408 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 1042:

/* Line 1455 of yacc.c  */
#line 3411 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 1043:

/* Line 1455 of yacc.c  */
#line 3415 "hphp.y"
    {;}
    break;

  case 1044:

/* Line 1455 of yacc.c  */
#line 3416 "hphp.y"
    {;}
    break;

  case 1045:

/* Line 1455 of yacc.c  */
#line 3417 "hphp.y"
    {;}
    break;

  case 1046:

/* Line 1455 of yacc.c  */
#line 3423 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 1047:

/* Line 1455 of yacc.c  */
#line 3428 "hphp.y"
    {
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   ;}
    break;

  case 1048:

/* Line 1455 of yacc.c  */
#line 3437 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 1049:

/* Line 1455 of yacc.c  */
#line 3443 "hphp.y"
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]), (yyvsp[(6) - (6)]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   ;}
    break;

  case 1050:

/* Line 1455 of yacc.c  */
#line 3451 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1051:

/* Line 1455 of yacc.c  */
#line 3452 "hphp.y"
    { ;}
    break;

  case 1052:

/* Line 1455 of yacc.c  */
#line 3458 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (3)]), true); ;}
    break;

  case 1053:

/* Line 1455 of yacc.c  */
#line 3460 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)]), false); ;}
    break;

  case 1054:

/* Line 1455 of yacc.c  */
#line 3461 "hphp.y"
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       ;}
    break;

  case 1055:

/* Line 1455 of yacc.c  */
#line 3466 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); ;}
    break;

  case 1056:

/* Line 1455 of yacc.c  */
#line 3472 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1057:

/* Line 1455 of yacc.c  */
#line 3477 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1058:

/* Line 1455 of yacc.c  */
#line 3482 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1059:

/* Line 1455 of yacc.c  */
#line 3486 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1060:

/* Line 1455 of yacc.c  */
#line 3491 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 1061:

/* Line 1455 of yacc.c  */
#line 3493 "hphp.y"
    { _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)])); (yyval) = (yyvsp[(2) - (5)]);;}
    break;

  case 1062:

/* Line 1455 of yacc.c  */
#line 3499 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1063:

/* Line 1455 of yacc.c  */
#line 3502 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1064:

/* Line 1455 of yacc.c  */
#line 3505 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1065:

/* Line 1455 of yacc.c  */
#line 3506 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1066:

/* Line 1455 of yacc.c  */
#line 3509 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1067:

/* Line 1455 of yacc.c  */
#line 3512 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1068:

/* Line 1455 of yacc.c  */
#line 3515 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1069:

/* Line 1455 of yacc.c  */
#line 3518 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1070:

/* Line 1455 of yacc.c  */
#line 3520 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1071:

/* Line 1455 of yacc.c  */
#line 3526 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1072:

/* Line 1455 of yacc.c  */
#line 3532 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1073:

/* Line 1455 of yacc.c  */
#line 3540 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1074:

/* Line 1455 of yacc.c  */
#line 3541 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14776 "hphp.7.tab.cpp"
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
#line 3544 "hphp.y"

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

