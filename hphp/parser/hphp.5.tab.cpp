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
#line 895 "hphp.5.tab.cpp"

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
#define YYLAST   17928

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  204
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  296
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1059
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1944

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
    1958,  1959,  1965,  1969,  1972,  1973,  1979,  1983,  1988,  1993,
    1998,  2003,  2008,  2013,  2018,  2023,  2028,  2031,  2032,  2035,
    2036,  2039,  2040,  2045,  2050,  2055,  2060,  2062,  2064,  2066,
    2068,  2070,  2072,  2074,  2078,  2080,  2084,  2089,  2091,  2094,
    2099,  2102,  2109,  2110,  2112,  2117,  2118,  2121,  2122,  2124,
    2126,  2130,  2132,  2136,  2138,  2140,  2144,  2148,  2150,  2152,
    2154,  2156,  2158,  2160,  2162,  2164,  2166,  2168,  2170,  2172,
    2174,  2176,  2178,  2180,  2182,  2184,  2186,  2188,  2190,  2192,
    2194,  2196,  2198,  2200,  2202,  2204,  2206,  2208,  2210,  2212,
    2214,  2216,  2218,  2220,  2222,  2224,  2226,  2228,  2230,  2232,
    2234,  2236,  2238,  2240,  2242,  2244,  2246,  2248,  2250,  2252,
    2254,  2256,  2258,  2260,  2262,  2264,  2266,  2268,  2270,  2272,
    2274,  2276,  2278,  2280,  2282,  2284,  2286,  2288,  2290,  2292,
    2294,  2296,  2298,  2300,  2302,  2304,  2306,  2308,  2310,  2315,
    2317,  2319,  2321,  2323,  2325,  2327,  2331,  2333,  2337,  2339,
    2341,  2343,  2347,  2349,  2351,  2353,  2356,  2358,  2359,  2360,
    2362,  2364,  2368,  2369,  2371,  2373,  2375,  2377,  2379,  2381,
    2383,  2385,  2387,  2389,  2391,  2393,  2395,  2399,  2402,  2404,
    2406,  2411,  2415,  2420,  2422,  2424,  2426,  2428,  2430,  2434,
    2438,  2442,  2446,  2450,  2454,  2458,  2462,  2466,  2470,  2474,
    2478,  2482,  2486,  2490,  2494,  2498,  2502,  2505,  2508,  2511,
    2514,  2518,  2522,  2526,  2530,  2534,  2538,  2542,  2546,  2550,
    2556,  2561,  2565,  2567,  2571,  2575,  2577,  2579,  2581,  2583,
    2585,  2589,  2593,  2597,  2600,  2601,  2603,  2604,  2606,  2607,
    2613,  2617,  2621,  2623,  2625,  2627,  2629,  2633,  2636,  2638,
    2640,  2642,  2644,  2646,  2650,  2652,  2654,  2656,  2660,  2662,
    2665,  2668,  2673,  2677,  2682,  2684,  2686,  2688,  2692,  2694,
    2697,  2698,  2704,  2708,  2712,  2714,  2718,  2720,  2723,  2724,
    2730,  2734,  2737,  2738,  2742,  2743,  2748,  2751,  2752,  2756,
    2760,  2762,  2763,  2765,  2767,  2769,  2771,  2775,  2777,  2779,
    2781,  2785,  2787,  2789,  2793,  2797,  2800,  2805,  2808,  2813,
    2819,  2825,  2831,  2837,  2839,  2841,  2843,  2845,  2847,  2849,
    2853,  2857,  2862,  2867,  2871,  2873,  2875,  2877,  2879,  2883,
    2885,  2890,  2894,  2896,  2898,  2900,  2902,  2904,  2908,  2912,
    2917,  2922,  2926,  2928,  2930,  2938,  2948,  2956,  2963,  2972,
    2974,  2977,  2982,  2987,  2989,  2991,  2993,  2998,  3000,  3001,
    3003,  3006,  3008,  3010,  3012,  3016,  3020,  3024,  3025,  3027,
    3029,  3033,  3037,  3040,  3044,  3051,  3052,  3054,  3059,  3062,
    3063,  3069,  3073,  3077,  3079,  3086,  3091,  3096,  3099,  3102,
    3103,  3109,  3113,  3117,  3119,  3122,  3123,  3129,  3133,  3137,
    3139,  3142,  3145,  3147,  3150,  3152,  3157,  3161,  3165,  3172,
    3176,  3178,  3180,  3182,  3187,  3192,  3197,  3202,  3207,  3212,
    3215,  3218,  3223,  3226,  3229,  3231,  3235,  3239,  3243,  3244,
    3247,  3253,  3260,  3267,  3275,  3277,  3280,  3282,  3285,  3287,
    3292,  3294,  3299,  3303,  3304,  3306,  3310,  3313,  3317,  3319,
    3321,  3322,  3323,  3327,  3329,  3333,  3337,  3340,  3341,  3344,
    3347,  3350,  3353,  3355,  3358,  3363,  3366,  3372,  3376,  3378,
    3380,  3381,  3385,  3390,  3396,  3403,  3407,  3409,  3413,  3416,
    3418,  3419,  3424,  3426,  3430,  3433,  3438,  3444,  3447,  3450,
    3452,  3454,  3456,  3458,  3462,  3465,  3467,  3476,  3483,  3485
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     205,     0,    -1,    -1,   206,   207,    -1,   207,   208,    -1,
      -1,   228,    -1,   245,    -1,   252,    -1,   249,    -1,   259,
      -1,   473,    -1,   129,   194,   195,   196,    -1,   159,   221,
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
     217,    -1,   217,    -1,   215,   416,    -1,   221,    -1,   162,
     221,    -1,   221,   102,   211,    -1,   162,   221,   102,   211,
      -1,   218,     9,   220,    -1,   220,    -1,   218,   416,    -1,
     217,    -1,   111,   217,    -1,   112,   217,    -1,   211,    -1,
     221,   162,   211,    -1,   221,    -1,   159,   162,   221,    -1,
     162,   221,    -1,   222,   478,    -1,   222,   478,    -1,   225,
       9,   474,    14,   409,    -1,   112,   474,    14,   409,    -1,
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
     151,   336,   196,    -1,   126,   194,   470,   195,   196,    -1,
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
      38,    -1,    -1,   111,    -1,    -1,   244,   243,   477,   246,
     194,   287,   195,   485,   321,    -1,    -1,   325,   244,   243,
     477,   247,   194,   287,   195,   485,   321,    -1,    -1,   433,
     324,   244,   243,   477,   248,   194,   287,   195,   485,   321,
      -1,    -1,   169,   211,   250,    32,   498,   472,   197,   294,
     198,    -1,    -1,   433,   169,   211,   251,    32,   498,   472,
     197,   294,   198,    -1,    -1,   265,   262,   253,   266,   267,
     197,   297,   198,    -1,    -1,   433,   265,   262,   254,   266,
     267,   197,   297,   198,    -1,    -1,   131,   263,   255,   268,
     197,   297,   198,    -1,    -1,   433,   131,   263,   256,   268,
     197,   297,   198,    -1,    -1,   130,   258,   407,   266,   267,
     197,   297,   198,    -1,    -1,   171,   264,   260,   267,   197,
     297,   198,    -1,    -1,   433,   171,   264,   261,   267,   197,
     297,   198,    -1,   477,    -1,   163,    -1,   477,    -1,   477,
      -1,   130,    -1,   123,   130,    -1,   123,   122,   130,    -1,
     122,   123,   130,    -1,   122,   130,    -1,   132,   400,    -1,
      -1,   133,   269,    -1,    -1,   132,   269,    -1,    -1,   400,
      -1,   269,     9,   400,    -1,   400,    -1,   270,     9,   400,
      -1,   136,   272,    -1,    -1,   445,    -1,    38,   445,    -1,
     137,   194,   459,   195,    -1,   228,    -1,    32,   226,    97,
     196,    -1,   228,    -1,    32,   226,    99,   196,    -1,   228,
      -1,    32,   226,    95,   196,    -1,   228,    -1,    32,   226,
     101,   196,    -1,   211,    14,   409,    -1,   277,     9,   211,
      14,   409,    -1,   197,   279,   198,    -1,   197,   196,   279,
     198,    -1,    32,   279,   105,   196,    -1,    32,   196,   279,
     105,   196,    -1,   279,   106,   346,   280,   226,    -1,   279,
     107,   280,   226,    -1,    -1,    32,    -1,   196,    -1,   281,
      75,   335,   228,    -1,    -1,   282,    75,   335,    32,   226,
      -1,    -1,    76,   228,    -1,    -1,    76,    32,   226,    -1,
      -1,   286,     9,   434,   327,   499,   172,    83,    -1,   286,
       9,   434,   327,   499,    38,   172,    83,    -1,   286,     9,
     434,   327,   499,   172,    -1,   286,   416,    -1,   434,   327,
     499,   172,    83,    -1,   434,   327,   499,    38,   172,    83,
      -1,   434,   327,   499,   172,    -1,    -1,   434,   327,   499,
      83,    -1,   434,   327,   499,    38,    83,    -1,   434,   327,
     499,    38,    83,    14,   346,    -1,   434,   327,   499,    83,
      14,   346,    -1,   286,     9,   434,   327,   499,    83,    -1,
     286,     9,   434,   327,   499,    38,    83,    -1,   286,     9,
     434,   327,   499,    38,    83,    14,   346,    -1,   286,     9,
     434,   327,   499,    83,    14,   346,    -1,   288,     9,   434,
     499,   172,    83,    -1,   288,     9,   434,   499,    38,   172,
      83,    -1,   288,     9,   434,   499,   172,    -1,   288,   416,
      -1,   434,   499,   172,    83,    -1,   434,   499,    38,   172,
      83,    -1,   434,   499,   172,    -1,    -1,   434,   499,    83,
      -1,   434,   499,    38,    83,    -1,   434,   499,    38,    83,
      14,   346,    -1,   434,   499,    83,    14,   346,    -1,   288,
       9,   434,   499,    83,    -1,   288,     9,   434,   499,    38,
      83,    -1,   288,     9,   434,   499,    38,    83,    14,   346,
      -1,   288,     9,   434,   499,    83,    14,   346,    -1,   290,
     416,    -1,    -1,   346,    -1,    38,   445,    -1,   172,   346,
      -1,   290,     9,   346,    -1,   290,     9,   172,   346,    -1,
     290,     9,    38,   445,    -1,   291,     9,   292,    -1,   292,
      -1,    83,    -1,   199,   445,    -1,   199,   197,   346,   198,
      -1,   293,     9,    83,    -1,   293,     9,    83,    14,   409,
      -1,    83,    -1,    83,    14,   409,    -1,   294,   295,    -1,
      -1,   296,   196,    -1,   475,    14,   409,    -1,   297,   298,
      -1,    -1,    -1,   323,   299,   329,   196,    -1,    -1,   325,
     498,   300,   329,   196,    -1,   330,   196,    -1,   331,   196,
      -1,   332,   196,    -1,    -1,   324,   244,   243,   476,   194,
     301,   285,   195,   485,   482,   322,    -1,    -1,   433,   324,
     244,   243,   477,   194,   302,   285,   195,   485,   482,   322,
      -1,   165,   307,   196,    -1,   166,   315,   196,    -1,   168,
     317,   196,    -1,     4,   132,   400,   196,    -1,     4,   133,
     400,   196,    -1,   117,   270,   196,    -1,   117,   270,   197,
     303,   198,    -1,   303,   304,    -1,   303,   305,    -1,    -1,
     224,   158,   211,   173,   270,   196,    -1,   306,   102,   324,
     211,   196,    -1,   306,   102,   325,   196,    -1,   224,   158,
     211,    -1,   211,    -1,   308,    -1,   307,     9,   308,    -1,
     309,   397,   313,   314,    -1,   163,    -1,    31,   310,    -1,
     310,    -1,   138,    -1,   138,   179,   498,   415,   180,    -1,
     138,   179,   498,     9,   498,   180,    -1,   400,    -1,   125,
      -1,   169,   197,   312,   198,    -1,   142,    -1,   408,    -1,
     311,     9,   408,    -1,   311,   415,    -1,    14,   409,    -1,
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
     475,    14,   409,    -1,   112,   475,    14,   409,    -1,   331,
       9,   475,    -1,   123,   112,   475,    -1,   123,   333,   472,
      -1,   333,   472,    14,   498,    -1,   112,   184,   477,    -1,
     194,   334,   195,    -1,    72,   404,   407,    -1,    72,   257,
      -1,    71,   346,    -1,   389,    -1,   384,    -1,   194,   346,
     195,    -1,   336,     9,   346,    -1,   346,    -1,   336,    -1,
      -1,    27,    -1,    27,   346,    -1,    27,   346,   136,   346,
      -1,   194,   338,   195,    -1,   445,    14,   338,    -1,   137,
     194,   459,   195,    14,   338,    -1,    29,   346,    -1,   445,
      14,   341,    -1,    28,   346,    -1,   445,    14,   343,    -1,
     137,   194,   459,   195,    14,   343,    -1,   347,    -1,   445,
      -1,   334,    -1,   449,    -1,   448,    -1,   137,   194,   459,
     195,    14,   346,    -1,   445,    14,   346,    -1,   445,    14,
      38,   445,    -1,   445,    14,    38,    72,   404,   407,    -1,
     445,    26,   346,    -1,   445,    25,   346,    -1,   445,    24,
     346,    -1,   445,    23,   346,    -1,   445,    22,   346,    -1,
     445,    21,   346,    -1,   445,    20,   346,    -1,   445,    19,
     346,    -1,   445,    18,   346,    -1,   445,    17,   346,    -1,
     445,    16,   346,    -1,   445,    15,   346,    -1,   445,    68,
      -1,    68,   445,    -1,   445,    67,    -1,    67,   445,    -1,
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
      33,   346,    -1,   469,    -1,    66,   346,    -1,    65,   346,
      -1,    64,   346,    -1,    63,   346,    -1,    62,   346,    -1,
      61,   346,    -1,    60,   346,    -1,    73,   405,    -1,    59,
     346,    -1,   413,    -1,   365,    -1,   372,    -1,   375,    -1,
     378,    -1,   364,    -1,   200,   406,   200,    -1,    13,   346,
      -1,   386,    -1,   117,   194,   388,   416,   195,    -1,    -1,
      -1,   244,   243,   194,   350,   287,   195,   485,   348,   485,
     197,   226,   198,    -1,    -1,   325,   244,   243,   194,   351,
     287,   195,   485,   348,   485,   197,   226,   198,    -1,    -1,
     189,    83,   353,   358,    -1,    -1,   189,   190,   354,   287,
     191,   485,   358,    -1,    -1,   189,   197,   355,   226,   198,
      -1,    -1,    83,   356,   358,    -1,    -1,   190,   357,   287,
     191,   485,   358,    -1,     8,   346,    -1,     8,   343,    -1,
       8,   197,   226,   198,    -1,    91,    -1,   471,    -1,   360,
       9,   359,   136,   346,    -1,   359,   136,   346,    -1,   361,
       9,   359,   136,   409,    -1,   359,   136,   409,    -1,   360,
     415,    -1,    -1,   361,   415,    -1,    -1,   183,   194,   362,
     195,    -1,   138,   194,   460,   195,    -1,    70,   460,   201,
      -1,   367,   415,    -1,    -1,   367,     9,   346,   136,   346,
      -1,   346,   136,   346,    -1,   369,   415,    -1,    -1,   369,
       9,   409,   136,   409,    -1,   409,   136,   409,    -1,   371,
     415,    -1,    -1,   371,     9,   421,   136,   421,    -1,   421,
     136,   421,    -1,   139,    70,   366,   201,    -1,   139,    70,
     368,   201,    -1,   139,    70,   370,   201,    -1,   140,    70,
     381,   201,    -1,   140,    70,   382,   201,    -1,   140,    70,
     383,   201,    -1,   141,    70,   381,   201,    -1,   141,    70,
     382,   201,    -1,   141,    70,   383,   201,    -1,   336,   415,
      -1,    -1,   410,   415,    -1,    -1,   422,   415,    -1,    -1,
     400,   197,   462,   198,    -1,   400,   197,   464,   198,    -1,
     386,    70,   455,   201,    -1,   387,    70,   455,   201,    -1,
     365,    -1,   372,    -1,   375,    -1,   378,    -1,   471,    -1,
     448,    -1,    91,    -1,   194,   347,   195,    -1,    81,    -1,
     388,     9,    83,    -1,   388,     9,    38,    83,    -1,    83,
      -1,    38,    83,    -1,   177,   163,   390,   178,    -1,   392,
      54,    -1,   392,   178,   393,   177,    54,   391,    -1,    -1,
     163,    -1,   392,   394,    14,   395,    -1,    -1,   393,   396,
      -1,    -1,   163,    -1,   164,    -1,   197,   346,   198,    -1,
     164,    -1,   197,   346,   198,    -1,   389,    -1,   398,    -1,
     397,    32,   398,    -1,   397,    51,   398,    -1,   211,    -1,
      73,    -1,   111,    -1,   112,    -1,   113,    -1,    27,    -1,
      29,    -1,    28,    -1,   114,    -1,   115,    -1,   176,    -1,
     116,    -1,    74,    -1,    75,    -1,    77,    -1,    76,    -1,
      94,    -1,    95,    -1,    93,    -1,    96,    -1,    97,    -1,
      98,    -1,    99,    -1,   100,    -1,   101,    -1,    57,    -1,
     102,    -1,   104,    -1,   105,    -1,   106,    -1,   107,    -1,
     108,    -1,   110,    -1,   109,    -1,    92,    -1,    13,    -1,
     130,    -1,   131,    -1,   132,    -1,   133,    -1,    72,    -1,
      71,    -1,   125,    -1,     5,    -1,     7,    -1,     6,    -1,
       4,    -1,     3,    -1,   159,    -1,   117,    -1,   118,    -1,
     127,    -1,   128,    -1,   129,    -1,   124,    -1,   123,    -1,
     122,    -1,   121,    -1,   120,    -1,   119,    -1,   189,    -1,
     126,    -1,   137,    -1,   138,    -1,    10,    -1,    12,    -1,
      11,    -1,   143,    -1,   145,    -1,   144,    -1,   146,    -1,
     147,    -1,   161,    -1,   160,    -1,   188,    -1,   171,    -1,
     174,    -1,   173,    -1,   184,    -1,   186,    -1,   183,    -1,
     223,   194,   289,   195,    -1,   224,    -1,   163,    -1,   400,
      -1,   408,    -1,   124,    -1,   453,    -1,   194,   347,   195,
      -1,   401,    -1,   402,   158,   452,    -1,   401,    -1,   451,
      -1,   399,    -1,   403,   158,   452,    -1,   400,    -1,   124,
      -1,   457,    -1,   194,   195,    -1,   335,    -1,    -1,    -1,
      90,    -1,   466,    -1,   194,   289,   195,    -1,    -1,    78,
      -1,    79,    -1,    80,    -1,    91,    -1,   146,    -1,   147,
      -1,   161,    -1,   143,    -1,   174,    -1,   144,    -1,   145,
      -1,   160,    -1,   188,    -1,   154,    90,   155,    -1,   154,
     155,    -1,   408,    -1,   222,    -1,   138,   194,   414,   195,
      -1,    70,   414,   201,    -1,   183,   194,   363,   195,    -1,
     373,    -1,   376,    -1,   379,    -1,   412,    -1,   385,    -1,
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
      -1,   410,     9,   409,    -1,   409,    -1,   400,   158,   130,
      -1,   400,   158,   212,    -1,   411,    -1,   222,    -1,    82,
      -1,   471,    -1,   408,    -1,   202,   466,   202,    -1,   203,
     466,   203,    -1,   154,   466,   155,    -1,   417,   415,    -1,
      -1,     9,    -1,    -1,     9,    -1,    -1,   417,     9,   409,
     136,   409,    -1,   417,     9,   409,    -1,   409,   136,   409,
      -1,   409,    -1,    78,    -1,    79,    -1,    80,    -1,   154,
      90,   155,    -1,   154,   155,    -1,    78,    -1,    79,    -1,
      80,    -1,   211,    -1,    91,    -1,    91,    52,   420,    -1,
     418,    -1,   420,    -1,   411,    -1,   400,   158,    81,    -1,
     211,    -1,    50,   419,    -1,    51,   419,    -1,   138,   194,
     423,   195,    -1,    70,   423,   201,    -1,   183,   194,   426,
     195,    -1,   374,    -1,   377,    -1,   380,    -1,   422,     9,
     421,    -1,   421,    -1,   424,   415,    -1,    -1,   424,     9,
     421,   136,   421,    -1,   424,     9,   421,    -1,   421,   136,
     421,    -1,   421,    -1,   425,     9,   421,    -1,   421,    -1,
     427,   415,    -1,    -1,   427,     9,   359,   136,   421,    -1,
     359,   136,   421,    -1,   425,   415,    -1,    -1,   194,   428,
     195,    -1,    -1,   430,     9,   211,   429,    -1,   211,   429,
      -1,    -1,   432,   430,   415,    -1,    49,   431,    48,    -1,
     433,    -1,    -1,   134,    -1,   135,    -1,   211,    -1,   163,
      -1,   197,   346,   198,    -1,   436,    -1,   452,    -1,   211,
      -1,   197,   346,   198,    -1,   438,    -1,   452,    -1,    70,
     455,   201,    -1,   197,   346,   198,    -1,   446,   440,    -1,
     194,   334,   195,   440,    -1,   458,   440,    -1,   194,   334,
     195,   440,    -1,   194,   334,   195,   435,   437,    -1,   194,
     347,   195,   435,   437,    -1,   194,   334,   195,   435,   436,
      -1,   194,   347,   195,   435,   436,    -1,   452,    -1,   399,
      -1,   450,    -1,   451,    -1,   441,    -1,   443,    -1,   445,
     435,   437,    -1,   403,   158,   452,    -1,   447,   194,   289,
     195,    -1,   448,   194,   289,   195,    -1,   194,   445,   195,
      -1,   399,    -1,   450,    -1,   451,    -1,   441,    -1,   445,
     435,   436,    -1,   444,    -1,   447,   194,   289,   195,    -1,
     194,   445,   195,    -1,   452,    -1,   441,    -1,   399,    -1,
     365,    -1,   408,    -1,   194,   445,   195,    -1,   194,   347,
     195,    -1,   448,   194,   289,   195,    -1,   447,   194,   289,
     195,    -1,   194,   449,   195,    -1,   349,    -1,   352,    -1,
     445,   435,   439,   478,   194,   289,   195,    -1,   194,   334,
     195,   435,   439,   478,   194,   289,   195,    -1,   403,   158,
     213,   478,   194,   289,   195,    -1,   403,   158,   452,   194,
     289,   195,    -1,   403,   158,   197,   346,   198,   194,   289,
     195,    -1,   453,    -1,   456,   453,    -1,   453,    70,   455,
     201,    -1,   453,   197,   346,   198,    -1,   454,    -1,    83,
      -1,    84,    -1,   199,   197,   346,   198,    -1,   346,    -1,
      -1,   199,    -1,   456,   199,    -1,   452,    -1,   442,    -1,
     443,    -1,   457,   435,   437,    -1,   402,   158,   452,    -1,
     194,   445,   195,    -1,    -1,   442,    -1,   444,    -1,   457,
     435,   436,    -1,   194,   445,   195,    -1,   459,     9,    -1,
     459,     9,   445,    -1,   459,     9,   137,   194,   459,   195,
      -1,    -1,   445,    -1,   137,   194,   459,   195,    -1,   461,
     415,    -1,    -1,   461,     9,   346,   136,   346,    -1,   461,
       9,   346,    -1,   346,   136,   346,    -1,   346,    -1,   461,
       9,   346,   136,    38,   445,    -1,   461,     9,    38,   445,
      -1,   346,   136,    38,   445,    -1,    38,   445,    -1,   463,
     415,    -1,    -1,   463,     9,   346,   136,   346,    -1,   463,
       9,   346,    -1,   346,   136,   346,    -1,   346,    -1,   465,
     415,    -1,    -1,   465,     9,   409,   136,   409,    -1,   465,
       9,   409,    -1,   409,   136,   409,    -1,   409,    -1,   466,
     467,    -1,   466,    90,    -1,   467,    -1,    90,   467,    -1,
      83,    -1,    83,    70,   468,   201,    -1,    83,   435,   211,
      -1,   156,   346,   198,    -1,   156,    82,    70,   346,   201,
     198,    -1,   157,   445,   198,    -1,   211,    -1,    85,    -1,
      83,    -1,   127,   194,   336,   195,    -1,   128,   194,   445,
     195,    -1,   128,   194,   347,   195,    -1,   128,   194,   449,
     195,    -1,   128,   194,   448,   195,    -1,   128,   194,   334,
     195,    -1,     7,   346,    -1,     6,   346,    -1,     5,   194,
     346,   195,    -1,     4,   346,    -1,     3,   346,    -1,   445,
      -1,   470,     9,   445,    -1,   403,   158,   212,    -1,   403,
     158,   130,    -1,    -1,   102,   498,    -1,   184,   477,    14,
     498,   196,    -1,   433,   184,   477,    14,   498,   196,    -1,
     186,   477,   472,    14,   498,   196,    -1,   433,   186,   477,
     472,    14,   498,   196,    -1,   213,    -1,   498,   213,    -1,
     212,    -1,   498,   212,    -1,   213,    -1,   213,   179,   487,
     180,    -1,   211,    -1,   211,   179,   487,   180,    -1,   179,
     480,   180,    -1,    -1,   498,    -1,   479,     9,   498,    -1,
     479,   415,    -1,   479,     9,   172,    -1,   480,    -1,   172,
      -1,    -1,    -1,   193,   483,   416,    -1,   484,    -1,   483,
       9,   484,    -1,   498,    14,   498,    -1,   498,   486,    -1,
      -1,    32,   498,    -1,   102,   498,    -1,   103,   498,    -1,
     489,   415,    -1,   486,    -1,   488,   486,    -1,   489,     9,
     490,   211,    -1,   490,   211,    -1,   489,     9,   490,   211,
     488,    -1,   490,   211,   488,    -1,    50,    -1,    51,    -1,
      -1,    91,   136,   498,    -1,    31,    91,   136,   498,    -1,
     224,   158,   211,   136,   498,    -1,    31,   224,   158,   211,
     136,   498,    -1,   492,     9,   491,    -1,   491,    -1,   492,
       9,   172,    -1,   492,   415,    -1,   172,    -1,    -1,   183,
     194,   493,   195,    -1,   224,    -1,   211,   158,   496,    -1,
     211,   478,    -1,   179,   498,   415,   180,    -1,   179,   498,
       9,   498,   180,    -1,    31,   498,    -1,    59,   498,    -1,
     224,    -1,   138,    -1,   142,    -1,   494,    -1,   495,   158,
     496,    -1,   138,   497,    -1,   163,    -1,   194,   111,   194,
     481,   195,    32,   498,   195,    -1,   194,   498,     9,   479,
     415,   195,    -1,   498,    -1,    -1
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
    2180,  2182,  2186,  2190,  2191,  2195,  2197,  2201,  2203,  2207,
    2209,  2213,  2216,  2221,  2223,  2227,  2230,  2235,  2239,  2243,
    2247,  2251,  2255,  2259,  2263,  2267,  2271,  2273,  2277,  2279,
    2283,  2285,  2289,  2296,  2303,  2305,  2310,  2311,  2312,  2313,
    2314,  2315,  2316,  2318,  2319,  2323,  2324,  2325,  2326,  2330,
    2336,  2345,  2358,  2359,  2362,  2365,  2368,  2369,  2372,  2376,
    2379,  2382,  2389,  2390,  2394,  2395,  2397,  2402,  2403,  2404,
    2405,  2406,  2407,  2408,  2409,  2410,  2411,  2412,  2413,  2414,
    2415,  2416,  2417,  2418,  2419,  2420,  2421,  2422,  2423,  2424,
    2425,  2426,  2427,  2428,  2429,  2430,  2431,  2432,  2433,  2434,
    2435,  2436,  2437,  2438,  2439,  2440,  2441,  2442,  2443,  2444,
    2445,  2446,  2447,  2448,  2449,  2450,  2451,  2452,  2453,  2454,
    2455,  2456,  2457,  2458,  2459,  2460,  2461,  2462,  2463,  2464,
    2465,  2466,  2467,  2468,  2469,  2470,  2471,  2472,  2473,  2474,
    2475,  2476,  2477,  2478,  2479,  2480,  2481,  2482,  2486,  2491,
    2492,  2496,  2497,  2498,  2499,  2501,  2505,  2506,  2517,  2518,
    2520,  2522,  2534,  2535,  2536,  2540,  2541,  2542,  2546,  2547,
    2548,  2551,  2553,  2557,  2558,  2559,  2560,  2562,  2563,  2564,
    2565,  2566,  2567,  2568,  2569,  2570,  2571,  2574,  2579,  2580,
    2581,  2583,  2584,  2586,  2587,  2588,  2589,  2590,  2591,  2592,
    2594,  2596,  2598,  2600,  2602,  2603,  2604,  2605,  2606,  2607,
    2608,  2609,  2610,  2611,  2612,  2613,  2614,  2615,  2616,  2617,
    2618,  2620,  2622,  2624,  2626,  2627,  2630,  2631,  2635,  2639,
    2641,  2645,  2646,  2650,  2656,  2659,  2663,  2664,  2665,  2666,
    2667,  2668,  2669,  2674,  2676,  2680,  2681,  2684,  2685,  2689,
    2692,  2694,  2696,  2700,  2701,  2702,  2703,  2706,  2710,  2711,
    2712,  2713,  2717,  2719,  2726,  2727,  2728,  2729,  2734,  2735,
    2736,  2737,  2739,  2740,  2742,  2743,  2744,  2748,  2750,  2754,
    2756,  2759,  2762,  2764,  2766,  2769,  2771,  2775,  2777,  2780,
    2783,  2789,  2791,  2794,  2795,  2800,  2803,  2807,  2807,  2812,
    2815,  2816,  2820,  2821,  2825,  2826,  2827,  2831,  2833,  2841,
    2842,  2846,  2848,  2856,  2857,  2861,  2862,  2867,  2869,  2874,
    2885,  2899,  2911,  2926,  2927,  2928,  2929,  2930,  2931,  2932,
    2942,  2951,  2953,  2955,  2959,  2960,  2961,  2962,  2963,  2979,
    2980,  2982,  2991,  2992,  2993,  2994,  2995,  2996,  2997,  2998,
    3000,  3005,  3009,  3010,  3014,  3017,  3024,  3028,  3037,  3044,
    3046,  3052,  3054,  3055,  3059,  3060,  3061,  3068,  3069,  3074,
    3075,  3080,  3081,  3082,  3083,  3094,  3097,  3100,  3101,  3102,
    3103,  3114,  3118,  3119,  3120,  3122,  3123,  3124,  3128,  3130,
    3133,  3135,  3136,  3137,  3138,  3141,  3143,  3144,  3148,  3150,
    3153,  3155,  3156,  3157,  3161,  3163,  3166,  3169,  3171,  3173,
    3177,  3178,  3180,  3181,  3187,  3188,  3190,  3200,  3202,  3204,
    3207,  3208,  3209,  3213,  3214,  3215,  3216,  3217,  3218,  3219,
    3220,  3221,  3222,  3223,  3227,  3228,  3232,  3234,  3242,  3244,
    3248,  3252,  3257,  3261,  3269,  3270,  3274,  3275,  3281,  3282,
    3291,  3292,  3300,  3303,  3307,  3310,  3315,  3320,  3322,  3323,
    3324,  3327,  3329,  3335,  3336,  3340,  3341,  3345,  3346,  3350,
    3351,  3354,  3359,  3360,  3364,  3367,  3369,  3373,  3379,  3380,
    3381,  3385,  3389,  3397,  3402,  3414,  3416,  3420,  3423,  3425,
    3430,  3435,  3441,  3444,  3449,  3454,  3456,  3463,  3466,  3469,
    3470,  3473,  3476,  3477,  3482,  3484,  3488,  3494,  3504,  3505
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
     363,   363,   364,   365,   365,   366,   366,   367,   367,   368,
     368,   369,   369,   370,   370,   371,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   381,   382,   382,
     383,   383,   384,   385,   386,   386,   387,   387,   387,   387,
     387,   387,   387,   387,   387,   388,   388,   388,   388,   389,
     390,   390,   391,   391,   392,   392,   393,   393,   394,   395,
     395,   396,   396,   396,   397,   397,   397,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   399,   400,
     400,   401,   401,   401,   401,   401,   402,   402,   403,   403,
     403,   403,   404,   404,   404,   405,   405,   405,   406,   406,
     406,   407,   407,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   410,   410,   411,   412,   412,   413,   413,   413,   413,
     413,   413,   413,   414,   414,   415,   415,   416,   416,   417,
     417,   417,   417,   418,   418,   418,   418,   418,   419,   419,
     419,   419,   420,   420,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   422,   422,   423,
     423,   424,   424,   424,   424,   425,   425,   426,   426,   427,
     427,   428,   428,   429,   429,   430,   430,   432,   431,   433,
     434,   434,   435,   435,   436,   436,   436,   437,   437,   438,
     438,   439,   439,   440,   440,   441,   441,   442,   442,   443,
     443,   444,   444,   445,   445,   445,   445,   445,   445,   445,
     445,   445,   445,   445,   446,   446,   446,   446,   446,   446,
     446,   446,   447,   447,   447,   447,   447,   447,   447,   447,
     447,   448,   449,   449,   450,   450,   451,   451,   451,   452,
     452,   453,   453,   453,   454,   454,   454,   455,   455,   456,
     456,   457,   457,   457,   457,   457,   457,   458,   458,   458,
     458,   458,   459,   459,   459,   459,   459,   459,   460,   460,
     461,   461,   461,   461,   461,   461,   461,   461,   462,   462,
     463,   463,   463,   463,   464,   464,   465,   465,   465,   465,
     466,   466,   466,   466,   467,   467,   467,   467,   467,   467,
     468,   468,   468,   469,   469,   469,   469,   469,   469,   469,
     469,   469,   469,   469,   470,   470,   471,   471,   472,   472,
     473,   473,   473,   473,   474,   474,   475,   475,   476,   476,
     477,   477,   478,   478,   479,   479,   480,   481,   481,   481,
     481,   482,   482,   483,   483,   484,   484,   485,   485,   486,
     486,   487,   488,   488,   489,   489,   489,   489,   490,   490,
     490,   491,   491,   491,   491,   492,   492,   493,   493,   493,
     493,   494,   495,   496,   496,   497,   497,   498,   498,   498,
     498,   498,   498,   498,   498,   498,   498,   498,   499,   499
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
       2,     0,     4,     4,     3,     2,     0,     5,     3,     2,
       0,     5,     3,     2,     0,     5,     3,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     2,     0,     2,     0,
       2,     0,     4,     4,     4,     4,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     3,     4,     1,     2,     4,
       2,     6,     0,     1,     4,     0,     2,     0,     1,     1,
       3,     1,     3,     1,     1,     3,     3,     1,     1,     1,
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
       4,     3,     4,     1,     1,     1,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       4,     3,     1,     3,     3,     1,     1,     1,     1,     1,
       3,     3,     3,     2,     0,     1,     0,     1,     0,     5,
       3,     3,     1,     1,     1,     1,     3,     2,     1,     1,
       1,     1,     1,     3,     1,     1,     1,     3,     1,     2,
       2,     4,     3,     4,     1,     1,     1,     3,     1,     2,
       0,     5,     3,     3,     1,     3,     1,     2,     0,     5,
       3,     2,     0,     3,     0,     4,     2,     0,     3,     3,
       1,     0,     1,     1,     1,     1,     3,     1,     1,     1,
       3,     1,     1,     3,     3,     2,     4,     2,     4,     5,
       5,     5,     5,     1,     1,     1,     1,     1,     1,     3,
       3,     4,     4,     3,     1,     1,     1,     1,     3,     1,
       4,     3,     1,     1,     1,     1,     1,     3,     3,     4,
       4,     3,     1,     1,     7,     9,     7,     6,     8,     1,
       2,     4,     4,     1,     1,     1,     4,     1,     0,     1,
       2,     1,     1,     1,     3,     3,     3,     0,     1,     1,
       3,     3,     2,     3,     6,     0,     1,     4,     2,     0,
       5,     3,     3,     1,     6,     4,     4,     2,     2,     0,
       5,     3,     3,     1,     2,     0,     5,     3,     3,     1,
       2,     2,     1,     2,     1,     4,     3,     3,     6,     3,
       1,     1,     1,     4,     4,     4,     4,     4,     4,     2,
       2,     4,     2,     2,     1,     3,     3,     3,     0,     2,
       5,     6,     6,     7,     1,     2,     1,     2,     1,     4,
       1,     4,     3,     0,     1,     3,     2,     3,     1,     1,
       0,     0,     3,     1,     3,     3,     2,     0,     2,     2,
       2,     2,     1,     2,     4,     2,     5,     3,     1,     1,
       0,     3,     4,     5,     6,     3,     1,     3,     2,     1,
       0,     4,     1,     3,     2,     4,     5,     2,     2,     1,
       1,     1,     1,     3,     2,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   432,     0,     0,   847,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   939,
       0,   927,   717,     0,   723,   724,   725,    25,   787,   914,
     915,   156,   157,   726,     0,   137,     0,     0,     0,     0,
      26,     0,     0,     0,     0,   191,     0,     0,     0,     0,
       0,     0,   398,   399,   400,   403,   402,   401,     0,     0,
       0,     0,   220,     0,     0,     0,    33,    34,    35,   730,
     732,   733,   727,   728,     0,     0,     0,   734,   729,     0,
     700,    28,    29,    30,    32,    31,     0,   731,     0,     0,
       0,     0,   735,   404,   537,    27,     0,   155,   127,   919,
     718,     0,     0,     4,   117,   119,   786,     0,   699,     0,
       6,   190,     7,     9,     8,    10,     0,     0,   396,   445,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   443,
     902,   903,   519,   515,   516,   517,   518,   426,   522,     0,
     425,   874,   701,   708,     0,   789,   514,   395,   877,   878,
     889,   444,     0,     0,   447,   446,   875,   876,   873,   909,
     913,     0,   504,   788,    11,   403,   402,   401,     0,     0,
      32,     0,   117,   190,     0,   983,   444,   982,     0,   980,
     979,   521,     0,   433,   440,   438,     0,     0,   486,   487,
     488,   489,   513,   511,   510,   509,   508,   507,   506,   505,
      25,   914,   726,   703,    33,    34,    35,     0,     0,  1003,
     895,   701,     0,   702,   467,     0,   465,     0,   943,     0,
     796,   424,   713,   210,     0,  1003,   423,   712,   706,     0,
     722,   702,   922,   923,   929,   921,   714,     0,     0,   716,
     512,     0,     0,     0,     0,   429,     0,   135,   431,     0,
       0,   141,   143,     0,     0,   145,     0,    76,    75,    70,
      69,    61,    62,    53,    73,    84,    85,     0,    56,     0,
      68,    60,    66,    87,    79,    78,    51,    74,    94,    95,
      52,    90,    49,    91,    50,    92,    48,    96,    83,    88,
      93,    80,    81,    55,    82,    86,    47,    77,    63,    97,
      71,    64,    54,    46,    45,    44,    43,    42,    41,    65,
      99,    98,   101,    58,    39,    40,    67,  1050,  1051,    59,
    1055,    38,    57,    89,     0,     0,   117,   100,   994,  1049,
       0,  1052,     0,     0,   147,     0,     0,     0,   181,     0,
       0,     0,     0,     0,     0,   798,     0,   105,   107,   309,
       0,     0,   308,     0,   224,     0,   221,   314,     0,     0,
       0,     0,     0,  1000,   206,   218,   935,   939,   556,   577,
     577,     0,   964,     0,   737,     0,     0,     0,   962,     0,
      16,     0,   121,   198,   212,   219,   605,   549,     0,   988,
     529,   531,   533,   851,   432,   445,     0,     0,   443,   444,
     446,     0,     0,   719,     0,   720,     0,     0,     0,   180,
       0,     0,   123,   300,     0,    24,   189,     0,   217,   202,
     216,   401,   404,   190,   397,   170,   171,   172,   173,   174,
     176,   177,   179,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   927,     0,   169,   918,   918,   949,     0,     0,     0,
       0,     0,     0,     0,     0,   394,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   466,
     464,   852,   853,     0,   918,     0,   865,   300,   300,   918,
       0,   920,   910,   935,     0,   190,     0,     0,   149,     0,
     849,   844,   796,     0,   445,   443,     0,   947,     0,   554,
     795,   938,   722,   445,   443,   444,   123,     0,   300,   422,
       0,   867,   715,     0,   127,   260,     0,   536,     0,   152,
       0,     0,   430,     0,     0,     0,     0,     0,   144,   168,
     146,  1050,  1051,  1047,  1048,     0,  1054,  1040,     0,     0,
       0,     0,    72,    37,    59,    36,   995,   175,   178,   148,
     127,     0,   165,   167,     0,     0,     0,     0,   108,     0,
     797,   106,    18,     0,   102,     0,   310,     0,   150,   223,
     222,     0,     0,   151,   984,     0,     0,   445,   443,   444,
     447,   446,     0,  1030,   230,     0,   936,     0,     0,     0,
       0,   796,   796,     0,     0,   153,     0,     0,   736,   963,
     787,     0,     0,   961,   792,   960,   120,     5,    13,    14,
       0,   228,     0,     0,   542,     0,     0,     0,   796,     0,
     710,     0,   709,   704,   543,     0,     0,     0,     0,   851,
     127,     0,   798,   850,  1059,   421,   435,   500,   883,   901,
     132,   126,   128,   129,   130,   131,   395,     0,   520,   790,
     791,   118,   796,     0,  1004,     0,     0,     0,   798,   301,
       0,   525,   192,   226,     0,   470,   472,   471,   483,     0,
       0,   503,   468,   469,   473,   475,   474,   491,   490,   493,
     492,   494,   496,   498,   497,   495,   485,   484,   477,   478,
     476,   479,   480,   482,   499,   481,   917,     0,     0,   953,
       0,   796,   987,     0,   986,  1003,   880,   909,   208,   200,
     214,     0,   988,   204,   190,     0,   436,   439,   441,   449,
     463,   462,   461,   460,   459,   458,   457,   456,   455,   454,
     453,   452,   855,     0,   854,   857,   879,   861,  1003,   858,
       0,     0,     0,     0,     0,     0,     0,     0,   981,   434,
     842,   846,   795,   848,     0,   705,     0,   942,     0,   941,
     226,     0,   705,   926,   925,     0,     0,   854,   857,   924,
     858,   427,   262,   264,   127,   540,   539,   428,     0,   127,
     244,   136,   431,     0,     0,     0,     0,     0,   256,   256,
     142,   796,     0,     0,  1039,     0,  1036,   796,     0,  1010,
       0,     0,     0,     0,     0,   794,     0,    33,    34,    35,
       0,     0,   739,   743,   744,   745,   747,     0,   738,   125,
     785,   746,  1003,  1053,     0,     0,     0,     0,    19,     0,
      20,     0,   103,     0,     0,     0,   114,   798,     0,   112,
     107,   104,   109,     0,   307,   315,   312,     0,     0,   973,
     978,   975,   974,   977,   976,    12,  1028,  1029,     0,   796,
       0,     0,     0,   935,   932,     0,   553,     0,   567,   795,
     555,   795,   576,   570,   573,   972,   971,   970,     0,   966,
       0,   967,   969,     0,     5,     0,     0,     0,   599,   600,
     608,   607,     0,   443,     0,   795,   548,   552,     0,     0,
     989,     0,   530,     0,     0,  1017,   851,   286,  1058,     0,
       0,   866,     0,   916,   795,  1006,  1002,   302,   303,   698,
     797,   299,     0,   851,     0,     0,   228,   527,   194,   502,
       0,   584,   585,     0,   582,   795,   948,     0,     0,   300,
     230,     0,   228,     0,     0,   226,     0,   927,   450,     0,
       0,   863,   864,   881,   882,   911,   912,     0,     0,     0,
     830,   803,   804,   805,   812,     0,    33,    34,    35,     0,
       0,   818,   824,   825,   826,     0,   816,   814,   815,   836,
     796,     0,   844,   946,   945,     0,   228,     0,   868,   721,
       0,   266,     0,     0,   133,     0,     0,     0,     0,     0,
       0,     0,   236,   237,   248,     0,   127,   246,   162,   256,
       0,   256,     0,   795,     0,     0,     0,     0,     0,   795,
    1038,  1041,  1009,   796,  1008,     0,   796,   768,   769,   766,
     767,   802,     0,   796,   794,   560,   579,   579,   551,     0,
       0,   955,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1044,   182,     0,   185,   166,     0,     0,   110,   115,   116,
     108,   797,   113,     0,   311,     0,   985,   154,  1001,  1030,
    1021,  1025,   229,   231,   321,     0,     0,   933,     0,   558,
       0,   965,     0,    17,     0,   988,   227,   321,     0,     0,
     705,   545,     0,   711,   990,     0,  1017,   534,     0,     0,
    1059,     0,   291,   289,   857,   869,  1003,   857,   870,  1005,
       0,     0,   304,   124,     0,   851,   225,     0,   851,     0,
     501,   952,   951,     0,   300,     0,     0,     0,     0,     0,
       0,   228,   196,   722,   856,   300,     0,   808,   809,   810,
     811,   819,   820,   834,     0,   796,     0,   830,   564,   581,
     581,     0,   807,   838,     0,   795,   841,   843,   845,     0,
     940,     0,   856,     0,     0,     0,     0,   263,   541,   138,
       0,   431,   236,   238,   935,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   250,     0,  1045,     0,     0,  1031,
       0,  1037,  1035,   795,     0,     0,     0,   741,   795,   793,
       0,     0,   796,     0,     0,   782,   796,     0,     0,   796,
       0,   748,   783,   784,   959,     0,   796,   751,   753,   752,
       0,     0,   749,   750,   754,   756,   755,   771,   770,   773,
     772,   774,   776,   778,   777,   775,   764,   763,   758,   759,
     757,   760,   761,   762,   765,  1043,     0,   127,     0,     0,
     111,    21,   313,     0,     0,     0,  1022,  1027,     0,   395,
     937,   935,   437,   442,   448,     0,     0,    15,     0,   395,
     611,     0,     0,   613,   606,   609,     0,   604,     0,   992,
       0,  1018,   538,     0,   292,     0,     0,   287,     0,   306,
     305,  1017,     0,   321,     0,   851,     0,   300,     0,   907,
     321,   988,   321,   991,     0,     0,     0,   451,     0,     0,
     822,   795,   829,   813,     0,     0,   796,     0,     0,   828,
     796,     0,   806,     0,     0,   796,   817,   835,   944,   321,
       0,   127,     0,   259,   245,     0,     0,     0,   235,   158,
     249,     0,     0,   252,     0,   257,   258,   127,   251,  1046,
    1032,     0,     0,  1007,     0,  1057,   801,   800,   740,   568,
     795,   559,     0,   571,   795,   578,   574,     0,   795,   550,
     742,     0,   583,   795,   954,   780,     0,     0,     0,    22,
      23,  1024,  1019,  1020,  1023,   232,     0,     0,     0,   402,
     393,     0,     0,     0,   207,   320,   322,     0,   392,     0,
       0,     0,   988,   395,     0,   557,   968,   317,   213,   602,
       0,     0,   544,   532,     0,   295,   285,     0,   288,   294,
     300,   524,  1017,   395,  1017,     0,   950,     0,   906,   395,
       0,   395,   993,   321,   851,   904,   833,   832,   821,   569,
     795,   563,     0,   572,   795,   580,   575,     0,   823,   795,
     837,   395,   127,   265,   134,   139,   160,   239,     0,   247,
     253,   127,   255,     0,  1033,     0,     0,     0,   562,   781,
     547,     0,   958,   957,   779,   127,   186,  1026,     0,     0,
       0,   996,     0,     0,     0,   233,     0,   988,     0,   358,
     354,   360,   700,    32,     0,   348,     0,   353,   357,   370,
       0,   368,   373,     0,   372,     0,   371,     0,   190,   324,
       0,   326,     0,   327,   328,     0,     0,   934,     0,   603,
     601,   612,   610,   296,     0,     0,   283,   293,     0,     0,
    1017,     0,   203,   524,  1017,   908,   209,   317,   215,   395,
       0,     0,     0,   566,   827,   840,     0,   211,   261,     0,
       0,   127,   242,   159,   254,  1034,  1056,   799,     0,     0,
       0,     0,     0,     0,   420,     0,   997,     0,   338,   342,
     417,   418,   352,     0,     0,     0,   333,   664,   663,   660,
     662,   661,   681,   683,   682,   652,   622,   624,   623,   642,
     658,   657,   618,   629,   630,   632,   631,   651,   635,   633,
     634,   636,   637,   638,   639,   640,   641,   643,   644,   645,
     646,   647,   648,   650,   649,   619,   620,   621,   625,   626,
     628,   666,   667,   676,   675,   674,   673,   672,   671,   659,
     678,   668,   669,   670,   653,   654,   655,   656,   679,   680,
     684,   686,   685,   687,   688,   665,   690,   689,   692,   694,
     693,   627,   697,   695,   696,   691,   677,   617,   365,   614,
       0,   334,   386,   387,   385,   378,     0,   379,   335,   412,
       0,     0,     0,     0,   416,     0,   190,   199,   316,     0,
       0,     0,   284,   298,   905,     0,     0,   388,   127,   193,
    1017,     0,     0,   205,  1017,   831,     0,     0,   127,   240,
     140,   161,     0,   561,   546,   956,   184,   336,   337,   415,
     234,     0,   796,   796,     0,   361,   349,     0,     0,     0,
     367,   369,     0,     0,   374,   381,   382,   380,     0,     0,
     323,   998,     0,     0,     0,   419,     0,   318,     0,   297,
       0,   597,   798,   127,     0,     0,   195,   201,     0,   565,
     839,     0,     0,   163,   339,   117,     0,   340,   341,     0,
     795,     0,   795,   363,   359,   364,   615,   616,     0,   350,
     383,   384,   376,   377,   375,   413,   410,  1030,   329,   325,
     414,     0,   319,   598,   797,     0,     0,   389,   127,   197,
       0,   243,     0,   188,     0,   395,     0,   355,   362,   366,
       0,     0,   851,   331,     0,   595,   523,   526,     0,   241,
       0,     0,   164,   346,     0,   394,   356,   411,   999,     0,
     798,   406,   851,   596,   528,     0,   187,     0,     0,   345,
    1017,   851,   270,   407,   408,   409,  1059,   405,     0,     0,
       0,   344,  1011,   406,     0,  1017,     0,   343,     0,     0,
    1059,     0,   275,   273,  1011,   127,   798,  1013,     0,   390,
     127,   330,     0,   276,     0,     0,   271,     0,     0,   797,
    1012,     0,  1016,     0,     0,   279,   269,     0,   272,   278,
     332,   183,  1014,  1015,   391,   280,     0,     0,   267,   277,
       0,   268,   282,   281
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   113,   914,   637,   182,  1531,   735,
     354,   355,   356,   357,   867,   868,   869,   115,   116,   117,
     118,   119,   411,   671,   672,   551,   256,  1599,   557,  1508,
    1600,  1843,   856,   349,   580,  1803,  1104,  1297,  1862,   427,
     183,   673,   954,  1169,  1356,   123,   640,   971,   674,   693,
     975,   614,   970,   236,   532,   675,   641,   972,   429,   374,
     394,   126,   956,   917,   892,  1122,  1534,  1226,  1032,  1750,
    1603,   811,  1038,   556,   820,  1040,  1397,   803,  1021,  1024,
    1215,  1869,  1870,   661,   662,   687,   688,   361,   362,   368,
    1568,  1728,  1729,  1309,  1445,  1557,  1722,  1852,  1872,  1761,
    1807,  1808,  1809,  1544,  1545,  1546,  1547,  1763,  1764,  1770,
    1819,  1550,  1551,  1555,  1715,  1716,  1717,  1739,  1911,  1446,
    1447,   184,   128,  1886,  1887,  1720,  1449,  1450,  1451,  1452,
     129,   249,   552,   553,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,  1580,   140,   953,  1168,   141,   658,
     659,   660,   253,   403,   547,   647,   648,  1259,   649,  1260,
     142,   143,   620,   621,  1251,  1252,  1365,  1366,   144,   843,
    1002,   145,   844,  1003,   146,   845,  1004,   623,  1254,  1368,
     147,   846,   148,   149,  1792,   150,   642,  1570,   643,  1138,
     922,  1327,  1324,  1708,  1709,   151,   152,   153,   239,   154,
     240,   250,   414,   539,   155,  1061,  1256,   850,   851,   156,
    1062,   945,   591,  1063,  1007,  1191,  1008,  1193,  1370,  1194,
    1195,  1010,  1374,  1375,  1011,   781,   522,   196,   197,   676,
     664,   503,  1154,  1155,   767,   768,   941,   158,   242,   159,
     160,   186,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   727,   171,   246,   247,   617,   229,   230,   730,   731,
    1265,  1266,   387,   388,   908,   172,   605,   173,   657,   174,
     340,  1730,  1782,   375,   422,   682,   683,  1055,  1899,  1906,
    1907,  1149,  1306,   888,  1307,   889,   890,   826,   827,   828,
     341,   342,   853,   566,  1533,   939
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1247
static const yytype_int16 yypact[] =
{
   -1247,   181, -1247, -1247,  5541, 13581, 13581,   -43, 13581, 13581,
   13581, 11370, 13581, 13581, -1247, 13581, 13581, 13581, 13581, 13581,
   13581, 13581, 13581, 13581, 13581, 13581, 13581, 16599, 16599, 11571,
   13581, 17263,   -16,     4, -1247, -1247, -1247,   180, -1247,   370,
   -1247, -1247, -1247,   332, 13581, -1247,     4,   194,   237,   282,
   -1247,     4, 11772,  5417, 11973, -1247, 14449, 10365,    -9, 13581,
    4357,    59, -1247, -1247, -1247,   297,   268,   103,   312,   340,
     344,   362, -1247,  5417,   378,   386,   439,   456,   458, -1247,
   -1247, -1247, -1247, -1247, 13581,   540,  2939, -1247, -1247,  5417,
   -1247, -1247, -1247, -1247,  5417, -1247,  5417, -1247,   382,   392,
    5417,  5417, -1247,   233, -1247, -1247, 12174, -1247, -1247,   391,
     411,   470,   470, -1247,   583,   459,   464,   441, -1247,    89,
   -1247,   580, -1247, -1247, -1247, -1247,  2576,   739, -1247, -1247,
     443,   448,   451,   467,   472,   478,   525,   527, 15617, -1247,
   -1247, -1247, -1247,   175,   617,   636,   664, -1247,   670,   672,
   -1247,   140,   564, -1247,   497,   177, -1247,  1618,    50, -1247,
   -1247,  1265,   129,   570,   193, -1247,   131,   141,   576,   171,
   -1247,    66, -1247,   698, -1247, -1247, -1247,   650,   587,   652,
   -1247, 13581, -1247,   580,   739, 17677,  2710, 17677, 13581, 17677,
   17677, 14973,   616, 16766, 14973, 17677,   776,  5417,   757,   757,
     137,   757,   757,   757,   757,   757,   757,   757,   757,   757,
   -1247, -1247, -1247, -1247, -1247, -1247, -1247,    48, 13581,   649,
   -1247, -1247,   671,   639,   537,   641,   537, 16599, 16814,   629,
     832, -1247,   650, -1247, 13581,   649, -1247,   684, -1247,   688,
     657, -1247,   144, -1247, -1247, -1247,   537,   129, 12375, -1247,
   -1247, 13581,  8958,   844,    91, 17677,  9963, -1247, 13581, 13581,
    5417, -1247, -1247, 15665,   659, -1247, 15713, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247, -1247, -1247, -1247,  4260, -1247,  4260,
   -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247, -1247, -1247, -1247,    98,    83,   652,
   -1247, -1247, -1247, -1247,   662,  3135,    97, -1247, -1247,   699,
     850, -1247,   707, 15160, -1247,   675,   682, 15781, -1247,    75,
   15829,  3450,  3450,  5417,   669,   859,   689, -1247,   230, -1247,
   16195,   100, -1247,   750, -1247,   758, -1247,   855,   104, 16599,
   13581, 13581,   694,   712, -1247, -1247, 16296, 11571, 13581, 13581,
   13581,   105,    84,   510, -1247, 13782, 16599,   593, -1247,  5417,
   -1247,   334,   459, -1247, -1247, -1247, -1247, 17361,   879,   793,
   -1247, -1247, -1247,    99, 13581,   702,   704, 17677,   709,   148,
     715,  5742, 13581,   394,   713,   574,   394,   449,   318, -1247,
    5417,  4260,   706, 10566, 14449, -1247, -1247,  2949, -1247, -1247,
   -1247, -1247, -1247,   580, -1247, -1247, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, 13581, 13581, 13581, 13581, 12576, 13581, 13581,
   13581, 13581, 13581, 13581, 13581, 13581, 13581, 13581, 13581, 13581,
   13581, 13581, 13581, 13581, 13581, 13581, 13581, 13581, 13581, 13581,
   13581, 17459, 13581, -1247, 13581, 13581, 13581,  4903,  5417,  5417,
    5417,  5417,  5417,  2576,   801,   868, 10164, 13581, 13581, 13581,
   13581, 13581, 13581, 13581, 13581, 13581, 13581, 13581, 13581, -1247,
   -1247, -1247, -1247,  2521, 13581, 13581, -1247, 10566, 10566, 13581,
   13581,   391,   150, 16296,   723,   580, 12777, 15877, -1247, 13581,
   -1247,   726,   912,   771,   732,   735, 13951,   537, 12978, -1247,
   13179, -1247,   657,   736,   738,  1750, -1247,   143, 10566, -1247,
    3355, -1247, -1247, 15945, -1247, -1247, 10767, -1247, 13581, -1247,
     840,  9159,   926,   751, 13766,   935,    76,    62, -1247, -1247,
   -1247,   773, -1247, -1247, -1247,  4260, -1247,  2308,   756,   944,
   16094,  5417, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247,
   -1247,   760, -1247, -1247,   765,   770,   778,   781,   254,  4389,
    4949, -1247, -1247,  5417,  5417, 13581,   537,    59, -1247, -1247,
   -1247, 16094,   884, -1247,   537,   122,   123,   784,   785,  2573,
     187,   788,   797,   173,   863,   802,   537,   125,   804, 16870,
     796,   996,   997,   806,   808, -1247,  4399,  5417, -1247, -1247,
     941,  2964,   440, -1247, -1247, -1247,   459, -1247, -1247, -1247,
     983,   883,   841,    92,   862, 13581,   391,   885,  1013,   830,
   -1247,   871, -1247,   150, -1247,  4260,  4260,  1016,   844,    99,
   -1247,   842,  1025, -1247,  4260,   269, -1247,   479,   139, -1247,
   -1247, -1247, -1247, -1247, -1247, -1247,   753,  3368, -1247, -1247,
   -1247, -1247,  1027,   858, -1247, 16599, 13581,   845,  1032, 17677,
    1029, -1247, -1247,   914,  3369, 11958, 17815, 14973, 14274, 13581,
   17629, 14448, 11550, 12956, 13357, 12552, 14791, 16361, 16361, 16361,
   16361,  3012,  3012,  3012,  3012,  3012,  1083,  1083,   763,   763,
     763,   137,   137,   137, -1247,   757, 17677,   843,   846, 16918,
     857,  1039,    10, 13581,   200,   649,   360,   150, -1247, -1247,
   -1247,  1042,   793, -1247,   580, 16397, -1247, -1247, -1247, 14973,
   14973, 14973, 14973, 14973, 14973, 14973, 14973, 14973, 14973, 14973,
   14973, 14973, -1247, 13581,   383,   151, -1247, -1247,   649,   437,
     852,  3928,   864,   872,   867,  4037,   127,   875, -1247, 17677,
    3959, -1247,  5417, -1247,   269,    18, 16599, 17677, 16599, 16974,
     914,   269,   537,   163,   915,   877, 13581, -1247,   165, -1247,
   -1247, -1247,  8757,   623, -1247, -1247, 17677, 17677,     4, -1247,
   -1247, -1247, 13581,   968,  4730, 16094,  5417,  9360,   881,   887,
   -1247,  1070,  4666,   950, -1247,   922, -1247,  1078,   893,  3575,
    4260, 16094, 16094, 16094, 16094, 16094,   895,  1020,  1022,  1030,
     903, 16094,   235, -1247, -1247, -1247, -1247,   203, -1247, 17771,
   -1247, -1247,    21, -1247,  5943,  4135,   902,  4949, -1247,  4949,
   -1247,  5417,  5417,  4949,  4949,  5417, -1247,  1094,   907, -1247,
     257, -1247, -1247,  4214, -1247, 17771,  1092, 16599,   913, -1247,
   -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247,   934,  1111,
    5417,  4135,   928, 16296, 16498,  1113, -1247, 13581, -1247, 13581,
   -1247, 13581, -1247, -1247, -1247, -1247, -1247, -1247,   927, -1247,
   13581, -1247, -1247,  5119, -1247,  4260,  4135,   932, -1247, -1247,
   -1247, -1247,  1116,   937, 13581, 17361, -1247, -1247,  4903,   946,
   -1247,  4260, -1247,   952,  6144,  1121,    69, -1247, -1247,   119,
    2521, -1247,  3355, -1247,  4260, -1247, -1247,   537, 17677, -1247,
   10968, -1247, 16094,    72,   960,  4135,   883, -1247, -1247, 14448,
   13581, -1247, -1247, 13581, -1247, 13581, -1247,  4641,   966, 10566,
     863,  1129,   883,  4260,  1152,   914,  5417, 17459,   537,  4689,
     973, -1247, -1247,   161,   974, -1247, -1247,  1156,  1472,  1472,
    3959, -1247, -1247, -1247,  1119,   978,  1103,  1104,  1105,    85,
     984,   531, -1247, -1247, -1247,  1019, -1247, -1247, -1247, -1247,
    1170,   987,   726,   537,   537, 13380,   883,  3355, -1247, -1247,
   11354,   627,     4,  9963, -1247,  6345,   988,  6546,   990,  4730,
   16599,   989,  1051,   537, 17771,  1174, -1247, -1247, -1247, -1247,
     673, -1247,   299,  4260,  1009,  1054,  1033,  4260,  5417,  2745,
   -1247, -1247, -1247,  1183, -1247,   999,  1027,   700,   700,  1128,
    1128, 16046,  1002,  1189, 16094, 16094, 16094, 16094, 17361,  2479,
   15305, 16094, 16094, 16094, 16094, 15973, 16094, 16094, 16094, 16094,
   16094, 16094, 16094, 16094, 16094, 16094, 16094, 16094, 16094, 16094,
   16094, 16094, 16094, 16094, 16094, 16094, 16094, 16094, 16094,  5417,
   -1247, -1247,  1122, -1247, -1247,  1006,  1008, -1247, -1247, -1247,
     275,  4389, -1247,  1011, -1247, 16094,   537, -1247, -1247,    94,
   -1247,   609,  1199, -1247, -1247,   128,  1015,   537, 11169, 17677,
   17022, -1247,  2382, -1247,  5340,   793,  1199, -1247,   405,   212,
   -1247, 17677,  1074,  1017, -1247,  1018,  1121, -1247,  4260,   844,
    4260,    45,  1198,  1130,   166, -1247,   649,   167, -1247, -1247,
   16599, 13581, 17677, 17771,  1024,    72, -1247,  1023,    72,  1028,
   14448, 17677, 17078,  1031, 10566,  1026,  1034,  4260,  1035,  1021,
    4260,   883, -1247,   657,   481, 10566, 13581, -1247, -1247, -1247,
   -1247, -1247, -1247,  1087,  1036,  1215,  1137,  3959,  3959,  3959,
    3959,  1079, -1247, 17361,    77,  3959, -1247, -1247, -1247, 16599,
   17677,  1038, -1247,     4,  1197,  1159,  9963, -1247, -1247, -1247,
    1044, 13581,  1051,   537, 16296,  4730,  1043, 16094,  6747,   687,
    1046, 13581,    70,   315, -1247,  1063, -1247,  4260,  5417, -1247,
    1109, -1247, -1247,  3622,  1214,  1053, 16094, -1247, 16094, -1247,
    1056,  1057,  1243, 16167,  1058, 17771,  1246,  1059,  1125,  1253,
    1068, -1247, -1247, -1247, 17126,  1067,  1257, 15157, 17859, 10546,
   16094, 17725, 12756, 13157, 13949, 14617, 16462, 16563, 16563, 16563,
   16563,  3307,  3307,  3307,  3307,  3307,  1096,  1096,   700,   700,
     700,  1128,  1128,  1128,  1128, -1247,  1072, -1247,  1073,  1076,
   -1247, -1247, 17771,  5417,  4260,  4260, -1247,   609,  4135,  1322,
   -1247, 16296, -1247, -1247, 14973, 13581,  1077, -1247,  1081,  1538,
   -1247,    93, 13581, -1247, -1247, -1247, 13581, -1247, 13581, -1247,
     844, -1247, -1247,   147,  1256,  1191, 13581, -1247,  1082,   537,
   17677,  1121,  1097, -1247,  1100,    72, 13581, 10566,  1102, -1247,
   -1247,   793, -1247, -1247,  1106,  1101,  1107, -1247,  1108,  3959,
   -1247,  3959, -1247, -1247,  1110,  1098,  1291,  1168,  1112, -1247,
    1297,  1114, -1247,  1173,  1117,  1302, -1247, -1247,   537, -1247,
    1282, -1247,  1120, -1247, -1247,  1123,  1127,   130, -1247, -1247,
   17771,  1124,  1131, -1247, 13565, -1247, -1247, -1247, -1247, -1247,
   -1247,  1188,  4260, -1247,  4260, -1247, 17771, 17181, -1247, -1247,
   16094, -1247, 16094, -1247, 16094, -1247, -1247, 16094, 17361, -1247,
   -1247, 16094, -1247, 16094, -1247, 10948, 16094,  1132,  6948, -1247,
   -1247,   609, -1247, -1247, -1247, -1247,   585, 14623,  4135,  1213,
   -1247,   597,  1161,  1505, -1247, -1247, -1247,   801,  3064,   106,
     107,  1139,   793,   868,   132, 17677, -1247, -1247, -1247,  1175,
   11756, 12359, 17677, -1247,   101,  1323,  1260, 13581, -1247, 17677,
   10566,  1219,  1121,  1710,  1121,  1145, 17677,  1151, -1247,  1948,
    1150,  2006, -1247, -1247,    72, -1247, -1247,  1218, -1247, -1247,
    3959, -1247,  3959, -1247,  3959, -1247, -1247,  3959, -1247, 17361,
   -1247,  2149, -1247,  8757, -1247, -1247, -1247, -1247,  9561, -1247,
   -1247, -1247,  8757,  4260, -1247,  1154, 16094, 17229, 17771, 17771,
   17771,  1220, 17771, 17284, 10948, -1247, -1247,   609,  4135,  4135,
    5417, -1247,  1341, 15450,    79, -1247, 14623,   793,  4227, -1247,
    1178, -1247,   108,  1162,   110, -1247, 14972, -1247, -1247, -1247,
     115, -1247, -1247,  1289, -1247,  1165, -1247,  1275,   580, -1247,
   14798, -1247, 14798, -1247, -1247,  1348,   801, -1247, 14101, -1247,
   -1247, -1247, -1247,  1349,  1283, 13581, -1247, 17677,  1172,  1171,
    1121,   529, -1247,  1219,  1121, -1247, -1247, -1247, -1247,  2177,
    1177,  3959,  1239, -1247, -1247, -1247,  1241, -1247,  8757,  9762,
    9561, -1247, -1247, -1247,  8757, -1247, -1247, 17771, 16094, 16094,
   16094,  7149,  1182,  1184, -1247, 16094, -1247,  4135, -1247, -1247,
   -1247, -1247, -1247,  4260,   693,   597, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247,   618, -1247,
    1161, -1247, -1247, -1247, -1247, -1247,   120,   528, -1247,  1365,
     116, 15160,  1275,  1369, -1247,  4260,   580, -1247, -1247,  1192,
    1371, 13581, -1247, 17677, -1247,   142,  1190, -1247, -1247, -1247,
    1121,   529, 14275, -1247,  1121, -1247,  3959,  3959, -1247, -1247,
   -1247, -1247,  7350, 17771, 17771, 17771, -1247, -1247, -1247, 17771,
   -1247,  1800,  1380,  1382,  1195, -1247, -1247, 16094, 14972, 14972,
    1335, -1247,  1289,  1289,   654, -1247, -1247, -1247, 16094,  1312,
   -1247,  1217,  1204,   117, 16094, -1247,  5417, -1247, 16094, 17677,
    1318, -1247,  1393, -1247,  7551,  1206, -1247, -1247,   529, -1247,
   -1247,  7752,  1209,  1292, -1247,  1307,  1254, -1247, -1247,  1311,
    4260,  1234,   693, -1247, -1247, 17771, -1247, -1247,  1245, -1247,
    1381, -1247, -1247, -1247, -1247, 17771,  1402,   173, -1247, -1247,
   17771,  1224, 17771, -1247,   351,  1225,  7953, -1247, -1247, -1247,
    1226, -1247,  1227,  1247,  5417,   868,  1244, -1247, -1247, -1247,
   16094,  1252,    73, -1247,  1343, -1247, -1247, -1247,  8154, -1247,
    4135,   902, -1247,  1262,  5417,   488, -1247, 17771, -1247,  1232,
    1427,   676,    73, -1247, -1247,  1354, -1247,  4135,  1242, -1247,
    1121,    74, -1247, -1247, -1247, -1247,  4260, -1247,  1266,  1268,
     118, -1247,  1255,   676,   149,  1121,  1267, -1247,  4260,   548,
    4260,   263,  1452,  1385,  1255, -1247,  1460, -1247,   146, -1247,
   -1247, -1247,   159,  1456,  1388, 13581, -1247,   548,  8355,  4260,
   -1247,  4260, -1247,  8556,   300,  1458,  1390, 13581, -1247, 17677,
   -1247, -1247, -1247, -1247, -1247,  1461,  1391, 13581, -1247, 17677,
   13581, -1247, 17677, 17677
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1247, -1247, -1247,  -561, -1247, -1247, -1247,   491,     1,   -54,
     424, -1247,  -256,  -512, -1247, -1247,   365,    44,  1723, -1247,
    1807, -1247,  -499, -1247,    28, -1247, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247, -1247, -1247,  -377, -1247, -1247,  -161,
      68,    23, -1247, -1247, -1247, -1247, -1247, -1247,    24, -1247,
   -1247, -1247, -1247, -1247, -1247,    30, -1247, -1247,  1003,  1007,
    1012,   -87,  -697,  -873,   519,   578,  -382,   277,  -939, -1247,
    -103, -1247, -1247, -1247, -1247,  -732,   109, -1247, -1247, -1247,
   -1247,  -371, -1247,  -568, -1247,  -433, -1247, -1247,   905, -1247,
     -82, -1247, -1247, -1055, -1247, -1247, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247,  -116, -1247,   -28, -1247, -1247, -1247,
   -1247, -1247,  -198, -1247,    78,  -986, -1247, -1225,  -400, -1247,
    -134,    25,  -118,  -370, -1247,  -197, -1247, -1247, -1247,    88,
     -26,     2,   178,  -748,   -62, -1247, -1247,    29, -1247,    -6,
   -1247, -1247,    -5,   -41,   -59, -1247, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247,  -603,  -863, -1247, -1247, -1247, -1247,
   -1247,  1000, -1247, -1247, -1247, -1247, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247, -1247, -1247, -1247,  1149,   463,   331,
   -1247, -1247, -1247, -1247, -1247,   395, -1247, -1247, -1247, -1247,
   -1247, -1247, -1247, -1247,  -969,  -360,  2796,    27, -1247,   663,
    -411, -1247, -1247,  -491,  3547,  3094, -1247,  -396, -1247, -1247,
     468,    -2,  -629, -1247, -1247,   546,   349,  -546, -1247,   350,
   -1247, -1247, -1247, -1247, -1247,   536, -1247, -1247, -1247,    65,
    -896,  -143,  -432,  -430, -1247,   614,  -112, -1247, -1247,    36,
      37,   705, -1247, -1247,   755,   -22, -1247,  -355,    58,   179,
   -1247,   211, -1247, -1247, -1247,  -467,  1181, -1247, -1247, -1247,
   -1247, -1247,   711,   695, -1247, -1247, -1247,  -354,  -706, -1247,
    1136, -1145, -1247,   -70,  -172,   -24,   733, -1247,  -349, -1247,
    -356,  -955, -1246,  -263,   136, -1247,   453,   524, -1247, -1247,
   -1247, -1247,   469, -1247,  1920, -1112
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1043
static const yytype_int16 yytable[] =
{
     185,   187,   338,   189,   190,   191,   193,   194,   195,   434,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   514,   484,   228,   231,   395,   122,   124,   127,
     398,   399,   120,   937,   125,   252,   974,   650,  1333,   255,
    1150,   790,   652,   654,   406,   802,   776,   263,   257,   266,
     506,   346,   347,   261,   350,   932,   430,   337,   238,   951,
     724,  1434,  1142,   536,  1028,   408,   434,   243,   244,   157,
     483,   765,   121,   766,   772,   773,   913,   866,   871,   255,
     405,   854,  1319,  1167,   410,   816,   345,  1042,  1617,   245,
    1222,   933,   -37,  1016,   818,   585,   587,   -37,   424,  1178,
     548,   407,  1395,   540,   358,   795,   -36,   -72,   798,   597,
     799,   -36,   -72,   602,   548,  1560,  1562,  -351,    14,  1625,
    -887,    14,    14,    14,  1710,  1779,  1779,  1617,  1334,  1772,
     391,   877,   548,   392,   894,   541,   894,   894,   523,   894,
     408,   894,   359,  1211,   886,   887,   919,  1459,    14,   211,
      40,   188,   501,   502,   626,   405,  1773,  1151,  1376,   410,
    1921,   934,   516,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,  1201,   407,   525,   248,  1099,
    1790,     3,   485,   517,  1573,  1464,   367,  1901,   348,  -101,
     581,  1330,   524,   534,   471,   433,   410,  1924,   251,   504,
     421,  -885,  1152,   384,  -101,  1258,   472,  1262,   533,  -891,
    -884,  -886,  -898,   407,  -928,   499,   500,  1335,   501,   502,
     509,  -888,   254,   886,   887,  1791,   211,    40,   531,   407,
    1465,  -890,  1902,  -931,  1009,  -930,  -871,  -872,  1112,   627,
    1202,   509,  1925,   543,  -893,  -586,   543,  -887,  1304,  1305,
    -594,   582,   515,   255,   554,   920,   396,  -591,   360,   819,
    -797,  -703,   381,  -591,  -797,   511,  1396,  -290,  -274,  -797,
     921,   817,   694,  1574,  -795,  1618,  1619,   565,  1181,   -37,
     545,  1434,   501,   502,   550,   425,  1388,   549,  1473,   576,
    -290,  1153,  1532,   -36,   -72,  1479,   598,  1481,  -710,  -709,
     603,   625,  1561,  1563,  -351,  1025,  1626,  1229,  1355,  1233,
    1027,  1711,  1780,  1829,  1897,  1774,   400,   878,   879,  1466,
     895,  1903,   987,  1310,  1501,  1507,   505,  1567,  -885,  -704,
     608,  1926,   593,  -897,  -894,  -702,  -891,  -884,  -886,   504,
    1373,  -928,   109,   668,   337,   607,  1913,   510,  -888,   611,
     512,  1108,  1109,  1134,   777,  -900,   861,   692,  -890,   593,
    -931,  1070,  -930,  -871,  -872,   255,   407,   434,   510,  -895,
     338,  -896,   228,   619,   255,   255,  1325,   861,  -535,  -100,
     631,   508,   883,  1935,  1006,  1164,  1471,   508,   258,  1854,
     365,  1620,   594, -1003,  -100,   358,   358,   588,   366,   193,
    1071,   382,  -592,   501,   502,  1231,  1232,   677,   633,  1326,
     395,   741,   742,   430,   421,  1723,   862,  1724,   689,   420,
     363,  1231,  1232,   401,   746,   337,  1125,   364,  1589,  1318,
     402,   259, -1003,   636,  1855,  1914,   127,   420,   695,   696,
     697,   698,   700,   701,   702,   703,   704,   705,   706,   707,
     708,   709,   710,   711,   712,   713,   714,   715,   716,   717,
     718,   719,   720,   721,   722,   723,   505,   725,   663,   726,
     726,   729,  1936,  1385,   385,   386,   260,   382,   734,   121,
     748,   749,   750,   751,   752,   753,   754,   755,   756,   757,
     758,   759,   760,   761,   382,   114,   420,  1234,   238,   726,
     771,   413,   689,   689,   726,   775,   369,   243,   244,   378,
    1157,   749,  1158,  1398,   779,   747,  1796,  1581,  -711,  1583,
     783,   680,   940,   787,   942,   789,   379,   337,   380,   245,
     638,   639,   382,   689,   370,   736,  1175,  1228,   371,   633,
     805,   806,   484,   807,   264,   396,  1332,   336,   606,  -593,
     385,   386,   744,   382,   969,  1521,   372,   622,   622,  1775,
     416,   769,  -859,   968,   373,   650,  1183,   385,   386,  1320,
     652,   654,   376,  1839,   501,   502,   653,  -859,  1776,   810,
     377,  1777,  1321,   976,   736,   393,   397,   373,   412,   483,
     873,   373,   373,   382,  1006,   794,   980,  1342,   800,   866,
    1344,  1105,  1322,  1106,   923,   385,   386,    62,    63,    64,
     175,   176,   431,   501,   502,   419,  -862,   373,   426,   900,
     902,   420, -1003,   382,   958,  1736,   385,   386,  1538,  1741,
     383,  -862,  1767,   870,   870,   423,  1596,  -705,   912,   435,
     407,   940,   942,   421,   436,  1480,   926,   437,  1017,   942,
    1768,   679,  1367,  1369,  1369,   477,   737,   382, -1003,  1377,
    -860, -1003,  1922,   438,   633,   628,   385,   386,   439,  1769,
     536,   501,   502,  -898,   440,  -860,   382,   432,   210,  1018,
    1100,   948,   737,   633,  1879,  1822,   728,  -587,   521,  -117,
     222,   222,  1357,  -117,   959,   384,   385,   386,  1022,  1023,
      50,   485,  1213,  1214,  1823,   737,  -588,  1824,   650,   161,
    -117,  1304,  1305,   652,   654,   770,   737,  1528,  1529,   737,
     774,   441,  1539,   442,   663,  1737,  1738,  1463,   967,   966,
     385,   386,   224,   226,  -589,  1540,   214,   215,   216,  1541,
     474,  1348,   475,   114,  1909,  1910,  1565,   114,   634,   385,
     386,   555,  1358,  1095,  1096,  1097,   179,  1387,   979,    89,
    1542,   476,    91,    92,   507,    93,  1543,    95,  -590,  1098,
    -892,    34,    35,    36,  1894,   584,   586,  1475,  1230,  1231,
    1232,   513,   225,   225,   212,  1795,  1820,  1821,  1912,  1798,
     105,  1020,  1392,  1231,  1232,  1883,  1884,  1885,  1428,  1816,
    1817,  1006,  1006,  1006,  1006,  1053,  1056,   255,  -703,  1006,
    1026,   409,   518,  1486,   389,  1487,   468,   469,   470,  1044,
     471,   415,   417,   418,   520,  1050,   472,   127,   421,   526,
     529,  1621,   472,  -896,   575,   508,    79,    80,    81,    82,
      83,   530,  -701,   650,  1454,  1037,   537,   217,   652,   654,
      55,   538,   546,    87,    88,   559,   567, -1042,    62,    63,
      64,   175,   176,   431,   570,   571,   589,    97,   590,   601,
     121,   577,    62,    63,    64,    65,    66,   431,   578,   127,
     599,   102,  1503,    72,   478,   592,   409,  1120,   600,   612,
     222,   613,  1129,   655,  1130,   656,   807,   665,  1512,   666,
    -122,   870,   114,   870,   667,  1132,  1182,   870,   870,  1110,
     669,   681,    55,   678,  1477,   336,  1590,   691,   373,  1141,
     780,   782,   121,   409,   480,  1892,   628,   784,   432,   734,
     785,   791,   527,   792,   808,   548,   122,   124,   127,   535,
    1904,   120,   432,   125,  1592,  1162,  1593,   812,  1594,   815,
     829,  1595,   565,   830,   855,  1170,  1871,   161,  1171,   127,
    1172,   161,   857,  1006,   689,  1006,   858,   876,   575,   373,
     739,   373,   373,   373,   373,   859,  1871,   860,   157,   880,
     881,   121,   225,   884,  1338,  1893,  1143,    62,    63,    64,
     175,   176,   431,   885,   764,   891,   893,   898,   769,   896,
     800,   663,   121,  1598,   238,   899,   901,   903,  1206,   904,
    1210,   910,  1604,   243,   244,   915,   916,   575,   663,   918,
    -726,   924,   925,   222,  1216,   927,  1611,   220,   220,   928,
     931,   797,   222,   935,   936,   245,   944,  1578,   946,   222,
     949,   950,   114,   952,   961,  1745,   955,   962,   965,   222,
     127,  1217,   127,   981,  1245,   964,   973,   432,   650,   983,
     651,  1249,   852,   652,   654,   596,  1312,   984,   985,   957,
    1029,  1263,  1019,  -707,   604,   800,   609,  1039,   629,  1043,
    1048,   616,   635,  1041,   872,   681,  1047,  1049,  1051,  1064,
    1065,   632,  1066,   121,  1006,   121,  1006,  1068,  1006,  1103,
    1067,  1006,  1752,  1111,   653,  1113,  1115,   737,   629,  1117,
     635,   629,   635,   635,  1118,   225,   161,   907,   909,   737,
    1119,   737,  1313,  1314,   225,  1124,   610,  1128,  1131,  1137,
    1139,   225,  1140,   465,   466,   467,   468,   469,   470,   650,
     471,   225,  1144,  1146,   652,   654,  1092,  1093,  1094,  1095,
    1096,  1097,   472,  1148,  1165,   870,  1340,   122,   124,   127,
    1174,  1177,   120,  1835,   125,  1098,  1180,  1185,  -899,   689,
    1186,  1196,  1197,  1198,  1199,  1200,   222,  1204,  1203,  1205,
     689,  1314,  1207,  1224,  1219,   373,  1221,  1225,  1227,  1236,
    1237,  1238,  1243,  1362,  1244,  1006,   737,  1098,  1248,   157,
    1799,  1800,   121,  1247,  1298,  1296,  1299,  1301,  1308,  1311,
    1328,   969,  1336,  1337,  1329,  1380,   255,  1353,   616,  1341,
    1343,  1349,  1345,  1359,  1361,  1347,  1394,   220,   994,  1381,
     663,  1350,  1352,   663,  1372,  1379,  1382,  1360,  1389,  1794,
    1384,  1882,  1393,  1399,  1383,  1402,  1404,   653,  1405,  1801,
    1411,  1408,  1410,   127,  1415,  1414,   161,  1419,  1409,  1413,
    1416,  1417,  1418,  1420,  1424,  1422,  1423,  1427,   225,  1429,
    1467,  1001,  1430,  1012,  1468,  1456,  1470,  1920,  1457,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,  1472,   114,  1836,  1474,   121,  1478,  1483,  1489,
    1490,  1484,  1482,  1485,  1492,  1488,  1494,  1035,   114,  1497,
    1455,  1499,  1498,  1493,  1502,  1496,  1504,  1460,  1505,  1566,
    1509,  1461,  1506,  1462,  1513,  1536,  1436,  1510,  1549,  1525,
     434,  1469,   499,   500,  1448,  1564,  1579,  1575,  1569,  1858,
    1584,  1476,   689,  1576,  1448,   114,  1585,  1587,   222,  1606,
    1006,  1006,  1107,   681,  1591,  1615,  1609,  1623,  1719,  1624,
     220,  1718,  1725,  1731,  1491,  1735,  1732,  1734,  1495,   220,
     210,    14,  1744,  1500,  1453,  1746,   220,  1747,  1757,  1778,
    1758,  1121,   653,  1784,  1453,  1788,   220,  1793,  1787,  1810,
     947,  1812,    50,  1814,  1818,  1826,  1827,  1721,  1828,   501,
     502,  1833,  1834,  1838,   114,  1841,  1918,  1842,   222,  -347,
     663,  1923,  1844,  1845,  1847,  1849,  1850,  1773,  1853,   575,
    1856,  1860,  1859,  1861,  1866,   114,  1873,  1880,   214,   215,
     216,   764,  1868,   797,  1437,  1877,  1881,  1889,  1891,  1438,
     225,    62,    63,    64,   175,  1439,   431,  1440,  1898,   222,
     978,   222,  1712,   127,    91,    92,  1713,    93,   180,    95,
    1614,  1895,  1577,  1896,  1905,   689,  1915,   373,  1916,  1919,
    1927,  1928,  1937,  1938,  1941,  1940,  1300,   222,   485,  1190,
    1190,  1001,   105,  1553,  1876,   738,   743,  1441,  1442,  1176,
    1443,  1013,   740,  1014,  1136,  1890,   121,  1751,  1448,  1386,
     225,  1888,   874,  1511,  1448,  1742,  1448,   161,   797,  1766,
    1622,   432,  1771,   220,   114,  1558,   114,  1930,   114,  1033,
    1444,  1556,   161,  1900,  1740,  1783,  1448,  1537,   127,   624,
    1257,  1371,  1250,  1323,  1616,  1192,  1602,   127,  1453,  1240,
     222,   225,  1436,   225,  1453,  1363,  1453,  1364,  1208,   663,
    1187,  1188,  1189,   210,  1156,  1917,   222,   222,   618,   161,
     690,   575,  1054,  1932,  1851,  1786,  1453,  1527,  1295,   225,
    1733,   121,  1303,  1242,     0,    50,     0,     0,     0,     0,
     121,     0,  1116,     0,     0,     0,   210,    14,   651,     0,
     852,     0,     0,     0,     0,     0,     0,   653,   616,  1127,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,   214,   215,   216,  1448,     0,     0,     0,   161,     0,
       0,     0,     0,   127,     0,   114,     0,  1749,  1602,   127,
       0,     0,   225,  1552,  1726,     0,   127,    91,    92,   161,
      93,   180,    95,     0,   214,   215,   216,     0,   225,   225,
    1437,     0,     0,     0,  1453,  1438,     0,    62,    63,    64,
     175,  1439,   431,  1440,     0,   105,   121,  1781,     0,     0,
      91,    92,   121,    93,   180,    95,     0,     0,   653,   121,
       0,     0,     0,     0,     0,   220,     0,     0,  1001,  1001,
    1001,  1001,   222,   222,     0,     0,  1001,     0,   105,  1553,
       0,     0,     0,  1441,  1442,     0,  1443,   114,     0,     0,
       0,  1864,     0,     0,  1436,     0,  1831,     0,     0,   114,
       0,     0,   337,     0,     0,     0,  1789,   432,   161,  1401,
     161,   651,   161,     0,  1033,  1223,  1458,    62,    63,    64,
      65,    66,   431,     0,     0,   220,     0,   434,    72,   478,
     219,   219,     0,     0,   235,     0,     0,     0,     0,    14,
    1811,  1813,     0,     0,   516,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   127,     0,   235,
       0,     0,     0,     0,   225,   225,   220,   479,   220,   480,
       0,     0,     0,     0,  1431,     0,     0,     0,     0,     0,
       0,     0,   481,     0,   482,     0,     0,   432,     0,     0,
       0,     0,     0,     0,   220,     0,     0,   499,   500,   127,
     121,     0,  1437,   222,     0,     0,   127,  1438,     0,    62,
      63,    64,   175,  1439,   431,  1440,     0,     0,     0,   161,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1001,     0,  1001,     0,     0,     0,     0,     0,     0,     0,
       0,   127,   121,   339,     0,  1339,   651,     0,     0,   121,
    1865,     0,   222,     0,     0,  1441,  1442,   220,  1443,     0,
       0,   210,     0,   127,   501,   502,     0,   222,   222,     0,
       0,     0,     0,   220,   220,     0,     0,     0,     0,   432,
       0,     0,     0,    50,   121,     0,     0,     0,  1582,     0,
    1929,     0,     0,     0,  1378,   225,     0,   663,     0,   114,
       0,   161,  1939,     0,     0,     0,   121,     0,   336,   616,
    1033,     0,  1942,   161,  1554,  1943,     0,   663,     0,   214,
     215,   216,     0,   127,     0,   793,   663,     0,   127,     0,
     219,     0,  1436,     0,     0,     0,     0,     0,     0,   179,
       0,     0,    89,     0,   225,    91,    92,     0,    93,   180,
      95,     0,     0,     0,   222,     0,   343,     0,     0,   225,
     225,  1001,     0,  1001,     0,  1001,   121,     0,  1001,     0,
       0,   121,     0,   105,   114,     0,     0,    14,  1804,   114,
     235,     0,   235,   114,     0,     0,     0,     0,     0,     0,
    1436,     0,     0,     0,     0,     0,   616,     0,     0,     0,
       0,   373,     0,     0,   575,     0,     0,   336,     0,   220,
     220,     0,     0,     0,     0,     0,     0,  1707,     0,     0,
       0,     0,     0,     0,  1714,     0,     0,     0,     0,     0,
       0,   336,     0,   336,     0,    14,     0,     0,   235,   336,
    1437,     0,     0,     0,     0,  1438,   225,    62,    63,    64,
     175,  1439,   431,  1440,     0,     0,     0,     0,     0,     0,
       0,   651,  1001,   219,   339,     0,   339,     0,     0,   114,
     114,   114,   219,     0,     0,   114,     0,     0,     0,   219,
       0,     0,   114,     0,     0,     0,     0,     0,     0,   219,
       0,     0,     0,  1441,  1442,     0,  1443,     0,  1437,     0,
     219,     0,     0,  1438,     0,    62,    63,    64,   175,  1439,
     431,  1440,     0,   161,     0,     0,     0,   432,     0,     0,
       0,     0,   339,     0,   235,     0,  1586,   235,     0,     0,
       0,     0,     0,  1436,     0,     0,     0,     0,     0,     0,
     220,     0,   651,     0,     0,     0,     0,     0,     0,     0,
       0,  1441,  1442,     0,  1443,     0,     0,     0,     0,     0,
       0,  1436,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   235,   432,     0,   563,    14,   564,
       0,     0,     0,     0,  1588,     0,     0,     0,   161,   220,
       0,     0,   575,   161,     0,     0,     0,   161,     0,     0,
       0,     0,     0,     0,   220,   220,    14,     0,   339,     0,
       0,   339,     0,   336,     0,     0,   219,  1001,  1001,     0,
       0,     0,     0,   114,     0,     0,     0,     0,     0,     0,
       0,     0,  1805,     0,     0,   569,     0,     0,     0,  1707,
    1707,  1437,     0,  1714,  1714,     0,  1438,     0,    62,    63,
      64,   175,  1439,   431,  1440,     0,     0,   373,     0,     0,
       0,     0,     0,     0,     0,   114,     0,     0,   235,  1437,
     235,     0,   114,   842,  1438,     0,    62,    63,    64,   175,
    1439,   431,  1440,   161,   161,   161,     0,     0,     0,   161,
       0,   220,     0,     0,  1441,  1442,   161,  1443,     0,     0,
       0,     0,     0,     0,   842,     0,     0,   114,     0,     0,
       0,     0,     0,     0,     0,  1863,     0,     0,   432,   822,
       0,   684,  1441,  1442,   343,  1443,     0,  1597,     0,   114,
       0,     0,     0,     0,     0,  1878,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   432,     0,     0,     0,
       0,     0,   339,     0,   825,  1743,     0,     0,   235,   235,
       0,     0,     0,     0,     0,     0,     0,   235,     0,   210,
       0,     0,   443,   444,   445,     0,     0,     0,     0,   823,
       0,     0,     0,     0,     0,     0,     0,     0,   219,   114,
       0,    50,   446,   447,   114,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,     0,   471,
       0,     0,     0,     0,     0,     0,     0,   214,   215,   216,
       0,   472,     0,     0,     0,     0,     0,   161,     0,     0,
       0,     0,   339,   339,     0,     0,     0,   179,   219,     0,
      89,   339,     0,    91,    92,     0,    93,   180,    95,     0,
     824,     0,     0,     0,     0,   821,     0,     0,     0,  1072,
    1073,  1074,     0,     0,     0,     0,     0,     0,     0,   161,
       0,   105,     0,   235,     0,     0,   161,     0,     0,   219,
    1075,   219,     0,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
    1093,  1094,  1095,  1096,  1097,     0,     0,   219,   842,     0,
       0,   161,     0,     0,     0,   235,     0,     0,  1098,     0,
       0,     0,   235,   235,   842,   842,   842,   842,   842,     0,
       0,     0,     0,   161,   842,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   929,   930,     0,   235,     0,
       0,     0,     0,  1316,   938,     0,     0,   516,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     219,     0,   210,     0,   211,    40,     0,     0,     0,     0,
       0,     0,     0,     0,   235,     0,   219,   219,     0,     0,
       0,     0,     0,   161,    50,     0,     0,     0,   161,  1046,
       0,     0,     0,     0,     0,     0,   339,   339,   235,   235,
     499,   500,     0,     0,     0,     0,     0,     0,   219,     0,
       0,     0,     0,     0,   235,     0,     0,   210,     0,     0,
     214,   215,   216,     0,     0,     0,     0,   235,     0,     0,
       0,     0,     0,     0,  1261,   842,     0,     0,   235,    50,
       0,     0,     0,     0,   762,     0,    91,    92,     0,    93,
     180,    95,     0,     0,     0,     0,   235,     0,     0,     0,
     235,     0,     0,     0,     0,     0,     0,   501,   502,     0,
       0,     0,     0,   235,   105,   214,   215,   216,   763,     0,
     109,     0,   339,     0,   516,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,     0,   339,   428,
       0,    91,    92,     0,    93,   180,    95,     0,     0,   684,
     684,   339,   219,   219,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   235,     0,   882,   105,
     235,     0,   235,     0,     0,     0,   822,   499,   500,     0,
     339,     0,     0,     0,     0,     0,     0,   842,   842,   842,
     842,   219,     0,     0,   842,   842,   842,   842,   842,   842,
     842,   842,   842,   842,   842,   842,   842,   842,   842,   842,
     842,   842,   842,   842,   842,   842,   842,   842,   842,   842,
     842,   842,     0,   221,   221,     0,   210,   237,     0,     0,
       0,     0,     0,     0,     0,  1135,   823,     0,   842,     0,
       0,     0,     0,     0,   501,   502,     0,     0,    50,     0,
     339,  1145,     0,     0,   339,     0,   825,     0,     0,     0,
       0,     0,     0,     0,  1159,     0,     0,     0,     0,     0,
       0,   235,     0,   235,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   219,   214,   215,   216,     0,     0,     0,
       0,     0,     0,  1179,     0,     0,     0,     0,     0,     0,
     235,     0,     0,   235,   179,     0,     0,    89,     0,     0,
      91,    92,     0,    93,   180,    95,     0,  1241,     0,     0,
     235,   235,   235,   235,     0,     0,   219,     0,   235,     0,
       0,     0,   219,     0,     0,     0,     0,     0,   105,     0,
       0,     0,     0,     0,     0,     0,     0,   219,   219,     0,
     842,     0,     0,     0,     0,   339,     0,   339,     0,     0,
     235,     0,     0,  1235,     0,     0,   235,  1239,     0,   842,
       0,   842,     0,     0,   443,   444,   445,     0,     0,     0,
       0,     0,     0,     0,   339,     0,     0,   339,     0,     0,
       0,     0,     0,   842,   446,   447,     0,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     210,   471,     0,   221,     0,     0,     0,   235,   235,     0,
     210,   235,     0,   472,   219,     0,     0,     0,     0,     0,
       0,     0,    50,     0,   339,     0,     0,     0,     0,     0,
     339,     0,    50,     0,     0, -1043, -1043, -1043, -1043, -1043,
     463,   464,   465,   466,   467,   468,   469,   470,  1331,   471,
     938,     0,     0,     0,     0,     0,     0,     0,   214,   215,
     216,   472,   235,     0,   235,     0,     0,     0,   214,   215,
     216,     0,     0,     0,     0,   277,     0,  1351,     0,     0,
    1354,   389,     0,     0,    91,    92,     0,    93,   180,    95,
       0,   339,   339,     0,    91,    92,     0,    93,   180,    95,
       0,     0,     0,   279,     0,   235,     0,   235,     0,     0,
       0,     0,   105,   842,     0,   842,   390,   842,     0,     0,
     842,   219,   105,   691,   842,   210,   842,     0,     0,   842,
       0,     0,     0,     0,     0,     0,   221,  1400,     0,     0,
     235,   235,   911,  1159,   235,   221,   277,    50,     0,     0,
       0,   235,   221,     0,     0,  -394,     0,     0,     0,     0,
       0,     0,   221,    62,    63,    64,   175,   176,   431,     0,
       0,     0,     0,   221,   279,     0,     0,     0,     0,     0,
       0,     0,   561,   214,   215,   216,   562,     0,     0,   339,
       0,   339,     0,   235,     0,   235,   210,   235,     0,     0,
     235,     0,   219,   179,  1432,  1433,    89,   330,     0,    91,
      92,     0,    93,   180,    95,     0,   235,     0,    50,   842,
       0,     0,     0,     0,   339,     0,   568,   334,     0,     0,
       0,   235,   235,   432,     0,   339,     0,   105,   335,   235,
       0,   235,     0,     0,     0,     0,     0,   237,     0,     0,
       0,     0,     0,   561,   214,   215,   216,   562,     0,     0,
       0,     0,     0,   235,     0,   235,     0,     0,     0,     0,
       0,   235,     0,     0,   179,     0,     0,    89,   330,     0,
      91,    92,     0,    93,   180,    95,     0,     0,     0,   221,
       0,     0,     0,     0,   235,     0,     0,     0,   334,     0,
     339,     0,  1514,     0,  1515,     0,     0,     0,   105,   335,
       0,   842,   842,   842,     0,     0,     0,     0,   842,     0,
     235,     0,     0,   339,     0,     0,   235,     0,   235,     0,
   -1043, -1043, -1043, -1043, -1043,  1090,  1091,  1092,  1093,  1094,
    1095,  1096,  1097,     0,     0,     0,   847,   339,  1559,   339,
       0,     0,     0,     0,     0,   339,  1098,     0,   443,   444,
     445,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   847,   446,   447,
       0,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,     0,   471,     0,     0,     0,     0,
     339,     0,     0,  1605,     0,     0,   210,   472,   211,    40,
       0,     0,     0,     0,     0,     0,     0,     0,   235,     0,
     210,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,   235,     0,     0,     0,   235,
     235,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,   221,     0,     0,   235,     0,     0,     0,     0,     0,
     842,     0,     0,     0,   214,   215,   216,     0,     0,     0,
       0,   842,     0,     0,     0,     0,     0,   842,   214,   215,
     216,   842,     0,     0,     0,     0,     0,     0,   762,     0,
      91,    92,     0,    93,   180,    95,     0,     0,     0,     0,
       0,   210,   339,   235,    91,    92,     0,    93,   180,    95,
       0,   221,     0,  1762,     0,     0,     0,     0,   105,   339,
       0,     0,   796,    50,   109,     0,     0,     0,     0,     0,
       0,     0,   105,   957,     0,     0,   943,     0,  1806,     0,
       0,     0,     0,   842,   223,   223,  1005,     0,   241,     0,
       0,     0,   221,   235,   221,     0,     0,     0,     0,   214,
     215,   216,     0,     0,     0,     0,     0,     0,     0,     0,
     235,     0,     0,     0,     0,     0,   277,     0,     0,   235,
     221,   847,   353,     0,     0,    91,    92,   339,    93,   180,
      95,   235,     0,   235,     0,     0,     0,   847,   847,   847,
     847,   847,     0,     0,   279,     0,     0,   847,     0,     0,
       0,     0,   235,   105,   235,  1785,     0,     0,     0,     0,
       0,  1102,     0,   277,     0,     0,   210,     0,     0,     0,
       0,     0,     0,     0,   849,     0,     0,     0,     0,     0,
       0,     0,     0,   221,     0,     0,     0,     0,    50,     0,
       0,   279,     0,     0,     0,     0,     0,  1123,     0,   221,
     221,     0,     0,   339,     0,   875,     0,     0,     0,     0,
       0,     0,     0,   210,     0,   339,     0,   339,     0,     0,
       0,     0,  1123,   561,   214,   215,   216,   562,     0,     0,
       0,   221,     0,     0,     0,    50,   339,     0,   339,     0,
    1846,     0,     0,     0,   179,     0,     0,    89,   330,     0,
      91,    92,     0,    93,   180,    95,     0,  1052,   847,     0,
       0,  1166,     0,     0,     0,     0,     0,     0,   334,     0,
     561,   214,   215,   216,   562,     0,     0,     0,   105,   335,
       0,     0,     0,   237,   223,     0,     0,     0,     0,     0,
       0,   179,     0,     0,    89,   330,  1005,    91,    92,     0,
      93,   180,    95,     0,  1403,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   334,   938,     0,     0,     0,
       0,     0,     0,     0,     0,   105,   335,     0,  1908,     0,
     938,     0,     0,     0,     0,   221,   221,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1908,
       0,  1933,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     847,   847,   847,   847,   221,     0,     0,   847,   847,   847,
     847,   847,   847,   847,   847,   847,   847,   847,   847,   847,
     847,   847,   847,   847,   847,   847,   847,   847,   847,   847,
     847,   847,   847,   847,   847,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   223,     0,  1034,
       0,   847,     0,     0,     0,     0,   223,     0,     0,     0,
       0,     0,     0,   223,     0,  1057,  1058,  1059,  1060,     0,
       0,     0,     0,   223,     0,  1069,     0,     0,   443,   444,
     445,     0,     0,     0,   241,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   221,     0,   446,   447,
       0,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,     0,   471,     0,     0,     0,     0,
       0,     0,     0,  1005,  1005,  1005,  1005,   472,     0,   221,
       0,  1005,     0,     0,     0,   221,     0,     0,     0,   988,
     989,     0,     0,     0,     0,     0,     0,     0,   241,     0,
     221,   221,     0,   847,     0,     0,     0,     0,     0,   990,
       0,     0,     0,     0,     0,     0,     0,   991,   992,   993,
     210,     0,   847,     0,   847,     0,  1163,   443,   444,   445,
     994,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     223,     0,    50,     0,     0,     0,   847,   446,   447,     0,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,     0,   471,     0,     0,   995,   996,   997,
     998,     0,     0,     0,  1435,     0,   472,   221,     0,     0,
       0,     0,     0,   999,     0,     0,     0,   848,   179,     0,
       0,    89,    90,     0,    91,    92,   982,    93,   180,    95,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1000,     0,     0,     0,     0,     0,   848,     0,
       0,     0,   105,     0,     0,  1005,     0,  1005,     0,  1253,
    1255,  1255,     0,     0,     0,  1264,  1267,  1268,  1269,  1271,
    1272,  1273,  1274,  1275,  1276,  1277,  1278,  1279,  1280,  1281,
    1282,  1283,  1284,  1285,  1286,  1287,  1288,  1289,  1290,  1291,
    1292,  1293,  1294,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   847,     0,   847,  1302,
     847,     0,     0,   847,   221,     0,   210,   847,     0,   847,
       0,     0,   847,     0,   443,   444,   445,     0,     0,     0,
       0,     0,   223,     0,  1535,   986,     0,  1548,    50,     0,
       0,     0,     0,     0,   446,   447,     0,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
       0,   471,     0,     0,   214,   215,   216,     0,     0,     0,
       0,     0,     0,   472,     0,     0,  1005,     0,  1005,     0,
    1005,   277,   223,  1005,   179,   221,     0,    89,    90,     0,
      91,    92,     0,    93,   180,    95,     0,     0,   210,     0,
       0,     0,   847,     0,     0,     0,     0,     0,     0,   279,
       0,  1390,     0,     0,  1612,  1613,     0,     0,   105,     0,
      50,     0,     0,   223,  1548,   223,     0,     0,     0,     0,
    1406,   210,  1407,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1539,     0,     0,     0,     0,     0,     0,     0,
       0,   223,   848,    50,  1425,  1540,   214,   215,   216,  1541,
       0,     0,     0,     0,     0,     0,     0,     0,   848,   848,
     848,   848,   848,     0,     0,     0,   179,  1005,   848,    89,
      90,     0,    91,    92,     0,    93,  1543,    95,   561,   214,
     215,   216,   562,     0,   847,   847,   847,     0,     0,     0,
       0,   847,  1114,  1760,     0,     0,     0,     0,     0,   179,
     105,  1548,    89,   330,   223,    91,    92,     0,    93,   180,
      95,     0,     0,     0,     0,     0,     0,     0,   210,     0,
     223,   223,     0,   334,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   105,   335,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,   351,   352,
     210,     0,   241,     0,     0,     0,     0,     0,     0,     0,
     210,     0,   905,     0,   906,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,   214,   215,   216,   848,
     863,   864,    50,     0,  1517,     0,  1518,     0,  1519,     0,
       0,  1520,     0,     0,     0,  1522,     0,  1523,     0,   353,
    1524,     0,    91,    92,   241,    93,   180,    95,   214,   215,
     216,     0,     0,     0,     0,     0,     0,     0,   214,   215,
     216,     0,  1005,  1005,     0,     0,     0,     0,     0,     0,
     105,   865,     0,     0,    91,    92,     0,    93,   180,    95,
       0,     0,     0,   847,    91,    92,     0,    93,   180,    95,
       0,     0,     0,     0,   847,     0,   223,   223,     0,     0,
     847,     0,   105,     0,   847,     0,     0,     0,     0,     0,
       0,     0,   105,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1607,   848,   848,   848,   848,   241,     0,     0,   848,   848,
     848,   848,   848,   848,   848,   848,   848,   848,   848,   848,
     848,   848,   848,   848,   848,   848,   848,   848,   848,   848,
     848,   848,   848,   848,   848,   848,   847,     0,     0,     0,
       0,   443,   444,   445,     0,     0,  1875,     0,     0,     0,
       0,     0,   848,     0,     0,     0,     0,     0,     0,     0,
       0,   446,   447,  1535,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,     0,   471,   443,
     444,   445,  1753,  1754,  1755,     0,     0,   223,     0,  1759,
     472,     0,     0,     0,     0,     0,     0,     0,     0,   446,
     447,     0,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,     0,   471,   210,     0,     0,
     241,     0,     0,     0,     0,     0,   223,  1045,   472,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1030,    50,
       0,   223,   223,     0,   848,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   848,     0,   848,     0,     0,     0,     0,
      29,     0,     0,     0,     0,   214,   215,   216,    34,    35,
      36,   210,     0,   211,    40,     0,     0,   848,     0,     0,
       0,   212,     0,     0,     0,   179,     0,     0,    89,     0,
       0,    91,    92,    50,    93,   180,    95,     0,     0,  1173,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   213,     0,     0,     0,   223,   105,
       0,  1815,     0,     0,     0,     0,     0,  1031,    75,   214,
     215,   216,  1825,    79,    80,    81,    82,    83,  1830,     0,
       0,     0,  1832,     0,   217,     0,     0,  1184,     0,   179,
      87,    88,    89,    90,     0,    91,    92,     0,    93,   180,
      95,     0,     0,     0,    97,     0,   267,   268,     0,   269,
     270,     0,     0,   271,   272,   273,   274,     0,   102,     0,
       0,     0,     0,   105,   218,     0,     0,     0,     0,   109,
     275,     0,   276,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1867,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   848,     0,   848,
     278,   848,     0,     0,   848,   241,     0,     0,   848,     0,
     848,     0,     0,   848,   280,   281,   282,   283,   284,   285,
     286,     0,     0,     0,   210,     0,   211,    40,     0,     0,
       0,     0,     0,     0,     0,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,    50,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     210,   321,     0,   732,   323,   324,   325,     0,     0,     0,
     326,   572,   214,   215,   216,   573,   241,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   574,   848,     0,     0,     0,     0,    91,    92,
       0,    93,   180,    95,   331,     0,   332,     0,     0,   333,
       0,     0,     0,     0,     0,     0,     0,     0,   214,   215,
     216,     0,     0,     0,     0,     0,   105,     0,     0,     0,
     733,     0,   109,     0,     0,     0,     0,     0,     0,     0,
       0,   865,     0,     0,    91,    92,     0,    93,   180,    95,
       0,     0,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   105,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,   848,   848,   848,     0,     0,
       0,     0,   848,     0,     0,     0,     0,     0,    14,    15,
      16,  1765,     0,     0,     0,    17,     0,    18,    19,    20,
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
       0,     0,   105,   106,   848,   107,   108,  1133,   109,   110,
       0,   111,   112,     0,     0,   848,     0,     0,     0,     0,
       0,   848,     0,     0,     0,   848,     0,     0,     0,     0,
       0,     0,     0,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,  1848,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,    16,     0,     0,     0,     0,    17,   848,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,    56,    57,    58,     0,    59,    60,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,     0,    79,    80,    81,    82,    83,     0,     0,
       0,    84,     0,     0,    85,     0,     0,     0,   210,    86,
      87,    88,    89,    90,     0,    91,    92,     0,    93,    94,
      95,    96,     0,     0,    97,     0,     0,    98,     0,     0,
      50,     0,     0,    99,   100,     0,   101,     0,   102,   103,
     104,     0,     0,   105,   106,     0,   107,   108,  1317,   109,
     110,     0,   111,   112,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,   214,   215,   216,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    91,    92,     0,    93,   180,    95,     0,     0,
      14,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
     105,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,    56,    57,    58,     0,    59,    60,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,     0,    79,    80,    81,    82,    83,     0,
       0,     0,    84,     0,     0,    85,     0,     0,     0,     0,
      86,    87,    88,    89,    90,     0,    91,    92,     0,    93,
      94,    95,    96,     0,     0,    97,     0,     0,    98,     0,
       0,     0,     0,     0,    99,   100,     0,   101,     0,   102,
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
       0,   179,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   180,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,   106,     0,   107,   108,
     670,   109,   110,     0,   111,   112,     5,     6,     7,     8,
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
       0,     0,   179,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   180,    95,    96,     0,     0,    97,     0,     0,
      98,     0,     0,     0,     0,     0,    99,     0,     0,     0,
       0,   102,   103,   104,     0,     0,   105,   106,     0,   107,
     108,  1101,   109,   110,     0,   111,   112,     5,     6,     7,
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
       0,     0,     0,   179,    87,    88,    89,    90,     0,    91,
      92,     0,    93,   180,    95,    96,     0,     0,    97,     0,
       0,    98,     0,     0,     0,     0,     0,    99,     0,     0,
       0,     0,   102,   103,   104,     0,     0,   105,   106,     0,
     107,   108,  1147,   109,   110,     0,   111,   112,     5,     6,
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
       0,     0,     0,     0,   179,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   180,    95,    96,     0,     0,    97,
       0,     0,    98,     0,     0,     0,     0,     0,    99,     0,
       0,     0,     0,   102,   103,   104,     0,     0,   105,   106,
       0,   107,   108,  1218,   109,   110,     0,   111,   112,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,  1220,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,     0,   179,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   180,    95,    96,     0,     0,
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
      45,    46,     0,    47,     0,    48,     0,    49,  1391,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,     0,
      79,    80,    81,    82,    83,     0,     0,     0,    84,     0,
       0,    85,     0,     0,     0,     0,   179,    87,    88,    89,
      90,     0,    91,    92,     0,    93,   180,    95,    96,     0,
       0,    97,     0,     0,    98,     0,     0,     0,     0,     0,
      99,     0,     0,     0,     0,   102,   103,   104,     0,     0,
     105,   106,     0,   107,   108,     0,   109,   110,     0,   111,
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
       0,     0,    85,     0,     0,     0,     0,   179,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   180,    95,    96,
       0,     0,    97,     0,     0,    98,     0,     0,     0,     0,
       0,    99,     0,     0,     0,     0,   102,   103,   104,     0,
       0,   105,   106,     0,   107,   108,  1526,   109,   110,     0,
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
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,     0,    79,    80,    81,    82,    83,     0,     0,     0,
      84,     0,     0,    85,     0,     0,     0,     0,   179,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   180,    95,
      96,     0,     0,    97,     0,     0,    98,     0,     0,     0,
       0,     0,    99,     0,     0,     0,     0,   102,   103,   104,
       0,     0,   105,   106,     0,   107,   108,  1756,   109,   110,
       0,   111,   112,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,  1802,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,     0,    79,    80,    81,    82,    83,     0,     0,
       0,    84,     0,     0,    85,     0,     0,     0,     0,   179,
      87,    88,    89,    90,     0,    91,    92,     0,    93,   180,
      95,    96,     0,     0,    97,     0,     0,    98,     0,     0,
       0,     0,     0,    99,     0,     0,     0,     0,   102,   103,
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
     179,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     180,    95,    96,     0,     0,    97,     0,     0,    98,     0,
       0,     0,     0,     0,    99,     0,     0,     0,     0,   102,
     103,   104,     0,     0,   105,   106,     0,   107,   108,  1837,
     109,   110,     0,   111,   112,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,  1840,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   179,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   180,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,   106,     0,   107,   108,
       0,   109,   110,     0,   111,   112,     5,     6,     7,     8,
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
       0,     0,   179,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   180,    95,    96,     0,     0,    97,     0,     0,
      98,     0,     0,     0,     0,     0,    99,     0,     0,     0,
       0,   102,   103,   104,     0,     0,   105,   106,     0,   107,
     108,  1857,   109,   110,     0,   111,   112,     5,     6,     7,
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
       0,     0,     0,   179,    87,    88,    89,    90,     0,    91,
      92,     0,    93,   180,    95,    96,     0,     0,    97,     0,
       0,    98,     0,     0,     0,     0,     0,    99,     0,     0,
       0,     0,   102,   103,   104,     0,     0,   105,   106,     0,
     107,   108,  1874,   109,   110,     0,   111,   112,     5,     6,
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
       0,     0,     0,     0,   179,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   180,    95,    96,     0,     0,    97,
       0,     0,    98,     0,     0,     0,     0,     0,    99,     0,
       0,     0,     0,   102,   103,   104,     0,     0,   105,   106,
       0,   107,   108,  1931,   109,   110,     0,   111,   112,     5,
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
      85,     0,     0,     0,     0,   179,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   180,    95,    96,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
     106,     0,   107,   108,  1934,   109,   110,     0,   111,   112,
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
       0,    85,     0,     0,     0,     0,   179,    87,    88,    89,
      90,     0,    91,    92,     0,    93,   180,    95,    96,     0,
       0,    97,     0,     0,    98,     0,     0,     0,     0,     0,
      99,     0,     0,     0,     0,   102,   103,   104,     0,     0,
     105,   106,     0,   107,   108,     0,   109,   110,     0,   111,
     112,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
     544,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
     175,   176,    67,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
       0,    79,    80,    81,    82,    83,     0,     0,     0,    84,
       0,     0,    85,     0,     0,     0,     0,   179,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   180,    95,     0,
       0,     0,    97,     0,     0,    98,     0,     0,     0,     0,
       0,    99,     0,     0,     0,     0,   102,   103,   104,     0,
       0,   105,   106,     0,   107,   108,     0,   109,   110,     0,
     111,   112,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,   809,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,   175,   176,    67,     0,    68,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,     0,    79,    80,    81,    82,    83,     0,     0,     0,
      84,     0,     0,    85,     0,     0,     0,     0,   179,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   180,    95,
       0,     0,     0,    97,     0,     0,    98,     0,     0,     0,
       0,     0,    99,     0,     0,     0,     0,   102,   103,   104,
       0,     0,   105,   106,     0,   107,   108,     0,   109,   110,
       0,   111,   112,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,  1036,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,    61,    62,
      63,    64,   175,   176,    67,     0,    68,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,     0,    79,    80,    81,    82,    83,     0,     0,
       0,    84,     0,     0,    85,     0,     0,     0,     0,   179,
      87,    88,    89,    90,     0,    91,    92,     0,    93,   180,
      95,     0,     0,     0,    97,     0,     0,    98,     0,     0,
       0,     0,     0,    99,     0,     0,     0,     0,   102,   103,
     104,     0,     0,   105,   106,     0,   107,   108,     0,   109,
     110,     0,   111,   112,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,  1601,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,   175,   176,    67,     0,    68,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,     0,    79,    80,    81,    82,    83,     0,
       0,     0,    84,     0,     0,    85,     0,     0,     0,     0,
     179,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     180,    95,     0,     0,     0,    97,     0,     0,    98,     0,
       0,     0,     0,     0,    99,     0,     0,     0,     0,   102,
     103,   104,     0,     0,   105,   106,     0,   107,   108,     0,
     109,   110,     0,   111,   112,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,  1748,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,   175,   176,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   179,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   180,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,   106,     0,   107,   108,
       0,   109,   110,     0,   111,   112,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,    63,    64,   175,   176,    67,     0,    68,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,     0,    79,    80,    81,    82,
      83,     0,     0,     0,    84,     0,     0,    85,     0,     0,
       0,     0,   179,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   180,    95,     0,     0,     0,    97,     0,     0,
      98,     0,     0,     0,     0,     0,    99,     0,     0,     0,
       0,   102,   103,   104,     0,     0,   105,   106,     0,   107,
     108,     0,   109,   110,     0,   111,   112,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   404,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,   745,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   175,   176,   177,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   178,    75,    76,    77,    78,     0,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,    85,     0,
       0,     0,     0,   179,    87,    88,    89,    90,     0,    91,
      92,     0,    93,   180,    95,     0,     0,     0,    97,     0,
       0,    98,     0,     0,     0,     0,     0,    99,     0,     0,
       0,     0,   102,   103,   104,     0,     0,   105,   106,     0,
       0,     0,     0,   109,   110,     0,   111,   112,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   175,   176,   177,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   178,    75,    76,    77,    78,     0,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,    85,
       0,     0,     0,     0,   179,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   180,    95,     0,     0,     0,    97,
       0,     0,    98,     0,     0,     0,     0,     0,    99,     0,
       0,     0,     0,   102,   103,   104,     0,     0,   105,   181,
       0,   344,     0,     0,   109,   110,     0,   111,   112,     5,
       6,     7,     8,     9,     0,     0,     0,  1075,     0,    10,
    1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,  1095,
    1096,  1097,     0,     0,   685,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1098,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   175,   176,
     177,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   178,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
      85,     0,     0,     0,     0,   179,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   180,    95,     0,   686,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
     181,     0,     0,     0,     0,   109,   110,     0,   111,   112,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   175,
     176,   177,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   178,    75,    76,    77,    78,     0,
      79,    80,    81,    82,    83,     0,     0,     0,     0,     0,
       0,    85,     0,     0,     0,     0,   179,    87,    88,    89,
      90,     0,    91,    92,     0,    93,   180,    95,     0,     0,
       0,    97,     0,     0,    98,     0,     0,     0,     0,     0,
      99,     0,     0,     0,     0,   102,   103,   104,     0,     0,
     105,   181,     0,     0,   804,     0,   109,   110,     0,   111,
     112,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,
    1094,  1095,  1096,  1097,     0,     0,  1160,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1098,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     175,   176,   177,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   178,    75,    76,    77,    78,
       0,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,    85,     0,     0,     0,     0,   179,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   180,    95,     0,
    1161,     0,    97,     0,     0,    98,     0,     0,     0,     0,
       0,    99,     0,     0,     0,     0,   102,   103,   104,     0,
       0,   105,   181,     0,     0,     0,     0,   109,   110,     0,
     111,   112,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   404,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   175,   176,   177,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   178,    75,    76,    77,
      78,     0,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,    85,     0,     0,     0,     0,   179,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   180,    95,
       0,     0,     0,    97,     0,     0,    98,     0,     0,     0,
       0,     0,    99,     0,     0,     0,     0,   102,   103,   104,
       0,     0,   105,   106,   443,   444,   445,     0,   109,   110,
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
       0,     0,     0,    50,     0,     0,     0,     0,   192,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   175,   176,   177,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   178,    75,    76,
      77,    78,     0,    79,    80,    81,    82,    83,     0,     0,
       0,     0,     0,     0,    85,     0,     0,     0,     0,   179,
      87,    88,    89,    90,     0,    91,    92,     0,    93,   180,
      95,     0,     0,     0,    97,     0,     0,    98,     0,     0,
       0,     0,  1212,    99,     0,     0,     0,     0,   102,   103,
     104,     0,     0,   105,   181,     0,     0,     0,     0,   109,
     110,     0,   111,   112,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,     0,   471,     0,   227,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   472,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   175,   176,   177,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   178,    75,
      76,    77,    78,     0,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,    85,     0,     0,     0,     0,
     179,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     180,    95,     0,     0,     0,    97,     0,     0,    98,     0,
       0,     0,     0,     0,    99,     0,     0,     0,     0,   102,
     103,   104,     0,     0,   105,   181,   443,   444,   445,     0,
     109,   110,     0,   111,   112,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   446,   447,     0,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,     0,   471,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   472,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   175,   176,   177,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   178,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   179,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   180,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,  1571,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,   181,     0,   262,   444,
     445,   109,   110,     0,   111,   112,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,   446,   447,
       0,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,     0,   471,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,   472,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   175,   176,   177,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     178,    75,    76,    77,    78,     0,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,    85,     0,     0,
       0,     0,   179,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   180,    95,     0,     0,     0,    97,     0,     0,
      98,     0,     0,     0,     0,     0,    99,     0,     0,     0,
       0,   102,   103,   104,     0,     0,   105,   181,     0,   265,
       0,     0,   109,   110,     0,   111,   112,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   404,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   175,   176,   177,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   178,    75,    76,    77,    78,     0,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,    85,     0,
       0,     0,     0,   179,    87,    88,    89,    90,     0,    91,
      92,     0,    93,   180,    95,     0,     0,     0,    97,     0,
       0,    98,     0,     0,     0,     0,     0,    99,     0,     0,
       0,     0,   102,   103,   104,     0,     0,   105,   106,   443,
     444,   445,     0,   109,   110,     0,   111,   112,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   446,
     447,     0,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,     0,   471,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,   472,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   175,   176,   177,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   178,    75,    76,    77,    78,     0,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,    85,
       0,     0,     0,     0,   179,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   180,    95,     0,     0,     0,    97,
       0,     0,    98,     0,     0,     0,     0,  1572,    99,     0,
       0,     0,     0,   102,   103,   104,     0,     0,   105,   181,
     542,     0,     0,     0,   109,   110,     0,   111,   112,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   699,   471,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   472,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   175,   176,
     177,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   178,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
      85,     0,     0,     0,     0,   179,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   180,    95,     0,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
     181,     0,     0,     0,     0,   109,   110,     0,   111,   112,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,  1095,
    1096,  1097,     0,     0,     0,   745,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1098,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   175,
     176,   177,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   178,    75,    76,    77,    78,     0,
      79,    80,    81,    82,    83,     0,     0,     0,     0,     0,
       0,    85,     0,     0,     0,     0,   179,    87,    88,    89,
      90,     0,    91,    92,     0,    93,   180,    95,     0,     0,
       0,    97,     0,     0,    98,     0,     0,     0,     0,     0,
      99,     0,     0,     0,     0,   102,   103,   104,     0,     0,
     105,   181,     0,     0,     0,     0,   109,   110,     0,   111,
     112,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,     0,   471,     0,     0,   786,     0,     0,     0,
       0,     0,     0,     0,     0,   472,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     175,   176,   177,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   178,    75,    76,    77,    78,
       0,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,    85,     0,     0,     0,     0,   179,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   180,    95,     0,
       0,     0,    97,     0,     0,    98,     0,     0,     0,     0,
       0,    99,     0,     0,     0,     0,   102,   103,   104,     0,
       0,   105,   181,     0,     0,     0,     0,   109,   110,     0,
     111,   112,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,
    1095,  1096,  1097,     0,     0,     0,     0,   788,     0,     0,
       0,     0,     0,     0,     0,     0,  1098,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   175,   176,   177,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   178,    75,    76,    77,
      78,     0,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,    85,     0,     0,     0,     0,   179,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   180,    95,
       0,     0,     0,    97,     0,     0,    98,     0,     0,     0,
       0,     0,    99,     0,     0,     0,     0,   102,   103,   104,
       0,     0,   105,   181,     0,     0,     0,     0,   109,   110,
       0,   111,   112,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,     0,   471,     0,     0,     0,  1209,     0,
       0,     0,     0,     0,     0,     0,   472,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   175,   176,   177,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   178,    75,    76,
      77,    78,     0,    79,    80,    81,    82,    83,     0,     0,
       0,     0,     0,     0,    85,     0,     0,     0,     0,   179,
      87,    88,    89,    90,     0,    91,    92,     0,    93,   180,
      95,     0,     0,     0,    97,     0,     0,    98,     0,     0,
       0,     0,     0,    99,     0,     0,     0,     0,   102,   103,
     104,     0,     0,   105,   181,   443,   444,   445,     0,   109,
     110,     0,   111,   112,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   446,   447,  1395,   448,   449,
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
      62,    63,    64,   175,   176,   177,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   178,    75,
      76,    77,    78,     0,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,    85,     0,     0,     0,     0,
     179,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     180,    95,     0,     0,     0,    97,     0,     0,    98,     0,
       0,  1396,     0,     0,    99,     0,     0,     0,     0,   102,
     103,   104,     0,     0,   105,   181,   443,   444,   445,     0,
     109,   110,     0,   111,   112,     5,     6,     7,     8,     9,
       0,     0,     0,     0,   813,    10,   446,   447,     0,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,     0,   471,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   472,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,   630,    39,    40,     0,   814,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   175,   176,   177,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   178,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   179,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   180,    95,     0,   267,   268,    97,   269,   270,    98,
       0,   271,   272,   273,   274,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,   181,     0,   275,     0,
     276,   109,   110,     0,   111,   112,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
    1093,  1094,  1095,  1096,  1097,     0,     0,     0,   278,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1098,     0,
       0,     0,   280,   281,   282,   283,   284,   285,   286,     0,
       0,     0,   210,     0,   211,    40,     0,     0,     0,     0,
       0,     0,     0,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,    50,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,     0,   321,
       0,   322,   323,   324,   325,     0,     0,     0,   326,   572,
     214,   215,   216,   573,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   267,   268,     0,   269,   270,     0,
     574,   271,   272,   273,   274,     0,    91,    92,     0,    93,
     180,    95,   331,     0,   332,     0,     0,   333,   275,     0,
     276,     0,   277,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   105,     0,     0,     0,   733,     0,
     109,     0,     0,     0,     0,     0,     0,     0,   278,     0,
     279,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   280,   281,   282,   283,   284,   285,   286,     0,
       0,     0,   210,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,    50,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,     0,   321,
       0,     0,   323,   324,   325,     0,     0,     0,   326,   327,
     214,   215,   216,   328,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     329,     0,     0,    89,   330,     0,    91,    92,     0,    93,
     180,    95,   331,     0,   332,     0,     0,   333,   267,   268,
       0,   269,   270,     0,   334,   271,   272,   273,   274,     0,
       0,     0,     0,     0,   105,   335,     0,     0,     0,  1727,
       0,     0,   275,     0,   276,   447,   277,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
       0,   471,   278,     0,   279,     0,     0,     0,     0,     0,
       0,     0,     0,   472,     0,     0,   280,   281,   282,   283,
     284,   285,   286,     0,     0,     0,   210,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,    50,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,     0,   321,     0,     0,   323,   324,   325,     0,
       0,     0,   326,   327,   214,   215,   216,   328,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   329,     0,     0,    89,   330,     0,
      91,    92,     0,    93,   180,    95,   331,     0,   332,     0,
       0,   333,   267,   268,     0,   269,   270,     0,   334,   271,
     272,   273,   274,     0,     0,     0,     0,     0,   105,   335,
       0,     0,     0,  1797,     0,     0,   275,     0,   276,     0,
     277,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,     0,   471,   278,     0,   279,     0,
       0,     0,     0,     0,     0,     0,     0,   472,     0,     0,
     280,   281,   282,   283,   284,   285,   286,     0,     0,     0,
     210,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,    50,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,     0,   321,     0,   322,
     323,   324,   325,     0,     0,     0,   326,   327,   214,   215,
     216,   328,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   329,     0,
       0,    89,   330,     0,    91,    92,     0,    93,   180,    95,
     331,     0,   332,     0,     0,   333,   267,   268,     0,   269,
     270,     0,   334,   271,   272,   273,   274,     0,     0,     0,
       0,     0,   105,   335,     0,     0,     0,     0,     0,     0,
     275,     0,   276,     0,   277,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,
    1095,  1096,  1097,     0,     0,     0,     0,     0,     0,     0,
     278,     0,   279,     0,     0,     0,  1098,     0,     0,     0,
       0,     0,     0,     0,   280,   281,   282,   283,   284,   285,
     286,     0,     0,     0,   210,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,    50,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
       0,   321,     0,     0,   323,   324,   325,     0,     0,     0,
     326,   327,   214,   215,   216,   328,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   329,     0,     0,    89,   330,     0,    91,    92,
       0,    93,   180,    95,   331,     0,   332,     0,     0,   333,
       0,   267,   268,     0,   269,   270,   334,  1530,   271,   272,
     273,   274,     0,     0,     0,     0,   105,   335,     0,     0,
       0,     0,     0,     0,     0,   275,     0,   276,     0,   277,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,     0,   471,     0,
       0,     0,     0,     0,     0,   278,     0,   279,     0,     0,
     472,     0,     0,     0,     0,     0,     0,     0,     0,   280,
     281,   282,   283,   284,   285,   286,     0,     0,     0,   210,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,    50,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,     0,   321,     0,     0,   323,
     324,   325,     0,     0,     0,   326,   327,   214,   215,   216,
     328,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   329,     0,     0,
      89,   330,     0,    91,    92,     0,    93,   180,    95,   331,
       0,   332,     0,     0,   333,  1627,  1628,  1629,  1630,  1631,
       0,   334,  1632,  1633,  1634,  1635,     0,     0,     0,     0,
       0,   105,   335,     0,     0,     0,     0,     0,     0,  1636,
    1637,  1638,     0,   446,   447,     0,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,  1639,
     471,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   472,  1640,  1641,  1642,  1643,  1644,  1645,  1646,
       0,     0,     0,   210,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1647,  1648,  1649,  1650,  1651,  1652,
    1653,  1654,  1655,  1656,  1657,    50,  1658,  1659,  1660,  1661,
    1662,  1663,  1664,  1665,  1666,  1667,  1668,  1669,  1670,  1671,
    1672,  1673,  1674,  1675,  1676,  1677,  1678,  1679,  1680,  1681,
    1682,  1683,  1684,  1685,  1686,  1687,     0,     0,     0,  1688,
    1689,   214,   215,   216,     0,  1690,  1691,  1692,  1693,  1694,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1695,  1696,  1697,     0,     0,     0,    91,    92,     0,
      93,   180,    95,  1698,     0,  1699,  1700,     0,  1701,     0,
       0,     0,     0,     0,     0,  1702,  1703,     0,  1704,     0,
    1705,  1706,     0,   267,   268,   105,   269,   270,  1073,  1074,
     271,   272,   273,   274,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   275,  1075,   276,
       0,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,
    1095,  1096,  1097,     0,     0,     0,     0,   278,     0,     0,
       0,     0,     0,     0,     0,     0,  1098,     0,     0,     0,
       0,   280,   281,   282,   283,   284,   285,   286,     0,     0,
       0,   210,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,    50,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,     0,   321,     0,
     322,   323,   324,   325,     0,     0,     0,   326,   572,   214,
     215,   216,   573,     0,     0,     0,     0,     0,   267,   268,
       0,   269,   270,     0,     0,   271,   272,   273,   274,   574,
       0,     0,     0,     0,     0,    91,    92,     0,    93,   180,
      95,   331,   275,   332,   276,     0,   333,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   105,     0,     0,     0,     0,     0,     0,
       0,     0,   278,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   280,   281,   282,   283,
     284,   285,   286,     0,     0,     0,   210,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,    50,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,     0,   321,     0,  1262,   323,   324,   325,     0,
       0,     0,   326,   572,   214,   215,   216,   573,     0,     0,
       0,     0,     0,   267,   268,     0,   269,   270,     0,     0,
     271,   272,   273,   274,   574,     0,     0,     0,     0,     0,
      91,    92,     0,    93,   180,    95,   331,   275,   332,   276,
       0,   333,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   105,     0,
       0,     0,     0,     0,     0,     0,     0,   278,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   280,   281,   282,   283,   284,   285,   286,     0,     0,
       0,   210,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,    50,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,     0,   321,     0,
       0,   323,   324,   325,     0,     0,     0,   326,   572,   214,
     215,   216,   573,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   574,
       0,     0,     0,     0,     0,    91,    92,     0,    93,   180,
      95,   331,     0,   332,     0,     0,   333,   443,   444,   445,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   105,     0,     0,     0,   446,   447,     0,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,     0,   471,   443,   444,   445,     0,     0,
       0,     0,     0,     0,     0,     0,   472,     0,     0,     0,
       0,     0,     0,     0,     0,   446,   447,     0,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,     0,   471,   443,   444,   445,     0,     0,     0,     0,
       0,     0,     0,     0,   472,     0,     0,     0,     0,     0,
       0,     0,     0,   446,   447,     0,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,     0,
     471,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   472,     0,     0,     0,     0,     0,     0,     0,
       0,   443,   444,   445,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   446,   447,   473,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,     0,   471,   443,
     444,   445,     0,     0,     0,     0,     0,     0,     0,     0,
     472,     0,     0,     0,     0,     0,     0,     0,     0,   446,
     447,   558,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,     0,   471,   443,   444,   445,
       0,     0,     0,     0,     0,     0,     0,     0,   472,     0,
       0,     0,     0,     0,     0,     0,     0,   446,   447,   560,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,     0,   471,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   472,     0,     0,     0,
       0,     0,     0,     0,     0,   443,   444,   445,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   446,   447,   579,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,     0,   471,     0,     0,  1270,     0,     0,     0,     0,
       0,     0,     0,     0,   472,     0,     0,     0,     0,     0,
       0,     0,     0,   831,   832,   583,     0,     0,     0,   833,
       0,   834,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   835,     0,     0,     0,     0,     0,     0,
       0,    34,    35,    36,   210,     0,  1072,  1073,  1074,     0,
       0,     0,     0,     0,   212,     0,     0,     0,     0,     0,
       0,     0,   778,     0,     0,     0,    50,  1075,     0,     0,
    1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,  1095,
    1096,  1097,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   836,   837,   838,   839,  1098,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,   217,     0,     0,
       0,     0,   179,    87,    88,    89,    90,     0,    91,    92,
     801,    93,   180,    95,   831,   832,     0,    97,     0,     0,
     833,     0,   834,     0,     0,     0,   840,     0,     0,     0,
       0,   102,     0,     0,   835,     0,   105,   841,     0,     0,
       0,     0,    34,    35,    36,   210,     0,  1072,  1073,  1074,
       0,     0,  1246,     0,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,  1075,     0,
       0,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,
    1095,  1096,  1097,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   836,   837,   838,   839,  1098,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,   217,     0,
       0,     0,     0,   179,    87,    88,    89,    90,     0,    91,
      92,     0,    93,   180,    95,    29,     0,     0,    97,     0,
       0,     0,     0,    34,    35,    36,   210,   840,   211,    40,
       0,     0,   102,     0,     0,     0,   212,   105,   841,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,  1412,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,   214,   215,   216,     0,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,   217,
       0,     0,     0,     0,   179,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   180,    95,    29,     0,     0,    97,
       0,     0,     0,     0,    34,    35,    36,   210,     0,   211,
      40,     0,     0,   102,     0,     0,     0,   212,   105,   218,
       0,     0,   595,     0,   109,     0,     0,     0,     0,    50,
   -1043, -1043, -1043, -1043,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,     0,   471,     0,
     213,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     472,     0,     0,   615,    75,   214,   215,   216,     0,    79,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
     217,     0,     0,     0,     0,   179,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   180,    95,    29,     0,   977,
      97,     0,     0,     0,     0,    34,    35,    36,   210,     0,
     211,    40,     0,     0,   102,     0,     0,     0,   212,   105,
     218,     0,     0,     0,     0,   109,     0,     0,     0,     0,
      50,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,  1093,  1094,  1095,  1096,  1097,     0,     0,
       0,   213,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1098,     0,     0,     0,    75,   214,   215,   216,     0,
      79,    80,    81,    82,    83,     0,     0,     0,     0,     0,
       0,   217,     0,     0,     0,     0,   179,    87,    88,    89,
      90,     0,    91,    92,     0,    93,   180,    95,    29,     0,
       0,    97,     0,     0,     0,     0,    34,    35,    36,   210,
       0,   211,    40,     0,     0,   102,     0,     0,     0,   212,
     105,   218,     0,     0,     0,     0,   109,     0,     0,     0,
       0,    50, -1043, -1043, -1043, -1043,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,  1093,  1094,  1095,  1096,  1097,     0,
       0,     0,   213,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1098,     0,     0,  1126,    75,   214,   215,   216,
       0,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,   217,     0,     0,     0,     0,   179,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   180,    95,    29,
       0,     0,    97,     0,     0,     0,     0,    34,    35,    36,
     210,     0,   211,    40,     0,     0,   102,     0,     0,     0,
     212,   105,   218,     0,     0,     0,     0,   109,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   213,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,   214,   215,
     216,     0,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,   217,     0,     0,     0,     0,   179,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   180,    95,
       0,     0,     0,    97,     0,     0,   443,   444,   445,     0,
       0,     0,     0,     0,     0,     0,     0,   102,     0,     0,
       0,     0,   105,   218,     0,     0,   446,   447,   109,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,     0,   471,   443,   444,   445,     0,     0,     0,
       0,     0,     0,     0,     0,   472,     0,     0,     0,     0,
       0,     0,     0,     0,   446,   447,     0,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
       0,   471,     0,     0,     0,     0,     0,     0,     0,     0,
     443,   444,   445,   472,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     446,   447,   519,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,     0,   471,   443,   444,
     445,     0,     0,     0,     0,     0,     0,     0,     0,   472,
       0,     0,     0,     0,     0,     0,     0,     0,   446,   447,
     528,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,     0,   471,     0,     0,     0,     0,
       0,     0,     0,     0,   443,   444,   445,   472,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   446,   447,   897,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
       0,   471,   443,   444,   445,     0,     0,     0,     0,     0,
       0,     0,     0,   472,     0,     0,     0,     0,     0,     0,
       0,     0,   446,   447,   963,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,     0,   471,
       0,     0,     0,     0,     0,     0,     0,     0,   443,   444,
     445,   472,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   446,   447,
    1015,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,     0,   471,  1072,  1073,  1074,     0,
       0,     0,     0,     0,     0,     0,     0,   472,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1075,  1315,     0,
    1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,  1095,
    1096,  1097,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1072,  1073,  1074,     0,  1098,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1075,     0,  1346,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,  1093,  1094,  1095,  1096,  1097,     0,     0,  1072,
    1073,  1074,     0,     0,     0,     0,     0,     0,     0,     0,
    1098,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1075,     0,  1421,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
    1093,  1094,  1095,  1096,  1097,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1072,  1073,  1074,     0,  1098,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1075,     0,  1516,  1076,  1077,
    1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,
    1088,  1089,  1090,  1091,  1092,  1093,  1094,  1095,  1096,  1097,
       0,    34,    35,    36,   210,     0,   211,    40,     0,     0,
       0,     0,     0,  1098,   212,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1608,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   232,     0,     0,
       0,     0,     0,   233,     0,     0,     0,     0,     0,     0,
       0,     0,   214,   215,   216,     0,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,   217,     0,     0,
    1610,     0,   179,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   180,    95,     0,     0,     0,    97,     0,    34,
      35,    36,   210,     0,   211,    40,     0,     0,     0,     0,
       0,   102,   644,     0,     0,     0,   105,   234,     0,     0,
       0,     0,   109,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   213,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     214,   215,   216,     0,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,   217,     0,     0,     0,     0,
     179,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     180,    95,     0,     0,     0,    97,     0,    34,    35,    36,
     210,     0,   211,    40,     0,     0,     0,     0,     0,   102,
     212,     0,     0,     0,   105,   645,     0,     0,     0,     0,
     646,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   232,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   214,   215,
     216,     0,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,   217,     0,     0,     0,     0,   179,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   180,    95,
       0,     0,     0,    97,     0,     0,     0,     0,     0,   443,
     444,   445,     0,     0,     0,     0,     0,   102,     0,     0,
       0,     0,   105,   234,     0,     0,     0,     0,   109,   446,
     447,   960,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,     0,   471,   443,   444,   445,
       0,     0,     0,     0,     0,     0,     0,     0,   472,     0,
       0,     0,     0,     0,     0,     0,     0,   446,   447,     0,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,     0,   471,  1072,  1073,  1074,     0,     0,
       0,     0,     0,     0,     0,     0,   472,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1075,  1426,     0,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,  1095,  1096,
    1097,  1072,  1073,  1074,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1098,     0,     0,     0,     0,     0,
       0,     0,  1075,     0,     0,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,  1093,  1094,  1095,  1096,  1097,   445,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1098,     0,     0,     0,     0,   446,   447,     0,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,  1074,   471,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   472,     0,     0,     0,     0,     0,
    1075,     0,     0,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
    1093,  1094,  1095,  1096,  1097,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1098
};

static const yytype_int16 yycheck[] =
{
       5,     6,    56,     8,     9,    10,    11,    12,    13,   127,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   183,   157,    29,    30,    96,     4,     4,     4,
     100,   101,     4,   662,     4,    33,   742,   397,  1150,    44,
     936,   532,   397,   397,   106,   544,   513,    52,    46,    54,
     162,    57,    57,    51,    59,   658,   126,    56,    31,   688,
     471,  1307,   925,   235,   812,   106,   184,    31,    31,     4,
     157,   503,     4,   503,   507,   508,   637,   589,   590,    84,
     106,   580,  1137,   956,   106,     9,    57,   819,     9,    31,
    1029,   659,     9,   790,    32,   351,   352,    14,     9,   972,
       9,   106,    32,   246,    60,   538,     9,     9,   540,     9,
     540,    14,    14,     9,     9,     9,     9,     9,    49,     9,
      70,    49,    49,    49,     9,     9,     9,     9,    83,     9,
      86,     9,     9,    89,     9,   247,     9,     9,    90,     9,
     181,     9,    83,  1016,    50,    51,    54,    54,    49,    83,
      84,   194,   134,   135,    70,   181,    36,    38,    81,   181,
      14,   660,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    90,   181,   218,   194,   158,
      38,     0,   157,   188,    83,    38,    83,    38,   197,   179,
     115,  1146,   218,   234,    57,   127,   218,    38,   194,    70,
     179,    70,    83,   155,   194,  1068,    69,   130,   234,    70,
      70,    70,   194,   218,    70,    67,    68,   172,   134,   135,
      70,    70,    44,    50,    51,    83,    83,    84,   230,   234,
      83,    70,    83,    70,   780,    70,    70,    70,   867,   382,
     155,    70,    83,   248,   194,    70,   251,   197,   102,   103,
      70,   176,   184,   258,   259,   163,   163,    70,   199,   197,
     191,   158,    84,    70,   195,   199,   196,   195,   195,   195,
     178,   195,   433,   172,   180,   196,   197,   179,   975,   196,
     252,  1527,   134,   135,   256,   196,  1225,   196,  1343,   343,
     191,   172,  1437,   196,   196,  1350,   196,  1352,   158,   158,
     196,   196,   196,   196,   196,   804,   196,  1039,  1181,  1041,
     809,   196,   196,   196,   196,   195,    83,   195,   195,   172,
     195,   172,   195,   195,  1379,   195,   197,   195,   197,   158,
     371,   172,   102,   194,   194,   158,   197,   197,   197,    70,
    1203,   197,   199,   195,   343,   371,    83,   197,   197,   371,
     171,   863,   864,   914,   515,   194,   102,   427,   197,   102,
     197,   158,   197,   197,   197,   370,   371,   485,   197,   194,
     424,   194,   377,   378,   379,   380,   164,   102,     8,   179,
     385,   194,   195,    83,   780,   953,  1341,   194,   194,    38,
     122,  1536,   162,   158,   194,   351,   352,   353,   130,   404,
     197,    83,    70,   134,   135,   106,   107,   412,    90,   197,
     480,   481,   482,   483,   179,  1560,   162,  1562,   423,   162,
     123,   106,   107,   190,   486,   424,   893,   130,  1483,  1135,
     197,   194,   197,   389,    83,   172,   411,   162,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   197,   472,   403,   474,
     475,   476,   172,  1221,   156,   157,   194,    83,   477,   411,
     486,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,    83,     4,   162,   198,   471,   504,
     505,    90,   507,   508,   509,   510,   194,   471,   471,    70,
     942,   516,   942,   198,   519,   486,  1741,  1472,   158,  1474,
     522,   203,   665,   528,   667,   530,    70,   526,    70,   471,
     196,   197,    83,   538,   194,   477,   969,  1036,   194,    90,
     546,   546,   676,   548,    53,   163,  1149,    56,   370,    70,
     156,   157,   484,    83,   194,  1418,   194,   379,   380,    31,
      90,   503,   179,   735,    73,   925,   977,   156,   157,   164,
     925,   925,   194,  1798,   134,   135,   397,   194,    50,   551,
     194,    53,   177,   744,   526,    94,   194,    96,   197,   676,
     595,   100,   101,    83,   990,   537,   768,  1165,   540,  1111,
    1168,   857,   197,   859,   645,   156,   157,   119,   120,   121,
     122,   123,   124,   134,   135,    32,   179,   126,    38,   621,
     622,   162,   158,    83,   694,  1580,   156,   157,    31,  1584,
      90,   194,    14,   589,   590,   194,  1499,   158,   198,   196,
     645,   784,   785,   179,   196,  1351,   648,   196,   791,   792,
      32,   202,  1198,  1199,  1200,   158,   477,    83,   194,  1205,
     179,   197,  1908,   196,    90,   155,   156,   157,   196,    51,
     842,   134,   135,   194,   196,   194,    83,   189,    81,   791,
     852,   686,   503,    90,   196,    31,   475,    70,   197,   158,
      27,    28,  1183,   162,   699,   155,   156,   157,    75,    76,
     103,   676,    75,    76,    50,   526,    70,    53,  1068,     4,
     179,   102,   103,  1068,  1068,   504,   537,   132,   133,   540,
     509,   196,   125,   196,   659,   196,   197,  1330,   733,   731,
     156,   157,    27,    28,    70,   138,   139,   140,   141,   142,
      70,  1174,    70,   252,   196,   197,  1452,   256,   155,   156,
     157,   260,  1185,    53,    54,    55,   159,  1224,   763,   162,
     163,   197,   165,   166,   194,   168,   169,   170,    70,    69,
     194,    78,    79,    80,  1886,   351,   352,  1345,   105,   106,
     107,   194,    27,    28,    91,  1740,  1772,  1773,  1900,  1744,
     193,   796,   105,   106,   107,   119,   120,   121,  1297,  1768,
    1769,  1197,  1198,  1199,  1200,   829,   830,   812,   158,  1205,
     808,   106,   196,  1359,   162,  1361,    53,    54,    55,   821,
      57,   110,   111,   112,    48,   827,    69,   802,   179,   158,
     201,  1537,    69,   194,   343,   194,   143,   144,   145,   146,
     147,     9,   158,  1203,  1311,   817,   158,   154,  1203,  1203,
     111,   194,     8,   160,   161,   196,   194,   158,   119,   120,
     121,   122,   123,   124,    14,   158,   197,   174,     9,    14,
     802,   196,   119,   120,   121,   122,   123,   124,   196,   854,
     130,   188,  1381,   130,   131,   196,   181,   889,   130,   195,
     227,   179,   897,    14,   899,   102,   901,   195,  1397,   195,
     194,   857,   411,   859,   195,   910,   976,   863,   864,   865,
     195,   420,   111,   200,  1347,   424,  1484,   194,   427,   924,
     194,     9,   854,   218,   171,  1880,   155,   195,   189,   928,
     195,   195,   227,   195,    94,     9,   913,   913,   913,   234,
    1895,   913,   189,   913,  1490,   950,  1492,   196,  1494,    14,
     194,  1497,   179,     9,   194,   960,  1852,   252,   963,   934,
     965,   256,   197,  1359,   969,  1361,   196,    83,   477,   478,
     479,   480,   481,   482,   483,   197,  1872,   196,   913,   195,
     195,   913,   227,   195,  1156,  1881,   928,   119,   120,   121,
     122,   123,   124,   196,   503,   132,   194,   201,   940,   195,
     942,   936,   934,  1502,   977,     9,     9,   201,  1010,   201,
    1015,    70,  1511,   977,   977,    32,   133,   526,   953,   178,
     158,   136,     9,   360,  1022,   195,  1525,    27,    28,   158,
      14,   540,   369,   191,     9,   977,     9,  1470,   180,   376,
     195,     9,   551,    14,   201,  1591,   132,   201,     9,   386,
    1025,  1023,  1027,   201,  1056,   198,    14,   189,  1418,   195,
     397,  1063,   571,  1418,  1418,   360,  1128,   195,   201,   194,
     102,  1070,   195,   158,   369,  1017,   371,   196,   383,     9,
     158,   376,   387,   196,   593,   594,   136,     9,   195,   194,
      70,   386,    70,  1025,  1490,  1027,  1492,   194,  1494,   197,
      70,  1497,  1601,     9,   925,   198,    14,   928,   413,   196,
     415,   416,   417,   418,   180,   360,   411,   626,   627,   940,
       9,   942,  1128,  1128,   369,   197,   371,    14,   201,   197,
      14,   376,   195,    50,    51,    52,    53,    54,    55,  1499,
      57,   386,   196,   191,  1499,  1499,    50,    51,    52,    53,
      54,    55,    69,    32,   194,  1111,  1161,  1134,  1134,  1134,
     194,    32,  1134,  1792,  1134,    69,    14,   194,   194,  1174,
      14,    52,   194,    70,    70,    70,   513,   158,   194,     9,
    1185,  1186,   195,   194,   196,   694,   196,   136,    14,   180,
     136,   158,     9,  1195,   195,  1591,  1017,    69,     9,  1134,
    1746,  1747,  1134,   201,   198,    83,   198,   196,     9,   194,
     136,   194,    14,    83,   196,  1213,  1221,   196,   513,   195,
     197,   195,   194,   136,     9,   194,  1231,   227,    91,    32,
    1165,   197,   197,  1168,   155,   197,    77,   201,   195,  1738,
     196,  1870,   196,   180,  1216,   136,    32,  1068,   195,  1748,
    1252,   195,     9,  1228,  1256,     9,   551,  1259,   201,   201,
     201,   136,     9,   195,  1266,   198,     9,   195,   513,   196,
      14,   780,   196,   782,    83,   198,   194,  1906,   197,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   195,   802,  1793,   195,  1228,   195,   197,   201,
       9,   194,   196,   195,   136,   195,     9,   816,   817,   136,
    1315,     9,   195,   201,    32,   201,   196,  1322,   195,  1453,
     196,  1326,   195,  1328,   136,   112,     4,   196,   167,   197,
    1448,  1336,    67,    68,  1309,   196,   117,    14,   163,  1838,
     195,  1346,  1347,    83,  1319,   854,   195,   197,   685,   195,
    1746,  1747,   861,   862,   136,    14,   136,   179,    83,   197,
     360,   196,    14,    14,  1366,   194,    83,   195,  1370,   369,
      81,    49,   195,  1375,  1309,   136,   376,   136,   196,    14,
     196,   890,  1203,    14,  1319,    14,   386,   197,   196,     9,
     685,     9,   103,   198,    59,    83,   179,  1558,   194,   134,
     135,    83,     9,   197,   913,   196,  1905,   115,   745,   102,
    1345,  1910,   158,   102,   180,   170,    14,    36,   194,   928,
     195,   194,   196,   176,   180,   934,    83,   195,   139,   140,
     141,   940,   180,   942,   112,   173,     9,    83,   196,   117,
     685,   119,   120,   121,   122,   123,   124,   125,   193,   786,
     745,   788,   163,  1428,   165,   166,   167,   168,   169,   170,
    1530,   195,  1467,   195,   197,  1470,    14,   976,    83,     9,
      14,    83,    14,    83,    83,    14,  1111,   814,  1453,   988,
     989,   990,   193,   194,  1861,   478,   483,   165,   166,   970,
     168,   786,   480,   788,   916,  1877,  1428,  1600,  1473,  1222,
     745,  1872,   597,  1394,  1479,  1587,  1481,   802,  1017,  1625,
    1538,   189,  1710,   513,  1023,  1447,  1025,  1917,  1027,   814,
     198,  1443,   817,  1893,  1583,  1722,  1501,  1439,  1503,   380,
    1067,  1200,  1064,  1138,  1533,   989,  1508,  1512,  1473,  1048,
     877,   786,     4,   788,  1479,  1196,  1481,  1197,  1012,  1484,
      78,    79,    80,    81,   940,  1904,   893,   894,   377,   854,
     424,  1070,   829,  1919,  1827,  1726,  1501,  1431,  1099,   814,
    1575,  1503,  1119,  1049,    -1,   103,    -1,    -1,    -1,    -1,
    1512,    -1,   877,    -1,    -1,    -1,    81,    49,   925,    -1,
    1099,    -1,    -1,    -1,    -1,    -1,    -1,  1418,   893,   894,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,   139,   140,   141,  1589,    -1,    -1,    -1,   913,    -1,
      -1,    -1,    -1,  1598,    -1,  1134,    -1,  1599,  1600,  1604,
      -1,    -1,   877,   128,  1566,    -1,  1611,   165,   166,   934,
     168,   169,   170,    -1,   139,   140,   141,    -1,   893,   894,
     112,    -1,    -1,    -1,  1589,   117,    -1,   119,   120,   121,
     122,   123,   124,   125,    -1,   193,  1598,  1721,    -1,    -1,
     165,   166,  1604,   168,   169,   170,    -1,    -1,  1499,  1611,
      -1,    -1,    -1,    -1,    -1,   685,    -1,    -1,  1197,  1198,
    1199,  1200,  1029,  1030,    -1,    -1,  1205,    -1,   193,   194,
      -1,    -1,    -1,   165,   166,    -1,   168,  1216,    -1,    -1,
      -1,  1845,    -1,    -1,     4,    -1,  1786,    -1,    -1,  1228,
      -1,    -1,  1721,    -1,    -1,    -1,  1731,   189,  1023,  1238,
    1025,  1068,  1027,    -1,  1029,  1030,   198,   119,   120,   121,
     122,   123,   124,    -1,    -1,   745,    -1,  1865,   130,   131,
      27,    28,    -1,    -1,    31,    -1,    -1,    -1,    -1,    49,
    1762,  1763,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,  1752,    -1,    56,
      -1,    -1,    -1,    -1,  1029,  1030,   786,   169,   788,   171,
      -1,    -1,    -1,    -1,  1303,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,    -1,   186,    -1,    -1,   189,    -1,    -1,
      -1,    -1,    -1,    -1,   814,    -1,    -1,    67,    68,  1794,
    1752,    -1,   112,  1160,    -1,    -1,  1801,   117,    -1,   119,
     120,   121,   122,   123,   124,   125,    -1,    -1,    -1,  1134,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1359,    -1,  1361,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1836,  1794,    56,    -1,  1160,  1203,    -1,    -1,  1801,
    1845,    -1,  1209,    -1,    -1,   165,   166,   877,   168,    -1,
      -1,    81,    -1,  1858,   134,   135,    -1,  1224,  1225,    -1,
      -1,    -1,    -1,   893,   894,    -1,    -1,    -1,    -1,   189,
      -1,    -1,    -1,   103,  1836,    -1,    -1,    -1,   198,    -1,
    1915,    -1,    -1,    -1,  1209,  1160,    -1,  1852,    -1,  1428,
      -1,  1216,  1927,    -1,    -1,    -1,  1858,    -1,  1437,  1224,
    1225,    -1,  1937,  1228,  1443,  1940,    -1,  1872,    -1,   139,
     140,   141,    -1,  1918,    -1,   195,  1881,    -1,  1923,    -1,
     227,    -1,     4,    -1,    -1,    -1,    -1,    -1,    -1,   159,
      -1,    -1,   162,    -1,  1209,   165,   166,    -1,   168,   169,
     170,    -1,    -1,    -1,  1311,    -1,    56,    -1,    -1,  1224,
    1225,  1490,    -1,  1492,    -1,  1494,  1918,    -1,  1497,    -1,
      -1,  1923,    -1,   193,  1503,    -1,    -1,    49,   198,  1508,
     277,    -1,   279,  1512,    -1,    -1,    -1,    -1,    -1,    -1,
       4,    -1,    -1,    -1,    -1,    -1,  1311,    -1,    -1,    -1,
      -1,  1530,    -1,    -1,  1533,    -1,    -1,  1536,    -1,  1029,
    1030,    -1,    -1,    -1,    -1,    -1,    -1,  1546,    -1,    -1,
      -1,    -1,    -1,    -1,  1553,    -1,    -1,    -1,    -1,    -1,
      -1,  1560,    -1,  1562,    -1,    49,    -1,    -1,   335,  1568,
     112,    -1,    -1,    -1,    -1,   117,  1311,   119,   120,   121,
     122,   123,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1418,  1591,   360,   277,    -1,   279,    -1,    -1,  1598,
    1599,  1600,   369,    -1,    -1,  1604,    -1,    -1,    -1,   376,
      -1,    -1,  1611,    -1,    -1,    -1,    -1,    -1,    -1,   386,
      -1,    -1,    -1,   165,   166,    -1,   168,    -1,   112,    -1,
     397,    -1,    -1,   117,    -1,   119,   120,   121,   122,   123,
     124,   125,    -1,  1428,    -1,    -1,    -1,   189,    -1,    -1,
      -1,    -1,   335,    -1,   421,    -1,   198,   424,    -1,    -1,
      -1,    -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    -1,
    1160,    -1,  1499,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   165,   166,    -1,   168,    -1,    -1,    -1,    -1,    -1,
      -1,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   471,   189,    -1,   277,    49,   279,
      -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,  1503,  1209,
      -1,    -1,  1721,  1508,    -1,    -1,    -1,  1512,    -1,    -1,
      -1,    -1,    -1,    -1,  1224,  1225,    49,    -1,   421,    -1,
      -1,   424,    -1,  1742,    -1,    -1,   513,  1746,  1747,    -1,
      -1,    -1,    -1,  1752,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1761,    -1,    -1,   335,    -1,    -1,    -1,  1768,
    1769,   112,    -1,  1772,  1773,    -1,   117,    -1,   119,   120,
     121,   122,   123,   124,   125,    -1,    -1,  1786,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1794,    -1,    -1,   565,   112,
     567,    -1,  1801,   570,   117,    -1,   119,   120,   121,   122,
     123,   124,   125,  1598,  1599,  1600,    -1,    -1,    -1,  1604,
      -1,  1311,    -1,    -1,   165,   166,  1611,   168,    -1,    -1,
      -1,    -1,    -1,    -1,   601,    -1,    -1,  1836,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1844,    -1,    -1,   189,    31,
      -1,   421,   165,   166,   424,   168,    -1,   198,    -1,  1858,
      -1,    -1,    -1,    -1,    -1,  1864,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   189,    -1,    -1,    -1,
      -1,    -1,   565,    -1,   567,   198,    -1,    -1,   655,   656,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   664,    -1,    81,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   685,  1918,
      -1,   103,    30,    31,  1923,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
      -1,    69,    -1,    -1,    -1,    -1,    -1,  1752,    -1,    -1,
      -1,    -1,   655,   656,    -1,    -1,    -1,   159,   745,    -1,
     162,   664,    -1,   165,   166,    -1,   168,   169,   170,    -1,
     172,    -1,    -1,    -1,    -1,   565,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1794,
      -1,   193,    -1,   780,    -1,    -1,  1801,    -1,    -1,   786,
      31,   788,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,   814,   815,    -1,
      -1,  1836,    -1,    -1,    -1,   822,    -1,    -1,    69,    -1,
      -1,    -1,   829,   830,   831,   832,   833,   834,   835,    -1,
      -1,    -1,    -1,  1858,   841,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   655,   656,    -1,   855,    -1,
      -1,    -1,    -1,   201,   664,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     877,    -1,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   891,    -1,   893,   894,    -1,    -1,
      -1,    -1,    -1,  1918,   103,    -1,    -1,    -1,  1923,   822,
      -1,    -1,    -1,    -1,    -1,    -1,   829,   830,   915,   916,
      67,    68,    -1,    -1,    -1,    -1,    -1,    -1,   925,    -1,
      -1,    -1,    -1,    -1,   931,    -1,    -1,    81,    -1,    -1,
     139,   140,   141,    -1,    -1,    -1,    -1,   944,    -1,    -1,
      -1,    -1,    -1,    -1,   195,   952,    -1,    -1,   955,   103,
      -1,    -1,    -1,    -1,   163,    -1,   165,   166,    -1,   168,
     169,   170,    -1,    -1,    -1,    -1,   973,    -1,    -1,    -1,
     977,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,
      -1,    -1,    -1,   990,   193,   139,   140,   141,   197,    -1,
     199,    -1,   915,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,   931,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,   829,
     830,   944,  1029,  1030,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1043,    -1,   195,   193,
    1047,    -1,  1049,    -1,    -1,    -1,    31,    67,    68,    -1,
     973,    -1,    -1,    -1,    -1,    -1,    -1,  1064,  1065,  1066,
    1067,  1068,    -1,    -1,  1071,  1072,  1073,  1074,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,  1095,  1096,
    1097,  1098,    -1,    27,    28,    -1,    81,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   915,    91,    -1,  1115,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,    -1,   103,    -1,
    1043,   931,    -1,    -1,  1047,    -1,  1049,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   944,    -1,    -1,    -1,    -1,    -1,
      -1,  1148,    -1,  1150,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1160,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,    -1,   973,    -1,    -1,    -1,    -1,    -1,    -1,
    1177,    -1,    -1,  1180,   159,    -1,    -1,   162,    -1,    -1,
     165,   166,    -1,   168,   169,   170,    -1,   172,    -1,    -1,
    1197,  1198,  1199,  1200,    -1,    -1,  1203,    -1,  1205,    -1,
      -1,    -1,  1209,    -1,    -1,    -1,    -1,    -1,   193,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1224,  1225,    -1,
    1227,    -1,    -1,    -1,    -1,  1148,    -1,  1150,    -1,    -1,
    1237,    -1,    -1,  1043,    -1,    -1,  1243,  1047,    -1,  1246,
      -1,  1248,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1177,    -1,    -1,  1180,    -1,    -1,
      -1,    -1,    -1,  1270,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      81,    57,    -1,   227,    -1,    -1,    -1,  1304,  1305,    -1,
      81,  1308,    -1,    69,  1311,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,  1237,    -1,    -1,    -1,    -1,    -1,
    1243,    -1,   103,    -1,    -1,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,  1148,    57,
    1150,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,    69,  1359,    -1,  1361,    -1,    -1,    -1,   139,   140,
     141,    -1,    -1,    -1,    -1,    31,    -1,  1177,    -1,    -1,
    1180,   162,    -1,    -1,   165,   166,    -1,   168,   169,   170,
      -1,  1304,  1305,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    -1,    59,    -1,  1402,    -1,  1404,    -1,    -1,
      -1,    -1,   193,  1410,    -1,  1412,   197,  1414,    -1,    -1,
    1417,  1418,   193,   194,  1421,    81,  1423,    -1,    -1,  1426,
      -1,    -1,    -1,    -1,    -1,    -1,   360,  1237,    -1,    -1,
    1437,  1438,   198,  1243,  1441,   369,    31,   103,    -1,    -1,
      -1,  1448,   376,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,   386,   119,   120,   121,   122,   123,   124,    -1,
      -1,    -1,    -1,   397,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,    -1,    -1,  1402,
      -1,  1404,    -1,  1490,    -1,  1492,    81,  1494,    -1,    -1,
    1497,    -1,  1499,   159,  1304,  1305,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,    -1,  1513,    -1,   103,  1516,
      -1,    -1,    -1,    -1,  1437,    -1,   111,   183,    -1,    -1,
      -1,  1528,  1529,   189,    -1,  1448,    -1,   193,   194,  1536,
      -1,  1538,    -1,    -1,    -1,    -1,    -1,   471,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,  1560,    -1,  1562,    -1,    -1,    -1,    -1,
      -1,  1568,    -1,    -1,   159,    -1,    -1,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,   513,
      -1,    -1,    -1,    -1,  1591,    -1,    -1,    -1,   183,    -1,
    1513,    -1,  1402,    -1,  1404,    -1,    -1,    -1,   193,   194,
      -1,  1608,  1609,  1610,    -1,    -1,    -1,    -1,  1615,    -1,
    1617,    -1,    -1,  1536,    -1,    -1,  1623,    -1,  1625,    -1,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,   570,  1560,  1448,  1562,
      -1,    -1,    -1,    -1,    -1,  1568,    69,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   601,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
    1623,    -1,    -1,  1513,    -1,    -1,    81,    69,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1725,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,  1742,    -1,    -1,    -1,  1746,
    1747,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   685,    -1,    -1,  1761,    -1,    -1,    -1,    -1,    -1,
    1767,    -1,    -1,    -1,   139,   140,   141,    -1,    -1,    -1,
      -1,  1778,    -1,    -1,    -1,    -1,    -1,  1784,   139,   140,
     141,  1788,    -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,    -1,
      -1,    81,  1725,  1810,   165,   166,    -1,   168,   169,   170,
      -1,   745,    -1,  1623,    -1,    -1,    -1,    -1,   193,  1742,
      -1,    -1,   197,   103,   199,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   193,   194,    -1,    -1,   198,    -1,  1761,    -1,
      -1,    -1,    -1,  1850,    27,    28,   780,    -1,    31,    -1,
      -1,    -1,   786,  1860,   788,    -1,    -1,    -1,    -1,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1877,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,  1886,
     814,   815,   162,    -1,    -1,   165,   166,  1810,   168,   169,
     170,  1898,    -1,  1900,    -1,    -1,    -1,   831,   832,   833,
     834,   835,    -1,    -1,    59,    -1,    -1,   841,    -1,    -1,
      -1,    -1,  1919,   193,  1921,  1725,    -1,    -1,    -1,    -1,
      -1,   855,    -1,    31,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   570,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   877,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    59,    -1,    -1,    -1,    -1,    -1,   891,    -1,   893,
     894,    -1,    -1,  1886,    -1,   601,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,  1898,    -1,  1900,    -1,    -1,
      -1,    -1,   916,   138,   139,   140,   141,   142,    -1,    -1,
      -1,   925,    -1,    -1,    -1,   103,  1919,    -1,  1921,    -1,
    1810,    -1,    -1,    -1,   159,    -1,    -1,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,   172,   952,    -1,
      -1,   955,    -1,    -1,    -1,    -1,    -1,    -1,   183,    -1,
     138,   139,   140,   141,   142,    -1,    -1,    -1,   193,   194,
      -1,    -1,    -1,   977,   227,    -1,    -1,    -1,    -1,    -1,
      -1,   159,    -1,    -1,   162,   163,   990,   165,   166,    -1,
     168,   169,   170,    -1,   172,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   183,  1886,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   193,   194,    -1,  1898,    -1,
    1900,    -1,    -1,    -1,    -1,  1029,  1030,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1919,
      -1,  1921,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1064,  1065,  1066,  1067,  1068,    -1,    -1,  1071,  1072,  1073,
    1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,
    1094,  1095,  1096,  1097,  1098,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   360,    -1,   815,
      -1,  1115,    -1,    -1,    -1,    -1,   369,    -1,    -1,    -1,
      -1,    -1,    -1,   376,    -1,   831,   832,   833,   834,    -1,
      -1,    -1,    -1,   386,    -1,   841,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,   397,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1160,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1197,  1198,  1199,  1200,    69,    -1,  1203,
      -1,  1205,    -1,    -1,    -1,  1209,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   471,    -1,
    1224,  1225,    -1,  1227,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,  1246,    -1,  1248,    -1,   952,    10,    11,    12,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     513,    -1,   103,    -1,    -1,    -1,  1270,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,   138,   139,   140,
     141,    -1,    -1,    -1,  1308,    -1,    69,  1311,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    -1,    -1,   570,   159,    -1,
      -1,   162,   163,    -1,   165,   166,   198,   168,   169,   170,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   183,    -1,    -1,    -1,    -1,    -1,   601,    -1,
      -1,    -1,   193,    -1,    -1,  1359,    -1,  1361,    -1,  1065,
    1066,  1067,    -1,    -1,    -1,  1071,  1072,  1073,  1074,  1075,
    1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,  1095,
    1096,  1097,  1098,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1410,    -1,  1412,  1115,
    1414,    -1,    -1,  1417,  1418,    -1,    81,  1421,    -1,  1423,
      -1,    -1,  1426,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,   685,    -1,  1438,   198,    -1,  1441,   103,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,  1490,    -1,  1492,    -1,
    1494,    31,   745,  1497,   159,  1499,    -1,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    81,    -1,
      -1,    -1,  1516,    -1,    -1,    -1,    -1,    -1,    -1,    59,
      -1,  1227,    -1,    -1,  1528,  1529,    -1,    -1,   193,    -1,
     103,    -1,    -1,   786,  1538,   788,    -1,    -1,    -1,    -1,
    1246,    81,  1248,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   814,   815,   103,  1270,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   831,   832,
     833,   834,   835,    -1,    -1,    -1,   159,  1591,   841,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,   138,   139,
     140,   141,   142,    -1,  1608,  1609,  1610,    -1,    -1,    -1,
      -1,  1615,   198,  1617,    -1,    -1,    -1,    -1,    -1,   159,
     193,  1625,   162,   163,   877,   165,   166,    -1,   168,   169,
     170,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,
     893,   894,    -1,   183,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   193,   194,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,   112,
      81,    -1,   925,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    83,    -1,    85,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,   139,   140,   141,   952,
     111,   112,   103,    -1,  1410,    -1,  1412,    -1,  1414,    -1,
      -1,  1417,    -1,    -1,    -1,  1421,    -1,  1423,    -1,   162,
    1426,    -1,   165,   166,   977,   168,   169,   170,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,    -1,  1746,  1747,    -1,    -1,    -1,    -1,    -1,    -1,
     193,   162,    -1,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    -1,  1767,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    -1,    -1,  1778,    -1,  1029,  1030,    -1,    -1,
    1784,    -1,   193,    -1,  1788,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   193,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1516,  1064,  1065,  1066,  1067,  1068,    -1,    -1,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
    1093,  1094,  1095,  1096,  1097,  1098,  1850,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,  1860,    -1,    -1,    -1,
      -1,    -1,  1115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,  1877,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,  1608,  1609,  1610,    -1,    -1,  1160,    -1,  1615,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    81,    -1,    -1,
    1203,    -1,    -1,    -1,    -1,    -1,  1209,    91,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,   103,
      -1,  1224,  1225,    -1,  1227,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1246,    -1,  1248,    -1,    -1,    -1,    -1,
      70,    -1,    -1,    -1,    -1,   139,   140,   141,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,  1270,    -1,    -1,
      -1,    91,    -1,    -1,    -1,   159,    -1,    -1,   162,    -1,
      -1,   165,   166,   103,   168,   169,   170,    -1,    -1,   198,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,  1311,   193,
      -1,  1767,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,  1778,   143,   144,   145,   146,   147,  1784,    -1,
      -1,    -1,  1788,    -1,   154,    -1,    -1,   198,    -1,   159,
     160,   161,   162,   163,    -1,   165,   166,    -1,   168,   169,
     170,    -1,    -1,    -1,   174,    -1,     3,     4,    -1,     6,
       7,    -1,    -1,    10,    11,    12,    13,    -1,   188,    -1,
      -1,    -1,    -1,   193,   194,    -1,    -1,    -1,    -1,   199,
      27,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1850,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1410,    -1,  1412,
      57,  1414,    -1,    -1,  1417,  1418,    -1,    -1,  1421,    -1,
    1423,    -1,    -1,  1426,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      81,   128,    -1,   130,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,  1499,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   159,  1516,    -1,    -1,    -1,    -1,   165,   166,
      -1,   168,   169,   170,   171,    -1,   173,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,
     197,    -1,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   162,    -1,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   193,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,  1608,  1609,  1610,    -1,    -1,
      -1,    -1,  1615,    -1,    -1,    -1,    -1,    -1,    49,    50,
      51,  1624,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
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
      -1,    -1,   193,   194,  1767,   196,   197,   198,   199,   200,
      -1,   202,   203,    -1,    -1,  1778,    -1,    -1,    -1,    -1,
      -1,  1784,    -1,    -1,    -1,  1788,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,  1812,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      50,    51,    -1,    -1,    -1,    -1,    56,  1850,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,   114,    -1,   116,   117,   118,   119,
     120,   121,   122,   123,   124,    -1,   126,   127,   128,   129,
     130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,   143,   144,   145,   146,   147,    -1,    -1,
      -1,   151,    -1,    -1,   154,    -1,    -1,    -1,    81,   159,
     160,   161,   162,   163,    -1,   165,   166,    -1,   168,   169,
     170,   171,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,
     103,    -1,    -1,   183,   184,    -1,   186,    -1,   188,   189,
     190,    -1,    -1,   193,   194,    -1,   196,   197,   198,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,   139,   140,   141,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,
      49,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
     193,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,    98,
      -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
     109,   110,   111,   112,   113,   114,    -1,   116,   117,   118,
     119,   120,   121,   122,   123,   124,    -1,   126,   127,   128,
     129,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,    -1,   143,   144,   145,   146,   147,    -1,
      -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,    -1,
     159,   160,   161,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,    -1,
      -1,    -1,    -1,    -1,   183,   184,    -1,   186,    -1,   188,
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
      94,    95,    96,    -1,    98,    -1,   100,    -1,    -1,   103,
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
      93,    94,    -1,    96,    -1,    98,    -1,   100,   101,    -1,
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,
     113,   114,    -1,   116,    -1,   118,   119,   120,   121,   122,
     123,   124,    -1,   126,   127,   128,    -1,   130,   131,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,
     143,   144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,
      -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,   171,    -1,
      -1,   174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,
     183,    -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,
     193,   194,    -1,   196,   197,    -1,   199,   200,    -1,   202,
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
      91,    92,    93,    94,    -1,    96,    -1,    98,    -1,   100,
      -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,    -1,   113,   114,    -1,   116,    -1,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,   127,   128,    -1,   130,
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
     151,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
     171,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,
      -1,    -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,
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
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    99,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,    -1,   113,   114,    -1,   116,    -1,   118,   119,
     120,   121,   122,   123,   124,    -1,   126,   127,   128,    -1,
     130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,   143,   144,   145,   146,   147,    -1,    -1,
      -1,   151,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,
     160,   161,   162,   163,    -1,   165,   166,    -1,   168,   169,
     170,   171,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,
      -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,   188,   189,
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
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    97,
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
      -1,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
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
     194,    -1,   196,   197,   198,   199,   200,    -1,   202,   203,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,   194,    -1,   196,   197,
      -1,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     137,   138,   139,   140,   141,    -1,   143,   144,   145,   146,
     147,    -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,
     177,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,
      -1,   188,   189,   190,    -1,    -1,   193,   194,    -1,   196,
     197,    -1,   199,   200,    -1,   202,   203,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    -1,
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
      -1,   196,    -1,    -1,   199,   200,    -1,   202,   203,     3,
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
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,   172,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
     194,    -1,    -1,    -1,    -1,   199,   200,    -1,   202,   203,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,
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
     193,   194,    -1,    -1,   197,    -1,   199,   200,    -1,   202,
     203,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    50,    51,
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
     172,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,    -1,
      -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,    -1,
      -1,   193,   194,    -1,    -1,    -1,    -1,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,   108,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,   143,   144,   145,   146,   147,    -1,    -1,
      -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,
     160,   161,   162,   163,    -1,   165,   166,    -1,   168,   169,
     170,    -1,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,
      -1,    -1,   198,   183,    -1,    -1,    -1,    -1,   188,   189,
     190,    -1,    -1,   193,   194,    -1,    -1,    -1,    -1,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
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
     189,   190,    -1,    -1,   193,   194,    10,    11,    12,    -1,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,   198,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,   194,    -1,   196,    11,
      12,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    69,    -1,    56,
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
      -1,   137,   138,   139,   140,   141,    -1,   143,   144,   145,
     146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,
      -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,    -1,    -1,    -1,   174,    -1,
      -1,   177,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,
      -1,    -1,   188,   189,   190,    -1,    -1,   193,   194,    10,
      11,    12,    -1,   199,   200,    -1,   202,   203,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
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
      -1,    -1,   137,   138,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,   174,
      -1,    -1,   177,    -1,    -1,    -1,    -1,   198,   183,    -1,
      -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,   194,
     195,    -1,    -1,    -1,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    32,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
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
      13,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    50,    51,    -1,
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
      -1,    13,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    50,    51,
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
      -1,    -1,    13,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    38,    -1,    -1,
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
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,
      -1,    -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,
      -1,    -1,   193,   194,    -1,    -1,    -1,    -1,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    13,    30,    31,    32,    33,    34,
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
      29,   199,   200,    -1,   202,   203,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
      -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
     159,    10,    11,    12,    13,    -1,   165,   166,    -1,   168,
     169,   170,   171,    -1,   173,    -1,    -1,   176,    27,    -1,
      29,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,   197,    -1,
     199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
      -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     159,    -1,    -1,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,   171,    -1,   173,    -1,    -1,   176,     3,     4,
      -1,     6,     7,    -1,   183,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   193,   194,    -1,    -1,    -1,   198,
      -1,    -1,    27,    -1,    29,    31,    31,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    57,    -1,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,    -1,    -1,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   159,    -1,    -1,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,   171,    -1,   173,    -1,
      -1,   176,     3,     4,    -1,     6,     7,    -1,   183,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,   193,   194,
      -1,    -1,    -1,   198,    -1,    -1,    27,    -1,    29,    -1,
      31,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    57,    -1,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,    -1,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,
      -1,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
     171,    -1,   173,    -1,    -1,   176,     3,     4,    -1,     6,
       7,    -1,   183,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,   193,   194,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    29,    -1,    31,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    59,    -1,    -1,    -1,    69,    -1,    -1,    -1,
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
      -1,     3,     4,    -1,     6,     7,   183,   184,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,   193,   194,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,    31,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    59,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
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
      -1,   173,    -1,    -1,   176,     3,     4,     5,     6,     7,
      -1,   183,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   193,   194,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    57,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   159,   160,   161,    -1,    -1,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,   173,   174,    -1,   176,    -1,
      -1,    -1,    -1,    -1,    -1,   183,   184,    -1,   186,    -1,
     188,   189,    -1,     3,     4,   193,     6,     7,    11,    12,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    31,    29,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
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
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,   196,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,   196,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   196,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,   196,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,   196,    -1,    -1,    -1,    56,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    -1,   103,    31,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,    69,   143,   144,   145,   146,
     147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
     195,   168,   169,   170,    50,    51,    -1,   174,    -1,    -1,
      56,    -1,    58,    -1,    -1,    -1,   183,    -1,    -1,    -1,
      -1,   188,    -1,    -1,    70,    -1,   193,   194,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    -1,    10,    11,    12,
      -1,    -1,   136,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    31,    -1,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,    69,   143,   144,   145,
     146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,
      -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,    70,    -1,    -1,   174,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,   183,    83,    84,
      -1,    -1,   188,    -1,    -1,    -1,    91,   193,   194,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    70,    -1,    -1,   174,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,   188,    -1,    -1,    -1,    91,   193,   194,
      -1,    -1,   197,    -1,   199,    -1,    -1,    -1,    -1,   103,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
     124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    70,    -1,    72,
     174,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,   188,    -1,    -1,    -1,    91,   193,
     194,    -1,    -1,    -1,    -1,   199,    -1,    -1,    -1,    -1,
     103,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,   138,   139,   140,   141,    -1,
     143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,
      -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,    70,    -1,
      -1,   174,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,   188,    -1,    -1,    -1,    91,
     193,   194,    -1,    -1,    -1,    -1,   199,    -1,    -1,    -1,
      -1,   103,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,   137,   138,   139,   140,   141,
      -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,    70,
      -1,    -1,   174,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,   188,    -1,    -1,    -1,
      91,   193,   194,    -1,    -1,    -1,    -1,   199,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    -1,   174,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   188,    -1,    -1,
      -1,    -1,   193,   194,    -1,    -1,    30,    31,   199,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
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
      -1,    -1,    30,    31,   136,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     136,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,   136,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,   136,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,   136,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    -1,   136,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
      -1,    -1,    -1,    69,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   136,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,
      -1,    -1,    -1,   130,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,    -1,   143,   144,   145,   146,
     147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,
     136,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,    -1,    -1,    -1,   174,    -1,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,   188,    91,    -1,    -1,    -1,   193,   194,    -1,    -1,
      -1,    -1,   199,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,    -1,   143,   144,   145,   146,   147,    -1,
      -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,
     159,   160,   161,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,    -1,    -1,    -1,   174,    -1,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,   188,
      91,    -1,    -1,    -1,   193,   194,    -1,    -1,    -1,    -1,
     199,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,   188,    -1,    -1,
      -1,    -1,   193,   194,    -1,    -1,    -1,    -1,   199,    30,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69
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
     389,   399,   400,   401,   403,   408,   413,   433,   441,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   456,   469,   471,   473,   122,   123,   124,   137,   159,
     169,   194,   211,   244,   325,   346,   445,   346,   194,   346,
     346,   346,   108,   346,   346,   346,   431,   432,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
      81,    83,    91,   124,   139,   140,   141,   154,   194,   222,
     365,   400,   403,   408,   445,   448,   445,    38,   346,   460,
     461,   346,   124,   130,   194,   222,   257,   400,   401,   402,
     404,   408,   442,   443,   444,   452,   457,   458,   194,   335,
     405,   194,   335,   356,   336,   346,   230,   335,   194,   194,
     194,   335,   196,   346,   211,   196,   346,     3,     4,     6,
       7,    10,    11,    12,    13,    27,    29,    31,    57,    59,
      71,    72,    73,    74,    75,    76,    77,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   128,   130,   131,   132,   133,   137,   138,   142,   159,
     163,   171,   173,   176,   183,   194,   211,   212,   213,   224,
     474,   494,   495,   498,   196,   341,   343,   346,   197,   237,
     346,   111,   112,   162,   214,   215,   216,   217,   221,    83,
     199,   291,   292,   123,   130,   122,   130,    83,   293,   194,
     194,   194,   194,   211,   263,   477,   194,   194,    70,    70,
      70,   336,    83,    90,   155,   156,   157,   466,   467,   162,
     197,   221,   221,   211,   264,   477,   163,   194,   477,   477,
      83,   190,   197,   357,    27,   334,   338,   346,   347,   445,
     449,   226,   197,    90,   406,   466,    90,   466,   466,    32,
     162,   179,   478,   194,     9,   196,    38,   243,   163,   262,
     477,   124,   189,   244,   326,   196,   196,   196,   196,   196,
     196,   196,   196,    10,    11,    12,    30,    31,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    57,    69,   196,    70,    70,   197,   158,   131,   169,
     171,   184,   186,   265,   324,   325,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    67,
      68,   134,   135,   435,    70,   197,   440,   194,   194,    70,
     197,   199,   453,   194,   243,   244,    14,   346,   196,   136,
      48,   211,   430,    90,   334,   347,   158,   445,   136,   201,
       9,   415,   258,   334,   347,   445,   478,   158,   194,   407,
     435,   440,   195,   346,    32,   228,     8,   358,     9,   196,
     228,   229,   336,   337,   346,   211,   277,   232,   196,   196,
     196,   138,   142,   498,   498,   179,   497,   194,   111,   498,
      14,   158,   138,   142,   159,   211,   213,   196,   196,   196,
     238,   115,   176,   196,   214,   216,   214,   216,   221,   197,
       9,   416,   196,   102,   162,   197,   445,     9,   196,   130,
     130,    14,     9,   196,   445,   470,   336,   334,   347,   445,
     448,   449,   195,   179,   255,   137,   445,   459,   460,   346,
     366,   367,   336,   381,   381,   196,    70,   435,   155,   467,
      82,   346,   445,    90,   155,   467,   221,   210,   196,   197,
     250,   260,   390,   392,    91,   194,   199,   359,   360,   362,
     399,   403,   451,   453,   471,    14,   102,   472,   353,   354,
     355,   287,   288,   433,   434,   195,   195,   195,   195,   195,
     198,   227,   228,   245,   252,   259,   433,   346,   200,   202,
     203,   211,   479,   480,   498,    38,   172,   289,   290,   346,
     474,   194,   477,   253,   243,   346,   346,   346,   346,    32,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   404,   346,   346,   455,   455,   346,
     462,   463,   130,   197,   212,   213,   452,   453,   263,   211,
     264,   477,   477,   262,   244,    38,   338,   341,   343,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   163,   197,   211,   436,   437,   438,   439,   452,
     455,   346,   289,   289,   455,   346,   459,   243,   195,   346,
     194,   429,     9,   415,   195,   195,    38,   346,    38,   346,
     407,   195,   195,   195,   452,   289,   197,   211,   436,   437,
     452,   195,   226,   281,   197,   343,   346,   346,    94,    32,
     228,   275,   196,    28,   102,    14,     9,   195,    32,   197,
     278,   498,    31,    91,   172,   224,   491,   492,   493,   194,
       9,    50,    51,    56,    58,    70,   138,   139,   140,   141,
     183,   194,   222,   373,   376,   379,   385,   400,   408,   409,
     411,   412,   211,   496,   226,   194,   236,   197,   196,   197,
     196,   102,   162,   111,   112,   162,   217,   218,   219,   220,
     221,   217,   211,   346,   292,   409,    83,     9,   195,   195,
     195,   195,   195,   195,   195,   196,    50,    51,   487,   489,
     490,   132,   268,   194,     9,   195,   195,   136,   201,     9,
     415,     9,   415,   201,   201,    83,    85,   211,   468,   211,
      70,   198,   198,   207,   209,    32,   133,   267,   178,    54,
     163,   178,   394,   347,   136,     9,   415,   195,   158,   498,
     498,    14,   358,   287,   226,   191,     9,   416,   498,   499,
     435,   440,   435,   198,     9,   415,   180,   445,   346,   195,
       9,   416,    14,   350,   246,   132,   266,   194,   477,   346,
      32,   201,   201,   136,   198,     9,   415,   346,   478,   194,
     256,   251,   261,    14,   472,   254,   243,    72,   445,   346,
     478,   201,   198,   195,   195,   201,   198,   195,    50,    51,
      70,    78,    79,    80,    91,   138,   139,   140,   141,   154,
     183,   211,   374,   377,   380,   400,   411,   418,   420,   421,
     425,   428,   211,   445,   445,   136,   266,   435,   440,   195,
     346,   282,    75,    76,   283,   226,   335,   226,   337,   102,
      38,   137,   272,   445,   409,   211,    32,   228,   276,   196,
     279,   196,   279,     9,   415,    91,   224,   136,   158,     9,
     415,   195,   172,   479,   480,   481,   479,   409,   409,   409,
     409,   409,   414,   417,   194,    70,    70,    70,   194,   409,
     158,   197,    10,    11,    12,    31,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    69,   158,
     478,   198,   400,   197,   240,   216,   216,   211,   217,   217,
     221,     9,   416,   198,   198,    14,   445,   196,   180,     9,
     415,   211,   269,   400,   197,   459,   137,   445,    14,   346,
     346,   201,   346,   198,   207,   498,   269,   197,   393,    14,
     195,   346,   359,   452,   196,   498,   191,   198,    32,   485,
     434,    38,    83,   172,   436,   437,   439,   436,   437,   498,
      38,   172,   346,   409,   287,   194,   400,   267,   351,   247,
     346,   346,   346,   198,   194,   289,   268,    32,   267,   498,
      14,   266,   477,   404,   198,   194,    14,    78,    79,    80,
     211,   419,   419,   421,   423,   424,    52,   194,    70,    70,
      70,    90,   155,   194,   158,     9,   415,   195,   429,    38,
     346,   267,   198,    75,    76,   284,   335,   228,   198,   196,
      95,   196,   272,   445,   194,   136,   271,    14,   226,   279,
     105,   106,   107,   279,   198,   498,   180,   136,   158,   498,
     211,   172,   491,     9,   195,   415,   136,   201,     9,   415,
     414,   368,   369,   409,   382,   409,   410,   382,   359,   361,
     363,   195,   130,   212,   409,   464,   465,   409,   409,   409,
      32,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   496,    83,   241,   198,   198,
     220,   196,   409,   490,   102,   103,   486,   488,     9,   297,
     195,   194,   338,   343,   346,   136,   201,   198,   472,   297,
     164,   177,   197,   389,   396,   164,   197,   395,   136,   196,
     485,   498,   358,   499,    83,   172,    14,    83,   478,   445,
     346,   195,   287,   197,   287,   194,   136,   194,   289,   195,
     197,   498,   197,   196,   498,   267,   248,   407,   289,   136,
     201,     9,   415,   420,   423,   370,   371,   421,   383,   421,
     422,   383,   155,   359,   426,   427,    81,   421,   445,   197,
     335,    32,    77,   228,   196,   337,   271,   459,   272,   195,
     409,   101,   105,   196,   346,    32,   196,   280,   198,   180,
     498,   211,   136,   172,    32,   195,   409,   409,   195,   201,
       9,   415,   136,   201,     9,   415,   201,   136,     9,   415,
     195,   136,   198,     9,   415,   409,    32,   195,   226,   196,
     196,   211,   498,   498,   486,   400,     4,   112,   117,   123,
     125,   165,   166,   168,   198,   298,   323,   324,   325,   330,
     331,   332,   333,   433,   459,   346,   198,   197,   198,    54,
     346,   346,   346,   358,    38,    83,   172,    14,    83,   346,
     194,   485,   195,   297,   195,   287,   346,   289,   195,   297,
     472,   297,   196,   197,   194,   195,   421,   421,   195,   201,
       9,   415,   136,   201,     9,   415,   201,   136,   195,     9,
     415,   297,    32,   226,   196,   195,   195,   195,   233,   196,
     196,   280,   226,   136,   498,   498,   136,   409,   409,   409,
     409,   359,   409,   409,   409,   197,   198,   488,   132,   133,
     184,   212,   475,   498,   270,   400,   112,   333,    31,   125,
     138,   142,   163,   169,   307,   308,   309,   310,   400,   167,
     315,   316,   128,   194,   211,   317,   318,   299,   244,   498,
       9,   196,     9,   196,   196,   472,   324,   195,   294,   163,
     391,   198,   198,    83,   172,    14,    83,   346,   289,   117,
     348,   485,   198,   485,   195,   195,   198,   197,   198,   297,
     287,   136,   421,   421,   421,   421,   359,   198,   226,   231,
     234,    32,   228,   274,   226,   498,   195,   409,   136,   136,
     136,   226,   400,   400,   477,    14,   212,     9,   196,   197,
     475,   472,   310,   179,   197,     9,   196,     3,     4,     5,
       6,     7,    10,    11,    12,    13,    27,    28,    29,    57,
      71,    72,    73,    74,    75,    76,    77,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   137,   138,
     143,   144,   145,   146,   147,   159,   160,   161,   171,   173,
     174,   176,   183,   184,   186,   188,   189,   211,   397,   398,
       9,   196,   163,   167,   211,   318,   319,   320,   196,    83,
     329,   243,   300,   475,   475,    14,   244,   198,   295,   296,
     475,    14,    83,   346,   195,   194,   485,   196,   197,   321,
     348,   485,   294,   198,   195,   421,   136,   136,    32,   228,
     273,   274,   226,   409,   409,   409,   198,   196,   196,   409,
     400,   303,   498,   311,   312,   408,   308,    14,    32,    51,
     313,   316,     9,    36,   195,    31,    50,    53,    14,     9,
     196,   213,   476,   329,    14,   498,   243,   196,    14,   346,
      38,    83,   388,   197,   226,   485,   321,   198,   485,   421,
     421,   226,    99,   239,   198,   211,   224,   304,   305,   306,
       9,   415,     9,   415,   198,   409,   398,   398,    59,   314,
     319,   319,    31,    50,    53,   409,    83,   179,   194,   196,
     409,   477,   409,    83,     9,   416,   226,   198,   197,   321,
      97,   196,   115,   235,   158,   102,   498,   180,   408,   170,
      14,   487,   301,   194,    38,    83,   195,   198,   226,   196,
     194,   176,   242,   211,   324,   325,   180,   409,   180,   285,
     286,   434,   302,    83,   198,   400,   240,   173,   211,   196,
     195,     9,   416,   119,   120,   121,   327,   328,   285,    83,
     270,   196,   485,   434,   499,   195,   195,   196,   193,   482,
     327,    38,    83,   172,   485,   197,   483,   484,   498,   196,
     197,   322,   499,    83,   172,    14,    83,   482,   226,     9,
     416,    14,   486,   226,    38,    83,   172,    14,    83,   346,
     322,   198,   484,   498,   198,    83,   172,    14,    83,   346,
      14,    83,   346,   346
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
#line 2208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval).reset();;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { (yyval).reset();;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { _p->onKeyset((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { _p->onKeyset((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { _p->onKeyset((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval).reset();;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval).reset();;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval).reset();;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
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

  case 601:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
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

  case 602:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { (yyval).reset();;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval).reset();;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2498 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { _p->onName((yyval), (yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2534 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { (yyval).reset();;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { (yyval).reset();;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { (yyval).reset();;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { (yyval).reset();;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2557 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2565 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2569 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2579 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2580 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2583 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2588 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2599 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2603 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2608 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2610 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2613 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2615 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2626 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2633 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2640 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2645 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2652 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2663 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2664 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2665 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2675 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2676 "hphp.y"
    { (yyval).reset();;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2680 "hphp.y"
    { (yyval).reset();;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2681 "hphp.y"
    { (yyval).reset();;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2684 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2685 "hphp.y"
    { (yyval).reset();;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2693 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2696 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2700 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2701 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2702 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2705 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2707 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2710 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2717 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2720 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2727 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2728 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2731 "hphp.y"
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2738 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2739 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2742 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2743 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2749 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2750 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2756 "hphp.y"
    { (yyval).reset();;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2761 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2763 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2766 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2771 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2777 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2782 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2785 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { (yyval).reset();;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2794 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2795 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2802 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2804 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2809 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2815 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2816 "hphp.y"
    { (yyval).reset();;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2820 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2821 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2827 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2831 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2833 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2848 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2856 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2857 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2861 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2863 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2868 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
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
#line 2887 "hphp.y"
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
#line 2902 "hphp.y"
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
#line 2914 "hphp.y"
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
#line 2926 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2927 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2929 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2931 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2933 "hphp.y"
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
#line 2950 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2952 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2954 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2959 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2970 "hphp.y"
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
#line 2979 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2991 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2994 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2995 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2996 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2999 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3001 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3005 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3010 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3016 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3020 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3027 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3036 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3040 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3044 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3053 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3054 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3059 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { _p->onPipeVariable((yyval));;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3068 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3069 "hphp.y"
    { (yyval).reset();;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3075 "hphp.y"
    { (yyval)++;;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3080 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3082 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3085 "hphp.y"
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

  case 925:

/* Line 1455 of yacc.c  */
#line 3096 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3097 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3101 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3102 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3105 "hphp.y"
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

  case 931:

/* Line 1455 of yacc.c  */
#line 3114 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3118 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3119 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3122 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3123 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3124 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3130 "hphp.y"
    { (yyval).reset();;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3135 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3136 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3140 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3142 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3143 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3144 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3149 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3150 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3154 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3155 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3156 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3157 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3162 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3163 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3168 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3170 "hphp.y"
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
#line 3177 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3179 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3180 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3182 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3187 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3189 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3191 "hphp.y"
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

  case 967:

/* Line 1455 of yacc.c  */
#line 3201 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3203 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3204 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3207 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3209 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3213 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3214 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3215 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3216 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3217 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3218 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3219 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3220 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3221 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3222 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3223 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3227 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3228 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3233 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3235 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3249 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3254 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3258 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3263 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 995:

/* Line 1455 of yacc.c  */
#line 3270 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 996:

/* Line 1455 of yacc.c  */
#line 3274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3275 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3281 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3285 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3291 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3295 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3302 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3303 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1004:

/* Line 1455 of yacc.c  */
#line 3307 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1005:

/* Line 1455 of yacc.c  */
#line 3310 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1006:

/* Line 1455 of yacc.c  */
#line 3316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3321 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3323 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1010:

/* Line 1455 of yacc.c  */
#line 3324 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3345 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1018:

/* Line 1455 of yacc.c  */
#line 3346 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1; ;}
    break;

  case 1021:

/* Line 1455 of yacc.c  */
#line 3355 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1024:

/* Line 1455 of yacc.c  */
#line 3366 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 1025:

/* Line 1455 of yacc.c  */
#line 3368 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 1026:

/* Line 1455 of yacc.c  */
#line 3372 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 1027:

/* Line 1455 of yacc.c  */
#line 3375 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 1028:

/* Line 1455 of yacc.c  */
#line 3379 "hphp.y"
    {;}
    break;

  case 1029:

/* Line 1455 of yacc.c  */
#line 3380 "hphp.y"
    {;}
    break;

  case 1030:

/* Line 1455 of yacc.c  */
#line 3381 "hphp.y"
    {;}
    break;

  case 1031:

/* Line 1455 of yacc.c  */
#line 3387 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 1032:

/* Line 1455 of yacc.c  */
#line 3392 "hphp.y"
    {
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   ;}
    break;

  case 1033:

/* Line 1455 of yacc.c  */
#line 3401 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 1034:

/* Line 1455 of yacc.c  */
#line 3407 "hphp.y"
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]), (yyvsp[(6) - (6)]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   ;}
    break;

  case 1035:

/* Line 1455 of yacc.c  */
#line 3415 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1036:

/* Line 1455 of yacc.c  */
#line 3416 "hphp.y"
    { ;}
    break;

  case 1037:

/* Line 1455 of yacc.c  */
#line 3422 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (3)]), true); ;}
    break;

  case 1038:

/* Line 1455 of yacc.c  */
#line 3424 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)]), false); ;}
    break;

  case 1039:

/* Line 1455 of yacc.c  */
#line 3425 "hphp.y"
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       ;}
    break;

  case 1040:

/* Line 1455 of yacc.c  */
#line 3430 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); ;}
    break;

  case 1041:

/* Line 1455 of yacc.c  */
#line 3436 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1042:

/* Line 1455 of yacc.c  */
#line 3441 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1043:

/* Line 1455 of yacc.c  */
#line 3446 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1044:

/* Line 1455 of yacc.c  */
#line 3450 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1045:

/* Line 1455 of yacc.c  */
#line 3455 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 1046:

/* Line 1455 of yacc.c  */
#line 3457 "hphp.y"
    { _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)])); (yyval) = (yyvsp[(2) - (5)]);;}
    break;

  case 1047:

/* Line 1455 of yacc.c  */
#line 3463 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1048:

/* Line 1455 of yacc.c  */
#line 3466 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1049:

/* Line 1455 of yacc.c  */
#line 3469 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1050:

/* Line 1455 of yacc.c  */
#line 3470 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1051:

/* Line 1455 of yacc.c  */
#line 3473 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1052:

/* Line 1455 of yacc.c  */
#line 3476 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1053:

/* Line 1455 of yacc.c  */
#line 3479 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1054:

/* Line 1455 of yacc.c  */
#line 3482 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1055:

/* Line 1455 of yacc.c  */
#line 3484 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1056:

/* Line 1455 of yacc.c  */
#line 3490 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1057:

/* Line 1455 of yacc.c  */
#line 3496 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1058:

/* Line 1455 of yacc.c  */
#line 3504 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1059:

/* Line 1455 of yacc.c  */
#line 3505 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14567 "hphp.5.tab.cpp"
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
#line 3508 "hphp.y"

/* !PHP5_ONLY*/
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}

