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
     T_UNRESOLVED_OP = 428
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
#line 894 "hphp.5.tab.cpp"

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
#define YYLAST   18213

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  203
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  292
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1048
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1926

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   428

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,   201,     2,   198,    55,    38,   202,
     193,   194,    53,    50,     9,    51,    52,    54,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    32,   195,
      43,    14,    44,    31,    59,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    70,     2,   200,    37,     2,   199,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   196,    36,   197,    58,     2,     2,     2,
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
     184,   185,   186,   187,   188,   189,   190,   191,   192
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
     237,   240,   244,   248,   250,   253,   255,   258,   262,   267,
     271,   273,   276,   278,   281,   284,   286,   290,   292,   296,
     299,   302,   305,   311,   316,   319,   320,   322,   324,   326,
     328,   332,   338,   347,   348,   353,   354,   361,   362,   373,
     374,   379,   382,   386,   389,   393,   396,   400,   404,   408,
     412,   416,   420,   426,   428,   430,   432,   433,   443,   444,
     455,   461,   462,   476,   477,   483,   487,   491,   494,   497,
     500,   503,   506,   509,   513,   516,   519,   523,   526,   529,
     530,   535,   545,   546,   547,   552,   555,   556,   558,   559,
     561,   562,   572,   573,   584,   585,   597,   598,   608,   609,
     620,   621,   630,   631,   641,   642,   650,   651,   660,   661,
     670,   671,   679,   680,   689,   691,   693,   695,   697,   699,
     702,   706,   710,   713,   716,   717,   720,   721,   724,   725,
     727,   731,   733,   737,   740,   741,   743,   746,   751,   753,
     758,   760,   765,   767,   772,   774,   779,   783,   789,   793,
     798,   803,   809,   815,   820,   821,   823,   825,   830,   831,
     837,   838,   841,   842,   846,   847,   855,   864,   871,   874,
     880,   887,   892,   893,   898,   904,   912,   919,   926,   934,
     944,   953,   960,   968,   974,   977,   982,   988,   992,   993,
     997,  1002,  1009,  1015,  1021,  1028,  1037,  1045,  1048,  1049,
    1051,  1054,  1057,  1061,  1066,  1071,  1075,  1077,  1079,  1082,
    1087,  1091,  1097,  1099,  1103,  1106,  1107,  1110,  1114,  1117,
    1118,  1119,  1124,  1125,  1131,  1134,  1137,  1140,  1141,  1152,
    1153,  1165,  1169,  1173,  1177,  1182,  1187,  1191,  1197,  1200,
    1203,  1204,  1211,  1217,  1222,  1226,  1228,  1230,  1234,  1239,
    1241,  1244,  1246,  1248,  1254,  1261,  1263,  1265,  1270,  1272,
    1274,  1278,  1281,  1284,  1285,  1288,  1289,  1291,  1295,  1297,
    1299,  1301,  1303,  1307,  1312,  1317,  1322,  1324,  1326,  1329,
    1332,  1335,  1339,  1343,  1345,  1347,  1349,  1351,  1355,  1357,
    1361,  1363,  1365,  1367,  1368,  1370,  1373,  1375,  1377,  1379,
    1381,  1383,  1385,  1387,  1389,  1390,  1392,  1394,  1396,  1400,
    1406,  1408,  1412,  1418,  1423,  1427,  1431,  1435,  1440,  1444,
    1448,  1452,  1455,  1458,  1460,  1462,  1466,  1470,  1472,  1474,
    1475,  1477,  1480,  1485,  1489,  1493,  1500,  1503,  1507,  1510,
    1514,  1521,  1523,  1525,  1527,  1529,  1531,  1538,  1542,  1547,
    1554,  1558,  1562,  1566,  1570,  1574,  1578,  1582,  1586,  1590,
    1594,  1598,  1602,  1605,  1608,  1611,  1614,  1618,  1622,  1626,
    1630,  1634,  1638,  1642,  1646,  1650,  1654,  1658,  1662,  1666,
    1670,  1674,  1678,  1682,  1686,  1689,  1692,  1695,  1698,  1702,
    1706,  1710,  1714,  1718,  1722,  1726,  1730,  1734,  1738,  1742,
    1748,  1753,  1757,  1759,  1762,  1765,  1768,  1771,  1774,  1777,
    1780,  1783,  1786,  1788,  1790,  1792,  1794,  1796,  1798,  1802,
    1805,  1807,  1813,  1814,  1815,  1828,  1829,  1843,  1844,  1849,
    1850,  1858,  1859,  1865,  1866,  1870,  1871,  1878,  1881,  1884,
    1889,  1891,  1893,  1899,  1903,  1909,  1913,  1916,  1917,  1920,
    1921,  1926,  1931,  1935,  1938,  1939,  1945,  1949,  1956,  1961,
    1964,  1965,  1971,  1975,  1978,  1979,  1985,  1989,  1994,  1999,
    2004,  2009,  2014,  2019,  2024,  2029,  2034,  2037,  2038,  2041,
    2042,  2045,  2046,  2051,  2056,  2061,  2066,  2068,  2070,  2072,
    2074,  2076,  2078,  2080,  2084,  2086,  2090,  2095,  2097,  2100,
    2105,  2108,  2115,  2116,  2118,  2123,  2124,  2127,  2128,  2130,
    2132,  2136,  2138,  2142,  2144,  2146,  2150,  2154,  2156,  2158,
    2160,  2162,  2164,  2166,  2168,  2170,  2172,  2174,  2176,  2178,
    2180,  2182,  2184,  2186,  2188,  2190,  2192,  2194,  2196,  2198,
    2200,  2202,  2204,  2206,  2208,  2210,  2212,  2214,  2216,  2218,
    2220,  2222,  2224,  2226,  2228,  2230,  2232,  2234,  2236,  2238,
    2240,  2242,  2244,  2246,  2248,  2250,  2252,  2254,  2256,  2258,
    2260,  2262,  2264,  2266,  2268,  2270,  2272,  2274,  2276,  2278,
    2280,  2282,  2284,  2286,  2288,  2290,  2292,  2294,  2296,  2298,
    2300,  2302,  2304,  2306,  2308,  2310,  2312,  2314,  2316,  2321,
    2323,  2325,  2327,  2329,  2331,  2333,  2337,  2339,  2343,  2345,
    2347,  2351,  2353,  2355,  2357,  2360,  2362,  2363,  2364,  2366,
    2368,  2372,  2373,  2375,  2377,  2379,  2381,  2383,  2385,  2387,
    2389,  2391,  2393,  2395,  2397,  2399,  2403,  2406,  2408,  2410,
    2415,  2419,  2424,  2426,  2428,  2430,  2432,  2434,  2438,  2442,
    2446,  2450,  2454,  2458,  2462,  2466,  2470,  2474,  2478,  2482,
    2486,  2490,  2494,  2498,  2502,  2506,  2509,  2512,  2515,  2518,
    2522,  2526,  2530,  2534,  2538,  2542,  2546,  2550,  2554,  2560,
    2565,  2569,  2571,  2575,  2579,  2583,  2587,  2589,  2591,  2593,
    2595,  2599,  2603,  2607,  2610,  2611,  2613,  2614,  2616,  2617,
    2623,  2627,  2631,  2633,  2635,  2637,  2639,  2643,  2646,  2648,
    2650,  2652,  2654,  2656,  2660,  2662,  2664,  2666,  2669,  2672,
    2677,  2681,  2686,  2688,  2690,  2692,  2696,  2698,  2701,  2702,
    2708,  2712,  2716,  2718,  2722,  2724,  2727,  2728,  2734,  2738,
    2741,  2742,  2746,  2747,  2752,  2755,  2756,  2760,  2764,  2766,
    2767,  2769,  2771,  2773,  2775,  2779,  2781,  2783,  2785,  2789,
    2791,  2793,  2797,  2801,  2804,  2809,  2812,  2817,  2823,  2829,
    2835,  2841,  2843,  2845,  2847,  2849,  2851,  2853,  2857,  2861,
    2866,  2871,  2875,  2877,  2879,  2881,  2883,  2887,  2889,  2894,
    2898,  2900,  2902,  2904,  2906,  2908,  2912,  2916,  2921,  2926,
    2930,  2932,  2934,  2942,  2952,  2960,  2967,  2976,  2978,  2981,
    2986,  2991,  2993,  2995,  2997,  3002,  3004,  3005,  3007,  3010,
    3012,  3014,  3016,  3020,  3024,  3028,  3029,  3031,  3033,  3037,
    3041,  3044,  3048,  3055,  3056,  3058,  3063,  3066,  3067,  3073,
    3077,  3081,  3083,  3090,  3095,  3100,  3103,  3106,  3107,  3113,
    3117,  3121,  3123,  3126,  3127,  3133,  3137,  3141,  3143,  3146,
    3149,  3151,  3154,  3156,  3161,  3165,  3169,  3176,  3180,  3182,
    3184,  3186,  3191,  3196,  3201,  3206,  3211,  3216,  3219,  3222,
    3227,  3230,  3233,  3235,  3239,  3243,  3247,  3248,  3251,  3257,
    3264,  3271,  3279,  3281,  3284,  3286,  3289,  3291,  3296,  3298,
    3303,  3307,  3308,  3310,  3314,  3317,  3321,  3323,  3325,  3326,
    3327,  3330,  3333,  3336,  3339,  3341,  3344,  3349,  3352,  3358,
    3362,  3364,  3366,  3367,  3371,  3376,  3382,  3386,  3388,  3391,
    3392,  3397,  3399,  3403,  3406,  3411,  3417,  3420,  3423,  3425,
    3427,  3429,  3431,  3435,  3438,  3440,  3449,  3456,  3458
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     204,     0,    -1,    -1,   205,   206,    -1,   206,   207,    -1,
      -1,   227,    -1,   244,    -1,   251,    -1,   248,    -1,   258,
      -1,   471,    -1,   129,   193,   194,   195,    -1,   159,   220,
     195,    -1,    -1,   159,   220,   196,   208,   206,   197,    -1,
      -1,   159,   196,   209,   206,   197,    -1,   117,   215,   195,
      -1,   117,   111,   215,   195,    -1,   117,   112,   215,   195,
      -1,   117,   213,   196,   218,   197,   195,    -1,   117,   111,
     213,   196,   215,   197,   195,    -1,   117,   112,   213,   196,
     215,   197,   195,    -1,   224,   195,    -1,    81,    -1,   103,
      -1,   165,    -1,   166,    -1,   168,    -1,   170,    -1,   169,
      -1,   139,    -1,   140,    -1,   141,    -1,   210,    -1,   142,
      -1,   171,    -1,   132,    -1,   133,    -1,   124,    -1,   123,
      -1,   122,    -1,   121,    -1,   120,    -1,   119,    -1,   112,
      -1,   101,    -1,    97,    -1,    99,    -1,    77,    -1,    95,
      -1,    12,    -1,   118,    -1,   109,    -1,    57,    -1,   173,
      -1,   131,    -1,   159,    -1,    72,    -1,    10,    -1,    11,
      -1,   114,    -1,   117,    -1,   125,    -1,    73,    -1,   137,
      -1,    71,    -1,     7,    -1,     6,    -1,   116,    -1,   138,
      -1,    13,    -1,    92,    -1,     4,    -1,     3,    -1,   113,
      -1,    76,    -1,    75,    -1,   107,    -1,   108,    -1,   110,
      -1,   104,    -1,    27,    -1,    29,    -1,   111,    -1,    74,
      -1,   105,    -1,   176,    -1,    96,    -1,    98,    -1,   100,
      -1,   106,    -1,    93,    -1,    94,    -1,   102,    -1,   115,
      -1,   126,    -1,   211,    -1,   130,    -1,   220,   162,    -1,
     162,   220,   162,    -1,   214,     9,   216,    -1,   216,    -1,
     214,   414,    -1,   220,    -1,   162,   220,    -1,   220,   102,
     210,    -1,   162,   220,   102,   210,    -1,   217,     9,   219,
      -1,   219,    -1,   217,   414,    -1,   216,    -1,   111,   216,
      -1,   112,   216,    -1,   210,    -1,   220,   162,   210,    -1,
     220,    -1,   159,   162,   220,    -1,   162,   220,    -1,   221,
     476,    -1,   221,   476,    -1,   224,     9,   472,    14,   408,
      -1,   112,   472,    14,   408,    -1,   225,   226,    -1,    -1,
     227,    -1,   244,    -1,   251,    -1,   258,    -1,   196,   225,
     197,    -1,    74,   334,   227,   280,   282,    -1,    74,   334,
      32,   225,   281,   283,    77,   195,    -1,    -1,    94,   334,
     228,   274,    -1,    -1,    93,   229,   227,    94,   334,   195,
      -1,    -1,    96,   193,   336,   195,   336,   195,   336,   194,
     230,   272,    -1,    -1,   104,   334,   231,   277,    -1,   108,
     195,    -1,   108,   345,   195,    -1,   110,   195,    -1,   110,
     345,   195,    -1,   113,   195,    -1,   113,   345,   195,    -1,
      27,   108,   195,    -1,   118,   290,   195,    -1,   124,   292,
     195,    -1,    92,   335,   195,    -1,   151,   335,   195,    -1,
     126,   193,   468,   194,   195,    -1,   195,    -1,    86,    -1,
      87,    -1,    -1,    98,   193,   345,   102,   271,   270,   194,
     232,   273,    -1,    -1,    98,   193,   345,    28,   102,   271,
     270,   194,   233,   273,    -1,   100,   193,   276,   194,   275,
      -1,    -1,   114,   236,   115,   193,   399,    83,   194,   196,
     225,   197,   238,   234,   241,    -1,    -1,   114,   236,   176,
     235,   239,    -1,   116,   345,   195,    -1,   109,   210,   195,
      -1,   345,   195,    -1,   337,   195,    -1,   338,   195,    -1,
     339,   195,    -1,   340,   195,    -1,   341,   195,    -1,   113,
     340,   195,    -1,   342,   195,    -1,   343,   195,    -1,   113,
     342,   195,    -1,   344,   195,    -1,   210,    32,    -1,    -1,
     196,   237,   225,   197,    -1,   238,   115,   193,   399,    83,
     194,   196,   225,   197,    -1,    -1,    -1,   196,   240,   225,
     197,    -1,   176,   239,    -1,    -1,    38,    -1,    -1,   111,
      -1,    -1,   243,   242,   475,   245,   193,   286,   194,   480,
     320,    -1,    -1,   324,   243,   242,   475,   246,   193,   286,
     194,   480,   320,    -1,    -1,   431,   323,   243,   242,   475,
     247,   193,   286,   194,   480,   320,    -1,    -1,   169,   210,
     249,    32,   493,   470,   196,   293,   197,    -1,    -1,   431,
     169,   210,   250,    32,   493,   470,   196,   293,   197,    -1,
      -1,   264,   261,   252,   265,   266,   196,   296,   197,    -1,
      -1,   431,   264,   261,   253,   265,   266,   196,   296,   197,
      -1,    -1,   131,   262,   254,   267,   196,   296,   197,    -1,
      -1,   431,   131,   262,   255,   267,   196,   296,   197,    -1,
      -1,   130,   257,   406,   265,   266,   196,   296,   197,    -1,
      -1,   171,   263,   259,   266,   196,   296,   197,    -1,    -1,
     431,   171,   263,   260,   266,   196,   296,   197,    -1,   475,
      -1,   163,    -1,   475,    -1,   475,    -1,   130,    -1,   123,
     130,    -1,   123,   122,   130,    -1,   122,   123,   130,    -1,
     122,   130,    -1,   132,   399,    -1,    -1,   133,   268,    -1,
      -1,   132,   268,    -1,    -1,   399,    -1,   268,     9,   399,
      -1,   399,    -1,   269,     9,   399,    -1,   136,   271,    -1,
      -1,   443,    -1,    38,   443,    -1,   137,   193,   457,   194,
      -1,   227,    -1,    32,   225,    97,   195,    -1,   227,    -1,
      32,   225,    99,   195,    -1,   227,    -1,    32,   225,    95,
     195,    -1,   227,    -1,    32,   225,   101,   195,    -1,   210,
      14,   408,    -1,   276,     9,   210,    14,   408,    -1,   196,
     278,   197,    -1,   196,   195,   278,   197,    -1,    32,   278,
     105,   195,    -1,    32,   195,   278,   105,   195,    -1,   278,
     106,   345,   279,   225,    -1,   278,   107,   279,   225,    -1,
      -1,    32,    -1,   195,    -1,   280,    75,   334,   227,    -1,
      -1,   281,    75,   334,    32,   225,    -1,    -1,    76,   227,
      -1,    -1,    76,    32,   225,    -1,    -1,   285,     9,   432,
     326,   494,   172,    83,    -1,   285,     9,   432,   326,   494,
      38,   172,    83,    -1,   285,     9,   432,   326,   494,   172,
      -1,   285,   414,    -1,   432,   326,   494,   172,    83,    -1,
     432,   326,   494,    38,   172,    83,    -1,   432,   326,   494,
     172,    -1,    -1,   432,   326,   494,    83,    -1,   432,   326,
     494,    38,    83,    -1,   432,   326,   494,    38,    83,    14,
     345,    -1,   432,   326,   494,    83,    14,   345,    -1,   285,
       9,   432,   326,   494,    83,    -1,   285,     9,   432,   326,
     494,    38,    83,    -1,   285,     9,   432,   326,   494,    38,
      83,    14,   345,    -1,   285,     9,   432,   326,   494,    83,
      14,   345,    -1,   287,     9,   432,   494,   172,    83,    -1,
     287,     9,   432,   494,    38,   172,    83,    -1,   287,     9,
     432,   494,   172,    -1,   287,   414,    -1,   432,   494,   172,
      83,    -1,   432,   494,    38,   172,    83,    -1,   432,   494,
     172,    -1,    -1,   432,   494,    83,    -1,   432,   494,    38,
      83,    -1,   432,   494,    38,    83,    14,   345,    -1,   432,
     494,    83,    14,   345,    -1,   287,     9,   432,   494,    83,
      -1,   287,     9,   432,   494,    38,    83,    -1,   287,     9,
     432,   494,    38,    83,    14,   345,    -1,   287,     9,   432,
     494,    83,    14,   345,    -1,   289,   414,    -1,    -1,   345,
      -1,    38,   443,    -1,   172,   345,    -1,   289,     9,   345,
      -1,   289,     9,   172,   345,    -1,   289,     9,    38,   443,
      -1,   290,     9,   291,    -1,   291,    -1,    83,    -1,   198,
     443,    -1,   198,   196,   345,   197,    -1,   292,     9,    83,
      -1,   292,     9,    83,    14,   408,    -1,    83,    -1,    83,
      14,   408,    -1,   293,   294,    -1,    -1,   295,   195,    -1,
     473,    14,   408,    -1,   296,   297,    -1,    -1,    -1,   322,
     298,   328,   195,    -1,    -1,   324,   493,   299,   328,   195,
      -1,   329,   195,    -1,   330,   195,    -1,   331,   195,    -1,
      -1,   323,   243,   242,   474,   193,   300,   284,   194,   480,
     321,    -1,    -1,   431,   323,   243,   242,   475,   193,   301,
     284,   194,   480,   321,    -1,   165,   306,   195,    -1,   166,
     314,   195,    -1,   168,   316,   195,    -1,     4,   132,   399,
     195,    -1,     4,   133,   399,   195,    -1,   117,   269,   195,
      -1,   117,   269,   196,   302,   197,    -1,   302,   303,    -1,
     302,   304,    -1,    -1,   223,   158,   210,   173,   269,   195,
      -1,   305,   102,   323,   210,   195,    -1,   305,   102,   324,
     195,    -1,   223,   158,   210,    -1,   210,    -1,   307,    -1,
     306,     9,   307,    -1,   308,   396,   312,   313,    -1,   163,
      -1,    31,   309,    -1,   309,    -1,   138,    -1,   138,   179,
     493,   413,   180,    -1,   138,   179,   493,     9,   493,   180,
      -1,   399,    -1,   125,    -1,   169,   196,   311,   197,    -1,
     142,    -1,   407,    -1,   310,     9,   407,    -1,   310,   413,
      -1,    14,   408,    -1,    -1,    59,   170,    -1,    -1,   315,
      -1,   314,     9,   315,    -1,   167,    -1,   317,    -1,   210,
      -1,   128,    -1,   193,   318,   194,    -1,   193,   318,   194,
      53,    -1,   193,   318,   194,    31,    -1,   193,   318,   194,
      50,    -1,   317,    -1,   319,    -1,   319,    53,    -1,   319,
      31,    -1,   319,    50,    -1,   318,     9,   318,    -1,   318,
      36,   318,    -1,   210,    -1,   163,    -1,   167,    -1,   195,
      -1,   196,   225,   197,    -1,   195,    -1,   196,   225,   197,
      -1,   324,    -1,   125,    -1,   324,    -1,    -1,   325,    -1,
     324,   325,    -1,   119,    -1,   120,    -1,   121,    -1,   124,
      -1,   123,    -1,   122,    -1,   189,    -1,   327,    -1,    -1,
     119,    -1,   120,    -1,   121,    -1,   328,     9,    83,    -1,
     328,     9,    83,    14,   408,    -1,    83,    -1,    83,    14,
     408,    -1,   329,     9,   473,    14,   408,    -1,   112,   473,
      14,   408,    -1,   330,     9,   473,    -1,   123,   112,   473,
      -1,   123,   332,   470,    -1,   332,   470,    14,   493,    -1,
     112,   184,   475,    -1,   193,   333,   194,    -1,    72,   403,
     406,    -1,    72,   256,    -1,    71,   345,    -1,   388,    -1,
     383,    -1,   193,   345,   194,    -1,   335,     9,   345,    -1,
     345,    -1,   335,    -1,    -1,    27,    -1,    27,   345,    -1,
      27,   345,   136,   345,    -1,   193,   337,   194,    -1,   443,
      14,   337,    -1,   137,   193,   457,   194,    14,   337,    -1,
      29,   345,    -1,   443,    14,   340,    -1,    28,   345,    -1,
     443,    14,   342,    -1,   137,   193,   457,   194,    14,   342,
      -1,   346,    -1,   443,    -1,   333,    -1,   447,    -1,   446,
      -1,   137,   193,   457,   194,    14,   345,    -1,   443,    14,
     345,    -1,   443,    14,    38,   443,    -1,   443,    14,    38,
      72,   403,   406,    -1,   443,    26,   345,    -1,   443,    25,
     345,    -1,   443,    24,   345,    -1,   443,    23,   345,    -1,
     443,    22,   345,    -1,   443,    21,   345,    -1,   443,    20,
     345,    -1,   443,    19,   345,    -1,   443,    18,   345,    -1,
     443,    17,   345,    -1,   443,    16,   345,    -1,   443,    15,
     345,    -1,   443,    68,    -1,    68,   443,    -1,   443,    67,
      -1,    67,   443,    -1,   345,    34,   345,    -1,   345,    35,
     345,    -1,   345,    10,   345,    -1,   345,    12,   345,    -1,
     345,    11,   345,    -1,   345,    36,   345,    -1,   345,    38,
     345,    -1,   345,    37,   345,    -1,   345,    52,   345,    -1,
     345,    50,   345,    -1,   345,    51,   345,    -1,   345,    53,
     345,    -1,   345,    54,   345,    -1,   345,    69,   345,    -1,
     345,    55,   345,    -1,   345,    30,   345,    -1,   345,    49,
     345,    -1,   345,    48,   345,    -1,    50,   345,    -1,    51,
     345,    -1,    56,   345,    -1,    58,   345,    -1,   345,    40,
     345,    -1,   345,    39,   345,    -1,   345,    42,   345,    -1,
     345,    41,   345,    -1,   345,    43,   345,    -1,   345,    47,
     345,    -1,   345,    44,   345,    -1,   345,    46,   345,    -1,
     345,    45,   345,    -1,   345,    57,   403,    -1,   193,   346,
     194,    -1,   345,    31,   345,    32,   345,    -1,   345,    31,
      32,   345,    -1,   345,    33,   345,    -1,   467,    -1,    66,
     345,    -1,    65,   345,    -1,    64,   345,    -1,    63,   345,
      -1,    62,   345,    -1,    61,   345,    -1,    60,   345,    -1,
      73,   404,    -1,    59,   345,    -1,   411,    -1,   364,    -1,
     371,    -1,   374,    -1,   377,    -1,   363,    -1,   199,   405,
     199,    -1,    13,   345,    -1,   385,    -1,   117,   193,   387,
     414,   194,    -1,    -1,    -1,   243,   242,   193,   349,   286,
     194,   480,   347,   480,   196,   225,   197,    -1,    -1,   324,
     243,   242,   193,   350,   286,   194,   480,   347,   480,   196,
     225,   197,    -1,    -1,   189,    83,   352,   357,    -1,    -1,
     189,   190,   353,   286,   191,   480,   357,    -1,    -1,   189,
     196,   354,   225,   197,    -1,    -1,    83,   355,   357,    -1,
      -1,   190,   356,   286,   191,   480,   357,    -1,     8,   345,
      -1,     8,   342,    -1,     8,   196,   225,   197,    -1,    91,
      -1,   469,    -1,   359,     9,   358,   136,   345,    -1,   358,
     136,   345,    -1,   360,     9,   358,   136,   408,    -1,   358,
     136,   408,    -1,   359,   413,    -1,    -1,   360,   413,    -1,
      -1,   183,   193,   361,   194,    -1,   138,   193,   458,   194,
      -1,    70,   458,   200,    -1,   366,   413,    -1,    -1,   366,
       9,   345,   136,   345,    -1,   345,   136,   345,    -1,   366,
       9,   345,   136,    38,   443,    -1,   345,   136,    38,   443,
      -1,   368,   413,    -1,    -1,   368,     9,   408,   136,   408,
      -1,   408,   136,   408,    -1,   370,   413,    -1,    -1,   370,
       9,   419,   136,   419,    -1,   419,   136,   419,    -1,   139,
      70,   365,   200,    -1,   139,    70,   367,   200,    -1,   139,
      70,   369,   200,    -1,   140,    70,   380,   200,    -1,   140,
      70,   381,   200,    -1,   140,    70,   382,   200,    -1,   141,
      70,   380,   200,    -1,   141,    70,   381,   200,    -1,   141,
      70,   382,   200,    -1,   335,   413,    -1,    -1,   409,   413,
      -1,    -1,   420,   413,    -1,    -1,   399,   196,   460,   197,
      -1,   399,   196,   462,   197,    -1,   385,    70,   453,   200,
      -1,   386,    70,   453,   200,    -1,   364,    -1,   371,    -1,
     374,    -1,   377,    -1,   469,    -1,   446,    -1,    91,    -1,
     193,   346,   194,    -1,    81,    -1,   387,     9,    83,    -1,
     387,     9,    38,    83,    -1,    83,    -1,    38,    83,    -1,
     177,   163,   389,   178,    -1,   391,    54,    -1,   391,   178,
     392,   177,    54,   390,    -1,    -1,   163,    -1,   391,   393,
      14,   394,    -1,    -1,   392,   395,    -1,    -1,   163,    -1,
     164,    -1,   196,   345,   197,    -1,   164,    -1,   196,   345,
     197,    -1,   388,    -1,   397,    -1,   396,    32,   397,    -1,
     396,    51,   397,    -1,   210,    -1,    73,    -1,   111,    -1,
     112,    -1,   113,    -1,    27,    -1,    29,    -1,    28,    -1,
     114,    -1,   115,    -1,   176,    -1,   116,    -1,    74,    -1,
      75,    -1,    77,    -1,    76,    -1,    94,    -1,    95,    -1,
      93,    -1,    96,    -1,    97,    -1,    98,    -1,    99,    -1,
     100,    -1,   101,    -1,    57,    -1,   102,    -1,   104,    -1,
     105,    -1,   106,    -1,   107,    -1,   108,    -1,   110,    -1,
     109,    -1,    92,    -1,    13,    -1,   130,    -1,   131,    -1,
     132,    -1,   133,    -1,    72,    -1,    71,    -1,   125,    -1,
       5,    -1,     7,    -1,     6,    -1,     4,    -1,     3,    -1,
     159,    -1,   117,    -1,   118,    -1,   127,    -1,   128,    -1,
     129,    -1,   124,    -1,   123,    -1,   122,    -1,   121,    -1,
     120,    -1,   119,    -1,   189,    -1,   126,    -1,   137,    -1,
     138,    -1,    10,    -1,    12,    -1,    11,    -1,   143,    -1,
     145,    -1,   144,    -1,   146,    -1,   147,    -1,   161,    -1,
     160,    -1,   188,    -1,   171,    -1,   174,    -1,   173,    -1,
     184,    -1,   186,    -1,   183,    -1,   222,   193,   288,   194,
      -1,   223,    -1,   163,    -1,   399,    -1,   407,    -1,   124,
      -1,   451,    -1,   193,   346,   194,    -1,   400,    -1,   401,
     158,   450,    -1,   400,    -1,   449,    -1,   402,   158,   450,
      -1,   399,    -1,   124,    -1,   455,    -1,   193,   194,    -1,
     334,    -1,    -1,    -1,    90,    -1,   464,    -1,   193,   288,
     194,    -1,    -1,    78,    -1,    79,    -1,    80,    -1,    91,
      -1,   146,    -1,   147,    -1,   161,    -1,   143,    -1,   174,
      -1,   144,    -1,   145,    -1,   160,    -1,   188,    -1,   154,
      90,   155,    -1,   154,   155,    -1,   407,    -1,   221,    -1,
     138,   193,   412,   194,    -1,    70,   412,   200,    -1,   183,
     193,   362,   194,    -1,   372,    -1,   375,    -1,   378,    -1,
     410,    -1,   384,    -1,   193,   408,   194,    -1,   408,    34,
     408,    -1,   408,    35,   408,    -1,   408,    10,   408,    -1,
     408,    12,   408,    -1,   408,    11,   408,    -1,   408,    36,
     408,    -1,   408,    38,   408,    -1,   408,    37,   408,    -1,
     408,    52,   408,    -1,   408,    50,   408,    -1,   408,    51,
     408,    -1,   408,    53,   408,    -1,   408,    54,   408,    -1,
     408,    55,   408,    -1,   408,    49,   408,    -1,   408,    48,
     408,    -1,   408,    69,   408,    -1,    56,   408,    -1,    58,
     408,    -1,    50,   408,    -1,    51,   408,    -1,   408,    40,
     408,    -1,   408,    39,   408,    -1,   408,    42,   408,    -1,
     408,    41,   408,    -1,   408,    43,   408,    -1,   408,    47,
     408,    -1,   408,    44,   408,    -1,   408,    46,   408,    -1,
     408,    45,   408,    -1,   408,    31,   408,    32,   408,    -1,
     408,    31,    32,   408,    -1,   409,     9,   408,    -1,   408,
      -1,   223,   158,   211,    -1,   163,   158,   211,    -1,   163,
     158,   130,    -1,   223,   158,   130,    -1,   221,    -1,    82,
      -1,   469,    -1,   407,    -1,   201,   464,   201,    -1,   202,
     464,   202,    -1,   154,   464,   155,    -1,   415,   413,    -1,
      -1,     9,    -1,    -1,     9,    -1,    -1,   415,     9,   408,
     136,   408,    -1,   415,     9,   408,    -1,   408,   136,   408,
      -1,   408,    -1,    78,    -1,    79,    -1,    80,    -1,   154,
      90,   155,    -1,   154,   155,    -1,    78,    -1,    79,    -1,
      80,    -1,   210,    -1,    91,    -1,    91,    52,   418,    -1,
     416,    -1,   418,    -1,   210,    -1,    50,   417,    -1,    51,
     417,    -1,   138,   193,   421,   194,    -1,    70,   421,   200,
      -1,   183,   193,   424,   194,    -1,   373,    -1,   376,    -1,
     379,    -1,   420,     9,   419,    -1,   419,    -1,   422,   413,
      -1,    -1,   422,     9,   419,   136,   419,    -1,   422,     9,
     419,    -1,   419,   136,   419,    -1,   419,    -1,   423,     9,
     419,    -1,   419,    -1,   425,   413,    -1,    -1,   425,     9,
     358,   136,   419,    -1,   358,   136,   419,    -1,   423,   413,
      -1,    -1,   193,   426,   194,    -1,    -1,   428,     9,   210,
     427,    -1,   210,   427,    -1,    -1,   430,   428,   413,    -1,
      49,   429,    48,    -1,   431,    -1,    -1,   134,    -1,   135,
      -1,   210,    -1,   163,    -1,   196,   345,   197,    -1,   434,
      -1,   450,    -1,   210,    -1,   196,   345,   197,    -1,   436,
      -1,   450,    -1,    70,   453,   200,    -1,   196,   345,   197,
      -1,   444,   438,    -1,   193,   333,   194,   438,    -1,   456,
     438,    -1,   193,   333,   194,   438,    -1,   193,   333,   194,
     433,   435,    -1,   193,   346,   194,   433,   435,    -1,   193,
     333,   194,   433,   434,    -1,   193,   346,   194,   433,   434,
      -1,   450,    -1,   398,    -1,   448,    -1,   449,    -1,   439,
      -1,   441,    -1,   443,   433,   435,    -1,   402,   158,   450,
      -1,   445,   193,   288,   194,    -1,   446,   193,   288,   194,
      -1,   193,   443,   194,    -1,   398,    -1,   448,    -1,   449,
      -1,   439,    -1,   443,   433,   434,    -1,   442,    -1,   445,
     193,   288,   194,    -1,   193,   443,   194,    -1,   450,    -1,
     439,    -1,   398,    -1,   364,    -1,   407,    -1,   193,   443,
     194,    -1,   193,   346,   194,    -1,   446,   193,   288,   194,
      -1,   445,   193,   288,   194,    -1,   193,   447,   194,    -1,
     348,    -1,   351,    -1,   443,   433,   437,   476,   193,   288,
     194,    -1,   193,   333,   194,   433,   437,   476,   193,   288,
     194,    -1,   402,   158,   212,   476,   193,   288,   194,    -1,
     402,   158,   450,   193,   288,   194,    -1,   402,   158,   196,
     345,   197,   193,   288,   194,    -1,   451,    -1,   454,   451,
      -1,   451,    70,   453,   200,    -1,   451,   196,   345,   197,
      -1,   452,    -1,    83,    -1,    84,    -1,   198,   196,   345,
     197,    -1,   345,    -1,    -1,   198,    -1,   454,   198,    -1,
     450,    -1,   440,    -1,   441,    -1,   455,   433,   435,    -1,
     401,   158,   450,    -1,   193,   443,   194,    -1,    -1,   440,
      -1,   442,    -1,   455,   433,   434,    -1,   193,   443,   194,
      -1,   457,     9,    -1,   457,     9,   443,    -1,   457,     9,
     137,   193,   457,   194,    -1,    -1,   443,    -1,   137,   193,
     457,   194,    -1,   459,   413,    -1,    -1,   459,     9,   345,
     136,   345,    -1,   459,     9,   345,    -1,   345,   136,   345,
      -1,   345,    -1,   459,     9,   345,   136,    38,   443,    -1,
     459,     9,    38,   443,    -1,   345,   136,    38,   443,    -1,
      38,   443,    -1,   461,   413,    -1,    -1,   461,     9,   345,
     136,   345,    -1,   461,     9,   345,    -1,   345,   136,   345,
      -1,   345,    -1,   463,   413,    -1,    -1,   463,     9,   408,
     136,   408,    -1,   463,     9,   408,    -1,   408,   136,   408,
      -1,   408,    -1,   464,   465,    -1,   464,    90,    -1,   465,
      -1,    90,   465,    -1,    83,    -1,    83,    70,   466,   200,
      -1,    83,   433,   210,    -1,   156,   345,   197,    -1,   156,
      82,    70,   345,   200,   197,    -1,   157,   443,   197,    -1,
     210,    -1,    85,    -1,    83,    -1,   127,   193,   335,   194,
      -1,   128,   193,   443,   194,    -1,   128,   193,   346,   194,
      -1,   128,   193,   447,   194,    -1,   128,   193,   446,   194,
      -1,   128,   193,   333,   194,    -1,     7,   345,    -1,     6,
     345,    -1,     5,   193,   345,   194,    -1,     4,   345,    -1,
       3,   345,    -1,   443,    -1,   468,     9,   443,    -1,   402,
     158,   211,    -1,   402,   158,   130,    -1,    -1,   102,   493,
      -1,   184,   475,    14,   493,   195,    -1,   431,   184,   475,
      14,   493,   195,    -1,   186,   475,   470,    14,   493,   195,
      -1,   431,   186,   475,   470,    14,   493,   195,    -1,   212,
      -1,   493,   212,    -1,   211,    -1,   493,   211,    -1,   212,
      -1,   212,   179,   482,   180,    -1,   210,    -1,   210,   179,
     482,   180,    -1,   179,   478,   180,    -1,    -1,   493,    -1,
     477,     9,   493,    -1,   477,   413,    -1,   477,     9,   172,
      -1,   478,    -1,   172,    -1,    -1,    -1,    32,   493,    -1,
     102,   493,    -1,   103,   493,    -1,   484,   413,    -1,   481,
      -1,   483,   481,    -1,   484,     9,   485,   210,    -1,   485,
     210,    -1,   484,     9,   485,   210,   483,    -1,   485,   210,
     483,    -1,    50,    -1,    51,    -1,    -1,    91,   136,   493,
      -1,    31,    91,   136,   493,    -1,   223,   158,   210,   136,
     493,    -1,   487,     9,   486,    -1,   486,    -1,   487,   413,
      -1,    -1,   183,   193,   488,   194,    -1,   223,    -1,   210,
     158,   491,    -1,   210,   476,    -1,   179,   493,   413,   180,
      -1,   179,   493,     9,   493,   180,    -1,    31,   493,    -1,
      59,   493,    -1,   223,    -1,   138,    -1,   142,    -1,   489,
      -1,   490,   158,   491,    -1,   138,   492,    -1,   163,    -1,
     193,   111,   193,   479,   194,    32,   493,   194,    -1,   193,
     493,     9,   477,   413,   194,    -1,   493,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   748,   748,   748,   757,   759,   762,   763,   764,   765,
     766,   767,   768,   771,   773,   773,   775,   775,   777,   779,
     782,   785,   789,   793,   797,   802,   803,   804,   805,   806,
     807,   808,   809,   810,   811,   815,   816,   817,   818,   819,
     820,   821,   822,   823,   824,   825,   826,   827,   828,   829,
     830,   831,   832,   833,   834,   835,   836,   837,   838,   839,
     840,   841,   842,   843,   844,   845,   846,   847,   848,   849,
     850,   851,   852,   853,   854,   855,   856,   857,   858,   859,
     860,   861,   862,   863,   864,   865,   866,   867,   868,   869,
     870,   871,   872,   873,   874,   875,   876,   880,   884,   885,
     889,   890,   895,   897,   902,   907,   908,   909,   911,   916,
     918,   923,   928,   930,   932,   937,   938,   942,   943,   945,
     949,   956,   963,   967,   973,   975,   978,   979,   980,   981,
     984,   985,   989,   994,   994,  1000,  1000,  1007,  1006,  1012,
    1012,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,  1028,  1029,  1030,  1031,  1035,  1033,  1042,  1040,
    1047,  1057,  1051,  1061,  1059,  1063,  1064,  1068,  1069,  1070,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1087,
    1087,  1092,  1098,  1102,  1102,  1110,  1111,  1115,  1116,  1120,
    1126,  1124,  1139,  1136,  1152,  1149,  1166,  1165,  1174,  1172,
    1184,  1183,  1202,  1200,  1219,  1218,  1227,  1225,  1236,  1236,
    1243,  1242,  1254,  1252,  1265,  1266,  1270,  1273,  1276,  1277,
    1278,  1281,  1282,  1285,  1287,  1290,  1291,  1294,  1295,  1298,
    1299,  1303,  1304,  1309,  1310,  1313,  1314,  1315,  1319,  1320,
    1324,  1325,  1329,  1330,  1334,  1335,  1340,  1341,  1347,  1348,
    1349,  1350,  1353,  1356,  1358,  1361,  1362,  1366,  1368,  1371,
    1374,  1377,  1378,  1381,  1382,  1386,  1392,  1398,  1405,  1407,
    1412,  1417,  1423,  1427,  1431,  1435,  1440,  1445,  1450,  1455,
    1461,  1470,  1475,  1480,  1486,  1488,  1492,  1496,  1501,  1505,
    1508,  1511,  1515,  1519,  1523,  1527,  1532,  1540,  1542,  1545,
    1546,  1547,  1548,  1550,  1552,  1557,  1558,  1561,  1562,  1563,
    1567,  1568,  1570,  1571,  1575,  1577,  1580,  1584,  1590,  1592,
    1595,  1595,  1599,  1598,  1602,  1604,  1607,  1610,  1608,  1624,
    1620,  1634,  1636,  1638,  1640,  1642,  1644,  1646,  1650,  1651,
    1652,  1655,  1661,  1665,  1671,  1674,  1679,  1681,  1686,  1691,
    1695,  1696,  1700,  1701,  1703,  1705,  1711,  1712,  1714,  1718,
    1719,  1724,  1728,  1729,  1733,  1734,  1738,  1740,  1746,  1751,
    1752,  1754,  1758,  1759,  1760,  1761,  1765,  1766,  1767,  1768,
    1769,  1770,  1772,  1777,  1780,  1781,  1785,  1786,  1790,  1791,
    1794,  1795,  1798,  1799,  1802,  1803,  1807,  1808,  1809,  1810,
    1811,  1812,  1813,  1817,  1818,  1821,  1822,  1823,  1826,  1828,
    1830,  1831,  1834,  1836,  1840,  1842,  1846,  1850,  1854,  1859,
    1860,  1862,  1863,  1864,  1865,  1868,  1872,  1873,  1877,  1878,
    1882,  1883,  1884,  1885,  1889,  1893,  1898,  1902,  1906,  1910,
    1914,  1919,  1920,  1921,  1922,  1923,  1927,  1929,  1930,  1931,
    1934,  1935,  1936,  1937,  1938,  1939,  1940,  1941,  1942,  1943,
    1944,  1945,  1946,  1947,  1948,  1949,  1950,  1951,  1952,  1953,
    1954,  1955,  1956,  1957,  1958,  1959,  1960,  1961,  1962,  1963,
    1964,  1965,  1966,  1967,  1968,  1969,  1970,  1971,  1972,  1973,
    1974,  1975,  1976,  1977,  1979,  1980,  1982,  1983,  1985,  1986,
    1987,  1988,  1989,  1990,  1991,  1992,  1993,  1994,  1995,  1996,
    1997,  1998,  1999,  2000,  2001,  2002,  2003,  2004,  2005,  2006,
    2007,  2011,  2015,  2020,  2019,  2034,  2032,  2050,  2049,  2068,
    2067,  2086,  2085,  2103,  2103,  2118,  2118,  2136,  2137,  2138,
    2143,  2145,  2149,  2153,  2159,  2163,  2169,  2171,  2175,  2177,
    2181,  2185,  2186,  2190,  2192,  2196,  2198,  2199,  2202,  2206,
    2208,  2212,  2215,  2220,  2222,  2226,  2229,  2234,  2238,  2242,
    2246,  2250,  2254,  2258,  2262,  2266,  2270,  2272,  2276,  2278,
    2282,  2284,  2288,  2295,  2302,  2304,  2309,  2310,  2311,  2312,
    2313,  2314,  2315,  2317,  2318,  2322,  2323,  2324,  2325,  2329,
    2335,  2344,  2357,  2358,  2361,  2364,  2367,  2368,  2371,  2375,
    2378,  2381,  2388,  2389,  2393,  2394,  2396,  2401,  2402,  2403,
    2404,  2405,  2406,  2407,  2408,  2409,  2410,  2411,  2412,  2413,
    2414,  2415,  2416,  2417,  2418,  2419,  2420,  2421,  2422,  2423,
    2424,  2425,  2426,  2427,  2428,  2429,  2430,  2431,  2432,  2433,
    2434,  2435,  2436,  2437,  2438,  2439,  2440,  2441,  2442,  2443,
    2444,  2445,  2446,  2447,  2448,  2449,  2450,  2451,  2452,  2453,
    2454,  2455,  2456,  2457,  2458,  2459,  2460,  2461,  2462,  2463,
    2464,  2465,  2466,  2467,  2468,  2469,  2470,  2471,  2472,  2473,
    2474,  2475,  2476,  2477,  2478,  2479,  2480,  2481,  2485,  2490,
    2491,  2495,  2496,  2497,  2498,  2500,  2504,  2505,  2516,  2517,
    2519,  2531,  2532,  2533,  2537,  2538,  2539,  2543,  2544,  2545,
    2548,  2550,  2554,  2555,  2556,  2557,  2559,  2560,  2561,  2562,
    2563,  2564,  2565,  2566,  2567,  2568,  2571,  2576,  2577,  2578,
    2580,  2581,  2583,  2584,  2585,  2586,  2587,  2588,  2589,  2591,
    2593,  2595,  2597,  2599,  2600,  2601,  2602,  2603,  2604,  2605,
    2606,  2607,  2608,  2609,  2610,  2611,  2612,  2613,  2614,  2615,
    2617,  2619,  2621,  2623,  2624,  2627,  2628,  2632,  2636,  2638,
    2642,  2643,  2647,  2650,  2653,  2656,  2662,  2663,  2664,  2665,
    2666,  2667,  2668,  2673,  2675,  2679,  2680,  2683,  2684,  2688,
    2691,  2693,  2695,  2699,  2700,  2701,  2702,  2705,  2709,  2710,
    2711,  2712,  2716,  2718,  2725,  2726,  2727,  2728,  2729,  2730,
    2732,  2733,  2735,  2736,  2737,  2741,  2743,  2747,  2749,  2752,
    2755,  2757,  2759,  2762,  2764,  2768,  2770,  2773,  2776,  2782,
    2784,  2787,  2788,  2793,  2796,  2800,  2800,  2805,  2808,  2809,
    2813,  2814,  2818,  2819,  2820,  2824,  2826,  2834,  2835,  2839,
    2841,  2849,  2850,  2854,  2855,  2860,  2862,  2867,  2878,  2892,
    2904,  2919,  2920,  2921,  2922,  2923,  2924,  2925,  2935,  2944,
    2946,  2948,  2952,  2953,  2954,  2955,  2956,  2972,  2973,  2975,
    2984,  2985,  2986,  2987,  2988,  2989,  2990,  2991,  2993,  2998,
    3002,  3003,  3007,  3010,  3017,  3021,  3030,  3037,  3039,  3045,
    3047,  3048,  3052,  3053,  3054,  3061,  3062,  3067,  3068,  3073,
    3074,  3075,  3076,  3087,  3090,  3093,  3094,  3095,  3096,  3107,
    3111,  3112,  3113,  3115,  3116,  3117,  3121,  3123,  3126,  3128,
    3129,  3130,  3131,  3134,  3136,  3137,  3141,  3143,  3146,  3148,
    3149,  3150,  3154,  3156,  3159,  3162,  3164,  3166,  3170,  3171,
    3173,  3174,  3180,  3181,  3183,  3193,  3195,  3197,  3200,  3201,
    3202,  3206,  3207,  3208,  3209,  3210,  3211,  3212,  3213,  3214,
    3215,  3216,  3220,  3221,  3225,  3227,  3235,  3237,  3241,  3245,
    3250,  3254,  3262,  3263,  3267,  3268,  3274,  3275,  3284,  3285,
    3293,  3296,  3300,  3303,  3308,  3313,  3315,  3316,  3317,  3321,
    3322,  3326,  3327,  3330,  3335,  3336,  3340,  3343,  3345,  3349,
    3355,  3356,  3357,  3361,  3365,  3375,  3383,  3385,  3389,  3391,
    3396,  3402,  3405,  3410,  3415,  3417,  3424,  3427,  3430,  3431,
    3434,  3437,  3438,  3443,  3445,  3449,  3455,  3465,  3466
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
  "T_UNRESOLVED_OP", "'('", "')'", "';'", "'{'", "'}'", "'$'", "'`'",
  "']'", "'\"'", "'\\''", "$accept", "start", "$@1", "top_statement_list",
  "top_statement", "$@2", "$@3", "ident_no_semireserved",
  "ident_for_class_const", "ident", "group_use_prefix",
  "non_empty_use_declarations", "use_declarations", "use_declaration",
  "non_empty_mixed_use_declarations", "mixed_use_declarations",
  "mixed_use_declaration", "namespace_name", "namespace_string",
  "namespace_string_typeargs", "class_namespace_string_typeargs",
  "constant_declaration", "inner_statement_list", "inner_statement",
  "statement", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10", "$@11",
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
  "opt_return_type", "hh_constraint", "hh_typevar_list",
  "hh_non_empty_constraint_list", "hh_non_empty_typevar_list",
  "hh_typevar_variance", "hh_shape_member_type",
  "hh_non_empty_shape_member_list", "hh_shape_member_list",
  "hh_shape_type", "hh_access_type_start", "hh_access_type",
  "array_typelist", "hh_type", "hh_type_opt", 0
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
     426,   427,   428,    40,    41,    59,   123,   125,    36,    96,
      93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   203,   205,   204,   206,   206,   207,   207,   207,   207,
     207,   207,   207,   207,   208,   207,   209,   207,   207,   207,
     207,   207,   207,   207,   207,   210,   210,   210,   210,   210,
     210,   210,   210,   210,   210,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   212,   212,
     213,   213,   214,   214,   215,   216,   216,   216,   216,   217,
     217,   218,   219,   219,   219,   220,   220,   221,   221,   221,
     222,   223,   224,   224,   225,   225,   226,   226,   226,   226,
     227,   227,   227,   228,   227,   229,   227,   230,   227,   231,
     227,   227,   227,   227,   227,   227,   227,   227,   227,   227,
     227,   227,   227,   227,   227,   227,   232,   227,   233,   227,
     227,   234,   227,   235,   227,   227,   227,   227,   227,   227,
     227,   227,   227,   227,   227,   227,   227,   227,   227,   237,
     236,   238,   238,   240,   239,   241,   241,   242,   242,   243,
     245,   244,   246,   244,   247,   244,   249,   248,   250,   248,
     252,   251,   253,   251,   254,   251,   255,   251,   257,   256,
     259,   258,   260,   258,   261,   261,   262,   263,   264,   264,
     264,   264,   264,   265,   265,   266,   266,   267,   267,   268,
     268,   269,   269,   270,   270,   271,   271,   271,   272,   272,
     273,   273,   274,   274,   275,   275,   276,   276,   277,   277,
     277,   277,   278,   278,   278,   279,   279,   280,   280,   281,
     281,   282,   282,   283,   283,   284,   284,   284,   284,   284,
     284,   284,   284,   285,   285,   285,   285,   285,   285,   285,
     285,   286,   286,   286,   286,   286,   286,   286,   286,   287,
     287,   287,   287,   287,   287,   287,   287,   288,   288,   289,
     289,   289,   289,   289,   289,   290,   290,   291,   291,   291,
     292,   292,   292,   292,   293,   293,   294,   295,   296,   296,
     298,   297,   299,   297,   297,   297,   297,   300,   297,   301,
     297,   297,   297,   297,   297,   297,   297,   297,   302,   302,
     302,   303,   304,   304,   305,   305,   306,   306,   307,   307,
     308,   308,   309,   309,   309,   309,   309,   309,   309,   310,
     310,   311,   312,   312,   313,   313,   314,   314,   315,   316,
     316,   316,   317,   317,   317,   317,   318,   318,   318,   318,
     318,   318,   318,   319,   319,   319,   320,   320,   321,   321,
     322,   322,   323,   323,   324,   324,   325,   325,   325,   325,
     325,   325,   325,   326,   326,   327,   327,   327,   328,   328,
     328,   328,   329,   329,   330,   330,   331,   331,   332,   333,
     333,   333,   333,   333,   333,   334,   335,   335,   336,   336,
     337,   337,   337,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   345,   345,   345,   345,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   347,   347,   349,   348,   350,   348,   352,   351,   353,
     351,   354,   351,   355,   351,   356,   351,   357,   357,   357,
     358,   358,   359,   359,   360,   360,   361,   361,   362,   362,
     363,   364,   364,   365,   365,   366,   366,   366,   366,   367,
     367,   368,   368,   369,   369,   370,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   380,   381,   381,
     382,   382,   383,   384,   385,   385,   386,   386,   386,   386,
     386,   386,   386,   386,   386,   387,   387,   387,   387,   388,
     389,   389,   390,   390,   391,   391,   392,   392,   393,   394,
     394,   395,   395,   395,   396,   396,   396,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   398,   399,
     399,   400,   400,   400,   400,   400,   401,   401,   402,   402,
     402,   403,   403,   403,   404,   404,   404,   405,   405,   405,
     406,   406,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     409,   409,   410,   410,   410,   410,   411,   411,   411,   411,
     411,   411,   411,   412,   412,   413,   413,   414,   414,   415,
     415,   415,   415,   416,   416,   416,   416,   416,   417,   417,
     417,   417,   418,   418,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   420,   420,   421,   421,   422,
     422,   422,   422,   423,   423,   424,   424,   425,   425,   426,
     426,   427,   427,   428,   428,   430,   429,   431,   432,   432,
     433,   433,   434,   434,   434,   435,   435,   436,   436,   437,
     437,   438,   438,   439,   439,   440,   440,   441,   441,   442,
     442,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   444,   444,   444,   444,   444,   444,   444,   444,
     445,   445,   445,   445,   445,   445,   445,   445,   445,   446,
     447,   447,   448,   448,   449,   449,   449,   450,   450,   451,
     451,   451,   452,   452,   452,   453,   453,   454,   454,   455,
     455,   455,   455,   455,   455,   456,   456,   456,   456,   456,
     457,   457,   457,   457,   457,   457,   458,   458,   459,   459,
     459,   459,   459,   459,   459,   459,   460,   460,   461,   461,
     461,   461,   462,   462,   463,   463,   463,   463,   464,   464,
     464,   464,   465,   465,   465,   465,   465,   465,   466,   466,
     466,   467,   467,   467,   467,   467,   467,   467,   467,   467,
     467,   467,   468,   468,   469,   469,   470,   470,   471,   471,
     471,   471,   472,   472,   473,   473,   474,   474,   475,   475,
     476,   476,   477,   477,   478,   479,   479,   479,   479,   480,
     480,   481,   481,   482,   483,   483,   484,   484,   484,   484,
     485,   485,   485,   486,   486,   486,   487,   487,   488,   488,
     489,   490,   491,   491,   492,   492,   493,   493,   493,   493,
     493,   493,   493,   493,   493,   493,   493,   494,   494
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
       2,     3,     3,     1,     2,     1,     2,     3,     4,     3,
       1,     2,     1,     2,     2,     1,     3,     1,     3,     2,
       2,     2,     5,     4,     2,     0,     1,     1,     1,     1,
       3,     5,     8,     0,     4,     0,     6,     0,    10,     0,
       4,     2,     3,     2,     3,     2,     3,     3,     3,     3,
       3,     3,     5,     1,     1,     1,     0,     9,     0,    10,
       5,     0,    13,     0,     5,     3,     3,     2,     2,     2,
       2,     2,     2,     3,     2,     2,     3,     2,     2,     0,
       4,     9,     0,     0,     4,     2,     0,     1,     0,     1,
       0,     9,     0,    10,     0,    11,     0,     9,     0,    10,
       0,     8,     0,     9,     0,     7,     0,     8,     0,     8,
       0,     7,     0,     8,     1,     1,     1,     1,     1,     2,
       3,     3,     2,     2,     0,     2,     0,     2,     0,     1,
       3,     1,     3,     2,     0,     1,     2,     4,     1,     4,
       1,     4,     1,     4,     1,     4,     3,     5,     3,     4,
       4,     5,     5,     4,     0,     1,     1,     4,     0,     5,
       0,     2,     0,     3,     0,     7,     8,     6,     2,     5,
       6,     4,     0,     4,     5,     7,     6,     6,     7,     9,
       8,     6,     7,     5,     2,     4,     5,     3,     0,     3,
       4,     6,     5,     5,     6,     8,     7,     2,     0,     1,
       2,     2,     3,     4,     4,     3,     1,     1,     2,     4,
       3,     5,     1,     3,     2,     0,     2,     3,     2,     0,
       0,     4,     0,     5,     2,     2,     2,     0,    10,     0,
      11,     3,     3,     3,     4,     4,     3,     5,     2,     2,
       0,     6,     5,     4,     3,     1,     1,     3,     4,     1,
       2,     1,     1,     5,     6,     1,     1,     4,     1,     1,
       3,     2,     2,     0,     2,     0,     1,     3,     1,     1,
       1,     1,     3,     4,     4,     4,     1,     1,     2,     2,
       2,     3,     3,     1,     1,     1,     1,     3,     1,     3,
       1,     1,     1,     0,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     0,     1,     1,     1,     3,     5,
       1,     3,     5,     4,     3,     3,     3,     4,     3,     3,
       3,     2,     2,     1,     1,     3,     3,     1,     1,     0,
       1,     2,     4,     3,     3,     6,     2,     3,     2,     3,
       6,     1,     1,     1,     1,     1,     6,     3,     4,     6,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       4,     3,     1,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     1,     1,     1,     1,     1,     1,     3,     2,
       1,     5,     0,     0,    12,     0,    13,     0,     4,     0,
       7,     0,     5,     0,     3,     0,     6,     2,     2,     4,
       1,     1,     5,     3,     5,     3,     2,     0,     2,     0,
       4,     4,     3,     2,     0,     5,     3,     6,     4,     2,
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
       3,     1,     1,     1,     2,     1,     0,     0,     1,     1,
       3,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     1,     1,     4,
       3,     4,     1,     1,     1,     1,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     5,     4,
       3,     1,     3,     3,     3,     3,     1,     1,     1,     1,
       3,     3,     3,     2,     0,     1,     0,     1,     0,     5,
       3,     3,     1,     1,     1,     1,     3,     2,     1,     1,
       1,     1,     1,     3,     1,     1,     1,     2,     2,     4,
       3,     4,     1,     1,     1,     3,     1,     2,     0,     5,
       3,     3,     1,     3,     1,     2,     0,     5,     3,     2,
       0,     3,     0,     4,     2,     0,     3,     3,     1,     0,
       1,     1,     1,     1,     3,     1,     1,     1,     3,     1,
       1,     3,     3,     2,     4,     2,     4,     5,     5,     5,
       5,     1,     1,     1,     1,     1,     1,     3,     3,     4,
       4,     3,     1,     1,     1,     1,     3,     1,     4,     3,
       1,     1,     1,     1,     1,     3,     3,     4,     4,     3,
       1,     1,     7,     9,     7,     6,     8,     1,     2,     4,
       4,     1,     1,     1,     4,     1,     0,     1,     2,     1,
       1,     1,     3,     3,     3,     0,     1,     1,     3,     3,
       2,     3,     6,     0,     1,     4,     2,     0,     5,     3,
       3,     1,     6,     4,     4,     2,     2,     0,     5,     3,
       3,     1,     2,     0,     5,     3,     3,     1,     2,     2,
       1,     2,     1,     4,     3,     3,     6,     3,     1,     1,
       1,     4,     4,     4,     4,     4,     4,     2,     2,     4,
       2,     2,     1,     3,     3,     3,     0,     2,     5,     6,
       6,     7,     1,     2,     1,     2,     1,     4,     1,     4,
       3,     0,     1,     3,     2,     3,     1,     1,     0,     0,
       2,     2,     2,     2,     1,     2,     4,     2,     5,     3,
       1,     1,     0,     3,     4,     5,     3,     1,     2,     0,
       4,     1,     3,     2,     4,     5,     2,     2,     1,     1,
       1,     1,     3,     2,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   430,     0,     0,   845,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   937,
       0,   925,   716,     0,   722,   723,   724,    25,   787,   912,
     913,   154,   155,   725,     0,   135,     0,     0,     0,     0,
      26,     0,     0,     0,     0,   189,     0,     0,     0,     0,
       0,     0,   396,   397,   398,   401,   400,   399,     0,     0,
       0,     0,   218,     0,     0,     0,    32,    33,    34,   729,
     731,   732,   726,   727,     0,     0,     0,   733,   728,     0,
     700,    27,    28,    29,    31,    30,     0,   730,     0,     0,
       0,     0,   734,   402,   535,     0,   153,   125,   917,   717,
       0,     0,     4,   115,   117,   786,     0,   699,     0,     6,
     188,     7,     9,     8,    10,     0,     0,   394,   443,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   441,   900,
     901,   517,   513,   514,   515,   516,   424,   520,     0,   423,
     872,   701,   708,     0,   789,   512,   393,   875,   876,   887,
     442,     0,     0,   445,   444,   873,   874,   871,   907,   911,
       0,   502,   788,    11,   401,   400,   399,     0,     0,    31,
       0,   115,   188,     0,   981,   442,   980,     0,   978,   977,
     519,     0,   431,   438,   436,     0,     0,   484,   485,   486,
     487,   511,   509,   508,   507,   506,   505,   504,   503,    25,
     912,   725,   703,    32,    33,    34,     0,     0,  1001,   893,
     701,     0,   702,   465,     0,   463,     0,   941,     0,   796,
     422,   712,   208,     0,  1001,   421,   711,   706,     0,   721,
     702,   920,   921,   927,   919,   713,     0,     0,   715,   510,
       0,     0,     0,     0,   427,     0,   133,   429,     0,     0,
     139,   141,     0,     0,   143,     0,    75,    74,    69,    68,
      60,    61,    52,    72,    83,    84,     0,    55,     0,    67,
      59,    65,    86,    78,    77,    50,    73,    93,    94,    51,
      89,    48,    90,    49,    91,    47,    95,    82,    87,    92,
      79,    80,    54,    81,    85,    46,    76,    62,    96,    70,
      63,    53,    45,    44,    43,    42,    41,    40,    64,    97,
      99,    57,    38,    39,    66,  1039,  1040,    58,  1044,    37,
      56,    88,     0,     0,   115,    98,   992,  1038,     0,  1041,
       0,     0,   145,     0,     0,     0,   179,     0,     0,     0,
       0,     0,     0,   798,     0,   103,   105,   307,     0,     0,
     306,     0,   222,     0,   219,   312,     0,     0,     0,     0,
       0,   998,   204,   216,   933,   937,   554,   577,   577,     0,
     962,     0,   736,     0,     0,     0,   960,     0,    16,     0,
     119,   196,   210,   217,   605,   547,     0,   986,   527,   529,
     531,   849,   430,   443,     0,     0,   441,   442,   444,     0,
       0,   718,     0,   719,     0,     0,     0,   178,     0,     0,
     121,   298,     0,    24,   187,     0,   215,   200,   214,   399,
     402,   188,   395,   168,   169,   170,   171,   172,   174,   175,
     177,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   925,
       0,   167,   916,   916,   947,     0,     0,     0,     0,     0,
       0,     0,     0,   392,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   464,   462,   850,
     851,     0,   916,     0,   863,   298,   298,   916,     0,   918,
     908,   933,     0,   188,     0,     0,   147,     0,   847,   842,
     796,     0,   443,   441,     0,   945,     0,   552,   795,   936,
     721,   443,   441,   442,   121,     0,   298,   420,     0,   865,
     714,     0,   125,   258,     0,   534,     0,   150,     0,     0,
     428,     0,     0,     0,     0,     0,   142,   166,   144,  1039,
    1040,  1036,  1037,     0,  1043,  1029,     0,     0,     0,     0,
      71,    36,    58,    35,   993,   173,   176,   146,   125,     0,
     163,   165,     0,     0,     0,     0,   106,     0,   797,   104,
      18,     0,   100,     0,   308,     0,   148,   221,   220,     0,
       0,   149,   982,     0,     0,   443,   441,   442,   445,   444,
       0,  1022,   228,     0,   934,     0,     0,     0,     0,   796,
     796,     0,     0,   151,     0,     0,   735,   961,   787,     0,
       0,   959,   792,   958,   118,     5,    13,    14,     0,   226,
       0,     0,   540,     0,     0,     0,   796,     0,     0,   709,
     704,   541,     0,     0,     0,     0,   849,   125,     0,   798,
     848,  1048,   419,   433,   498,   881,   899,   130,   124,   126,
     127,   128,   129,   393,     0,   518,   790,   791,   116,   796,
       0,  1002,     0,     0,     0,   798,   299,     0,   523,   190,
     224,     0,   468,   470,   469,   481,     0,     0,   501,   466,
     467,   471,   473,   472,   489,   488,   491,   490,   492,   494,
     496,   495,   493,   483,   482,   475,   476,   474,   477,   478,
     480,   497,   479,   915,     0,     0,   951,     0,   796,   985,
       0,   984,  1001,   878,   907,   206,   198,   212,     0,   986,
     202,   188,     0,   434,   437,   439,   447,   461,   460,   459,
     458,   457,   456,   455,   454,   453,   452,   451,   450,   853,
       0,   852,   855,   877,   859,  1001,   856,     0,     0,     0,
       0,     0,     0,     0,     0,   979,   432,   840,   844,   795,
     846,     0,   705,     0,   940,     0,   939,   224,     0,   705,
     924,   923,     0,     0,   852,   855,   922,   856,   425,   260,
     262,   125,   538,   537,   426,     0,   125,   242,   134,   429,
       0,     0,     0,     0,     0,   254,   254,   140,   796,     0,
       0,     0,  1027,   796,     0,  1008,     0,     0,     0,     0,
       0,   794,     0,    32,    33,    34,   700,     0,     0,   738,
     699,   742,   743,   744,   746,     0,   737,   123,   745,  1001,
    1042,     0,     0,     0,     0,    19,     0,    20,     0,   101,
       0,     0,     0,   112,   798,     0,   110,   105,   102,   107,
       0,   305,   313,   310,     0,     0,   971,   976,   973,   972,
     975,   974,    12,  1020,  1021,     0,   796,     0,     0,     0,
     933,   930,     0,   551,     0,   567,   795,   553,   795,   576,
     570,   573,   970,   969,   968,     0,   964,     0,   965,   967,
       0,     5,     0,     0,     0,   599,   600,   608,   607,     0,
     441,     0,   795,   546,   550,     0,     0,   987,     0,   528,
       0,     0,  1009,   849,   284,  1047,     0,     0,   864,     0,
     914,   795,  1004,  1000,   300,   301,   698,   797,   297,     0,
     849,     0,     0,   226,   525,   192,   500,     0,   584,   585,
       0,   582,   795,   946,     0,     0,   298,   228,     0,   226,
       0,     0,   224,     0,   925,   448,     0,     0,   861,   862,
     879,   880,   909,   910,     0,     0,     0,   828,   803,   804,
     805,   812,     0,    32,    33,    34,     0,     0,   816,   822,
     823,   824,   814,   815,   834,   796,     0,   842,   944,   943,
       0,   226,     0,   866,   720,     0,   264,     0,     0,   131,
       0,     0,     0,     0,     0,     0,     0,   234,   235,   246,
       0,   125,   244,   160,   254,     0,   254,     0,   795,     0,
       0,     0,     0,   795,  1028,  1030,  1007,   796,  1006,     0,
     796,   767,   768,   765,   766,   802,     0,   796,   794,   560,
     579,   579,     0,   549,     0,     0,   953,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1033,   180,     0,   183,   164,
       0,     0,   108,   113,   114,   106,   797,   111,     0,   309,
       0,   983,   152,   999,  1022,  1013,  1017,   227,   229,   319,
       0,     0,   931,     0,     0,   556,     0,   963,     0,    17,
       0,   986,   225,   319,     0,     0,   705,   543,     0,   710,
     988,     0,  1009,   532,     0,     0,  1048,     0,   289,   287,
     855,   867,  1001,   855,   868,  1003,     0,     0,   302,   122,
       0,   849,   223,     0,   849,     0,   499,   950,   949,     0,
     298,     0,     0,     0,     0,     0,     0,   226,   194,   721,
     854,   298,     0,   808,   809,   810,   811,   817,   818,   832,
       0,   796,     0,   828,   564,   581,   581,     0,   807,   836,
     795,   839,   841,   843,     0,   938,     0,   854,     0,     0,
       0,     0,   261,   539,   136,     0,   429,   234,   236,   933,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   248,
       0,  1034,     0,  1023,     0,  1026,   795,     0,     0,     0,
     740,   795,   793,     0,     0,   796,     0,     0,   781,   796,
       0,   784,   783,     0,   796,     0,   747,   785,   782,   957,
       0,   796,   750,   752,   751,     0,     0,   748,   749,   753,
     755,   754,   770,   769,   772,   771,   773,   775,   777,   776,
     774,   763,   762,   757,   758,   756,   759,   760,   761,   764,
    1032,     0,   125,     0,     0,   109,    21,   311,     0,     0,
       0,  1014,  1019,     0,   393,   935,   933,   435,   440,   446,
     558,     0,     0,    15,     0,   393,   611,     0,     0,   613,
     606,   609,     0,   604,     0,   990,     0,  1010,   536,     0,
     290,     0,     0,   285,     0,   304,   303,  1009,     0,   319,
       0,   849,     0,   298,     0,   905,   319,   986,   319,   989,
       0,     0,     0,   449,     0,     0,   820,   795,   827,   813,
       0,     0,   796,     0,     0,   826,   796,     0,   806,     0,
       0,   796,   833,   942,   319,     0,   125,     0,   257,   243,
       0,     0,     0,   233,   156,   247,     0,     0,   250,     0,
     255,   256,   125,   249,  1035,  1024,     0,  1005,     0,  1046,
     801,   800,   739,   568,   795,   559,     0,   571,   795,   578,
     574,     0,   795,   548,   741,     0,   583,   795,   952,   779,
       0,     0,     0,    22,    23,  1016,  1011,  1012,  1015,   230,
       0,     0,     0,   400,   391,     0,     0,     0,   205,   318,
     320,     0,   390,     0,     0,     0,   986,   393,     0,     0,
     555,   966,   315,   211,   602,     0,     0,   542,   530,     0,
     293,   283,     0,   286,   292,   298,   522,  1009,   393,  1009,
       0,   948,     0,   904,   393,     0,   393,   991,   319,   849,
     902,   831,   830,   819,   569,   795,   563,     0,   572,   795,
     580,   575,     0,   821,   795,   835,   393,   125,   263,   132,
     137,   158,   237,     0,   245,   251,   125,   253,  1025,     0,
       0,     0,   562,   780,   545,     0,   956,   955,   778,   125,
     184,  1018,     0,     0,     0,   994,     0,     0,     0,   231,
       0,   986,     0,   356,   352,   358,   700,    31,     0,   346,
       0,   351,   355,   368,     0,   366,   371,     0,   370,     0,
     369,     0,   188,   322,     0,   324,     0,   325,   326,     0,
       0,   932,   557,     0,   603,   601,   612,   610,   294,     0,
       0,   281,   291,     0,     0,  1009,     0,   201,   522,  1009,
     906,   207,   315,   213,   393,     0,     0,     0,   566,   825,
     838,     0,   209,   259,     0,     0,   125,   240,   157,   252,
    1045,   799,     0,     0,     0,     0,     0,     0,   418,     0,
     995,     0,   336,   340,   415,   416,   350,     0,     0,     0,
     331,   664,   663,   660,   662,   661,   681,   683,   682,   652,
     622,   624,   623,   642,   658,   657,   618,   629,   630,   632,
     631,   651,   635,   633,   634,   636,   637,   638,   639,   640,
     641,   643,   644,   645,   646,   647,   648,   650,   649,   619,
     620,   621,   625,   626,   628,   666,   667,   676,   675,   674,
     673,   672,   671,   659,   678,   668,   669,   670,   653,   654,
     655,   656,   679,   680,   684,   686,   685,   687,   688,   665,
     690,   689,   692,   694,   693,   627,   697,   695,   696,   691,
     677,   617,   363,   614,     0,   332,   384,   385,   383,   376,
       0,   377,   333,   410,     0,     0,     0,     0,   414,     0,
     188,   197,   314,     0,     0,     0,   282,   296,   903,     0,
       0,   386,   125,   191,  1009,     0,     0,   203,  1009,   829,
       0,     0,   125,   238,   138,   159,     0,   561,   544,   954,
     182,   334,   335,   413,   232,     0,   796,   796,     0,   359,
     347,     0,     0,     0,   365,   367,     0,     0,   372,   379,
     380,   378,     0,     0,   321,   996,     0,     0,     0,   417,
       0,   316,     0,   295,     0,   597,   798,   125,     0,     0,
     193,   199,     0,   565,   837,     0,     0,   161,   337,   115,
       0,   338,   339,     0,   795,     0,   795,   361,   357,   362,
     615,   616,     0,   348,   381,   382,   374,   375,   373,   411,
     408,  1022,   327,   323,   412,     0,   317,   598,   797,     0,
       0,   387,   125,   195,     0,   241,     0,   186,     0,   393,
       0,   353,   360,   364,     0,     0,   849,   329,     0,   595,
     521,   524,     0,   239,     0,     0,   162,   344,     0,   392,
     354,   409,   997,     0,   798,   404,   849,   596,   526,     0,
     185,     0,     0,   343,  1009,   849,   268,   405,   406,   407,
    1048,   403,     0,     0,     0,   342,     0,   404,     0,  1009,
       0,   341,   388,   125,   328,  1048,     0,   273,   271,     0,
     125,     0,     0,   274,     0,     0,   269,   330,     0,   389,
       0,   277,   267,     0,   270,   276,   181,   278,     0,     0,
     265,   275,     0,   266,   280,   279
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   112,   911,   635,   181,  1525,   732,
     352,   353,   354,   355,   864,   865,   866,   114,   115,   116,
     117,   118,   409,   668,   669,   549,   255,  1594,   555,  1503,
    1595,  1837,   853,   347,   578,  1797,  1099,  1292,  1856,   425,
     182,   670,   951,  1165,  1352,   122,   638,   968,   671,   690,
     972,   612,   967,   235,   530,   672,   639,   969,   427,   372,
     392,   125,   953,   914,   889,  1117,  1528,  1221,  1027,  1744,
    1598,   808,  1033,   554,   817,  1035,  1392,   800,  1016,  1019,
    1210,  1863,  1864,   658,   659,   684,   685,   359,   360,   366,
    1563,  1722,  1723,  1304,  1439,  1551,  1716,  1846,  1866,  1755,
    1801,  1802,  1803,  1538,  1539,  1540,  1541,  1757,  1758,  1764,
    1813,  1544,  1545,  1549,  1709,  1710,  1711,  1733,  1894,  1440,
    1441,   183,   127,  1880,  1881,  1714,  1443,  1444,  1445,  1446,
     128,   248,   550,   551,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,  1575,   139,   950,  1164,   140,   655,
     656,   657,   252,   401,   545,   645,   646,  1254,   647,  1255,
     141,   142,   618,   619,  1244,  1245,  1361,  1362,   143,   841,
     999,   144,   842,  1000,   145,   843,  1001,   621,  1247,  1364,
     146,   844,   147,   148,  1786,   149,   640,  1565,   641,  1134,
     919,  1323,  1320,  1702,  1703,   150,   151,   152,   238,   153,
     239,   249,   412,   537,   154,  1055,  1249,   848,   155,  1056,
     942,   589,  1057,  1002,  1187,  1003,  1189,  1366,  1190,  1191,
    1005,  1370,  1371,  1006,   778,   520,   195,   196,   673,   661,
     501,  1150,  1151,   764,   765,   938,   157,   241,   158,   159,
     185,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     724,   170,   245,   246,   615,   228,   229,   727,   728,  1260,
    1261,   385,   386,   905,   171,   603,   172,   654,   173,   338,
    1724,  1776,   373,   420,   679,   680,  1049,  1145,  1301,   885,
    1302,   886,   887,   822,   823,   824,   339,   340,   850,   564,
    1527,   936
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1223
static const yytype_int16 yypact[] =
{
   -1223,   147, -1223, -1223,  5556, 13556, 13556,   -33, 13556, 13556,
   13556, 10956, 13556, 13556, -1223, 13556, 13556, 13556, 13556, 13556,
   13556, 13556, 13556, 13556, 13556, 13556, 13556, 16904, 16904, 11156,
   13556, 17567,   -26,    18, -1223, -1223, -1223,   176, -1223,   367,
   -1223, -1223, -1223,   363, 13556, -1223,    18,   318,   337,   339,
   -1223,    18, 11356,  2270, 11556, -1223, 14599,  4885,   364, 13556,
    3425,   132, -1223, -1223, -1223,   262,   247,   316,   371,   378,
     387,   398, -1223,  2270,   400,   402,   533,   549,   562, -1223,
   -1223, -1223, -1223, -1223, 13556,   494,  1994, -1223, -1223,  2270,
   -1223, -1223, -1223, -1223,  2270, -1223,  2270, -1223,   444,   441,
    2270,  2270, -1223,   409, -1223, 11756, -1223, -1223,   449,   502,
     537,   537, -1223,   616,   491,   446,   462, -1223,    87, -1223,
     623, -1223, -1223, -1223, -1223,  3159,   627, -1223, -1223,   471,
     484,   493,   495,   504,   506,   508,   512, 11940, -1223, -1223,
   -1223, -1223,    55,   614,   646,   662, -1223,   669,   675, -1223,
     129,   557, -1223,   601,   -10, -1223,  1998,   135, -1223, -1223,
    2963,   102,   569,   128, -1223,   131,    66,   570,    92, -1223,
     156, -1223,   694, -1223, -1223, -1223,   608,   574,   607, -1223,
   13556, -1223,   623,   627, 18050,  2998, 18050, 13556, 18050, 18050,
   13957,   575, 17070, 13957, 18050,   723,  2270,   703,   703,   500,
     703,   703,   703,   703,   703,   703,   703,   703,   703, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223,    59, 13556,   594, -1223,
   -1223,   620,   582,    30,   596,    30, 16904, 17118,   590,   784,
   -1223,   608, -1223, 13556,   594, -1223,   636, -1223,   638,   604,
   -1223,   138, -1223, -1223, -1223,    30,   102, 11956, -1223, -1223,
   13556,  8956,   790,    88, 18050,  9956, -1223, 13556, 13556,  2270,
   -1223, -1223, 13540,   605, -1223, 13740, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223,  2184, -1223,  2184, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223,    81,    80,   607, -1223, -1223,
   -1223, -1223,   609,  1646,    89, -1223, -1223,   643,   789, -1223,
     647, 15560, -1223,   619,   624, 15289, -1223,    15, 15337,  4269,
    4269,  2270,   621,   803,   626, -1223,   214, -1223, 16500,    90,
   -1223,   688, -1223,   690, -1223,   812,    95, 16904, 13556, 13556,
     634,   650, -1223, -1223, 16601, 11156, 13556, 13556, 13556,    96,
     292,   273, -1223, 13756, 16904,   507, -1223,  2270, -1223,   222,
     491, -1223, -1223, -1223, -1223, 17665,   823,   736, -1223, -1223,
   -1223,    58, 13556,   648,   649, 18050,   651,  2181,   652,  5756,
   13556,   250,   642,   539,   250,   275,    61, -1223,  2270,  2184,
     655, 10156, 14599, -1223, -1223,  1613, -1223, -1223, -1223, -1223,
   -1223,   623, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, 13556, 13556, 13556, 13556, 12156, 13556, 13556, 13556, 13556,
   13556, 13556, 13556, 13556, 13556, 13556, 13556, 13556, 13556, 13556,
   13556, 13556, 13556, 13556, 13556, 13556, 13556, 13556, 13556, 17763,
   13556, -1223, 13556, 13556, 13556, 13956,  2270,  2270,  2270,  2270,
    2270,  3159,   739,   711,  4679, 13556, 13556, 13556, 13556, 13556,
   13556, 13556, 13556, 13556, 13556, 13556, 13556, -1223, -1223, -1223,
   -1223,   741, 13556, 13556, -1223, 10156, 10156, 13556, 13556,   449,
     151, 16601,   658,   623, 12356, 15385, -1223, 13556, -1223,   660,
     845,   700,   663,   665, 14101,    30, 12556, -1223, 12756, -1223,
     604,   666,   667,  2587, -1223,   296, 10156, -1223,  1380, -1223,
   -1223, 15452, -1223, -1223, 10356, -1223, 13556, -1223,   771,  9156,
     859,   674, 17929,   856,    77,    78, -1223, -1223, -1223,   692,
   -1223, -1223, -1223,  2184, -1223,  1205,   681,   866, 16399,  2270,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,   683,
   -1223, -1223,   682,   684,   691,   689,   224,  4007,  4280, -1223,
   -1223,  2270,  2270, 13556,    30,   132, -1223, -1223, -1223, 16399,
     794, -1223,    30,    84,    99,   707,   714,  2606,   207,   718,
     693,   126,   782,   724,    30,   117,   740, 17174,   733,   931,
     934,   746,   748, -1223,  4150,  2270, -1223, -1223,   884,  3038,
     401, -1223, -1223, -1223,   491, -1223, -1223, -1223,   926,   826,
     786,   211,   804, 13556,   449,   831,   960,   778,   815, -1223,
     151, -1223,  2184,  2184,   964,   790,    58, -1223,   783,   970,
   -1223,  2184,    68, -1223,   509,   143, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223,   962,  3435, -1223, -1223, -1223, -1223,   971,
     805, -1223, 16904, 13556,   792,   980, 18050,   977, -1223, -1223,
     860,  2404, 11541,  4565, 13957, 14424, 13556, 18002, 14598, 12535,
   12934, 13333, 12132,  4462, 14941, 14941, 14941, 14941,  3362,  3362,
    3362,  3362,  3362,   963,   963,   664,   664,   664,   500,   500,
     500, -1223,   703, 18050,   793,   798, 17222,   802,   994,    50,
   13556,   203,   594,     3,   151, -1223, -1223, -1223,   990,   736,
   -1223,   623, 16702, -1223, -1223, -1223, 13957, 13957, 13957, 13957,
   13957, 13957, 13957, 13957, 13957, 13957, 13957, 13957, 13957, -1223,
   13556,   336,   153, -1223, -1223,   594,   372,   806,  3843,   813,
     818,   809,  3935,   119,   829, -1223, 18050,  3103, -1223,  2270,
   -1223,    68,   413, 16904, 18050, 16904, 17278,   860,    68,    30,
     155,   861,   832, 13556, -1223,   157, -1223, -1223, -1223,  8756,
     112, -1223, -1223, 18050, 18050,    18, -1223, -1223, -1223, 13556,
     925, 16278, 16399,  2270,  9356,   833,   835, -1223,  1024,   943,
     899,   878, -1223,  1028,   846,  2645,  2184, 16399, 16399, 16399,
   16399, 16399,   851,   969,   975,   981,   889,   857, 16399,     1,
     896, -1223, -1223, -1223, -1223,   862, -1223, 18144, -1223,    -1,
   -1223,  5956,  3489,   863,  4280, -1223,  4280, -1223,  2270,  2270,
    4280,  4280,  2270, -1223,  1046,   871, -1223,   242, -1223, -1223,
    4012, -1223, 18144,  1043, 16904,   865, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223,   886,  1061,  2270,  3489,   880,
   16601, 16803,  1060, -1223, 12956, -1223, 13556, -1223, 13556, -1223,
   -1223, -1223, -1223, -1223, -1223,   879, -1223, 13556, -1223, -1223,
    5130, -1223,  2184,  3489,   882, -1223, -1223, -1223, -1223,  1066,
     893, 13556, 17665, -1223, -1223, 13956,   895, -1223,  2184, -1223,
     897,  6156,  1059,    60, -1223, -1223,    83,   741, -1223,  1380,
   -1223,  2184, -1223, -1223,    30, 18050, -1223, 10556, -1223, 16399,
      62,   901,  3489,   826, -1223, -1223, 14598, 13556, -1223, -1223,
   13556, -1223, 13556, -1223,  4227,   902, 10156,   782,  1064,   826,
    2184,  1083,   860,  2270, 17763,    30,  4291,   906, -1223, -1223,
     150,   908, -1223, -1223,  1089,  3415,  3415,  3103, -1223, -1223,
   -1223,  1052,   913,  1037,  1038,  1040,   417,   918, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223,  1105,   921,   660,    30,    30,
   13156,   826,  1380, -1223, -1223,  4514,   327,    18,  9956, -1223,
    6356,   924,  6556,   933, 16278, 16904,   923,   984,    30, 18144,
    1112, -1223, -1223, -1223, -1223,   503, -1223,   313,  2184,   950,
     998,  2184,  2270,  1205, -1223, -1223, -1223,  1127, -1223,   944,
     971,   116,   116,  1077,  1077,  4149,   947,  1141, 16399, 16399,
   16399, 16399, 15705, 17665, 15500, 15850, 16399, 16399, 16399, 16399,
   16167, 16399, 16399, 16399, 16399, 16399, 16399, 16399, 16399, 16399,
   16399, 16399, 16399, 16399, 16399, 16399, 16399, 16399, 16399, 16399,
   16399, 16399, 16399, 16399,  2270, -1223, -1223,  1074, -1223, -1223,
     965,   966, -1223, -1223, -1223,   259,  4007, -1223,   974, -1223,
   16399,    30, -1223, -1223,   144, -1223,   464,  1150, -1223, -1223,
     120,   979,    30, 10756, 16904, 18050, 17326, -1223,  2720, -1223,
    5356,   736,  1150, -1223,   385,   -23, -1223, 18050,  1034,   982,
   -1223,   978,  1059, -1223,  2184,   790,  2184,    63,  1160,  1096,
     163, -1223,   594,   164, -1223, -1223, 16904, 13556, 18050, 18144,
     986,    62, -1223,   985,    62,   991, 14598, 18050, 17382,   996,
   10156,   993,   995,  2184,   997,   999,  2184,   826, -1223,   604,
     377, 10156, 13556, -1223, -1223, -1223, -1223, -1223, -1223,  1067,
    1000,  1186,  1106,  3103,  3103,  3103,  3103,  1041, -1223, 17665,
    3103, -1223, -1223, -1223, 16904, 18050,  1008, -1223,    18,  1173,
    1129,  9956, -1223, -1223, -1223,  1012, 13556,   984,    30, 16601,
   16278,  1016, 16399,  6756,   576,  1013, 13556,    86,   421, -1223,
    1032, -1223,  2184, -1223,  1078, -1223,  2764,  1181,  1021, 16399,
   -1223, 16399, -1223,  1022,  1019,  1211, 16472,  1023, 18144,  1213,
    1025, -1223, -1223,  1088,  1217,  1035, -1223, -1223, -1223, 17430,
    1036,  1223, 10136, 10536, 11136, 16399, 18098, 12735, 13134, 14096,
   14767, 15113, 15551, 15551, 15551, 15551,  1720,  1720,  1720,  1720,
    1720,   869,   869,   116,   116,   116,  1077,  1077,  1077,  1077,
   -1223,  1045, -1223,  1042,  1047, -1223, -1223, 18144,  2270,  2184,
    2184, -1223,   464,  3489,  1020, -1223, 16601, -1223, -1223, 13957,
      30, 13356,  1044, -1223,  1039,  1417, -1223,   172, 13556, -1223,
   -1223, -1223, 13556, -1223, 13556, -1223,   790, -1223, -1223,    85,
    1230,  1162, 13556, -1223,  1053,    30, 18050,  1059,  1054, -1223,
    1055,    62, 13556, 10156,  1056, -1223, -1223,   736, -1223, -1223,
    1057,  1051,  1058, -1223,  1063,  3103, -1223,  3103, -1223, -1223,
    1071,  1069,  1244,  1118,  1075, -1223,  1246,  1079, -1223,  1124,
    1073,  1252, -1223,    30, -1223,  1245, -1223,  1085, -1223, -1223,
    1084,  1090,   123, -1223, -1223, 18144,  1087,  1094, -1223,  5060,
   -1223, -1223, -1223, -1223, -1223, -1223,  2184, -1223,  2184, -1223,
   18144, 17485, -1223, -1223, 16399, -1223, 16399, -1223, 16399, -1223,
   -1223, 16399, 17665, -1223, -1223, 16399, -1223, 16399, -1223, 12336,
   16399,  1095,  6956, -1223, -1223,   464, -1223, -1223, -1223, -1223,
     468, 14773,  3489,  1171, -1223,  3027,  1123,  1312, -1223, -1223,
   -1223,   739, 16091,    97,   105,  1097,   736,   711,   124, 16904,
   18050, -1223, -1223, -1223,  1131, 10940, 11340, 18050, -1223,    69,
    1281,  1216, 13556, -1223, 18050, 10156,  1184,  1059,  1948,  1059,
    1108, 18050,  1110, -1223,  1986,  1109,  2139, -1223, -1223,    62,
   -1223, -1223,  1174, -1223, -1223,  3103, -1223,  3103, -1223,  3103,
   -1223, -1223,  3103, -1223, 17665, -1223,  2281, -1223,  8756, -1223,
   -1223, -1223, -1223,  9556, -1223, -1223, -1223,  8756, -1223,  1113,
   16399, 17533, 18144, 18144, 18144,  1175, 18144, 17588, 12336, -1223,
   -1223,   464,  3489,  3489,  2270, -1223,  1295, 15995,    74, -1223,
   14773,   736,  2731, -1223,  1135, -1223,   106,  1119,   107, -1223,
   15122, -1223, -1223, -1223,   108, -1223, -1223,   617, -1223,  1125,
   -1223,  1235,   623, -1223, 14948, -1223, 14948, -1223, -1223,  1308,
     739, -1223,    30, 14251, -1223, -1223, -1223, -1223,  1309,  1241,
   13556, -1223, 18050,  1132,  1142,  1059,   445, -1223,  1184,  1059,
   -1223, -1223, -1223, -1223,  2308,  1140,  3103,  1203, -1223, -1223,
   -1223,  1206, -1223,  8756,  9756,  9556, -1223, -1223, -1223,  8756,
   -1223, 18144, 16399, 16399, 16399,  7156,  1146,  1148, -1223, 16399,
   -1223,  3489, -1223, -1223, -1223, -1223, -1223,  2184,  3158,  3027,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223,   622, -1223,  1123, -1223, -1223, -1223, -1223, -1223,
     118,   615, -1223,  1334,   110, 15560,  1235,  1335, -1223,  2184,
     623, -1223, -1223,  1155,  1337, 13556, -1223, 18050, -1223,   310,
    1156, -1223, -1223, -1223,  1059,   445, 14425, -1223,  1059, -1223,
    3103,  3103, -1223, -1223, -1223, -1223,  7356, 18144, 18144, 18144,
   -1223, -1223, -1223, 18144, -1223,  1862,  1346,  1348,  1161, -1223,
   -1223, 16399, 15122, 15122,  1302, -1223,   617,   617,   639, -1223,
   -1223, -1223, 16399,  1279, -1223,  1187,  1172,   111, 16399, -1223,
    2270, -1223, 16399, 18050,  1285, -1223,  1367, -1223,  7556,  1182,
   -1223, -1223,   445, -1223, -1223,  7756,  1188,  1262, -1223,  1280,
    1226, -1223, -1223,  1283,  2184,  1207,  3158, -1223, -1223, 18144,
   -1223, -1223,  1219, -1223,  1350, -1223, -1223, -1223, -1223, 18144,
    1376,   126, -1223, -1223, 18144,  1198, 18144, -1223,   329,  1200,
    7956, -1223, -1223, -1223,  1197, -1223,  1202,  1220,  2270,   711,
    1221, -1223, -1223, -1223, 16399,  1224,    73, -1223,  1314, -1223,
   -1223, -1223,  8156, -1223,  3489,   863, -1223,  1227,  2270,   807,
   -1223, 18144, -1223,  1209,  1396,   591,    73, -1223, -1223,  1323,
   -1223,  3489,  1215, -1223,  1059,    93, -1223, -1223, -1223, -1223,
    2184, -1223,  1214,  1222,   115, -1223,   476,   591,   101,  1059,
    1218, -1223, -1223, -1223, -1223,  2184,    72,  1403,  1336,   476,
   -1223,  8356,   148,  1404,  1339, 13556, -1223, -1223,  8556, -1223,
      75,  1409,  1341, 13556, -1223, 18050, -1223,  1411,  1343, 13556,
   -1223, 18050, 13556, -1223, 18050, 18050
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1223, -1223, -1223,  -561, -1223, -1223, -1223,   136,     0,   -34,
     326, -1223,  -280,  -523, -1223, -1223,   321,   121,  1443, -1223,
    2696, -1223,  -263, -1223,    53, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223,  -427, -1223, -1223,  -145,
     146,    31, -1223, -1223, -1223, -1223, -1223, -1223,    34, -1223,
   -1223, -1223, -1223, -1223, -1223,    37, -1223, -1223,   949,   959,
     958,  -105,  -653,  -868,   474,   529,  -428,   227,  -940, -1223,
    -150, -1223, -1223, -1223, -1223,  -739,    57, -1223, -1223, -1223,
   -1223,  -419, -1223,  -627, -1223,  -444, -1223, -1223,   854, -1223,
    -132, -1223, -1223, -1080, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223,  -164, -1223,   -76, -1223, -1223, -1223,
   -1223, -1223,  -246, -1223,    22, -1043, -1223, -1210,  -437, -1223,
    -154,   133,  -117,  -422, -1223,  -249, -1223, -1223, -1223,    35,
     -27,    -6,    47,  -738,   -71, -1223, -1223,    11, -1223,   -11,
   -1223, -1223,    -5,   -17,  -106, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223,  -595,  -859, -1223, -1223, -1223, -1223,
   -1223,  1807, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223,  1098,   412,   279,
   -1223, -1223, -1223, -1223, -1223,   350, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1033, -1223,  2311,    -3, -1223,  1014,
    -396, -1223, -1223,  -482,  3569,  2610, -1223, -1223, -1223,   427,
      67,  -626, -1223, -1223,   501,   294,  -387, -1223,   295, -1223,
   -1223, -1223, -1223, -1223,   482, -1223, -1223, -1223,    98,  -910,
    -158,  -426,  -425, -1223,   553,  -111, -1223, -1223,    27,    36,
     489, -1223, -1223,   407,   -24, -1223,  -363,    51,   348, -1223,
    -328, -1223, -1223, -1223,  -467,  1117, -1223, -1223, -1223, -1223,
   -1223,   625,   481, -1223, -1223, -1223,  -359,  -667, -1223,  1072,
   -1173, -1223,   -70,  -192,   -84,   670, -1223,  -923, -1222,  -325,
      79, -1223,   383,   455, -1223, -1223, -1223, -1223,   406, -1223,
     849, -1103
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1032
static const yytype_int16 yytable[] =
{
     184,   186,   482,   188,   189,   190,   192,   193,   194,   432,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   336,  1146,   227,   230,   393,   251,   237,   930,
     396,   397,   649,   934,   404,   121,   651,   512,   123,   254,
     256,   124,   534,  1329,   773,   260,   344,   262,   787,   265,
     504,   481,   345,  1315,   348,   428,   335,   119,   242,   948,
     929,   769,   770,  1138,   863,   868,   432,   243,   343,   583,
     585,  1023,   971,   721,   910,   762,   763,  1037,   403,   254,
    1428,   408,   244,  1611,  1217,  1163,   813,   538,   406,   -36,
     -71,   253,   792,   874,   -36,   -71,   422,   546,   -35,   595,
     405,  1174,   156,   -35,   600,   546,  1554,    14,   546,    14,
     815,    14,   795,   796,  1556,  -349,  1619,  1704,  1390,  1773,
    1773,  1147,    14,  1459,  1611,  -586,   891,  1766,   891,   891,
     579,   379,   891,   891,  1011,   539,  -884,   126,   502,  1896,
     113,  1321,    14,  1206,   380,   725,  1330,     3,  -702,   521,
     120,   631,  1568,   403,  1767,  1903,   408,  1094,  1917, -1001,
     187,  -710,   507,   406,   499,   500,  1148,   247,  1460,  1090,
    1091,  1092,   502,  1322,   767,   405,   883,   884,   419,   771,
     419,   356,   515,  -894,  1897,  1093,  1910,  1017,  1018,   263,
     522,   580,   334,   408,   883,   884,   966, -1001,  -591,  -882,
     523,  -883,   499,   500,  1253,  -885,   531,   389,  -926,   371,
     390,   250,   405,  -889,   382,   357,   532,   383,   384,  1326,
    -888,   507,   625,  -886,  -709,  -929,  1454,  -928,   405,   -99,
     391,  1911,   371,  -869,  -870,  1331,   371,   371,  1107,   210,
      40,  1569,   541,   -99,  1904,   541,  -594,  1918,  -893,  -288,
    -704,  -797,   254,   552,  -797,  1149,  -288,  1461,  1526,  1468,
     563,   371,  -884,   677,   503,   916,  1474,  -272,  1476,  1612,
    1613,   814,   431,  1898,   816,   -36,   -71,  -591,   875,   799,
    1383,  1391,   423,   547,   -35,   596,   691,  -797,   508,   483,
     601,   623,  1555,   876,  1496,  1224,   529,  1228,   503,  1428,
    1557,  -349,  1620,  1705,   543,  1774,  1823,   574,   548,  1351,
    1891,   892,  1768,   984,  1305,   851,   591,  1502,  1561,  1177,
    1912,   506,  -892,  1160,  -795,  -882,   858,  -883,  -891,   513,
     358,  -885,   519,   380,  -926,   394,  -895,  1103,  1104,  -889,
    1369,   335,   605,  -898,   591,   609,  -888,   508,  1784,  -886,
    1130,  -929,   606,  -928,   509,   689,   380,  1614,   380,  -869,
    -870,   858,   624,   254,   405,   631,   432,  1848,   774,   363,
     227,   617,   254,   254,   917,  -533,   592,   364,   629,   210,
      40,  1717,   -98,  1718,   418,   361,   859,   113,   336,   918,
    1004,   113,   362,  1785,   931,   553,   -98,   192,  1584,   365,
     506,   880,  1208,  1209,   418,   674,   383,   384,   393,   738,
     739,   428,  1849,   743,  1466,   604,   686,   636,   637,  1226,
    1227,   418,   335,  1120,   620,   620,   499,   500,   626,   383,
     384,   383,   384,  -592,   224,   224,   692,   693,   694,   695,
     697,   698,   699,   700,   701,   702,   703,   704,   705,   706,
     707,   708,   709,   710,   711,   712,   713,   714,   715,   716,
     717,   718,   719,   720,  1314,   722,   237,   723,   723,   726,
     356,   356,   586,   745,  -703,   731,   676,   573,  1380,   746,
     747,   748,   749,   750,   751,   752,   753,   754,   755,   756,
     757,   758,   398,   160,   108,   744,   242,   723,   768,   660,
     686,   686,   723,   772,   937,   243,   939,  1197,   634,   746,
    1229,   257,   776,  1153,  1154,  -857,   223,   225,   510,   482,
     244,   784,  1171,   786,   335,  1790,   733,  1226,  1227,  -857,
     258,   686,   259,   802,  1338,   499,   500,  1340,  1020,   803,
     965,   804,   126,  1022,  1576,   113,  1578,   499,   500,  1316,
    1328,  -860,   766,  1515,   678,   120,  -858,   469,   334,   649,
     346,   371,  1317,   651,   367,  -860,  1299,  1300,   481,   470,
    -858,   368,  1198,   977,  1100,   733,  1101,   380,  1179,  -593,
     369,  1318,  1833,   863,   381,   380,   791,   780,   870,   797,
     380,   370,   411,   374,   407,   375,   973,   631,   909,   399,
    1522,  1523,   807,   376, -1001,   400,  -896,   394,  1225,  1226,
    1227,   573,   371,   736,   371,   371,   371,   371,  1393,   377,
     380,   955,   380,   937,   939,   419,   920,   414,   741,   631,
    1012,   939,   378,   224,   395,  1591,  1761,   761,   405, -1001,
    1731,  1732, -1001,   499,   500,   410,  1769,   534,   417,   382,
     383,   384,  1730,   418,  1762,   421,  1735,  1095,   383,   384,
     573,   424,   632,   383,   384,  1770,   433,  -705,  1771,   407,
    1816,  1892,  1893,  1763,   794,   582,   584,  1013,   945,   434,
    1475,  1387,  1226,  1227,  -587,   113,   897,   899,   435,  1817,
     436,   956,  1818,   383,   384,   383,   384,  1353,   209,   437,
     649,   438,  -896,   439,   651,   849,   407,   440,   867,   867,
    1877,  1878,  1879,   923,  1470,   525,  -588,   466,   467,   468,
      50,   469,   533,  1814,  1815,   964,  1344,   869,   678,  1810,
    1811,  1458,  -589,   470,   413,   415,   416,  1354,    55,   472,
     160,  1047,  1050,   650,   160,   473,    62,    63,    64,   174,
     175,   429,  1382,   474,   660,   976,   213,   214,   215,   475,
     904,   906,   505,  -890,  -590,   224,  -703,   511,  1223,   387,
     516,   518,   470,   419,   224,  -894,   608,  1888,   524,  1559,
    1706,   224,    91,    92,  1707,    93,   179,    95,  1015,   506,
     527,   224,  1902,   528,  -701,   963,   535,   536,   544,  1021,
     557, -1031,   565,   568,   254,   569,   483,  1363,  1365,  1365,
    1547,  1789,   588,  1372,   575,  1792,   430,   587,   597,   576,
     598,   590,   209,   734,   210,    40,   599,   371,   610,   611,
      62,    63,    64,   174,   175,   429,   649,   652,   653,  1448,
     651,   675,   662,   663,    50,   664,   666,   594,  -120,   734,
      55,   688,  1585,   777,   779,   626,   602,   781,   607,   782,
     788,   789,   627,   614,  1615,   805,   633,  1032,   546,   809,
     812,   563,   734,   630,   825,   826,   852,   873,   854,   855,
     213,   214,   215,   734,   857,  1039,   734,   856,   882,  1125,
    1044,  1126,   627,   804,   633,   627,   633,   633,   160,  1472,
     430,   877,  1128,  1178,   759,   341,    91,    92,   878,    93,
     179,    95,   881,   998,   888,  1007,  1137,   890,   224,  1087,
    1088,  1089,  1090,  1091,  1092,   731,    62,    63,    64,   174,
     175,   429,   126,   895,   893,   113,  1865,   760,  1093,   108,
     896,   121,  1158,   898,   123,   120,   900,   124,   901,  1030,
     113,  1886,  1166,  1115,   907,  1167,  1865,  1168,   912,   913,
    1334,   686,  -725,   119,   915,  1887,  1899,   921,  1481,   922,
    1482,   237,   924,   925,   932,   867,  1139,   867,   928,   933,
     941,   867,   867,  1105,   126,   943,   946,   113,   766,   947,
     797,   949,   952,   958,  1102,   678,   430,   120,   959,   961,
     614,   242,  1873,   962,   970,  1205,   978,   980,   156,   982,
     243,  1211,   981,   463,   464,   465,   466,   467,   468,  -707,
     469,  1573,   954,  1116,  1430,   244,  1014,  1024,  1034,  1422,
    1036,   660,   470,  1038,  1040,  1041,  1042,  1043,   160,  1059,
    1045,   221,   221,   126,  1058,  1060,   113,  1062,   660,   649,
    1063,  1061,  1307,   651,  1065,  1106,   120,  1110,  1066,  1098,
    1112,   573,  1252,   797,   126,  1258,  1113,   113,  1108,    14,
    1114,  1212,  1201,   761,  1123,   794,  1119,   120,  1133,  1127,
    1135,    62,    63,    64,    65,    66,   429,  1136,  1142,   224,
    1140,  1144,    72,   476,  1161,  1170,  1173,  1176,  1587,  1181,
    1588,  -897,  1589,  1182,  1192,  1590,  1193,  1194,  1195,   371,
    1196,  1199,  1308,  1498,  1200,  1202,  1219,  1238,  1309,  1214,
    1220,  1186,  1186,   998,  1242,   561,  1222,   562,  1216,  1507,
    1231,   649,  1431,   478,  1232,   651,  1236,  1432,  1237,    62,
      63,    64,   174,  1433,   429,  1434,  1093,  1240,   794,   224,
    1241,   430,  1336,   126,   113,   126,   113,  1291,   113,  1303,
    1829,   121,  1293,  1294,   123,   686,   120,   124,   120,  1296,
    1324,   944,  1306,  1325,  1332,   966,   686,  1309,  1234,  1333,
    1337,  1339,   567,   119,  1341,  1435,  1436,  1345,  1437,  1343,
     224,  1346,   224,  1348,  1349,  1357,  1368,   991,   573,  1739,
    1356,   573,  1375,  1355,  1374,  1376,  1377,  1379,  1388,   430,
    1384,   254,  1394,  1398,  1396,  1399,  1402,  1438,   224,  1403,
    1404,  1389,  1408,  1407,  1411,  1410,  1412,   867,   156,  1414,
     849,   975,  1417,  1416,  1593,  1452,   819,  1423,  1876,  1421,
     221,  1451,  1424,  1599,  1462,  1463,  1465,  1478,  1467,  1469,
    1473,  1479,  1477,  1485,  1487,  1489,  1605,  1480,  1358,   660,
    1492,  1494,   660,   126,  1378,  1483,   113,  1493,   681,  1484,
     650,   341,  1008,   734,  1009,  1488,   120,  1497,  1500,  1491,
    1499,   224,  1504,  1530,  1501,   734,   209,   734,   160,  1505,
    1543,  1519,  1558,  1560,  1564,  1570,   820,   224,   224,  1571,
    1028,  1574,  1579,   160,  1580,  1582,  1450,  1600,    50,  1609,
    1586,  1603,  1405,  1455,  1617,  1618,  1409,  1456,  1713,  1457,
    1712,  1413,  1719,  1725,  1726,   432,  1728,  1464,  1418,   998,
     998,   998,   998,  1746,  1738,  1729,   998,  1471,   686,  1740,
     160,  1751,  1741,  1752,   213,   214,   215,   113,  1772,  1778,
    1781,  1782,  1787,  1793,  1794,  1804,   126,  1806,  1808,   113,
     734,  1812,  1820,  1111,   178,  1822,  1821,    89,  1827,   120,
      91,    92,   221,    93,   179,    95,  1828,  1836,  1832,   614,
    1122,   221,  -345,  1835,  1838,  1839,  1767,  1841,   221,  1843,
    1844,  1847,  1853,   209,  1850,  1854,  1855,  1867,   221,   160,
    1871,  1860,  1447,  1874,  1862,  1875,  1883,  1715,  1889,   648,
    1885,   650,   818,  1447,  1900,    50,  1890,  1905,  1913,  1906,
     160,  1430,  1914,  1919,  1920,  1922,  1923,  1295,  1870,  1486,
     740,   224,   224,  1490,  1425,   735,   737,  1442,  1495,   660,
    1546,  1172,  1132,  1884,  1381,  1745,  1506,  1882,  1442,   871,
    1736,   213,   214,   215,  1608,  1760,  1616,  1572,  1765,  1550,
     686,   209,  1907,   210,    40,  1895,    14,  1777,  1531,  1788,
     218,   218,  1734,  1250,   234,  1367,   622,    91,    92,  1795,
      93,   179,    95,    50,  1319,  1243,  1359,  1188,  1360,  1203,
    1152,   998,   616,   998,   687,  1048,  1845,  1298,  1235,   234,
    1290,   926,   927,     0,  1521,  1547,     0,   160,     0,   160,
     935,   160,     0,  1028,  1218,     0,     0,     0,     0,   213,
     214,   215,     0,     0,  1830,   221,     0,  1610,     0,  1431,
       0,   224,     0,     0,  1432,     0,    62,    63,    64,   174,
    1433,   429,  1434,   759,     0,    91,    92,   650,    93,   179,
      95,     0,     0,     0,     0,   126,  1597,     0,   113,     0,
       0,     0,     0,   224,     0,  1727,  1447,   334,   120,  1852,
       0,     0,  1447,  1548,  1447,  1780,   793,   660,   108,     0,
     483,     0,  1435,  1436,     0,  1437,     0,  1552,     0,     0,
       0,     0,     0,     0,  1447,     0,     0,     0,     0,     0,
       0,  1442,     0,     0,     0,     0,   430,  1442,     0,  1442,
       0,   224,     0,  1310,  1453,     0,     0,     0,     0,   160,
       0,   998,     0,   998,     0,   998,   224,   224,   998,  1442,
    1901,   126,     0,     0,   113,     0,     0,  1908,     0,   113,
     126,     0,     0,   113,   120,  1335,     0,  1743,  1597,     0,
       0,     0,     0,   120,     0,     0,     0,     0,     0,     0,
     371,     0,     0,   573,     0,     0,   334,     0,     0,   218,
       0,     0,     0,     0,   681,   681,  1701,   276,     0,     0,
       0,  1775,  1447,  1708,     0,  1858,     0,     0,     0,     0,
     334,     0,   334,  1373,   209,     0,   221,     0,     0,   334,
     160,     0,     0,     0,     0,   278,  1720,     0,   614,  1028,
    1825,     0,   160,   224,     0,   335,    50,  1442,     0,   234,
    1783,   234,   998,     0,     0,     0,   126,   209,     0,   113,
     113,   113,   126,     0,     0,   113,     0,     0,   126,   120,
       0,   113,   432,     0,     0,   120,     0,     0,     0,    50,
       0,   120,   213,   214,   215,     0,   221,   566,     0,     0,
     650,  1131,     0, -1032, -1032, -1032, -1032, -1032,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,  1092,   234,  1141,    91,    92,
       0,    93,   179,    95,   559,   213,   214,   215,   560,  1093,
    1155,     0,     0,     0,     0,   614,     0,   221,     0,   221,
       0,   218,     0,     0,     0,   178,   688,     0,    89,   328,
     218,    91,    92,     0,    93,   179,    95,   218,     0,  1175,
       0,     0,     0,  1805,  1807,   221,     0,   218,     0,   332,
       0,     0,     0,     0,   219,   219,     0,     0,   234,   333,
       0,     0,   650,     0,     0,     0,     0,     0,     0,     0,
       0,   573,     0,     0,     0,     0,   224,     0,     0,     0,
       0,     0,   234,     0,     0,   234,     0,     0,     0,     0,
       0,     0,   334,     0,     0,     0,   998,   998,     0,   126,
       0,     0,   113,     0,     0,     0,     0,  1230,   221,     0,
    1233,  1799,   120,     0,     0,     0,     0,     0,  1701,  1701,
    1915,     0,  1708,  1708,   221,   221,     0,     0,  1921,     0,
       0,   160,   234,     0,  1924,     0,   371,  1925,     0,     0,
       0,   126,     0,     0,   113,     0,     0,     0,   126,     0,
       0,   113,     0,     0,   120,     0,   648,     0,  1562,     0,
       0,   120,     0,   209,   660,     0,     0,     0,     0,     0,
       0,     0,  1430,     0,   218,     0,     0,     0,     0,     0,
       0,     0,     0,   126,   660,    50,   113,     0,     0,     0,
       0,     0,  1859,   660,  1857,     0,   120,     0,     0,     0,
       0,     0,     0,     0,     0,   126,     0,   160,   113,     0,
    1430,     0,   160,  1327,  1872,   935,   160,    14,   120,     0,
       0,   213,   214,   215,     0,     0,   234,     0,   234,     0,
       0,   839,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   178,  1347,     0,    89,  1350,     0,    91,    92,     0,
      93,   179,    95,   219,   126,    14,     0,   113,   221,   221,
       0,   126,   839,     0,   113,     0,     0,   120,     0,     0,
       0,     0,     0,     0,   120,     0,     0,     0,     0,  1798,
    1431,     0,     0,     0,     0,  1432,     0,    62,    63,    64,
     174,  1433,   429,  1434,     0,   209,     0,   648,     0,     0,
       0,  1395,   160,   160,   160,  1155,     0,     0,   160,     0,
       0,     0,     0,     0,   160,   234,   234,    50,  1431,     0,
       0,     0,     0,  1432,   234,    62,    63,    64,   174,  1433,
     429,  1434,     0,  1435,  1436,     0,  1437,    62,    63,    64,
      65,    66,   429,     0,     0,   218,     0,     0,    72,   476,
       0,     0,     0,   213,   214,   215,     0,   430,   221,     0,
       0,     0,     0,  1430,     0,  1577,     0,     0,  1426,  1427,
       0,  1435,  1436,     0,  1437,     0,   387,     0,     0,    91,
      92,     0,    93,   179,    95,   219,     0,   477,     0,   478,
     221,     0,     0,     0,   219,   430,     0,     0,     0,     0,
       0,   219,   479,  1581,   480,   218,     0,   430,    14,     0,
     388,   219,     0,     0,     0,   514,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,     0,     0,
       0,     0,     0,   648,     0,   276,     0,     0,   221,     0,
       0,     0,     0,     0,     0,     0,   218,     0,   218,     0,
       0,     0,     0,   221,   221,   160,     0,     0,     0,     0,
       0,     0,     0,   278,     0,  1508,     0,  1509,   497,   498,
       0,  1431,     0,     0,   218,   839,  1432,     0,    62,    63,
      64,   174,  1433,   429,  1434,   209,     0,     0,   234,   234,
     839,   839,   839,   839,   839,     0,     0,   160,     0,     0,
       0,   839,     0,     0,   160,  1430,     0,    50,     0,     0,
       0,  1553,     0,     0,     0,   234,     0,     0,     0,     0,
       0,     0,     0,     0,  1435,  1436,     0,  1437,     0,     0,
       0,     0,  1430,     0,     0,   499,   500,   218,   219,   160,
     221,     0,   559,   213,   214,   215,   560,     0,   430,     0,
      14,   234,     0,   218,   218,     0,  1583,     0,   220,   220,
       0,   160,   236,   178,     0,     0,    89,   328,     0,    91,
      92,   209,    93,   179,    95,   234,   234,    14,     0,     0,
       0,     0,     0,     0,     0,   234,     0,   332,     0,     0,
       0,   234,     0,    50,     0,   665,     0,   333,     0,     0,
       0,     0,     0,     0,   234,     0,     0,     0,     0,     0,
     160,     0,   839,  1431,     0,   234,     0,   160,  1432,     0,
      62,    63,    64,   174,  1433,   429,  1434,     0,     0,   213,
     214,   215,     0,   234,     0,     0,     0,   234,     0,     0,
    1431,     0,     0,     0,     0,  1432,   648,    62,    63,    64,
     174,  1433,   429,  1434,     0,    91,    92,     0,    93,   179,
      95,     0,     0,     0,     0,     0,  1435,  1436,     0,  1437,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   221,     0,     0,  1756,   218,   218,     0,
     430,     0,     0,  1435,  1436,     0,  1437,     0,  1592,     0,
       0,   234,     0,     0,   234,   209,   234,     0,     0,   219,
       0,     0,     0,     0,     0,     0,     0,   430,     0,     0,
       0,   839,   839,   839,   839,  1737,   234,    50,   648,   839,
     839,   839,   839,   839,   839,   839,   839,   839,   839,   839,
     839,   839,   839,   839,   839,   839,   839,   839,   839,   839,
     839,   839,   839,   839,   839,   839,   839,   220,     0,     0,
       0,     0,     0,   213,   214,   215,     0,     0,     0,   219,
       0,     0,     0,   839,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   218,  1779,    91,
      92,     0,    93,   179,    95,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   234,     0,   234,
     219,     0,   219,     0,     0,     0,     0,   954,     0,   218,
       0,   514,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,     0,     0,   234,     0,   219,   234,
     514,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   234,     0,     0,     0,     0,   218,     0,     0,
       0,     0,     0,  1840,   497,   498,     0,     0,     0,     0,
       0,     0,   218,   218,     0,   839,     0,     0,     0,   220,
       0,     0,     0,   497,   498,   234,   276,     0,   220,   234,
       0,   219,   839,     0,   839,   220,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   220,     0,   219,   219,     0,
       0,     0,     0,     0,   278,     0,   220,     0,   839,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   499,   500,     0,     0,     0,   209,     0,     0,   935,
     441,   442,   443,     0,     0,     0,     0,     0,     0,     0,
     499,   500,   234,   234,   935,     0,   234,     0,    50,   218,
     444,   445,   337,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,     0,   469,     0,     0,
     236,   790,     0,   559,   213,   214,   215,   560,     0,   470,
       0,     0,     0,     0,     0,   276,     0,     0,     0,     0,
     879,     0,     0,     0,   178,     0,     0,    89,   328,     0,
      91,    92,   209,    93,   179,    95,     0,  1046,     0,     0,
       0,     0,   220,   278,     0,     0,     0,     0,   332,     0,
       0,   219,   219,     0,    50,     0,     0,     0,   333,   234,
       0,   234,     0,     0,     0,   209,     0,   839,     0,   839,
       0,   839,     0,     0,   839,   234,  1533,     0,   839,     0,
     839,     0,     0,   839,     0,     0,     0,    50,     0,  1534,
     213,   214,   215,  1535,   234,   234,     0,     0,   234,   845,
       0,     0,     0,     0,     0,   234,     0,     0,     0,     0,
     178,     0,   218,    89,    90,     0,    91,    92,     0,    93,
    1537,    95,   559,   213,   214,   215,   560,     0,     0,     0,
     845,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1312,     0,     0,   178,     0,     0,    89,   328,     0,    91,
      92,   219,    93,   179,    95,     0,  1397,   234,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   332,     0,     0,
       0,     0,     0,   839,     0,     0,     0,   333,     0,     0,
       0,     0,     0,   219,     0,   234,   234,     0,     0,     0,
       0,     0,   337,   234,   337,   234,     0,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
       0,     0,     0,   220,     0,     0,     0,   234,     0,   234,
       0,     0,     0,     0,     0,     0,   234,     0,     0,     0,
       0,   219,   514,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,     0,   219,   219,     0,   337,
     497,   498,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   839,   839,   839,   441,   442,
     443,     0,   839,   220,   234,     0,     0,     0,  1532,     0,
     234,     0,   234,     0,     0,   497,   498,     0,   444,   445,
       0,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   220,   469,   220,   499,   500,     0,
       0,     0,     0,     0,     0,     0,     0,   470,   209,     0,
       0,     0,     0,   219,     0,   337,     0,     0,   337,     0,
       0,     0,   220,   845,     0,     0,     0,     0,     0,     0,
      50,     0,   499,   500,     0,     0,     0,     0,   845,   845,
     845,   845,   845,     0,     0,     0,     0,     0,     0,   845,
       0,     0,  1533,   985,   986,     0,     0,     0,     0,     0,
       0,     0,   234,  1097,     0,  1534,   213,   214,   215,  1535,
       0,     0,     0,   987,     0,     0,     0,     0,   847,   234,
       0,   988,   989,   990,   209,   220,   178,     0,     0,    89,
    1536,     0,    91,    92,   991,    93,  1537,    95,   234,  1118,
       0,   220,   220,     0,   839,     0,    50,     0,     0,   872,
       0,     0,     0,     0,     0,   839,     0,     0,     0,     0,
       0,   839,     0,     0,  1118,   839,     0,     0,     0,     0,
       0,     0,     0,   220,     0,   908,    34,    35,    36,     0,
     209,   992,   993,   994,   995,     0,     0,   234,     0,   211,
       0,     0,     0,     0,     0,     0,   219,   996,     0,   337,
     845,   821,    50,  1162,   840,     0,     0,     0,    91,    92,
       0,    93,   179,    95,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   236,   997,   839,     0,     0,
       0,     0,     0,     0,     0,   840,     0,   234,   213,   214,
     215,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,   216,     0,   234,     0,     0,     0,    87,    88,
       0,     0,   426,   234,    91,    92,     0,    93,   179,    95,
       0,     0,    97,     0,     0,   220,   220,     0,   234,     0,
       0,     0,     0,     0,     0,     0,   102,     0,   337,   337,
       0,     0,     0,     0,     0,     0,     0,   337,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   845,
     845,   845,   845,     0,   220,     0,     0,   845,   845,   845,
     845,   845,   845,   845,   845,   845,   845,   845,   845,   845,
     845,   845,   845,   845,   845,   845,   845,   845,   845,   845,
     845,   845,   845,   845,   845, -1032, -1032, -1032, -1032, -1032,
     461,   462,   463,   464,   465,   466,   467,   468,     0,   469,
       0,   845,  1029,     0,     0,     0,     0,     0,     0,     0,
       0,   470,     0,     0,     0,   220,     0,  1051,  1052,  1053,
    1054,     0,     0,     0,     0,   441,   442,   443,  1064,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   444,   445,   220,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,     0,   469,  1183,  1184,  1185,   209,     0,     0,     0,
       0,     0,     0,     0,   470,     0,   209,     0,   840,     0,
     220,     0,     0,     0,     0,   220,     0,     0,    50,     0,
       0,   337,   337,   840,   840,   840,   840,   840,    50,     0,
     220,   220,     0,   845,   840,     0,   349,   350,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     845,     0,   845,     0,   213,   214,   215,     0,     0,  1159,
       0,     0,     0,     0,   213,   214,   215,     0,     0,     0,
     209,     0,     0,     0,     0,     0,   845,     0,     0,     0,
      91,    92,     0,    93,   179,    95,     0,   351,     0,     0,
      91,    92,    50,    93,   179,    95,   222,   222,     0,     0,
     240,     0,     0,     0,     0,     0,     0,     0,   337,     0,
       0,     0,     0,     0,  1429,     0,     0,   220,     0,     0,
       0,     0,     0,     0,   337,     0,     0,     0,   213,   214,
     215,     0,   940,     0,     0,     0,     0,   337,     0,     0,
       0,     0,     0,     0,     0,   840,     0,     0,   178,     0,
       0,    89,    90,     0,    91,    92,     0,    93,   179,    95,
       0,     0,     0,     0,     0,     0,   337,     0,     0,  1246,
    1248,  1248,     0,     0,     0,     0,  1259,  1262,  1263,  1264,
    1266,  1267,  1268,  1269,  1270,  1271,  1272,  1273,  1274,  1275,
    1276,  1277,  1278,  1279,  1280,  1281,  1282,  1283,  1284,  1285,
    1286,  1287,  1288,  1289,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   845,     0,   845,     0,   845,
    1297,     0,   845,   220,     0,     0,   845,     0,   845,     0,
       0,   845,     0,     0,   337,     0,     0,   337,     0,   821,
       0,     0,     0,  1529,     0,     0,  1542,     0,     0,     0,
       0,     0,     0,     0,   840,   840,   840,   840,     0,     0,
     220,     0,   840,   840,   840,   840,   840,   840,   840,   840,
     840,   840,   840,   840,   840,   840,   840,   840,   840,   840,
     840,   840,   840,   840,   840,   840,   840,   840,   840,   840,
       0,     0,     0,     0,     0,   222,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   220,   840,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   845,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1385,  1606,  1607,     0,     0,     0,     0,     0,
     337,     0,   337,  1542,     0,     0,     0,     0,     0,  1400,
       0,  1401,     0,   441,   442,   443,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   337,
       0,     0,   337,   444,   445,  1419,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,     0,
     469,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   470,   845,   845,   845,     0,     0,   840,     0,
     845,     0,  1754,     0,     0,     0,     0,   222,   337,     0,
    1542,     0,   337,     0,     0,   840,   222,   840,     0,     0,
       0,     0,     0,   222,     0,   441,   442,   443,     0,     0,
       0,     0,     0,   222,     0,     0,     0,     0,     0,     0,
       0,   840,     0,     0,   240,   444,   445,     0,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,     0,   469,     0,     0,   337,   337,     0,     0,     0,
       0,     0,     0,     0,   470,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1511,     0,  1512,     0,  1513,     0,
       0,  1514,   441,   442,   443,  1516,     0,  1517,     0,     0,
    1518,     0,     0,     0,     0,     0,     0,     0,   240,     0,
     979,     0,   444,   445,     0,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,     0,   469,
       0,     0,   845,     0,     0,     0,     0,     0,     0,     0,
     222,   470,     0,   845,     0,     0,     0,     0,   209,   845,
       0,     0,   337,   845,   337,     0,     0,     0,     0,     0,
     840,     0,   840,     0,   840,     0,     0,   840,     0,     0,
      50,   840,     0,   840,     0,     0,   840,     0,   860,   861,
    1601,     0,     0,     0,     0,     0,     0,   337,     0,     0,
       0,     0,   983,     0,     0,     0,     0,   846,   337,     0,
       0,     0,     0,     0,     0,     0,   213,   214,   215,     0,
       0,     0,     0,     0,     0,   845,     0,     0,     0,  1067,
    1068,  1069,     0,     0,     0,  1869,     0,     0,   846,   862,
       0,     0,    91,    92,     0,    93,   179,    95,     0,     0,
    1070,     0,  1529,  1071,  1072,  1073,  1074,  1075,  1076,  1077,
    1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,
    1088,  1089,  1090,  1091,  1092,     0,   840,     0,     0,  1109,
       0,     0,  1747,  1748,  1749,     0,     0,     0,  1093,  1753,
       0,     0,     0,     0,     0,     0,   337,     0,     0,     0,
       0,   209,     0,   902,     0,   903,     0,   441,   442,   443,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     337,   222,   337,    50,     0,     0,     0,   444,   445,   337,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,     0,   469,  1239,     0,     0,     0,   213,
     214,   215,     0,     0,     0,     0,   470,     0,   840,   840,
     840,   441,   442,   443,     0,   840,     0,     0,     0,     0,
       0,   222,     0,   337,     0,    91,    92,     0,    93,   179,
      95,   444,   445,     0,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,     0,   469,     0,
     209,     0,   222,     0,   222,     0,     0,     0,     0,     0,
     470,   209,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1809,    50,     0,     0,     0,     0,     0,     0,     0,
     222,   846,  1819,    50,     0,     0,     0,     0,  1824,     0,
       0,     0,  1826,     0,     0,     0,   846,   846,   846,   846,
     846,     0,     0,     0,     0,     0,     0,   846,   213,   214,
     215,     0,     0,     0,     0,   337,     0,     0,     0,   213,
     214,   215,     0,     0,  1169,     0,     0,     0,     0,     0,
       0,   351,   337,     0,    91,    92,     0,    93,   179,    95,
       0,     0,   862,   222,     0,    91,    92,     0,    93,   179,
      95,  1800,     0,     0,  1861,     0,     0,   840,     0,   222,
     222,     0,     0,     0,     0,     0,     0,     0,   840,     0,
       0,     0,     0,     0,   840,     0,     0,     0,   840,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1180,     0,
       0,   240,     0,     0,     0,     0,     0,     0,     0,     0,
     337,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   846,   469,
       0,     0,     0,     0,   441,   442,   443,     0,     0,     0,
       0,   470,     0,     0,     0,     0,     0,     0,     0,     0,
     840,     0,     0,   240,   444,   445,     0,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
       0,   469,     0,     0,     0,     0,   337,   443,     0,     0,
       0,     0,     0,   470,     0,     0,     0,     0,     0,     0,
       0,   337,     0,   222,   222,   444,   445,     0,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,     0,   469,     0,     0,     0,     0,   846,   846,   846,
     846,     0,   240,     0,   470,   846,   846,   846,   846,   846,
     846,   846,   846,   846,   846,   846,   846,   846,   846,   846,
     846,   846,   846,   846,   846,   846,   846,   846,   846,   846,
     846,   846,   846,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   846,
       0,     0,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   222,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   402,    12,    13,     0,
       0,  1207,     0,     0,     0,     0,     0,   742,     0,     0,
       0,     0,     0,     0,     0,   222,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,   240,     0,
      43,     0,     0,   222,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,   222,   222,
      55,   846,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   174,   175,   176,     0,     0,    69,    70,   846,     0,
     846,     0,     0,     0,     0,     0,   177,    75,    76,    77,
      78,     0,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,    85,   846,     0,     0,     0,   178,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   179,    95,
       0,     0,     0,    97,     0,     0,    98,     0,     0,     0,
       0,     0,    99,     0,     0,     0,     0,   102,   103,   104,
       0,     0,   105,     0,     0,   222,     0,   108,   109,     0,
     110,   111,     0,     0,     0,     0,     0,     0,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,   846,     0,   846,    43,   846,     0,     0,
     846,   240,     0,     0,   846,     0,   846,     0,    50,   846,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   174,   175,   176,
       0,     0,    69,    70,     0,     0,     0,     0,   222,     0,
       0,     0,   177,    75,    76,    77,    78,     0,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,    85,
       0,     0,     0,     0,   178,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   179,    95,     0,     0,     0,    97,
       0,     0,    98,   240,     0,     0,     0,     0,    99,     0,
     441,   442,   443,   102,   103,   104,     0,     0,   180,   846,
     342,     0,     0,   108,   109,     0,   110,   111,     0,     0,
     444,   445,  1390,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,     0,   469,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   470,
       0,     0,     0,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   846,   846,   846,     0,     0,     0,     0,   846,    14,
      15,    16,     0,     0,     0,     0,    17,  1759,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,    56,    57,    58,     0,    59,    60,    61,    62,
      63,    64,    65,    66,    67,  1391,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,     0,    79,    80,    81,    82,    83,     0,     0,
       0,    84,     0,     0,    85,     0,     0,     0,     0,    86,
      87,    88,    89,    90,     0,    91,    92,     0,    93,    94,
      95,    96,     0,     0,    97,     0,     0,    98,     0,     0,
       0,     0,     0,    99,   100,     0,   101,     0,   102,   103,
     104,     0,     0,   105,     0,   106,   107,  1129,   108,   109,
     846,   110,   111,     0,     0,     0,     0,     0,     0,     0,
       0,   846,     0,     0,     0,     0,     0,   846,     0,     0,
       0,   846,     0,     0,     0,     0,     0,     0,     0,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,  1842,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,   846,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,    56,    57,
      58,     0,    59,    60,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,     0,    86,    87,    88,    89,    90,
       0,    91,    92,     0,    93,    94,    95,    96,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
     100,     0,   101,     0,   102,   103,   104,     0,     0,   105,
       0,   106,   107,  1313,   108,   109,     0,   110,   111,     5,
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
      51,     0,     0,     0,    52,    53,    54,    55,    56,    57,
      58,     0,    59,    60,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,     0,    86,    87,    88,    89,    90,
       0,    91,    92,     0,    93,    94,    95,    96,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
     100,     0,   101,     0,   102,   103,   104,     0,     0,   105,
       0,   106,   107,     0,   108,   109,     0,   110,   111,     5,
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
       0,   106,   107,   667,   108,   109,     0,   110,   111,     5,
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
       0,   106,   107,  1096,   108,   109,     0,   110,   111,     5,
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
       0,   106,   107,  1143,   108,   109,     0,   110,   111,     5,
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
       0,   106,   107,  1213,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,  1215,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,    96,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
       0,   106,   107,     0,   108,   109,     0,   110,   111,     5,
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
       0,   106,   107,     0,   108,   109,     0,   110,   111,     5,
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
       0,   106,   107,  1520,   108,   109,     0,   110,   111,     5,
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
       0,   106,   107,  1750,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,  1796,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,    96,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
       0,   106,   107,     0,   108,   109,     0,   110,   111,     5,
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
       0,   106,   107,  1831,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,  1834,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,    96,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
       0,   106,   107,     0,   108,   109,     0,   110,   111,     5,
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
       0,   106,   107,  1851,   108,   109,     0,   110,   111,     5,
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
       0,   106,   107,  1868,   108,   109,     0,   110,   111,     5,
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
       0,   106,   107,  1909,   108,   109,     0,   110,   111,     5,
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
       0,   106,   107,  1916,   108,   109,     0,   110,   111,     5,
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
       0,   106,   107,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,   542,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,   174,   175,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,     0,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
       0,   106,   107,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,   806,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,   174,   175,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,     0,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
       0,   106,   107,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,  1031,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,   174,   175,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,     0,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
       0,   106,   107,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,  1596,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,   174,   175,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,     0,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
       0,   106,   107,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,  1742,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,   174,   175,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,     0,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
       0,   106,   107,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,   174,   175,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,     0,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,  1068,  1069,   105,
       0,   106,   107,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,  1070,     0,    10,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,     0,     0,   682,     0,     0,     0,     0,     0,
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
       0,    91,    92,     0,    93,   179,    95,     0,   683,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   180,
       0,     0,     0,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
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
       0,     0,     0,     0,   102,   103,   104,     0,  1069,   180,
       0,     0,   801,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,  1070,     0,    10,
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
       0,     0,     0,     0,   102,   103,   104,     0,     0,   180,
       0,     0,     0,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   402,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
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
     441,   442,   443,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     444,   445,     0,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,     0,   469,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,   470,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,   191,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   174,   175,
     176,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   177,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
      85,     0,     0,     0,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,     0,     0,     0,
      97,     0,     0,    98,     0,     0,     0,  1566,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   180,
       0,     0,     0,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,  1070,     0,    10,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,     0,     0,   226,     0,     0,     0,     0,     0,
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
       0,    91,    92,     0,    93,   179,    95,     0,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   180,
     441,   442,   443,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     444,   445,     0,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,     0,   469,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,   470,
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
      97,     0,     0,    98,     0,     0,     0,  1567,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   180,
       0,   261,   442,   443,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,   444,   445,     0,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,     0,   469,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
     470,     0,    17,     0,    18,    19,    20,    21,    22,    23,
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
       0,     0,     0,     0,   102,   103,   104,     0,     0,   180,
       0,   264,     0,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   402,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
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
     441,   442,   443,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     444,   445,     0,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,     0,   469,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,   470,
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
      97,     0,     0,    98,     0,   471,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   180,
     540,     0,     0,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   696,   469,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   470,     0,     0,     0,     0,    15,    16,     0,     0,
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
       0,     0,     0,     0,   102,   103,   104,     0,     0,   180,
       0,     0,     0,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,     0,     0,   742,     0,     0,     0,     0,     0,
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
       0,    91,    92,     0,    93,   179,    95,     0,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   180,
       0,     0,     0,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,     0,   469,     0,   783,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   470,     0,    15,    16,     0,     0,
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
       0,     0,     0,     0,   102,   103,   104,     0,     0,   180,
       0,     0,     0,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,     0,     0,     0,   785,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1093,     0,    15,    16,     0,     0,
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
       0,     0,     0,     0,   102,   103,   104,     0,     0,   180,
       0,     0,     0,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
       0,   469,     0,     0,  1124,     0,     0,     0,     0,     0,
       0,     0,     0,   470,     0,     0,    15,    16,     0,     0,
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
       0,     0,     0,     0,   102,   103,   104,     0,     0,   180,
       0,     0,     0,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
       0,     0,     0,     0,  1204,     0,     0,     0,     0,     0,
       0,     0,     0,  1093,     0,     0,    15,    16,     0,     0,
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
       0,     0,     0,     0,   102,   103,   104,     0,     0,   180,
       0,     0,     0,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,     0,
     469,     0,     0,     0,  1449,     0,     0,     0,     0,     0,
       0,     0,   470,     0,     0,     0,    15,    16,     0,     0,
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
       0,     0,     0,     0,   102,   103,   104,     0,     0,   180,
     441,   442,   443,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     444,   445,     0,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,     0,   469,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,   470,
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
      97,     0,     0,    98,     0,   556,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   180,
     441,   442,   443,     0,   108,   109,     0,   110,   111,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     444,   445,     0,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,     0,   469,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,   470,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,   628,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   174,   175,
     176,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   177,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
      85,     0,     0,     0,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,     0,     0,     0,
      97,     0,     0,    98,     0,   558,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   180,
       0,     0,     0,     0,   108,   109,     0,   110,   111,   266,
     267,     0,   268,   269,     0,     0,   270,   271,   272,   273,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   274,     0,   275,     0,   444,   445,     0,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   277,   469,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   470,   279,   280,   281,
     282,   283,   284,   285,     0,     0,     0,   209,     0,   210,
      40,     0,     0,     0,     0,     0,     0,     0,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,    50,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,     0,     0,     0,   729,   321,   322,   323,
       0,     0,     0,   324,   570,   213,   214,   215,   571,     0,
       0,     0,     0,     0,   266,   267,     0,   268,   269,     0,
       0,   270,   271,   272,   273,   572,     0,     0,     0,     0,
       0,    91,    92,     0,    93,   179,    95,   329,   274,   330,
     275,     0,   331,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,   730,     0,   108,     0,     0,     0,   277,     0,
       0,     0,     0,     0,     0,  1093,     0,     0,     0,     0,
       0,     0,   279,   280,   281,   282,   283,   284,   285,     0,
       0,     0,   209,     0,   210,    40,     0,     0,     0,     0,
       0,     0,     0,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,    50,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,     0,     0,
       0,   320,   321,   322,   323,     0,     0,     0,   324,   570,
     213,   214,   215,   571,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   266,   267,     0,   268,   269,     0,
     572,   270,   271,   272,   273,     0,    91,    92,     0,    93,
     179,    95,   329,     0,   330,     0,     0,   331,   274,     0,
     275,     0,   276,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   730,     0,   108,
       0,     0,     0,     0,     0,     0,     0,     0,   277,     0,
     278,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   279,   280,   281,   282,   283,   284,   285,     0,
       0,     0,   209,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,    50,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,     0,     0,
       0,     0,   321,   322,   323,     0,     0,     0,   324,   325,
     213,   214,   215,   326,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     327,     0,     0,    89,   328,     0,    91,    92,     0,    93,
     179,    95,   329,     0,   330,     0,     0,   331,   266,   267,
       0,   268,   269,     0,   332,   270,   271,   272,   273,     0,
       0,     0,     0,     0,   333,     0,     0,     0,  1721,     0,
       0,     0,   274,     0,   275,   445,   276,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
       0,   469,   277,     0,   278,     0,     0,     0,     0,     0,
       0,     0,     0,   470,     0,     0,   279,   280,   281,   282,
     283,   284,   285,     0,     0,     0,   209,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,    50,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,     0,     0,     0,     0,   321,   322,   323,     0,
       0,     0,   324,   325,   213,   214,   215,   326,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   327,     0,     0,    89,   328,     0,
      91,    92,     0,    93,   179,    95,   329,     0,   330,     0,
       0,   331,   266,   267,     0,   268,   269,     0,   332,   270,
     271,   272,   273,     0,     0,     0,     0,     0,   333,     0,
       0,     0,  1791,     0,     0,     0,   274,     0,   275,     0,
     276,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,     0,   469,   277,     0,   278,     0,
       0,     0,     0,     0,     0,     0,     0,   470,     0,     0,
     279,   280,   281,   282,   283,   284,   285,     0,     0,     0,
     209,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,    50,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,     0,     0,     0,   320,
     321,   322,   323,     0,     0,     0,   324,   325,   213,   214,
     215,   326,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   327,     0,
       0,    89,   328,     0,    91,    92,     0,    93,   179,    95,
     329,     0,   330,     0,     0,   331,   266,   267,     0,   268,
     269,     0,   332,   270,   271,   272,   273,     0,     0,     0,
       0,     0,   333,     0,     0,     0,     0,     0,     0,     0,
     274,     0,   275,     0,   276,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,     0,     0,     0,     0,     0,     0,     0,
     277,     0,   278,     0,     0,     0,  1093,     0,     0,     0,
       0,     0,     0,     0,   279,   280,   281,   282,   283,   284,
     285,     0,     0,     0,   209,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,    50,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
       0,     0,     0,     0,   321,   322,   323,     0,     0,     0,
     324,   325,   213,   214,   215,   326,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   327,     0,     0,    89,   328,     0,    91,    92,
       0,    93,   179,    95,   329,     0,   330,     0,     0,   331,
       0,   266,   267,     0,   268,   269,   332,  1524,   270,   271,
     272,   273,     0,     0,     0,     0,   333,     0,     0,     0,
       0,     0,     0,     0,     0,   274,     0,   275,     0,   276,
   -1032, -1032, -1032, -1032,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,     0,   469,     0,
       0,     0,     0,     0,     0,   277,     0,   278,     0,     0,
     470,     0,     0,     0,     0,     0,     0,     0,     0,   279,
     280,   281,   282,   283,   284,   285,     0,     0,     0,   209,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,    50,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,     0,     0,     0,     0,   321,
     322,   323,     0,     0,     0,   324,   325,   213,   214,   215,
     326,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   327,     0,     0,
      89,   328,     0,    91,    92,     0,    93,   179,    95,   329,
       0,   330,     0,     0,   331,  1621,  1622,  1623,  1624,  1625,
       0,   332,  1626,  1627,  1628,  1629,     0,     0,     0,     0,
       0,   333,     0,     0,     0,     0,     0,     0,     0,  1630,
    1631,  1632,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1633,
       0,     0,  1093,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1634,  1635,  1636,  1637,  1638,  1639,  1640,
       0,     0,     0,   209,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1641,  1642,  1643,  1644,  1645,  1646,
    1647,  1648,  1649,  1650,  1651,    50,  1652,  1653,  1654,  1655,
    1656,  1657,  1658,  1659,  1660,  1661,  1662,  1663,  1664,  1665,
    1666,  1667,  1668,  1669,  1670,  1671,  1672,  1673,  1674,  1675,
    1676,  1677,  1678,  1679,  1680,  1681,     0,     0,     0,  1682,
    1683,   213,   214,   215,     0,  1684,  1685,  1686,  1687,  1688,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1689,  1690,  1691,     0,     0,     0,    91,    92,     0,
      93,   179,    95,  1692,     0,  1693,  1694,     0,  1695,   441,
     442,   443,     0,     0,     0,  1696,  1697,     0,  1698,     0,
    1699,  1700,     0,     0,     0,     0,     0,     0,     0,   444,
     445,     0,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,     0,   469,   441,   442,   443,
       0,     0,     0,     0,     0,     0,     0,     0,   470,     0,
       0,     0,     0,     0,     0,     0,     0,   444,   445,     0,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,     0,   469,   441,   442,   443,     0,     0,
       0,     0,     0,     0,     0,     0,   470,     0,     0,     0,
       0,     0,     0,     0,     0,   444,   445,     0,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,     0,   469,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   470,     0,     0,     0,     0,     0,
       0,     0,   441,   442,   443,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   444,   445,   577,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,     0,   469,
    1067,  1068,  1069,     0,     0,     0,     0,     0,     0,     0,
       0,   470,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1070,   581,     0,  1071,  1072,  1073,  1074,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,  1092,     0,     0,     0,     0,
       0,     0,     0,   266,   267,     0,   268,   269,     0,  1093,
     270,   271,   272,   273,     0,     0,     0,     0,     0,   775,
       0,     0,     0,     0,     0,     0,     0,   274,     0,   275,
   -1032, -1032, -1032, -1032,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   277,     0,     0,
    1093,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   279,   280,   281,   282,   283,   284,   285,     0,     0,
       0,   209,     0,     0,     0,     0,   798,     0,     0,     0,
       0,     0,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,    50,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,     0,     0,     0,
     320,   321,   322,   323,  1256,     0,     0,   324,   570,   213,
     214,   215,   571,     0,     0,     0,     0,     0,   266,   267,
       0,   268,   269,     0,     0,   270,   271,   272,   273,   572,
       0,     0,     0,     0,     0,    91,    92,     0,    93,   179,
      95,   329,   274,   330,   275,     0,   331,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   277,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   279,   280,   281,   282,
     283,   284,   285,     0,     0,     0,   209,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,    50,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,     0,     0,     0,  1251,   321,   322,   323,     0,
       0,     0,   324,   570,   213,   214,   215,   571,     0,     0,
       0,     0,     0,   266,   267,     0,   268,   269,     0,     0,
     270,   271,   272,   273,   572,     0,     0,     0,     0,     0,
      91,    92,     0,    93,   179,    95,   329,   274,   330,   275,
       0,   331,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   277,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   279,   280,   281,   282,   283,   284,   285,     0,     0,
       0,   209,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,    50,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,     0,     0,     0,
    1257,   321,   322,   323,     0,     0,     0,   324,   570,   213,
     214,   215,   571,     0,     0,     0,     0,     0,   266,   267,
       0,   268,   269,     0,     0,   270,   271,   272,   273,   572,
       0,     0,     0,     0,     0,    91,    92,     0,    93,   179,
      95,   329,   274,   330,   275,     0,   331,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   277,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   279,   280,   281,   282,
     283,   284,   285,     0,     0,     0,   209,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,    50,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   276,     0,     0,     0,   321,   322,   323,     0,
       0,     0,   324,   570,   213,   214,   215,   571,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     278,     0,     0,     0,   572,     0,     0,     0,     0,     0,
      91,    92,     0,    93,   179,    95,   329,     0,   330,     0,
       0,   331,   209,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,  1265,
       0,     0,  -392,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   174,   175,   429,     0,   827,   828,     0,
       0,     0,     0,   829,     0,   830,     0,     0,     0,   559,
     213,   214,   215,   560,     0,     0,     0,   831,     0,     0,
       0,     0,     0,     0,     0,    34,    35,    36,   209,     0,
     178,     0,     0,    89,   328,     0,    91,    92,   211,    93,
     179,    95,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,   332,     0,     0,     0,     0,     0,
     430,     0,     0,     0,   333,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   832,   833,   834,   835,     0,
      79,    80,    81,    82,    83,     0,  1025,     0,     0,     0,
       0,   216,     0,     0,     0,     0,   178,    87,    88,    89,
     836,     0,    91,    92,     0,    93,   179,    95,     0,     0,
       0,    97,     0,     0,     0,     0,     0,     0,    29,     0,
     837,     0,     0,     0,     0,   102,    34,    35,    36,   209,
     838,   210,    40,     0,     0,     0,     0,     0,     0,   211,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   212,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1026,    75,   213,   214,   215,
       0,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,   216,     0,     0,     0,     0,   178,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   179,    95,   827,
     828,     0,    97,     0,     0,   829,     0,   830,     0,     0,
       0,     0,     0,     0,     0,     0,   102,     0,     0,   831,
       0,   217,     0,     0,     0,     0,   108,    34,    35,    36,
     209,     0,  1067,  1068,  1069,     0,     0,     0,     0,     0,
     211,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,  1070,     0,     0,  1071,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   832,   833,   834,
     835,  1093,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,   216,     0,     0,     0,     0,   178,    87,
      88,    89,   836,     0,    91,    92,     0,    93,   179,    95,
      29,     0,     0,    97,     0,     0,     0,     0,    34,    35,
      36,   209,   837,   210,    40,     0,     0,   102,     0,     0,
       0,   211,   838,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,  1406,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   212,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,   213,
     214,   215,     0,    79,    80,    81,    82,    83,     0,     0,
       0,     0,     0,     0,   216,     0,     0,     0,     0,   178,
      87,    88,    89,    90,     0,    91,    92,     0,    93,   179,
      95,    29,     0,     0,    97,     0,     0,     0,     0,    34,
      35,    36,   209,     0,   210,    40,     0,     0,   102,     0,
       0,     0,   211,   217,     0,     0,   593,     0,   108,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   613,    75,
     213,   214,   215,     0,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,   216,     0,     0,     0,     0,
     178,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     179,    95,    29,     0,   974,    97,     0,     0,     0,     0,
      34,    35,    36,   209,     0,   210,    40,     0,     0,   102,
       0,     0,     0,   211,   217,     0,     0,     0,     0,   108,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,   213,   214,   215,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,   216,     0,     0,     0,
       0,   178,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   179,    95,    29,     0,     0,    97,     0,     0,     0,
       0,    34,    35,    36,   209,     0,   210,    40,     0,     0,
     102,     0,     0,     0,   211,   217,     0,     0,     0,     0,
     108,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   212,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1121,    75,   213,   214,   215,     0,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,   216,     0,     0,
       0,     0,   178,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   179,    95,    29,     0,     0,    97,     0,     0,
       0,     0,    34,    35,    36,   209,     0,   210,    40,     0,
       0,   102,     0,     0,     0,   211,   217,     0,     0,     0,
       0,   108,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,   213,   214,   215,     0,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,   216,     0,
       0,     0,     0,   178,    87,    88,    89,    90,     0,    91,
      92,     0,    93,   179,    95,     0,     0,     0,    97,     0,
     441,   442,   443,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   102,     0,     0,     0,     0,   217,     0,     0,
     444,   445,   108,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,     0,   469,   441,   442,
     443,     0,     0,     0,     0,     0,     0,     0,     0,   470,
       0,     0,     0,     0,     0,     0,     0,     0,   444,   445,
       0,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,     0,   469,     0,     0,     0,     0,
       0,     0,     0,     0,   441,   442,   443,   470,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   444,   445,   517,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
       0,   469,   441,   442,   443,     0,     0,     0,     0,     0,
       0,     0,     0,   470,     0,     0,     0,     0,     0,     0,
       0,     0,   444,   445,   526,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,     0,   469,
       0,     0,     0,     0,     0,     0,     0,     0,   441,   442,
     443,   470,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   444,   445,
     894,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,     0,   469,   441,   442,   443,     0,
       0,     0,     0,     0,     0,     0,     0,   470,     0,     0,
       0,     0,     0,     0,     0,     0,   444,   445,   960,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,     0,   469,     0,     0,     0,     0,     0,     0,
       0,     0,   441,   442,   443,   470,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   444,   445,  1010,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,     0,   469,
    1067,  1068,  1069,     0,     0,     0,     0,     0,     0,     0,
       0,   470,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1070,  1311,     0,  1071,  1072,  1073,  1074,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,  1092,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1067,  1068,  1069,     0,  1093,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1070,     0,  1342,  1071,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,     0,     0,  1067,  1068,  1069,     0,     0,     0,     0,
       0,     0,     0,     0,  1093,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1070,     0,  1415,  1071,  1072,  1073,
    1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1067,  1068,
    1069,     0,  1093,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1070,
       0,  1510,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,     0,    34,    35,    36,   209,     0,
     210,    40,     0,     0,     0,     0,     0,  1093,   211,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1602,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   231,     0,     0,     0,     0,     0,   232,     0,     0,
       0,     0,     0,     0,     0,     0,   213,   214,   215,     0,
      79,    80,    81,    82,    83,     0,     0,     0,     0,     0,
       0,   216,     0,     0,  1604,     0,   178,    87,    88,    89,
      90,     0,    91,    92,     0,    93,   179,    95,     0,     0,
       0,    97,     0,    34,    35,    36,   209,     0,   210,    40,
       0,     0,     0,     0,     0,   102,   642,     0,     0,     0,
     233,     0,     0,     0,     0,   108,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   212,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   213,   214,   215,     0,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,   216,
       0,     0,     0,     0,   178,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   179,    95,     0,     0,     0,    97,
       0,    34,    35,    36,   209,     0,   210,    40,     0,     0,
       0,     0,     0,   102,   211,     0,     0,     0,   643,     0,
       0,     0,     0,   644,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   231,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   213,   214,   215,     0,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,   216,     0,     0,
       0,     0,   178,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   179,    95,     0,     0,     0,    97,     0,   441,
     442,   443,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   102,     0,     0,     0,     0,   233,   810,     0,   444,
     445,   108,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,     0,   469,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   470,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   441,   442,   443,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   811,   444,   445,   957,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,     0,   469,
     441,   442,   443,     0,     0,     0,     0,     0,     0,     0,
       0,   470,     0,     0,     0,     0,     0,     0,     0,     0,
     444,   445,     0,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,     0,   469,  1067,  1068,
    1069,     0,     0,     0,     0,     0,     0,     0,     0,   470,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1070,
    1420,     0,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,  1067,  1068,  1069,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1093,     0,     0,
       0,     0,     0,     0,     0,  1070,     0,     0,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1093
};

static const yytype_int16 yycheck[] =
{
       5,     6,   156,     8,     9,    10,    11,    12,    13,   126,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    56,   933,    29,    30,    96,    33,    31,   656,
     100,   101,   395,   659,   105,     4,   395,   182,     4,    44,
      46,     4,   234,  1146,   511,    51,    57,    52,   530,    54,
     161,   156,    57,  1133,    59,   125,    56,     4,    31,   685,
     655,   505,   506,   922,   587,   588,   183,    31,    57,   349,
     350,   809,   739,   469,   635,   501,   501,   816,   105,    84,
    1302,   105,    31,     9,  1024,   953,     9,   245,   105,     9,
       9,    44,   536,     9,    14,    14,     9,     9,     9,     9,
     105,   969,     4,    14,     9,     9,     9,    49,     9,    49,
      32,    49,   538,   538,     9,     9,     9,     9,    32,     9,
       9,    38,    49,    38,     9,    70,     9,     9,     9,     9,
     115,    84,     9,     9,   787,   246,    70,     4,    70,    38,
       4,   164,    49,  1011,    83,   473,    83,     0,   158,    90,
       4,    90,    83,   180,    36,    83,   180,   158,    83,   158,
     193,   158,    70,   180,   134,   135,    83,   193,    83,    53,
      54,    55,    70,   196,   502,   180,    50,    51,   179,   507,
     179,    60,   187,   193,    83,    69,    38,    75,    76,    53,
     217,   176,    56,   217,    50,    51,   193,   196,    70,    70,
     217,    70,   134,   135,  1063,    70,   233,    86,    70,    73,
      89,   193,   217,    70,   155,    83,   233,   156,   157,  1142,
      70,    70,   380,    70,   158,    70,    54,    70,   233,   179,
      94,    83,    96,    70,    70,   172,   100,   101,   864,    83,
      84,   172,   247,   193,   172,   250,    70,   172,   193,   191,
     158,   191,   257,   258,   194,   172,   194,   172,  1431,  1339,
     179,   125,   196,   202,   196,    54,  1346,   194,  1348,   195,
     196,   194,   126,   172,   196,   195,   195,    70,   194,   542,
    1220,   195,   195,   195,   195,   195,   431,   194,   196,   156,
     195,   195,   195,   194,  1374,  1034,   229,  1036,   196,  1521,
     195,   195,   195,   195,   251,   195,   195,   341,   255,  1177,
     195,   194,   194,   194,   194,   578,   102,   194,   194,   972,
     172,   193,   193,   950,   180,   196,   102,   196,   193,   183,
     198,   196,   196,    83,   196,   163,   193,   860,   861,   196,
    1199,   341,   369,   193,   102,   369,   196,   196,    38,   196,
     911,   196,   369,   196,   198,   425,    83,  1530,    83,   196,
     196,   102,    70,   368,   369,    90,   483,    38,   513,   122,
     375,   376,   377,   378,   163,     8,   162,   130,   383,    83,
      84,  1554,   179,  1556,   162,   123,   162,   251,   422,   178,
     777,   255,   130,    83,   657,   259,   193,   402,  1478,    83,
     193,   194,    75,    76,   162,   410,   156,   157,   478,   479,
     480,   481,    83,   484,  1337,   368,   421,   195,   196,   106,
     107,   162,   422,   890,   377,   378,   134,   135,   155,   156,
     157,   156,   157,    70,    27,    28,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,  1131,   470,   469,   472,   473,   474,
     349,   350,   351,   484,   158,   475,   201,   341,  1216,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,    83,     4,   198,   484,   469,   502,   503,   401,
     505,   506,   507,   508,   662,   469,   664,    90,   387,   514,
     197,   193,   517,   939,   939,   179,    27,    28,   170,   673,
     469,   526,   966,   528,   524,  1735,   475,   106,   107,   193,
     193,   536,   193,   544,  1161,   134,   135,  1164,   801,   544,
     732,   546,   409,   806,  1467,   409,  1469,   134,   135,   164,
    1145,   179,   501,  1412,   418,   409,   179,    57,   422,   922,
     196,   425,   177,   922,   193,   193,   102,   103,   673,    69,
     193,   193,   155,   765,   854,   524,   856,    83,   974,    70,
     193,   196,  1792,  1106,    90,    83,   535,   520,   593,   538,
      83,   193,    90,   193,   105,   193,   741,    90,   197,   190,
     132,   133,   549,    70,   158,   196,   193,   163,   105,   106,
     107,   475,   476,   477,   478,   479,   480,   481,   197,    70,
      83,   691,    83,   781,   782,   179,   643,    90,   482,    90,
     788,   789,    70,   226,   193,  1494,    14,   501,   643,   193,
     195,   196,   196,   134,   135,   196,    31,   839,    32,   155,
     156,   157,  1575,   162,    32,   193,  1579,   849,   156,   157,
     524,    38,   155,   156,   157,    50,   195,   158,    53,   180,
      31,   195,   196,    51,   538,   349,   350,   788,   683,   195,
    1347,   105,   106,   107,    70,   549,   619,   620,   195,    50,
     195,   696,    53,   156,   157,   156,   157,  1179,    81,   195,
    1063,   195,   193,   195,  1063,   569,   217,   195,   587,   588,
     119,   120,   121,   646,  1341,   226,    70,    53,    54,    55,
     103,    57,   233,  1766,  1767,   730,  1170,   591,   592,  1762,
    1763,  1326,    70,    69,   109,   110,   111,  1181,   111,    70,
     251,   825,   826,   395,   255,    70,   119,   120,   121,   122,
     123,   124,  1219,   196,   656,   760,   139,   140,   141,   158,
     624,   625,   193,   193,    70,   358,   158,   193,  1031,   162,
     195,    48,    69,   179,   367,   193,   369,  1880,   158,  1446,
     163,   374,   165,   166,   167,   168,   169,   170,   793,   193,
     200,   384,  1895,     9,   158,   728,   158,   193,     8,   805,
     195,   158,   193,    14,   809,   158,   673,  1194,  1195,  1196,
     193,  1734,     9,  1200,   195,  1738,   189,   196,   130,   195,
     130,   195,    81,   475,    83,    84,    14,   691,   194,   179,
     119,   120,   121,   122,   123,   124,  1199,    14,   102,  1306,
    1199,   199,   194,   194,   103,   194,   194,   358,   193,   501,
     111,   193,  1479,   193,     9,   155,   367,   194,   369,   194,
     194,   194,   381,   374,  1531,    94,   385,   814,     9,   195,
      14,   179,   524,   384,   193,     9,   193,    83,   196,   195,
     139,   140,   141,   535,   195,   818,   538,   196,   195,   894,
     823,   896,   411,   898,   413,   414,   415,   416,   409,  1343,
     189,   194,   907,   973,   163,    56,   165,   166,   194,   168,
     169,   170,   194,   777,   132,   779,   921,   193,   511,    50,
      51,    52,    53,    54,    55,   925,   119,   120,   121,   122,
     123,   124,   799,   200,   194,   799,  1846,   196,    69,   198,
       9,   910,   947,     9,   910,   799,   200,   910,   200,   813,
     814,  1874,   957,   886,    70,   960,  1866,   962,    32,   133,
    1152,   966,   158,   910,   178,  1875,  1889,   136,  1355,     9,
    1357,   974,   194,   158,   191,   854,   925,   856,    14,     9,
       9,   860,   861,   862,   851,   180,   194,   851,   937,     9,
     939,    14,   132,   200,   858,   859,   189,   851,   200,   197,
     511,   974,   195,     9,    14,  1010,   200,   194,   910,   200,
     974,  1017,   194,    50,    51,    52,    53,    54,    55,   158,
      57,  1465,   193,   887,     4,   974,   194,   102,   195,  1292,
     195,   933,    69,     9,    91,   136,   158,     9,   549,    70,
     194,    27,    28,   910,   193,    70,   910,   158,   950,  1412,
     193,    70,  1123,  1412,   158,     9,   910,    14,   196,   196,
     195,   925,  1062,  1012,   931,  1065,   180,   931,   197,    49,
       9,  1018,  1005,   937,    14,   939,   196,   931,   196,   200,
      14,   119,   120,   121,   122,   123,   124,   194,   191,   682,
     195,    32,   130,   131,   193,   193,    32,    14,  1485,   193,
    1487,   193,  1489,    14,    52,  1492,   193,    70,    70,   973,
      70,   193,  1123,  1376,     9,   194,   193,  1050,  1123,   195,
     136,   985,   986,   987,  1057,   276,    14,   278,   195,  1392,
     180,  1494,   112,   171,   136,  1494,     9,   117,   194,   119,
     120,   121,   122,   123,   124,   125,    69,   200,  1012,   742,
       9,   189,  1157,  1020,  1018,  1022,  1020,    83,  1022,     9,
    1786,  1130,   197,   197,  1130,  1170,  1020,  1130,  1022,   195,
     136,   682,   193,   195,    14,   193,  1181,  1182,  1042,    83,
     194,   196,   333,  1130,   193,   165,   166,   194,   168,   193,
     783,   196,   785,   196,   195,     9,   155,    91,  1062,  1586,
     200,  1065,  1208,   136,   196,    32,    77,   195,   195,   189,
     194,  1216,   180,    32,   136,   194,   194,   197,   811,   200,
       9,  1226,     9,   200,   136,   200,     9,  1106,  1130,   194,
    1094,   742,     9,   197,  1497,   196,    31,   195,  1864,   194,
     226,   197,   195,  1506,    14,    83,   193,   196,   194,   194,
     194,   193,   195,     9,   136,     9,  1519,   194,  1191,  1161,
     136,     9,  1164,  1130,  1211,   194,  1130,   194,   419,   200,
     922,   422,   783,   925,   785,   200,  1130,    32,   194,   200,
     195,   874,   195,   112,   194,   937,    81,   939,   799,   195,
     167,   196,   195,  1447,   163,    14,    91,   890,   891,    83,
     811,   117,   194,   814,   194,   196,  1311,   194,   103,    14,
     136,   136,  1245,  1318,   179,   196,  1249,  1322,    83,  1324,
     195,  1254,    14,    14,    83,  1442,   194,  1332,  1261,  1193,
    1194,  1195,  1196,  1596,   194,   193,  1200,  1342,  1343,   136,
     851,   195,   136,   195,   139,   140,   141,  1211,    14,    14,
     195,    14,   196,  1740,  1741,     9,  1223,     9,   197,  1223,
    1012,    59,    83,   874,   159,   193,   179,   162,    83,  1223,
     165,   166,   358,   168,   169,   170,     9,   115,   196,   890,
     891,   367,   102,   195,   158,   102,    36,   180,   374,   170,
      14,   193,   195,    81,   194,   193,   176,    83,   384,   910,
     173,   180,  1304,   194,   180,     9,    83,  1552,   194,   395,
     195,  1063,   563,  1315,   196,   103,   194,    14,    14,    83,
     931,     4,    83,    14,    83,    14,    83,  1106,  1855,  1362,
     481,  1024,  1025,  1366,  1298,   476,   478,  1304,  1371,  1341,
     128,   967,   913,  1871,  1217,  1595,  1389,  1866,  1315,   595,
    1582,   139,   140,   141,  1524,  1619,  1532,  1462,  1704,  1437,
    1465,    81,  1899,    83,    84,  1887,    49,  1716,  1433,  1732,
      27,    28,  1578,  1061,    31,  1196,   378,   165,   166,  1742,
     168,   169,   170,   103,  1134,  1058,  1192,   986,  1193,  1007,
     937,  1355,   375,  1357,   422,   825,  1821,  1114,  1043,    56,
    1094,   652,   653,    -1,  1425,   193,    -1,  1018,    -1,  1020,
     661,  1022,    -1,  1024,  1025,    -1,    -1,    -1,    -1,   139,
     140,   141,    -1,    -1,  1787,   511,    -1,  1527,    -1,   112,
      -1,  1124,    -1,    -1,   117,    -1,   119,   120,   121,   122,
     123,   124,   125,   163,    -1,   165,   166,  1199,   168,   169,
     170,    -1,    -1,    -1,    -1,  1422,  1503,    -1,  1422,    -1,
      -1,    -1,    -1,  1156,    -1,  1570,  1468,  1431,  1422,  1832,
      -1,    -1,  1474,  1437,  1476,  1720,   196,  1479,   198,    -1,
    1447,    -1,   165,   166,    -1,   168,    -1,  1441,    -1,    -1,
      -1,    -1,    -1,    -1,  1496,    -1,    -1,    -1,    -1,    -1,
      -1,  1468,    -1,    -1,    -1,    -1,   189,  1474,    -1,  1476,
      -1,  1204,    -1,  1124,   197,    -1,    -1,    -1,    -1,  1130,
      -1,  1485,    -1,  1487,    -1,  1489,  1219,  1220,  1492,  1496,
    1893,  1498,    -1,    -1,  1498,    -1,    -1,  1900,    -1,  1503,
    1507,    -1,    -1,  1507,  1498,  1156,    -1,  1594,  1595,    -1,
      -1,    -1,    -1,  1507,    -1,    -1,    -1,    -1,    -1,    -1,
    1524,    -1,    -1,  1527,    -1,    -1,  1530,    -1,    -1,   226,
      -1,    -1,    -1,    -1,   825,   826,  1540,    31,    -1,    -1,
      -1,  1715,  1584,  1547,    -1,  1839,    -1,    -1,    -1,    -1,
    1554,    -1,  1556,  1204,    81,    -1,   682,    -1,    -1,  1563,
    1211,    -1,    -1,    -1,    -1,    59,  1560,    -1,  1219,  1220,
    1780,    -1,  1223,  1306,    -1,  1715,   103,  1584,    -1,   276,
    1725,   278,  1586,    -1,    -1,    -1,  1593,    81,    -1,  1593,
    1594,  1595,  1599,    -1,    -1,  1599,    -1,    -1,  1605,  1593,
      -1,  1605,  1859,    -1,    -1,  1599,    -1,    -1,    -1,   103,
      -1,  1605,   139,   140,   141,    -1,   742,   111,    -1,    -1,
    1412,   912,    -1,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,   333,   928,   165,   166,
      -1,   168,   169,   170,   138,   139,   140,   141,   142,    69,
     941,    -1,    -1,    -1,    -1,  1306,    -1,   783,    -1,   785,
      -1,   358,    -1,    -1,    -1,   159,   193,    -1,   162,   163,
     367,   165,   166,    -1,   168,   169,   170,   374,    -1,   970,
      -1,    -1,    -1,  1756,  1757,   811,    -1,   384,    -1,   183,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,   395,   193,
      -1,    -1,  1494,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1715,    -1,    -1,    -1,    -1,  1449,    -1,    -1,    -1,
      -1,    -1,   419,    -1,    -1,   422,    -1,    -1,    -1,    -1,
      -1,    -1,  1736,    -1,    -1,    -1,  1740,  1741,    -1,  1746,
      -1,    -1,  1746,    -1,    -1,    -1,    -1,  1038,   874,    -1,
    1041,  1755,  1746,    -1,    -1,    -1,    -1,    -1,  1762,  1763,
    1905,    -1,  1766,  1767,   890,   891,    -1,    -1,  1913,    -1,
      -1,  1422,   469,    -1,  1919,    -1,  1780,  1922,    -1,    -1,
      -1,  1788,    -1,    -1,  1788,    -1,    -1,    -1,  1795,    -1,
      -1,  1795,    -1,    -1,  1788,    -1,   922,    -1,  1449,    -1,
      -1,  1795,    -1,    81,  1846,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     4,    -1,   511,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1830,  1866,   103,  1830,    -1,    -1,    -1,
      -1,    -1,  1839,  1875,  1838,    -1,  1830,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1852,    -1,  1498,  1852,    -1,
       4,    -1,  1503,  1144,  1858,  1146,  1507,    49,  1852,    -1,
      -1,   139,   140,   141,    -1,    -1,   563,    -1,   565,    -1,
      -1,   568,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   159,  1173,    -1,   162,  1176,    -1,   165,   166,    -1,
     168,   169,   170,   226,  1901,    49,    -1,  1901,  1024,  1025,
      -1,  1908,   599,    -1,  1908,    -1,    -1,  1901,    -1,    -1,
      -1,    -1,    -1,    -1,  1908,    -1,    -1,    -1,    -1,   197,
     112,    -1,    -1,    -1,    -1,   117,    -1,   119,   120,   121,
     122,   123,   124,   125,    -1,    81,    -1,  1063,    -1,    -1,
      -1,  1232,  1593,  1594,  1595,  1236,    -1,    -1,  1599,    -1,
      -1,    -1,    -1,    -1,  1605,   652,   653,   103,   112,    -1,
      -1,    -1,    -1,   117,   661,   119,   120,   121,   122,   123,
     124,   125,    -1,   165,   166,    -1,   168,   119,   120,   121,
     122,   123,   124,    -1,    -1,   682,    -1,    -1,   130,   131,
      -1,    -1,    -1,   139,   140,   141,    -1,   189,  1124,    -1,
      -1,    -1,    -1,     4,    -1,   197,    -1,    -1,  1299,  1300,
      -1,   165,   166,    -1,   168,    -1,   162,    -1,    -1,   165,
     166,    -1,   168,   169,   170,   358,    -1,   169,    -1,   171,
    1156,    -1,    -1,    -1,   367,   189,    -1,    -1,    -1,    -1,
      -1,   374,   184,   197,   186,   742,    -1,   189,    49,    -1,
     196,   384,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,    -1,    -1,  1199,    -1,    31,    -1,    -1,  1204,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   783,    -1,   785,    -1,
      -1,    -1,    -1,  1219,  1220,  1746,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    59,    -1,  1396,    -1,  1398,    67,    68,
      -1,   112,    -1,    -1,   811,   812,   117,    -1,   119,   120,
     121,   122,   123,   124,   125,    81,    -1,    -1,   825,   826,
     827,   828,   829,   830,   831,    -1,    -1,  1788,    -1,    -1,
      -1,   838,    -1,    -1,  1795,     4,    -1,   103,    -1,    -1,
      -1,  1442,    -1,    -1,    -1,   852,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   165,   166,    -1,   168,    -1,    -1,
      -1,    -1,     4,    -1,    -1,   134,   135,   874,   511,  1830,
    1306,    -1,   138,   139,   140,   141,   142,    -1,   189,    -1,
      49,   888,    -1,   890,   891,    -1,   197,    -1,    27,    28,
      -1,  1852,    31,   159,    -1,    -1,   162,   163,    -1,   165,
     166,    81,   168,   169,   170,   912,   913,    49,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   922,    -1,   183,    -1,    -1,
      -1,   928,    -1,   103,    -1,   194,    -1,   193,    -1,    -1,
      -1,    -1,    -1,    -1,   941,    -1,    -1,    -1,    -1,    -1,
    1901,    -1,   949,   112,    -1,   952,    -1,  1908,   117,    -1,
     119,   120,   121,   122,   123,   124,   125,    -1,    -1,   139,
     140,   141,    -1,   970,    -1,    -1,    -1,   974,    -1,    -1,
     112,    -1,    -1,    -1,    -1,   117,  1412,   119,   120,   121,
     122,   123,   124,   125,    -1,   165,   166,    -1,   168,   169,
     170,    -1,    -1,    -1,    -1,    -1,   165,   166,    -1,   168,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1449,    -1,    -1,  1617,  1024,  1025,    -1,
     189,    -1,    -1,   165,   166,    -1,   168,    -1,   197,    -1,
      -1,  1038,    -1,    -1,  1041,    81,  1043,    -1,    -1,   682,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   189,    -1,    -1,
      -1,  1058,  1059,  1060,  1061,   197,  1063,   103,  1494,  1066,
    1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,  1092,  1093,   226,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,    -1,    -1,    -1,   742,
      -1,    -1,    -1,  1110,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1124,  1719,   165,
     166,    -1,   168,   169,   170,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1144,    -1,  1146,
     783,    -1,   785,    -1,    -1,    -1,    -1,   193,    -1,  1156,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,  1173,    -1,   811,  1176,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1199,    -1,    -1,    -1,    -1,  1204,    -1,    -1,
      -1,    -1,    -1,  1804,    67,    68,    -1,    -1,    -1,    -1,
      -1,    -1,  1219,  1220,    -1,  1222,    -1,    -1,    -1,   358,
      -1,    -1,    -1,    67,    68,  1232,    31,    -1,   367,  1236,
      -1,   874,  1239,    -1,  1241,   374,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   384,    -1,   890,   891,    -1,
      -1,    -1,    -1,    -1,    59,    -1,   395,    -1,  1265,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   134,   135,    -1,    -1,    -1,    81,    -1,    -1,  1880,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,   135,  1299,  1300,  1895,    -1,  1303,    -1,   103,  1306,
      30,    31,    56,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
     469,   194,    -1,   138,   139,   140,   141,   142,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,
     194,    -1,    -1,    -1,   159,    -1,    -1,   162,   163,    -1,
     165,   166,    81,   168,   169,   170,    -1,   172,    -1,    -1,
      -1,    -1,   511,    59,    -1,    -1,    -1,    -1,   183,    -1,
      -1,  1024,  1025,    -1,   103,    -1,    -1,    -1,   193,  1396,
      -1,  1398,    -1,    -1,    -1,    81,    -1,  1404,    -1,  1406,
      -1,  1408,    -1,    -1,  1411,  1412,   125,    -1,  1415,    -1,
    1417,    -1,    -1,  1420,    -1,    -1,    -1,   103,    -1,   138,
     139,   140,   141,   142,  1431,  1432,    -1,    -1,  1435,   568,
      -1,    -1,    -1,    -1,    -1,  1442,    -1,    -1,    -1,    -1,
     159,    -1,  1449,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,   138,   139,   140,   141,   142,    -1,    -1,    -1,
     599,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     200,    -1,    -1,   159,    -1,    -1,   162,   163,    -1,   165,
     166,  1124,   168,   169,   170,    -1,   172,  1494,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,
      -1,    -1,    -1,  1510,    -1,    -1,    -1,   193,    -1,    -1,
      -1,    -1,    -1,  1156,    -1,  1522,  1523,    -1,    -1,    -1,
      -1,    -1,   276,  1530,   278,  1532,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,   682,    -1,    -1,    -1,  1554,    -1,  1556,
      -1,    -1,    -1,    -1,    -1,    -1,  1563,    -1,    -1,    -1,
      -1,  1204,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,  1219,  1220,    -1,   333,
      67,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1602,  1603,  1604,    10,    11,
      12,    -1,  1609,   742,  1611,    -1,    -1,    -1,    31,    -1,
    1617,    -1,  1619,    -1,    -1,    67,    68,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,   783,    57,   785,   134,   135,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    81,    -1,
      -1,    -1,    -1,  1306,    -1,   419,    -1,    -1,   422,    -1,
      -1,    -1,   811,   812,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,   134,   135,    -1,    -1,    -1,    -1,   827,   828,
     829,   830,   831,    -1,    -1,    -1,    -1,    -1,    -1,   838,
      -1,    -1,   125,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1719,   852,    -1,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,   568,  1736,
      -1,    78,    79,    80,    81,   874,   159,    -1,    -1,   162,
     163,    -1,   165,   166,    91,   168,   169,   170,  1755,   888,
      -1,   890,   891,    -1,  1761,    -1,   103,    -1,    -1,   599,
      -1,    -1,    -1,    -1,    -1,  1772,    -1,    -1,    -1,    -1,
      -1,  1778,    -1,    -1,   913,  1782,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   922,    -1,   197,    78,    79,    80,    -1,
      81,   138,   139,   140,   141,    -1,    -1,  1804,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,  1449,   154,    -1,   563,
     949,   565,   103,   952,   568,    -1,    -1,    -1,   165,   166,
      -1,   168,   169,   170,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   974,   183,  1844,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   599,    -1,  1854,   139,   140,
     141,   143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,  1871,    -1,    -1,    -1,   160,   161,
      -1,    -1,   163,  1880,   165,   166,    -1,   168,   169,   170,
      -1,    -1,   174,    -1,    -1,  1024,  1025,    -1,  1895,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   188,    -1,   652,   653,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   661,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1058,
    1059,  1060,  1061,    -1,  1063,    -1,    -1,  1066,  1067,  1068,
    1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,  1093,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,  1110,   812,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,  1124,    -1,   827,   828,   829,
     830,    -1,    -1,    -1,    -1,    10,    11,    12,   838,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,  1156,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    81,    -1,   812,    -1,
    1199,    -1,    -1,    -1,    -1,  1204,    -1,    -1,   103,    -1,
      -1,   825,   826,   827,   828,   829,   830,   831,   103,    -1,
    1219,  1220,    -1,  1222,   838,    -1,   111,   112,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1239,    -1,  1241,    -1,   139,   140,   141,    -1,    -1,   949,
      -1,    -1,    -1,    -1,   139,   140,   141,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,  1265,    -1,    -1,    -1,
     165,   166,    -1,   168,   169,   170,    -1,   162,    -1,    -1,
     165,   166,   103,   168,   169,   170,    27,    28,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   912,    -1,
      -1,    -1,    -1,    -1,  1303,    -1,    -1,  1306,    -1,    -1,
      -1,    -1,    -1,    -1,   928,    -1,    -1,    -1,   139,   140,
     141,    -1,   197,    -1,    -1,    -1,    -1,   941,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   949,    -1,    -1,   159,    -1,
      -1,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    -1,    -1,    -1,    -1,   970,    -1,    -1,  1059,
    1060,  1061,    -1,    -1,    -1,    -1,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,  1093,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1404,    -1,  1406,    -1,  1408,
    1110,    -1,  1411,  1412,    -1,    -1,  1415,    -1,  1417,    -1,
      -1,  1420,    -1,    -1,  1038,    -1,    -1,  1041,    -1,  1043,
      -1,    -1,    -1,  1432,    -1,    -1,  1435,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1058,  1059,  1060,  1061,    -1,    -1,
    1449,    -1,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,
    1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,
      -1,    -1,    -1,    -1,    -1,   226,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1494,  1110,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1510,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1222,  1522,  1523,    -1,    -1,    -1,    -1,    -1,
    1144,    -1,  1146,  1532,    -1,    -1,    -1,    -1,    -1,  1239,
      -1,  1241,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1173,
      -1,    -1,  1176,    30,    31,  1265,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,  1602,  1603,  1604,    -1,    -1,  1222,    -1,
    1609,    -1,  1611,    -1,    -1,    -1,    -1,   358,  1232,    -1,
    1619,    -1,  1236,    -1,    -1,  1239,   367,  1241,    -1,    -1,
      -1,    -1,    -1,   374,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,   384,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1265,    -1,    -1,   395,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,  1299,  1300,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1404,    -1,  1406,    -1,  1408,    -1,
      -1,  1411,    10,    11,    12,  1415,    -1,  1417,    -1,    -1,
    1420,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   469,    -1,
     197,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,  1761,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     511,    69,    -1,  1772,    -1,    -1,    -1,    -1,    81,  1778,
      -1,    -1,  1396,  1782,  1398,    -1,    -1,    -1,    -1,    -1,
    1404,    -1,  1406,    -1,  1408,    -1,    -1,  1411,    -1,    -1,
     103,  1415,    -1,  1417,    -1,    -1,  1420,    -1,   111,   112,
    1510,    -1,    -1,    -1,    -1,    -1,    -1,  1431,    -1,    -1,
      -1,    -1,   197,    -1,    -1,    -1,    -1,   568,  1442,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,    -1,
      -1,    -1,    -1,    -1,    -1,  1844,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,  1854,    -1,    -1,   599,   162,
      -1,    -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,
      31,    -1,  1871,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,  1510,    -1,    -1,   197,
      -1,    -1,  1602,  1603,  1604,    -1,    -1,    -1,    69,  1609,
      -1,    -1,    -1,    -1,    -1,    -1,  1530,    -1,    -1,    -1,
      -1,    81,    -1,    83,    -1,    85,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1554,   682,  1556,   103,    -1,    -1,    -1,    30,    31,  1563,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,   136,    -1,    -1,    -1,   139,
     140,   141,    -1,    -1,    -1,    -1,    69,    -1,  1602,  1603,
    1604,    10,    11,    12,    -1,  1609,    -1,    -1,    -1,    -1,
      -1,   742,    -1,  1617,    -1,   165,   166,    -1,   168,   169,
     170,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      81,    -1,   783,    -1,   785,    -1,    -1,    -1,    -1,    -1,
      69,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1761,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     811,   812,  1772,   103,    -1,    -1,    -1,    -1,  1778,    -1,
      -1,    -1,  1782,    -1,    -1,    -1,   827,   828,   829,   830,
     831,    -1,    -1,    -1,    -1,    -1,    -1,   838,   139,   140,
     141,    -1,    -1,    -1,    -1,  1719,    -1,    -1,    -1,   139,
     140,   141,    -1,    -1,   197,    -1,    -1,    -1,    -1,    -1,
      -1,   162,  1736,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,   162,   874,    -1,   165,   166,    -1,   168,   169,
     170,  1755,    -1,    -1,  1844,    -1,    -1,  1761,    -1,   890,
     891,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1772,    -1,
      -1,    -1,    -1,    -1,  1778,    -1,    -1,    -1,  1782,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,    -1,
      -1,   922,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1804,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   949,    57,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1844,    -1,    -1,   974,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,  1880,    12,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1895,    -1,  1024,  1025,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,  1058,  1059,  1060,
    1061,    -1,  1063,    -1,    69,  1066,  1067,  1068,  1069,  1070,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,  1093,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1110,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,  1124,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,   197,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1156,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,  1199,    -1,
      91,    -1,    -1,  1204,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,  1219,  1220,
     111,  1222,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,  1239,    -1,
    1241,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
      -1,    -1,    -1,   154,  1265,    -1,    -1,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,
      -1,    -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,
      -1,    -1,   193,    -1,    -1,  1306,    -1,   198,   199,    -1,
     201,   202,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,  1404,    -1,  1406,    91,  1408,    -1,    -1,
    1411,  1412,    -1,    -1,  1415,    -1,  1417,    -1,   103,  1420,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,  1449,    -1,
      -1,    -1,   137,   138,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,   174,
      -1,    -1,   177,  1494,    -1,    -1,    -1,    -1,   183,    -1,
      10,    11,    12,   188,   189,   190,    -1,    -1,   193,  1510,
     195,    -1,    -1,   198,   199,    -1,   201,   202,    -1,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1602,  1603,  1604,    -1,    -1,    -1,    -1,  1609,    49,
      50,    51,    -1,    -1,    -1,    -1,    56,  1618,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,   114,    -1,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   195,   126,   127,   128,   129,
     130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,   143,   144,   145,   146,   147,    -1,    -1,
      -1,   151,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,
     160,   161,   162,   163,    -1,   165,   166,    -1,   168,   169,
     170,   171,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,
      -1,    -1,    -1,   183,   184,    -1,   186,    -1,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,   197,   198,   199,
    1761,   201,   202,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1772,    -1,    -1,    -1,    -1,    -1,  1778,    -1,    -1,
      -1,  1782,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,  1806,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,
      -1,    -1,    56,  1844,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,
      94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
     114,    -1,   116,   117,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,   129,   130,   131,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,   171,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
     184,    -1,   186,    -1,   188,   189,   190,    -1,    -1,   193,
      -1,   195,   196,   197,   198,   199,    -1,   201,   202,     3,
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
     104,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
     114,    -1,   116,   117,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,   129,   130,   131,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,   171,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
     184,    -1,   186,    -1,   188,   189,   190,    -1,    -1,   193,
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,     3,
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
      -1,   195,   196,   197,   198,   199,    -1,   201,   202,     3,
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
      -1,   195,   196,   197,   198,   199,    -1,   201,   202,     3,
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
      -1,   195,   196,   197,   198,   199,    -1,   201,   202,     3,
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
      -1,   195,   196,   197,   198,   199,    -1,   201,   202,     3,
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
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,     3,
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
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,     3,
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
      -1,   195,   196,   197,   198,   199,    -1,   201,   202,     3,
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
      -1,   195,   196,   197,   198,   199,    -1,   201,   202,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,
      94,    -1,    96,    -1,    98,    99,   100,    -1,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,
     114,    -1,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,   171,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,     3,
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
      -1,   195,   196,   197,   198,   199,    -1,   201,   202,     3,
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
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,   171,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,     3,
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
      -1,   195,   196,   197,   198,   199,    -1,   201,   202,     3,
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
      -1,   195,   196,   197,   198,   199,    -1,   201,   202,     3,
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
      -1,   195,   196,   197,   198,   199,    -1,   201,   202,     3,
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
      -1,   195,   196,   197,   198,   199,    -1,   201,   202,     3,
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
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,     3,
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
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,     3,
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
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,     3,
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
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,     3,
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
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,     3,
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
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    11,    12,   193,
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,     3,
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
      -1,    -1,    -1,    -1,   198,   199,    -1,   201,   202,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
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
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    12,   193,
      -1,    -1,   196,    -1,   198,   199,    -1,   201,   202,     3,
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
      -1,    -1,    -1,    -1,   198,   199,    -1,   201,   202,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
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
      10,    11,    12,    -1,   198,   199,    -1,   201,   202,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    69,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,   108,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,
     124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,   197,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
      -1,    -1,    -1,    -1,   198,   199,    -1,   201,   202,     3,
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
      -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
      10,    11,    12,    -1,   198,   199,    -1,   201,   202,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    69,
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
     174,    -1,    -1,   177,    -1,    -1,    -1,   197,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
      -1,   195,    11,    12,   198,   199,    -1,   201,   202,     3,
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
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
      -1,   195,    -1,    -1,   198,   199,    -1,   201,   202,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
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
      10,    11,    12,    -1,   198,   199,    -1,   201,   202,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    69,
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
     174,    -1,    -1,   177,    -1,   195,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
     194,    -1,    -1,    -1,   198,   199,    -1,   201,   202,     3,
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
      -1,    -1,    -1,    -1,   198,   199,    -1,   201,   202,     3,
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
      -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
      -1,    -1,    -1,    -1,   198,   199,    -1,   201,   202,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    50,    51,    -1,    -1,
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
      -1,    -1,    -1,    -1,   198,   199,    -1,   201,   202,     3,
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
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
      -1,    -1,    -1,    -1,   198,   199,    -1,   201,   202,     3,
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
      -1,    -1,    -1,    -1,   198,   199,    -1,   201,   202,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   198,   199,    -1,   201,   202,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    50,    51,    -1,    -1,
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
      10,    11,    12,    -1,   198,   199,    -1,   201,   202,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    69,
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
     174,    -1,    -1,   177,    -1,   195,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
      10,    11,    12,    -1,   198,   199,    -1,   201,   202,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    69,
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
     174,    -1,    -1,   177,    -1,   195,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
      -1,    -1,    -1,    -1,   198,   199,    -1,   201,   202,     3,
       4,    -1,     6,     7,    -1,    -1,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    29,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    57,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,    -1,    -1,   130,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
      -1,    10,    11,    12,    13,   159,    -1,    -1,    -1,    -1,
      -1,   165,   166,    -1,   168,   169,   170,   171,    27,   173,
      29,    -1,   176,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   196,    -1,   198,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
     159,    10,    11,    12,    13,    -1,   165,   166,    -1,   168,
     169,   170,   171,    -1,   173,    -1,    -1,   176,    27,    -1,
      29,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   196,    -1,   198,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     159,    -1,    -1,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,   171,    -1,   173,    -1,    -1,   176,     3,     4,
      -1,     6,     7,    -1,   183,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,   197,    -1,
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
     125,   126,    -1,    -1,    -1,    -1,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   159,    -1,    -1,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,   171,    -1,   173,    -1,
      -1,   176,     3,     4,    -1,     6,     7,    -1,   183,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,   193,    -1,
      -1,    -1,   197,    -1,    -1,    -1,    27,    -1,    29,    -1,
      31,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    57,    -1,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,    -1,    -1,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,
      -1,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
     171,    -1,   173,    -1,    -1,   176,     3,     4,    -1,     6,
       7,    -1,   183,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,   193,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   159,    -1,    -1,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,   171,    -1,   173,    -1,    -1,   176,
      -1,     3,     4,    -1,     6,     7,   183,   184,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,
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
     122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,   171,
      -1,   173,    -1,    -1,   176,     3,     4,     5,     6,     7,
      -1,   183,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   193,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   159,   160,   161,    -1,    -1,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,   173,   174,    -1,   176,    10,
      11,    12,    -1,    -1,    -1,   183,   184,    -1,   186,    -1,
     188,   189,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
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
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,   195,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,   195,    -1,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,    69,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   194,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,   194,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,    -1,    -1,
     130,   131,   132,   133,   194,    -1,    -1,   137,   138,   139,
     140,   141,   142,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,    -1,    10,    11,    12,    13,   159,
      -1,    -1,    -1,    -1,    -1,   165,   166,    -1,   168,   169,
     170,   171,    27,   173,    29,    -1,   176,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,    -1,    -1,   130,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,
      10,    11,    12,    13,   159,    -1,    -1,    -1,    -1,    -1,
     165,   166,    -1,   168,   169,   170,   171,    27,   173,    29,
      -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,    -1,    -1,
     130,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,    -1,    10,    11,    12,    13,   159,
      -1,    -1,    -1,    -1,    -1,   165,   166,    -1,   168,   169,
     170,   171,    27,   173,    29,    -1,   176,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    31,    -1,    -1,    -1,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      59,    -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,
     165,   166,    -1,   168,   169,   170,   171,    -1,   173,    -1,
      -1,   176,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    32,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    -1,    -1,    -1,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,
     159,    -1,    -1,   162,   163,    -1,   165,   166,    91,   168,
     169,   170,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,    -1,
     189,    -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,    -1,
     143,   144,   145,   146,   147,    -1,    38,    -1,    -1,    -1,
      -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,
     183,    -1,    -1,    -1,    -1,   188,    78,    79,    80,    81,
     193,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,    50,
      51,    -1,   174,    -1,    -1,    56,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   188,    -1,    -1,    70,
      -1,   193,    -1,    -1,    -1,    -1,   198,    78,    79,    80,
      81,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    31,    -1,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,    69,   143,   144,   145,   146,   147,    -1,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
      70,    -1,    -1,   174,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,   183,    83,    84,    -1,    -1,   188,    -1,    -1,
      -1,    91,   193,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,    -1,   143,   144,   145,   146,   147,    -1,    -1,
      -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,
     160,   161,   162,   163,    -1,   165,   166,    -1,   168,   169,
     170,    70,    -1,    -1,   174,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,   188,    -1,
      -1,    -1,    91,   193,    -1,    -1,   196,    -1,   198,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,    -1,   143,   144,   145,   146,   147,    -1,
      -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,
     159,   160,   161,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,    70,    -1,    72,   174,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,   188,
      -1,    -1,    -1,    91,   193,    -1,    -1,    -1,    -1,   198,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    70,    -1,    -1,   174,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
     188,    -1,    -1,    -1,    91,   193,    -1,    -1,    -1,    -1,
     198,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,    -1,   143,   144,   145,   146,
     147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,    70,    -1,    -1,   174,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,   188,    -1,    -1,    -1,    91,   193,    -1,    -1,    -1,
      -1,   198,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,    -1,   143,   144,   145,
     146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,
      -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,    -1,    -1,    -1,   174,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   188,    -1,    -1,    -1,    -1,   193,    -1,    -1,
      30,    31,   198,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
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
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,    -1,
     143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,
      -1,   154,    -1,    -1,   136,    -1,   159,   160,   161,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,
      -1,   174,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    -1,   188,    91,    -1,    -1,    -1,
     193,    -1,    -1,    -1,    -1,   198,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,   174,
      -1,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
      -1,    -1,    -1,   188,    91,    -1,    -1,    -1,   193,    -1,
      -1,    -1,    -1,   198,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,    -1,   143,   144,   145,   146,
     147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,    -1,    -1,    -1,   174,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   188,    -1,    -1,    -1,    -1,   193,    28,    -1,    30,
      31,   198,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   102,    30,    31,    32,    33,    34,    35,    36,    37,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   204,   205,     0,   206,     3,     4,     5,     6,     7,
      13,    27,    28,    29,    49,    50,    51,    56,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    70,
      71,    72,    73,    74,    78,    79,    80,    81,    82,    83,
      84,    86,    87,    91,    92,    93,    94,    96,    98,   100,
     103,   104,   108,   109,   110,   111,   112,   113,   114,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   126,   127,
     128,   129,   130,   131,   137,   138,   139,   140,   141,   143,
     144,   145,   146,   147,   151,   154,   159,   160,   161,   162,
     163,   165,   166,   168,   169,   170,   171,   174,   177,   183,
     184,   186,   188,   189,   190,   193,   195,   196,   198,   199,
     201,   202,   207,   210,   220,   221,   222,   223,   224,   227,
     243,   244,   248,   251,   258,   264,   324,   325,   333,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   348,
     351,   363,   364,   371,   374,   377,   383,   385,   386,   388,
     398,   399,   400,   402,   407,   411,   431,   439,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     454,   467,   469,   471,   122,   123,   124,   137,   159,   169,
     193,   210,   243,   324,   345,   443,   345,   193,   345,   345,
     345,   108,   345,   345,   345,   429,   430,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,    81,
      83,    91,   124,   139,   140,   141,   154,   193,   221,   364,
     399,   402,   407,   443,   446,   443,    38,   345,   458,   459,
     345,   124,   130,   193,   221,   256,   399,   400,   401,   403,
     407,   440,   441,   442,   450,   455,   456,   193,   334,   404,
     193,   334,   355,   335,   345,   229,   334,   193,   193,   193,
     334,   195,   345,   210,   195,   345,     3,     4,     6,     7,
      10,    11,    12,    13,    27,    29,    31,    57,    59,    71,
      72,    73,    74,    75,    76,    77,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     130,   131,   132,   133,   137,   138,   142,   159,   163,   171,
     173,   176,   183,   193,   210,   211,   212,   223,   472,   489,
     490,   493,   195,   340,   342,   345,   196,   236,   345,   111,
     112,   162,   213,   214,   215,   216,   220,    83,   198,   290,
     291,   123,   130,   122,   130,    83,   292,   193,   193,   193,
     193,   210,   262,   475,   193,   193,    70,    70,    70,   335,
      83,    90,   155,   156,   157,   464,   465,   162,   196,   220,
     220,   210,   263,   475,   163,   193,   475,   475,    83,   190,
     196,   356,    27,   333,   337,   345,   346,   443,   447,   225,
     196,    90,   405,   464,    90,   464,   464,    32,   162,   179,
     476,   193,     9,   195,    38,   242,   163,   261,   475,   124,
     189,   243,   325,   195,   195,   195,   195,   195,   195,   195,
     195,    10,    11,    12,    30,    31,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    57,
      69,   195,    70,    70,   196,   158,   131,   169,   171,   184,
     186,   264,   323,   324,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    67,    68,   134,
     135,   433,    70,   196,   438,   193,   193,    70,   196,   198,
     451,   193,   242,   243,    14,   345,   195,   136,    48,   210,
     428,    90,   333,   346,   158,   443,   136,   200,     9,   413,
     257,   333,   346,   443,   476,   158,   193,   406,   433,   438,
     194,   345,    32,   227,     8,   357,     9,   195,   227,   228,
     335,   336,   345,   210,   276,   231,   195,   195,   195,   138,
     142,   493,   493,   179,   492,   193,   111,   493,    14,   158,
     138,   142,   159,   210,   212,   195,   195,   195,   237,   115,
     176,   195,   213,   215,   213,   215,   220,   196,     9,   414,
     195,   102,   162,   196,   443,     9,   195,   130,   130,    14,
       9,   195,   443,   468,   335,   333,   346,   443,   446,   447,
     194,   179,   254,   137,   443,   457,   458,   345,   365,   366,
     335,   380,   380,   195,    70,   433,   155,   465,    82,   345,
     443,    90,   155,   465,   220,   209,   195,   196,   249,   259,
     389,   391,    91,   193,   198,   358,   359,   361,   402,   449,
     451,   469,    14,   102,   470,   352,   353,   354,   286,   287,
     431,   432,   194,   194,   194,   194,   194,   197,   226,   227,
     244,   251,   258,   431,   345,   199,   201,   202,   210,   477,
     478,   493,    38,   172,   288,   289,   345,   472,   193,   475,
     252,   242,   345,   345,   345,   345,    32,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   403,   345,   345,   453,   453,   345,   460,   461,   130,
     196,   211,   212,   450,   451,   262,   210,   263,   475,   475,
     261,   243,    38,   337,   340,   342,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   163,
     196,   210,   434,   435,   436,   437,   450,   453,   345,   288,
     288,   453,   345,   457,   242,   194,   345,   193,   427,     9,
     413,   194,   194,    38,   345,    38,   345,   406,   194,   194,
     194,   450,   288,   196,   210,   434,   435,   450,   194,   225,
     280,   196,   342,   345,   345,    94,    32,   227,   274,   195,
      28,   102,    14,     9,   194,    32,   196,   277,   493,    31,
      91,   223,   486,   487,   488,   193,     9,    50,    51,    56,
      58,    70,   138,   139,   140,   141,   163,   183,   193,   221,
     223,   372,   375,   378,   384,   399,   407,   408,   410,   210,
     491,   225,   193,   235,   196,   195,   196,   195,   102,   162,
     111,   112,   162,   216,   217,   218,   219,   220,   216,   210,
     345,   291,   408,    83,     9,   194,   194,   194,   194,   194,
     194,   194,   195,    50,    51,   482,   484,   485,   132,   267,
     193,     9,   194,   194,   136,   200,     9,   413,     9,   413,
     200,   200,    83,    85,   210,   466,   210,    70,   197,   197,
     206,   208,    32,   133,   266,   178,    54,   163,   178,   393,
     346,   136,     9,   413,   194,   158,   493,   493,    14,   357,
     286,   225,   191,     9,   414,   493,   494,   433,   438,   433,
     197,     9,   413,   180,   443,   345,   194,     9,   414,    14,
     349,   245,   132,   265,   193,   475,   345,    32,   200,   200,
     136,   197,     9,   413,   345,   476,   193,   255,   250,   260,
      14,   470,   253,   242,    72,   443,   345,   476,   200,   197,
     194,   194,   200,   197,   194,    50,    51,    70,    78,    79,
      80,    91,   138,   139,   140,   141,   154,   183,   210,   373,
     376,   379,   416,   418,   419,   423,   426,   210,   443,   443,
     136,   265,   433,   438,   194,   345,   281,    75,    76,   282,
     225,   334,   225,   336,   102,    38,   137,   271,   443,   408,
     210,    32,   227,   275,   195,   278,   195,   278,     9,   413,
      91,   136,   158,     9,   413,   194,   172,   477,   478,   479,
     477,   408,   408,   408,   408,   408,   412,   415,   193,    70,
      70,    70,   158,   193,   408,   158,   196,    10,    11,    12,
      31,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    69,   158,   476,   197,   399,   196,   239,
     215,   215,   210,   216,   216,   220,     9,   414,   197,   197,
      14,   443,   195,   180,     9,   413,   210,   268,   399,   196,
     457,   137,   443,    14,    38,   345,   345,   200,   345,   197,
     206,   493,   268,   196,   392,    14,   194,   345,   358,   450,
     195,   493,   191,   197,    32,   480,   432,    38,    83,   172,
     434,   435,   437,   434,   435,   493,    38,   172,   345,   408,
     286,   193,   399,   266,   350,   246,   345,   345,   345,   197,
     193,   288,   267,    32,   266,   493,    14,   265,   475,   403,
     197,   193,    14,    78,    79,    80,   210,   417,   417,   419,
     421,   422,    52,   193,    70,    70,    70,    90,   155,   193,
       9,   413,   194,   427,    38,   345,   266,   197,    75,    76,
     283,   334,   227,   197,   195,    95,   195,   271,   443,   193,
     136,   270,    14,   225,   278,   105,   106,   107,   278,   197,
     493,   180,   136,   493,   210,   486,     9,   194,   413,   136,
     200,     9,   413,   412,   367,   368,   408,   381,   408,   409,
     381,   130,   211,   358,   360,   362,   194,   130,   211,   408,
     462,   463,   408,   408,   408,    32,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     491,    83,   240,   197,   197,   219,   195,   408,   485,   102,
     103,   481,   483,     9,   296,   194,   193,   337,   342,   345,
     443,   136,   200,   197,   470,   296,   164,   177,   196,   388,
     395,   164,   196,   394,   136,   195,   480,   493,   357,   494,
      83,   172,    14,    83,   476,   443,   345,   194,   286,   196,
     286,   193,   136,   193,   288,   194,   196,   493,   196,   195,
     493,   266,   247,   406,   288,   136,   200,     9,   413,   418,
     421,   369,   370,   419,   382,   419,   420,   382,   155,   358,
     424,   425,   419,   443,   196,   334,    32,    77,   227,   195,
     336,   270,   457,   271,   194,   408,   101,   105,   195,   345,
      32,   195,   279,   197,   180,   493,   136,   172,    32,   194,
     408,   408,   194,   200,     9,   413,   136,   200,     9,   413,
     200,   136,     9,   413,   194,   136,   197,     9,   413,   408,
      32,   194,   225,   195,   195,   210,   493,   493,   481,   399,
       4,   112,   117,   123,   125,   165,   166,   168,   197,   297,
     322,   323,   324,   329,   330,   331,   332,   431,   457,    38,
     345,   197,   196,   197,    54,   345,   345,   345,   357,    38,
      83,   172,    14,    83,   345,   193,   480,   194,   296,   194,
     286,   345,   288,   194,   296,   470,   296,   195,   196,   193,
     194,   419,   419,   194,   200,     9,   413,   136,   200,     9,
     413,   200,   136,   194,     9,   413,   296,    32,   225,   195,
     194,   194,   194,   232,   195,   195,   279,   225,   493,   493,
     136,   408,   408,   408,   408,   358,   408,   408,   408,   196,
     197,   483,   132,   133,   184,   211,   473,   493,   269,   399,
     112,   332,    31,   125,   138,   142,   163,   169,   306,   307,
     308,   309,   399,   167,   314,   315,   128,   193,   210,   316,
     317,   298,   243,   493,     9,   195,     9,   195,   195,   470,
     323,   194,   443,   293,   163,   390,   197,   197,    83,   172,
      14,    83,   345,   288,   117,   347,   480,   197,   480,   194,
     194,   197,   196,   197,   296,   286,   136,   419,   419,   419,
     419,   358,   197,   225,   230,   233,    32,   227,   273,   225,
     194,   408,   136,   136,   136,   225,   399,   399,   475,    14,
     211,     9,   195,   196,   473,   470,   309,   179,   196,     9,
     195,     3,     4,     5,     6,     7,    10,    11,    12,    13,
      27,    28,    29,    57,    71,    72,    73,    74,    75,    76,
      77,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   137,   138,   143,   144,   145,   146,   147,   159,
     160,   161,   171,   173,   174,   176,   183,   184,   186,   188,
     189,   210,   396,   397,     9,   195,   163,   167,   210,   317,
     318,   319,   195,    83,   328,   242,   299,   473,   473,    14,
     243,   197,   294,   295,   473,    14,    83,   345,   194,   193,
     480,   195,   196,   320,   347,   480,   293,   197,   194,   419,
     136,   136,    32,   227,   272,   273,   225,   408,   408,   408,
     197,   195,   195,   408,   399,   302,   493,   310,   311,   407,
     307,    14,    32,    51,   312,   315,     9,    36,   194,    31,
      50,    53,    14,     9,   195,   212,   474,   328,    14,   493,
     242,   195,    14,   345,    38,    83,   387,   196,   225,   480,
     320,   197,   480,   419,   419,   225,    99,   238,   197,   210,
     223,   303,   304,   305,     9,   413,     9,   413,   197,   408,
     397,   397,    59,   313,   318,   318,    31,    50,    53,   408,
      83,   179,   193,   195,   408,   475,   408,    83,     9,   414,
     225,   197,   196,   320,    97,   195,   115,   234,   158,   102,
     493,   180,   407,   170,    14,   482,   300,   193,    38,    83,
     194,   197,   225,   195,   193,   176,   241,   210,   323,   324,
     180,   408,   180,   284,   285,   432,   301,    83,   197,   399,
     239,   173,   210,   195,   194,     9,   414,   119,   120,   121,
     326,   327,   284,    83,   269,   195,   480,   432,   494,   194,
     194,   195,   195,   196,   321,   326,    38,    83,   172,   480,
     196,   225,   494,    83,   172,    14,    83,   321,   225,   197,
      38,    83,   172,    14,    83,   345,   197,    83,   172,    14,
      83,   345,    14,    83,   345,   345
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
#line 748 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 751 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 758 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 759 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 762 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 763 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 764 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 765 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 766 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 768 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 771 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 773 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 774 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 775 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 776 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 777 "hphp.y"
    { _p->onUse((yyvsp[(2) - (3)]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 780 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 783 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 786 "hphp.y"
    { _p->onGroupUse((yyvsp[(2) - (6)]).text(), (yyvsp[(4) - (6)]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 790 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 794 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 797 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 803 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 889 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 891 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 896 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 897 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 903 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 907 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 908 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 910 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 912 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 917 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 918 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(1) - (1)]),
                                           &Parser::useClass);;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useFunction);;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useConst);;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 937 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 939 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 942 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 944 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 945 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 950 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 957 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 965 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 968 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 981 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 984 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 988 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 993 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 994 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1000 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1009 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1017 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1018 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1019 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1020 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1021 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1022 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1023 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1024 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1025 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1026 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1027 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1028 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1029 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1030 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1031 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1035 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1037 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1042 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1044 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1048 "hphp.y"
    { _p->onDeclare((yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                         (yyval) = (yyvsp[(3) - (5)]);
                                         (yyval) = T_DECLARE;;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1057 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1058 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1061 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1062 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1063 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1064 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1068 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1069 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1070 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1071 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1072 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1073 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1074 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1075 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1076 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1077 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1078 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1079 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1087 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1088 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1097 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1098 "hphp.y"
    { (yyval).reset();;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1102 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1104 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1110 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1111 "hphp.y"
    { (yyval).reset();;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1115 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1116 "hphp.y"
    { (yyval).reset();;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1120 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1126 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1132 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1139 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1145 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1152 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1158 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1166 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1170 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1174 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1178 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1184 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1187 "hphp.y"
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

  case 202:

/* Line 1455 of yacc.c  */
#line 1202 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1205 "hphp.y"
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

  case 204:

/* Line 1455 of yacc.c  */
#line 1219 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1222 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1227 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1230 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1236 "hphp.y"
    { _p->onClassExpressionStart(); ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1239 "hphp.y"
    { _p->onClassExpression((yyval), (yyvsp[(3) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(5) - (8)]), (yyvsp[(7) - (8)])); ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1243 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1254 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1266 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1270 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1276 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1277 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1278 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1281 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1282 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1286 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1287 "hphp.y"
    { (yyval).reset();;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1290 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1291 "hphp.y"
    { (yyval).reset();;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1294 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1295 "hphp.y"
    { (yyval).reset();;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1298 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1300 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1303 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1305 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1309 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1310 "hphp.y"
    { (yyval).reset();;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1314 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1315 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1321 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1324 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1326 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1331 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1334 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1336 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (4)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1340 "hphp.y"
    {_p->onDeclareList((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1342 "hphp.y"
    {_p->onDeclareList((yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                           (yyval) = (yyvsp[(1) - (5)]);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1347 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1348 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1349 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1350 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1355 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1357 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1358 "hphp.y"
    { (yyval).reset();;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1361 "hphp.y"
    { (yyval).reset();;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1362 "hphp.y"
    { (yyval).reset();;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1367 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1368 "hphp.y"
    { (yyval).reset();;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1373 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1374 "hphp.y"
    { (yyval).reset();;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1377 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1378 "hphp.y"
    { (yyval).reset();;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1381 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1382 "hphp.y"
    { (yyval).reset();;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1390 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1396 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1402 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1406 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1410 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1415 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1420 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1423 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1429 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1433 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1438 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1443 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1448 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1453 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1459 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1465 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1473 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1478 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1483 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1490 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1498 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1501 "hphp.y"
    { (yyval).reset();;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1506 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1513 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1517 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1521 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1525 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1535 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { (yyval).reset();;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1545 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1546 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1557 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { (yyval).reset();;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1592 "hphp.y"
    { (yyval).reset();;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1600 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1637 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1644 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1651 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1658 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1673 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1695 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1751 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { (yyval).reset();;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
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
#line 1794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval).reset();;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval).reset();;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { (yyval).reset();;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1878 "hphp.y"
    { (yyval).reset();;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { _p->onYieldFrom((yyval),&(yyvsp[(2) - (2)]));;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PIPE);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1975 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1982 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1985 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1987 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1988 "hphp.y"
    { _p->onNullCoalesce((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1989 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1991 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1992 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1994 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1996 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1997 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1998 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2000 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2003 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2004 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2005 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2006 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2014 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2015 "hphp.y"
    { (yyval).reset();;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2020 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2026 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[(2) - (12)]),(yyvsp[(5) - (12)]),(yyvsp[(8) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(7) - (12)]),&(yyvsp[(9) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2034 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2040 "hphp.y"
    { _p->finishStatement((yyvsp[(12) - (13)]), (yyvsp[(12) - (13)])); (yyvsp[(12) - (13)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[(1) - (13)]),
                                           (yyvsp[(3) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(12) - (13)]),(yyvsp[(8) - (13)]),&(yyvsp[(10) - (13)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2050 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2058 "hphp.y"
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

  case 529:

/* Line 1455 of yacc.c  */
#line 2068 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2076 "hphp.y"
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

  case 531:

/* Line 1455 of yacc.c  */
#line 2086 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2092 "hphp.y"
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

  case 533:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2111 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2118 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2137 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2155 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2162 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2177 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2185 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2191 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { (yyval).reset();;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { (yyval).reset();;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { (yyval).reset();;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { _p->onKeyset((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { _p->onKeyset((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { _p->onKeyset((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval).reset();;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval).reset();;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval).reset();;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
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
#line 2346 "hphp.y"
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
#line 2357 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { (yyval).reset();;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { (yyval).reset();;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
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
#line 2388 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2498 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { (yyval).reset();;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2539 "hphp.y"
    { (yyval).reset();;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { (yyval).reset();;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { (yyval).reset();;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2557 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2565 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2577 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2579 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2580 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2583 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2588 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2596 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2599 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2603 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2608 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2610 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2613 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2634 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2654 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2662 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2663 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2664 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2665 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2669 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2675 "hphp.y"
    { (yyval).reset();;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { (yyval).reset();;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2680 "hphp.y"
    { (yyval).reset();;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2683 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2684 "hphp.y"
    { (yyval).reset();;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2690 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2692 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2700 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2701 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2709 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2710 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2716 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2726 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2727 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2728 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2729 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2731 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2732 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2737 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2742 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2743 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2748 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2749 "hphp.y"
    { (yyval).reset();;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2756 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2758 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2759 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2763 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2764 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2778 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    { (yyval).reset();;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2787 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2788 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2795 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2797 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2802 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2805 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2809 "hphp.y"
    { (yyval).reset();;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2819 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2820 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2824 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2835 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2849 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2850 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2854 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2856 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
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
#line 2869 "hphp.y"
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

  case 868:

/* Line 1455 of yacc.c  */
#line 2880 "hphp.y"
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

  case 869:

/* Line 1455 of yacc.c  */
#line 2895 "hphp.y"
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
#line 2907 "hphp.y"
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
#line 2919 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2922 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2926 "hphp.y"
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

  case 878:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2945 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2947 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2948 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2952 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2953 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2954 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
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

  case 887:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2975 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2985 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2987 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2989 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2990 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2994 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2998 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3002 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3003 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3013 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3020 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3037 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3040 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3048 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3052 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3053 "hphp.y"
    { _p->onPipeVariable((yyval));;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3054 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { (yyval).reset();;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3067 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3068 "hphp.y"
    { (yyval)++;;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3073 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3075 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3078 "hphp.y"
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

  case 923:

/* Line 1455 of yacc.c  */
#line 3089 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3090 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3094 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3095 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3098 "hphp.y"
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

  case 929:

/* Line 1455 of yacc.c  */
#line 3107 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3111 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3112 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3114 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3115 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3117 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3122 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3123 "hphp.y"
    { (yyval).reset();;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3128 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3129 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3130 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3133 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3135 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3136 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3142 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3143 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3147 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3148 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3149 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3150 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3155 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3156 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3161 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3163 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3165 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3166 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3170 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3172 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3173 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3175 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3180 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3182 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3184 "hphp.y"
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

  case 965:

/* Line 1455 of yacc.c  */
#line 3194 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3196 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3197 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3200 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3201 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3202 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3206 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3207 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3208 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3209 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3210 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3211 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3212 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3213 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3214 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3215 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3216 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3220 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3221 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3226 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3228 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3242 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3247 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3251 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3256 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3263 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 995:

/* Line 1455 of yacc.c  */
#line 3268 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 996:

/* Line 1455 of yacc.c  */
#line 3274 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3278 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3284 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3288 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3295 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3296 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3300 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3303 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1004:

/* Line 1455 of yacc.c  */
#line 3309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1005:

/* Line 1455 of yacc.c  */
#line 3314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1006:

/* Line 1455 of yacc.c  */
#line 3315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3316 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3317 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3321 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1010:

/* Line 1455 of yacc.c  */
#line 3322 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1; ;}
    break;

  case 1013:

/* Line 1455 of yacc.c  */
#line 3331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1016:

/* Line 1455 of yacc.c  */
#line 3342 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3344 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 1018:

/* Line 1455 of yacc.c  */
#line 3348 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 1019:

/* Line 1455 of yacc.c  */
#line 3351 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 1020:

/* Line 1455 of yacc.c  */
#line 3355 "hphp.y"
    {;}
    break;

  case 1021:

/* Line 1455 of yacc.c  */
#line 3356 "hphp.y"
    {;}
    break;

  case 1022:

/* Line 1455 of yacc.c  */
#line 3357 "hphp.y"
    {;}
    break;

  case 1023:

/* Line 1455 of yacc.c  */
#line 3363 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 1024:

/* Line 1455 of yacc.c  */
#line 3368 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 1025:

/* Line 1455 of yacc.c  */
#line 3379 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 1026:

/* Line 1455 of yacc.c  */
#line 3384 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1027:

/* Line 1455 of yacc.c  */
#line 3385 "hphp.y"
    { ;}
    break;

  case 1028:

/* Line 1455 of yacc.c  */
#line 3390 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 1029:

/* Line 1455 of yacc.c  */
#line 3391 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 1030:

/* Line 1455 of yacc.c  */
#line 3397 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1031:

/* Line 1455 of yacc.c  */
#line 3402 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1032:

/* Line 1455 of yacc.c  */
#line 3407 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1033:

/* Line 1455 of yacc.c  */
#line 3411 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1034:

/* Line 1455 of yacc.c  */
#line 3416 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 1035:

/* Line 1455 of yacc.c  */
#line 3418 "hphp.y"
    { _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)])); (yyval) = (yyvsp[(2) - (5)]);;}
    break;

  case 1036:

/* Line 1455 of yacc.c  */
#line 3424 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1037:

/* Line 1455 of yacc.c  */
#line 3427 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1038:

/* Line 1455 of yacc.c  */
#line 3430 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1039:

/* Line 1455 of yacc.c  */
#line 3431 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1040:

/* Line 1455 of yacc.c  */
#line 3434 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1041:

/* Line 1455 of yacc.c  */
#line 3437 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1042:

/* Line 1455 of yacc.c  */
#line 3440 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1043:

/* Line 1455 of yacc.c  */
#line 3443 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1044:

/* Line 1455 of yacc.c  */
#line 3445 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1045:

/* Line 1455 of yacc.c  */
#line 3451 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1046:

/* Line 1455 of yacc.c  */
#line 3457 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1047:

/* Line 1455 of yacc.c  */
#line 3465 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1048:

/* Line 1455 of yacc.c  */
#line 3466 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14574 "hphp.5.tab.cpp"
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
#line 3469 "hphp.y"

/* !PHP5_ONLY*/
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}

