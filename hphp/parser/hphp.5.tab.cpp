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
#define yyparse         Compiler5parse
#define yylex           Compiler5lex
#define yyerror         Compiler5error
#define yylval          Compiler5lval
#define yychar          Compiler5char
#define yydebug         Compiler5debug
#define yynerrs         Compiler5nerrs
#define yylloc          Compiler5lloc

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
#line 666 "hphp.5.tab.cpp"

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
#line 897 "hphp.5.tab.cpp"

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
#define YYLAST   18190

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  206
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  302
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1075
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1976

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
    2921,  2923,  2925,  2927,  2929,  2933,  2935,  2940,  2944,  2946,
    2948,  2950,  2952,  2954,  2958,  2962,  2967,  2972,  2976,  2978,
    2980,  2988,  2998,  3006,  3013,  3022,  3024,  3027,  3032,  3037,
    3039,  3041,  3043,  3048,  3050,  3051,  3053,  3056,  3058,  3060,
    3062,  3066,  3070,  3074,  3075,  3077,  3079,  3083,  3087,  3090,
    3094,  3101,  3102,  3104,  3109,  3112,  3113,  3119,  3123,  3127,
    3129,  3136,  3141,  3146,  3149,  3152,  3153,  3159,  3163,  3167,
    3169,  3172,  3173,  3179,  3183,  3187,  3189,  3192,  3195,  3197,
    3200,  3202,  3207,  3211,  3215,  3222,  3226,  3228,  3230,  3232,
    3237,  3242,  3247,  3252,  3257,  3262,  3265,  3268,  3273,  3276,
    3279,  3281,  3285,  3289,  3293,  3294,  3297,  3303,  3310,  3317,
    3325,  3327,  3330,  3332,  3335,  3337,  3342,  3344,  3349,  3353,
    3354,  3356,  3360,  3363,  3367,  3369,  3371,  3372,  3373,  3377,
    3379,  3383,  3387,  3390,  3391,  3394,  3397,  3400,  3403,  3405,
    3408,  3413,  3416,  3422,  3426,  3428,  3430,  3431,  3435,  3440,
    3446,  3453,  3457,  3459,  3463,  3466,  3468,  3469,  3474,  3476,
    3480,  3483,  3488,  3494,  3497,  3500,  3502,  3504,  3506,  3508,
    3512,  3515,  3517,  3526,  3533,  3535
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     207,     0,    -1,    -1,   208,   209,    -1,   209,   210,    -1,
      -1,   230,    -1,   247,    -1,   254,    -1,   251,    -1,   261,
      -1,   481,    -1,   129,   196,   197,   198,    -1,   161,   223,
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
     161,   164,   223,    -1,   164,   223,    -1,   224,   486,    -1,
     224,   486,    -1,   227,     9,   482,    14,   417,    -1,   112,
     482,    14,   417,    -1,   228,   229,    -1,    -1,   230,    -1,
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
     478,   197,   198,    -1,   198,    -1,    86,    -1,    87,    -1,
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
     246,   245,   485,   248,   196,   289,   197,   493,   323,    -1,
      -1,   327,   246,   245,   485,   249,   196,   289,   197,   493,
     323,    -1,    -1,   441,   326,   246,   245,   485,   250,   196,
     289,   197,   493,   323,    -1,    -1,   171,   213,   252,    32,
     506,   480,   199,   296,   200,    -1,    -1,   441,   171,   213,
     253,    32,   506,   480,   199,   296,   200,    -1,    -1,   267,
     264,   255,   268,   269,   199,   299,   200,    -1,    -1,   441,
     267,   264,   256,   268,   269,   199,   299,   200,    -1,    -1,
     131,   265,   257,   270,   199,   299,   200,    -1,    -1,   441,
     131,   265,   258,   270,   199,   299,   200,    -1,    -1,   130,
     260,   415,   268,   269,   199,   299,   200,    -1,    -1,   173,
     266,   262,   269,   199,   299,   200,    -1,    -1,   441,   173,
     266,   263,   269,   199,   299,   200,    -1,   485,    -1,   165,
      -1,   485,    -1,   485,    -1,   130,    -1,   123,   130,    -1,
     123,   122,   130,    -1,   122,   123,   130,    -1,   122,   130,
      -1,   132,   408,    -1,    -1,   133,   271,    -1,    -1,   132,
     271,    -1,    -1,   408,    -1,   271,     9,   408,    -1,   408,
      -1,   272,     9,   408,    -1,   136,   274,    -1,    -1,   453,
      -1,    38,   453,    -1,   137,   196,   467,   197,    -1,   230,
      -1,    32,   228,    97,   198,    -1,   230,    -1,    32,   228,
      99,   198,    -1,   230,    -1,    32,   228,    95,   198,    -1,
     230,    -1,    32,   228,   101,   198,    -1,   213,    14,   417,
      -1,   279,     9,   213,    14,   417,    -1,   199,   281,   200,
      -1,   199,   198,   281,   200,    -1,    32,   281,   105,   198,
      -1,    32,   198,   281,   105,   198,    -1,   281,   106,   348,
     282,   228,    -1,   281,   107,   282,   228,    -1,    -1,    32,
      -1,   198,    -1,   283,    75,   337,   230,    -1,    -1,   284,
      75,   337,    32,   228,    -1,    -1,    76,   230,    -1,    -1,
      76,    32,   228,    -1,    -1,   288,     9,   442,   329,   507,
     174,    83,    -1,   288,     9,   442,   329,   507,    38,   174,
      83,    -1,   288,     9,   442,   329,   507,   174,    -1,   288,
     424,    -1,   442,   329,   507,   174,    83,    -1,   442,   329,
     507,    38,   174,    83,    -1,   442,   329,   507,   174,    -1,
      -1,   442,   329,   507,    83,    -1,   442,   329,   507,    38,
      83,    -1,   442,   329,   507,    38,    83,    14,   348,    -1,
     442,   329,   507,    83,    14,   348,    -1,   288,     9,   442,
     329,   507,    83,    -1,   288,     9,   442,   329,   507,    38,
      83,    -1,   288,     9,   442,   329,   507,    38,    83,    14,
     348,    -1,   288,     9,   442,   329,   507,    83,    14,   348,
      -1,   290,     9,   442,   507,   174,    83,    -1,   290,     9,
     442,   507,    38,   174,    83,    -1,   290,     9,   442,   507,
     174,    -1,   290,   424,    -1,   442,   507,   174,    83,    -1,
     442,   507,    38,   174,    83,    -1,   442,   507,   174,    -1,
      -1,   442,   507,    83,    -1,   442,   507,    38,    83,    -1,
     442,   507,    38,    83,    14,   348,    -1,   442,   507,    83,
      14,   348,    -1,   290,     9,   442,   507,    83,    -1,   290,
       9,   442,   507,    38,    83,    -1,   290,     9,   442,   507,
      38,    83,    14,   348,    -1,   290,     9,   442,   507,    83,
      14,   348,    -1,   292,   424,    -1,    -1,   348,    -1,    38,
     453,    -1,   174,   348,    -1,   292,     9,   348,    -1,   292,
       9,   174,   348,    -1,   292,     9,    38,   453,    -1,   293,
       9,   294,    -1,   294,    -1,    83,    -1,   201,   453,    -1,
     201,   199,   348,   200,    -1,   295,     9,    83,    -1,   295,
       9,    83,    14,   417,    -1,    83,    -1,    83,    14,   417,
      -1,   296,   297,    -1,    -1,   298,   198,    -1,   483,    14,
     417,    -1,   299,   300,    -1,    -1,    -1,   325,   301,   331,
     198,    -1,    -1,   327,   506,   302,   331,   198,    -1,   332,
     198,    -1,   333,   198,    -1,   334,   198,    -1,    -1,   326,
     246,   245,   484,   196,   303,   287,   197,   493,   490,   324,
      -1,    -1,   441,   326,   246,   245,   485,   196,   304,   287,
     197,   493,   490,   324,    -1,   167,   309,   198,    -1,   168,
     317,   198,    -1,   170,   319,   198,    -1,     4,   132,   408,
     198,    -1,     4,   133,   408,   198,    -1,   117,   272,   198,
      -1,   117,   272,   199,   305,   200,    -1,   305,   306,    -1,
     305,   307,    -1,    -1,   226,   160,   213,   175,   272,   198,
      -1,   308,   102,   326,   213,   198,    -1,   308,   102,   327,
     198,    -1,   226,   160,   213,    -1,   213,    -1,   310,    -1,
     309,     9,   310,    -1,   311,   405,   315,   316,    -1,   165,
      -1,    31,   312,    -1,   312,    -1,   138,    -1,   138,   181,
     506,   423,   182,    -1,   138,   181,   506,     9,   506,   182,
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
     417,    -1,   332,     9,   483,    14,   417,    -1,   112,   483,
      14,   417,    -1,   333,     9,   483,    -1,   123,   112,   483,
      -1,   123,   335,   480,    -1,   335,   480,    14,   506,    -1,
     112,   186,   485,    -1,   196,   336,   197,    -1,    72,   412,
     415,    -1,    72,   259,    -1,    71,   348,    -1,   397,    -1,
     392,    -1,   196,   348,   197,    -1,   338,     9,   348,    -1,
     348,    -1,   338,    -1,    -1,    27,    -1,    27,   348,    -1,
      27,   348,   136,   348,    -1,   196,   340,   197,    -1,   453,
      14,   340,    -1,   137,   196,   467,   197,    14,   340,    -1,
      29,   348,    -1,   453,    14,   343,    -1,    28,   348,    -1,
     453,    14,   345,    -1,   137,   196,   467,   197,    14,   345,
      -1,   349,    -1,   453,    -1,   336,    -1,   457,    -1,   456,
      -1,   137,   196,   467,   197,    14,   348,    -1,   453,    14,
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
      32,   348,    -1,   348,    33,   348,    -1,   477,    -1,    66,
     348,    -1,    65,   348,    -1,    64,   348,    -1,    63,   348,
      -1,    62,   348,    -1,    61,   348,    -1,    60,   348,    -1,
      73,   413,    -1,    59,   348,    -1,   421,    -1,   367,    -1,
     374,    -1,   377,    -1,   380,    -1,   383,    -1,   386,    -1,
     366,    -1,   202,   414,   202,    -1,    13,   348,    -1,   394,
      -1,   117,   196,   396,   424,   197,    -1,    -1,    -1,   246,
     245,   196,   352,   289,   197,   493,   350,   493,   199,   228,
     200,    -1,    -1,   327,   246,   245,   196,   353,   289,   197,
     493,   350,   493,   199,   228,   200,    -1,    -1,   191,    83,
     355,   360,    -1,    -1,   191,   192,   356,   289,   193,   493,
     360,    -1,    -1,   191,   199,   357,   228,   200,    -1,    -1,
      83,   358,   360,    -1,    -1,   192,   359,   289,   193,   493,
     360,    -1,     8,   348,    -1,     8,   345,    -1,     8,   199,
     228,   200,    -1,    91,    -1,   479,    -1,   362,     9,   361,
     136,   348,    -1,   361,   136,   348,    -1,   363,     9,   361,
     136,   417,    -1,   361,   136,   417,    -1,   362,   423,    -1,
      -1,   363,   423,    -1,    -1,   185,   196,   364,   197,    -1,
     138,   196,   468,   197,    -1,    70,   468,   203,    -1,   369,
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
     470,   200,    -1,   408,   199,   472,   200,    -1,   394,    70,
     463,   203,    -1,   395,    70,   463,   203,    -1,   367,    -1,
     374,    -1,   377,    -1,   380,    -1,   383,    -1,   386,    -1,
     479,    -1,   456,    -1,    91,    -1,   196,   349,   197,    -1,
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
     349,   197,    -1,   409,    -1,   410,   160,   460,    -1,   409,
      -1,   459,    -1,   407,    -1,   411,   160,   460,    -1,   408,
      -1,   124,    -1,   465,    -1,   196,   197,    -1,   337,    -1,
      -1,    -1,    90,    -1,   474,    -1,   196,   291,   197,    -1,
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
      -1,   419,    -1,   224,    -1,    82,    -1,   479,    -1,   416,
      -1,   204,   474,   204,    -1,   205,   474,   205,    -1,   156,
     474,   157,    -1,   425,   423,    -1,    -1,     9,    -1,    -1,
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
      -1,   199,   348,   200,    -1,   444,    -1,   460,    -1,   213,
      -1,   199,   348,   200,    -1,   446,    -1,   460,    -1,    70,
     463,   203,    -1,   199,   348,   200,    -1,   454,   448,    -1,
     196,   336,   197,   448,    -1,   466,   448,    -1,   196,   336,
     197,   448,    -1,   196,   336,   197,   443,   445,    -1,   196,
     349,   197,   443,   445,    -1,   196,   336,   197,   443,   444,
      -1,   196,   349,   197,   443,   444,    -1,   460,    -1,   407,
      -1,   458,    -1,   459,    -1,   449,    -1,   451,    -1,   453,
     443,   445,    -1,   411,   160,   460,    -1,   455,   196,   291,
     197,    -1,   456,   196,   291,   197,    -1,   196,   453,   197,
      -1,   407,    -1,   458,    -1,   459,    -1,   449,    -1,   453,
     443,   444,    -1,   452,    -1,   455,   196,   291,   197,    -1,
     196,   453,   197,    -1,   460,    -1,   449,    -1,   407,    -1,
     367,    -1,   416,    -1,   196,   453,   197,    -1,   196,   349,
     197,    -1,   456,   196,   291,   197,    -1,   455,   196,   291,
     197,    -1,   196,   457,   197,    -1,   351,    -1,   354,    -1,
     453,   443,   447,   486,   196,   291,   197,    -1,   196,   336,
     197,   443,   447,   486,   196,   291,   197,    -1,   411,   160,
     215,   486,   196,   291,   197,    -1,   411,   160,   460,   196,
     291,   197,    -1,   411,   160,   199,   348,   200,   196,   291,
     197,    -1,   461,    -1,   464,   461,    -1,   461,    70,   463,
     203,    -1,   461,   199,   348,   200,    -1,   462,    -1,    83,
      -1,    84,    -1,   201,   199,   348,   200,    -1,   348,    -1,
      -1,   201,    -1,   464,   201,    -1,   460,    -1,   450,    -1,
     451,    -1,   465,   443,   445,    -1,   410,   160,   460,    -1,
     196,   453,   197,    -1,    -1,   450,    -1,   452,    -1,   465,
     443,   444,    -1,   196,   453,   197,    -1,   467,     9,    -1,
     467,     9,   453,    -1,   467,     9,   137,   196,   467,   197,
      -1,    -1,   453,    -1,   137,   196,   467,   197,    -1,   469,
     423,    -1,    -1,   469,     9,   348,   136,   348,    -1,   469,
       9,   348,    -1,   348,   136,   348,    -1,   348,    -1,   469,
       9,   348,   136,    38,   453,    -1,   469,     9,    38,   453,
      -1,   348,   136,    38,   453,    -1,    38,   453,    -1,   471,
     423,    -1,    -1,   471,     9,   348,   136,   348,    -1,   471,
       9,   348,    -1,   348,   136,   348,    -1,   348,    -1,   473,
     423,    -1,    -1,   473,     9,   417,   136,   417,    -1,   473,
       9,   417,    -1,   417,   136,   417,    -1,   417,    -1,   474,
     475,    -1,   474,    90,    -1,   475,    -1,    90,   475,    -1,
      83,    -1,    83,    70,   476,   203,    -1,    83,   443,   213,
      -1,   158,   348,   200,    -1,   158,    82,    70,   348,   203,
     200,    -1,   159,   453,   200,    -1,   213,    -1,    85,    -1,
      83,    -1,   127,   196,   338,   197,    -1,   128,   196,   453,
     197,    -1,   128,   196,   349,   197,    -1,   128,   196,   457,
     197,    -1,   128,   196,   456,   197,    -1,   128,   196,   336,
     197,    -1,     7,   348,    -1,     6,   348,    -1,     5,   196,
     348,   197,    -1,     4,   348,    -1,     3,   348,    -1,   453,
      -1,   478,     9,   453,    -1,   411,   160,   214,    -1,   411,
     160,   130,    -1,    -1,   102,   506,    -1,   186,   485,    14,
     506,   198,    -1,   441,   186,   485,    14,   506,   198,    -1,
     188,   485,   480,    14,   506,   198,    -1,   441,   188,   485,
     480,    14,   506,   198,    -1,   215,    -1,   506,   215,    -1,
     214,    -1,   506,   214,    -1,   215,    -1,   215,   181,   495,
     182,    -1,   213,    -1,   213,   181,   495,   182,    -1,   181,
     488,   182,    -1,    -1,   506,    -1,   487,     9,   506,    -1,
     487,   423,    -1,   487,     9,   174,    -1,   488,    -1,   174,
      -1,    -1,    -1,   195,   491,   424,    -1,   492,    -1,   491,
       9,   492,    -1,   506,    14,   506,    -1,   506,   494,    -1,
      -1,    32,   506,    -1,   102,   506,    -1,   103,   506,    -1,
     497,   423,    -1,   494,    -1,   496,   494,    -1,   497,     9,
     498,   213,    -1,   498,   213,    -1,   497,     9,   498,   213,
     496,    -1,   498,   213,   496,    -1,    50,    -1,    51,    -1,
      -1,    91,   136,   506,    -1,    31,    91,   136,   506,    -1,
     226,   160,   213,   136,   506,    -1,    31,   226,   160,   213,
     136,   506,    -1,   500,     9,   499,    -1,   499,    -1,   500,
       9,   174,    -1,   500,   423,    -1,   174,    -1,    -1,   185,
     196,   501,   197,    -1,   226,    -1,   213,   160,   504,    -1,
     213,   486,    -1,   181,   506,   423,   182,    -1,   181,   506,
       9,   506,   182,    -1,    31,   506,    -1,    59,   506,    -1,
     226,    -1,   138,    -1,   144,    -1,   502,    -1,   503,   160,
     504,    -1,   138,   505,    -1,   165,    -1,   196,   111,   196,
     489,   197,    32,   506,   197,    -1,   196,   506,     9,   487,
     423,   197,    -1,   506,    -1,    -1
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
    2861,  2862,  2863,  2867,  2869,  2877,  2878,  2882,  2884,  2892,
    2893,  2897,  2898,  2903,  2905,  2910,  2921,  2935,  2947,  2962,
    2963,  2964,  2965,  2966,  2967,  2968,  2978,  2987,  2989,  2991,
    2995,  2996,  2997,  2998,  2999,  3015,  3016,  3018,  3027,  3028,
    3029,  3030,  3031,  3032,  3033,  3034,  3036,  3041,  3045,  3046,
    3050,  3053,  3060,  3064,  3073,  3080,  3082,  3088,  3090,  3091,
    3095,  3096,  3097,  3104,  3105,  3110,  3111,  3116,  3117,  3118,
    3119,  3130,  3133,  3136,  3137,  3138,  3139,  3150,  3154,  3155,
    3156,  3158,  3159,  3160,  3164,  3166,  3169,  3171,  3172,  3173,
    3174,  3177,  3179,  3180,  3184,  3186,  3189,  3191,  3192,  3193,
    3197,  3199,  3202,  3205,  3207,  3209,  3213,  3214,  3216,  3217,
    3223,  3224,  3226,  3236,  3238,  3240,  3243,  3244,  3245,  3249,
    3250,  3251,  3252,  3253,  3254,  3255,  3256,  3257,  3258,  3259,
    3263,  3264,  3268,  3270,  3278,  3280,  3284,  3288,  3293,  3297,
    3305,  3306,  3310,  3311,  3317,  3318,  3327,  3328,  3336,  3339,
    3343,  3346,  3351,  3356,  3358,  3359,  3360,  3363,  3365,  3371,
    3372,  3376,  3377,  3381,  3382,  3386,  3387,  3390,  3395,  3396,
    3400,  3403,  3405,  3409,  3415,  3416,  3417,  3421,  3425,  3433,
    3438,  3450,  3452,  3456,  3459,  3461,  3466,  3471,  3477,  3480,
    3485,  3490,  3492,  3499,  3502,  3505,  3506,  3509,  3512,  3513,
    3518,  3520,  3524,  3530,  3540,  3541
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
  "simple_indirect_reference", "variable_no_calls",
  "dimmable_variable_no_calls", "assignment_list", "array_pair_list",
  "non_empty_array_pair_list", "collection_init",
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
     454,   454,   454,   454,   454,   454,   454,   454,   455,   455,
     455,   455,   455,   455,   455,   455,   455,   456,   457,   457,
     458,   458,   459,   459,   459,   460,   460,   461,   461,   461,
     462,   462,   462,   463,   463,   464,   464,   465,   465,   465,
     465,   465,   465,   466,   466,   466,   466,   466,   467,   467,
     467,   467,   467,   467,   468,   468,   469,   469,   469,   469,
     469,   469,   469,   469,   470,   470,   471,   471,   471,   471,
     472,   472,   473,   473,   473,   473,   474,   474,   474,   474,
     475,   475,   475,   475,   475,   475,   476,   476,   476,   477,
     477,   477,   477,   477,   477,   477,   477,   477,   477,   477,
     478,   478,   479,   479,   480,   480,   481,   481,   481,   481,
     482,   482,   483,   483,   484,   484,   485,   485,   486,   486,
     487,   487,   488,   489,   489,   489,   489,   490,   490,   491,
     491,   492,   492,   493,   493,   494,   494,   495,   496,   496,
     497,   497,   497,   497,   498,   498,   498,   499,   499,   499,
     499,   500,   500,   501,   501,   501,   501,   502,   503,   504,
     504,   505,   505,   506,   506,   506,   506,   506,   506,   506,
     506,   506,   506,   506,   507,   507
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
       1,     1,     1,     1,     3,     1,     4,     3,     1,     1,
       1,     1,     1,     3,     3,     4,     4,     3,     1,     1,
       7,     9,     7,     6,     8,     1,     2,     4,     4,     1,
       1,     1,     4,     1,     0,     1,     2,     1,     1,     1,
       3,     3,     3,     0,     1,     1,     3,     3,     2,     3,
       6,     0,     1,     4,     2,     0,     5,     3,     3,     1,
       6,     4,     4,     2,     2,     0,     5,     3,     3,     1,
       2,     0,     5,     3,     3,     1,     2,     2,     1,     2,
       1,     4,     3,     3,     6,     3,     1,     1,     1,     4,
       4,     4,     4,     4,     4,     2,     2,     4,     2,     2,
       1,     3,     3,     3,     0,     2,     5,     6,     6,     7,
       1,     2,     1,     2,     1,     4,     1,     4,     3,     0,
       1,     3,     2,     3,     1,     1,     0,     0,     3,     1,
       3,     3,     2,     0,     2,     2,     2,     2,     1,     2,
       4,     2,     5,     3,     1,     1,     0,     3,     4,     5,
       6,     3,     1,     3,     2,     1,     0,     4,     1,     3,
       2,     4,     5,     2,     2,     1,     1,     1,     1,     3,
       2,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   434,     0,     0,   863,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   955,
       0,   943,   729,     0,   735,   736,   737,    25,   801,   930,
     931,   158,   159,   738,     0,   139,     0,     0,     0,     0,
      26,     0,     0,     0,     0,   193,     0,     0,     0,     0,
       0,     0,   400,   401,   402,   405,   404,   403,     0,     0,
       0,     0,   222,     0,     0,     0,    33,    34,    36,    37,
      35,   742,   744,   745,   739,   740,     0,     0,     0,   746,
     741,     0,   712,    28,    29,    30,    32,    31,     0,   743,
       0,     0,     0,     0,   747,   406,   541,    27,     0,   157,
     129,   935,   730,     0,     0,     4,   119,   121,   800,     0,
     711,     0,     6,   192,     7,     9,     8,    10,     0,     0,
     398,   447,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   445,   918,   919,   523,   517,   518,   519,   520,   521,
     522,   428,   526,     0,   427,   890,   713,   720,     0,   803,
     516,   397,   893,   894,   905,   446,     0,     0,   449,   448,
     891,   892,   889,   925,   929,     0,   506,   802,    11,   405,
     404,   403,     0,     0,    32,     0,   119,   192,     0,   999,
     446,   998,     0,   996,   995,   525,     0,   435,   442,   440,
       0,     0,   488,   489,   490,   491,   515,   513,   512,   511,
     510,   509,   508,   507,    25,   930,   738,   715,    33,    34,
      36,    37,    35,     0,     0,  1019,   911,   713,     0,   714,
     469,     0,   467,     0,   959,     0,   810,   426,   725,   212,
       0,  1019,   425,   724,   718,     0,   734,   714,   938,   939,
     945,   937,   726,     0,     0,   728,   514,     0,     0,     0,
       0,   431,     0,   137,   433,     0,     0,   143,   145,     0,
       0,   147,     0,    78,    77,    72,    71,    63,    64,    55,
      75,    86,    87,     0,    58,     0,    70,    62,    68,    89,
      81,    80,    53,    76,    96,    97,    54,    92,    51,    93,
      52,    94,    50,    98,    85,    90,    95,    82,    83,    57,
      84,    88,    49,    79,    65,    99,    73,    66,    56,    48,
      47,    46,    45,    44,    43,    67,   101,   100,   103,    60,
      41,    42,    69,  1066,  1067,    61,  1071,    40,    59,    91,
       0,     0,   119,   102,  1010,  1065,     0,  1068,     0,     0,
     149,     0,     0,     0,   183,     0,     0,     0,     0,     0,
       0,   812,     0,   107,   109,   311,     0,     0,   310,     0,
     226,     0,   223,   316,     0,     0,     0,     0,     0,  1016,
     208,   220,   951,   955,   560,   587,   587,   560,   587,     0,
     980,     0,   749,     0,     0,     0,   978,     0,    16,     0,
     123,   200,   214,   221,   617,   553,     0,  1004,   533,   535,
     537,   867,   434,   447,     0,     0,   445,   446,   448,     0,
       0,   731,     0,   732,     0,     0,     0,   182,     0,     0,
     125,   302,     0,    24,   191,     0,   219,   204,   218,   403,
     406,   192,   399,   172,   173,   174,   175,   176,   178,   179,
     181,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   943,
       0,   171,   934,   934,   965,     0,     0,     0,     0,     0,
       0,     0,     0,   396,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   468,   466,   868,
     869,     0,   934,     0,   881,   302,   302,   934,     0,   936,
     926,   951,     0,   192,     0,     0,   151,     0,   865,   860,
     810,     0,   447,   445,     0,   963,     0,   558,   809,   954,
     734,   447,   445,   446,   125,     0,   302,   424,     0,   883,
     727,     0,   129,   262,     0,   540,     0,   154,     0,     0,
     432,     0,     0,     0,     0,     0,   146,   170,   148,  1066,
    1067,  1063,  1064,     0,  1070,  1056,     0,     0,     0,     0,
      74,    39,    61,    38,  1011,   177,   180,   150,   129,     0,
     167,   169,     0,     0,     0,     0,   110,     0,   811,   108,
      18,     0,   104,     0,   312,     0,   152,   225,   224,     0,
       0,   153,  1000,     0,     0,   447,   445,   446,   449,   448,
       0,  1046,   232,     0,   952,     0,     0,     0,     0,   810,
     810,     0,     0,     0,     0,   155,     0,     0,   748,   979,
     801,     0,     0,   977,   806,   976,   122,     5,    13,    14,
       0,   230,     0,     0,   546,     0,     0,     0,   810,     0,
     722,     0,   721,   716,   547,     0,     0,     0,     0,   867,
     129,     0,   812,   866,  1075,   423,   437,   502,   899,   917,
     134,   128,   130,   131,   132,   133,   397,     0,   524,   804,
     805,   120,   810,     0,  1020,     0,     0,     0,   812,   303,
       0,   529,   194,   228,     0,   472,   474,   473,   485,     0,
       0,   505,   470,   471,   475,   477,   476,   493,   492,   495,
     494,   496,   498,   500,   499,   497,   487,   486,   479,   480,
     478,   481,   482,   484,   501,   483,   933,     0,     0,   969,
       0,   810,  1003,     0,  1002,  1019,   896,   925,   210,   202,
     216,     0,  1004,   206,   192,     0,   438,   441,   443,   451,
     465,   464,   463,   462,   461,   460,   459,   458,   457,   456,
     455,   454,   871,     0,   870,   873,   895,   877,  1019,   874,
       0,     0,     0,     0,     0,     0,     0,     0,   997,   436,
     858,   862,   809,   864,     0,   717,     0,   958,     0,   957,
     228,     0,   717,   942,   941,     0,     0,   870,   873,   940,
     874,   429,   264,   266,   129,   544,   543,   430,     0,   129,
     246,   138,   433,     0,     0,     0,     0,     0,   258,   258,
     144,   810,     0,     0,  1055,     0,  1052,   810,     0,  1026,
       0,     0,     0,     0,     0,   808,     0,    33,    34,    36,
      37,    35,     0,     0,   751,   755,   756,   757,   758,   759,
     761,     0,   750,   127,   799,   760,  1019,  1069,     0,     0,
       0,     0,    19,     0,    20,     0,   105,     0,     0,     0,
     116,   812,     0,   114,   109,   106,   111,     0,   309,   317,
     314,     0,     0,   989,   994,   991,   990,   993,   992,    12,
    1044,  1045,     0,   810,     0,     0,     0,   951,   948,     0,
     557,     0,   571,   809,   559,   809,   586,   574,   580,   583,
     577,   988,   987,   986,     0,   982,     0,   983,   985,     0,
       5,     0,     0,     0,   611,   612,   620,   619,     0,   445,
       0,   809,   552,   556,     0,     0,  1005,     0,   534,     0,
       0,  1033,   867,   288,  1074,     0,     0,   882,     0,   932,
     809,  1022,  1018,   304,   305,   710,   811,   301,     0,   867,
       0,     0,   230,   531,   196,   504,     0,   594,   595,     0,
     592,   809,   964,     0,     0,   302,   232,     0,   230,     0,
       0,   228,     0,   943,   452,     0,     0,   879,   880,   897,
     898,   927,   928,     0,     0,     0,   846,   817,   818,   819,
     826,     0,    33,    34,    36,    37,    35,     0,     0,   832,
     838,   839,   840,   841,   842,     0,   830,   828,   829,   852,
     810,     0,   860,   962,   961,     0,   230,     0,   884,   733,
       0,   268,     0,     0,   135,     0,     0,     0,     0,     0,
       0,     0,   238,   239,   250,     0,   129,   248,   164,   258,
       0,   258,     0,   809,     0,     0,     0,     0,     0,   809,
    1054,  1057,  1025,   810,  1024,     0,   810,   782,   783,   780,
     781,   816,     0,   810,   808,   564,   589,   589,   564,   589,
     555,     0,     0,   971,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1060,   184,     0,   187,   168,     0,     0,   112,
     117,   118,   110,   811,   115,     0,   313,     0,  1001,   156,
    1017,  1046,  1037,  1041,   231,   233,   323,     0,     0,   949,
       0,   562,     0,   981,     0,    17,     0,  1004,   229,   323,
       0,     0,   717,   549,     0,   723,  1006,     0,  1033,   538,
       0,     0,  1075,     0,   293,   291,   873,   885,  1019,   873,
     886,  1021,     0,     0,   306,   126,     0,   867,   227,     0,
     867,     0,   503,   968,   967,     0,   302,     0,     0,     0,
       0,     0,     0,   230,   198,   734,   872,   302,     0,   822,
     823,   824,   825,   833,   834,   850,     0,   810,     0,   846,
     568,   591,   591,   568,   591,     0,   821,   854,     0,   809,
     857,   859,   861,     0,   956,     0,   872,     0,     0,     0,
       0,   265,   545,   140,     0,   433,   238,   240,   951,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   252,     0,
    1061,     0,     0,  1047,     0,  1053,  1051,   809,     0,     0,
       0,   753,   809,   807,     0,     0,   810,     0,     0,   796,
     810,     0,     0,     0,     0,   810,     0,   762,   797,   798,
     975,     0,   810,   765,   767,   766,     0,     0,   763,   764,
     768,   770,   769,   785,   784,   787,   786,   788,   790,   792,
     791,   789,   778,   777,   772,   773,   771,   774,   775,   776,
     779,  1059,     0,   129,     0,     0,   113,    21,   315,     0,
       0,     0,  1038,  1043,     0,   397,   953,   951,   439,   444,
     450,     0,     0,    15,     0,   397,   623,     0,     0,   625,
     618,   621,     0,   616,     0,  1008,     0,  1034,   542,     0,
     294,     0,     0,   289,     0,   308,   307,  1033,     0,   323,
       0,   867,     0,   302,     0,   923,   323,  1004,   323,  1007,
       0,     0,     0,   453,     0,     0,   836,   809,   845,   827,
       0,     0,   810,     0,     0,   844,   810,     0,     0,     0,
     820,     0,     0,   810,   831,   851,   960,   323,     0,   129,
       0,   261,   247,     0,     0,     0,   237,   160,   251,     0,
       0,   254,     0,   259,   260,   129,   253,  1062,  1048,     0,
       0,  1023,     0,  1073,   815,   814,   752,   572,   809,   563,
       0,   575,   809,   588,   581,   584,   578,     0,   809,   554,
     754,     0,   593,   809,   970,   794,     0,     0,     0,    22,
      23,  1040,  1035,  1036,  1039,   234,     0,     0,     0,   404,
     395,     0,     0,     0,   209,   322,   324,     0,   394,     0,
       0,     0,  1004,   397,     0,   561,   984,   319,   215,   614,
       0,     0,   548,   536,     0,   297,   287,     0,   290,   296,
     302,   528,  1033,   397,  1033,     0,   966,     0,   922,   397,
       0,   397,  1009,   323,   867,   920,   849,   848,   835,   573,
     809,   567,     0,   576,   809,   590,   582,   585,   579,     0,
     837,   809,   853,   397,   129,   267,   136,   141,   162,   241,
       0,   249,   255,   129,   257,     0,  1049,     0,     0,     0,
     566,   795,   551,     0,   974,   973,   793,   129,   188,  1042,
       0,     0,     0,  1012,     0,     0,     0,   235,     0,  1004,
       0,   360,   356,   362,   712,    32,     0,   350,     0,   355,
     359,   372,     0,   370,   375,     0,   374,     0,   373,     0,
     192,   326,     0,   328,     0,   329,   330,     0,     0,   950,
       0,   615,   613,   624,   622,   298,     0,     0,   285,   295,
       0,     0,  1033,     0,   205,   528,  1033,   924,   211,   319,
     217,   397,     0,     0,     0,   570,   843,   856,     0,   213,
     263,     0,     0,   129,   244,   161,   256,  1050,  1072,   813,
       0,     0,     0,     0,     0,     0,   422,     0,  1013,     0,
     340,   344,   419,   420,   354,     0,     0,     0,   335,   676,
     675,   672,   674,   673,   693,   695,   694,   664,   634,   636,
     635,   654,   670,   669,   630,   641,   642,   644,   643,   663,
     647,   645,   646,   648,   649,   650,   651,   652,   653,   655,
     656,   657,   658,   659,   660,   662,   661,   631,   632,   633,
     637,   638,   640,   678,   679,   688,   687,   686,   685,   684,
     683,   671,   690,   680,   681,   682,   665,   666,   667,   668,
     691,   692,   696,   698,   697,   699,   700,   677,   702,   701,
     704,   706,   705,   639,   709,   707,   708,   703,   689,   629,
     367,   626,     0,   336,   388,   389,   387,   380,     0,   381,
     337,   414,     0,     0,     0,     0,   418,     0,   192,   201,
     318,     0,     0,     0,   286,   300,   921,     0,     0,   390,
     129,   195,  1033,     0,     0,   207,  1033,   847,     0,     0,
     129,   242,   142,   163,     0,   565,   550,   972,   186,   338,
     339,   417,   236,     0,   810,   810,     0,   363,   351,     0,
       0,     0,   369,   371,     0,     0,   376,   383,   384,   382,
       0,     0,   325,  1014,     0,     0,     0,   421,     0,   320,
       0,   299,     0,   609,   812,   129,     0,     0,   197,   203,
       0,   569,   855,     0,     0,   165,   341,   119,     0,   342,
     343,     0,   809,     0,   809,   365,   361,   366,   627,   628,
       0,   352,   385,   386,   378,   379,   377,   415,   412,  1046,
     331,   327,   416,     0,   321,   610,   811,     0,     0,   391,
     129,   199,     0,   245,     0,   190,     0,   397,     0,   357,
     364,   368,     0,     0,   867,   333,     0,   607,   527,   530,
       0,   243,     0,     0,   166,   348,     0,   396,   358,   413,
    1015,     0,   812,   408,   867,   608,   532,     0,   189,     0,
       0,   347,  1033,   867,   272,   409,   410,   411,  1075,   407,
       0,     0,     0,   346,  1027,   408,     0,  1033,     0,   345,
       0,     0,  1075,     0,   277,   275,  1027,   129,   812,  1029,
       0,   392,   129,   332,     0,   278,     0,     0,   273,     0,
       0,   811,  1028,     0,  1032,     0,     0,   281,   271,     0,
     274,   280,   334,   185,  1030,  1031,   393,   282,     0,     0,
     269,   279,     0,   270,   284,   283
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   115,   930,   647,   186,  1563,   745,
     360,   361,   362,   363,   881,   882,   883,   117,   118,   119,
     120,   121,   419,   681,   682,   559,   262,  1631,   565,  1540,
    1632,  1875,   870,   355,   588,  1835,  1126,  1323,  1894,   435,
     187,   683,   970,  1191,  1382,   125,   650,   987,   684,   703,
     991,   622,   986,   242,   540,   685,   651,   988,   437,   380,
     402,   128,   972,   933,   906,  1144,  1566,  1250,  1052,  1782,
    1635,   821,  1058,   564,   830,  1060,  1425,   813,  1041,  1044,
    1239,  1901,  1902,   671,   672,   697,   698,   367,   368,   374,
    1600,  1760,  1761,  1335,  1475,  1589,  1754,  1884,  1904,  1793,
    1839,  1840,  1841,  1576,  1577,  1578,  1579,  1795,  1796,  1802,
    1851,  1582,  1583,  1587,  1747,  1748,  1749,  1771,  1943,  1476,
    1477,   188,   130,  1918,  1919,  1752,  1479,  1480,  1481,  1482,
     131,   255,   560,   561,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,  1612,   142,   969,  1190,   143,   668,
     669,   670,   259,   411,   555,   657,   658,  1285,   659,  1286,
     144,   145,   628,   629,  1275,  1276,  1391,  1392,   146,   855,
    1020,   147,   856,  1021,   148,   857,  1022,   149,   858,  1023,
     150,   859,  1024,   631,  1278,  1394,   151,   860,   152,   153,
    1824,   154,   652,  1602,   653,  1160,   938,  1353,  1350,  1740,
    1741,   155,   156,   157,   245,   158,   246,   256,   422,   547,
     159,  1279,  1280,   864,   865,   160,  1082,   961,   599,  1083,
    1027,  1213,  1028,  1395,  1396,  1216,  1217,  1030,  1402,  1403,
    1031,   791,   530,   200,   201,   686,   674,   511,  1176,  1177,
     777,   778,   957,   162,   248,   163,   164,   190,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   737,   175,   252,
     253,   625,   235,   236,   740,   741,  1291,  1292,   395,   396,
     924,   176,   613,   177,   667,   178,   346,  1762,  1814,   381,
     430,   692,   693,  1075,  1931,  1938,  1939,  1171,  1332,   902,
    1333,   903,   904,   836,   837,   838,   347,   348,   867,   574,
    1565,   955
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1633
static const yytype_int16 yypact[] =
{
   -1633,   188, -1633, -1633,  5693, 13610, 13610,   -18, 13610, 13610,
   13610, 11377, 13610, 13610, -1633, 13610, 13610, 13610, 13610, 13610,
   13610, 13610, 13610, 13610, 13610, 13610, 13610, 16915, 16915, 11580,
   13610, 17477,    10,    30, -1633, -1633, -1633,   133, -1633,   228,
   -1633, -1633, -1633,   298, 13610, -1633,    30,   191,   214,   232,
   -1633,    30, 11783,  4572, 11986, -1633, 14488, 10362,   185, 13610,
   15916,   132, -1633, -1633, -1633,   446,   286,    53,   290,   313,
     329,   383, -1633,  4572,   397,   417,   526,   551,   584,   596,
     601, -1633, -1633, -1633, -1633, -1633, 13610,   577,  3119, -1633,
   -1633,  4572, -1633, -1633, -1633, -1633,  4572, -1633,  4572, -1633,
     510,   482,  4572,  4572, -1633,   420, -1633, -1633, 12189, -1633,
   -1633,   481,   562,   594,   594, -1633,   164,   542,   558,   512,
   -1633,   100, -1633,   687, -1633, -1633, -1633, -1633,  4635,   761,
   -1633, -1633,   525,   530,   545,   549,   592,   602,   604,   625,
   13594, -1633, -1633, -1633, -1633,    78,   743,   762,   774,   779,
     782, -1633,   785,   795, -1633,   357,   668, -1633,   706,    13,
   -1633,  1199,   194, -1633, -1633,  2618,   115,   677,   359, -1633,
     120,    72,   681,   316, -1633,   349, -1633,   808, -1633, -1633,
   -1633,   719,   692,   726, -1633, 13610, -1633,   687,   761, 17902,
    2900, 17902, 13610, 17902, 17902, 18121,   695, 16166, 18121, 17902,
     846,  4572,   829,   829,   352,   829,   829,   829,   829,   829,
     829,   829,   829,   829, -1633, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633,    57, 13610,   715, -1633, -1633,   739,   711,
     615,   716,   615, 16915, 16409,   710,   900, -1633,   719, -1633,
   13610,   715, -1633,   754, -1633,   755,   720, -1633,   141, -1633,
   -1633, -1633,   615,   115, 12392, -1633, -1633, 13610,  9144,   910,
     106, 17902, 10159, -1633, 13610, 13610,  4572, -1633, -1633, 15670,
     724, -1633, 15718, -1633, -1633, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633,  3189, -1633,  3189, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633,   112,    87,   726, -1633, -1633, -1633, -1633,
     727, 15995,   103, -1633, -1633,   764,   911, -1633,   767, 15207,
   -1633,   732,   733, 15766, -1633,    43, 15836, 14030, 14030,  4572,
     735,   923,   741, -1633,    60, -1633, 16503,   109, -1633,   807,
   -1633,   810, -1633,   929,   111, 16915, 13610, 13610,   758,   783,
   -1633, -1633, 16606, 11580, 13610, 13610, 13610, 13610, 13610,   113,
     535,   505, -1633, 13813, 16915,   672, -1633,  4572, -1633,   543,
     542, -1633, -1633, -1633, -1633, 17577,   948,   861, -1633, -1633,
   -1633,    70, 13610,   768,   769, 17902,   772,  2195,   776,  5896,
   13610,   457,   775,   639,   457,   541,   528, -1633,  4572,  3189,
     786, 10565, 14488, -1633, -1633,  3682, -1633, -1633, -1633, -1633,
   -1633,   687, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633,
   -1633, 13610, 13610, 13610, 13610, 12595, 13610, 13610, 13610, 13610,
   13610, 13610, 13610, 13610, 13610, 13610, 13610, 13610, 13610, 13610,
   13610, 13610, 13610, 13610, 13610, 13610, 13610, 13610, 13610, 17677,
   13610, -1633, 13610, 13610, 13610,  4742,  4572,  4572,  4572,  4572,
    4572,  4635,   867,  1002,  5052, 13610, 13610, 13610, 13610, 13610,
   13610, 13610, 13610, 13610, 13610, 13610, 13610, -1633, -1633, -1633,
   -1633,  1756, 13610, 13610, -1633, 10565, 10565, 13610, 13610,   481,
     158, 16606,   787,   687, 12798, 15884, -1633, 13610, -1633,   788,
     972,   836,   797,   800, 13984,   615, 13001, -1633, 13204, -1633,
     720,   801,   802,  2461, -1633,   400, 10565, -1633,  2200, -1633,
   -1633, 15932, -1633, -1633, 10768, -1633, 13610, -1633,   907,  9347,
     994,   809, 13797,   995,   119,    66, -1633, -1633, -1633,   832,
   -1633, -1633, -1633,  3189, -1633,  2571,   812,  1001, 16334,  4572,
   -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633,   825,
   -1633, -1633,   823,   826,   828,   834,   428, 17688, 14735, -1633,
   -1633,  4572,  4572, 13610,   615,   132, -1633, -1633, -1633, 16334,
     942, -1633,   615,   123,   150,   840,   841,  2883,   347,   842,
     843,   431,   894,   850,   615,   152,   853, 17084,   844,  1042,
    1046,   855,   856,   865,   869, -1633,  4143,  4572, -1633, -1633,
    1003,  3084,   548, -1633, -1633, -1633,   542, -1633, -1633, -1633,
    1043,   945,   901,   224,   922, 13610,   481,   947,  1079,   893,
   -1633,   931, -1633,   158, -1633,  3189,  3189,  1078,   910,    70,
   -1633,   902,  1084, -1633,  3189,   146, -1633,   493,   199, -1633,
   -1633, -1633, -1633, -1633, -1633, -1633,   827,  3417, -1633, -1633,
   -1633, -1633,  1085,   916, -1633, 16915, 13610,   903,  1090, 17902,
    1087, -1633, -1633,   975,  4130, 11971, 18040, 18121, 14311, 13610,
   17854, 14487, 11559, 12979,  3456, 12571, 13980, 14657, 14657, 14657,
   14657,  2885,  2885,  2885,  2885,  2885,  1367,  1367,   734,   734,
     734,   352,   352,   352, -1633,   829, 17902,   905,   908, 17132,
     904,  1101,    -4, 13610,     1,   715,   386,   158, -1633, -1633,
   -1633,  1099,   861, -1633,   687, 16709, -1633, -1633, -1633, 18121,
   18121, 18121, 18121, 18121, 18121, 18121, 18121, 18121, 18121, 18121,
   18121, 18121, -1633, 13610,     5,   178, -1633, -1633,   715,   215,
     913,  3850,   917,   920,   915,  3958,   155,   924, -1633, 17902,
   16437, -1633,  4572, -1633,   146,   310, 16915, 17902, 16915, 17188,
     975,   146,   615,   192,   967,   932, 13610, -1633,   202, -1633,
   -1633, -1633,  8941,   685, -1633, -1633, 17902, 17902,    30, -1633,
   -1633, -1633, 13610,  1026, 16210, 16334,  4572,  9550,   933,   934,
   -1633,  1125,  4499,   999, -1633,   976, -1633,  1130,   943,  3481,
    3189, 16334, 16334, 16334, 16334, 16334,   946,  1074,  1076,  1077,
    1083,  1086,   952, 16334,   392, -1633, -1633, -1633, -1633, -1633,
   -1633,    19, -1633, 17996, -1633, -1633,    21, -1633,  6099, 14912,
     956, 14735, -1633, 14735, -1633,  4572,  4572, 14735, 14735,  4572,
   -1633,  1142,   957, -1633,   462, -1633, -1633,  4058, -1633, 17996,
    1144, 16915,   961, -1633, -1633, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633,   979,  1154,  4572, 14912,   965, 16606, 16812,  1152,
   -1633, 13610, -1633, 13610, -1633, 13610, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633, -1633,   966, -1633, 13610, -1633, -1633,  5287,
   -1633,  3189, 14912,   977, -1633, -1633, -1633, -1633,  1167,   988,
   13610, 17577, -1633, -1633,  4742,   992, -1633,  3189, -1633,   993,
    6302,  1164,    61, -1633, -1633,   101,  1756, -1633,  2200, -1633,
    3189, -1633, -1633,   615, 17902, -1633, 10971, -1633, 16334,    50,
    1004, 14912,   945, -1633, -1633, 14487, 13610, -1633, -1633, 13610,
   -1633, 13610, -1633,  4144,  1005, 10565,   894,  1165,   945,  3189,
    1184,   975,  4572, 17677,   615,  4967,  1008, -1633, -1633,   203,
    1011, -1633, -1633,  1185,  2150,  2150, 16437, -1633, -1633, -1633,
    1156,  1014,  1141,  1143,  1146,  1147,  1148,    73,  1016,    12,
   -1633, -1633, -1633, -1633, -1633,  1054, -1633, -1633, -1633, -1633,
    1216,  1029,   788,   615,   615, 13407,   945,  2200, -1633, -1633,
   11361,   689,    30, 10159, -1633,  6505,  1030,  6708,  1032, 16210,
   16915,  1035,  1096,   615, 17996,  1219, -1633, -1633, -1633, -1633,
     678, -1633,    17,  3189,  1052,  1100,  1075,  3189,  4572,  3578,
   -1633, -1633, -1633,  1229, -1633,  1044,  1085,   642,   642,  1170,
    1170,  3144,  1040,  1235, 16334, 16334, 16334, 16334, 16334, 16334,
   17577,  2768, 15354, 16334, 16334, 16334, 16334, 16091, 16334, 16334,
   16334, 16334, 16334, 16334, 16334, 16334, 16334, 16334, 16334, 16334,
   16334, 16334, 16334, 16334, 16334, 16334, 16334, 16334, 16334, 16334,
   16334,  4572, -1633, -1633,  1163, -1633, -1633,  1047,  1055, -1633,
   -1633, -1633,   470, 17688, -1633,  1056, -1633, 16334,   615, -1633,
   -1633,    79, -1633,   667,  1248, -1633, -1633,   156,  1062,   615,
   11174, 17902, 17236, -1633,  2502, -1633,  5490,   861,  1248, -1633,
     423,   201, -1633, 17902,  1123,  1065, -1633,  1064,  1164, -1633,
    3189,   910,  3189,    51,  1251,  1186,   305, -1633,   715,   322,
   -1633, -1633, 16915, 13610, 17902, 17996,  1070,    50, -1633,  1073,
      50,  1082, 14487, 17902, 17292,  1088, 10565,  1080,  1089,  3189,
    1091,  1094,  3189,   945, -1633,   720,   444, 10565, 13610, -1633,
   -1633, -1633, -1633, -1633, -1633,  1139,  1092,  1270,  1189, 16437,
   16437, 16437, 16437, 16437, 16437,  1132, -1633, 17577,    90, 16437,
   -1633, -1633, -1633, 16915, 17902,  1095, -1633,    30,  1250,  1210,
   10159, -1633, -1633, -1633,  1102, 13610,  1096,   615, 16606, 16210,
    1104, 16334,  6911,   700,  1106, 13610,    68,   461, -1633,  1115,
   -1633,  3189,  4572, -1633,  1162, -1633, -1633,  3881,  1267,  1108,
   16334, -1633, 16334, -1633,  1109,  1105,  1298,  4386,  1107, 17996,
    1300,  1112,  1113,  1122,  1176,  1317,  1134, -1633, -1633, -1633,
   17340,  1133,  1323, 15204, 18084, 10545, 16334, 17950, 12777, 13182,
   13384,  4999,  5233, 14840, 14840, 14840, 14840,  2937,  2937,  2937,
    2937,  2937,  1120,  1120,   642,   642,   642,  1170,  1170,  1170,
    1170, -1633,  1137, -1633,  1150,  1155, -1633, -1633, 17996,  4572,
    3189,  3189, -1633,   667, 14912,   121, -1633, 16606, -1633, -1633,
   18121, 13610,  1135, -1633,  1140,   206, -1633,    86, 13610, -1633,
   -1633, -1633, 13610, -1633, 13610, -1633,   910, -1633, -1633,   129,
    1326,  1258, 13610, -1633,  1149,   615, 17902,  1164,  1153, -1633,
    1157,    50, 13610, 10565,  1158, -1633, -1633,   861, -1633, -1633,
    1175,  1161,  1169, -1633,  1166, 16437, -1633, 16437, -1633, -1633,
    1178,  1177,  1337,  1225,  1179, -1633,  1343,  1180,  1183,  1188,
   -1633,  1242,  1182,  1372, -1633, -1633,   615, -1633,  1357, -1633,
    1194, -1633, -1633,  1196,  1197,   165, -1633, -1633, 17996,  1198,
    1200, -1633,  4336, -1633, -1633, -1633, -1633, -1633, -1633,  1261,
    3189, -1633,  3189, -1633, 17996, 17395, -1633, -1633, 16334, -1633,
   16334, -1633, 16334, -1633, -1633, -1633, -1633, 16334, 17577, -1633,
   -1633, 16334, -1633, 16334, -1633, 10951, 16334,  1201,  7114, -1633,
   -1633,   667, -1633, -1633, -1633, -1633,   662, 14664, 14912,  1289,
   -1633,  1653,  1233,  1400, -1633, -1633, -1633,   867, 15929,   126,
     134,  1206,   861,  1002,   166, 17902, -1633, -1633, -1633,  1244,
   11767, 12376, 17902, -1633,    55,  1393,  1328, 13610, -1633, 17902,
   10565,  1295,  1164,   654,  1164,  1217, 17902,  1232, -1633,  1354,
    1214,  1795, -1633, -1633,    50, -1633, -1633,  1280, -1633, -1633,
   16437, -1633, 16437, -1633, 16437, -1633, -1633, -1633, -1633, 16437,
   -1633, 17577, -1633,  2067, -1633,  8941, -1633, -1633, -1633, -1633,
    9753, -1633, -1633, -1633,  8941,  3189, -1633,  1234, 16334, 17443,
   17996, 17996, 17996,  1296, 17996, 17498, 10951, -1633, -1633,   667,
   14912, 14912,  4572, -1633,  1419, 15501,    98, -1633, 14664,   861,
    4565, -1633,  1253, -1633,   136,  1236,   140, -1633, 15017, -1633,
   -1633, -1633,   143, -1633, -1633,  3239, -1633,  1240, -1633,  1360,
     687, -1633, 14841, -1633, 14841, -1633, -1633,  1426,   867, -1633,
   14136, -1633, -1633, -1633, -1633,  1430,  1363, 13610, -1633, 17902,
    1252,  1254,  1164,   582, -1633,  1295,  1164, -1633, -1633, -1633,
   -1633,  2238,  1257, 16437,  1311, -1633, -1633, -1633,  1312, -1633,
    8941,  9956,  9753, -1633, -1633, -1633,  8941, -1633, -1633, 17996,
   16334, 16334, 16334,  7317,  1260,  1262, -1633, 16334, -1633, 14912,
   -1633, -1633, -1633, -1633, -1633,  3189,   568,  1653, -1633, -1633,
   -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633,
     623, -1633,  1233, -1633, -1633, -1633, -1633, -1633,    97,   648,
   -1633,  1441,   144, 15207,  1360,  1447, -1633,  3189,   687, -1633,
   -1633,  1265,  1450, 13610, -1633, 17902, -1633,   108,  1271, -1633,
   -1633, -1633,  1164,   582, 14312, -1633,  1164, -1633, 16437, 16437,
   -1633, -1633, -1633, -1633,  7520, 17996, 17996, 17996, -1633, -1633,
   -1633, 17996, -1633,  1081,  1460,  1471,  1282, -1633, -1633, 16334,
   15017, 15017,  1424, -1633,  3239,  3239,   659, -1633, -1633, -1633,
   16334,  1401, -1633,  1305,  1291,   145, 16334, -1633,  4572, -1633,
   16334, 17902,  1405, -1633,  1480, -1633,  7723,  1292, -1633, -1633,
     582, -1633, -1633,  7926,  1301,  1375, -1633,  1392,  1340, -1633,
   -1633,  1399,  3189,  1320,   568, -1633, -1633, 17996, -1633, -1633,
    1332, -1633,  1472, -1633, -1633, -1633, -1633, 17996,  1496,   431,
   -1633, -1633, 17996,  1315, 17996, -1633,   117,  1316,  8129, -1633,
   -1633, -1633,  1314, -1633,  1318,  1338,  4572,  1002,  1333, -1633,
   -1633, -1633, 16334,  1335,    56, -1633,  1436, -1633, -1633, -1633,
    8332, -1633, 14912,   956, -1633,  1350,  4572,   781, -1633, 17996,
   -1633,  1329,  1518,   691,    56, -1633, -1633,  1448, -1633, 14912,
    1334, -1633,  1164,    59, -1633, -1633, -1633, -1633,  3189, -1633,
    1336,  1352,   148, -1633,  1339,   691,   184,  1164,  1331, -1633,
    3189,   617,  3189,    83,  1521,  1464,  1339, -1633,  1541, -1633,
     102, -1633, -1633, -1633,   196,  1537,  1470, 13610, -1633,   617,
    8535,  3189, -1633,  3189, -1633,  8738,   256,  1542,  1474, 13610,
   -1633, 17902, -1633, -1633, -1633, -1633, -1633,  1545,  1477, 13610,
   -1633, 17902, 13610, -1633, 17902, 17902
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1633, -1633, -1633,  -565, -1633, -1633, -1633,   501,   -47,   -27,
     483, -1633,  -263,  -527, -1633, -1633,   422,    -3,  1776, -1633,
    1771, -1633,  -514, -1633,    24, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633, -1633, -1633, -1633,  -332, -1633, -1633,  -164,
     147,    27, -1633, -1633, -1633, -1633, -1633, -1633,    31, -1633,
   -1633, -1633, -1633, -1633, -1633,    39, -1633, -1633,  1071,  1097,
    1093,   -86,  -696,  -886,   580,   631,  -336,   328,  -962, -1633,
     -57, -1633, -1633, -1633, -1633,  -746,   154, -1633, -1633, -1633,
   -1633,  -327, -1633,  -625, -1633,  -456, -1633, -1633,   973, -1633,
     -39, -1633, -1633, -1070, -1633, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633, -1633,   -75, -1633,    14, -1633, -1633, -1633,
   -1633, -1633,  -157, -1633,   114,  -985, -1633, -1632,  -360, -1633,
    -139,   107,  -127,  -339, -1633,  -163, -1633, -1633, -1633,   130,
     -17,     0,   151,  -743,   -81, -1633, -1633,    40, -1633,    -9,
   -1633, -1633,    -5,   -41,   -15, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633, -1633,  -600,  -863, -1633, -1633, -1633, -1633,
   -1633,  1440,  1211, -1633,   513, -1633,   380, -1633, -1633, -1633,
   -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633,
   -1633, -1633, -1633,    49,  -647,  -573, -1633, -1633, -1633, -1633,
   -1633,   447, -1633, -1633, -1633, -1633, -1633, -1633, -1633, -1633,
    -943,  -371,  2694,     9, -1633,  1619,  -406, -1633, -1633,  -495,
    3451,  3512, -1633,  -158, -1633, -1633,   524,   312,  -642, -1633,
   -1633,   606,   391,  -114, -1633,   393, -1633, -1633, -1633, -1633,
   -1633,   581, -1633, -1633, -1633,   127,  -899,  -159,  -435,  -434,
   -1633,   658,  -116, -1633, -1633,    35,    41,   629, -1633, -1633,
      37,   -16, -1633,  -364,    96,    75, -1633,  -323, -1633, -1633,
   -1633,  -466,  1243, -1633, -1633, -1633, -1633, -1633,   713,   413,
   -1633, -1633, -1633,  -363,  -683, -1633,  1187, -1033, -1633,   -66,
    -161,    20,   778, -1633,  -321, -1633,  -328, -1084, -1275,  -234,
     167, -1633,   486,   560, -1633, -1633, -1633, -1633,   511, -1633,
       7, -1146
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1059
static const yytype_int16 yytable[] =
{
     189,   191,   442,   193,   194,   195,   197,   198,   199,   343,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   492,   522,   234,   237,  1359,   414,   122,   344,
     953,   124,   403,   258,   660,   126,   406,   407,   812,   261,
     244,   662,   664,   127,   949,   800,   263,   269,   352,   272,
     514,   267,   353,  1172,   356,   786,   967,   364,  1464,   782,
     783,   442,   438,   349,   231,   231,   249,   416,   948,   990,
     880,   885,   250,   734,   868,   491,   775,   776,  1164,  1048,
     544,   261,   929,  1062,  1356,   399,  1189,  1246,   400,  1345,
     805,   413,   418,   548,   593,   595,   -39,   351,   828,    14,
    1423,   -39,  1200,   415,  1036,    14,  1804,  1649,    14,   432,
      14,   129,   -38,   808,   809,   556,  1953,   -38,   605,    14,
     610,   -74,   556,  1255,  1256,  1466,   -74,   251,   826,   900,
     901,   161,   891,  1805,  1360,  1592,   373,   549,  1605,  1173,
    1489,  1828,  -902,  1594,   416,  -353,  1822,   531,  -596,  1657,
    1235,   123,  1742,  1811,  1811,  1886,   950,  1649,   589,   556,
     738,   908,   601,  1225,   908,   908,  1945,  1494,   413,   418,
      14,  1404,  -119,  -714,   908,   908,  -119,  -103,   192,  1092,
     415,  1121,  -102,   533,  1174,   512,  -875,   525,     3,   780,
    -901,  1823,  -103,  -119,   784,   260,   427,  -102,  1871,   542,
    1887,  -875,   429,  -606,  1330,  1331,   254,   532,   418,  -912,
    1466,  -944,  1495,  -715,   392,   365,   512,  1258,  1093,   415,
    1288,   590,  1933,   541,   602,  1361,   257,  1284,   517,  1606,
    1226,   637,  -721,  1467,  1956,   415,  -539,   389,  1468,  1134,
      62,    63,    64,   179,  1469,   439,  1470,  -292,  -904,   551,
     520,   404,   551,  -276,  -811,    14,  -811,  1946,  -811,   261,
     562,  -809,  -947,  -292,  -903,   829,  1424,  1934,   493,  -907,
     231,  -902,  -946,  -906,  -911,  1175,   441,   704,   935,  1957,
     509,   510,   553,  1501,  1464,   -39,   558,  1416,  1471,  1472,
     571,  1473,   572,   573,  1806,  1203,  1650,  1651,   433,  1503,
    1045,   -38,   343,  1496,   557,  1047,  1509,   606,  1511,   611,
     -74,   635,   440,  1253,   513,  1257,   827,  1381,  1467,  -901,
     892,  1474,   584,  1468,  1593,    62,    63,    64,   179,  1469,
     439,  1470,  1595,   366,  -353,   523,   616,  1533,  1658,  1967,
    -944,  1743,  1812,  1861,  1186,   513,  1929,   893,   577,   909,
    1130,  1131,  1003,  1336,   364,   364,   596,   518,  1935,   787,
     615,   619,  1539,  1599,  1401,  1156,   442,  1351,  -604,   702,
    1958,   261,   415,  1471,  1472,  -887,  1473,  -904,   234,   627,
     261,   261,   627,   261,   354,   343,   517,   264,   641,   936,
    -909,  -947,  -888,  -903,   646,  -913,  -878,   440,  -907,  -916,
    1352,  -946,  -906,   231,   937,   344,  1488,   197,   371,   479,
     265,  -878,   231,   756,   618,   687,   372,  -603,  1613,   231,
    1615,   480,   403,   751,   752,   438,   699,  -900,   266,  -603,
    1968,   231,   215,    40,  1564,   632,   694,   634,   744,   349,
    1281,  1147,  1283,  1621,   509,   510,   705,   706,   707,   708,
     710,   711,   712,   713,   714,   715,   716,   717,   718,   719,
     720,   721,   722,   723,   724,   725,   726,   727,   728,   729,
     730,   731,   732,   733,  1344,   735,  -716,   736,   736,   739,
     663,   900,   901,   215,    40,   758,   375,   343,   244,   759,
     760,   761,   762,   763,   764,   765,   766,   767,   768,   769,
     770,   771,  1413,   408,  -887,   116,  -914,   736,   781,   376,
     699,   699,   736,   785,   249,   518,   956,  -722,   958,   759,
     250,  -888,   789,  1179,  1180,   377,   129,   614,  1768,  1197,
     875,   797,  1773,   799,   757,  1652,   630,   630,   673,   630,
     390,   699,  1252,   516,   897,   815,  -723,   492,   539,   816,
     519,   817, -1019,  -910,   270,   516,  -900,   342,   231,  1755,
     747,  1756,  1368,  -605,   601,  1370,   123,  1255,  1256,   369,
     660,  1358,   875,   429,   379,   251,   370,   662,   664,   378,
     831,   746,   985,   820,   984,  1553,   747,  1205,   390,  1346,
     992, -1019,   876,   382,   884,   884,   384,   401,   887,   379,
     491,   111,  1347,   379,   379,   636,   880,   779,  1127,   747,
    1128,   390,   409,   383,   939,   393,   394,   996,   643,   410,
     747,   385,  1348,   747,   390,  -876,   428,   509,   510,   379,
     746,   643,  1026,   165,   428,   956,   958,  1799,   974,   754,
    -876,   804,  1037,   958,   810,   390,    34,    35,    36,  1397,
     415,  1399,   421,  -717,   386,  1800,   230,   232,  1466,   216,
     390,  1426,   638,   393,   394,  1954,   387,   391,  1628,   509,
     510,   388,   945,   946,  1801,   404,  1029,   390,   405,  1807,
     420,   954,   509,   510,   424,  1038,   393,   394,  1827,  -914,
    1854,   964,  1830,   544,  1510,  1117,  1118,  1119,  1808,   393,
     394,  1809,   529,    14,   975,  1122,   428,   428,   431,  1855,
    1383,  1120,  1856,    81,    82,    83,    84,    85, -1019,   660,
     393,   394,   390,   443,   223,   434,   662,   664,   444,   643,
      89,    90,   231,   690,   392,   393,   394,   417,   983,   429,
    1374,   648,   649,   445,    99,   689,  1505,   446,   928,   509,
     510,  1384,   393,   394, -1019,   390,  1493, -1019,   104,   116,
    1042,  1043,   643,   116,  1237,  1238,  1467,   563,   995,  1330,
    1331,  1468,  1926,    62,    63,    64,   179,  1469,   439,  1470,
    1769,  1770,  1415,  1254,  1255,  1256,  1944,   476,   477,   478,
     447,   479,   231,   493,  1560,  1561,   673,   393,   394,  1597,
     448,  1040,   449,   480,   639,  1420,  1255,  1256,   645,  1458,
    1915,  1916,  1917,  -597,   417,  1941,  1942,   261,  1046,  1852,
    1853,  1471,  1472,   450,  1473,   423,   425,   426,  1924,   644,
     393,   394,  -598,   231,   639,   231,   645,   639,   645,   645,
     592,   594,   793,  1936,  -599,   440,   694,   694,  1026,  -600,
     583,  1057,  -601,   417,  1614,   482,   660,  1848,  1849,  1073,
    1076,   231,   535,   662,   664,   483,   485,   484,   884,   543,
     884,  1484,    55,   515,   884,   884,  1132,  -908,  -602,  -715,
      62,    63,    64,   179,   180,   439,  1653,   165,   521,  1622,
     397,   165,  1215,   526,   528,  1535,   429,   744,   480,   534,
      62,    63,    64,   179,   180,   439,  1151,  -912,  1152,   538,
     817,  1544,   516,   537,  -713,   545,   546,  1507,   554,   129,
     116,  1154,   567,   575, -1058,   578,  1204,   579,   231,   691,
     585,   586,   598,   342,   597,  1163,   379,   607,  1157,   600,
     608,   914,   916,   609,   231,   231,    62,    63,    64,    65,
      66,   439,   440,   122,  1167,   620,   124,    72,   486,   123,
     126,  1184,   665,   666,   621,   675,   676,  1181,   127,   677,
     942,  1192,   440,   679,  1193,   129,  1194,   688,    55,  1911,
     699,   792,  -124,   701,   790,  1903,   583,   379,   749,   379,
     379,   379,   379,   638,   794,   604,  1201,   795,   801,   802,
     488,   818,   244,   556,   612,  1903,   617,   822,   839,   825,
     840,   624,   774,   573,  1925,   123,   663,  1364,   440,   747,
    1630,   869,   871,   642,   872,   890,   905,   873,   249,  1636,
    1234,   747,   874,   747,   250,   583,   129,   894,   895,   898,
    1165,   899,  1240,  1643,  1610,  1289,   907,   912,   165,   807,
     910,   913,   779,   982,   810,   915,   161,   129,   917,   918,
     116,  1026,  1026,  1026,  1026,  1026,  1026,  1241,   919,  1338,
    1259,  1026,   920,   926,  1263,   931,   123,   660,   932,   673,
     866,   934,  -738,   940,   662,   664,   231,   231,   941,   251,
     943,   944,   947,   952,   960,   951,   673,   123,   962,   966,
     965,   968,   886,   691,   980,  1215,  1393,   971,   977,  1393,
     981,   978,   747,   989,   999,  1405,   997,  1000,  1001,  1784,
     973,    62,    63,    64,   179,   180,   439,  -719,  1049,  1039,
     884,  1059,  1061,   810,  1063,  1067,  1068,   923,   925,  1069,
    1071,  1339,  1084,  1064,  1085,  1340,  1086,  1087,  1090,  1070,
     624,  1133,   129,  1088,   129,  1125,  1089,  1135,  1137,  1139,
     660,  1140,   214,  1141,  1146,   663,  1150,   662,   664,  1153,
    1114,  1115,  1116,  1117,  1118,  1119,  1159,  1357,  1366,   954,
     122,  1161,  1867,   124,    50,  1162,  1168,   126,   165,  1120,
    1166,   699,   123,   440,   123,   127,  1170,  1199,  1202,  1208,
    1187,  1196,   699,  1340,  1207,   379,  1377,  -915,  1218,  1380,
    1219,  1220,  1227,  1221,  1228,  1142,  1222,  1223,  1224,   231,
     218,   219,   220,   221,   222,  1229,  1231,  1026,  1243,  1026,
    1245,  1248,  1249,  1251,  1260,  1262,  1261,  1408,  1267,  1120,
     261,  1268,   183,  1271,  1272,    91,  1322,  1324,    93,    94,
    1422,    95,   184,    97,  1327,  1325,  1826,  1334,  1337,  1354,
    1914,   985,  1355,   129,  1411,  1362,  1833,  1367,  1428,  1363,
     231,  1516,  1369,  1517,  1181,  1385,   107,  1375,  1371,  1387,
    1010,  1836,  1409,   161,  1373,   231,   231,  1410,  1376,  1400,
    1378,  1019,  1379,  1032,  1407,  1386,  1952,  1427,  1430,  1432,
    1412,  1417,   663,   123,  1421,  1433,  1436,  1438,  1437,  1442,
    1441,  1868,  1447,   116,   673,  1444,  1445,   673,    62,    63,
      64,    65,    66,   439,   963,  1446,  1448,  1055,   116,    72,
     486,  1450,  1453,  1452,  1457,  1486,  1485,  1462,  1463,  1487,
    1497,  1498,  1230,  1490,  1598,  1500,  1520,  1491,  1459,  1492,
    1502,   442,  1524,  1460,  1504,  1508,  1890,  1499,  1466,   129,
    1513,  1522,  1026,  1515,  1026,  1514,  1026,  1506,   699,   116,
     487,  1026,   488,  1512,   231,  1518,  1129,   691,  1529,  1530,
    1519,  1531,  1523,  1526,   994,   489,  1527,   490,  1269,  1534,
     440,  1528,  1536,  1537,  1538,  1273,  1541,  1545,  1542,   123,
    1557,  1568,  1581,    14,  1596,  1143,  1624,  1607,  1625,  1601,
    1626,  1608,  1611,  1619,  1616,  1627,  1623,   473,   474,   475,
     476,   477,   478,  1950,   479,  1033,  1753,  1034,  1955,  1617,
     116,  1638,  1641,  1647,  1655,  1656,   480,  1546,  1750,  1547,
    1757,   165,  1478,  1751,  1763,   583,  1764,  1778,  1779,  1766,
    1767,   116,  1478,  1053,  1776,  1810,   165,   774,  1789,   807,
    1790,  1816,  1483,  1819,  1820,  1026,  1467,   226,   226,  1842,
    1825,  1468,  1483,    62,    63,    64,   179,  1469,   439,  1470,
    1844,   214,  1846,  1850,  1858,  1591,  1859,  1860,  1865,  1866,
    1874,  1870,  1609,   379,  -349,   699,  1646,   165,   673,  1873,
    1876,  1877,  1879,    50,  1881,  1212,  1212,  1019,  1805,  1777,
    1882,  1885,  1891,  1888,  1892,  1898,  1893,  1900,  1648,  1905,
    1138,  1471,  1472,   663,  1473,  1909,  1912,  1913,  1584,  1388,
    1937,  1921,  1923,  1927,  1930,  1947,   624,  1149,   807,   218,
     219,   220,   221,   222,   116,   440,   116,  1948,   116,  1928,
    1951,  1959,  1637,  1960,  1618,  1326,  1969,  1970,   165,  1972,
    1973,  1908,   753,  1158,  1634,   129,  1198,    93,    94,  1264,
      95,   184,    97,  1922,  1414,  1783,  1543,  1920,   888,   165,
    1774,   750,  1798,   748,  1654,  1803,  1932,  1588,  1439,  1962,
     493,  1815,  1443,   583,  1818,   107,  1585,  1449,   633,  1569,
    1772,  1282,  1765,  1398,  1454,   123,   663,  1349,  1274,  1389,
    1478,  1214,  1390,  1232,  1178,  1949,  1478,  1074,  1478,   700,
    1026,  1026,   866,  1964,  1590,  1883,   626,  1329,  1559,  1266,
    1483,     0,  1321,     0,     0,     0,  1483,     0,  1483,     0,
    1478,   673,   129,     0,     0,     0,   228,   228,     0,     0,
       0,   129,     0,     0,     0,  1781,  1634,   116,     0,     0,
    1483,     0,  1794,     0,  1831,  1832,     0,     0,     0,     0,
       0,     0,   165,   226,   165,     0,   165,     0,  1053,  1247,
       0,     0,   123,     0,  1570,     0,     0,     0,     0,     0,
       0,   123,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1521,     0,   343,     0,  1525,     0,
       0,     0,     0,     0,     0,  1532,     0,     0,     0,     0,
    1019,  1019,  1019,  1019,  1019,  1019,  1813,     0,  1478,     0,
    1019,     0,     0,     0,   214,     0,     0,   129,  1896,     0,
       0,   116,     0,   129,     0,  1758,     0,     0,  1483,     0,
     129,     0,  1863,   116,     0,     0,    50,     0,  1821,     0,
       0,     0,     0,  1429,  1817,     0,     0,     0,     0,     0,
     442,     0,     0,     0,     0,     0,     0,   123,  1571,     0,
       0,     0,     0,   123,     0,   165,     0,     0,     0,     0,
     123,  1572,   218,   219,   220,   221,   222,  1573,     0,  1466,
       0,     0,     0,   225,   225,     0,   226,   241,     0,     0,
       0,  1365,     0,     0,   183,   226,     0,    91,  1574,     0,
      93,    94,   226,    95,  1575,    97,     0,   345,     0,     0,
    1461,     0,   241,     0,   226,     0,     0,   214,     0,   215,
      40,     0,     0,     0,    14,     0,     0,     0,   107,  1878,
       0,     0,   228,     0,     0,     0,     0,     0,     0,    50,
       0,     0,  1406,     0,     0,     0,     0,     0,     0,   165,
       0,     0,     0,     0,     0,     0,     0,   624,  1053,     0,
       0,   165,     0,     0,     0,     0,  1019,     0,  1019,     0,
       0,   129,     0,     0,     0,   218,   219,   220,   221,   222,
       0,     0,     0,     0,     0,     0,     0,  1467,     0,     0,
       0,     0,  1468,     0,    62,    63,    64,   179,  1469,   439,
    1470,   772,     0,    93,    94,   954,    95,   184,    97,     0,
       0,   123,     0,   129,     0,     0,     0,  1940,     0,   954,
     129,     0,  1961,     0,     0,     0,     0,     0,     0,     0,
       0,   107,     0,     0,  1971,   773,     0,   111,  1940,   116,
    1965,   226,  1471,  1472,  1974,  1473,   624,  1975,   342,     0,
       0,     0,     0,   123,  1586,   129,     0,     0,     0,     0,
     123,     0,     0,     0,  1897,   228,   440,     0,     0,     0,
       0,     0,     0,     0,   228,  1620,     0,   129,     0,     0,
       0,   228,     0,     0,     0,     0,     0,     0,     0,   225,
       0,   673,     0,   228,     0,   123,     0,     0,     0,     0,
       0,  1019,     0,  1019,   661,  1019,     0,     0,     0,     0,
    1019,   673,     0,     0,     0,     0,   116,   123,     0,     0,
     673,   116,     0,     0,     0,   116,     0,     0,     0,     0,
       0,     0,     0,     0,   345,     0,   345,   129,     0,   241,
       0,   241,   129,   379,     0,     0,   583,     0,     0,   342,
       0,  1466,     0,     0,     0,     0,     0,     0,     0,  1739,
       0,     0,     0,     0,     0,     0,  1746,   165,     0,     0,
       0,     0,     0,   342,     0,   342,     0,   123,     0,     0,
       0,   342,   123,     0,     0,     0,  1843,  1845,     0,     0,
       0,     0,   345,     0,     0,     0,    14,   241,     0,     0,
       0,     0,     0,     0,  1019,     0,     0,     0,     0,     0,
       0,   116,   116,   116,     0,   226,     0,   116,     0,     0,
     228,     0,   225,     0,   116,     0,     0,     0,     0,     0,
       0,   225,     0,     0,     0,     0,     0,     0,   225,     0,
       0,     0,     0,     0,   165,     0,     0,     0,     0,   165,
     225,     0,     0,   165,     0,     0,     0,     0,     0,  1467,
       0,   225,     0,     0,  1468,     0,    62,    63,    64,   179,
    1469,   439,  1470,     0,     0,   226,     0,     0,     0,     0,
     345,     0,     0,   345,     0,   241,     0,     0,   241,   524,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,     0,     0,     0,     0,     0,     0,  1209,  1210,
    1211,   214,     0,     0,  1471,  1472,   226,  1473,   226,     0,
       0,     0,  1466,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,   583,   241,     0,     0,   440,   165,
     165,   165,   507,   508,   226,   165,     0,  1629,     0,     0,
       0,     0,   165,     0,     0,   342,     0,     0,     0,  1019,
    1019,   214,     0,   215,    40,   116,     0,    14,     0,   218,
     219,   220,   221,   222,  1837,     0,     0,   225,     0,     0,
       0,  1739,  1739,    50,     0,  1746,  1746,     0,     0,     0,
       0,     0,     0,     0,   228,     0,     0,    93,    94,   379,
      95,   184,    97,     0,     0,     0,     0,   116,     0,   509,
     510,   226,     0,     0,   116,     0,     0,     0,     0,   218,
     219,   220,   221,   222,   345,   107,   835,   226,   226,   241,
    1467,   241,     0,     0,   854,  1468,     0,    62,    63,    64,
     179,  1469,   439,  1470,     0,   772,     0,    93,    94,   116,
      95,   184,    97,     0,   228,     0,     0,  1895,     0,     0,
       0,     0,     0,     0,     0,   854,     0,     0,     0,     0,
       0,   116,   678,     0,     0,   107,     0,  1910,     0,   806,
       0,   111,     0,     0,     0,  1471,  1472,     0,  1473,     0,
       0,     0,     0,   165,     0,   228,     0,   228,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   440,
       0,     0,     0,     0,     0,     0,   345,   345,  1775,     0,
       0,   241,   241,   228,     0,   345,     0,     0,     0,     0,
     241,   116,     0,     0,     0,   165,   116,     0,     0,     0,
       0,     0,   165,     0,     0,     0,     0,     0,     0,     0,
       0,   225,     0,     0,     0,   524,   495,   496,   497,   498,
     499,   500,   501,   502,   503,   504,   505,   506,     0,   226,
     226,     0,     0,     0,     0,     0,     0,   165,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     228,     0,   451,   452,   453,     0,     0,     0,     0,   165,
       0,     0,     0,     0,     0,     0,   228,   228,   507,   508,
       0,   225,   454,   455,     0,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,     0,   479,
     661,     0,     0,     0,     0,     0,   241,     0,     0,     0,
       0,   480,   225,     0,   225,     0,     0,     0,     0,   165,
       0,     0,     0,     0,   165,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   509,   510,     0,     0,     0,
     225,   854,   832,  1066,     0,     0,     0,     0,   241,     0,
     345,   345,     0,     0,     0,   241,   241,   854,   854,   854,
     854,   854,   226,     0,     0,     0,     0,     0,     0,   854,
       0,     0,   494,   495,   496,   497,   498,   499,   500,   501,
     502,   503,   504,   505,   506,   241,     0,     0,     0,     0,
       0,     0,   214,     0,     0,     0,     0,     0,   803,     0,
       0,     0,   833,     0,     0,     0,     0,   225,   228,   228,
       0,     0,     0,   226,    50,     0,     0,     0,     0,     0,
       0,   241,     0,   225,   225,   507,   508,     0,   226,   226,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   345,     0,     0,  1342,     0,   241,   241,   661,
     218,   219,   220,   221,   222,     0,     0,   225,   345,     0,
       0,   227,   227,   241,     0,   243,     0,     0,     0,     0,
       0,   345,   183,     0,     0,    91,   241,     0,    93,    94,
       0,    95,   184,    97,   854,   834,     0,   241,     0,     0,
       0,     0,   509,   510,     0,     0,     0,     0,     0,     0,
     345,     0,     0,     0,     0,   241,   107,     0,     0,   241,
       0,     0,     0,     0,     0,     0,     0,   226,  1094,  1095,
    1096,     0,   241,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1097,
       0,   228,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,     0,   225,   225,     0,     0,     0,
       0,     0,     0,     0,   345,     0,     0,  1120,   345,   241,
     835,     0,     0,   241,     0,   241,   661,     0,     0,     0,
       0,     0,   228,     0,     0,     0,     0,     0,     0,     0,
     854,   854,   854,   854,   854,   854,   225,   228,   228,   854,
     854,   854,   854,   854,   854,   854,   854,   854,   854,   854,
     854,   854,   854,   854,   854,   854,   854,   854,   854,   854,
     854,   854,   854,   854,   854,   854,   854,   524,   495,   496,
     497,   498,   499,   500,   501,   502,   503,   504,   505,   506,
       0,     0,     0,   854,   524,   495,   496,   497,   498,   499,
     500,   501,   502,   503,   504,   505,   506,   227, -1059, -1059,
   -1059, -1059, -1059,   471,   472,   473,   474,   475,   476,   477,
     478,   345,   479,   345,     0,     0,   241,     0,   241,     0,
     507,   508,     0,     0,   480,     0,   228,     0,   225,     0,
       0,     0,     0,     0,     0,  1287,     0,   507,   508,     0,
     345,     0,     0,   345,     0,   241,     0,     0,   241,     0,
   -1059, -1059, -1059, -1059, -1059,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,     0,     0,   241,   241,   241,   241,   241,
     241,     0,     0,   225,     0,   241,  1120,     0,     0,   225,
       0,     0,     0,     0,     0,     0,     0,   509,   510,     0,
       0,     0,     0,     0,   225,   225,     0,   854,     0,     0,
       0,     0,   345,     0,   509,   510,     0,   241,   345,     0,
       0,     0,     0,   241,     0,     0,   854,     0,   854,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     227,     0,     0,     0,     0,     0,     0,   661,     0,   227,
       0,     0,   854,     0,     0,     0,   227,     0,     0,     0,
     896,     0,     0,     0,     0,     0,     0,     0,   227,     0,
       0,     0,     0,     0,   451,   452,   453,     0,     0,   227,
       0,   345,   345,     0,     0,     0,   241,   241,     0,     0,
     241,     0,     0,   225,   454,   455,     0,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,     0,     0,     0,     0,     0,     0,     0,     0,
     661,     0,     0,   480,  1094,  1095,  1096,     0,     0,     0,
       0,   241,     0,   241,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   243,     0,  1097,     0,     0,  1098,  1099,
    1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
     214,   345,     0,   345,     0,     0,   241,     0,   241,     0,
       0,     0,     0,  1120,   854,   227,   854,     0,   854,     0,
     283,     0,    50,   854,   225,     0,     0,   854,     0,   854,
       0,     0,   854,     0,     0,     0,     0,     0,   345,     0,
       0,     0,     0,   241,   241,     0,     0,   241,   285,   345,
       0,     0,     0,     0,   241,     0,     0,     0,   218,   219,
     220,   221,   222,     0,     0,     0,     0,     0,     0,     0,
     214,     0,   861,     0,     0,     0,     0,     0,     0,     0,
    1270,     0,     0,   397,   927,     0,    93,    94,     0,    95,
     184,    97,    50,     0,     0,     0,   241,     0,   241,     0,
     241,     0,     0,   861,     0,   241,     0,   225,     0,     0,
       0,     0,     0,     0,   107,     0,   345,     0,   398,     0,
     214,   241,     0,     0,   854,     0,     0,   569,   218,   219,
     220,   221,   222,   570,     0,     0,   241,   241,     0,   345,
       0,     0,    50,     0,   241,     0,   241,     0,     0,     0,
     183,     0,     0,    91,   336,     0,    93,    94,     0,    95,
     184,    97,     0,   345,     0,   345,     0,     0,   241,     0,
     241,   345,     0,     0,   340,     0,   241,     0,   218,   219,
     220,   221,   222,     0,   107,   341,     0,     0,     0,   227,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   241,
       0,     0,     0,     0,  1744,     0,    93,    94,  1745,    95,
     184,    97,     0,     0,     0,     0,   854,   854,   854,     0,
       0,     0,     0,   854,     0,   241,   345,   451,   452,   453,
       0,   241,     0,   241,   107,  1585,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   454,   455,   227,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,     0,     0,     0,   229,   229,
       0,     0,   247,     0,  1025,     0,   480,     0,     0,     0,
     227,     0,   227,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   283,   479,     0,     0,     0,     0,   227,   861,
       0,     0,     0,     0,     0,   480,     0,     0,   345,     0,
       0,     0,     0,   241,     0,   861,   861,   861,   861,   861,
     285,     0,     0,     0,     0,   345,     0,   861,     0,     0,
     241,     0,     0,     0,   241,   241,     0,     0,     0,     0,
       0,     0,   214,  1124,  1838,     0,     0,     0,     0,   241,
       0,     0,     0,     0,     0,   854,     0,     0,     0,     0,
       0,     0,     0,     0,    50,   227,   854,     0,     0,     0,
       0,     0,   854,     0,     0,     0,   854,     0,     0,  1145,
       0,   227,   227,     0,     0,     0,     0,     0,     0,   832,
       0,     0,     0,   345,     0,     0,     0,   959,   241,   569,
     218,   219,   220,   221,   222,   570,  1145,     0,     0,     0,
       0,     0,     0,     0,     0,   227,     0,     0,     0,     0,
       0,     0,   183,     0,     0,    91,   336,     0,    93,    94,
       0,    95,   184,    97,     0,  1072,     0,     0,   854,   214,
       0,     0,   861,     0,     0,  1188,   340,     0,   241,   833,
       0,     0,     0,     0,     0,     0,   107,   341,     0,     0,
       0,    50,     0,     0,   229,   241,     0,   243,     0,   345,
       0,     0,     0,     0,   241,     0,     0,     0,     0,     0,
    1025,   345,     0,   345,     0,     0,   241,     0,   241,     0,
       0,     0,     0,     0,     0,     0,     0,   218,   219,   220,
     221,   222,   345,     0,   345,     0,     0,   241,     0,   241,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   183,
       0,     0,    91,   227,   227,    93,    94,     0,    95,   184,
      97,     0,  1265,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   214,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   107,     0,     0,     0,     0,   861,   861,
     861,   861,   861,   861,   227,    50,     0,   861,   861,   861,
     861,   861,   861,   861,   861,   861,   861,   861,   861,   861,
     861,   861,   861,   861,   861,   861,   861,   861,   861,   861,
     861,   861,   861,   861,   861,     0,     0,   229,     0,     0,
       0,   218,   219,   220,   221,   222,   229,     0,     0,     0,
       0,   861,     0,   229,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   229,     0,     0,     0,    93,
      94,     0,    95,   184,    97,     0,   247,     0,     0,     0,
     451,   452,   453,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   227,   107,   701,     0,
     454,   455,     0,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,     0,   479,     0,     0,
       0,     0,   283,  1025,  1025,  1025,  1025,  1025,  1025,   480,
       0,   227,     0,  1025,     0,     0,     0,   227,     0,     0,
     247,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     285,     0,   227,   227,     0,   861,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   214,     0,   861,     0,   861,     0,   451,   452,
     453,     0,   229,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,   454,   455,
     861,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,     0,   479,     0,     0,     0,   569,
     218,   219,   220,   221,   222,   570,     0,   480,  1465,   862,
       0,   227,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   183,     0,     0,    91,   336,     0,    93,    94,
     998,    95,   184,    97,     0,  1431,     0,     0,     0,     0,
     862,     0,     0,     0,     0,     0,   340,     0,   451,   452,
     453,     0,     0,     0,     0,     0,   107,   341,     0,  1025,
       0,  1025,     0,     0,     0,     0,     0,     0,   454,   455,
     863,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,     0,   479,     0,     0,     0,     0,
       0,   889,     0,     0,     0,     0,     0,   480,     0,     0,
       0,     0,   861,     0,   861,     0,   861,     0,     0,     0,
       0,   861,   227,     0,     0,   861,   229,   861,     0,     0,
     861,     0,     0,     0,   451,   452,   453,     0,  1002,     0,
       0,     0,  1567,     0,     0,  1580,     0,     0,     0,     0,
       0,     0,     0,     0,   454,   455,     0,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,     0,     0,     0,     0,   229,     0,     0,     0,
       0,   214,     0,   480,  1025,     0,  1025,     0,  1025,     0,
       0,     0,     0,  1025,   214,   227,   921,     0,   922,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,   861,     0,     0,     0,    50,   229,     0,   229,
       0,     0,     0,     0,  1644,  1645,     0,     0,  1136,     0,
       0,     0,     0,     0,  1580,     0,     0,     0,     0,   218,
     219,   220,   221,   222,     0,   229,   862,     0,     0,     0,
       0,     0,   218,   219,   220,   221,   222,     0,     0,     0,
       0,     0,   862,   862,   862,   862,   862,    93,    94,     0,
      95,   184,    97,     0,   862,     0,     0,     0,     0,     0,
      93,    94,     0,    95,   184,    97,     0,  1025,     0,     0,
       0,     0,     0,     0,     0,   107,   973,     0,     0,     0,
       0,     0,     0,     0,   861,   861,   861,  1054,   107,     0,
       0,   861,   229,  1792,  1195,     0,   451,   452,   453,     0,
       0,  1580,     0,  1077,  1078,  1079,  1080,  1081,   229,   229,
       0,     0,     0,     0,     0,  1091,   454,   455,  1423,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   247,   479,     0,     0,  1094,  1095,  1096,     0,
       0,     0,     0,     0,     0,   480,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1097,     0,   862,
    1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,     0,     0,   247,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1120,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1025,  1025,     0,     0,     0,     0,     0,     0,
    1185,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   861,     0,     0,     0,     0,     0,     0,
     229,   229,     0,     0,   861,     0,     0,     0,     0,     0,
     861,     0,     0,     0,   861,     0,     0,     0,     0,     0,
       0,     0,  1440,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1424,   862,   862,   862,   862,   862,
     862,   247,     0,     0,   862,   862,   862,   862,   862,   862,
     862,   862,   862,   862,   862,   862,   862,   862,   862,   862,
     862,   862,   862,   862,   862,   862,   862,   862,   862,   862,
     862,   862,     0,     0,     0,     0,   861,     0,     0,     0,
     214,     0,     0,     0,     0,     0,  1907,     0,   862,     0,
    1065,     0,     0,     0,     0,     0,  1081,  1277,     0,     0,
    1277,     0,    50,  1567,     0,  1290,  1293,  1294,  1295,  1297,
    1298,  1299,  1300,  1301,  1302,  1303,  1304,  1305,  1306,  1307,
    1308,  1309,  1310,  1311,  1312,  1313,  1314,  1315,  1316,  1317,
    1318,  1319,  1320,   229,     0,     0,     0,     0,   218,   219,
     220,   221,   222,     0,     0,     0,   214,     0,     0,  1328,
       0,     0,     0,   214,     0,     0,     0,     0,     0,     0,
     183,     0,     0,    91,     0,     0,    93,    94,    50,    95,
     184,    97,     0,     0,     0,    50,     0,     0,   247,     0,
       0,     0,     0,     0,   229,     0,     0,     0,     0,     0,
    1571,     0,     0,     0,   107,     0,     0,     0,     0,   229,
     229,     0,   862,  1572,   218,   219,   220,   221,   222,  1573,
       0,   218,   219,   220,   221,   222,   214,     0,     0,     0,
       0,   862,     0,   862,     0,     0,   183,     0,     0,    91,
      92,     0,    93,    94,     0,    95,  1575,    97,    50,    93,
      94,     0,    95,   184,    97,   273,   274,   862,   275,   276,
       0,     0,   277,   278,   279,   280,     0,     0,     0,     0,
     107,     0,     0,  1418,     0,     0,     0,   107,     0,   281,
       0,   282,     0,     0,   218,   219,   220,   221,   222,     0,
       0,     0,  1434,     0,  1435,     0,     0,     0,   229,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   284,
     436,     0,    93,    94,     0,    95,   184,    97,  1455,     0,
       0,     0,     0,   286,   287,   288,   289,   290,   291,   292,
       0,     0,     0,   214,     0,   215,    40,     0,     0,     0,
     107,     0,     0,     0,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,    50,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,     0,
     327,     0,   742,   329,   330,   331,     0,     0,     0,   332,
     580,   218,   219,   220,   221,   222,   581,     0,     0,   862,
       0,   862,     0,   862,     0,     0,     0,     0,   862,   247,
       0,     0,   862,   582,   862,     0,     0,   862,     0,    93,
      94,     0,    95,   184,    97,   337,     0,   338,     0,     0,
     339,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   107,     0,     0,
       0,   743,     0,   111,     0,     0,     0,     0,     0,     0,
    1549,     0,  1550,     0,  1551,     0,     0,     0,     0,  1552,
       0,     0,     0,  1554,     0,  1555,     0,     0,  1556,     0,
       0,     0,     0,     0,     0,     0,     0,   451,   452,   453,
       0,     0,   247,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   454,   455,   862,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   480,  1102,  1103,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,     5,     6,     7,     8,     9,
    1639,     0,     0,     0,     0,    10,     0,     0,  1120,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   412,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
     755,   862,   862,   862,     0,     0,     0,     0,   862,     0,
       0,     0,    15,    16,     0,     0,     0,  1797,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,  1785,  1786,  1787,    50,     0,     0,     0,  1791,
       0,     0,     0,    55,     0,     0,     0,  1206,     0,     0,
       0,    62,    63,    64,   179,   180,   181,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   182,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     862,     0,     0,   111,   112,     0,   113,   114,     0,     0,
       0,   862,     0,     0,     0,     0,     0,   862,     0,     0,
       0,   862,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,     0,
       5,     6,     7,     8,     9,  1880,     0,     0,     0,     0,
      10,     0,  1120,     0,     0,     0,     0,     0,     0,     0,
       0,  1847,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,  1857,     0,     0,     0,     0,     0,  1862,     0,
       0,     0,  1864,   862,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,  1899,    52,    53,    54,    55,    56,
      57,    58,     0,    59,    60,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,    88,    89,
      90,    91,    92,     0,    93,    94,     0,    95,    96,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,   103,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,  1155,   111,   112,
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
      54,    55,    56,    57,    58,     0,    59,    60,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,    88,    89,    90,    91,    92,     0,    93,    94,     0,
      95,    96,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,   103,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    1343,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
       0,    52,    53,    54,    55,    56,    57,    58,     0,    59,
      60,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,    88,    89,    90,    91,    92,     0,
      93,    94,     0,    95,    96,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,   103,     0,   104,   105,   106,     0,     0,   107,   108,
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
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,   680,   111,   112,     0,
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
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1123,
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
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  1169,   111,   112,     0,   113,   114,     5,     6,
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
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   183,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   184,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  1242,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,  1244,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
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
       0,    49,  1419,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   183,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   184,    97,    98,     0,     0,    99,     0,     0,
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
      87,     0,     0,     0,     0,   183,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   184,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,  1558,   111,   112,     0,   113,   114,
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
      86,     0,     0,    87,     0,     0,     0,     0,   183,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   184,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,  1788,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,  1834,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
       0,     0,     0,     0,   183,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   184,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   108,
       0,   109,   110,  1869,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,  1872,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,    98,
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
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1889,
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
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  1906,   111,   112,     0,   113,   114,     5,     6,
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
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   183,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   184,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  1963,   111,   112,     0,   113,
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
       0,    86,     0,     0,    87,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   109,   110,  1966,   111,
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
       0,     0,   183,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   184,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   108,     0,   109,
     110,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,   552,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,    61,    62,    63,    64,   179,   180,    67,     0,
      68,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   183,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   184,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,   819,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,    63,    64,   179,
     180,    67,     0,    68,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   183,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   184,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,  1056,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,    61,    62,
      63,    64,   179,   180,    67,     0,    68,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,  1633,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,    63,    64,   179,   180,    67,     0,    68,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   183,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   184,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   108,
       0,   109,   110,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,  1780,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,   179,   180,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,   179,   180,    67,     0,    68,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   179,   180,   181,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   182,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   185,     0,
     350,     0,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,  1097,     0,    10,  1098,
    1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,     0,     0,   695,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1120,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   179,   180,   181,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   182,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   183,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   184,    97,     0,   696,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   185,     0,     0,     0,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     179,   180,   181,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   182,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   185,     0,     0,   814,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,  1098,  1099,  1100,  1101,  1102,
    1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,     0,     0,  1182,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1120,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   179,   180,   181,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   182,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   183,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   184,    97,     0,  1183,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   185,     0,     0,
       0,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   412,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   179,   180,   181,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   182,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   183,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   184,    97,     0,     0,     0,
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
      50,     0,     0,     0,     0,   196,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   179,
     180,   181,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   182,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   183,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   184,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,  1236,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   185,     0,     0,     0,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,     0,   479,     0,   233,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   480,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   179,   180,   181,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   182,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   185,   451,   452,   453,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   454,   455,     0,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,   480,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   179,   180,   181,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     182,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   183,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   184,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,  1603,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   185,
       0,   268,   452,   453,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,   454,   455,     0,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,     0,   479,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
     480,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   179,   180,
     181,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   182,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   185,     0,   271,     0,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   412,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   179,   180,   181,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   182,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,     0,     0,     0,    99,     0,     0,   100,     0,
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
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   179,   180,   181,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   182,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,  1604,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   185,   550,
       0,     0,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   709,   479,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     480,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   179,   180,   181,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   182,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   183,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   184,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   185,     0,     0,     0,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,     0,     0,     0,   755,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1120,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     179,   180,   181,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   182,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   183,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   184,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   185,     0,     0,     0,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,     0,   479,     0,     0,   796,
       0,     0,     0,     0,     0,     0,     0,     0,   480,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   179,   180,   181,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   182,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   183,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   184,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   185,     0,     0,
       0,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,  1100,  1101,
    1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,     0,     0,
       0,     0,   798,     0,     0,     0,     0,     0,     0,     0,
       0,  1120,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   179,   180,   181,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   182,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   183,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   184,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     185,     0,     0,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
       0,     0,     0,     0,     0,  1233,     0,     0,     0,     0,
       0,     0,     0,  1120,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   179,
     180,   181,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   182,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   183,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   184,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   185,   451,   452,   453,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   454,   455,     0,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,   480,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   179,   180,   181,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   182,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,   481,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   185,   451,   452,   453,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,   823,    10,   454,   455,     0,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,   480,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,   640,    39,    40,     0,   824,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   179,   180,   181,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     182,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   183,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   184,    97,     0,   273,   274,    99,
     275,   276,   100,     0,   277,   278,   279,   280,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   185,
       0,   281,     0,   282,   111,   112,     0,   113,   114,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,     0,   479,     0,     0,
       0,   284,     0,     0,     0,     0,     0,     0,     0,   480,
       0,     0,     0,     0,     0,   286,   287,   288,   289,   290,
     291,   292,     0,     0,     0,   214,     0,   215,    40,     0,
       0,     0,     0,     0,     0,     0,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,    50,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   214,   327,     0,   328,   329,   330,   331,     0,     0,
       0,   332,   580,   218,   219,   220,   221,   222,   581,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,   273,
     274,     0,   275,   276,     0,   582,   277,   278,   279,   280,
       0,    93,    94,     0,    95,   184,    97,   337,     0,   338,
       0,     0,   339,   281,     0,   282,     0,   283,     0,   218,
     219,   220,   221,   222,     0,     0,     0,     0,     0,   107,
       0,     0,     0,   743,     0,   111,     0,     0,     0,     0,
       0,     0,     0,   284,   359,   285,     0,    93,    94,     0,
      95,   184,    97,     0,     0,     0,     0,   286,   287,   288,
     289,   290,   291,   292,     0,     0,     0,   214,     0,     0,
       0,     0,     0,     0,     0,   107,     0,     0,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,    50,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,     0,   327,     0,     0,   329,   330,   331,
       0,     0,     0,   332,   333,   218,   219,   220,   221,   222,
     334,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   335,     0,     0,
      91,   336,     0,    93,    94,     0,    95,   184,    97,   337,
       0,   338,     0,     0,   339,   273,   274,     0,   275,   276,
       0,   340,   277,   278,   279,   280,     0,     0,     0,     0,
       0,   107,   341,     0,     0,     0,  1759,     0,     0,   281,
       0,   282,   455,   283,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,     0,   479,   284,
       0,   285,     0,     0,     0,     0,     0,     0,     0,     0,
     480,     0,     0,   286,   287,   288,   289,   290,   291,   292,
       0,     0,     0,   214,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,    50,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,     0,
     327,     0,     0,   329,   330,   331,     0,     0,     0,   332,
     333,   218,   219,   220,   221,   222,   334,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   335,     0,     0,    91,   336,     0,    93,
      94,     0,    95,   184,    97,   337,     0,   338,     0,     0,
     339,   273,   274,     0,   275,   276,     0,   340,   277,   278,
     279,   280,     0,     0,     0,     0,     0,   107,   341,     0,
       0,     0,  1829,     0,     0,   281,     0,   282,     0,   283,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,   284,     0,   285,     0,     0,
       0,     0,     0,     0,     0,     0,   480,     0,     0,   286,
     287,   288,   289,   290,   291,   292,     0,     0,     0,   214,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,    50,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,     0,   327,     0,   328,   329,
     330,   331,     0,     0,     0,   332,   333,   218,   219,   220,
     221,   222,   334,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   335,
       0,     0,    91,   336,     0,    93,    94,     0,    95,   184,
      97,   337,     0,   338,     0,     0,   339,   273,   274,     0,
     275,   276,     0,   340,   277,   278,   279,   280,     0,     0,
       0,     0,     0,   107,   341,     0,     0,     0,     0,     0,
       0,   281,     0,   282,     0,   283, -1059, -1059, -1059, -1059,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,     0,     0,     0,     0,     0,
       0,   284,     0,   285,     0,     0,   480,     0,     0,     0,
       0,     0,     0,     0,     0,   286,   287,   288,   289,   290,
     291,   292,     0,     0,     0,   214,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,    50,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,     0,   327,     0,     0,   329,   330,   331,     0,     0,
       0,   332,   333,   218,   219,   220,   221,   222,   334,     0,
       0,     0,     0,     0,     0,     0,   214,     0,     0,     0,
       0,     0,     0,     0,     0,   335,     0,     0,    91,   336,
       0,    93,    94,     0,    95,   184,    97,   337,    50,   338,
       0,     0,   339,     0,   273,   274,     0,   275,   276,   340,
    1562,   277,   278,   279,   280,     0,     0,     0,     0,   107,
     341,     0,     0,     0,     0,     0,     0,     0,   281,     0,
     282,     0,   283,     0,   218,   219,   220,   221,   222, -1059,
   -1059, -1059, -1059,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,     0,     0,   284,   879,
     285,     0,    93,    94,     0,    95,   184,    97,     0,  1120,
       0,     0,   286,   287,   288,   289,   290,   291,   292,     0,
       0,     0,   214,     0,     0,     0,     0,     0,     0,     0,
     107,     0,     0,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,    50,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,     0,   327,
       0,     0,   329,   330,   331,     0,     0,     0,   332,   333,
     218,   219,   220,   221,   222,   334,     0,     0,     0,     0,
       0,     0,     0,   214,     0,     0,     0,     0,     0,     0,
       0,     0,   335,     0,     0,    91,   336,     0,    93,    94,
       0,    95,   184,    97,   337,    50,   338,     0,     0,   339,
    1659,  1660,  1661,  1662,  1663,     0,   340,  1664,  1665,  1666,
    1667,     0,     0,     0,     0,     0,   107,   341,     0,     0,
       0,     0,     0,     0,  1668,  1669,  1670,     0,     0,     0,
       0,   218,   219,   220,   221,   222,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   183,  1671,     0,    91,    92,     0,    93,
      94,     0,    95,   184,    97,     0,     0,     0,  1672,  1673,
    1674,  1675,  1676,  1677,  1678,     0,     0,     0,   214,     0,
       0,     0,     0,     0,     0,     0,     0,   107,     0,  1679,
    1680,  1681,  1682,  1683,  1684,  1685,  1686,  1687,  1688,  1689,
      50,  1690,  1691,  1692,  1693,  1694,  1695,  1696,  1697,  1698,
    1699,  1700,  1701,  1702,  1703,  1704,  1705,  1706,  1707,  1708,
    1709,  1710,  1711,  1712,  1713,  1714,  1715,  1716,  1717,  1718,
    1719,     0,     0,     0,  1720,  1721,   218,   219,   220,   221,
     222,     0,  1722,  1723,  1724,  1725,  1726,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1727,  1728,
    1729,     0,     0,     0,    93,    94,     0,    95,   184,    97,
    1730,     0,  1731,  1732,     0,  1733,     0,     0,     0,     0,
       0,     0,  1734,  1735,     0,  1736,     0,  1737,  1738,     0,
     273,   274,   107,   275,   276,  1095,  1096,   277,   278,   279,
     280,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   281,  1097,   282,     0,  1098,  1099,
    1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
       0,     0,     0,     0,   284,     0,     0,     0,     0,     0,
       0,     0,     0,  1120,     0,     0,     0,     0,   286,   287,
     288,   289,   290,   291,   292,     0,     0,     0,   214,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
      50,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,     0,   327,     0,   328,   329,   330,
     331,     0,     0,     0,   332,   580,   218,   219,   220,   221,
     222,   581,     0,     0,     0,     0,     0,   273,   274,     0,
     275,   276,     0,     0,   277,   278,   279,   280,   582,     0,
       0,     0,     0,     0,    93,    94,     0,    95,   184,    97,
     337,   281,   338,   282,     0,   339,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   107,     0,     0,     0,     0,     0,     0,     0,
       0,   284,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   286,   287,   288,   289,   290,
     291,   292,     0,     0,     0,   214,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,    50,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,     0,   327,     0,  1288,   329,   330,   331,     0,     0,
       0,   332,   580,   218,   219,   220,   221,   222,   581,     0,
       0,     0,     0,     0,   273,   274,     0,   275,   276,     0,
       0,   277,   278,   279,   280,   582,     0,     0,     0,     0,
       0,    93,    94,     0,    95,   184,    97,   337,   281,   338,
     282,     0,   339,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   107,
       0,     0,     0,     0,     0,     0,     0,     0,   284,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   286,   287,   288,   289,   290,   291,   292,     0,
       0,     0,   214,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,    50,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,     0,   327,
       0,     0,   329,   330,   331,     0,     0,     0,   332,   580,
     218,   219,   220,   221,   222,   581,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   582,     0,     0,     0,     0,     0,    93,    94,
       0,    95,   184,    97,   337,     0,   338,     0,     0,   339,
     451,   452,   453,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   107,     0,     0,     0,
     454,   455,     0,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,     0,   479,   451,   452,
     453,     0,     0,     0,     0,     0,     0,     0,     0,   480,
       0,     0,     0,     0,     0,     0,     0,     0,   454,   455,
       0,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,     0,   479,   451,   452,   453,     0,
       0,     0,     0,     0,     0,     0,     0,   480,     0,     0,
       0,     0,     0,     0,     0,     0,   454,   455,     0,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,     0,   479,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   480,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   451,   452,   453,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   454,   455,   566,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,     0,   479,   451,   452,   453,     0,     0,     0,
       0,     0,     0,     0,     0,   480,     0,     0,     0,     0,
       0,     0,     0,     0,   454,   455,   568,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,   451,   452,   453,     0,     0,     0,     0,     0,
       0,     0,     0,   480,     0,     0,     0,     0,     0,     0,
     283,     0,   454,   455,   587,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   285,   479,
       0,     0,     0,     0,     0,     0,     0,   214,     0,     0,
       0,   480,     0,     0,     0,     0,     0,     0,     0,     0,
     214,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,   283,   357,   358,     0,
       0,     0,    50,     0,   591,     0,     0,     0,     0,     0,
    -396,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   179,   180,   439,   285,   218,   219,   220,   221,   222,
       0,     0,     0,     0,     0,     0,     0,   569,   218,   219,
     220,   221,   222,   570,     0,     0,   214,     0,     0,     0,
     359,   788,     0,    93,    94,     0,    95,   184,    97,     0,
     183,     0,     0,    91,   336,     0,    93,    94,    50,    95,
     184,    97,     0,     0,     0,     0,   576,     0,     0,     0,
       0,   107,     0,     0,   340,     0,     0,     0,     0,     0,
     440,     0,     0,  1296,   107,   341,     0,     0,     0,   811,
       0,     0,     0,   569,   218,   219,   220,   221,   222,   570,
       0,   841,   842,     0,     0,     0,     0,   843,     0,   844,
       0,     0,     0,     0,     0,     0,   183,     0,     0,    91,
     336,   845,    93,    94,     0,    95,   184,    97,     0,    34,
      35,    36,   214,     0,     0,     0,   451,   452,   453,     0,
     340,     0,   216,     0,     0,     0,     0,     0,     0,     0,
     107,   341,     0,     0,    50,     0,   454,   455,     0,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,     0,   479,     0,     0,     0,     0,     0,   846,
     847,   848,   849,   850,   851,   480,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,   223,  1050,     0,
       0,     0,   183,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   184,    97,     0,     0,     0,    99,     0,     0,
       0,     0,     0,     0,     0,     0,   852,     0,     0,     0,
      29,   104,     0,     0,     0,     0,   107,   853,    34,    35,
      36,   214,     0,   215,    40,     0,     0,     0,     0,     0,
       0,   216,   527,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   217,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1051,    75,   218,
     219,   220,   221,   222,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   223,     0,     0,     0,
       0,   183,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   184,    97,     0,   841,   842,    99,     0,     0,     0,
     843,     0,   844,     0,     0,     0,     0,     0,     0,     0,
     104,     0,     0,     0,   845,   107,   224,     0,     0,     0,
       0,   111,    34,    35,    36,   214,     0,     0,     0,   451,
     452,   453,     0,     0,     0,   216,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,   454,
     455,     0,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,     0,   479,     0,     0,     0,
       0,     0,   846,   847,   848,   849,   850,   851,   480,    81,
      82,    83,    84,    85,     0,     0,     0,  1004,  1005,     0,
     223,     0,     0,     0,     0,   183,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   184,    97,  1006,     0,     0,
      99,     0,     0,     0,     0,  1007,  1008,  1009,   214,   852,
       0,     0,     0,     0,   104,     0,     0,     0,  1010,   107,
     853,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,   536,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    29,     0,  1011,  1012,  1013,  1014,  1015,
    1016,    34,    35,    36,   214,     0,   215,    40,     0,     0,
       0,     0,     0,  1017,   216,     0,     0,     0,   183,     0,
       0,    91,    92,     0,    93,    94,    50,    95,   184,    97,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1018,     0,     0,     0,     0,   217,     0,     0,
       0,     0,   107,     0,     0,     0,     0,     0,     0,     0,
       0,    75,   218,   219,   220,   221,   222,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,   223,
       0,     0,     0,     0,   183,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   184,    97,    29,     0,     0,    99,
       0,     0,     0,     0,    34,    35,    36,   214,     0,   215,
      40,     0,     0,   104,     0,     0,     0,   216,   107,   224,
       0,     0,   603,     0,   111,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     217,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   623,    75,   218,   219,   220,   221,   222,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,   223,     0,     0,     0,     0,   183,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   184,    97,    29,
       0,   993,    99,     0,     0,     0,     0,    34,    35,    36,
     214,     0,   215,    40,     0,     0,   104,     0,     0,     0,
     216,   107,   224,     0,     0,     0,     0,   111,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   217,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,   218,   219,
     220,   221,   222,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   223,     0,     0,     0,     0,
     183,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     184,    97,    29,     0,     0,    99,     0,     0,     0,     0,
      34,    35,    36,   214,     0,   215,    40,     0,     0,   104,
       0,     0,     0,   216,   107,   224,     0,     0,     0,     0,
     111,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   217,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1148,
      75,   218,   219,   220,   221,   222,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   223,     0,
       0,     0,     0,   183,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   184,    97,    29,     0,     0,    99,     0,
       0,     0,     0,    34,    35,    36,   214,     0,   215,    40,
       0,     0,   104,     0,     0,     0,   216,   107,   224,     0,
       0,     0,     0,   111,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   217,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,   218,   219,   220,   221,   222,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,   223,     0,     0,     0,     0,   183,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   184,    97,     0,     0,
       0,    99,     0,     0,   451,   452,   453,     0,     0,     0,
       0,     0,     0,     0,     0,   104,     0,     0,     0,     0,
     107,   224,     0,     0,   454,   455,   111,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,   451,   452,   453,     0,     0,     0,     0,     0,
       0,     0,     0,   480,     0,     0,     0,     0,     0,     0,
       0,     0,   454,   455,     0,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,     0,   479,
       0,     0,     0,     0,     0,     0,     0,     0,   451,   452,
     453,   480,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   454,   455,
     911,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,     0,   479,   451,   452,   453,     0,
       0,     0,     0,     0,     0,     0,     0,   480,     0,     0,
       0,     0,     0,     0,     0,     0,   454,   455,   979,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,     0,   479,     0,     0,     0,     0,     0,     0,
       0,     0,   451,   452,   453,   480,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   454,   455,  1035,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,     0,   479,
    1094,  1095,  1096,     0,     0,     0,     0,     0,     0,     0,
       0,   480,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1097,  1341,     0,  1098,  1099,  1100,  1101,  1102,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1094,  1095,  1096,     0,  1120,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1097,     0,  1372,  1098,
    1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,     0,     0,  1094,  1095,  1096,     0,     0,     0,     0,
       0,     0,     0,     0,  1120,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1097,     0,  1451,  1098,  1099,  1100,
    1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1094,  1095,
    1096,     0,  1120,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1097,
       0,  1548,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,     0,    34,    35,    36,   214,     0,
     215,    40,     0,     0,     0,     0,     0,  1120,   216,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1640,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   238,     0,     0,     0,     0,     0,   239,     0,     0,
       0,     0,     0,     0,     0,     0,   218,   219,   220,   221,
     222,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,   223,  1642,     0,     0,     0,   183,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   184,    97,
       0,     0,     0,    99,     0,    34,    35,    36,   214,     0,
     215,    40,     0,     0,     0,     0,     0,   104,   654,     0,
       0,     0,   107,   240,     0,     0,     0,     0,   111,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   217,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   218,   219,   220,   221,
     222,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,   223,     0,     0,     0,     0,   183,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   184,    97,
       0,     0,     0,    99,     0,    34,    35,    36,   214,     0,
     215,    40,     0,     0,     0,     0,     0,   104,   216,   214,
       0,     0,   107,   655,     0,     0,     0,     0,   656,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,   877,
     878,   238,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   218,   219,   220,   221,
     222,     0,    81,    82,    83,    84,    85,   218,   219,   220,
     221,   222,     0,   223,     0,     0,     0,     0,   183,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   184,    97,
       0,     0,   879,    99,     0,    93,    94,     0,    95,   184,
      97,     0,     0,     0,   451,   452,   453,   104,     0,     0,
       0,     0,   107,   240,     0,     0,     0,     0,   111,     0,
       0,     0,     0,   107,   454,   455,   976,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,   451,   452,   453,     0,     0,     0,     0,     0,
       0,     0,     0,   480,     0,     0,     0,     0,     0,     0,
       0,     0,   454,   455,     0,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,     0,   479,
    1094,  1095,  1096,     0,     0,     0,     0,     0,     0,     0,
       0,   480,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1097,  1456,     0,  1098,  1099,  1100,  1101,  1102,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1094,  1095,  1096,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1120,
       0,     0,     0,     0,     0,     0,     0,  1097,     0,     0,
    1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,   453,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1120,     0,     0,     0,     0,
     454,   455,     0,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,  1096,   479,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   480,
       0,     0,     0,     0,     0,  1097,     0,     0,  1098,  1099,
    1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   454,   455,  1120,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,     0,   479,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     480
};

static const yytype_int16 yycheck[] =
{
       5,     6,   129,     8,     9,    10,    11,    12,    13,    56,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   161,   187,    29,    30,  1172,   108,     4,    56,
     672,     4,    98,    33,   405,     4,   102,   103,   552,    44,
      31,   405,   405,     4,   669,   540,    46,    52,    57,    54,
     166,    51,    57,   952,    59,   521,   698,    60,  1333,   515,
     516,   188,   128,    56,    27,    28,    31,   108,   668,   752,
     597,   598,    31,   479,   588,   161,   511,   511,   941,   822,
     241,    86,   647,   829,  1168,    88,   972,  1049,    91,  1159,
     546,   108,   108,   252,   357,   358,     9,    57,    32,    49,
      32,    14,   988,   108,   800,    49,     9,     9,    49,     9,
      49,     4,     9,   548,   548,     9,    14,    14,     9,    49,
       9,     9,     9,   106,   107,     4,    14,    31,     9,    50,
      51,     4,     9,    36,    83,     9,    83,   253,    83,    38,
      54,  1773,    70,     9,   185,     9,    38,    90,    70,     9,
    1036,     4,     9,     9,     9,    38,   670,     9,   115,     9,
     483,     9,   102,    90,     9,     9,    83,    38,   185,   185,
      49,    81,   160,   160,     9,     9,   164,   181,   196,   160,
     185,   160,   181,   224,    83,    70,   181,   192,     0,   512,
      70,    83,   196,   181,   517,    44,    32,   196,  1830,   240,
      83,   196,   181,    70,   102,   103,   196,   224,   224,   196,
       4,    70,    83,   160,   157,    83,    70,   200,   199,   224,
     130,   178,    38,   240,   164,   174,   196,  1090,    70,   174,
     157,   390,   160,   112,    38,   240,     8,    86,   117,   881,
     119,   120,   121,   122,   123,   124,   125,   197,    70,   254,
     175,   165,   257,   197,   193,    49,   197,   174,   197,   264,
     265,   182,    70,   193,    70,   199,   198,    83,   161,    70,
     233,   199,    70,    70,   196,   174,   129,   441,    54,    83,
     134,   135,   258,  1367,  1559,   198,   262,  1249,   167,   168,
     283,   170,   285,   181,   197,   991,   198,   199,   198,  1369,
     814,   198,   349,   174,   198,   819,  1376,   198,  1378,   198,
     198,   198,   191,  1059,   199,  1061,   197,  1203,   112,   199,
     197,   200,   349,   117,   198,   119,   120,   121,   122,   123,
     124,   125,   198,   201,   198,   188,   377,  1407,   198,    83,
     199,   198,   198,   198,   969,   199,   198,   197,   341,   197,
     877,   878,   197,   197,   357,   358,   359,   199,   174,   523,
     377,   377,   197,   197,  1227,   930,   493,   166,    70,   435,
     174,   376,   377,   167,   168,    70,   170,   199,   383,   384,
     385,   386,   387,   388,   199,   432,    70,   196,   393,   165,
     196,   199,    70,   199,   397,   196,   181,   191,   199,   196,
     199,   199,   199,   366,   180,   432,   200,   412,   122,    57,
     196,   196,   375,   494,   377,   420,   130,    70,  1502,   382,
    1504,    69,   488,   489,   490,   491,   431,    70,   196,    70,
     174,   394,    83,    84,  1467,   386,   429,   388,   485,   432,
    1087,   907,  1089,  1513,   134,   135,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,  1157,   480,   160,   482,   483,   484,
     405,    50,    51,    83,    84,   494,   196,   534,   479,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,  1245,    83,   199,     4,   196,   512,   513,   196,
     515,   516,   517,   518,   479,   199,   675,   160,   677,   524,
     479,   199,   527,   958,   958,   196,   419,   376,  1612,   985,
     102,   536,  1616,   538,   494,  1568,   385,   386,   411,   388,
      83,   546,  1056,   196,   197,   554,   160,   686,   236,   554,
     201,   556,   160,   196,    53,   196,   199,    56,   521,  1592,
     485,  1594,  1187,    70,   102,  1190,   419,   106,   107,   123,
     941,  1171,   102,   181,    73,   479,   130,   941,   941,   196,
     573,   485,   196,   559,   745,  1448,   511,   993,    83,   166,
     754,   199,   164,   196,   597,   598,    70,    96,   603,    98,
     686,   201,   179,   102,   103,    70,  1133,   511,   871,   534,
     873,    83,   192,   196,   655,   158,   159,   778,    90,   199,
     545,    70,   199,   548,    83,   181,   164,   134,   135,   128,
     534,    90,   790,     4,   164,   794,   795,    14,   704,   492,
     196,   545,   801,   802,   548,    83,    78,    79,    80,  1222,
     655,  1224,    90,   160,    70,    32,    27,    28,     4,    91,
      83,   200,   157,   158,   159,  1940,    70,    90,  1531,   134,
     135,    70,   665,   666,    51,   165,   790,    83,   196,    31,
     199,   674,   134,   135,    90,   801,   158,   159,  1772,   196,
      31,   696,  1776,   854,  1377,    53,    54,    55,    50,   158,
     159,    53,   201,    49,   709,   866,   164,   164,   196,    50,
    1205,    69,    53,   145,   146,   147,   148,   149,   160,  1090,
     158,   159,    83,   198,   156,    38,  1090,  1090,   198,    90,
     162,   163,   695,   205,   157,   158,   159,   108,   743,   181,
    1196,   198,   199,   198,   176,   204,  1371,   198,   200,   134,
     135,  1207,   158,   159,   196,    83,  1356,   199,   190,   258,
      75,    76,    90,   262,    75,    76,   112,   266,   773,   102,
     103,   117,  1918,   119,   120,   121,   122,   123,   124,   125,
     198,   199,  1248,   105,   106,   107,  1932,    53,    54,    55,
     198,    57,   755,   686,   132,   133,   669,   158,   159,  1482,
     198,   806,   198,    69,   391,   105,   106,   107,   395,  1323,
     119,   120,   121,    70,   185,   198,   199,   822,   818,  1804,
    1805,   167,   168,   198,   170,   112,   113,   114,  1912,   157,
     158,   159,    70,   796,   421,   798,   423,   424,   425,   426,
     357,   358,   530,  1927,    70,   191,   839,   840,  1006,    70,
     349,   827,    70,   224,   200,    70,  1227,  1800,  1801,   839,
     840,   824,   233,  1227,  1227,    70,   160,   199,   871,   240,
     873,  1337,   111,   196,   877,   878,   879,   196,    70,   160,
     119,   120,   121,   122,   123,   124,  1569,   258,   196,  1514,
     164,   262,  1006,   198,    48,  1409,   181,   944,    69,   160,
     119,   120,   121,   122,   123,   124,   911,   196,   913,     9,
     915,  1425,   196,   203,   160,   160,   196,  1373,     8,   812,
     419,   926,   198,   196,   160,    14,   992,   160,   891,   428,
     198,   198,     9,   432,   199,   940,   435,   130,   931,   198,
     130,   629,   630,    14,   907,   908,   119,   120,   121,   122,
     123,   124,   191,   929,   947,   197,   929,   130,   131,   812,
     929,   966,    14,   102,   181,   197,   197,   960,   929,   197,
     658,   976,   191,   197,   979,   868,   981,   202,   111,   198,
     985,     9,   196,   196,   196,  1884,   485,   486,   487,   488,
     489,   490,   491,   157,   197,   366,   989,   197,   197,   197,
     173,    94,   993,     9,   375,  1904,   377,   198,   196,    14,
       9,   382,   511,   181,  1913,   868,   941,  1178,   191,   944,
    1534,   196,   199,   394,   198,    83,   132,   199,   993,  1543,
    1035,   956,   198,   958,   993,   534,   929,   197,   197,   197,
     944,   198,  1042,  1557,  1500,  1092,   196,   203,   419,   548,
     197,     9,   956,   741,   958,     9,   929,   950,   203,   203,
     559,  1219,  1220,  1221,  1222,  1223,  1224,  1043,   203,  1150,
    1063,  1229,   203,    70,  1067,    32,   929,  1448,   133,   952,
     579,   180,   160,   136,  1448,  1448,  1049,  1050,     9,   993,
     197,   160,    14,     9,     9,   193,   969,   950,   182,     9,
     197,    14,   601,   602,   200,  1219,  1220,   132,   203,  1223,
       9,   203,  1037,    14,   197,  1229,   203,   197,   203,  1633,
     196,   119,   120,   121,   122,   123,   124,   160,   102,   197,
    1133,   198,   198,  1037,     9,   136,   160,   636,   637,     9,
     197,  1150,   196,   831,    70,  1150,    70,    70,   196,   837,
     521,     9,  1045,    70,  1047,   199,    70,   200,    14,   198,
    1531,   182,    81,     9,   199,  1090,    14,  1531,  1531,   203,
      50,    51,    52,    53,    54,    55,   199,  1170,  1183,  1172,
    1156,    14,  1824,  1156,   103,   197,   193,  1156,   559,    69,
     198,  1196,  1045,   191,  1047,  1156,    32,    32,    14,    14,
     196,   196,  1207,  1208,   196,   704,  1199,   196,    52,  1202,
     196,    70,   196,    70,   160,   903,    70,    70,    70,  1182,
     139,   140,   141,   142,   143,     9,   197,  1385,   198,  1387,
     198,   196,   136,    14,   182,   160,   136,  1237,     9,    69,
    1245,   197,   161,   203,     9,   164,    83,   200,   167,   168,
    1255,   170,   171,   172,   198,   200,  1770,     9,   196,   136,
    1902,   196,   198,  1156,  1240,    14,  1780,   197,  1261,    83,
    1233,  1385,   199,  1387,  1267,   136,   195,   197,   196,     9,
      91,   200,    32,  1156,   196,  1248,  1249,    77,   199,   157,
     199,   790,   198,   792,   199,   203,  1938,   182,   136,    32,
     198,   197,  1227,  1156,   198,   197,   197,     9,   203,     9,
     203,  1825,   136,   812,  1187,   203,   203,  1190,   119,   120,
     121,   122,   123,   124,   695,   203,     9,   826,   827,   130,
     131,   197,     9,   200,   197,   200,  1341,  1330,  1331,   199,
      14,    83,  1030,  1348,  1483,   196,     9,  1352,   198,  1354,
     197,  1478,     9,   198,   197,   197,  1870,  1362,     4,  1252,
     199,   136,  1520,   197,  1522,   196,  1524,  1372,  1373,   868,
     171,  1529,   173,   198,  1337,   197,   875,   876,   136,   197,
     203,     9,   203,   203,   755,   186,   203,   188,  1076,    32,
     191,   203,   198,   197,   197,  1083,   198,   136,   198,  1252,
     199,   112,   169,    49,   198,   904,  1520,    14,  1522,   165,
    1524,    83,   117,   199,   197,  1529,   136,    50,    51,    52,
      53,    54,    55,  1937,    57,   796,  1590,   798,  1942,   197,
     929,   197,   136,    14,   181,   199,    69,  1430,   198,  1432,
      14,   812,  1335,    83,    14,   944,    83,   136,   136,   197,
     196,   950,  1345,   824,   197,    14,   827,   956,   198,   958,
     198,    14,  1335,   198,    14,  1623,   112,    27,    28,     9,
     199,   117,  1345,   119,   120,   121,   122,   123,   124,   125,
       9,    81,   200,    59,    83,  1478,   181,   196,    83,     9,
     115,   199,  1497,   992,   102,  1500,  1562,   868,  1371,   198,
     160,   102,   182,   103,   172,  1004,  1005,  1006,    36,  1623,
      14,   196,   198,   197,   196,   182,   178,   182,  1565,    83,
     891,   167,   168,  1448,   170,   175,   197,     9,   128,  1217,
     199,    83,   198,   197,   195,    14,   907,   908,  1037,   139,
     140,   141,   142,   143,  1043,   191,  1045,    83,  1047,   197,
       9,    14,  1545,    83,   200,  1133,    14,    83,   929,    14,
      83,  1893,   491,   932,  1540,  1458,   986,   167,   168,  1068,
     170,   171,   172,  1909,  1246,  1632,  1422,  1904,   605,   950,
    1619,   488,  1657,   486,  1570,  1742,  1925,  1473,  1276,  1949,
    1483,  1754,  1280,  1092,  1758,   195,   196,  1285,   387,  1469,
    1615,  1088,  1607,  1223,  1292,  1458,  1531,  1160,  1084,  1218,
    1503,  1005,  1219,  1032,   956,  1936,  1509,   839,  1511,   432,
    1778,  1779,  1121,  1951,  1477,  1859,   383,  1141,  1461,  1069,
    1503,    -1,  1121,    -1,    -1,    -1,  1509,    -1,  1511,    -1,
    1533,  1514,  1535,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,  1544,    -1,    -1,    -1,  1631,  1632,  1156,    -1,    -1,
    1533,    -1,  1655,    -1,  1778,  1779,    -1,    -1,    -1,    -1,
      -1,    -1,  1043,   233,  1045,    -1,  1047,    -1,  1049,  1050,
      -1,    -1,  1535,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,  1544,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1392,    -1,  1753,    -1,  1396,    -1,
      -1,    -1,    -1,    -1,    -1,  1403,    -1,    -1,    -1,    -1,
    1219,  1220,  1221,  1222,  1223,  1224,  1753,    -1,  1621,    -1,
    1229,    -1,    -1,    -1,    81,    -1,    -1,  1630,  1877,    -1,
      -1,  1240,    -1,  1636,    -1,  1598,    -1,    -1,  1621,    -1,
    1643,    -1,  1818,  1252,    -1,    -1,   103,    -1,  1763,    -1,
      -1,    -1,    -1,  1262,  1757,    -1,    -1,    -1,    -1,    -1,
    1897,    -1,    -1,    -1,    -1,    -1,    -1,  1630,   125,    -1,
      -1,    -1,    -1,  1636,    -1,  1156,    -1,    -1,    -1,    -1,
    1643,   138,   139,   140,   141,   142,   143,   144,    -1,     4,
      -1,    -1,    -1,    27,    28,    -1,   366,    31,    -1,    -1,
      -1,  1182,    -1,    -1,   161,   375,    -1,   164,   165,    -1,
     167,   168,   382,   170,   171,   172,    -1,    56,    -1,    -1,
    1329,    -1,    56,    -1,   394,    -1,    -1,    81,    -1,    83,
      84,    -1,    -1,    -1,    49,    -1,    -1,    -1,   195,  1842,
      -1,    -1,   233,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,  1233,    -1,    -1,    -1,    -1,    -1,    -1,  1240,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1248,  1249,    -1,
      -1,  1252,    -1,    -1,    -1,    -1,  1385,    -1,  1387,    -1,
      -1,  1784,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,
      -1,    -1,   117,    -1,   119,   120,   121,   122,   123,   124,
     125,   165,    -1,   167,   168,  1918,   170,   171,   172,    -1,
      -1,  1784,    -1,  1826,    -1,    -1,    -1,  1930,    -1,  1932,
    1833,    -1,  1947,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   195,    -1,    -1,  1959,   199,    -1,   201,  1951,  1458,
    1953,   521,   167,   168,  1969,   170,  1337,  1972,  1467,    -1,
      -1,    -1,    -1,  1826,  1473,  1868,    -1,    -1,    -1,    -1,
    1833,    -1,    -1,    -1,  1877,   366,   191,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   375,   200,    -1,  1890,    -1,    -1,
      -1,   382,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   233,
      -1,  1884,    -1,   394,    -1,  1868,    -1,    -1,    -1,    -1,
      -1,  1520,    -1,  1522,   405,  1524,    -1,    -1,    -1,    -1,
    1529,  1904,    -1,    -1,    -1,    -1,  1535,  1890,    -1,    -1,
    1913,  1540,    -1,    -1,    -1,  1544,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   283,    -1,   285,  1950,    -1,   283,
      -1,   285,  1955,  1562,    -1,    -1,  1565,    -1,    -1,  1568,
      -1,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1578,
      -1,    -1,    -1,    -1,    -1,    -1,  1585,  1458,    -1,    -1,
      -1,    -1,    -1,  1592,    -1,  1594,    -1,  1950,    -1,    -1,
      -1,  1600,  1955,    -1,    -1,    -1,  1794,  1795,    -1,    -1,
      -1,    -1,   341,    -1,    -1,    -1,    49,   341,    -1,    -1,
      -1,    -1,    -1,    -1,  1623,    -1,    -1,    -1,    -1,    -1,
      -1,  1630,  1631,  1632,    -1,   695,    -1,  1636,    -1,    -1,
     521,    -1,   366,    -1,  1643,    -1,    -1,    -1,    -1,    -1,
      -1,   375,    -1,    -1,    -1,    -1,    -1,    -1,   382,    -1,
      -1,    -1,    -1,    -1,  1535,    -1,    -1,    -1,    -1,  1540,
     394,    -1,    -1,  1544,    -1,    -1,    -1,    -1,    -1,   112,
      -1,   405,    -1,    -1,   117,    -1,   119,   120,   121,   122,
     123,   124,   125,    -1,    -1,   755,    -1,    -1,    -1,    -1,
     429,    -1,    -1,   432,    -1,   429,    -1,    -1,   432,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    -1,   167,   168,   796,   170,   798,    -1,
      -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,  1753,   479,    -1,    -1,   191,  1630,
    1631,  1632,    67,    68,   824,  1636,    -1,   200,    -1,    -1,
      -1,    -1,  1643,    -1,    -1,  1774,    -1,    -1,    -1,  1778,
    1779,    81,    -1,    83,    84,  1784,    -1,    49,    -1,   139,
     140,   141,   142,   143,  1793,    -1,    -1,   521,    -1,    -1,
      -1,  1800,  1801,   103,    -1,  1804,  1805,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   695,    -1,    -1,   167,   168,  1818,
     170,   171,   172,    -1,    -1,    -1,    -1,  1826,    -1,   134,
     135,   891,    -1,    -1,  1833,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   573,   195,   575,   907,   908,   573,
     112,   575,    -1,    -1,   578,   117,    -1,   119,   120,   121,
     122,   123,   124,   125,    -1,   165,    -1,   167,   168,  1868,
     170,   171,   172,    -1,   755,    -1,    -1,  1876,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   609,    -1,    -1,    -1,    -1,
      -1,  1890,   197,    -1,    -1,   195,    -1,  1896,    -1,   199,
      -1,   201,    -1,    -1,    -1,   167,   168,    -1,   170,    -1,
      -1,    -1,    -1,  1784,    -1,   796,    -1,   798,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   191,
      -1,    -1,    -1,    -1,    -1,    -1,   665,   666,   200,    -1,
      -1,   665,   666,   824,    -1,   674,    -1,    -1,    -1,    -1,
     674,  1950,    -1,    -1,    -1,  1826,  1955,    -1,    -1,    -1,
      -1,    -1,  1833,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   695,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,  1049,
    1050,    -1,    -1,    -1,    -1,    -1,    -1,  1868,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     891,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,  1890,
      -1,    -1,    -1,    -1,    -1,    -1,   907,   908,    67,    68,
      -1,   755,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
     941,    -1,    -1,    -1,    -1,    -1,   790,    -1,    -1,    -1,
      -1,    69,   796,    -1,   798,    -1,    -1,    -1,    -1,  1950,
      -1,    -1,    -1,    -1,  1955,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   134,   135,    -1,    -1,    -1,
     824,   825,    31,   832,    -1,    -1,    -1,    -1,   832,    -1,
     839,   840,    -1,    -1,    -1,   839,   840,   841,   842,   843,
     844,   845,  1182,    -1,    -1,    -1,    -1,    -1,    -1,   853,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,   869,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,   197,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,   891,  1049,  1050,
      -1,    -1,    -1,  1233,   103,    -1,    -1,    -1,    -1,    -1,
      -1,   905,    -1,   907,   908,    67,    68,    -1,  1248,  1249,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   931,    -1,    -1,   203,    -1,   931,   932,  1090,
     139,   140,   141,   142,   143,    -1,    -1,   941,   947,    -1,
      -1,    27,    28,   947,    -1,    31,    -1,    -1,    -1,    -1,
      -1,   960,   161,    -1,    -1,   164,   960,    -1,   167,   168,
      -1,   170,   171,   172,   968,   174,    -1,   971,    -1,    -1,
      -1,    -1,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,
     989,    -1,    -1,    -1,    -1,   989,   195,    -1,    -1,   993,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1337,    10,    11,
      12,    -1,  1006,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,  1182,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,  1049,  1050,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1063,    -1,    -1,    69,  1067,  1063,
    1069,    -1,    -1,  1067,    -1,  1069,  1227,    -1,    -1,    -1,
      -1,    -1,  1233,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1248,  1249,  1093,
    1094,  1095,  1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,  1137,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,   233,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,  1170,    57,  1172,    -1,    -1,  1170,    -1,  1172,    -1,
      67,    68,    -1,    -1,    69,    -1,  1337,    -1,  1182,    -1,
      -1,    -1,    -1,    -1,    -1,   197,    -1,    67,    68,    -1,
    1199,    -1,    -1,  1202,    -1,  1199,    -1,    -1,  1202,    -1,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,  1219,  1220,  1221,  1222,  1223,
    1224,    -1,    -1,  1227,    -1,  1229,    69,    -1,    -1,  1233,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,
      -1,    -1,    -1,    -1,  1248,  1249,    -1,  1251,    -1,    -1,
      -1,    -1,  1261,    -1,   134,   135,    -1,  1261,  1267,    -1,
      -1,    -1,    -1,  1267,    -1,    -1,  1270,    -1,  1272,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     366,    -1,    -1,    -1,    -1,    -1,    -1,  1448,    -1,   375,
      -1,    -1,  1296,    -1,    -1,    -1,   382,    -1,    -1,    -1,
     197,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   394,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,   405,
      -1,  1330,  1331,    -1,    -1,    -1,  1330,  1331,    -1,    -1,
    1334,    -1,    -1,  1337,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1531,    -1,    -1,    69,    10,    11,    12,    -1,    -1,    -1,
      -1,  1385,    -1,  1387,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   479,    -1,    31,    -1,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      81,  1430,    -1,  1432,    -1,    -1,  1430,    -1,  1432,    -1,
      -1,    -1,    -1,    69,  1438,   521,  1440,    -1,  1442,    -1,
      31,    -1,   103,  1447,  1448,    -1,    -1,  1451,    -1,  1453,
      -1,    -1,  1456,    -1,    -1,    -1,    -1,    -1,  1467,    -1,
      -1,    -1,    -1,  1467,  1468,    -1,    -1,  1471,    59,  1478,
      -1,    -1,    -1,    -1,  1478,    -1,    -1,    -1,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    -1,   578,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,   164,   200,    -1,   167,   168,    -1,   170,
     171,   172,   103,    -1,    -1,    -1,  1520,    -1,  1522,    -1,
    1524,    -1,    -1,   609,    -1,  1529,    -1,  1531,    -1,    -1,
      -1,    -1,    -1,    -1,   195,    -1,  1545,    -1,   199,    -1,
      81,  1545,    -1,    -1,  1548,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,  1560,  1561,    -1,  1568,
      -1,    -1,   103,    -1,  1568,    -1,  1570,    -1,    -1,    -1,
     161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,  1592,    -1,  1594,    -1,    -1,  1592,    -1,
    1594,  1600,    -1,    -1,   185,    -1,  1600,    -1,   139,   140,
     141,   142,   143,    -1,   195,   196,    -1,    -1,    -1,   695,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1623,
      -1,    -1,    -1,    -1,   165,    -1,   167,   168,   169,   170,
     171,   172,    -1,    -1,    -1,    -1,  1640,  1641,  1642,    -1,
      -1,    -1,    -1,  1647,    -1,  1649,  1655,    10,    11,    12,
      -1,  1655,    -1,  1657,   195,   196,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   755,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    27,    28,
      -1,    -1,    31,    -1,   790,    -1,    69,    -1,    -1,    -1,
     796,    -1,   798,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    31,    57,    -1,    -1,    -1,    -1,   824,   825,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,  1757,    -1,
      -1,    -1,    -1,  1757,    -1,   841,   842,   843,   844,   845,
      59,    -1,    -1,    -1,    -1,  1774,    -1,   853,    -1,    -1,
    1774,    -1,    -1,    -1,  1778,  1779,    -1,    -1,    -1,    -1,
      -1,    -1,    81,   869,  1793,    -1,    -1,    -1,    -1,  1793,
      -1,    -1,    -1,    -1,    -1,  1799,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,   891,  1810,    -1,    -1,    -1,
      -1,    -1,  1816,    -1,    -1,    -1,  1820,    -1,    -1,   905,
      -1,   907,   908,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,  1842,    -1,    -1,    -1,   200,  1842,   138,
     139,   140,   141,   142,   143,   144,   932,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   941,    -1,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,   174,    -1,    -1,  1882,    81,
      -1,    -1,   968,    -1,    -1,   971,   185,    -1,  1892,    91,
      -1,    -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,
      -1,   103,    -1,    -1,   233,  1909,    -1,   993,    -1,  1918,
      -1,    -1,    -1,    -1,  1918,    -1,    -1,    -1,    -1,    -1,
    1006,  1930,    -1,  1932,    -1,    -1,  1930,    -1,  1932,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,  1951,    -1,  1953,    -1,    -1,  1951,    -1,  1953,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,
      -1,    -1,   164,  1049,  1050,   167,   168,    -1,   170,   171,
     172,    -1,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,   103,    -1,  1093,  1094,  1095,
    1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,    -1,    -1,   366,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   375,    -1,    -1,    -1,
      -1,  1137,    -1,   382,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   394,    -1,    -1,    -1,   167,
     168,    -1,   170,   171,   172,    -1,   405,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1182,   195,   196,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    31,  1219,  1220,  1221,  1222,  1223,  1224,    69,
      -1,  1227,    -1,  1229,    -1,    -1,    -1,  1233,    -1,    -1,
     479,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      59,    -1,  1248,  1249,    -1,  1251,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,  1270,    -1,  1272,    -1,    10,    11,
      12,    -1,   521,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    30,    31,
    1296,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,    69,  1334,   578,
      -1,  1337,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,
     200,   170,   171,   172,    -1,   174,    -1,    -1,    -1,    -1,
     609,    -1,    -1,    -1,    -1,    -1,   185,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,   195,   196,    -1,  1385,
      -1,  1387,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     578,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,   609,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,  1438,    -1,  1440,    -1,  1442,    -1,    -1,    -1,
      -1,  1447,  1448,    -1,    -1,  1451,   695,  1453,    -1,    -1,
    1456,    -1,    -1,    -1,    10,    11,    12,    -1,   200,    -1,
      -1,    -1,  1468,    -1,    -1,  1471,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,   755,    -1,    -1,    -1,
      -1,    81,    -1,    69,  1520,    -1,  1522,    -1,  1524,    -1,
      -1,    -1,    -1,  1529,    81,  1531,    83,    -1,    85,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1548,    -1,    -1,    -1,   103,   796,    -1,   798,
      -1,    -1,    -1,    -1,  1560,  1561,    -1,    -1,   200,    -1,
      -1,    -1,    -1,    -1,  1570,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,    -1,   824,   825,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      -1,    -1,   841,   842,   843,   844,   845,   167,   168,    -1,
     170,   171,   172,    -1,   853,    -1,    -1,    -1,    -1,    -1,
     167,   168,    -1,   170,   171,   172,    -1,  1623,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1640,  1641,  1642,   825,   195,    -1,
      -1,  1647,   891,  1649,   200,    -1,    10,    11,    12,    -1,
      -1,  1657,    -1,   841,   842,   843,   844,   845,   907,   908,
      -1,    -1,    -1,    -1,    -1,   853,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   941,    57,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   968,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,   993,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1778,  1779,    -1,    -1,    -1,    -1,    -1,    -1,
     968,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1799,    -1,    -1,    -1,    -1,    -1,    -1,
    1049,  1050,    -1,    -1,  1810,    -1,    -1,    -1,    -1,    -1,
    1816,    -1,    -1,    -1,  1820,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   198,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,    -1,    -1,  1093,  1094,  1095,  1096,  1097,  1098,
    1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,    -1,    -1,    -1,    -1,  1882,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,  1892,    -1,  1137,    -1,
      91,    -1,    -1,    -1,    -1,    -1,  1084,  1085,    -1,    -1,
    1088,    -1,   103,  1909,    -1,  1093,  1094,  1095,  1096,  1097,
    1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1182,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    81,    -1,    -1,  1137,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
     161,    -1,    -1,   164,    -1,    -1,   167,   168,   103,   170,
     171,   172,    -1,    -1,    -1,   103,    -1,    -1,  1227,    -1,
      -1,    -1,    -1,    -1,  1233,    -1,    -1,    -1,    -1,    -1,
     125,    -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,  1248,
    1249,    -1,  1251,   138,   139,   140,   141,   142,   143,   144,
      -1,   139,   140,   141,   142,   143,    81,    -1,    -1,    -1,
      -1,  1270,    -1,  1272,    -1,    -1,   161,    -1,    -1,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,   103,   167,
     168,    -1,   170,   171,   172,     3,     4,  1296,     6,     7,
      -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
     195,    -1,    -1,  1251,    -1,    -1,    -1,   195,    -1,    27,
      -1,    29,    -1,    -1,   139,   140,   141,   142,   143,    -1,
      -1,    -1,  1270,    -1,  1272,    -1,    -1,    -1,  1337,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,
     165,    -1,   167,   168,    -1,   170,   171,   172,  1296,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    83,    84,    -1,    -1,    -1,
     195,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,    -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,  1438,
      -1,  1440,    -1,  1442,    -1,    -1,    -1,    -1,  1447,  1448,
      -1,    -1,  1451,   161,  1453,    -1,    -1,  1456,    -1,   167,
     168,    -1,   170,   171,   172,   173,    -1,   175,    -1,    -1,
     178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,
      -1,   199,    -1,   201,    -1,    -1,    -1,    -1,    -1,    -1,
    1438,    -1,  1440,    -1,  1442,    -1,    -1,    -1,    -1,  1447,
      -1,    -1,    -1,  1451,    -1,  1453,    -1,    -1,  1456,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,  1531,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,  1548,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,     3,     4,     5,     6,     7,
    1548,    -1,    -1,    -1,    -1,    13,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      38,  1640,  1641,  1642,    -1,    -1,    -1,    -1,  1647,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,  1656,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1640,  1641,  1642,   103,    -1,    -1,    -1,  1647,
      -1,    -1,    -1,   111,    -1,    -1,    -1,   200,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
    1799,    -1,    -1,   201,   202,    -1,   204,   205,    -1,    -1,
      -1,  1810,    -1,    -1,    -1,    -1,    -1,  1816,    -1,    -1,
      -1,  1820,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
       3,     4,     5,     6,     7,  1844,    -1,    -1,    -1,    -1,
      13,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1799,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,  1810,    -1,    -1,    -1,    -1,    -1,  1816,    -1,
      -1,    -1,  1820,  1882,    -1,    -1,    49,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,   104,    -1,    -1,  1882,   108,   109,   110,   111,   112,
     113,   114,    -1,   116,   117,   118,   119,   120,   121,   122,
     123,   124,    -1,   126,   127,   128,   129,   130,   131,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
     153,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
     173,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,
      -1,    -1,   185,   186,    -1,   188,    -1,   190,   191,   192,
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
     110,   111,   112,   113,   114,    -1,   116,   117,   118,   119,
     120,   121,   122,   123,   124,    -1,   126,   127,   128,   129,
     130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,   153,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,   173,    -1,    -1,   176,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,   185,   186,    -1,   188,    -1,
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
      -1,   108,   109,   110,   111,   112,   113,   114,    -1,   116,
     117,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,   153,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,   173,    -1,    -1,   176,
      -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,   186,
      -1,   188,    -1,   190,   191,   192,    -1,    -1,   195,   196,
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
      -1,   195,   196,    -1,   198,   199,   200,   201,   202,    -1,
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
      -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,   114,
      -1,   116,    -1,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,   153,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,   173,    -1,
      -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
     185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,
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
      92,    93,    94,    95,    96,    -1,    98,    -1,   100,    -1,
      -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
      -1,   113,   114,    -1,   116,    -1,   118,   119,   120,   121,
     122,   123,   124,    -1,   126,   127,   128,    -1,   130,   131,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,   153,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,   173,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,
      -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,
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
      -1,   100,   101,    -1,   103,   104,    -1,    -1,    -1,   108,
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
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    99,
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
      -1,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
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
      94,    -1,    96,    97,    98,    -1,   100,    -1,    -1,   103,
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
      -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,   114,
      -1,   116,    -1,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,   153,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,   173,    -1,
      -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
     185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,
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
     199,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    32,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
     153,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,
      -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,
      -1,    -1,   195,   196,    -1,   198,   199,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
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
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,   153,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,   198,   199,
      -1,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,
      -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,    -1,   113,   114,    -1,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,   153,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,
      -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,
      -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,
      -1,   198,   199,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,
      94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,
     114,    -1,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,   153,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
      -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,
      -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
     198,    -1,    -1,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    31,    -1,    13,    34,
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
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,   174,
      -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
     185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,
     195,   196,    -1,    -1,    -1,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
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
      -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,
     192,    -1,    -1,   195,   196,    -1,    -1,   199,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
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
      -1,   170,   171,   172,    -1,   174,    -1,   176,    -1,    -1,
     179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,    -1,
      -1,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     103,    -1,    -1,    -1,    -1,   108,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,
      -1,   200,   185,    -1,    -1,    -1,    -1,   190,   191,   192,
      -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
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
     190,   191,   192,    -1,    -1,   195,   196,    10,    11,    12,
      -1,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
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
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,
      -1,    -1,   179,    -1,    -1,    -1,    -1,   200,   185,    -1,
      -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,
      -1,   198,    11,    12,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      69,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
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
      -1,   195,   196,    -1,   198,    -1,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,
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
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      -1,   179,    -1,    -1,    -1,    -1,   200,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,   197,
      -1,    -1,    -1,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    32,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
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
     195,   196,    -1,    -1,    -1,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
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
      -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,
     192,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
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
      -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,    -1,
      -1,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
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
     196,    -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,
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
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,
      -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,
      -1,    -1,   195,   196,    10,    11,    12,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
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
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,
      -1,    -1,   198,    -1,    -1,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    10,    11,    12,
      -1,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    28,    13,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    69,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,   102,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,     3,     4,   176,
       6,     7,   179,    -1,    10,    11,    12,    13,   185,    -1,
      -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,
      -1,    27,    -1,    29,   201,   202,    -1,   204,   205,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    81,   128,    -1,   130,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,    -1,   161,    10,    11,    12,    13,
      -1,   167,   168,    -1,   170,   171,   172,   173,    -1,   175,
      -1,    -1,   178,    27,    -1,    29,    -1,    31,    -1,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,   195,
      -1,    -1,    -1,   199,    -1,   201,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    57,   164,    59,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,    -1,    -1,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,   173,
      -1,   175,    -1,    -1,   178,     3,     4,    -1,     6,     7,
      -1,   185,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   195,   196,    -1,    -1,    -1,   200,    -1,    -1,    27,
      -1,    29,    31,    31,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    57,
      -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   161,    -1,    -1,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,   173,    -1,   175,    -1,    -1,
     178,     3,     4,    -1,     6,     7,    -1,   185,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,   195,   196,    -1,
      -1,    -1,   200,    -1,    -1,    27,    -1,    29,    -1,    31,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,    -1,   130,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,
      -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,   173,    -1,   175,    -1,    -1,   178,     3,     4,    -1,
       6,     7,    -1,   185,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    29,    -1,    31,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    59,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,    -1,    -1,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,   173,   103,   175,
      -1,    -1,   178,    -1,     3,     4,    -1,     6,     7,   185,
     186,    10,    11,    12,    13,    -1,    -1,    -1,    -1,   195,
     196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      29,    -1,    31,    -1,   139,   140,   141,   142,   143,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    57,   164,
      59,    -1,   167,   168,    -1,   170,   171,   172,    -1,    69,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     195,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
      -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,   173,   103,   175,    -1,    -1,   178,
       3,     4,     5,     6,     7,    -1,   185,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   161,    57,    -1,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,   162,
     163,    -1,    -1,    -1,   167,   168,    -1,   170,   171,   172,
     173,    -1,   175,   176,    -1,   178,    -1,    -1,    -1,    -1,
      -1,    -1,   185,   186,    -1,   188,    -1,   190,   191,    -1,
       3,     4,   195,     6,     7,    11,    12,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    31,    29,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,    -1,   130,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,   161,    -1,
      -1,    -1,    -1,    -1,   167,   168,    -1,   170,   171,   172,
     173,    27,   175,    29,    -1,   178,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,    -1,   130,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
      -1,    10,    11,    12,    13,   161,    -1,    -1,    -1,    -1,
      -1,   167,   168,    -1,   170,   171,   172,   173,    27,   175,
      29,    -1,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
      -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    -1,    -1,    -1,    -1,   167,   168,
      -1,   170,   171,   172,   173,    -1,   175,    -1,    -1,   178,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
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
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   198,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   198,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    30,    31,   198,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    59,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    31,   111,   112,    -1,
      -1,    -1,   103,    -1,   198,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    59,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    81,    -1,    -1,    -1,
     164,   197,    -1,   167,   168,    -1,   170,   171,   172,    -1,
     161,    -1,    -1,   164,   165,    -1,   167,   168,   103,   170,
     171,   172,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,   195,    -1,    -1,   185,    -1,    -1,    -1,    -1,    -1,
     191,    -1,    -1,    32,   195,   196,    -1,    -1,    -1,   197,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,   164,
     165,    70,   167,   168,    -1,   170,   171,   172,    -1,    78,
      79,    80,    81,    -1,    -1,    -1,    10,    11,    12,    -1,
     185,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     195,   196,    -1,    -1,   103,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,    69,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    38,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      70,   190,    -1,    -1,    -1,    -1,   195,   196,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,   136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    50,    51,   176,    -1,    -1,    -1,
      56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     190,    -1,    -1,    -1,    70,   195,   196,    -1,    -1,    -1,
      -1,   201,    78,    79,    80,    81,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,    69,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    50,    51,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    70,    -1,    -1,
     176,    -1,    -1,    -1,    -1,    78,    79,    80,    81,   185,
      -1,    -1,    -1,    -1,   190,    -1,    -1,    -1,    91,   195,
     196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    -1,   138,   139,   140,   141,   142,
     143,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
      -1,    -1,    -1,   156,    91,    -1,    -1,    -1,   161,    -1,
      -1,   164,   165,    -1,   167,   168,   103,   170,   171,   172,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   185,    -1,    -1,    -1,    -1,   124,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    70,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,   190,    -1,    -1,    -1,    91,   195,   196,
      -1,    -1,   199,    -1,   201,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    70,
      -1,    72,   176,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,   190,    -1,    -1,    -1,
      91,   195,   196,    -1,    -1,    -1,    -1,   201,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    70,    -1,    -1,   176,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,   190,
      -1,    -1,    -1,    91,   195,   196,    -1,    -1,    -1,    -1,
     201,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    70,    -1,    -1,   176,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,   190,    -1,    -1,    -1,    91,   195,   196,    -1,
      -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,   176,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   190,    -1,    -1,    -1,    -1,
     195,   196,    -1,    -1,    30,    31,   201,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     136,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   136,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,   136,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,   136,    -1,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   136,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,   136,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,   136,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    69,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,    -1,    -1,    -1,    -1,    -1,   130,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,   156,   136,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,   176,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,   190,    91,    -1,
      -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,   176,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,   190,    91,    81,
      -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
     112,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,   139,   140,   141,
     142,   143,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,   164,   176,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,    10,    11,    12,   190,    -1,    -1,
      -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,    -1,
      -1,    -1,    -1,   195,    30,    31,    32,    33,    34,    35,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69
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
     458,   459,   460,   461,   462,   464,   477,   479,   481,   122,
     123,   124,   137,   161,   171,   196,   213,   246,   327,   348,
     453,   348,   196,   348,   348,   348,   108,   348,   348,   348,
     439,   440,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,    81,    83,    91,   124,   139,   140,
     141,   142,   143,   156,   196,   224,   367,   408,   411,   416,
     453,   456,   453,    38,   348,   468,   469,   348,   124,   130,
     196,   224,   259,   408,   409,   410,   412,   416,   450,   451,
     452,   460,   465,   466,   196,   337,   413,   196,   337,   358,
     338,   348,   232,   337,   196,   196,   196,   337,   198,   348,
     213,   198,   348,     3,     4,     6,     7,    10,    11,    12,
      13,    27,    29,    31,    57,    59,    71,    72,    73,    74,
      75,    76,    77,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   128,   130,   131,
     132,   133,   137,   138,   144,   161,   165,   173,   175,   178,
     185,   196,   213,   214,   215,   226,   482,   502,   503,   506,
     198,   343,   345,   348,   199,   239,   348,   111,   112,   164,
     216,   217,   218,   219,   223,    83,   201,   293,   294,   123,
     130,   122,   130,    83,   295,   196,   196,   196,   196,   213,
     265,   485,   196,   196,    70,    70,    70,    70,    70,   338,
      83,    90,   157,   158,   159,   474,   475,   164,   199,   223,
     223,   213,   266,   485,   165,   196,   485,   485,    83,   192,
     199,   359,    27,   336,   340,   348,   349,   453,   457,   228,
     199,    90,   414,   474,    90,   474,   474,    32,   164,   181,
     486,   196,     9,   198,    38,   245,   165,   264,   485,   124,
     191,   246,   328,   198,   198,   198,   198,   198,   198,   198,
     198,    10,    11,    12,    30,    31,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    57,
      69,   198,    70,    70,   199,   160,   131,   171,   173,   186,
     188,   267,   326,   327,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    67,    68,   134,
     135,   443,    70,   199,   448,   196,   196,    70,   199,   201,
     461,   196,   245,   246,    14,   348,   198,   136,    48,   213,
     438,    90,   336,   349,   160,   453,   136,   203,     9,   423,
     260,   336,   349,   453,   486,   160,   196,   415,   443,   448,
     197,   348,    32,   230,     8,   360,     9,   198,   230,   231,
     338,   339,   348,   213,   279,   234,   198,   198,   198,   138,
     144,   506,   506,   181,   505,   196,   111,   506,    14,   160,
     138,   144,   161,   213,   215,   198,   198,   198,   240,   115,
     178,   198,   216,   218,   216,   218,   223,   199,     9,   424,
     198,   102,   164,   199,   453,     9,   198,   130,   130,    14,
       9,   198,   453,   478,   338,   336,   349,   453,   456,   457,
     197,   181,   257,   137,   453,   467,   468,   348,   368,   369,
     338,   389,   389,   368,   389,   198,    70,   443,   157,   475,
      82,   348,   453,    90,   157,   475,   223,   212,   198,   199,
     252,   262,   398,   400,    91,   196,   201,   361,   362,   364,
     407,   411,   459,   461,   479,    14,   102,   480,   355,   356,
     357,   289,   290,   441,   442,   197,   197,   197,   197,   197,
     200,   229,   230,   247,   254,   261,   441,   348,   202,   204,
     205,   213,   487,   488,   506,    38,   174,   291,   292,   348,
     482,   196,   485,   255,   245,   348,   348,   348,   348,    32,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   412,   348,   348,   463,   463,   348,
     470,   471,   130,   199,   214,   215,   460,   461,   265,   213,
     266,   485,   485,   264,   246,    38,   340,   343,   345,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   165,   199,   213,   444,   445,   446,   447,   460,
     463,   348,   291,   291,   463,   348,   467,   245,   197,   348,
     196,   437,     9,   423,   197,   197,    38,   348,    38,   348,
     415,   197,   197,   197,   460,   291,   199,   213,   444,   445,
     460,   197,   228,   283,   199,   345,   348,   348,    94,    32,
     230,   277,   198,    28,   102,    14,     9,   197,    32,   199,
     280,   506,    31,    91,   174,   226,   499,   500,   501,   196,
       9,    50,    51,    56,    58,    70,   138,   139,   140,   141,
     142,   143,   185,   196,   224,   375,   378,   381,   384,   387,
     393,   408,   416,   417,   419,   420,   213,   504,   228,   196,
     238,   199,   198,   199,   198,   102,   164,   111,   112,   164,
     219,   220,   221,   222,   223,   219,   213,   348,   294,   417,
      83,     9,   197,   197,   197,   197,   197,   197,   197,   198,
      50,    51,   495,   497,   498,   132,   270,   196,     9,   197,
     197,   136,   203,     9,   423,     9,   423,   203,   203,   203,
     203,    83,    85,   213,   476,   213,    70,   200,   200,   209,
     211,    32,   133,   269,   180,    54,   165,   180,   402,   349,
     136,     9,   423,   197,   160,   506,   506,    14,   360,   289,
     228,   193,     9,   424,   506,   507,   443,   448,   443,   200,
       9,   423,   182,   453,   348,   197,     9,   424,    14,   352,
     248,   132,   268,   196,   485,   348,    32,   203,   203,   136,
     200,     9,   423,   348,   486,   196,   258,   253,   263,    14,
     480,   256,   245,    72,   453,   348,   486,   203,   200,   197,
     197,   203,   200,   197,    50,    51,    70,    78,    79,    80,
      91,   138,   139,   140,   141,   142,   143,   156,   185,   213,
     376,   379,   382,   385,   388,   408,   419,   426,   428,   429,
     433,   436,   213,   453,   453,   136,   268,   443,   448,   197,
     348,   284,    75,    76,   285,   228,   337,   228,   339,   102,
      38,   137,   274,   453,   417,   213,    32,   230,   278,   198,
     281,   198,   281,     9,   423,    91,   226,   136,   160,     9,
     423,   197,   174,   487,   488,   489,   487,   417,   417,   417,
     417,   417,   422,   425,   196,    70,    70,    70,    70,    70,
     196,   417,   160,   199,    10,    11,    12,    31,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      69,   160,   486,   200,   408,   199,   242,   218,   218,   213,
     219,   219,   223,     9,   424,   200,   200,    14,   453,   198,
     182,     9,   423,   213,   271,   408,   199,   467,   137,   453,
      14,   348,   348,   203,   348,   200,   209,   506,   271,   199,
     401,    14,   197,   348,   361,   460,   198,   506,   193,   200,
      32,   493,   442,    38,    83,   174,   444,   445,   447,   444,
     445,   506,    38,   174,   348,   417,   289,   196,   408,   269,
     353,   249,   348,   348,   348,   200,   196,   291,   270,    32,
     269,   506,    14,   268,   485,   412,   200,   196,    14,    78,
      79,    80,   213,   427,   427,   429,   431,   432,    52,   196,
      70,    70,    70,    70,    70,    90,   157,   196,   160,     9,
     423,   197,   437,    38,   348,   269,   200,    75,    76,   286,
     337,   230,   200,   198,    95,   198,   274,   453,   196,   136,
     273,    14,   228,   281,   105,   106,   107,   281,   200,   506,
     182,   136,   160,   506,   213,   174,   499,     9,   197,   423,
     136,   203,     9,   423,   422,   370,   371,   417,   390,   417,
     418,   390,   370,   390,   361,   363,   365,   197,   130,   214,
     417,   472,   473,   417,   417,   417,    32,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   504,    83,   243,   200,   200,   222,   198,   417,   498,
     102,   103,   494,   496,     9,   299,   197,   196,   340,   345,
     348,   136,   203,   200,   480,   299,   166,   179,   199,   397,
     404,   166,   199,   403,   136,   198,   493,   506,   360,   507,
      83,   174,    14,    83,   486,   453,   348,   197,   289,   199,
     289,   196,   136,   196,   291,   197,   199,   506,   199,   198,
     506,   269,   250,   415,   291,   136,   203,     9,   423,   428,
     431,   372,   373,   429,   391,   429,   430,   391,   372,   391,
     157,   361,   434,   435,    81,   429,   453,   199,   337,    32,
      77,   230,   198,   339,   273,   467,   274,   197,   417,   101,
     105,   198,   348,    32,   198,   282,   200,   182,   506,   213,
     136,   174,    32,   197,   417,   417,   197,   203,     9,   423,
     136,   203,     9,   423,   203,   203,   203,   136,     9,   423,
     197,   136,   200,     9,   423,   417,    32,   197,   228,   198,
     198,   213,   506,   506,   494,   408,     4,   112,   117,   123,
     125,   167,   168,   170,   200,   300,   325,   326,   327,   332,
     333,   334,   335,   441,   467,   348,   200,   199,   200,    54,
     348,   348,   348,   360,    38,    83,   174,    14,    83,   348,
     196,   493,   197,   299,   197,   289,   348,   291,   197,   299,
     480,   299,   198,   199,   196,   197,   429,   429,   197,   203,
       9,   423,   136,   203,     9,   423,   203,   203,   203,   136,
     197,     9,   423,   299,    32,   228,   198,   197,   197,   197,
     235,   198,   198,   282,   228,   136,   506,   506,   136,   417,
     417,   417,   417,   361,   417,   417,   417,   199,   200,   496,
     132,   133,   186,   214,   483,   506,   272,   408,   112,   335,
      31,   125,   138,   144,   165,   171,   309,   310,   311,   312,
     408,   169,   317,   318,   128,   196,   213,   319,   320,   301,
     246,   506,     9,   198,     9,   198,   198,   480,   326,   197,
     296,   165,   399,   200,   200,    83,   174,    14,    83,   348,
     291,   117,   350,   493,   200,   493,   197,   197,   200,   199,
     200,   299,   289,   136,   429,   429,   429,   429,   361,   200,
     228,   233,   236,    32,   230,   276,   228,   506,   197,   417,
     136,   136,   136,   228,   408,   408,   485,    14,   214,     9,
     198,   199,   483,   480,   312,   181,   199,     9,   198,     3,
       4,     5,     6,     7,    10,    11,    12,    13,    27,    28,
      29,    57,    71,    72,    73,    74,    75,    76,    77,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     137,   138,   145,   146,   147,   148,   149,   161,   162,   163,
     173,   175,   176,   178,   185,   186,   188,   190,   191,   213,
     405,   406,     9,   198,   165,   169,   213,   320,   321,   322,
     198,    83,   331,   245,   302,   483,   483,    14,   246,   200,
     297,   298,   483,    14,    83,   348,   197,   196,   493,   198,
     199,   323,   350,   493,   296,   200,   197,   429,   136,   136,
      32,   230,   275,   276,   228,   417,   417,   417,   200,   198,
     198,   417,   408,   305,   506,   313,   314,   416,   310,    14,
      32,    51,   315,   318,     9,    36,   197,    31,    50,    53,
      14,     9,   198,   215,   484,   331,    14,   506,   245,   198,
      14,   348,    38,    83,   396,   199,   228,   493,   323,   200,
     493,   429,   429,   228,    99,   241,   200,   213,   226,   306,
     307,   308,     9,   423,     9,   423,   200,   417,   406,   406,
      59,   316,   321,   321,    31,    50,    53,   417,    83,   181,
     196,   198,   417,   485,   417,    83,     9,   424,   228,   200,
     199,   323,    97,   198,   115,   237,   160,   102,   506,   182,
     416,   172,    14,   495,   303,   196,    38,    83,   197,   200,
     228,   198,   196,   178,   244,   213,   326,   327,   182,   417,
     182,   287,   288,   442,   304,    83,   200,   408,   242,   175,
     213,   198,   197,     9,   424,   119,   120,   121,   329,   330,
     287,    83,   272,   198,   493,   442,   507,   197,   197,   198,
     195,   490,   329,    38,    83,   174,   493,   199,   491,   492,
     506,   198,   199,   324,   507,    83,   174,    14,    83,   490,
     228,     9,   424,    14,   494,   228,    38,    83,   174,    14,
      83,   348,   324,   200,   492,   506,   200,    83,   174,    14,
      83,   348,    14,    83,   348,   348
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
#line 2869 "hphp.y"
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
#line 2884 "hphp.y"
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
#line 3027 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3028 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3030 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3031 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3032 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3037 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3045 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3052 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3056 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3063 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3072 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3076 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3080 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3083 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
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
#line 3104 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3105 "hphp.y"
    { (yyval).reset();;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3110 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3111 "hphp.y"
    { (yyval)++;;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3117 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3118 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 940:

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

  case 941:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3133 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3138 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 946:

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

  case 947:

/* Line 1455 of yacc.c  */
#line 3150 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3154 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3155 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3157 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3158 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3159 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3160 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3165 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3166 "hphp.y"
    { (yyval).reset();;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3170 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3171 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3172 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3173 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3176 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3178 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3179 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3180 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3185 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3186 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3190 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3191 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3192 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3193 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3198 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3199 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3204 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3206 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3208 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3209 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3213 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3215 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3216 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3218 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3223 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3225 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 982:

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

  case 983:

/* Line 1455 of yacc.c  */
#line 3237 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3239 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3240 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3243 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3244 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3249 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3250 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3251 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3252 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3253 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3254 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 995:

/* Line 1455 of yacc.c  */
#line 3255 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 996:

/* Line 1455 of yacc.c  */
#line 3256 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3257 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3258 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3259 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3263 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3264 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3269 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3271 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 1006:

/* Line 1455 of yacc.c  */
#line 3285 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3290 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3294 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3299 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 1010:

/* Line 1455 of yacc.c  */
#line 3305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1011:

/* Line 1455 of yacc.c  */
#line 3306 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1012:

/* Line 1455 of yacc.c  */
#line 3310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1013:

/* Line 1455 of yacc.c  */
#line 3311 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1014:

/* Line 1455 of yacc.c  */
#line 3317 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1015:

/* Line 1455 of yacc.c  */
#line 3321 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 1016:

/* Line 1455 of yacc.c  */
#line 3327 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3331 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 1018:

/* Line 1455 of yacc.c  */
#line 3338 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 1019:

/* Line 1455 of yacc.c  */
#line 3339 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1020:

/* Line 1455 of yacc.c  */
#line 3343 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1021:

/* Line 1455 of yacc.c  */
#line 3346 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1022:

/* Line 1455 of yacc.c  */
#line 3352 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1023:

/* Line 1455 of yacc.c  */
#line 3357 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1024:

/* Line 1455 of yacc.c  */
#line 3358 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1025:

/* Line 1455 of yacc.c  */
#line 3359 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1026:

/* Line 1455 of yacc.c  */
#line 3360 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1033:

/* Line 1455 of yacc.c  */
#line 3381 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1034:

/* Line 1455 of yacc.c  */
#line 3382 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1; ;}
    break;

  case 1037:

/* Line 1455 of yacc.c  */
#line 3391 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1040:

/* Line 1455 of yacc.c  */
#line 3402 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 1041:

/* Line 1455 of yacc.c  */
#line 3404 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 1042:

/* Line 1455 of yacc.c  */
#line 3408 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 1043:

/* Line 1455 of yacc.c  */
#line 3411 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 1044:

/* Line 1455 of yacc.c  */
#line 3415 "hphp.y"
    {;}
    break;

  case 1045:

/* Line 1455 of yacc.c  */
#line 3416 "hphp.y"
    {;}
    break;

  case 1046:

/* Line 1455 of yacc.c  */
#line 3417 "hphp.y"
    {;}
    break;

  case 1047:

/* Line 1455 of yacc.c  */
#line 3423 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 1048:

/* Line 1455 of yacc.c  */
#line 3428 "hphp.y"
    {
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   ;}
    break;

  case 1049:

/* Line 1455 of yacc.c  */
#line 3437 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 1050:

/* Line 1455 of yacc.c  */
#line 3443 "hphp.y"
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]), (yyvsp[(6) - (6)]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   ;}
    break;

  case 1051:

/* Line 1455 of yacc.c  */
#line 3451 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1052:

/* Line 1455 of yacc.c  */
#line 3452 "hphp.y"
    { ;}
    break;

  case 1053:

/* Line 1455 of yacc.c  */
#line 3458 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (3)]), true); ;}
    break;

  case 1054:

/* Line 1455 of yacc.c  */
#line 3460 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)]), false); ;}
    break;

  case 1055:

/* Line 1455 of yacc.c  */
#line 3461 "hphp.y"
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       ;}
    break;

  case 1056:

/* Line 1455 of yacc.c  */
#line 3466 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); ;}
    break;

  case 1057:

/* Line 1455 of yacc.c  */
#line 3472 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1058:

/* Line 1455 of yacc.c  */
#line 3477 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1059:

/* Line 1455 of yacc.c  */
#line 3482 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1060:

/* Line 1455 of yacc.c  */
#line 3486 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1061:

/* Line 1455 of yacc.c  */
#line 3491 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 1062:

/* Line 1455 of yacc.c  */
#line 3493 "hphp.y"
    { _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)])); (yyval) = (yyvsp[(2) - (5)]);;}
    break;

  case 1063:

/* Line 1455 of yacc.c  */
#line 3499 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1064:

/* Line 1455 of yacc.c  */
#line 3502 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1065:

/* Line 1455 of yacc.c  */
#line 3505 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1066:

/* Line 1455 of yacc.c  */
#line 3506 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1067:

/* Line 1455 of yacc.c  */
#line 3509 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1068:

/* Line 1455 of yacc.c  */
#line 3512 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1069:

/* Line 1455 of yacc.c  */
#line 3515 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1070:

/* Line 1455 of yacc.c  */
#line 3518 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1071:

/* Line 1455 of yacc.c  */
#line 3520 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1072:

/* Line 1455 of yacc.c  */
#line 3526 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1073:

/* Line 1455 of yacc.c  */
#line 3532 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1074:

/* Line 1455 of yacc.c  */
#line 3540 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1075:

/* Line 1455 of yacc.c  */
#line 3541 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14762 "hphp.5.tab.cpp"
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
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}

