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
#define YYLAST   18505

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  204
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  295
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1055
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1939

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
     237,   239,   242,   246,   250,   252,   255,   257,   260,   264,
     269,   273,   275,   278,   280,   283,   286,   288,   292,   294,
     298,   301,   304,   307,   313,   318,   321,   322,   324,   326,
     328,   330,   334,   340,   349,   350,   355,   356,   363,   364,
     375,   376,   381,   384,   388,   391,   395,   398,   402,   406,
     410,   414,   418,   422,   428,   430,   432,   434,   435,   445,
     446,   457,   463,   464,   478,   479,   485,   489,   493,   496,
     499,   502,   505,   508,   511,   515,   518,   521,   525,   528,
     531,   532,   537,   547,   548,   549,   554,   557,   558,   560,
     561,   563,   564,   574,   575,   586,   587,   599,   600,   610,
     611,   622,   623,   632,   633,   643,   644,   652,   653,   662,
     663,   672,   673,   681,   682,   691,   693,   695,   697,   699,
     701,   704,   708,   712,   715,   718,   719,   722,   723,   726,
     727,   729,   733,   735,   739,   742,   743,   745,   748,   753,
     755,   760,   762,   767,   769,   774,   776,   781,   785,   791,
     795,   800,   805,   811,   817,   822,   823,   825,   827,   832,
     833,   839,   840,   843,   844,   848,   849,   857,   866,   873,
     876,   882,   889,   894,   895,   900,   906,   914,   921,   928,
     936,   946,   955,   962,   970,   976,   979,   984,   990,   994,
     995,   999,  1004,  1011,  1017,  1023,  1030,  1039,  1047,  1050,
    1051,  1053,  1056,  1059,  1063,  1068,  1073,  1077,  1079,  1081,
    1084,  1089,  1093,  1099,  1101,  1105,  1108,  1109,  1112,  1116,
    1119,  1120,  1121,  1126,  1127,  1133,  1136,  1139,  1142,  1143,
    1155,  1156,  1169,  1173,  1177,  1181,  1186,  1191,  1195,  1201,
    1204,  1207,  1208,  1215,  1221,  1226,  1230,  1232,  1234,  1238,
    1243,  1245,  1248,  1250,  1252,  1258,  1265,  1267,  1269,  1274,
    1276,  1278,  1282,  1285,  1288,  1289,  1292,  1293,  1295,  1299,
    1301,  1303,  1305,  1307,  1311,  1316,  1321,  1326,  1328,  1330,
    1333,  1336,  1339,  1343,  1347,  1349,  1351,  1353,  1355,  1359,
    1361,  1365,  1367,  1369,  1371,  1372,  1374,  1377,  1379,  1381,
    1383,  1385,  1387,  1389,  1391,  1393,  1394,  1396,  1398,  1400,
    1404,  1410,  1412,  1416,  1422,  1427,  1431,  1435,  1439,  1444,
    1448,  1452,  1456,  1459,  1462,  1464,  1466,  1470,  1474,  1476,
    1478,  1479,  1481,  1484,  1489,  1493,  1497,  1504,  1507,  1511,
    1514,  1518,  1525,  1527,  1529,  1531,  1533,  1535,  1542,  1546,
    1551,  1558,  1562,  1566,  1570,  1574,  1578,  1582,  1586,  1590,
    1594,  1598,  1602,  1606,  1609,  1612,  1615,  1618,  1622,  1626,
    1630,  1634,  1638,  1642,  1646,  1650,  1654,  1658,  1662,  1666,
    1670,  1674,  1678,  1682,  1686,  1690,  1693,  1696,  1699,  1702,
    1706,  1710,  1714,  1718,  1722,  1726,  1730,  1734,  1738,  1742,
    1746,  1752,  1757,  1761,  1763,  1766,  1769,  1772,  1775,  1778,
    1781,  1784,  1787,  1790,  1792,  1794,  1796,  1798,  1800,  1802,
    1806,  1809,  1811,  1817,  1818,  1819,  1832,  1833,  1847,  1848,
    1853,  1854,  1862,  1863,  1869,  1870,  1874,  1875,  1882,  1885,
    1888,  1893,  1895,  1897,  1903,  1907,  1913,  1917,  1920,  1921,
    1924,  1925,  1930,  1935,  1939,  1942,  1943,  1949,  1953,  1960,
    1965,  1968,  1969,  1975,  1979,  1982,  1983,  1989,  1993,  1998,
    2003,  2008,  2013,  2018,  2023,  2028,  2033,  2038,  2041,  2042,
    2045,  2046,  2049,  2050,  2055,  2060,  2065,  2070,  2072,  2074,
    2076,  2078,  2080,  2082,  2084,  2088,  2090,  2094,  2099,  2101,
    2104,  2109,  2112,  2119,  2120,  2122,  2127,  2128,  2131,  2132,
    2134,  2136,  2140,  2142,  2146,  2148,  2150,  2154,  2158,  2160,
    2162,  2164,  2166,  2168,  2170,  2172,  2174,  2176,  2178,  2180,
    2182,  2184,  2186,  2188,  2190,  2192,  2194,  2196,  2198,  2200,
    2202,  2204,  2206,  2208,  2210,  2212,  2214,  2216,  2218,  2220,
    2222,  2224,  2226,  2228,  2230,  2232,  2234,  2236,  2238,  2240,
    2242,  2244,  2246,  2248,  2250,  2252,  2254,  2256,  2258,  2260,
    2262,  2264,  2266,  2268,  2270,  2272,  2274,  2276,  2278,  2280,
    2282,  2284,  2286,  2288,  2290,  2292,  2294,  2296,  2298,  2300,
    2302,  2304,  2306,  2308,  2310,  2312,  2314,  2316,  2318,  2320,
    2325,  2327,  2329,  2331,  2333,  2335,  2337,  2341,  2343,  2347,
    2349,  2351,  2355,  2357,  2359,  2361,  2364,  2366,  2367,  2368,
    2370,  2372,  2376,  2377,  2379,  2381,  2383,  2385,  2387,  2389,
    2391,  2393,  2395,  2397,  2399,  2401,  2403,  2407,  2410,  2412,
    2414,  2419,  2423,  2428,  2430,  2432,  2434,  2436,  2438,  2442,
    2446,  2450,  2454,  2458,  2462,  2466,  2470,  2474,  2478,  2482,
    2486,  2490,  2494,  2498,  2502,  2506,  2510,  2513,  2516,  2519,
    2522,  2526,  2530,  2534,  2538,  2542,  2546,  2550,  2554,  2558,
    2564,  2569,  2573,  2575,  2579,  2583,  2587,  2591,  2593,  2595,
    2597,  2599,  2603,  2607,  2611,  2614,  2615,  2617,  2618,  2620,
    2621,  2627,  2631,  2635,  2637,  2639,  2641,  2643,  2647,  2650,
    2652,  2654,  2656,  2658,  2660,  2664,  2666,  2668,  2670,  2673,
    2676,  2681,  2685,  2690,  2692,  2694,  2696,  2700,  2702,  2705,
    2706,  2712,  2716,  2720,  2722,  2726,  2728,  2731,  2732,  2738,
    2742,  2745,  2746,  2750,  2751,  2756,  2759,  2760,  2764,  2768,
    2770,  2771,  2773,  2775,  2777,  2779,  2783,  2785,  2787,  2789,
    2793,  2795,  2797,  2801,  2805,  2808,  2813,  2816,  2821,  2827,
    2833,  2839,  2845,  2847,  2849,  2851,  2853,  2855,  2857,  2861,
    2865,  2870,  2875,  2879,  2881,  2883,  2885,  2887,  2891,  2893,
    2898,  2902,  2904,  2906,  2908,  2910,  2912,  2916,  2920,  2925,
    2930,  2934,  2936,  2938,  2946,  2956,  2964,  2971,  2980,  2982,
    2985,  2990,  2995,  2997,  2999,  3001,  3006,  3008,  3009,  3011,
    3014,  3016,  3018,  3020,  3024,  3028,  3032,  3033,  3035,  3037,
    3041,  3045,  3048,  3052,  3059,  3060,  3062,  3067,  3070,  3071,
    3077,  3081,  3085,  3087,  3094,  3099,  3104,  3107,  3110,  3111,
    3117,  3121,  3125,  3127,  3130,  3131,  3137,  3141,  3145,  3147,
    3150,  3153,  3155,  3158,  3160,  3165,  3169,  3173,  3180,  3184,
    3186,  3188,  3190,  3195,  3200,  3205,  3210,  3215,  3220,  3223,
    3226,  3231,  3234,  3237,  3239,  3243,  3247,  3251,  3252,  3255,
    3261,  3268,  3275,  3283,  3285,  3288,  3290,  3293,  3295,  3300,
    3302,  3307,  3311,  3312,  3314,  3318,  3321,  3325,  3327,  3329,
    3330,  3331,  3335,  3337,  3341,  3345,  3348,  3349,  3352,  3355,
    3358,  3361,  3363,  3366,  3371,  3374,  3380,  3384,  3386,  3388,
    3389,  3393,  3398,  3404,  3408,  3410,  3413,  3414,  3419,  3421,
    3425,  3428,  3433,  3439,  3442,  3445,  3447,  3449,  3451,  3453,
    3457,  3460,  3462,  3471,  3478,  3480
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     205,     0,    -1,    -1,   206,   207,    -1,   207,   208,    -1,
      -1,   228,    -1,   245,    -1,   252,    -1,   249,    -1,   259,
      -1,   472,    -1,   129,   194,   195,   196,    -1,   159,   221,
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
      -1,   115,    -1,   126,    -1,   212,    -1,   130,    -1,   221,
     162,    -1,   162,   221,   162,    -1,   215,     9,   217,    -1,
     217,    -1,   215,   415,    -1,   221,    -1,   162,   221,    -1,
     221,   102,   211,    -1,   162,   221,   102,   211,    -1,   218,
       9,   220,    -1,   220,    -1,   218,   415,    -1,   217,    -1,
     111,   217,    -1,   112,   217,    -1,   211,    -1,   221,   162,
     211,    -1,   221,    -1,   159,   162,   221,    -1,   162,   221,
      -1,   222,   477,    -1,   222,   477,    -1,   225,     9,   473,
      14,   409,    -1,   112,   473,    14,   409,    -1,   226,   227,
      -1,    -1,   228,    -1,   245,    -1,   252,    -1,   259,    -1,
     197,   226,   198,    -1,    74,   335,   228,   281,   283,    -1,
      74,   335,    32,   226,   282,   284,    77,   196,    -1,    -1,
      94,   335,   229,   275,    -1,    -1,    93,   230,   228,    94,
     335,   196,    -1,    -1,    96,   194,   337,   196,   337,   196,
     337,   195,   231,   273,    -1,    -1,   104,   335,   232,   278,
      -1,   108,   196,    -1,   108,   346,   196,    -1,   110,   196,
      -1,   110,   346,   196,    -1,   113,   196,    -1,   113,   346,
     196,    -1,    27,   108,   196,    -1,   118,   291,   196,    -1,
     124,   293,   196,    -1,    92,   336,   196,    -1,   151,   336,
     196,    -1,   126,   194,   469,   195,   196,    -1,   196,    -1,
      86,    -1,    87,    -1,    -1,    98,   194,   346,   102,   272,
     271,   195,   233,   274,    -1,    -1,    98,   194,   346,    28,
     102,   272,   271,   195,   234,   274,    -1,   100,   194,   277,
     195,   276,    -1,    -1,   114,   237,   115,   194,   400,    83,
     195,   197,   226,   198,   239,   235,   242,    -1,    -1,   114,
     237,   176,   236,   240,    -1,   116,   346,   196,    -1,   109,
     211,   196,    -1,   346,   196,    -1,   338,   196,    -1,   339,
     196,    -1,   340,   196,    -1,   341,   196,    -1,   342,   196,
      -1,   113,   341,   196,    -1,   343,   196,    -1,   344,   196,
      -1,   113,   343,   196,    -1,   345,   196,    -1,   211,    32,
      -1,    -1,   197,   238,   226,   198,    -1,   239,   115,   194,
     400,    83,   195,   197,   226,   198,    -1,    -1,    -1,   197,
     241,   226,   198,    -1,   176,   240,    -1,    -1,    38,    -1,
      -1,   111,    -1,    -1,   244,   243,   476,   246,   194,   287,
     195,   484,   321,    -1,    -1,   325,   244,   243,   476,   247,
     194,   287,   195,   484,   321,    -1,    -1,   432,   324,   244,
     243,   476,   248,   194,   287,   195,   484,   321,    -1,    -1,
     169,   211,   250,    32,   497,   471,   197,   294,   198,    -1,
      -1,   432,   169,   211,   251,    32,   497,   471,   197,   294,
     198,    -1,    -1,   265,   262,   253,   266,   267,   197,   297,
     198,    -1,    -1,   432,   265,   262,   254,   266,   267,   197,
     297,   198,    -1,    -1,   131,   263,   255,   268,   197,   297,
     198,    -1,    -1,   432,   131,   263,   256,   268,   197,   297,
     198,    -1,    -1,   130,   258,   407,   266,   267,   197,   297,
     198,    -1,    -1,   171,   264,   260,   267,   197,   297,   198,
      -1,    -1,   432,   171,   264,   261,   267,   197,   297,   198,
      -1,   476,    -1,   163,    -1,   476,    -1,   476,    -1,   130,
      -1,   123,   130,    -1,   123,   122,   130,    -1,   122,   123,
     130,    -1,   122,   130,    -1,   132,   400,    -1,    -1,   133,
     269,    -1,    -1,   132,   269,    -1,    -1,   400,    -1,   269,
       9,   400,    -1,   400,    -1,   270,     9,   400,    -1,   136,
     272,    -1,    -1,   444,    -1,    38,   444,    -1,   137,   194,
     458,   195,    -1,   228,    -1,    32,   226,    97,   196,    -1,
     228,    -1,    32,   226,    99,   196,    -1,   228,    -1,    32,
     226,    95,   196,    -1,   228,    -1,    32,   226,   101,   196,
      -1,   211,    14,   409,    -1,   277,     9,   211,    14,   409,
      -1,   197,   279,   198,    -1,   197,   196,   279,   198,    -1,
      32,   279,   105,   196,    -1,    32,   196,   279,   105,   196,
      -1,   279,   106,   346,   280,   226,    -1,   279,   107,   280,
     226,    -1,    -1,    32,    -1,   196,    -1,   281,    75,   335,
     228,    -1,    -1,   282,    75,   335,    32,   226,    -1,    -1,
      76,   228,    -1,    -1,    76,    32,   226,    -1,    -1,   286,
       9,   433,   327,   498,   172,    83,    -1,   286,     9,   433,
     327,   498,    38,   172,    83,    -1,   286,     9,   433,   327,
     498,   172,    -1,   286,   415,    -1,   433,   327,   498,   172,
      83,    -1,   433,   327,   498,    38,   172,    83,    -1,   433,
     327,   498,   172,    -1,    -1,   433,   327,   498,    83,    -1,
     433,   327,   498,    38,    83,    -1,   433,   327,   498,    38,
      83,    14,   346,    -1,   433,   327,   498,    83,    14,   346,
      -1,   286,     9,   433,   327,   498,    83,    -1,   286,     9,
     433,   327,   498,    38,    83,    -1,   286,     9,   433,   327,
     498,    38,    83,    14,   346,    -1,   286,     9,   433,   327,
     498,    83,    14,   346,    -1,   288,     9,   433,   498,   172,
      83,    -1,   288,     9,   433,   498,    38,   172,    83,    -1,
     288,     9,   433,   498,   172,    -1,   288,   415,    -1,   433,
     498,   172,    83,    -1,   433,   498,    38,   172,    83,    -1,
     433,   498,   172,    -1,    -1,   433,   498,    83,    -1,   433,
     498,    38,    83,    -1,   433,   498,    38,    83,    14,   346,
      -1,   433,   498,    83,    14,   346,    -1,   288,     9,   433,
     498,    83,    -1,   288,     9,   433,   498,    38,    83,    -1,
     288,     9,   433,   498,    38,    83,    14,   346,    -1,   288,
       9,   433,   498,    83,    14,   346,    -1,   290,   415,    -1,
      -1,   346,    -1,    38,   444,    -1,   172,   346,    -1,   290,
       9,   346,    -1,   290,     9,   172,   346,    -1,   290,     9,
      38,   444,    -1,   291,     9,   292,    -1,   292,    -1,    83,
      -1,   199,   444,    -1,   199,   197,   346,   198,    -1,   293,
       9,    83,    -1,   293,     9,    83,    14,   409,    -1,    83,
      -1,    83,    14,   409,    -1,   294,   295,    -1,    -1,   296,
     196,    -1,   474,    14,   409,    -1,   297,   298,    -1,    -1,
      -1,   323,   299,   329,   196,    -1,    -1,   325,   497,   300,
     329,   196,    -1,   330,   196,    -1,   331,   196,    -1,   332,
     196,    -1,    -1,   324,   244,   243,   475,   194,   301,   285,
     195,   484,   481,   322,    -1,    -1,   432,   324,   244,   243,
     476,   194,   302,   285,   195,   484,   481,   322,    -1,   165,
     307,   196,    -1,   166,   315,   196,    -1,   168,   317,   196,
      -1,     4,   132,   400,   196,    -1,     4,   133,   400,   196,
      -1,   117,   270,   196,    -1,   117,   270,   197,   303,   198,
      -1,   303,   304,    -1,   303,   305,    -1,    -1,   224,   158,
     211,   173,   270,   196,    -1,   306,   102,   324,   211,   196,
      -1,   306,   102,   325,   196,    -1,   224,   158,   211,    -1,
     211,    -1,   308,    -1,   307,     9,   308,    -1,   309,   397,
     313,   314,    -1,   163,    -1,    31,   310,    -1,   310,    -1,
     138,    -1,   138,   179,   497,   414,   180,    -1,   138,   179,
     497,     9,   497,   180,    -1,   400,    -1,   125,    -1,   169,
     197,   312,   198,    -1,   142,    -1,   408,    -1,   311,     9,
     408,    -1,   311,   414,    -1,    14,   409,    -1,    -1,    59,
     170,    -1,    -1,   316,    -1,   315,     9,   316,    -1,   167,
      -1,   318,    -1,   211,    -1,   128,    -1,   194,   319,   195,
      -1,   194,   319,   195,    53,    -1,   194,   319,   195,    31,
      -1,   194,   319,   195,    50,    -1,   318,    -1,   320,    -1,
     320,    53,    -1,   320,    31,    -1,   320,    50,    -1,   319,
       9,   319,    -1,   319,    36,   319,    -1,   211,    -1,   163,
      -1,   167,    -1,   196,    -1,   197,   226,   198,    -1,   196,
      -1,   197,   226,   198,    -1,   325,    -1,   125,    -1,   325,
      -1,    -1,   326,    -1,   325,   326,    -1,   119,    -1,   120,
      -1,   121,    -1,   124,    -1,   123,    -1,   122,    -1,   189,
      -1,   328,    -1,    -1,   119,    -1,   120,    -1,   121,    -1,
     329,     9,    83,    -1,   329,     9,    83,    14,   409,    -1,
      83,    -1,    83,    14,   409,    -1,   330,     9,   474,    14,
     409,    -1,   112,   474,    14,   409,    -1,   331,     9,   474,
      -1,   123,   112,   474,    -1,   123,   333,   471,    -1,   333,
     471,    14,   497,    -1,   112,   184,   476,    -1,   194,   334,
     195,    -1,    72,   404,   407,    -1,    72,   257,    -1,    71,
     346,    -1,   389,    -1,   384,    -1,   194,   346,   195,    -1,
     336,     9,   346,    -1,   346,    -1,   336,    -1,    -1,    27,
      -1,    27,   346,    -1,    27,   346,   136,   346,    -1,   194,
     338,   195,    -1,   444,    14,   338,    -1,   137,   194,   458,
     195,    14,   338,    -1,    29,   346,    -1,   444,    14,   341,
      -1,    28,   346,    -1,   444,    14,   343,    -1,   137,   194,
     458,   195,    14,   343,    -1,   347,    -1,   444,    -1,   334,
      -1,   448,    -1,   447,    -1,   137,   194,   458,   195,    14,
     346,    -1,   444,    14,   346,    -1,   444,    14,    38,   444,
      -1,   444,    14,    38,    72,   404,   407,    -1,   444,    26,
     346,    -1,   444,    25,   346,    -1,   444,    24,   346,    -1,
     444,    23,   346,    -1,   444,    22,   346,    -1,   444,    21,
     346,    -1,   444,    20,   346,    -1,   444,    19,   346,    -1,
     444,    18,   346,    -1,   444,    17,   346,    -1,   444,    16,
     346,    -1,   444,    15,   346,    -1,   444,    68,    -1,    68,
     444,    -1,   444,    67,    -1,    67,   444,    -1,   346,    34,
     346,    -1,   346,    35,   346,    -1,   346,    10,   346,    -1,
     346,    12,   346,    -1,   346,    11,   346,    -1,   346,    36,
     346,    -1,   346,    38,   346,    -1,   346,    37,   346,    -1,
     346,    52,   346,    -1,   346,    50,   346,    -1,   346,    51,
     346,    -1,   346,    53,   346,    -1,   346,    54,   346,    -1,
     346,    69,   346,    -1,   346,    55,   346,    -1,   346,    30,
     346,    -1,   346,    49,   346,    -1,   346,    48,   346,    -1,
      50,   346,    -1,    51,   346,    -1,    56,   346,    -1,    58,
     346,    -1,   346,    40,   346,    -1,   346,    39,   346,    -1,
     346,    42,   346,    -1,   346,    41,   346,    -1,   346,    43,
     346,    -1,   346,    47,   346,    -1,   346,    44,   346,    -1,
     346,    46,   346,    -1,   346,    45,   346,    -1,   346,    57,
     404,    -1,   194,   347,   195,    -1,   346,    31,   346,    32,
     346,    -1,   346,    31,    32,   346,    -1,   346,    33,   346,
      -1,   468,    -1,    66,   346,    -1,    65,   346,    -1,    64,
     346,    -1,    63,   346,    -1,    62,   346,    -1,    61,   346,
      -1,    60,   346,    -1,    73,   405,    -1,    59,   346,    -1,
     412,    -1,   365,    -1,   372,    -1,   375,    -1,   378,    -1,
     364,    -1,   200,   406,   200,    -1,    13,   346,    -1,   386,
      -1,   117,   194,   388,   415,   195,    -1,    -1,    -1,   244,
     243,   194,   350,   287,   195,   484,   348,   484,   197,   226,
     198,    -1,    -1,   325,   244,   243,   194,   351,   287,   195,
     484,   348,   484,   197,   226,   198,    -1,    -1,   189,    83,
     353,   358,    -1,    -1,   189,   190,   354,   287,   191,   484,
     358,    -1,    -1,   189,   197,   355,   226,   198,    -1,    -1,
      83,   356,   358,    -1,    -1,   190,   357,   287,   191,   484,
     358,    -1,     8,   346,    -1,     8,   343,    -1,     8,   197,
     226,   198,    -1,    91,    -1,   470,    -1,   360,     9,   359,
     136,   346,    -1,   359,   136,   346,    -1,   361,     9,   359,
     136,   409,    -1,   359,   136,   409,    -1,   360,   414,    -1,
      -1,   361,   414,    -1,    -1,   183,   194,   362,   195,    -1,
     138,   194,   459,   195,    -1,    70,   459,   201,    -1,   367,
     414,    -1,    -1,   367,     9,   346,   136,   346,    -1,   346,
     136,   346,    -1,   367,     9,   346,   136,    38,   444,    -1,
     346,   136,    38,   444,    -1,   369,   414,    -1,    -1,   369,
       9,   409,   136,   409,    -1,   409,   136,   409,    -1,   371,
     414,    -1,    -1,   371,     9,   420,   136,   420,    -1,   420,
     136,   420,    -1,   139,    70,   366,   201,    -1,   139,    70,
     368,   201,    -1,   139,    70,   370,   201,    -1,   140,    70,
     381,   201,    -1,   140,    70,   382,   201,    -1,   140,    70,
     383,   201,    -1,   141,    70,   381,   201,    -1,   141,    70,
     382,   201,    -1,   141,    70,   383,   201,    -1,   336,   414,
      -1,    -1,   410,   414,    -1,    -1,   421,   414,    -1,    -1,
     400,   197,   461,   198,    -1,   400,   197,   463,   198,    -1,
     386,    70,   454,   201,    -1,   387,    70,   454,   201,    -1,
     365,    -1,   372,    -1,   375,    -1,   378,    -1,   470,    -1,
     447,    -1,    91,    -1,   194,   347,   195,    -1,    81,    -1,
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
      -1,   408,    -1,   124,    -1,   452,    -1,   194,   347,   195,
      -1,   401,    -1,   402,   158,   451,    -1,   401,    -1,   450,
      -1,   403,   158,   451,    -1,   400,    -1,   124,    -1,   456,
      -1,   194,   195,    -1,   335,    -1,    -1,    -1,    90,    -1,
     465,    -1,   194,   289,   195,    -1,    -1,    78,    -1,    79,
      -1,    80,    -1,    91,    -1,   146,    -1,   147,    -1,   161,
      -1,   143,    -1,   174,    -1,   144,    -1,   145,    -1,   160,
      -1,   188,    -1,   154,    90,   155,    -1,   154,   155,    -1,
     408,    -1,   222,    -1,   138,   194,   413,   195,    -1,    70,
     413,   201,    -1,   183,   194,   363,   195,    -1,   373,    -1,
     376,    -1,   379,    -1,   411,    -1,   385,    -1,   194,   409,
     195,    -1,   409,    34,   409,    -1,   409,    35,   409,    -1,
     409,    10,   409,    -1,   409,    12,   409,    -1,   409,    11,
     409,    -1,   409,    36,   409,    -1,   409,    38,   409,    -1,
     409,    37,   409,    -1,   409,    52,   409,    -1,   409,    50,
     409,    -1,   409,    51,   409,    -1,   409,    53,   409,    -1,
     409,    54,   409,    -1,   409,    55,   409,    -1,   409,    49,
     409,    -1,   409,    48,   409,    -1,   409,    69,   409,    -1,
      56,   409,    -1,    58,   409,    -1,    50,   409,    -1,    51,
     409,    -1,   409,    40,   409,    -1,   409,    39,   409,    -1,
     409,    42,   409,    -1,   409,    41,   409,    -1,   409,    43,
     409,    -1,   409,    47,   409,    -1,   409,    44,   409,    -1,
     409,    46,   409,    -1,   409,    45,   409,    -1,   409,    31,
     409,    32,   409,    -1,   409,    31,    32,   409,    -1,   410,
       9,   409,    -1,   409,    -1,   224,   158,   212,    -1,   163,
     158,   212,    -1,   163,   158,   130,    -1,   224,   158,   130,
      -1,   222,    -1,    82,    -1,   470,    -1,   408,    -1,   202,
     465,   202,    -1,   203,   465,   203,    -1,   154,   465,   155,
      -1,   416,   414,    -1,    -1,     9,    -1,    -1,     9,    -1,
      -1,   416,     9,   409,   136,   409,    -1,   416,     9,   409,
      -1,   409,   136,   409,    -1,   409,    -1,    78,    -1,    79,
      -1,    80,    -1,   154,    90,   155,    -1,   154,   155,    -1,
      78,    -1,    79,    -1,    80,    -1,   211,    -1,    91,    -1,
      91,    52,   419,    -1,   417,    -1,   419,    -1,   211,    -1,
      50,   418,    -1,    51,   418,    -1,   138,   194,   422,   195,
      -1,    70,   422,   201,    -1,   183,   194,   425,   195,    -1,
     374,    -1,   377,    -1,   380,    -1,   421,     9,   420,    -1,
     420,    -1,   423,   414,    -1,    -1,   423,     9,   420,   136,
     420,    -1,   423,     9,   420,    -1,   420,   136,   420,    -1,
     420,    -1,   424,     9,   420,    -1,   420,    -1,   426,   414,
      -1,    -1,   426,     9,   359,   136,   420,    -1,   359,   136,
     420,    -1,   424,   414,    -1,    -1,   194,   427,   195,    -1,
      -1,   429,     9,   211,   428,    -1,   211,   428,    -1,    -1,
     431,   429,   414,    -1,    49,   430,    48,    -1,   432,    -1,
      -1,   134,    -1,   135,    -1,   211,    -1,   163,    -1,   197,
     346,   198,    -1,   435,    -1,   451,    -1,   211,    -1,   197,
     346,   198,    -1,   437,    -1,   451,    -1,    70,   454,   201,
      -1,   197,   346,   198,    -1,   445,   439,    -1,   194,   334,
     195,   439,    -1,   457,   439,    -1,   194,   334,   195,   439,
      -1,   194,   334,   195,   434,   436,    -1,   194,   347,   195,
     434,   436,    -1,   194,   334,   195,   434,   435,    -1,   194,
     347,   195,   434,   435,    -1,   451,    -1,   399,    -1,   449,
      -1,   450,    -1,   440,    -1,   442,    -1,   444,   434,   436,
      -1,   403,   158,   451,    -1,   446,   194,   289,   195,    -1,
     447,   194,   289,   195,    -1,   194,   444,   195,    -1,   399,
      -1,   449,    -1,   450,    -1,   440,    -1,   444,   434,   435,
      -1,   443,    -1,   446,   194,   289,   195,    -1,   194,   444,
     195,    -1,   451,    -1,   440,    -1,   399,    -1,   365,    -1,
     408,    -1,   194,   444,   195,    -1,   194,   347,   195,    -1,
     447,   194,   289,   195,    -1,   446,   194,   289,   195,    -1,
     194,   448,   195,    -1,   349,    -1,   352,    -1,   444,   434,
     438,   477,   194,   289,   195,    -1,   194,   334,   195,   434,
     438,   477,   194,   289,   195,    -1,   403,   158,   213,   477,
     194,   289,   195,    -1,   403,   158,   451,   194,   289,   195,
      -1,   403,   158,   197,   346,   198,   194,   289,   195,    -1,
     452,    -1,   455,   452,    -1,   452,    70,   454,   201,    -1,
     452,   197,   346,   198,    -1,   453,    -1,    83,    -1,    84,
      -1,   199,   197,   346,   198,    -1,   346,    -1,    -1,   199,
      -1,   455,   199,    -1,   451,    -1,   441,    -1,   442,    -1,
     456,   434,   436,    -1,   402,   158,   451,    -1,   194,   444,
     195,    -1,    -1,   441,    -1,   443,    -1,   456,   434,   435,
      -1,   194,   444,   195,    -1,   458,     9,    -1,   458,     9,
     444,    -1,   458,     9,   137,   194,   458,   195,    -1,    -1,
     444,    -1,   137,   194,   458,   195,    -1,   460,   414,    -1,
      -1,   460,     9,   346,   136,   346,    -1,   460,     9,   346,
      -1,   346,   136,   346,    -1,   346,    -1,   460,     9,   346,
     136,    38,   444,    -1,   460,     9,    38,   444,    -1,   346,
     136,    38,   444,    -1,    38,   444,    -1,   462,   414,    -1,
      -1,   462,     9,   346,   136,   346,    -1,   462,     9,   346,
      -1,   346,   136,   346,    -1,   346,    -1,   464,   414,    -1,
      -1,   464,     9,   409,   136,   409,    -1,   464,     9,   409,
      -1,   409,   136,   409,    -1,   409,    -1,   465,   466,    -1,
     465,    90,    -1,   466,    -1,    90,   466,    -1,    83,    -1,
      83,    70,   467,   201,    -1,    83,   434,   211,    -1,   156,
     346,   198,    -1,   156,    82,    70,   346,   201,   198,    -1,
     157,   444,   198,    -1,   211,    -1,    85,    -1,    83,    -1,
     127,   194,   336,   195,    -1,   128,   194,   444,   195,    -1,
     128,   194,   347,   195,    -1,   128,   194,   448,   195,    -1,
     128,   194,   447,   195,    -1,   128,   194,   334,   195,    -1,
       7,   346,    -1,     6,   346,    -1,     5,   194,   346,   195,
      -1,     4,   346,    -1,     3,   346,    -1,   444,    -1,   469,
       9,   444,    -1,   403,   158,   212,    -1,   403,   158,   130,
      -1,    -1,   102,   497,    -1,   184,   476,    14,   497,   196,
      -1,   432,   184,   476,    14,   497,   196,    -1,   186,   476,
     471,    14,   497,   196,    -1,   432,   186,   476,   471,    14,
     497,   196,    -1,   213,    -1,   497,   213,    -1,   212,    -1,
     497,   212,    -1,   213,    -1,   213,   179,   486,   180,    -1,
     211,    -1,   211,   179,   486,   180,    -1,   179,   479,   180,
      -1,    -1,   497,    -1,   478,     9,   497,    -1,   478,   414,
      -1,   478,     9,   172,    -1,   479,    -1,   172,    -1,    -1,
      -1,   193,   482,   415,    -1,   483,    -1,   482,     9,   483,
      -1,   497,    14,   497,    -1,   497,   485,    -1,    -1,    32,
     497,    -1,   102,   497,    -1,   103,   497,    -1,   488,   414,
      -1,   485,    -1,   487,   485,    -1,   488,     9,   489,   211,
      -1,   489,   211,    -1,   488,     9,   489,   211,   487,    -1,
     489,   211,   487,    -1,    50,    -1,    51,    -1,    -1,    91,
     136,   497,    -1,    31,    91,   136,   497,    -1,   224,   158,
     211,   136,   497,    -1,   491,     9,   490,    -1,   490,    -1,
     491,   414,    -1,    -1,   183,   194,   492,   195,    -1,   224,
      -1,   211,   158,   495,    -1,   211,   477,    -1,   179,   497,
     414,   180,    -1,   179,   497,     9,   497,   180,    -1,    31,
     497,    -1,    59,   497,    -1,   224,    -1,   138,    -1,   142,
      -1,   493,    -1,   494,   158,   495,    -1,   138,   496,    -1,
     163,    -1,   194,   111,   194,   480,   195,    32,   497,   195,
      -1,   194,   497,     9,   478,   414,   195,    -1,   497,    -1,
      -1
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
     871,   872,   873,   874,   875,   876,   877,   878,   882,   886,
     887,   891,   892,   897,   899,   904,   909,   910,   911,   913,
     918,   920,   925,   930,   932,   934,   939,   940,   944,   945,
     947,   951,   958,   965,   969,   975,   977,   980,   981,   982,
     983,   986,   987,   991,   996,   996,  1002,  1002,  1009,  1008,
    1014,  1014,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,  1031,  1032,  1033,  1037,  1035,  1044,
    1042,  1049,  1059,  1053,  1063,  1061,  1065,  1066,  1070,  1071,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1089,  1089,  1094,  1100,  1104,  1104,  1112,  1113,  1117,  1118,
    1122,  1128,  1126,  1141,  1138,  1154,  1151,  1168,  1167,  1176,
    1174,  1186,  1185,  1204,  1202,  1221,  1220,  1229,  1227,  1238,
    1238,  1245,  1244,  1256,  1254,  1267,  1268,  1272,  1275,  1278,
    1279,  1280,  1283,  1284,  1287,  1289,  1292,  1293,  1296,  1297,
    1300,  1301,  1305,  1306,  1311,  1312,  1315,  1316,  1317,  1321,
    1322,  1326,  1327,  1331,  1332,  1336,  1337,  1342,  1343,  1349,
    1350,  1351,  1352,  1355,  1358,  1360,  1363,  1364,  1368,  1370,
    1373,  1376,  1379,  1380,  1383,  1384,  1388,  1394,  1400,  1407,
    1409,  1414,  1419,  1425,  1429,  1433,  1437,  1442,  1447,  1452,
    1457,  1463,  1472,  1477,  1482,  1488,  1490,  1494,  1498,  1503,
    1507,  1510,  1513,  1517,  1521,  1525,  1529,  1534,  1542,  1544,
    1547,  1548,  1549,  1550,  1552,  1554,  1559,  1560,  1563,  1564,
    1565,  1569,  1570,  1572,  1573,  1577,  1579,  1582,  1586,  1592,
    1594,  1597,  1597,  1601,  1600,  1604,  1606,  1609,  1612,  1610,
    1627,  1623,  1638,  1640,  1642,  1644,  1646,  1648,  1650,  1654,
    1655,  1656,  1659,  1665,  1669,  1675,  1678,  1683,  1685,  1690,
    1695,  1699,  1700,  1704,  1705,  1707,  1709,  1715,  1716,  1718,
    1722,  1723,  1728,  1732,  1733,  1737,  1738,  1742,  1744,  1750,
    1755,  1756,  1758,  1762,  1763,  1764,  1765,  1769,  1770,  1771,
    1772,  1773,  1774,  1776,  1781,  1784,  1785,  1789,  1790,  1794,
    1795,  1798,  1799,  1802,  1803,  1806,  1807,  1811,  1812,  1813,
    1814,  1815,  1816,  1817,  1821,  1822,  1825,  1826,  1827,  1830,
    1832,  1834,  1835,  1838,  1840,  1844,  1846,  1850,  1854,  1858,
    1863,  1864,  1866,  1867,  1868,  1869,  1872,  1876,  1877,  1881,
    1882,  1886,  1887,  1888,  1889,  1893,  1897,  1902,  1906,  1910,
    1914,  1918,  1923,  1924,  1925,  1926,  1927,  1931,  1933,  1934,
    1935,  1938,  1939,  1940,  1941,  1942,  1943,  1944,  1945,  1946,
    1947,  1948,  1949,  1950,  1951,  1952,  1953,  1954,  1955,  1956,
    1957,  1958,  1959,  1960,  1961,  1962,  1963,  1964,  1965,  1966,
    1967,  1968,  1969,  1970,  1971,  1972,  1973,  1974,  1975,  1976,
    1977,  1978,  1979,  1980,  1981,  1983,  1984,  1986,  1987,  1989,
    1990,  1991,  1992,  1993,  1994,  1995,  1996,  1997,  1998,  1999,
    2000,  2001,  2002,  2003,  2004,  2005,  2006,  2007,  2008,  2009,
    2010,  2011,  2015,  2019,  2024,  2023,  2038,  2036,  2054,  2053,
    2072,  2071,  2090,  2089,  2107,  2107,  2122,  2122,  2140,  2141,
    2142,  2147,  2149,  2153,  2157,  2163,  2167,  2173,  2175,  2179,
    2181,  2185,  2189,  2190,  2194,  2196,  2200,  2202,  2203,  2206,
    2210,  2212,  2216,  2219,  2224,  2226,  2230,  2233,  2238,  2242,
    2246,  2250,  2254,  2258,  2262,  2266,  2270,  2274,  2276,  2280,
    2282,  2286,  2288,  2292,  2299,  2306,  2308,  2313,  2314,  2315,
    2316,  2317,  2318,  2319,  2321,  2322,  2326,  2327,  2328,  2329,
    2333,  2339,  2348,  2361,  2362,  2365,  2368,  2371,  2372,  2375,
    2379,  2382,  2385,  2392,  2393,  2397,  2398,  2400,  2405,  2406,
    2407,  2408,  2409,  2410,  2411,  2412,  2413,  2414,  2415,  2416,
    2417,  2418,  2419,  2420,  2421,  2422,  2423,  2424,  2425,  2426,
    2427,  2428,  2429,  2430,  2431,  2432,  2433,  2434,  2435,  2436,
    2437,  2438,  2439,  2440,  2441,  2442,  2443,  2444,  2445,  2446,
    2447,  2448,  2449,  2450,  2451,  2452,  2453,  2454,  2455,  2456,
    2457,  2458,  2459,  2460,  2461,  2462,  2463,  2464,  2465,  2466,
    2467,  2468,  2469,  2470,  2471,  2472,  2473,  2474,  2475,  2476,
    2477,  2478,  2479,  2480,  2481,  2482,  2483,  2484,  2485,  2489,
    2494,  2495,  2499,  2500,  2501,  2502,  2504,  2508,  2509,  2520,
    2521,  2523,  2535,  2536,  2537,  2541,  2542,  2543,  2547,  2548,
    2549,  2552,  2554,  2558,  2559,  2560,  2561,  2563,  2564,  2565,
    2566,  2567,  2568,  2569,  2570,  2571,  2572,  2575,  2580,  2581,
    2582,  2584,  2585,  2587,  2588,  2589,  2590,  2591,  2592,  2593,
    2595,  2597,  2599,  2601,  2603,  2604,  2605,  2606,  2607,  2608,
    2609,  2610,  2611,  2612,  2613,  2614,  2615,  2616,  2617,  2618,
    2619,  2621,  2623,  2625,  2627,  2628,  2631,  2632,  2636,  2640,
    2642,  2646,  2647,  2651,  2654,  2657,  2660,  2666,  2667,  2668,
    2669,  2670,  2671,  2672,  2677,  2679,  2683,  2684,  2687,  2688,
    2692,  2695,  2697,  2699,  2703,  2704,  2705,  2706,  2709,  2713,
    2714,  2715,  2716,  2720,  2722,  2729,  2730,  2731,  2732,  2733,
    2734,  2736,  2737,  2739,  2740,  2741,  2745,  2747,  2751,  2753,
    2756,  2759,  2761,  2763,  2766,  2768,  2772,  2774,  2777,  2780,
    2786,  2788,  2791,  2792,  2797,  2800,  2804,  2804,  2809,  2812,
    2813,  2817,  2818,  2822,  2823,  2824,  2828,  2830,  2838,  2839,
    2843,  2845,  2853,  2854,  2858,  2859,  2864,  2866,  2871,  2882,
    2896,  2908,  2923,  2924,  2925,  2926,  2927,  2928,  2929,  2939,
    2948,  2950,  2952,  2956,  2957,  2958,  2959,  2960,  2976,  2977,
    2979,  2988,  2989,  2990,  2991,  2992,  2993,  2994,  2995,  2997,
    3002,  3006,  3007,  3011,  3014,  3021,  3025,  3034,  3041,  3043,
    3049,  3051,  3052,  3056,  3057,  3058,  3065,  3066,  3071,  3072,
    3077,  3078,  3079,  3080,  3091,  3094,  3097,  3098,  3099,  3100,
    3111,  3115,  3116,  3117,  3119,  3120,  3121,  3125,  3127,  3130,
    3132,  3133,  3134,  3135,  3138,  3140,  3141,  3145,  3147,  3150,
    3152,  3153,  3154,  3158,  3160,  3163,  3166,  3168,  3170,  3174,
    3175,  3177,  3178,  3184,  3185,  3187,  3197,  3199,  3201,  3204,
    3205,  3206,  3210,  3211,  3212,  3213,  3214,  3215,  3216,  3217,
    3218,  3219,  3220,  3224,  3225,  3229,  3231,  3239,  3241,  3245,
    3249,  3254,  3258,  3266,  3267,  3271,  3272,  3278,  3279,  3288,
    3289,  3297,  3300,  3304,  3307,  3312,  3317,  3319,  3320,  3321,
    3324,  3326,  3332,  3333,  3337,  3338,  3342,  3343,  3347,  3348,
    3351,  3356,  3357,  3361,  3364,  3366,  3370,  3376,  3377,  3378,
    3382,  3386,  3396,  3404,  3406,  3410,  3412,  3417,  3423,  3426,
    3431,  3436,  3438,  3445,  3448,  3451,  3452,  3455,  3458,  3459,
    3464,  3466,  3470,  3476,  3486,  3487
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
     212,   212,   212,   212,   212,   212,   212,   212,   212,   213,
     213,   214,   214,   215,   215,   216,   217,   217,   217,   217,
     218,   218,   219,   220,   220,   220,   221,   221,   222,   222,
     222,   223,   224,   225,   225,   226,   226,   227,   227,   227,
     227,   228,   228,   228,   229,   228,   230,   228,   231,   228,
     232,   228,   228,   228,   228,   228,   228,   228,   228,   228,
     228,   228,   228,   228,   228,   228,   228,   233,   228,   234,
     228,   228,   235,   228,   236,   228,   228,   228,   228,   228,
     228,   228,   228,   228,   228,   228,   228,   228,   228,   228,
     238,   237,   239,   239,   241,   240,   242,   242,   243,   243,
     244,   246,   245,   247,   245,   248,   245,   250,   249,   251,
     249,   253,   252,   254,   252,   255,   252,   256,   252,   258,
     257,   260,   259,   261,   259,   262,   262,   263,   264,   265,
     265,   265,   265,   265,   266,   266,   267,   267,   268,   268,
     269,   269,   270,   270,   271,   271,   272,   272,   272,   273,
     273,   274,   274,   275,   275,   276,   276,   277,   277,   278,
     278,   278,   278,   279,   279,   279,   280,   280,   281,   281,
     282,   282,   283,   283,   284,   284,   285,   285,   285,   285,
     285,   285,   285,   285,   286,   286,   286,   286,   286,   286,
     286,   286,   287,   287,   287,   287,   287,   287,   287,   287,
     288,   288,   288,   288,   288,   288,   288,   288,   289,   289,
     290,   290,   290,   290,   290,   290,   291,   291,   292,   292,
     292,   293,   293,   293,   293,   294,   294,   295,   296,   297,
     297,   299,   298,   300,   298,   298,   298,   298,   301,   298,
     302,   298,   298,   298,   298,   298,   298,   298,   298,   303,
     303,   303,   304,   305,   305,   306,   306,   307,   307,   308,
     308,   309,   309,   310,   310,   310,   310,   310,   310,   310,
     311,   311,   312,   313,   313,   314,   314,   315,   315,   316,
     317,   317,   317,   318,   318,   318,   318,   319,   319,   319,
     319,   319,   319,   319,   320,   320,   320,   321,   321,   322,
     322,   323,   323,   324,   324,   325,   325,   326,   326,   326,
     326,   326,   326,   326,   327,   327,   328,   328,   328,   329,
     329,   329,   329,   330,   330,   331,   331,   332,   332,   333,
     334,   334,   334,   334,   334,   334,   335,   336,   336,   337,
     337,   338,   338,   338,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   346,   346,   346,   346,   347,   347,   347,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   348,   348,   350,   349,   351,   349,   353,   352,
     354,   352,   355,   352,   356,   352,   357,   352,   358,   358,
     358,   359,   359,   360,   360,   361,   361,   362,   362,   363,
     363,   364,   365,   365,   366,   366,   367,   367,   367,   367,
     368,   368,   369,   369,   370,   370,   371,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   381,   382,
     382,   383,   383,   384,   385,   386,   386,   387,   387,   387,
     387,   387,   387,   387,   387,   387,   388,   388,   388,   388,
     389,   390,   390,   391,   391,   392,   392,   393,   393,   394,
     395,   395,   396,   396,   396,   397,   397,   397,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   399,
     400,   400,   401,   401,   401,   401,   401,   402,   402,   403,
     403,   403,   404,   404,   404,   405,   405,   405,   406,   406,
     406,   407,   407,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   410,   410,   411,   411,   411,   411,   412,   412,   412,
     412,   412,   412,   412,   413,   413,   414,   414,   415,   415,
     416,   416,   416,   416,   417,   417,   417,   417,   417,   418,
     418,   418,   418,   419,   419,   420,   420,   420,   420,   420,
     420,   420,   420,   420,   420,   420,   421,   421,   422,   422,
     423,   423,   423,   423,   424,   424,   425,   425,   426,   426,
     427,   427,   428,   428,   429,   429,   431,   430,   432,   433,
     433,   434,   434,   435,   435,   435,   436,   436,   437,   437,
     438,   438,   439,   439,   440,   440,   441,   441,   442,   442,
     443,   443,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   445,   445,   445,   445,   445,   445,   445,
     445,   446,   446,   446,   446,   446,   446,   446,   446,   446,
     447,   448,   448,   449,   449,   450,   450,   450,   451,   451,
     452,   452,   452,   453,   453,   453,   454,   454,   455,   455,
     456,   456,   456,   456,   456,   456,   457,   457,   457,   457,
     457,   458,   458,   458,   458,   458,   458,   459,   459,   460,
     460,   460,   460,   460,   460,   460,   460,   461,   461,   462,
     462,   462,   462,   463,   463,   464,   464,   464,   464,   465,
     465,   465,   465,   466,   466,   466,   466,   466,   466,   467,
     467,   467,   468,   468,   468,   468,   468,   468,   468,   468,
     468,   468,   468,   469,   469,   470,   470,   471,   471,   472,
     472,   472,   472,   473,   473,   474,   474,   475,   475,   476,
     476,   477,   477,   478,   478,   479,   480,   480,   480,   480,
     481,   481,   482,   482,   483,   483,   484,   484,   485,   485,
     486,   487,   487,   488,   488,   488,   488,   489,   489,   489,
     490,   490,   490,   491,   491,   492,   492,   493,   494,   495,
     495,   496,   496,   497,   497,   497,   497,   497,   497,   497,
     497,   497,   497,   497,   498,   498
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
       1,     2,     3,     3,     1,     2,     1,     2,     3,     4,
       3,     1,     2,     1,     2,     2,     1,     3,     1,     3,
       2,     2,     2,     5,     4,     2,     0,     1,     1,     1,
       1,     3,     5,     8,     0,     4,     0,     6,     0,    10,
       0,     4,     2,     3,     2,     3,     2,     3,     3,     3,
       3,     3,     3,     5,     1,     1,     1,     0,     9,     0,
      10,     5,     0,    13,     0,     5,     3,     3,     2,     2,
       2,     2,     2,     2,     3,     2,     2,     3,     2,     2,
       0,     4,     9,     0,     0,     4,     2,     0,     1,     0,
       1,     0,     9,     0,    10,     0,    11,     0,     9,     0,
      10,     0,     8,     0,     9,     0,     7,     0,     8,     0,
       8,     0,     7,     0,     8,     1,     1,     1,     1,     1,
       2,     3,     3,     2,     2,     0,     2,     0,     2,     0,
       1,     3,     1,     3,     2,     0,     1,     2,     4,     1,
       4,     1,     4,     1,     4,     1,     4,     3,     5,     3,
       4,     4,     5,     5,     4,     0,     1,     1,     4,     0,
       5,     0,     2,     0,     3,     0,     7,     8,     6,     2,
       5,     6,     4,     0,     4,     5,     7,     6,     6,     7,
       9,     8,     6,     7,     5,     2,     4,     5,     3,     0,
       3,     4,     6,     5,     5,     6,     8,     7,     2,     0,
       1,     2,     2,     3,     4,     4,     3,     1,     1,     2,
       4,     3,     5,     1,     3,     2,     0,     2,     3,     2,
       0,     0,     4,     0,     5,     2,     2,     2,     0,    11,
       0,    12,     3,     3,     3,     4,     4,     3,     5,     2,
       2,     0,     6,     5,     4,     3,     1,     1,     3,     4,
       1,     2,     1,     1,     5,     6,     1,     1,     4,     1,
       1,     3,     2,     2,     0,     2,     0,     1,     3,     1,
       1,     1,     1,     3,     4,     4,     4,     1,     1,     2,
       2,     2,     3,     3,     1,     1,     1,     1,     3,     1,
       3,     1,     1,     1,     0,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     1,     1,     1,     3,
       5,     1,     3,     5,     4,     3,     3,     3,     4,     3,
       3,     3,     2,     2,     1,     1,     3,     3,     1,     1,
       0,     1,     2,     4,     3,     3,     6,     2,     3,     2,
       3,     6,     1,     1,     1,     1,     1,     6,     3,     4,
       6,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       5,     4,     3,     1,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     1,     1,     1,     1,     1,     1,     3,
       2,     1,     5,     0,     0,    12,     0,    13,     0,     4,
       0,     7,     0,     5,     0,     3,     0,     6,     2,     2,
       4,     1,     1,     5,     3,     5,     3,     2,     0,     2,
       0,     4,     4,     3,     2,     0,     5,     3,     6,     4,
       2,     0,     5,     3,     2,     0,     5,     3,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     2,     0,     2,
       0,     2,     0,     4,     4,     4,     4,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     3,     4,     1,     2,
       4,     2,     6,     0,     1,     4,     0,     2,     0,     1,
       1,     3,     1,     3,     1,     1,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       1,     1,     1,     1,     1,     1,     3,     1,     3,     1,
       1,     3,     1,     1,     1,     2,     1,     0,     0,     1,
       1,     3,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     1,     1,
       4,     3,     4,     1,     1,     1,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       4,     3,     1,     3,     3,     3,     3,     1,     1,     1,
       1,     3,     3,     3,     2,     0,     1,     0,     1,     0,
       5,     3,     3,     1,     1,     1,     1,     3,     2,     1,
       1,     1,     1,     1,     3,     1,     1,     1,     2,     2,
       4,     3,     4,     1,     1,     1,     3,     1,     2,     0,
       5,     3,     3,     1,     3,     1,     2,     0,     5,     3,
       2,     0,     3,     0,     4,     2,     0,     3,     3,     1,
       0,     1,     1,     1,     1,     3,     1,     1,     1,     3,
       1,     1,     3,     3,     2,     4,     2,     4,     5,     5,
       5,     5,     1,     1,     1,     1,     1,     1,     3,     3,
       4,     4,     3,     1,     1,     1,     1,     3,     1,     4,
       3,     1,     1,     1,     1,     1,     3,     3,     4,     4,
       3,     1,     1,     7,     9,     7,     6,     8,     1,     2,
       4,     4,     1,     1,     1,     4,     1,     0,     1,     2,
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
       0,   431,     0,     0,   846,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   938,
       0,   926,   717,     0,   723,   724,   725,    25,   788,   913,
     914,   155,   156,   726,     0,   136,     0,     0,     0,     0,
      26,     0,     0,     0,     0,   190,     0,     0,     0,     0,
       0,     0,   397,   398,   399,   402,   401,   400,     0,     0,
       0,     0,   219,     0,     0,     0,    33,    34,    35,   730,
     732,   733,   727,   728,     0,     0,     0,   734,   729,     0,
     701,    28,    29,    30,    32,    31,     0,   731,     0,     0,
       0,     0,   735,   403,   536,    27,     0,   154,   126,   918,
     718,     0,     0,     4,   116,   118,   787,     0,   700,     0,
       6,   189,     7,     9,     8,    10,     0,     0,   395,   444,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   442,
     901,   902,   518,   514,   515,   516,   517,   425,   521,     0,
     424,   873,   702,   709,     0,   790,   513,   394,   876,   877,
     888,   443,     0,     0,   446,   445,   874,   875,   872,   908,
     912,     0,   503,   789,    11,   402,   401,   400,     0,     0,
      32,     0,   116,   189,     0,   982,   443,   981,     0,   979,
     978,   520,     0,   432,   439,   437,     0,     0,   485,   486,
     487,   488,   512,   510,   509,   508,   507,   506,   505,   504,
      25,   913,   726,   704,    33,    34,    35,     0,     0,  1002,
     894,   702,     0,   703,   466,     0,   464,     0,   942,     0,
     797,   423,   713,   209,     0,  1002,   422,   712,   707,     0,
     722,   703,   921,   922,   928,   920,   714,     0,     0,   716,
     511,     0,     0,     0,     0,   428,     0,   134,   430,     0,
       0,   140,   142,     0,     0,   144,     0,    76,    75,    70,
      69,    61,    62,    53,    73,    84,    85,     0,    56,     0,
      68,    60,    66,    87,    79,    78,    51,    74,    94,    95,
      52,    90,    49,    91,    50,    92,    48,    96,    83,    88,
      93,    80,    81,    55,    82,    86,    47,    77,    63,    97,
      71,    64,    54,    46,    45,    44,    43,    42,    41,    65,
      98,   100,    58,    39,    40,    67,  1046,  1047,    59,  1051,
      38,    57,    89,     0,     0,   116,    99,   993,  1045,     0,
    1048,     0,     0,   146,     0,     0,     0,   180,     0,     0,
       0,     0,     0,     0,   799,     0,   104,   106,   308,     0,
       0,   307,     0,   223,     0,   220,   313,     0,     0,     0,
       0,     0,   999,   205,   217,   934,   938,   555,   578,   578,
       0,   963,     0,   737,     0,     0,     0,   961,     0,    16,
       0,   120,   197,   211,   218,   606,   548,     0,   987,   528,
     530,   532,   850,   431,   444,     0,     0,   442,   443,   445,
       0,     0,   719,     0,   720,     0,     0,     0,   179,     0,
       0,   122,   299,     0,    24,   188,     0,   216,   201,   215,
     400,   403,   189,   396,   169,   170,   171,   172,   173,   175,
     176,   178,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     926,     0,   168,   917,   917,   948,     0,     0,     0,     0,
       0,     0,     0,     0,   393,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   465,   463,
     851,   852,     0,   917,     0,   864,   299,   299,   917,     0,
     919,   909,   934,     0,   189,     0,     0,   148,     0,   848,
     843,   797,     0,   444,   442,     0,   946,     0,   553,   796,
     937,   722,   444,   442,   443,   122,     0,   299,   421,     0,
     866,   715,     0,   126,   259,     0,   535,     0,   151,     0,
       0,   429,     0,     0,     0,     0,     0,   143,   167,   145,
    1046,  1047,  1043,  1044,     0,  1050,  1036,     0,     0,     0,
       0,    72,    37,    59,    36,   994,   174,   177,   147,   126,
       0,   164,   166,     0,     0,     0,     0,   107,     0,   798,
     105,    18,     0,   101,     0,   309,     0,   149,   222,   221,
       0,     0,   150,   983,     0,     0,   444,   442,   443,   446,
     445,     0,  1029,   229,     0,   935,     0,     0,     0,     0,
     797,   797,     0,     0,   152,     0,     0,   736,   962,   788,
       0,     0,   960,   793,   959,   119,     5,    13,    14,     0,
     227,     0,     0,   541,     0,     0,     0,   797,     0,     0,
     710,   705,   542,     0,     0,     0,     0,   850,   126,     0,
     799,   849,  1055,   420,   434,   499,   882,   900,   131,   125,
     127,   128,   129,   130,   394,     0,   519,   791,   792,   117,
     797,     0,  1003,     0,     0,     0,   799,   300,     0,   524,
     191,   225,     0,   469,   471,   470,   482,     0,     0,   502,
     467,   468,   472,   474,   473,   490,   489,   492,   491,   493,
     495,   497,   496,   494,   484,   483,   476,   477,   475,   478,
     479,   481,   498,   480,   916,     0,     0,   952,     0,   797,
     986,     0,   985,  1002,   879,   908,   207,   199,   213,     0,
     987,   203,   189,     0,   435,   438,   440,   448,   462,   461,
     460,   459,   458,   457,   456,   455,   454,   453,   452,   451,
     854,     0,   853,   856,   878,   860,  1002,   857,     0,     0,
       0,     0,     0,     0,     0,     0,   980,   433,   841,   845,
     796,   847,     0,   706,     0,   941,     0,   940,   225,     0,
     706,   925,   924,     0,     0,   853,   856,   923,   857,   426,
     261,   263,   126,   539,   538,   427,     0,   126,   243,   135,
     430,     0,     0,     0,     0,     0,   255,   255,   141,   797,
       0,     0,     0,  1034,   797,     0,  1009,     0,     0,     0,
       0,     0,   795,     0,    33,    34,    35,   701,     0,     0,
     739,   700,   743,   744,   745,   747,     0,   738,   124,   746,
    1002,  1049,     0,     0,     0,     0,    19,     0,    20,     0,
     102,     0,     0,     0,   113,   799,     0,   111,   106,   103,
     108,     0,   306,   314,   311,     0,     0,   972,   977,   974,
     973,   976,   975,    12,  1027,  1028,     0,   797,     0,     0,
       0,   934,   931,     0,   552,     0,   568,   796,   554,   796,
     577,   571,   574,   971,   970,   969,     0,   965,     0,   966,
     968,     0,     5,     0,     0,     0,   600,   601,   609,   608,
       0,   442,     0,   796,   547,   551,     0,     0,   988,     0,
     529,     0,     0,  1016,   850,   285,  1054,     0,     0,   865,
       0,   915,   796,  1005,  1001,   301,   302,   699,   798,   298,
       0,   850,     0,     0,   227,   526,   193,   501,     0,   585,
     586,     0,   583,   796,   947,     0,     0,   299,   229,     0,
     227,     0,     0,   225,     0,   926,   449,     0,     0,   862,
     863,   880,   881,   910,   911,     0,     0,     0,   829,   804,
     805,   806,   813,     0,    33,    34,    35,     0,     0,   817,
     823,   824,   825,   815,   816,   835,   797,     0,   843,   945,
     944,     0,   227,     0,   867,   721,     0,   265,     0,     0,
     132,     0,     0,     0,     0,     0,     0,     0,   235,   236,
     247,     0,   126,   245,   161,   255,     0,   255,     0,   796,
       0,     0,     0,     0,   796,  1035,  1037,  1008,   797,  1007,
       0,   797,   768,   769,   766,   767,   803,     0,   797,   795,
     561,   580,   580,     0,   550,     0,     0,   954,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1040,   181,     0,   184,
     165,     0,     0,   109,   114,   115,   107,   798,   112,     0,
     310,     0,   984,   153,  1000,  1029,  1020,  1024,   228,   230,
     320,     0,     0,   932,     0,     0,   557,     0,   964,     0,
      17,     0,   987,   226,   320,     0,     0,   706,   544,     0,
     711,   989,     0,  1016,   533,     0,     0,  1055,     0,   290,
     288,   856,   868,  1002,   856,   869,  1004,     0,     0,   303,
     123,     0,   850,   224,     0,   850,     0,   500,   951,   950,
       0,   299,     0,     0,     0,     0,     0,     0,   227,   195,
     722,   855,   299,     0,   809,   810,   811,   812,   818,   819,
     833,     0,   797,     0,   829,   565,   582,   582,     0,   808,
     837,   796,   840,   842,   844,     0,   939,     0,   855,     0,
       0,     0,     0,   262,   540,   137,     0,   430,   235,   237,
     934,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     249,     0,  1041,     0,  1030,     0,  1033,   796,     0,     0,
       0,   741,   796,   794,     0,     0,   797,     0,     0,   782,
     797,     0,   785,   784,     0,   797,     0,   748,   786,   783,
     958,     0,   797,   751,   753,   752,     0,     0,   749,   750,
     754,   756,   755,   771,   770,   773,   772,   774,   776,   778,
     777,   775,   764,   763,   758,   759,   757,   760,   761,   762,
     765,  1039,     0,   126,     0,     0,   110,    21,   312,     0,
       0,     0,  1021,  1026,     0,   394,   936,   934,   436,   441,
     447,   559,     0,     0,    15,     0,   394,   612,     0,     0,
     614,   607,   610,     0,   605,     0,   991,     0,  1017,   537,
       0,   291,     0,     0,   286,     0,   305,   304,  1016,     0,
     320,     0,   850,     0,   299,     0,   906,   320,   987,   320,
     990,     0,     0,     0,   450,     0,     0,   821,   796,   828,
     814,     0,     0,   797,     0,     0,   827,   797,     0,   807,
       0,     0,   797,   834,   943,   320,     0,   126,     0,   258,
     244,     0,     0,     0,   234,   157,   248,     0,     0,   251,
       0,   256,   257,   126,   250,  1042,  1031,     0,  1006,     0,
    1053,   802,   801,   740,   569,   796,   560,     0,   572,   796,
     579,   575,     0,   796,   549,   742,     0,   584,   796,   953,
     780,     0,     0,     0,    22,    23,  1023,  1018,  1019,  1022,
     231,     0,     0,     0,   401,   392,     0,     0,     0,   206,
     319,   321,     0,   391,     0,     0,     0,   987,   394,     0,
       0,   556,   967,   316,   212,   603,     0,     0,   543,   531,
       0,   294,   284,     0,   287,   293,   299,   523,  1016,   394,
    1016,     0,   949,     0,   905,   394,     0,   394,   992,   320,
     850,   903,   832,   831,   820,   570,   796,   564,     0,   573,
     796,   581,   576,     0,   822,   796,   836,   394,   126,   264,
     133,   138,   159,   238,     0,   246,   252,   126,   254,  1032,
       0,     0,     0,   563,   781,   546,     0,   957,   956,   779,
     126,   185,  1025,     0,     0,     0,   995,     0,     0,     0,
     232,     0,   987,     0,   357,   353,   359,   701,    32,     0,
     347,     0,   352,   356,   369,     0,   367,   372,     0,   371,
       0,   370,     0,   189,   323,     0,   325,     0,   326,   327,
       0,     0,   933,   558,     0,   604,   602,   613,   611,   295,
       0,     0,   282,   292,     0,     0,  1016,     0,   202,   523,
    1016,   907,   208,   316,   214,   394,     0,     0,     0,   567,
     826,   839,     0,   210,   260,     0,     0,   126,   241,   158,
     253,  1052,   800,     0,     0,     0,     0,     0,     0,   419,
       0,   996,     0,   337,   341,   416,   417,   351,     0,     0,
       0,   332,   665,   664,   661,   663,   662,   682,   684,   683,
     653,   623,   625,   624,   643,   659,   658,   619,   630,   631,
     633,   632,   652,   636,   634,   635,   637,   638,   639,   640,
     641,   642,   644,   645,   646,   647,   648,   649,   651,   650,
     620,   621,   622,   626,   627,   629,   667,   668,   677,   676,
     675,   674,   673,   672,   660,   679,   669,   670,   671,   654,
     655,   656,   657,   680,   681,   685,   687,   686,   688,   689,
     666,   691,   690,   693,   695,   694,   628,   698,   696,   697,
     692,   678,   618,   364,   615,     0,   333,   385,   386,   384,
     377,     0,   378,   334,   411,     0,     0,     0,     0,   415,
       0,   189,   198,   315,     0,     0,     0,   283,   297,   904,
       0,     0,   387,   126,   192,  1016,     0,     0,   204,  1016,
     830,     0,     0,   126,   239,   139,   160,     0,   562,   545,
     955,   183,   335,   336,   414,   233,     0,   797,   797,     0,
     360,   348,     0,     0,     0,   366,   368,     0,     0,   373,
     380,   381,   379,     0,     0,   322,   997,     0,     0,     0,
     418,     0,   317,     0,   296,     0,   598,   799,   126,     0,
       0,   194,   200,     0,   566,   838,     0,     0,   162,   338,
     116,     0,   339,   340,     0,   796,     0,   796,   362,   358,
     363,   616,   617,     0,   349,   382,   383,   375,   376,   374,
     412,   409,  1029,   328,   324,   413,     0,   318,   599,   798,
       0,     0,   388,   126,   196,     0,   242,     0,   187,     0,
     394,     0,   354,   361,   365,     0,     0,   850,   330,     0,
     596,   522,   525,     0,   240,     0,     0,   163,   345,     0,
     393,   355,   410,   998,     0,   799,   405,   850,   597,   527,
       0,   186,     0,     0,   344,  1016,   850,   269,   406,   407,
     408,  1055,   404,     0,     0,     0,   343,  1010,   405,     0,
    1016,     0,   342,     0,     0,  1055,     0,   274,   272,  1010,
     126,   799,  1012,     0,   389,   126,   329,     0,   275,     0,
       0,   270,     0,     0,   798,  1011,     0,  1015,     0,     0,
     278,   268,     0,   271,   277,   331,   182,  1013,  1014,   390,
     279,     0,     0,   266,   276,     0,   267,   281,   280
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   113,   912,   636,   182,  1526,   733,
     353,   354,   355,   356,   865,   866,   867,   115,   116,   117,
     118,   119,   410,   669,   670,   550,   256,  1595,   556,  1504,
    1596,  1838,   854,   348,   579,  1798,  1100,  1293,  1857,   426,
     183,   671,   952,  1166,  1353,   123,   639,   969,   672,   691,
     973,   613,   968,   236,   531,   673,   640,   970,   428,   373,
     393,   126,   954,   915,   890,  1118,  1529,  1222,  1028,  1745,
    1599,   809,  1034,   555,   818,  1036,  1393,   801,  1017,  1020,
    1211,  1864,  1865,   659,   660,   685,   686,   360,   361,   367,
    1564,  1723,  1724,  1305,  1440,  1552,  1717,  1847,  1867,  1756,
    1802,  1803,  1804,  1539,  1540,  1541,  1542,  1758,  1759,  1765,
    1814,  1545,  1546,  1550,  1710,  1711,  1712,  1734,  1906,  1441,
    1442,   184,   128,  1881,  1882,  1715,  1444,  1445,  1446,  1447,
     129,   249,   551,   552,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,  1576,   140,   951,  1165,   141,   656,
     657,   658,   253,   402,   546,   646,   647,  1255,   648,  1256,
     142,   143,   619,   620,  1245,  1246,  1362,  1363,   144,   842,
    1000,   145,   843,  1001,   146,   844,  1002,   622,  1248,  1365,
     147,   845,   148,   149,  1787,   150,   641,  1566,   642,  1135,
     920,  1324,  1321,  1703,  1704,   151,   152,   153,   239,   154,
     240,   250,   413,   538,   155,  1056,  1250,   849,   156,  1057,
     943,   590,  1058,  1003,  1188,  1004,  1190,  1367,  1191,  1192,
    1006,  1371,  1372,  1007,   779,   521,   196,   197,   674,   662,
     502,  1151,  1152,   765,   766,   939,   158,   242,   159,   160,
     186,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     725,   171,   246,   247,   616,   229,   230,   728,   729,  1261,
    1262,   386,   387,   906,   172,   604,   173,   655,   174,   339,
    1725,  1777,   374,   421,   680,   681,  1050,  1894,  1901,  1902,
    1146,  1302,   886,  1303,   887,   888,   823,   824,   825,   340,
     341,   851,   565,  1528,   937
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1589
static const yytype_int16 yypact[] =
{
   -1589,   156, -1589, -1589,  5849, 14291, 14291,   -25, 14291, 14291,
   14291, 11678, 14291, 14291, -1589, 14291, 14291, 14291, 14291, 14291,
   14291, 14291, 14291, 14291, 14291, 14291, 14291, 17128, 17128, 11879,
   14291, 17840,   -22,   -12, -1589, -1589, -1589,   120, -1589,   213,
   -1589, -1589, -1589,   153, 14291, -1589,   -12,     6,    47,   143,
   -1589,   -12, 12080,  2800, 12281, -1589, 14985, 10673,   173, 14291,
    2535,    71, -1589, -1589, -1589,   391,   290,    60,   208,   219,
     236,   283, -1589,  2800,   326,   329,   463,   474,   482, -1589,
   -1589, -1589, -1589, -1589, 14291,   456,  1164, -1589, -1589,  2800,
   -1589, -1589, -1589, -1589,  2800, -1589,  2800, -1589,   402,   384,
    2800,  2800, -1589,   321, -1589, -1589, 12482, -1589, -1589,   395,
     515,   553,   553, -1589,   567,   444,   520,   414, -1589,    88,
   -1589,   584, -1589, -1589, -1589, -1589,   756,   901, -1589, -1589,
     455,   459,   490,   497,   499,   504,   506,   517,  5009, -1589,
   -1589, -1589, -1589,    43,   587,   600,   646, -1589,   654,   659,
   -1589,   141,   538, -1589,   580,   -13, -1589,  2989,   158, -1589,
   -1589,  2276,   144,   556,    66, -1589,   165,   178,   581,   185,
   -1589,    79, -1589,   710, -1589, -1589, -1589,   624,   590,   651,
   -1589, 14291, -1589,   584,   901, 18254,  2331, 18254, 14291, 18254,
   18254, 15509,   629, 17295, 15509, 18254,   781,  2800,   717,   717,
     146,   717,   717,   717,   717,   717,   717,   717,   717,   717,
   -1589, -1589, -1589, -1589, -1589, -1589, -1589,    54, 14291,   656,
   -1589, -1589,   682,   647,   434,   648,   434, 17128, 17343,   643,
     837, -1589,   624, -1589, 14291,   656, -1589,   693, -1589,   695,
     661, -1589,   166, -1589, -1589, -1589,   434,   144, 12683, -1589,
   -1589, 14291,  9266,   849,    96, 18254, 10271, -1589, 14291, 14291,
    2800, -1589, -1589,  5376,   662, -1589, 11662, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589, -1589, -1589, -1589, 16343, -1589, 16343,
   -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589, -1589, -1589,    84,    90,   651, -1589,
   -1589, -1589, -1589,   670,  3123,    94, -1589, -1589,   707,   852,
   -1589,   711, 15696, -1589,   671,   674, 12064, -1589,    11, 12667,
    3404,  3404,  2800,   676,   871,   685, -1589,    39, -1589, 16724,
      97, -1589,   752, -1589,   753, -1589,   877,   101, 17128, 14291,
   14291,   704,   721, -1589, -1589, 16825, 11879, 14291, 14291, 14291,
     103,    91,   447, -1589, 14492, 17128,   510, -1589,  2800, -1589,
     367,   444, -1589, -1589, -1589, -1589, 17938,   887,   802, -1589,
   -1589, -1589,    78, 14291,   713,   715, 18254,   716,  2188,   718,
    6050, 14291,   464,   705,   571,   464,    63,   276, -1589,  2800,
   16343,   712, 10874, 14985, -1589, -1589,  1495, -1589, -1589, -1589,
   -1589, -1589,   584, -1589, -1589, -1589, -1589, -1589, -1589, -1589,
   -1589, -1589, 14291, 14291, 14291, 14291, 12884, 14291, 14291, 14291,
   14291, 14291, 14291, 14291, 14291, 14291, 14291, 14291, 14291, 14291,
   14291, 14291, 14291, 14291, 14291, 14291, 14291, 14291, 14291, 14291,
   18036, 14291, -1589, 14291, 14291, 14291,  5069,  2800,  2800,  2800,
    2800,  2800,   756,   798,  1039, 10472, 14291, 14291, 14291, 14291,
   14291, 14291, 14291, 14291, 14291, 14291, 14291, 14291, -1589, -1589,
   -1589, -1589,   904, 14291, 14291, -1589, 10874, 10874, 14291, 14291,
     395,   172, 16825,   724,   584, 13085, 14275, -1589, 14291, -1589,
     726,   903,   768,   735,   737, 14661,   434, 13286, -1589, 13487,
   -1589,   661,   739,   743,  2430, -1589,   133, 10874, -1589,  1709,
   -1589, -1589, 16298, -1589, -1589, 11075, -1589, 14291, -1589,   839,
    9467,   930,   744, 14476,   928,   119,    64, -1589, -1589, -1589,
     766, -1589, -1589, -1589, 16343, -1589,   476,   761,   937, 16623,
    2800, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589,
     775, -1589, -1589,   750,   763,   773,   764,    68,  2545,  3418,
   -1589, -1589,  2800,  2800, 14291,   434,    71, -1589, -1589, -1589,
   16623,   888, -1589,   434,   121,   123,   777,   779,  2912,   186,
     780,   783,   522,   845,   786,   434,   124,   787, 17399,   782,
     972,   977,   788,   789, -1589,  2167,  2800, -1589, -1589,   921,
    2725,   461, -1589, -1589, -1589,   444, -1589, -1589, -1589,   961,
     861,   817,   222,   841, 14291,   395,   865,   988,   807,   847,
   -1589,   172, -1589, 16343, 16343,   994,   849,    78, -1589,   823,
    1007, -1589, 16343,   104, -1589,   489,   163, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589,  1651,  2779, -1589, -1589, -1589, -1589,
    1008,   846, -1589, 17128, 14291,   832,  1020, 18254,  1016, -1589,
   -1589,   899,  1858, 12266, 18392, 15509, 14984, 14291, 18206, 15158,
   11858, 13264,  4830, 12860,  3802, 14065, 14065, 14065, 14065,  3145,
    3145,  3145,  3145,  3145,  1989,  1989,   738,   738,   738,   146,
     146,   146, -1589,   717, 18254,   834,   835, 17447,   840,  1028,
      -6, 14291,    18,   656,    -8,   172, -1589, -1589, -1589,  1032,
     802, -1589,   584, 16926, -1589, -1589, -1589, 15509, 15509, 15509,
   15509, 15509, 15509, 15509, 15509, 15509, 15509, 15509, 15509, 15509,
   -1589, 14291,   525,   180, -1589, -1589,   656,   543,   850,  3047,
     853,   855,   854,  3427,   125,   848, -1589, 18254,  4335, -1589,
    2800, -1589,   104,    33, 17128, 18254, 17128, 17503,   899,   104,
     434,   187,   895,   862, 14291, -1589,   189, -1589, -1589, -1589,
    9065,   550, -1589, -1589, 18254, 18254,   -12, -1589, -1589, -1589,
   14291,   956, 16501, 16623,  2800,  9668,   864,   867, -1589,  1052,
     973,   939,   908, -1589,  1069,   889,  3643, 16343, 16623, 16623,
   16623, 16623, 16623,   891,  1017,  1018,  1022,   935,   902, 16623,
     536,   940, -1589, -1589, -1589, -1589,   898, -1589, 18348, -1589,
      13, -1589,  6251,  2872,   905,  3418, -1589,  3418, -1589,  2800,
    2800,  3418,  3418,  2800, -1589,  1090,   907, -1589,    87, -1589,
   -1589,  3616, -1589, 18348,  1086, 17128,   912, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589, -1589, -1589,   929,  1101,  2800,  2872,
     914, 16825, 17027,  1098, -1589, 13688, -1589, 14291, -1589, 14291,
   -1589, -1589, -1589, -1589, -1589, -1589,   913, -1589, 14291, -1589,
   -1589,  5447, -1589, 16343,  2872,   916, -1589, -1589, -1589, -1589,
    1102,   922, 14291, 17938, -1589, -1589,  5069,   924, -1589, 16343,
   -1589,   932,  6452,  1089,    73, -1589, -1589,   113,   904, -1589,
    1709, -1589, 16343, -1589, -1589,   434, 18254, -1589, 11276, -1589,
   16623,    80,   936,  2872,   861, -1589, -1589, 15158, 14291, -1589,
   -1589, 14291, -1589, 14291, -1589,  3723,   941, 10874,   845,  1092,
     861, 16343,  1112,   899,  2800, 18036,   434,  4576,   946, -1589,
   -1589,   164,   947, -1589, -1589,  1115,  1065,  1065,  4335, -1589,
   -1589, -1589,  1079,   948,  1063,  1077,  1081,    76,   955, -1589,
   -1589, -1589, -1589, -1589, -1589, -1589,  1143,   970,   726,   434,
     434, 13889,   861,  1709, -1589, -1589,  4706,   703,   -12, 10271,
   -1589,  6653,   971,  6854,   979, 16501, 17128,   986,  1037,   434,
   18348,  1167, -1589, -1589, -1589, -1589,   635, -1589,   328, 16343,
    1002,  1048, 16343,  2800,   476, -1589, -1589, -1589,  1176, -1589,
     995,  1008,   341,   341,  1118,  1118, 16696,   990,  1180, 16623,
   16623, 16623, 16623, 15841, 17938, 16346, 15986, 16623, 16623, 16623,
   16623, 16384, 16623, 16623, 16623, 16623, 16623, 16623, 16623, 16623,
   16623, 16623, 16623, 16623, 16623, 16623, 16623, 16623, 16623, 16623,
   16623, 16623, 16623, 16623, 16623,  2800, -1589, -1589,  1109, -1589,
   -1589,   997,   998, -1589, -1589, -1589,   243,  2545, -1589,  1001,
   -1589, 16623,   434, -1589, -1589,   109, -1589,   516,  1184, -1589,
   -1589,   126,  1005,   434, 11477, 17128, 18254, 17551, -1589,  1501,
   -1589,  5648,   802,  1184, -1589,   374,   -17, -1589, 18254,  1066,
    1009, -1589,  1011,  1089, -1589, 16343,   849, 16343,   102,  1187,
    1125,   192, -1589,   656,   196, -1589, -1589, 17128, 14291, 18254,
   18348,  1015,    80, -1589,  1014,    80,  1019, 15158, 18254, 17607,
    1021, 10874,  1023,  1024, 16343,  1026,  1030, 16343,   861, -1589,
     661,   551, 10874, 14291, -1589, -1589, -1589, -1589, -1589, -1589,
    1080,  1035,  1210,  1146,  4335,  4335,  4335,  4335,  1083, -1589,
   17938,  4335, -1589, -1589, -1589, 17128, 18254,  1043, -1589,   -12,
    1209,  1165, 10271, -1589, -1589, -1589,  1047, 14291,  1037,   434,
   16825, 16501,  1049, 16623,  7055,   641,  1051, 14291,    77,   368,
   -1589,  1068, -1589, 16343, -1589,  1113, -1589,  3873,  1218,  1056,
   16623, -1589, 16623, -1589,  1057,  1058,  1244, 17655,  1060, 18348,
    1248,  1061, -1589, -1589,  1130,  1261,  1076, -1589, -1589, -1589,
   17710,  1074,  1266, 15693, 18436, 10854, 16623, 18302, 13064, 13465,
   13665, 13865, 14657, 15327, 15327, 15327, 15327,  3644,  3644,  3644,
    3644,  3644,   714,   714,   341,   341,   341,  1118,  1118,  1118,
    1118, -1589,  1082, -1589,  1084,  1085, -1589, -1589, 18348,  2800,
   16343, 16343, -1589,   516,  2872,  1175, -1589, 16825, -1589, -1589,
   15509,   434, 14090,  1078, -1589,  1087,  1847, -1589,   313, 14291,
   -1589, -1589, -1589, 14291, -1589, 14291, -1589,   849, -1589, -1589,
     139,  1271,  1203, 14291, -1589,  1094,   434, 18254,  1089,  1096,
   -1589,  1106,    80, 14291, 10874,  1107, -1589, -1589,   802, -1589,
   -1589,  1093,  1111,  1116, -1589,  1117,  4335, -1589,  4335, -1589,
   -1589,  1120,  1105,  1302,  1177,  1121, -1589,  1307,  1122, -1589,
    1181,  1124,  1312, -1589,   434, -1589,  1299, -1589,  1140, -1589,
   -1589,  1142,  1149,   130, -1589, -1589, 18348,  1153,  1154, -1589,
    5299, -1589, -1589, -1589, -1589, -1589, -1589, 16343, -1589, 16343,
   -1589, 18348, 17758, -1589, -1589, 16623, -1589, 16623, -1589, 16623,
   -1589, -1589, 16623, 17938, -1589, -1589, 16623, -1589, 16623, -1589,
   11256, 16623,  1156,  7256, -1589, -1589,   516, -1589, -1589, -1589,
   -1589,   665, 15159,  2872,  1239, -1589,  2471,  1188,  1688, -1589,
   -1589, -1589,   798,  3827,   106,   107,  1160,   802,  1039,   131,
   17128, 18254, -1589, -1589, -1589,  1195,  4949,  5239, 18254, -1589,
     295,  1346,  1279, 14291, -1589, 18254, 10874,  1250,  1089,  1953,
    1089,  1170, 18254,  1174, -1589,  1972,  1173,  2033, -1589, -1589,
      80, -1589, -1589,  1235, -1589, -1589,  4335, -1589,  4335, -1589,
    4335, -1589, -1589,  4335, -1589, 17938, -1589,  2120, -1589,  9065,
   -1589, -1589, -1589, -1589,  9869, -1589, -1589, -1589,  9065, -1589,
    1182, 16623, 17813, 18348, 18348, 18348,  1240, 18348, 17861, 11256,
   -1589, -1589,   516,  2872,  2872,  2800, -1589,  1365, 16131,    86,
   -1589, 15159,   802,  3078, -1589,  1201, -1589,   108,  1186,   110,
   -1589, 15508, -1589, -1589, -1589,   111, -1589, -1589,  1278, -1589,
    1189, -1589,  1301,   584, -1589, 15334, -1589, 15334, -1589, -1589,
    1372,   798, -1589,   434,  4225, -1589, -1589, -1589, -1589,  1373,
    1305, 14291, -1589, 18254,  1198,  1202,  1089,   606, -1589,  1250,
    1089, -1589, -1589, -1589, -1589,  2434,  1204,  4335,  1262, -1589,
   -1589, -1589,  1265, -1589,  9065, 10070,  9869, -1589, -1589, -1589,
    9065, -1589, 18348, 16623, 16623, 16623,  7457,  1206,  1214, -1589,
   16623, -1589,  2872, -1589, -1589, -1589, -1589, -1589, 16343,   627,
    2471, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589,
   -1589, -1589, -1589,   626, -1589,  1188, -1589, -1589, -1589, -1589,
   -1589,   122,   632, -1589,  1391,   112, 15696,  1301,  1397, -1589,
   16343,   584, -1589, -1589,  1216,  1399, 14291, -1589, 18254, -1589,
     318,  1219, -1589, -1589, -1589,  1089,   606, 14811, -1589,  1089,
   -1589,  4335,  4335, -1589, -1589, -1589, -1589,  7658, 18348, 18348,
   18348, -1589, -1589, -1589, 18348, -1589,   709,  1406,  1411,  1224,
   -1589, -1589, 16623, 15508, 15508,  1364, -1589,  1278,  1278,   658,
   -1589, -1589, -1589, 16623,  1341, -1589,  1246,  1233,   114, 16623,
   -1589,  2800, -1589, 16623, 18254,  1347, -1589,  1420, -1589,  7859,
    1234, -1589, -1589,   606, -1589, -1589,  8060,  1236,  1318, -1589,
    1333,  1280, -1589, -1589,  1334, 16343,  1257,   627, -1589, -1589,
   18348, -1589, -1589,  1269, -1589,  1413, -1589, -1589, -1589, -1589,
   18348,  1428,   522, -1589, -1589, 18348,  1258, 18348, -1589,   389,
    1256,  8261, -1589, -1589, -1589,  1259, -1589,  1260,  1286,  2800,
    1039,  1276, -1589, -1589, -1589, 16623,  1283,   129, -1589,  1382,
   -1589, -1589, -1589,  8462, -1589,  2872,   905, -1589,  1293,  2800,
     765, -1589, 18348, -1589,  1274,  1461,   633,   129, -1589, -1589,
    1393, -1589,  2872,  1284, -1589,  1089,   135, -1589, -1589, -1589,
   -1589, 16343, -1589,  1287,  1288,   116, -1589,  1291,   633,   157,
    1089,  1282, -1589, 16343,   614, 16343,   345,  1472,  1404,  1291,
   -1589,  1479, -1589,   394, -1589, -1589, -1589,   161,  1476,  1408,
   14291, -1589,   614,  8663, 16343, -1589, 16343, -1589,  8864,   412,
    1478,  1412, 14291, -1589, 18254, -1589, -1589, -1589, -1589, -1589,
    1480,  1414, 14291, -1589, 18254, 14291, -1589, 18254, 18254
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1589, -1589, -1589,  -565, -1589, -1589, -1589,   151,     2,   -47,
     467, -1589,  -275,  -522, -1589, -1589,   392,    -1,  1450, -1589,
    2309, -1589,  -479, -1589,    57, -1589, -1589, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589, -1589, -1589,  -360, -1589, -1589,  -161,
     204,    24, -1589, -1589, -1589, -1589, -1589, -1589,    25, -1589,
   -1589, -1589, -1589, -1589, -1589,    26, -1589, -1589,  1033,  1025,
    1029,   -84,  -694,  -863,   530,   586,  -369,   286,  -933, -1589,
     -91, -1589, -1589, -1589, -1589,  -737,   127, -1589, -1589, -1589,
   -1589,  -357, -1589,  -617, -1589,  -451, -1589, -1589,   920, -1589,
     -65, -1589, -1589, -1053, -1589, -1589, -1589, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589,  -100, -1589,   -14, -1589, -1589, -1589,
   -1589, -1589,  -183, -1589,    85,  -947, -1589, -1588,  -388, -1589,
    -155,   148,  -101,  -363, -1589,  -191, -1589, -1589, -1589,    93,
     -24,     0,    40,  -723,   -70, -1589, -1589,    21, -1589,   -20,
   -1589, -1589,    -5,   -43,   -51, -1589, -1589, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589,  -615,  -858, -1589, -1589, -1589, -1589,
   -1589,  1100, -1589, -1589, -1589, -1589, -1589, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589, -1589, -1589, -1589,  1150,   471,   360,
   -1589, -1589, -1589, -1589, -1589,   424, -1589, -1589, -1589, -1589,
   -1589, -1589, -1589, -1589,  -940, -1589,  3048,     1, -1589,  2103,
    -400, -1589, -1589,  -483,  3709,  3620, -1589, -1589, -1589,   503,
      28,  -633, -1589, -1589,   576,   371,  -660, -1589,   378, -1589,
   -1589, -1589, -1589, -1589,   557, -1589, -1589, -1589,    98,  -903,
    -109,  -428,  -425, -1589,   630,  -105, -1589, -1589,     3,    38,
     582, -1589, -1589,   364,   -16, -1589,  -373,    58,   260, -1589,
    -310, -1589, -1589, -1589,  -462,  1205, -1589, -1589, -1589, -1589,
   -1589,   649,   992, -1589, -1589, -1589,  -361,  -696, -1589,  1157,
   -1141, -1589,   -58,  -190,     5,   751, -1589,  -320, -1589,  -332,
   -1071, -1241,  -236,   162, -1589,   472,   546, -1589, -1589, -1589,
   -1589,   496, -1589,  1445, -1087
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1039
static const yytype_int16 yytable[] =
{
     185,   187,   483,   189,   190,   191,   193,   194,   195,   337,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   513,   650,   228,   231,   433,   935,   122,   124,
     125,  1147,   238,   252,   243,   652,   405,   345,   394,   255,
     931,   930,   397,   398,   972,   535,   257,   263,   788,   266,
     774,   261,   346,   949,   349,   770,   771,   505,   336,   357,
    1330,   120,  1429,   407,   800,  1139,   864,   869,   429,   244,
     722,   911,  1327,   482,   763,   584,   586,   764,   344,   255,
    1038,  1316,   404,   433,   254,   390,   793,  1024,   391,   245,
     409,  1164,  1218,   -72,  1012,  1612,   816,   423,   -72,   -37,
     852,   406,   157,   -36,   -37,   547,   596,  1175,   -36,  1391,
     601,   796,   547,  -587,   797,  1555,  1557,  -350,  1005,  1620,
    1705,  1774,    14,  1774,   380,  1612,   580,    14,   814,    14,
     875,  1767,   547,   892,   892,   892,  -592,   539,   407,   892,
     892,   592,   540,   366,   522,  -703,   381,  1322,  1791,  1207,
    -711,  1148,   127,   632,   358,   114,     3,   404,  1768,   884,
     885,   625,   211,    40,   726,   409,  1198,   500,   501,   188,
     859,  1095,   248,  -100,   503,   524,   406,  1460,    14,   932,
    1323,  -895,   251,   516,    14,  1331,   967,   581,  -100,   592,
    -595,   533,   420,   768,   523,  1896,  1149,   -99,   772,  1919,
     258,   593,   409,   470,   264,  1834,  1254,   335,   121,   383,
     532,  -883,   -99,   406,   503,   471,   211,    40,  -704,   384,
     385,  -534,  1461,  -593,   372,   500,   501,  -897,  -886,   406,
     860,  1199,  1108,  -890,  -889,  -884,  -927,  -894,   500,   501,
    1897,   259,   508,   542,  1920,   392,   542,   372,  -885,   419,
    -887,   372,   372,   255,   553,   508,  -592,  -930,   530,  -929,
     507,   817,  -870,   564,  -798,   677,  -871,  1467,  -798,  -289,
     359,   692,   626,  1392,  1332,  -289,   917,   372,   510,  1178,
     -72,  1429,  1613,  1614,   424,  1150,   -37,  1469,  1384,  -796,
     -36,  1527,   548,   597,  1475,   575,  1477,   602,  1225,   624,
    1229,   504,  1556,  1558,  -350,   484,  1621,  1706,  1775,   544,
    1824,  1462,  1892,   549,   815,  1352,   876,  1769,   877,   893,
     985,  1306,  1497,  1021,  -273,  1503,  1562,   607,  1023,  1898,
    -798,   432,   109,  1921,  1161,  -893,  -710,   260,  -883,  1104,
    1105,   504,  1370,  -705,   336,   859,   606,  1131,   520,   357,
     357,   587,  -892,   775,   610,  -886,  1785,  -896,  -899,   381,
    -890,  -889,  -884,  -927,   255,   406,   632,  1455,   690,   509,
     347,   228,   618,   255,   255,  -885,   337,  -887,  1569,   630,
     507,   881,   509,   433,  -930,   918,  -929,   635,   514,  -870,
    1615,   225,   225,  -871,  1091,  1092,  1093,  1577,   193,  1579,
     919,  1786,   368,   114,   399,   419,   675,   114,  1916,   605,
    1094,   554,   364,   369,  1718,   744,  1719,   687,   621,   621,
     365,   394,   739,   740,   429,   336,  1585,  1849,  1908,  1121,
     370,   511,   384,   385,  1227,  1228,  1315,   693,   694,   695,
     696,   698,   699,   700,   701,   702,   703,   704,   705,   706,
     707,   708,   709,   710,   711,   712,   713,   714,   715,   716,
     717,   718,   719,   720,   721,   746,   723,  1570,   724,   724,
     727,   238,  1850,   243,  1227,  1228,   395,   371,   732,   678,
     747,   748,   749,   750,   751,   752,   753,   754,   755,   756,
     757,   758,   759,   574,  1381,  1930,  1300,  1301,   724,   769,
     661,   687,   687,   724,   773,  1731,   745,   820,   244,  1736,
     747,   400,  1154,   777,   362,  1155,  1172,  1909,   401,   483,
     375,   363,   785,   376,   787,   803,  1230,   336,   245,   419,
     381,  1329,   687,   377,   734,  1364,  1366,  1366,  1317,   381,
     804,  1373,   805,   966,   378,  1339,   382,   381,  1341,   781,
     650,  1318,   379,  1224,   938,  1516,   940,   210,   127,  -594,
     767,   114,   652,   637,   638,   395,  1394,   821,   500,   501,
     679,  1319,   884,   885,   335,  1180,   978,   372,   396,    50,
    1101,   974,  1102,   734,  1931,   864,   161,   868,   868,   871,
     482,   225,   411,   381,   792,   500,   501,   798,   381,   418,
     632,   921,   627,   384,   385,   412,   419,   808,   422,   224,
     226,   383,   384,   385,   121,   214,   215,   216,  1300,  1301,
     384,   385,   425,   500,   501,  1018,  1019,   574,   372,   737,
     372,   372,   372,   372,   956,   179,   381,  1592,    89,   406,
    1762,    91,    92,   415,    93,   180,    95,  -706,   898,   900,
     535,   434,  1476,   762,   381,   435,   651,  -588,  1763,   910,
    1096,   632,  1917,  1770,  1790,   633,   384,   385,  1793,   105,
    -589,   384,   385,   938,   940,   924,   574,  1764, -1002,   946,
    1013,   940,  1771,  -897,  1014,  1772,   436,   742,   408,  1817,
     795,   650,   957,   437, -1002,   438,  1482,  1354,  1483,   420,
     439,   114,   440,   652,  -858,    34,    35,    36,  1818,   384,
     385,  1819,  1459,   441, -1002,   420,  -590, -1002,   212,  -858,
    1345,   850,  -861,   225,   473,  1471,   965,   384,   385,   474,
    -859,  1355,   225, -1002,   609,   475,   735,  -861,   476,   225,
    1226,  1227,  1228,   870,   679,  -859,  1388,  1227,  1228,   225,
     506,  1560,  1878,  1879,  1880,   661,   977,   964,  1383,   414,
     416,   417,   735,   408,  1088,  1089,  1090,  1091,  1092,  1093,
      79,    80,    81,    82,    83,  -891,   905,   907,  1209,  1210,
    -591,   217,  -704,  1094,   512,   735,   471,    87,    88,  1016,
     210,   467,   468,   469,  1889,   470,   735,  1523,  1524,   735,
     408,    97,  1732,  1733,  1887,   255,  1022,   471,  1907,   526,
    1904,  1905,    50,   388,  1423,   102,   534,   583,   585,  1899,
    1815,  1816,   484,  1811,  1812,   517,  1588,   650,  1589,   519,
    1590,  1048,  1051,  1591,   161,   420,  1616,   210,   161,   652,
     525,  -895,   507,   372,   528,  1449,   529,  1040,   214,   215,
     216,  -702,  1045,   536,   868,   537,   868,   545,   558,    50,
     868,   868,  1106,  1586,   566, -1038,   569,   576,   179,   570,
     577,    89,  1033,   588,    91,    92,   225,    93,   180,    95,
     589,   591,   598,   599,    62,    63,    64,   175,   176,   430,
    1126,   600,  1127,  1473,   805,   214,   215,   216,  1499,   611,
     612,   653,   105,  1129,   654,   676,  -121,  1799,   663,    55,
     664,   665,   780,   667,  1508,  1116,  1179,  1138,   689,   427,
     778,    91,    92,   627,    93,   180,    95,  1740,   732,   999,
     782,  1008,   783,   806,   789,   122,   124,   125,   790,   547,
     810,   595,   813,  1159,  1866,   564,   827,   855,   127,   105,
     603,   114,   608,  1167,   431,   826,  1168,   615,  1169,   856,
     858,  1874,   687,  1335,  1866,  1031,   114,   631,   120,   853,
     857,   874,   878,  1888,   879,   882,   238,   889,   243,   883,
     891,   897,   894,   896,  1140,   210,   899,   211,    40,   901,
     902,   908,   161,   913,   914,   916,   767,   923,   798,  -726,
     127,   922,   925,   114,   121,   926,  1206,    50,   929,   157,
    1103,   679,    55,   244,   933,  1574,   934,   942,  1212,  1594,
      62,    63,    64,   175,   176,   430,   944,   947,  1600,   948,
     950,   953,   661,   245,  1202,   959,   960,   963,   962,  1117,
     650,  1606,   955,   214,   215,   216,   971,   225,   981,   661,
     982,   979,   652,  -708,  1308,   983,   121,  1015,  1025,   127,
    1035,  1039,   114,  1037,  1041,  1253,  1043,   760,  1259,    91,
      92,   798,    93,   180,    95,  1042,  1213,   574,  1044,  1239,
     127,  1794,  1795,   114,  1046,  1059,  1243,  1060,  1061,   762,
     431,   795,  1062,  1063,   615,  1067,  1064,   105,  1066,  1107,
    1111,   761,  1099,   109,  1309,  1109,   868,   225,  1113,  1114,
    1115,  1120,  1124,  1134,  1128,   121,  1136,  1137,  1747,  1310,
    1141,  1145,   650,  1143,  1174,   372,  1177,   220,   220,  1183,
    1162,  1193,   161,  1195,   652,  1171,   121,  1187,  1187,   999,
    1182,  -898,  1194,  1184,  1185,  1186,   210,  1196,   225,  1200,
     225,  1197,  1201,  1337,  1830,   122,   124,   125,    62,    63,
      64,   175,   176,   430,   795,  1203,   687,  1215,    50,   127,
     114,   127,   114,  1221,   114,  1217,   225,   687,  1310,  1431,
    1220,  1223,  1232,   651,  1233,  1237,   735,  1094,   120,  1242,
    1238,  1241,  1292,  1304,  1235,  1294,  1295,  1297,   735,  1307,
     735,  1333,  1325,   967,   214,   215,   216,  1326,  1334,  1376,
    1338,  1340,   255,  1342,   574,  1344,  1356,   574,  1346,  1358,
    1359,  1347,  1390,  1349,    14,   121,  1350,   121,   431,   157,
      91,    92,  1877,    93,   180,    95,  1357,   992,  1369,   225,
    1375,  1377,  1378,  1380,  1385,   210,   850,  1389,  1395,  1397,
    1399,  1400,  1403,  1405,  1789,   225,   225,  1409,   105,  1404,
     661,  1408,  1411,   661,  1796,   945,  1412,    50,  1915,  1379,
    1413,  1415,  1417,   735,  1406,  1418,  1452,  1422,  1410,   127,
    1424,  1425,   114,  1414,  1453,  1463,  1464,  1432,  1466,  1478,
    1419,  1468,  1433,  1561,    62,    63,    64,   175,  1434,   430,
    1435,  1470,  1474,   214,   215,   216,  1485,  1451,  1479,  1831,
    1480,  1486,  1481,  1488,  1456,  1484,  1490,  1493,  1457,  1494,
    1458,  1495,  1489,  1492,   651,   976,   388,   220,  1465,    91,
      92,  1498,    93,   180,    95,   121,  1500,  1501,  1472,   687,
    1436,  1437,   433,  1438,  1502,   999,   999,   999,   999,  1505,
    1506,  1531,   999,  1520,  1853,  1544,  1559,   105,  1565,   210,
    1571,   389,  1572,   114,   431,  1580,  1009,  1575,  1010,  1581,
    1583,  1587,   127,  1439,   628,   114,  1604,  1601,   634,  1610,
    1618,    50,   161,  1619,  1714,  1713,  1720,  1726,  1727,   225,
     225,  1487,  1716,  1729,  1029,  1491,  1730,   161,  1741,  1739,
    1496,  1742,  1752,  1448,   628,  1773,   634,   628,   634,   634,
    1753,  1779,  1782,  1783,  1448,  1805,  1788,   214,   215,   216,
    1807,  1913,  1809,  1813,  1821,  1822,  1918,  1823,   121,  1829,
    1828,  1833,  1836,  1837,   161,  -346,  1840,  1842,  1839,  1844,
     661,  1707,  1845,    91,    92,  1708,    93,   180,    95,  1768,
    1426,  1851,  1848,  1443,  1855,  1854,  1861,  1112,  1573,   220,
     651,   687,  1856,  1863,  1443,  1868,  1872,  1609,   220,  1875,
    1876,   105,  1548,   615,  1123,   220,  1884,   219,   219,  1900,
    1886,   235,  1890,  1891,  1893,   220,  1910,  1911,  1914,   225,
    1922,  1923,  1932,   161,  1935,  1933,  1871,  1936,  1173,  1296,
    1133,   342,   736,  1885,  1382,  1746,   235,   999,   738,   999,
    1883,   442,   443,   444,   161,   741,   872,  1507,  1737,  1617,
    1761,   225,  1766,  1551,  1925,  1895,  1778,  1532,  1735,   623,
    1611,   445,   446,  1251,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,  1368,   470,  1320,
    1781,  1598,  1244,  1189,  1360,  1204,  1728,  1448,  1153,   225,
     471,   127,  1361,  1448,   114,  1448,   210,  1049,   661,  1912,
     688,   617,  1927,   335,   225,   225,  1846,  1299,  1522,  1549,
    1236,  1291,     0,     0,     0,  1448,   484,     0,    50,     0,
       0,   161,     0,   161,     0,   161,     0,  1029,  1219,     0,
       0,     0,   220,     0,     0,     0,     0,  1443,     0,     0,
       0,     0,     0,  1443,     0,  1443,     0,   121,     0,     0,
       0,     0,     0,     0,   214,   215,   216,   999,     0,   999,
       0,   999,     0,     0,   999,  1443,  1553,   127,     0,     0,
     114,     0,  1744,  1598,     0,   114,   127,     0,     0,   114,
      91,    92,     0,    93,   180,    95,     0,     0,     0,  1776,
       0,   225,     0,   651,     0,     0,   372,   219,     0,   574,
       0,     0,   335,  1448,     0,  1859,     0,     0,   105,   689,
       0,     0,  1702,     0,     0,     0,     0,     0,     0,  1709,
       0,     0,  1313,   121,     0,     0,   335,  1311,   335,     0,
       0,     0,   121,   161,     0,   335,     0,     0,   336,     0,
       0,  1784,   562,  1826,   563,     0,     0,   235,     0,   235,
       0,     0,     0,  1443,     0,     0,     0,     0,   999,  1336,
       0,     0,   127,     0,     0,   114,   114,   114,   127,     0,
       0,   114,     0,     0,   127,   651,     0,   114,     0,   433,
       0,     0,     0,     0,     0,  1721,     0,     0,     0,   210,
      62,    63,    64,    65,    66,   430,     0,     0,     0,   568,
       0,    72,   477,   220,   235,  1806,  1808,  1374,     0,     0,
     210,    50,   211,    40,   161,     0,     0,     0,   121,     0,
       0,     0,   615,  1029,   121,     0,   161,     0,     0,   219,
     121,     0,    50,     0,   225,     0,  1547,     0,   219,     0,
       0,     0,   479,     0,     0,   219,     0,   214,   215,   216,
       0,     0,     0,     0,     0,   219,     0,     0,     0,     0,
     431,     0,     0,   220,     0,     0,   235,     0,   214,   215,
     216,  1431,     0,    91,    92,     0,    93,   180,    95,     0,
       0,     0,     0,     0,     0,   682,     0,   574,   342,     0,
     235,     0,   760,   235,    91,    92,     0,    93,   180,    95,
       0,   105,  1548,     0,   220,     0,   220,     0,   335,   615,
       0,     0,   999,   999,     0,   127,    14,     0,   114,     0,
       0,     0,   105,     0,     0,  1924,   794,  1800,   109,     0,
       0,     0,   220,     0,  1702,  1702,     0,  1934,  1709,  1709,
     235,     0,     0,     0,     0,     0,     0,  1937,     0,     0,
    1938,     0,   372,     0,     0,     0,     0,   127,     0,   210,
     114,     0,     0,     0,   127,   661,     0,   114,     0,     0,
       0,   121,     0,     0,     0,     0,     0,  1431,     0,  1432,
       0,    50,   219,     0,  1433,   661,    62,    63,    64,   175,
    1434,   430,  1435,     0,   661,   220,  1431,     0,     0,   127,
       0,     0,   114,     0,     0,     0,     0,     0,  1860,     0,
    1858,   220,   220,   121,     0,     0,     0,   214,   215,   216,
     121,   127,    14,     0,   114,   161,     0,     0,     0,   819,
    1873,     0,  1436,  1437,   235,  1438,   235,     0,     0,   840,
       0,    14,     0,    91,    92,     0,    93,   180,    95,     0,
       0,     0,  1563,     0,     0,   121,   431,  1431,     0,   464,
     465,   466,   467,   468,   469,  1454,   470,     0,     0,     0,
     840,   105,   955,     0,     0,     0,     0,   121,   471,     0,
       0,   127,     0,     0,   114,  1432,   127,     0,     0,   114,
    1433,     0,    62,    63,    64,   175,  1434,   430,  1435,     0,
       0,   161,    14,     0,  1432,     0,   161,     0,     0,  1433,
     161,    62,    63,    64,   175,  1434,   430,  1435,   927,   928,
       0,     0,     0,   235,   235,     0,     0,   936,     0,     0,
       0,     0,   235,     0,     0,     0,     0,   121,  1436,  1437,
       0,  1438,   121,     0,  1431,   220,   220,     0,     0,     0,
     222,   222,     0,   219,     0,     0,     0,  1436,  1437,     0,
    1438,     0,   431,     0,     0,  1432,     0,     0,     0,     0,
    1433,  1578,    62,    63,    64,   175,  1434,   430,  1435,     0,
       0,   431,     0,     0,     0,     0,     0,     0,     0,    14,
    1582,     0,     0,     0,     0,     0,   161,   161,   161,     0,
       0,     0,   161,     0,     0,     0,     0,     0,   161,     0,
       0,     0,     0,   219,     0,     0,     0,     0,  1436,  1437,
       0,  1438,   515,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,     0,     0,     0,     0,     0,
       0,     0,   431,     0,     0,   220,     0,     0,     0,     0,
       0,  1584,  1432,     0,   219,     0,   219,  1433,     0,    62,
      63,    64,   175,  1434,   430,  1435,     0,     0,   210,     0,
     903,     0,   904,     0,     0,   498,   499,   220,     0,     0,
       0,     0,   219,   840,     0,     0,     0,     0,     0,     0,
      50,   682,   682,     0,     0,     0,   235,   235,   840,   840,
     840,   840,   840,     0,     0,  1436,  1437,     0,  1438,   840,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   235,     0,   220,   214,   215,   216,   431,
       0,     0,     0,     0,     0,     0,     0,     0,  1593,     0,
     220,   220,   500,   501,     0,   219,     0,     0,     0,   161,
     222,     0,    91,    92,     0,    93,   180,    95,     0,   235,
       0,   219,   219,   498,   499,   515,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,  1132,     0,
     105,     0,     0,   235,   235,   338,     0,     0,     0,     0,
       0,   161,     0,   235,  1142,     0,     0,     0,   161,   235,
       0,     0,     0,   666,     0,     0,     0,  1156,     0,     0,
       0,     0,   235,     0,     0,     0,     0,     0,   498,   499,
     840,     0,     0,   235,     0,     0,     0,   220,     0,     0,
     500,   501,     0,   161,     0,     0,  1176,     0,     0,     0,
       0,   235,     0,     0,     0,   235,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   161,     0,     0,  1431,     0,
       0,     0,     0,     0,   515,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,     0,     0,     0,
       0,     0,   222,     0,     0,   500,   501,     0,     0,     0,
       0,   222,     0,     0,     0,   219,   219,     0,   222,     0,
       0,     0,     0,    14,  1231,     0,     0,  1234,   222,   235,
       0,     0,   235,     0,   235,   161,     0,   498,   499,   649,
     161,     0,  1533,     0,     0,     0,     0,     0,     0,   840,
     840,   840,   840,     0,   235,     0,     0,   840,   840,   840,
     840,   840,   840,   840,   840,   840,   840,   840,   840,   840,
     840,   840,   840,   840,   840,   840,   840,   840,   840,   840,
     840,   840,   840,   840,   840,     0,  1432,     0,     0,     0,
     220,  1433,   210,    62,    63,    64,   175,  1434,   430,  1435,
       0,   840,     0,     0,   500,   501,     0,     0,     0,     0,
       0,     0,     0,     0,    50,   219,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   338,     0,   338,     0,
    1328,     0,   936,     0,     0,   235,  1534,   235,     0,  1436,
    1437,     0,  1438,     0,     0,     0,     0,   219,     0,  1535,
     214,   215,   216,  1536,     0,   222,   210,     0,     0,  1348,
       0,     0,  1351,   431,   235,   791,   210,   235,     0,     0,
     179,     0,  1738,    89,  1537,     0,    91,    92,    50,    93,
    1538,    95,     0,   338,     0,     0,   350,   351,    50,     0,
     235,     0,     0,     0,     0,   219,   861,   862,     0,     0,
       0,     0,     0,     0,   105,     0,     0,     0,     0,     0,
     219,   219,     0,   840,   214,   215,   216,     0,  1396,     0,
       0,     0,  1156,   235,   214,   215,   216,   235,     0,     0,
     840,     0,   840,     0,     0,     0,     0,   352,     0,     0,
      91,    92,     0,    93,   180,    95,     0,   863,     0,     0,
      91,    92,     0,    93,   180,    95,   840,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   105,   338,
       0,     0,   338,     0,     0,   442,   443,   444,   105,     0,
       0,     0,     0,     0,     0,  1427,  1428,     0,     0,     0,
     235,   235,     0,     0,   235,   445,   446,   219,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,     0,   470,     0,     0,     0,   222,     0,     0,   442,
     443,   444,     0,     0,   471,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   445,
     446,     0,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,     0,   470,     0,     0,     0,
       0,     0,  1509,     0,  1510,     0,   222,   235,   471,   235,
       0,     0,     0,     0,     0,   840,     0,   840,     0,   840,
       0,     0,   840,   235,     0,     0,   840,     0,   840,     0,
       0,   840,     0,   338,     0,   822,     0,     0,   841,     0,
       0,   210,   235,   235,     0,     0,   235,   222,  1554,   222,
       0,     0,     0,   235,     0,     0,     0,     0,     0,     0,
     219,     0,     0,    50,     0,     0,     0,     0,     0,   841,
       0,     0,     0,     0,     0,   222,     0,     0,     0,     0,
       0,     0,     0,   909,     0,     0,   515,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   214,
     215,   216,     0,     0,     0,   235,     0,     0,     0,     0,
       0,     0,     0,   210,     0,     0,     0,     0,     0,     0,
       0,   840,   338,   338,     0,    91,    92,     0,    93,   180,
      95,   338,     0,   235,   235,    50,     0,   941,   222,   498,
     499,   235,     0,   235,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   105,   222,   222,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   235,     0,   235,     0,     0,
       0,   214,   215,   216,   235,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   649,     0,     0,     0,
       0,   179,     0,     0,    89,    90,     0,    91,    92,     0,
      93,   180,    95,     0,     0,     0,   500,   501,     0,     0,
       0,     0,     0,   840,   840,   840,     0,   442,   443,   444,
     840,     0,   235,  1757,     0,   105,     0,     0,   235,     0,
     235,     0,     0,     0,     0,   221,   221,   445,   446,   237,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,     0,   470,     0,     0,   880,    62,    63,
      64,    65,    66,   430,     0,     0,   471,     0,     0,    72,
     477,     0,   841,     0,     0,     0,     0,     0,   222,   222,
       0,     0,     0,     0,     0,   338,   338,   841,   841,   841,
     841,   841,     0,     0,     0,     0,     0,     0,   841,     0,
       0,     0,     0,     0,   277,     0,     0,     0,   478,   210,
     479,     0,     0,     0,     0,  1780,     0,   649,     0,     0,
     235,     0,     0,   480,     0,   481,     0,     0,   431,     0,
       0,    50,   279,     0,     0,     0,     0,   235, -1039, -1039,
   -1039, -1039, -1039,   462,   463,   464,   465,   466,   467,   468,
     469,     0,   470,  1534,   210,     0,   235,     0,     0,     0,
       0,     0,   840,     0,   471,     0,  1535,   214,   215,   216,
    1536,     0,   338,   840,     0,     0,    50,     0,   222,   840,
       0,     0,     0,   840,   567,     0,     0,   179,   338,     0,
      89,    90,     0,    91,    92,   980,    93,  1538,    95,     0,
    1841,   338,     0,     0,     0,   235,     0,     0,     0,   841,
     222,   560,   214,   215,   216,   561,     0,     0,     0,     0,
       0,   105,     0,     0,     0,   221,     0,     0,     0,     0,
     338,     0,   179,     0,     0,    89,   329,     0,    91,    92,
       0,    93,   180,    95,     0,   840,     0,     0,     0,     0,
       0,     0,     0,   649,     0,   235,   333,     0,   222,     0,
       0,     0,     0,     0,     0,     0,   105,   334,     0,     0,
       0,     0,   235,   222,   222,     0,   936,     0,     0,     0,
       0,   235,     0,     0,     0,     0,     0,     0,  1903,     0,
     936,     0,     0,   235,     0,   235,     0,     0,   338,     0,
       0,   338,     0,   822,     0,     0,     0,     0,     0,  1903,
       0,  1928,     0,     0,   235,     0,   235,     0,   841,   841,
     841,   841,     0,     0,     0,     0,   841,   841,   841,   841,
     841,   841,   841,   841,   841,   841,   841,   841,   841,   841,
     841,   841,   841,   841,   841,   841,   841,   841,   841,   841,
     841,   841,   841,   841,     0,     0,     0,   221,     0,     0,
     222,     0,     0,     0,     0,     0,   221,     0,     0,     0,
     841,     0,     0,   221,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   221,     0,     0,     0,   442,   443,   444,
       0,     0,     0,     0,   221,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   338,     0,   338,   445,   446,     0,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   338,   470,   210,   338,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   471,     0,     0,   210,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,   649,     0,   237,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   841,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   338,   214,   215,   216,   338,     0,     0,   841,
       0,   841,     0,   222,     0,     0,     0,   214,   215,   216,
     221,     0,     0,     0,     0,     0,   352,     0,     0,    91,
      92,     0,    93,   180,    95,   841,     0,     0,     0,     0,
     863,     0,     0,    91,    92,     0,    93,   180,    95,     0,
       0,     0,     0,     0,     0,     0,     0,   105,   649,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   338,
     338,   105,     0,     0,     0,     0,     0,   846,     0,     0,
       0,     0,     0,     0,     0,   984,   442,   443,   444,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   445,   446,   846,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,     0,   470,   277,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   471,     0, -1039, -1039, -1039,
   -1039, -1039,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,
       0,     0,   279,     0,     0,     0,   338,     0,   338,     0,
       0,     0,     0,  1094,   841,     0,   841,     0,   841,     0,
       0,   841,     0,     0,   210,   841,     0,   841,     0,     0,
     841,   221,     0,   442,   443,   444,   223,   223,     0,     0,
     241,   338,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,   338,   445,   446,     0,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,     0,
     470,   560,   214,   215,   216,   561,     0,     0,     0,     0,
       0,   221,   471,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   179,     0,     0,    89,   329,     0,    91,    92,
       0,    93,   180,    95,  1110,  1047,     0,     0,     0,     0,
     841,     0,     0,     0,     0,     0,   333,     0,     0,     0,
       0,     0,   221,     0,   221,     0,   105,   334,     0,     0,
     338,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   277,   470,
     221,   846,     0,     0,   338,     0,   338,     0,     0,     0,
       0,   471,     0,   338,     0,     0,   846,   846,   846,   846,
     846,     0,     0,     0,     0,     0,   279,   846,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1098,     0,     0,   277,     0,     0,     0,   210,     0,
       0,     0,   841,   841,   841,     0,     0,     0,     0,   841,
       0,  1170,     0,   221,     0,     0,     0,   338,     0,     0,
      50,     0,   279,     0,     0,     0,   223,  1119,  -393,   221,
     221,     0,     0,     0,     0,     0,    62,    63,    64,   175,
     176,   430,     0,     0,   210,     0,     0,     0,     0,     0,
       0,     0,  1119,     0,     0,   560,   214,   215,   216,   561,
       0,   221,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   179,     0,     0,    89,
     329,     0,    91,    92,     0,    93,   180,    95,   846,     0,
       0,  1163,     0,     0,     0,     0,     0,     0,     0,     0,
     333,   560,   214,   215,   216,   561,   431,     0,     0,     0,
     105,   334,     0,   237,     0,     0,     0,     0,     0,   338,
       0,     0,   179,     0,     0,    89,   329,     0,    91,    92,
       0,    93,   180,    95,     0,  1398,   338,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   333,     0,     0,     0,
       0,     0,     0,     0,     0,  1801,   105,   334,   223,     0,
       0,   841,     0,   221,   221,     0,     0,   223,     0,     0,
       0,     0,   841,     0,   223,     0,     0,     0,   841,     0,
       0,     0,   841,     0,   223,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   241,     0,   846,   846,   846,
     846,     0,   221,     0,   338,   846,   846,   846,   846,   846,
     846,   846,   846,   846,   846,   846,   846,   846,   846,   846,
     846,   846,   846,   846,   846,   846,   846,   846,   846,   846,
     846,   846,   846,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   841,     0,     0,     0,     0,   846,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   221,     0,     0,     0,     0,     0,   241,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   848,
     338,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   338,     0,   338,   221,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     873,   223,     0,   338,     0,   338,     0,     0,   267,   268,
       0,   269,   270,     0,     0,   271,   272,   273,   274,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   221,     0,
       0,     0,   275,   221,   276,     0,   277,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   221,   221,
       0,   846,     0,     0,     0,     0,     0,     0,   847,     0,
       0,     0,   278,     0,   279,     0,     0,     0,   846,     0,
     846,     0,     0,     0,     0,     0,   280,   281,   282,   283,
     284,   285,   286,     0,     0,     0,   210,     0,     0,   847,
       0,     0,     0,     0,   846,     0,     0,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,    50,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,  1430,     0,     0,   221,   322,   323,   324,     0,
       0,     0,   325,   326,   214,   215,   216,   327,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   328,   986,   987,    89,   329,     0,
      91,    92,   223,    93,   180,    95,   330,     0,   331,     0,
       0,   332,     0,     0,     0,   988,     0,     0,   333,     0,
       0,     0,     0,   989,   990,   991,   210,     0,   105,   334,
       0,     0,     0,  1722,     0,     0,   992,     0,     0,     0,
       0,     0,     0,  1030,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1052,  1053,
    1054,  1055,   223,   846,     0,   846,     0,   846,     0,  1065,
     846,   221,     0,     0,   846,     0,   846,     0,     0,   846,
       0,     0,     0,   993,   994,   995,   996,     0,     0,     0,
       0,  1530,     0,     0,  1543,     0,     0,     0,     0,   997,
       0,     0,     0,   223,     0,   223,     0,     0,   221,     0,
      91,    92,     0,    93,   180,    95,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   998,     0,
       0,   223,   847,     0,     0,     0,     0,     0,   105,     0,
       0,     0,     0,     0,     0,     0,     0,   847,   847,   847,
     847,   847,     0,   221,     0,     0,     0,     0,   847,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   846,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1160,  1607,  1608,     0,     0,     0,     0,     0,     0,     0,
       0,  1543,     0,     0,   223,     0,   442,   443,   444,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     223,   223,     0,     0,     0,     0,   445,   446,     0,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   241,   470,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   471,     0,     0,     0,     0,
       0,   846,   846,   846,     0,     0,     0,     0,   846,   847,
    1755,     0,     0,     0,     0,     0,     0,     0,  1543,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1247,  1249,  1249,     0,   241,     0,     0,  1260,  1263,  1264,
    1265,  1267,  1268,  1269,  1270,  1271,  1272,  1273,  1274,  1275,
    1276,  1277,  1278,  1279,  1280,  1281,  1282,  1283,  1284,  1285,
    1286,  1287,  1288,  1289,  1290,     0,   442,   443,   444,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1298,     0,     0,   223,   223,   445,   446,     0,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,     0,   470,     0,     0,     0,     0,   847,   847,
     847,   847,     0,   241,  1181,   471,   847,   847,   847,   847,
     847,   847,   847,   847,   847,   847,   847,   847,   847,   847,
     847,   847,   847,   847,   847,   847,   847,   847,   847,   847,
     847,   847,   847,   847,     0,     0,     0,     0,     0,     0,
     846,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     847,   846,     0,     0,     0,     0,     0,   846,     0,     0,
       0,   846,     0,     0,   223,     0,     0,     0,     0,     0,
       0,     0,     0,  1386,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1401,     0,  1402,     0,     0,     0,   223,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,  1420,   470,     0,     0,
       0,     0,     0,   846,     0,     0,     0,     0,     0,   471,
       0,     0,     0,  1870,  1208,     0,     0,     0,     0,   241,
       0,     0,     0,     0,   223,     0,     0,     0,     0,     0,
    1530,     0,     0,     0,     0,     0,     0,     0,     0,   223,
     223,     0,   847,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   847,
       0,   847,     0,     0,     0,     0,     0,     0,     0,   442,
     443,   444,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   847,     0,     0,     0,   445,
     446,     0,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,     0,   470,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   223,     0,   471,   442,
     443,   444,     0,     0,     0,  1512,     0,  1513,     0,  1514,
       0,     0,  1515,     0,     0,     0,  1517,     0,  1518,   445,
     446,  1519,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,     0,   470,     0,     0,     0,
       0,     0,   267,   268,     0,   269,   270,     0,   471,   271,
     272,   273,   274,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   275,     0,   276,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   847,     0,   847,     0,   847,     0,
       0,   847,   241,     0,     0,   847,   278,   847,     0,     0,
     847,  1602,     0,     0,     0,     0,     0,     0,     0,     0,
     280,   281,   282,   283,   284,   285,   286,  1567,     0,     0,
     210,     0,   211,    40,     0,     0,     0,     0,     0,   223,
       0,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,    50,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,     0,     0,     0,   730,
     322,   323,   324,     0,   241,   472,   325,   571,   214,   215,
     216,   572,     0,     0,     0,     0,     0,     0,     0,     0,
     847,     0,     0,  1748,  1749,  1750,     0,     0,   573,     0,
    1754,     0,     0,     0,    91,    92,     0,    93,   180,    95,
     330,     0,   331,     0,     0,   332,     0,     0,     0,   442,
     443,   444,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   105,     0,     0,     0,   731,     0,   109,   445,
     446,     0,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,     0,   470,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   471,   442,
     443,   444,   847,   847,   847,     0,     0,     0,     0,   847,
       0,     0,     0,     0,     0,     0,     0,     0,  1760,   445,
     446,  1391,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,     0,   470,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   471,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1810,     0,     0,     0,   442,   443,   444,     0,
       0,     0,     0,  1820,     0,     0,     0,     0,     0,  1825,
       0,     0,     0,  1827,     0,     0,   445,   446,     0,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,     0,   470,     0,     0,     0,  1568,     0,     0,
       0,     0,     0,     0,     0,   471,     0,     0,     0,     0,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,  1862,     0,     0,     0,     0,
       0,   847,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,   847,     0,     0,     0,     0,     0,   847,     0,
       0,     0,   847,     0,     0,  1392,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,  1843,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,   847,    52,    53,    54,    55,    56,
      57,    58,     0,    59,    60,    61,    62,    63,    64,    65,
      66,    67,   557,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,     0,
      79,    80,    81,    82,    83,     0,     0,     0,    84,     0,
       0,    85,     0,     0,     0,     0,    86,    87,    88,    89,
      90,     0,    91,    92,     0,    93,    94,    95,    96,     0,
       0,    97,     0,     0,    98,     0,     0,     0,     0,     0,
      99,   100,     0,   101,     0,   102,   103,   104,     0,     0,
     105,   106,     0,   107,   108,  1130,   109,   110,     0,   111,
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
      56,    57,    58,     0,    59,    60,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
       0,    79,    80,    81,    82,    83,     0,     0,     0,    84,
       0,     0,    85,     0,     0,     0,     0,    86,    87,    88,
      89,    90,     0,    91,    92,     0,    93,    94,    95,    96,
       0,     0,    97,     0,     0,    98,     0,     0,     0,     0,
       0,    99,   100,     0,   101,     0,   102,   103,   104,     0,
       0,   105,   106,     0,   107,   108,  1314,   109,   110,     0,
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
       0,    84,     0,     0,    85,     0,     0,     0,     0,   179,
      87,    88,    89,    90,     0,    91,    92,     0,    93,   180,
      95,    96,     0,     0,    97,     0,     0,    98,     0,     0,
       0,     0,     0,    99,     0,     0,     0,     0,   102,   103,
     104,     0,     0,   105,   106,     0,   107,   108,   668,   109,
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
     103,   104,     0,     0,   105,   106,     0,   107,   108,  1097,
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
    1144,   109,   110,     0,   111,   112,     5,     6,     7,     8,
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
     108,  1214,   109,   110,     0,   111,   112,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,  1216,
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
     107,   108,     0,   109,   110,     0,   111,   112,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,  1387,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,     0,    79,    80,
      81,    82,    83,     0,     0,     0,    84,     0,     0,    85,
       0,     0,     0,     0,   179,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   180,    95,    96,     0,     0,    97,
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
     106,     0,   107,   108,  1521,   109,   110,     0,   111,   112,
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
     105,   106,     0,   107,   108,  1751,   109,   110,     0,   111,
     112,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,  1797,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
       0,    79,    80,    81,    82,    83,     0,     0,     0,    84,
       0,     0,    85,     0,     0,     0,     0,   179,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   180,    95,    96,
       0,     0,    97,     0,     0,    98,     0,     0,     0,     0,
       0,    99,     0,     0,     0,     0,   102,   103,   104,     0,
       0,   105,   106,     0,   107,   108,     0,   109,   110,     0,
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
       0,     0,   105,   106,     0,   107,   108,  1832,   109,   110,
       0,   111,   112,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,  1835,    48,     0,
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
     103,   104,     0,     0,   105,   106,     0,   107,   108,  1852,
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
    1869,   109,   110,     0,   111,   112,     5,     6,     7,     8,
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
     108,  1926,   109,   110,     0,   111,   112,     5,     6,     7,
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
     107,   108,  1929,   109,   110,     0,   111,   112,     5,     6,
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
       0,   107,   108,     0,   109,   110,     0,   111,   112,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,   543,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,   175,   176,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,     0,   179,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   180,    95,     0,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
     106,     0,   107,   108,     0,   109,   110,     0,   111,   112,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,   807,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,    63,    64,   175,
     176,    67,     0,    68,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,     0,
      79,    80,    81,    82,    83,     0,     0,     0,    84,     0,
       0,    85,     0,     0,     0,     0,   179,    87,    88,    89,
      90,     0,    91,    92,     0,    93,   180,    95,     0,     0,
       0,    97,     0,     0,    98,     0,     0,     0,     0,     0,
      99,     0,     0,     0,     0,   102,   103,   104,     0,     0,
     105,   106,     0,   107,   108,     0,   109,   110,     0,   111,
     112,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
    1032,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,  1597,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,  1743,     0,     0,     0,     0,     0,     0,     0,
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
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,   403,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
     743,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
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
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,   106,     0,     0,     0,
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
       0,     0,    62,    63,    64,   175,   176,   177,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     178,    75,    76,    77,    78,     0,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,    85,     0,     0,
       0,     0,   179,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   180,    95,     0,     0,     0,    97,     0,     0,
      98,     0,     0,     0,     0,     0,    99,     0,     0,     0,
       0,   102,   103,   104,     0,     0,   105,   181,     0,   343,
       0,     0,   109,   110,     0,   111,   112,     5,     6,     7,
       8,     9,     0,     0,     0,  1071,     0,    10,  1072,  1073,
    1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,
       0,     0,   683,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1094,    15,    16,     0,     0,     0,     0,
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
      92,     0,    93,   180,    95,     0,   684,     0,    97,     0,
       0,    98,     0,     0,     0,     0,     0,    99,     0,     0,
       0,     0,   102,   103,   104,     0,     0,   105,   181,     0,
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
       0,     0,     0,     0,    62,    63,    64,   175,   176,   177,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   178,    75,    76,    77,    78,     0,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,    85,
       0,     0,     0,     0,   179,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   180,    95,     0,     0,     0,    97,
       0,     0,    98,     0,     0,     0,     0,     0,    99,     0,
       0,     0,     0,   102,   103,   104,     0,     0,   105,   181,
       0,     0,   802,     0,   109,   110,     0,   111,   112,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,  1093,     0,     0,  1157,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1094,    15,    16,     0,     0,
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
       0,    91,    92,     0,    93,   180,    95,     0,  1158,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
     181,     0,     0,     0,     0,   109,   110,     0,   111,   112,
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
       0,     0,     0,     0,     0,     0,    62,    63,    64,   175,
     176,   177,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   178,    75,    76,    77,    78,     0,
      79,    80,    81,    82,    83,     0,     0,     0,     0,     0,
       0,    85,     0,     0,     0,     0,   179,    87,    88,    89,
      90,     0,    91,    92,     0,    93,   180,    95,     0,     0,
       0,    97,     0,     0,    98,     0,     0,     0,     0,     0,
      99,     0,     0,     0,     0,   102,   103,   104,     0,     0,
     105,   106,   442,   443,   444,     0,   109,   110,     0,   111,
     112,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   445,   446,     0,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,     0,   470,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,   471,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,   192,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     175,   176,   177,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   178,    75,    76,    77,    78,
       0,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,    85,     0,     0,     0,     0,   179,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   180,    95,     0,
       0,     0,    97,     0,     0,    98,     0,     0,   559,     0,
       0,    99,     0,     0,     0,     0,   102,   103,   104,     0,
       0,   105,   181,     0,     0,     0,     0,   109,   110,     0,
     111,   112,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,     0,   470,     0,   227,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   471,     0,    15,
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
       0,     0,   105,   181,   442,   443,   444,     0,   109,   110,
       0,   111,   112,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   445,   446,     0,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
       0,   470,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,   471,     0,     0,    17,     0,    18,    19,
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
     578,     0,     0,    99,     0,     0,     0,     0,   102,   103,
     104,     0,     0,   105,   181,     0,   262,   443,   444,   109,
     110,     0,   111,   112,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,   445,   446,     0,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,     0,   470,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,   471,     0,    17,     0,    18,
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
     103,   104,     0,     0,   105,   181,     0,   265,     0,     0,
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
       0,    62,    63,    64,   175,   176,   177,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   178,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   179,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   180,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,   106,   442,   443,   444,
       0,   109,   110,     0,   111,   112,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   445,   446,     0,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,     0,   470,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,   471,     0,     0,    17,
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
      98,     0,     0,   582,     0,     0,    99,     0,     0,     0,
       0,   102,   103,   104,     0,     0,   105,   181,   541,     0,
       0,     0,   109,   110,     0,   111,   112,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   697,   470,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   471,
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
       0,     0,   102,   103,   104,     0,     0,   105,   181,     0,
       0,     0,     0,   109,   110,     0,   111,   112,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,  1073,
    1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,
       0,     0,     0,   743,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1094,     0,    15,    16,     0,     0,     0,
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
       0,     0,     0,     0,   109,   110,     0,   111,   112,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
       0,   470,     0,     0,   784,     0,     0,     0,     0,     0,
       0,     0,     0,   471,     0,     0,    15,    16,     0,     0,
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
      10,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
    1093,     0,     0,     0,     0,   786,     0,     0,     0,     0,
       0,     0,     0,     0,  1094,     0,     0,    15,    16,     0,
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
       0,    10,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
    1093,     0,     0,     0,     0,     0,  1125,     0,     0,     0,
       0,     0,     0,     0,  1094,     0,     0,     0,    15,    16,
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
       0,     0,    10,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
    1093,     0,     0,     0,     0,     0,     0,  1205,     0,     0,
       0,     0,     0,     0,  1094,     0,     0,     0,     0,    15,
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
       0,     0,     0,    10, -1039, -1039, -1039, -1039,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,     0,   470,     0,     0,     0,     0,     0,  1450,     0,
       0,     0,     0,     0,   471,     0,     0,     0,     0,     0,
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
     104,     0,     0,   105,   181,   442,   443,   444,     0,   109,
     110,     0,   111,   112,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   445,   446,     0,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,     0,   470,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,   471,     0,     0,    17,     0,    18,
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
     776,     0,     0,     0,    99,     0,     0,     0,     0,   102,
     103,   104,     0,     0,   105,   181,   442,   443,   444,     0,
     109,   110,     0,   111,   112,     5,     6,     7,     8,     9,
       0,     0,     0,     0,   811,    10,   445,   446,     0,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,     0,   470,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   471,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,   629,    39,    40,     0,   812,     0,
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
     276,   109,   110,     0,   111,   112,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,  1093,     0,     0,     0,     0,     0,   278,     0,
       0,     0,     0,     0,     0,     0,  1094,     0,     0,     0,
       0,     0,   280,   281,   282,   283,   284,   285,   286,     0,
       0,     0,   210,     0,   211,    40,     0,     0,     0,     0,
       0,     0,     0,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,    50,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,     0,     0,
       0,   321,   322,   323,   324,     0,     0,     0,   325,   571,
     214,   215,   216,   572,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   267,   268,     0,   269,   270,     0,
     573,   271,   272,   273,   274,     0,    91,    92,     0,    93,
     180,    95,   330,     0,   331,     0,     0,   332,   275,     0,
     276,     0,   277,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   105,     0,     0,     0,   731,     0,
     109,     0,     0,     0,     0,     0,     0,     0,   278,     0,
     279,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   280,   281,   282,   283,   284,   285,   286,     0,
       0,     0,   210,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,    50,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,     0,     0,
       0,     0,   322,   323,   324,     0,     0,     0,   325,   326,
     214,   215,   216,   327,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     328,     0,     0,    89,   329,     0,    91,    92,     0,    93,
     180,    95,   330,     0,   331,     0,     0,   332,   267,   268,
       0,   269,   270,     0,   333,   271,   272,   273,   274,     0,
       0,     0,     0,     0,   105,   334,     0,     0,     0,  1792,
       0,     0,   275,     0,   276,   446,   277,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
       0,   470,   278,     0,   279,     0,     0,     0,     0,     0,
       0,     0,     0,   471,     0,     0,   280,   281,   282,   283,
     284,   285,   286,     0,     0,     0,   210,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,    50,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,     0,     0,     0,   321,   322,   323,   324,     0,
       0,     0,   325,   326,   214,   215,   216,   327,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   328,     0,     0,    89,   329,     0,
      91,    92,     0,    93,   180,    95,   330,     0,   331,     0,
       0,   332,   267,   268,     0,   269,   270,     0,   333,   271,
     272,   273,   274,     0,     0,     0,     0,     0,   105,   334,
       0,     0,     0,     0,     0,     0,   275,     0,   276,     0,
     277,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,     0,   470,   278,     0,   279,     0,
       0,     0,     0,     0,     0,     0,     0,   471,     0,     0,
     280,   281,   282,   283,   284,   285,   286,     0,     0,     0,
     210,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,    50,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,     0,     0,     0,     0,
     322,   323,   324,     0,     0,     0,   325,   326,   214,   215,
     216,   327,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   328,     0,
       0,    89,   329,     0,    91,    92,     0,    93,   180,    95,
     330,     0,   331,     0,     0,   332,     0,   267,   268,     0,
     269,   270,   333,  1525,   271,   272,   273,   274,     0,     0,
       0,     0,   105,   334,     0,     0,     0,     0,     0,     0,
       0,   275,     0,   276,     0,   277, -1039, -1039, -1039, -1039,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,  1093,     0,     0,     0,     0,     0,     0,     0,
       0,   278,     0,   279,     0,     0,  1094,     0,     0,     0,
       0,     0,     0,     0,     0,   280,   281,   282,   283,   284,
     285,   286,     0,     0,     0,   210,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,    50,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,     0,     0,     0,     0,   322,   323,   324,     0,     0,
       0,   325,   326,   214,   215,   216,   327,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   328,     0,     0,    89,   329,     0,    91,
      92,     0,    93,   180,    95,   330,     0,   331,     0,     0,
     332,  1622,  1623,  1624,  1625,  1626,     0,   333,  1627,  1628,
    1629,  1630,     0,     0,     0,     0,     0,   105,   334,     0,
       0,     0,     0,     0,     0,  1631,  1632,  1633,     0,   445,
     446,     0,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,  1634,   470,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   471,  1635,
    1636,  1637,  1638,  1639,  1640,  1641,     0,     0,     0,   210,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1642,  1643,  1644,  1645,  1646,  1647,  1648,  1649,  1650,  1651,
    1652,    50,  1653,  1654,  1655,  1656,  1657,  1658,  1659,  1660,
    1661,  1662,  1663,  1664,  1665,  1666,  1667,  1668,  1669,  1670,
    1671,  1672,  1673,  1674,  1675,  1676,  1677,  1678,  1679,  1680,
    1681,  1682,     0,     0,     0,  1683,  1684,   214,   215,   216,
       0,  1685,  1686,  1687,  1688,  1689,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1690,  1691,  1692,
       0,     0,     0,    91,    92,     0,    93,   180,    95,  1693,
       0,  1694,  1695,     0,  1696,     0,     0,     0,     0,     0,
       0,  1697,  1698,     0,  1699,     0,  1700,  1701,     0,   267,
     268,   105,   269,   270,  1069,  1070,   271,   272,   273,   274,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   275,  1071,   276,     0,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,     0,
       0,     0,     0,   278,     0,     0,     0,     0,     0,     0,
       0,     0,  1094,     0,     0,     0,     0,   280,   281,   282,
     283,   284,   285,   286,     0,     0,     0,   210,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,    50,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,     0,     0,     0,   321,   322,   323,   324,
       0,     0,     0,   325,   571,   214,   215,   216,   572,     0,
       0,     0,     0,     0,   267,   268,     0,   269,   270,     0,
       0,   271,   272,   273,   274,   573,     0,     0,     0,     0,
       0,    91,    92,     0,    93,   180,    95,   330,   275,   331,
     276,     0,   332,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   105,
       0,     0,     0,     0,     0,     0,     0,     0,   278,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   280,   281,   282,   283,   284,   285,   286,     0,
       0,     0,   210,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,    50,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,     0,     0,
       0,  1252,   322,   323,   324,     0,     0,     0,   325,   571,
     214,   215,   216,   572,     0,     0,     0,     0,     0,   267,
     268,     0,   269,   270,     0,     0,   271,   272,   273,   274,
     573,     0,     0,     0,     0,     0,    91,    92,     0,    93,
     180,    95,   330,   275,   331,   276,     0,   332,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   105,     0,     0,     0,     0,     0,
       0,     0,     0,   278,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   280,   281,   282,
     283,   284,   285,   286,     0,     0,     0,   210,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,    50,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,     0,     0,     0,  1258,   322,   323,   324,
       0,     0,     0,   325,   571,   214,   215,   216,   572,     0,
       0,     0,     0,     0,   267,   268,     0,   269,   270,     0,
       0,   271,   272,   273,   274,   573,     0,     0,     0,     0,
       0,    91,    92,     0,    93,   180,    95,   330,   275,   331,
     276,     0,   332,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   105,
       0,     0,     0,     0,     0,     0,     0,     0,   278,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   280,   281,   282,   283,   284,   285,   286,     0,
       0,     0,   210,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,    50,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,     0,     0,
       0,     0,   322,   323,   324,     0,     0,     0,   325,   571,
     214,   215,   216,   572,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     573,     0,     0,     0,     0,     0,    91,    92,     0,    93,
     180,    95,   330,     0,   331,     0,     0,   332,   442,   443,
     444,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   105,     0,     0,     0,   445,   446,
       0,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,     0,   470,  1068,  1069,  1070,     0,
       0,     0,     0,     0,     0,     0,     0,   471,     0,     0,
       0,     0,     0,     0,   277,     0,     0,  1071,     0,     0,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,  1093,   279,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1094,  1266,     0,     0,     0,
       0,     0,     0,     0,   210,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   828,   829,     0,     0,     0,     0,
     830,     0,   831,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,   832,     0,     0,     0,     0,     0,
       0,     0,    34,    35,    36,   210,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   212,     0,     0,     0,     0,
       0,   560,   214,   215,   216,   561,     0,    50,     0,     0,
       0,     0,     0,   799,     0,     0,     0,     0,     0,     0,
       0,     0,   179,     0,     0,    89,   329,     0,    91,    92,
       0,    93,   180,    95,     0,     0,     0,     0,     0,     0,
       0,     0,   833,   834,   835,   836,   333,    79,    80,    81,
      82,    83,     0,     0,     0,     0,   105,   334,   217,  1026,
       0,  1257,     0,   179,    87,    88,    89,   837,     0,    91,
      92,     0,    93,   180,    95,     0,     0,     0,    97,     0,
       0,     0,     0,     0,     0,     0,     0,   838,     0,     0,
       0,    29,   102,     0,     0,     0,     0,   105,   839,    34,
      35,    36,   210,     0,   211,    40,     0,     0,     0,     0,
       0,     0,   212,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   213,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1027,    75,
     214,   215,   216,     0,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,   217,     0,     0,     0,     0,
     179,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     180,    95,     0,   828,   829,    97,     0,     0,     0,   830,
       0,   831,     0,     0,     0,     0,     0,     0,     0,   102,
       0,     0,     0,   832,   105,   218,     0,     0,     0,     0,
     109,    34,    35,    36,   210,     0,  1068,  1069,  1070,     0,
       0,     0,     0,     0,   212,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,  1071,     0,     0,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,  1093,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   833,   834,   835,   836,  1094,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,   217,     0,     0,
       0,     0,   179,    87,    88,    89,   837,     0,    91,    92,
       0,    93,   180,    95,    29,     0,     0,    97,     0,     0,
       0,     0,    34,    35,    36,   210,   838,   211,    40,     0,
       0,   102,     0,     0,     0,   212,   105,   839,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,  1240,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   213,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,   214,   215,   216,     0,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,   217,     0,
       0,     0,     0,   179,    87,    88,    89,    90,     0,    91,
      92,     0,    93,   180,    95,    29,     0,     0,    97,     0,
       0,     0,     0,    34,    35,    36,   210,     0,   211,    40,
       0,     0,   102,     0,     0,     0,   212,   105,   218,     0,
       0,   594,     0,   109,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   614,    75,   214,   215,   216,     0,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,   217,
       0,     0,     0,     0,   179,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   180,    95,    29,     0,   975,    97,
       0,     0,     0,     0,    34,    35,    36,   210,     0,   211,
      40,     0,     0,   102,     0,     0,     0,   212,   105,   218,
       0,     0,     0,     0,   109,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     213,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,   214,   215,   216,     0,    79,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
     217,     0,     0,     0,     0,   179,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   180,    95,    29,     0,     0,
      97,     0,     0,     0,     0,    34,    35,    36,   210,     0,
     211,    40,     0,     0,   102,     0,     0,     0,   212,   105,
     218,     0,     0,     0,     0,   109,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   213,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1122,    75,   214,   215,   216,     0,
      79,    80,    81,    82,    83,     0,     0,     0,     0,     0,
       0,   217,     0,     0,     0,     0,   179,    87,    88,    89,
      90,     0,    91,    92,     0,    93,   180,    95,    29,     0,
       0,    97,     0,     0,     0,     0,    34,    35,    36,   210,
       0,   211,    40,     0,     0,   102,     0,     0,     0,   212,
     105,   218,     0,     0,     0,     0,   109,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   213,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    75,   214,   215,   216,
       0,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,   217,     0,     0,     0,     0,   179,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   180,    95,     0,
       0,     0,    97,     0,     0,   442,   443,   444,     0,     0,
       0,     0,     0,     0,     0,     0,   102,     0,     0,     0,
       0,   105,   218,     0,     0,   445,   446,   109,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,     0,   470,   442,   443,   444,     0,     0,     0,     0,
       0,     0,     0,     0,   471,     0,     0,     0,     0,     0,
       0,     0,     0,   445,   446,     0,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,     0,
     470,     0,     0,     0,     0,     0,     0,     0,     0,   442,
     443,   444,   471,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   445,
     446,   518,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,     0,   470,   442,   443,   444,
       0,     0,     0,     0,     0,     0,     0,     0,   471,     0,
       0,     0,     0,     0,     0,     0,     0,   445,   446,   527,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,     0,   470,     0,     0,     0,     0,     0,
       0,     0,     0,   442,   443,   444,   471,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   445,   446,   895,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,     0,
     470,   442,   443,   444,     0,     0,     0,     0,     0,     0,
       0,     0,   471,     0,     0,     0,     0,     0,     0,     0,
       0,   445,   446,   961,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,     0,   470,     0,
       0,     0,     0,     0,     0,     0,     0,   442,   443,   444,
     471,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   445,   446,  1011,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,     0,   470,  1068,  1069,  1070,     0,     0,
       0,     0,     0,     0,     0,     0,   471,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1071,  1312,     0,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
    1093,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1068,  1069,  1070,     0,  1094,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1071,     0,  1343,  1072,  1073,  1074,  1075,  1076,  1077,
    1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,
    1088,  1089,  1090,  1091,  1092,  1093,     0,     0,  1068,  1069,
    1070,     0,     0,     0,     0,     0,     0,     0,     0,  1094,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1071,
       0,  1407,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,  1093,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1068,  1069,  1070,     0,  1094,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1071,     0,  1416,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,     0,
       0,  1068,  1069,  1070,     0,     0,     0,     0,     0,     0,
       0,     0,  1094,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1071,     0,  1511,  1072,  1073,  1074,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,  1092,  1093,     0,    34,    35,
      36,   210,     0,   211,    40,     0,     0,     0,     0,     0,
    1094,   212,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,  1603,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   232,     0,     0,     0,     0,     0,
     233,     0,     0,     0,     0,     0,     0,     0,     0,   214,
     215,   216,     0,    79,    80,    81,    82,    83,     0,     0,
       0,     0,     0,     0,   217,     0,     0,  1605,     0,   179,
      87,    88,    89,    90,     0,    91,    92,     0,    93,   180,
      95,     0,     0,     0,    97,     0,    34,    35,    36,   210,
       0,   211,    40,     0,     0,     0,     0,     0,   102,   643,
       0,     0,     0,   105,   234,     0,     0,     0,     0,   109,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   213,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   214,   215,   216,
       0,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,   217,     0,     0,     0,     0,   179,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   180,    95,     0,
       0,     0,    97,     0,    34,    35,    36,   210,     0,   211,
      40,     0,     0,     0,     0,     0,   102,   212,     0,     0,
       0,   105,   644,     0,     0,     0,     0,   645,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     232,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   214,   215,   216,     0,    79,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
     217,     0,     0,     0,     0,   179,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   180,    95,     0,     0,     0,
      97,     0,     0,     0,     0,     0,   442,   443,   444,     0,
       0,     0,     0,     0,   102,     0,     0,     0,     0,   105,
     234,     0,     0,     0,     0,   109,   445,   446,   958,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,     0,   470,   442,   443,   444,     0,     0,     0,
       0,     0,     0,     0,     0,   471,     0,     0,     0,     0,
       0,     0,     0,     0,   445,   446,     0,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
       0,   470,  1068,  1069,  1070,     0,     0,     0,     0,     0,
       0,     0,     0,   471,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1071,  1421,     0,  1072,  1073,  1074,  1075,
    1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,  1068,  1069,
    1070,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1094,     0,     0,     0,     0,     0,     0,     0,  1071,
       0,     0,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,  1093,   444,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1094,     0,     0,
       0,     0,   445,   446,     0,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,  1070,   470,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   471,     0,     0,     0,     0,     0,  1071,     0,     0,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,  1093,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1094
};

static const yytype_int16 yycheck[] =
{
       5,     6,   157,     8,     9,    10,    11,    12,    13,    56,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   183,   396,    29,    30,   127,   660,     4,     4,
       4,   934,    31,    33,    31,   396,   106,    57,    96,    44,
     657,   656,   100,   101,   740,   235,    46,    52,   531,    54,
     512,    51,    57,   686,    59,   506,   507,   162,    56,    60,
    1147,     4,  1303,   106,   543,   923,   588,   589,   126,    31,
     470,   636,  1143,   157,   502,   350,   351,   502,    57,    84,
     817,  1134,   106,   184,    44,    86,   537,   810,    89,    31,
     106,   954,  1025,     9,   788,     9,    32,     9,    14,     9,
     579,   106,     4,     9,    14,     9,     9,   970,    14,    32,
       9,   539,     9,    70,   539,     9,     9,     9,   778,     9,
       9,     9,    49,     9,    84,     9,   115,    49,     9,    49,
       9,     9,     9,     9,     9,     9,    70,   246,   181,     9,
       9,   102,   247,    83,    90,   158,    83,   164,  1736,  1012,
     158,    38,     4,    90,    83,     4,     0,   181,    36,    50,
      51,    70,    83,    84,   474,   181,    90,   134,   135,   194,
     102,   158,   194,   179,    70,   218,   181,    38,    49,   658,
     197,   194,   194,   188,    49,    83,   194,   176,   194,   102,
      70,   234,   179,   503,   218,    38,    83,   179,   508,    38,
     194,   162,   218,    57,    53,  1793,  1064,    56,     4,   155,
     234,    70,   194,   218,    70,    69,    83,    84,   158,   156,
     157,     8,    83,    70,    73,   134,   135,   194,    70,   234,
     162,   155,   865,    70,    70,    70,    70,   194,   134,   135,
      83,   194,    70,   248,    83,    94,   251,    96,    70,   162,
      70,   100,   101,   258,   259,    70,    70,    70,   230,    70,
     194,   197,    70,   179,   191,   202,    70,  1338,   195,   191,
     199,   432,   381,   196,   172,   195,    54,   126,   199,   973,
     196,  1522,   196,   197,   196,   172,   196,  1340,  1221,   180,
     196,  1432,   196,   196,  1347,   342,  1349,   196,  1035,   196,
    1037,   197,   196,   196,   196,   157,   196,   196,   196,   252,
     196,   172,   196,   256,   195,  1178,   195,   195,   195,   195,
     195,   195,  1375,   802,   195,   195,   195,   370,   807,   172,
     195,   127,   199,   172,   951,   194,   158,   194,   197,   861,
     862,   197,  1200,   158,   342,   102,   370,   912,   197,   350,
     351,   352,   194,   514,   370,   197,    38,   194,   194,    83,
     197,   197,   197,   197,   369,   370,    90,    54,   426,   197,
     197,   376,   377,   378,   379,   197,   423,   197,    83,   384,
     194,   195,   197,   484,   197,   163,   197,   388,   184,   197,
    1531,    27,    28,   197,    53,    54,    55,  1468,   403,  1470,
     178,    83,   194,   252,    83,   162,   411,   256,    14,   369,
      69,   260,   122,   194,  1555,   485,  1557,   422,   378,   379,
     130,   479,   480,   481,   482,   423,  1479,    38,    83,   891,
     194,   171,   156,   157,   106,   107,  1132,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   485,   471,   172,   473,   474,
     475,   470,    83,   470,   106,   107,   163,   194,   476,   203,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   342,  1217,    83,   102,   103,   503,   504,
     402,   506,   507,   508,   509,  1576,   485,    31,   470,  1580,
     515,   190,   940,   518,   123,   940,   967,   172,   197,   674,
     194,   130,   527,   194,   529,   545,   198,   525,   470,   162,
      83,  1146,   537,    70,   476,  1195,  1196,  1197,   164,    83,
     545,  1201,   547,   733,    70,  1162,    90,    83,  1165,   521,
     923,   177,    70,  1032,   663,  1413,   665,    81,   410,    70,
     502,   410,   923,   196,   197,   163,   198,    91,   134,   135,
     419,   197,    50,    51,   423,   975,   766,   426,   194,   103,
     855,   742,   857,   525,   172,  1107,     4,   588,   589,   594,
     674,   227,   197,    83,   536,   134,   135,   539,    83,    32,
      90,   644,   155,   156,   157,    90,   162,   550,   194,    27,
      28,   155,   156,   157,   410,   139,   140,   141,   102,   103,
     156,   157,    38,   134,   135,    75,    76,   476,   477,   478,
     479,   480,   481,   482,   692,   159,    83,  1495,   162,   644,
      14,   165,   166,    90,   168,   169,   170,   158,   620,   621,
     840,   196,  1348,   502,    83,   196,   396,    70,    32,   198,
     850,    90,  1903,    31,  1735,   155,   156,   157,  1739,   193,
      70,   156,   157,   782,   783,   647,   525,    51,   158,   684,
     789,   790,    50,   194,   789,    53,   196,   483,   106,    31,
     539,  1064,   697,   196,   158,   196,  1356,  1180,  1358,   179,
     196,   550,   196,  1064,   179,    78,    79,    80,    50,   156,
     157,    53,  1327,   196,   194,   179,    70,   197,    91,   194,
    1171,   570,   179,   359,    70,  1342,   731,   156,   157,    70,
     179,  1182,   368,   197,   370,   197,   476,   194,   158,   375,
     105,   106,   107,   592,   593,   194,   105,   106,   107,   385,
     194,  1447,   119,   120,   121,   657,   761,   729,  1220,   110,
     111,   112,   502,   181,    50,    51,    52,    53,    54,    55,
     143,   144,   145,   146,   147,   194,   625,   626,    75,    76,
      70,   154,   158,    69,   194,   525,    69,   160,   161,   794,
      81,    53,    54,    55,  1881,    57,   536,   132,   133,   539,
     218,   174,   196,   197,  1875,   810,   806,    69,  1895,   227,
     196,   197,   103,   162,  1293,   188,   234,   350,   351,  1890,
    1767,  1768,   674,  1763,  1764,   196,  1486,  1200,  1488,    48,
    1490,   826,   827,  1493,   252,   179,  1532,    81,   256,  1200,
     158,   194,   194,   692,   201,  1307,     9,   819,   139,   140,
     141,   158,   824,   158,   855,   194,   857,     8,   196,   103,
     861,   862,   863,  1480,   194,   158,    14,   196,   159,   158,
     196,   162,   815,   197,   165,   166,   512,   168,   169,   170,
       9,   196,   130,   130,   119,   120,   121,   122,   123,   124,
     895,    14,   897,  1344,   899,   139,   140,   141,  1377,   195,
     179,    14,   193,   908,   102,   200,   194,   198,   195,   111,
     195,   195,     9,   195,  1393,   887,   974,   922,   194,   163,
     194,   165,   166,   155,   168,   169,   170,  1587,   926,   778,
     195,   780,   195,    94,   195,   911,   911,   911,   195,     9,
     196,   359,    14,   948,  1847,   179,     9,   197,   800,   193,
     368,   800,   370,   958,   189,   194,   961,   375,   963,   196,
     196,   196,   967,  1153,  1867,   814,   815,   385,   911,   194,
     197,    83,   195,  1876,   195,   195,   975,   132,   975,   196,
     194,     9,   195,   201,   926,    81,     9,    83,    84,   201,
     201,    70,   410,    32,   133,   178,   938,     9,   940,   158,
     852,   136,   195,   852,   800,   158,  1011,   103,    14,   911,
     859,   860,   111,   975,   191,  1466,     9,     9,  1018,  1498,
     119,   120,   121,   122,   123,   124,   180,   195,  1507,     9,
      14,   132,   934,   975,  1006,   201,   201,     9,   198,   888,
    1413,  1520,   194,   139,   140,   141,    14,   683,   195,   951,
     195,   201,  1413,   158,  1124,   201,   852,   195,   102,   911,
     196,     9,   911,   196,    91,  1063,   158,   163,  1066,   165,
     166,  1013,   168,   169,   170,   136,  1019,   926,     9,  1051,
     932,  1741,  1742,   932,   195,   194,  1058,    70,    70,   938,
     189,   940,    70,   158,   512,   197,   194,   193,   158,     9,
      14,   197,   197,   199,  1124,   198,  1107,   743,   196,   180,
       9,   197,    14,   197,   201,   911,    14,   195,  1597,  1124,
     196,    32,  1495,   191,    32,   974,    14,    27,    28,    14,
     194,    52,   550,    70,  1495,   194,   932,   986,   987,   988,
     194,   194,   194,    78,    79,    80,    81,    70,   784,   194,
     786,    70,     9,  1158,  1787,  1131,  1131,  1131,   119,   120,
     121,   122,   123,   124,  1013,   195,  1171,   196,   103,  1021,
    1019,  1023,  1021,   136,  1023,   196,   812,  1182,  1183,     4,
     194,    14,   180,   923,   136,     9,   926,    69,  1131,     9,
     195,   201,    83,     9,  1043,   198,   198,   196,   938,   194,
     940,    14,   136,   194,   139,   140,   141,   196,    83,  1209,
     195,   197,  1217,   194,  1063,   194,   136,  1066,   195,     9,
    1192,   197,  1227,   197,    49,  1021,   196,  1023,   189,  1131,
     165,   166,  1865,   168,   169,   170,   201,    91,   155,   875,
     197,    32,    77,   196,   195,    81,  1095,   196,   180,   136,
      32,   195,   195,     9,  1733,   891,   892,     9,   193,   201,
    1162,   201,   201,  1165,  1743,   683,   136,   103,  1901,  1212,
       9,   195,   198,  1013,  1246,     9,   198,   195,  1250,  1131,
     196,   196,  1131,  1255,   197,    14,    83,   112,   194,   196,
    1262,   195,   117,  1448,   119,   120,   121,   122,   123,   124,
     125,   195,   195,   139,   140,   141,   201,  1312,   197,  1788,
     194,     9,   195,   136,  1319,   195,     9,   136,  1323,   195,
    1325,     9,   201,   201,  1064,   743,   162,   227,  1333,   165,
     166,    32,   168,   169,   170,  1131,   196,   195,  1343,  1344,
     165,   166,  1443,   168,   195,  1194,  1195,  1196,  1197,   196,
     196,   112,  1201,   197,  1833,   167,   196,   193,   163,    81,
      14,   197,    83,  1212,   189,   195,   784,   117,   786,   195,
     197,   136,  1224,   198,   382,  1224,   136,   195,   386,    14,
     179,   103,   800,   197,    83,   196,    14,    14,    83,  1025,
    1026,  1363,  1553,   195,   812,  1367,   194,   815,   136,   195,
    1372,   136,   196,  1305,   412,    14,   414,   415,   416,   417,
     196,    14,   196,    14,  1316,     9,   197,   139,   140,   141,
       9,  1900,   198,    59,    83,   179,  1905,   194,  1224,     9,
      83,   197,   196,   115,   852,   102,   102,   180,   158,   170,
    1342,   163,    14,   165,   166,   167,   168,   169,   170,    36,
    1299,   195,   194,  1305,   194,   196,   180,   875,  1463,   359,
    1200,  1466,   176,   180,  1316,    83,   173,  1525,   368,   195,
       9,   193,   194,   891,   892,   375,    83,    27,    28,   197,
     196,    31,   195,   195,   193,   385,    14,    83,     9,  1125,
      14,    83,    14,   911,    14,    83,  1856,    83,   968,  1107,
     914,    56,   477,  1872,  1218,  1596,    56,  1356,   479,  1358,
    1867,    10,    11,    12,   932,   482,   596,  1390,  1583,  1533,
    1620,  1157,  1705,  1438,  1912,  1888,  1717,  1434,  1579,   379,
    1528,    30,    31,  1062,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,  1197,    57,  1135,
    1721,  1504,  1059,   987,  1193,  1008,  1571,  1469,   938,  1205,
      69,  1423,  1194,  1475,  1423,  1477,    81,   826,  1480,  1899,
     423,   376,  1914,  1432,  1220,  1221,  1822,  1115,  1426,  1438,
    1044,  1095,    -1,    -1,    -1,  1497,  1448,    -1,   103,    -1,
      -1,  1019,    -1,  1021,    -1,  1023,    -1,  1025,  1026,    -1,
      -1,    -1,   512,    -1,    -1,    -1,    -1,  1469,    -1,    -1,
      -1,    -1,    -1,  1475,    -1,  1477,    -1,  1423,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,  1486,    -1,  1488,
      -1,  1490,    -1,    -1,  1493,  1497,  1442,  1499,    -1,    -1,
    1499,    -1,  1595,  1596,    -1,  1504,  1508,    -1,    -1,  1508,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,  1716,
      -1,  1307,    -1,  1413,    -1,    -1,  1525,   227,    -1,  1528,
      -1,    -1,  1531,  1585,    -1,  1840,    -1,    -1,   193,   194,
      -1,    -1,  1541,    -1,    -1,    -1,    -1,    -1,    -1,  1548,
      -1,    -1,   201,  1499,    -1,    -1,  1555,  1125,  1557,    -1,
      -1,    -1,  1508,  1131,    -1,  1564,    -1,    -1,  1716,    -1,
      -1,  1726,   277,  1781,   279,    -1,    -1,   277,    -1,   279,
      -1,    -1,    -1,  1585,    -1,    -1,    -1,    -1,  1587,  1157,
      -1,    -1,  1594,    -1,    -1,  1594,  1595,  1596,  1600,    -1,
      -1,  1600,    -1,    -1,  1606,  1495,    -1,  1606,    -1,  1860,
      -1,    -1,    -1,    -1,    -1,  1561,    -1,    -1,    -1,    81,
     119,   120,   121,   122,   123,   124,    -1,    -1,    -1,   334,
      -1,   130,   131,   683,   334,  1757,  1758,  1205,    -1,    -1,
      81,   103,    83,    84,  1212,    -1,    -1,    -1,  1594,    -1,
      -1,    -1,  1220,  1221,  1600,    -1,  1224,    -1,    -1,   359,
    1606,    -1,   103,    -1,  1450,    -1,   128,    -1,   368,    -1,
      -1,    -1,   171,    -1,    -1,   375,    -1,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,   385,    -1,    -1,    -1,    -1,
     189,    -1,    -1,   743,    -1,    -1,   396,    -1,   139,   140,
     141,     4,    -1,   165,   166,    -1,   168,   169,   170,    -1,
      -1,    -1,    -1,    -1,    -1,   420,    -1,  1716,   423,    -1,
     420,    -1,   163,   423,   165,   166,    -1,   168,   169,   170,
      -1,   193,   194,    -1,   784,    -1,   786,    -1,  1737,  1307,
      -1,    -1,  1741,  1742,    -1,  1747,    49,    -1,  1747,    -1,
      -1,    -1,   193,    -1,    -1,  1910,   197,  1756,   199,    -1,
      -1,    -1,   812,    -1,  1763,  1764,    -1,  1922,  1767,  1768,
     470,    -1,    -1,    -1,    -1,    -1,    -1,  1932,    -1,    -1,
    1935,    -1,  1781,    -1,    -1,    -1,    -1,  1789,    -1,    81,
    1789,    -1,    -1,    -1,  1796,  1847,    -1,  1796,    -1,    -1,
      -1,  1747,    -1,    -1,    -1,    -1,    -1,     4,    -1,   112,
      -1,   103,   512,    -1,   117,  1867,   119,   120,   121,   122,
     123,   124,   125,    -1,  1876,   875,     4,    -1,    -1,  1831,
      -1,    -1,  1831,    -1,    -1,    -1,    -1,    -1,  1840,    -1,
    1839,   891,   892,  1789,    -1,    -1,    -1,   139,   140,   141,
    1796,  1853,    49,    -1,  1853,  1423,    -1,    -1,    -1,   564,
    1859,    -1,   165,   166,   564,   168,   566,    -1,    -1,   569,
      -1,    49,    -1,   165,   166,    -1,   168,   169,   170,    -1,
      -1,    -1,  1450,    -1,    -1,  1831,   189,     4,    -1,    50,
      51,    52,    53,    54,    55,   198,    57,    -1,    -1,    -1,
     600,   193,   194,    -1,    -1,    -1,    -1,  1853,    69,    -1,
      -1,  1913,    -1,    -1,  1913,   112,  1918,    -1,    -1,  1918,
     117,    -1,   119,   120,   121,   122,   123,   124,   125,    -1,
      -1,  1499,    49,    -1,   112,    -1,  1504,    -1,    -1,   117,
    1508,   119,   120,   121,   122,   123,   124,   125,   653,   654,
      -1,    -1,    -1,   653,   654,    -1,    -1,   662,    -1,    -1,
      -1,    -1,   662,    -1,    -1,    -1,    -1,  1913,   165,   166,
      -1,   168,  1918,    -1,     4,  1025,  1026,    -1,    -1,    -1,
      27,    28,    -1,   683,    -1,    -1,    -1,   165,   166,    -1,
     168,    -1,   189,    -1,    -1,   112,    -1,    -1,    -1,    -1,
     117,   198,   119,   120,   121,   122,   123,   124,   125,    -1,
      -1,   189,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,
     198,    -1,    -1,    -1,    -1,    -1,  1594,  1595,  1596,    -1,
      -1,    -1,  1600,    -1,    -1,    -1,    -1,    -1,  1606,    -1,
      -1,    -1,    -1,   743,    -1,    -1,    -1,    -1,   165,   166,
      -1,   168,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   189,    -1,    -1,  1125,    -1,    -1,    -1,    -1,
      -1,   198,   112,    -1,   784,    -1,   786,   117,    -1,   119,
     120,   121,   122,   123,   124,   125,    -1,    -1,    81,    -1,
      83,    -1,    85,    -1,    -1,    67,    68,  1157,    -1,    -1,
      -1,    -1,   812,   813,    -1,    -1,    -1,    -1,    -1,    -1,
     103,   826,   827,    -1,    -1,    -1,   826,   827,   828,   829,
     830,   831,   832,    -1,    -1,   165,   166,    -1,   168,   839,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,   853,    -1,  1205,   139,   140,   141,   189,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,
    1220,  1221,   134,   135,    -1,   875,    -1,    -1,    -1,  1747,
     227,    -1,   165,   166,    -1,   168,   169,   170,    -1,   889,
      -1,   891,   892,    67,    68,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,   913,    -1,
     193,    -1,    -1,   913,   914,    56,    -1,    -1,    -1,    -1,
      -1,  1789,    -1,   923,   929,    -1,    -1,    -1,  1796,   929,
      -1,    -1,    -1,   195,    -1,    -1,    -1,   942,    -1,    -1,
      -1,    -1,   942,    -1,    -1,    -1,    -1,    -1,    67,    68,
     950,    -1,    -1,   953,    -1,    -1,    -1,  1307,    -1,    -1,
     134,   135,    -1,  1831,    -1,    -1,   971,    -1,    -1,    -1,
      -1,   971,    -1,    -1,    -1,   975,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1853,    -1,    -1,     4,    -1,
      -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      -1,    -1,   359,    -1,    -1,   134,   135,    -1,    -1,    -1,
      -1,   368,    -1,    -1,    -1,  1025,  1026,    -1,   375,    -1,
      -1,    -1,    -1,    49,  1039,    -1,    -1,  1042,   385,  1039,
      -1,    -1,  1042,    -1,  1044,  1913,    -1,    67,    68,   396,
    1918,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,  1059,
    1060,  1061,  1062,    -1,  1064,    -1,    -1,  1067,  1068,  1069,
    1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,  1093,  1094,    -1,   112,    -1,    -1,    -1,
    1450,   117,    81,   119,   120,   121,   122,   123,   124,   125,
      -1,  1111,    -1,    -1,   134,   135,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,  1125,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   277,    -1,   279,    -1,
    1145,    -1,  1147,    -1,    -1,  1145,   125,  1147,    -1,   165,
     166,    -1,   168,    -1,    -1,    -1,    -1,  1157,    -1,   138,
     139,   140,   141,   142,    -1,   512,    81,    -1,    -1,  1174,
      -1,    -1,  1177,   189,  1174,   195,    81,  1177,    -1,    -1,
     159,    -1,   198,   162,   163,    -1,   165,   166,   103,   168,
     169,   170,    -1,   334,    -1,    -1,   111,   112,   103,    -1,
    1200,    -1,    -1,    -1,    -1,  1205,   111,   112,    -1,    -1,
      -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,    -1,
    1220,  1221,    -1,  1223,   139,   140,   141,    -1,  1233,    -1,
      -1,    -1,  1237,  1233,   139,   140,   141,  1237,    -1,    -1,
    1240,    -1,  1242,    -1,    -1,    -1,    -1,   162,    -1,    -1,
     165,   166,    -1,   168,   169,   170,    -1,   162,    -1,    -1,
     165,   166,    -1,   168,   169,   170,  1266,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,   420,
      -1,    -1,   423,    -1,    -1,    10,    11,    12,   193,    -1,
      -1,    -1,    -1,    -1,    -1,  1300,  1301,    -1,    -1,    -1,
    1300,  1301,    -1,    -1,  1304,    30,    31,  1307,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,   683,    -1,    -1,    10,
      11,    12,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,  1397,    -1,  1399,    -1,   743,  1397,    69,  1399,
      -1,    -1,    -1,    -1,    -1,  1405,    -1,  1407,    -1,  1409,
      -1,    -1,  1412,  1413,    -1,    -1,  1416,    -1,  1418,    -1,
      -1,  1421,    -1,   564,    -1,   566,    -1,    -1,   569,    -1,
      -1,    81,  1432,  1433,    -1,    -1,  1436,   784,  1443,   786,
      -1,    -1,    -1,  1443,    -1,    -1,    -1,    -1,    -1,    -1,
    1450,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,   600,
      -1,    -1,    -1,    -1,    -1,   812,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   198,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,   139,
     140,   141,    -1,    -1,    -1,  1495,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1511,   653,   654,    -1,   165,   166,    -1,   168,   169,
     170,   662,    -1,  1523,  1524,   103,    -1,   198,   875,    67,
      68,  1531,    -1,  1533,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   193,   891,   892,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1555,    -1,  1557,    -1,    -1,
      -1,   139,   140,   141,  1564,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   923,    -1,    -1,    -1,
      -1,   159,    -1,    -1,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   134,   135,    -1,    -1,
      -1,    -1,    -1,  1603,  1604,  1605,    -1,    10,    11,    12,
    1610,    -1,  1612,  1618,    -1,   193,    -1,    -1,  1618,    -1,
    1620,    -1,    -1,    -1,    -1,    27,    28,    30,    31,    31,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,   195,   119,   120,
     121,   122,   123,   124,    -1,    -1,    69,    -1,    -1,   130,
     131,    -1,   813,    -1,    -1,    -1,    -1,    -1,  1025,  1026,
      -1,    -1,    -1,    -1,    -1,   826,   827,   828,   829,   830,
     831,   832,    -1,    -1,    -1,    -1,    -1,    -1,   839,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,   169,    81,
     171,    -1,    -1,    -1,    -1,  1720,    -1,  1064,    -1,    -1,
    1720,    -1,    -1,   184,    -1,   186,    -1,    -1,   189,    -1,
      -1,   103,    59,    -1,    -1,    -1,    -1,  1737,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,   125,    81,    -1,  1756,    -1,    -1,    -1,
      -1,    -1,  1762,    -1,    69,    -1,   138,   139,   140,   141,
     142,    -1,   913,  1773,    -1,    -1,   103,    -1,  1125,  1779,
      -1,    -1,    -1,  1783,   111,    -1,    -1,   159,   929,    -1,
     162,   163,    -1,   165,   166,   198,   168,   169,   170,    -1,
    1805,   942,    -1,    -1,    -1,  1805,    -1,    -1,    -1,   950,
    1157,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,   193,    -1,    -1,    -1,   227,    -1,    -1,    -1,    -1,
     971,    -1,   159,    -1,    -1,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,    -1,  1845,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1200,    -1,  1855,   183,    -1,  1205,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   193,   194,    -1,    -1,
      -1,    -1,  1872,  1220,  1221,    -1,  1881,    -1,    -1,    -1,
      -1,  1881,    -1,    -1,    -1,    -1,    -1,    -1,  1893,    -1,
    1895,    -1,    -1,  1893,    -1,  1895,    -1,    -1,  1039,    -1,
      -1,  1042,    -1,  1044,    -1,    -1,    -1,    -1,    -1,  1914,
      -1,  1916,    -1,    -1,  1914,    -1,  1916,    -1,  1059,  1060,
    1061,  1062,    -1,    -1,    -1,    -1,  1067,  1068,  1069,  1070,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,  1093,  1094,    -1,    -1,    -1,   359,    -1,    -1,
    1307,    -1,    -1,    -1,    -1,    -1,   368,    -1,    -1,    -1,
    1111,    -1,    -1,   375,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   385,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,   396,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1145,    -1,  1147,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,  1174,    57,    81,  1177,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1413,    -1,   470,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1223,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1233,   139,   140,   141,  1237,    -1,    -1,  1240,
      -1,  1242,    -1,  1450,    -1,    -1,    -1,   139,   140,   141,
     512,    -1,    -1,    -1,    -1,    -1,   162,    -1,    -1,   165,
     166,    -1,   168,   169,   170,  1266,    -1,    -1,    -1,    -1,
     162,    -1,    -1,   165,   166,    -1,   168,   169,   170,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,  1495,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1300,
    1301,   193,    -1,    -1,    -1,    -1,    -1,   569,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   198,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   600,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    59,    -1,    -1,    -1,  1397,    -1,  1399,    -1,
      -1,    -1,    -1,    69,  1405,    -1,  1407,    -1,  1409,    -1,
      -1,  1412,    -1,    -1,    81,  1416,    -1,  1418,    -1,    -1,
    1421,   683,    -1,    10,    11,    12,    27,    28,    -1,    -1,
      31,  1432,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,  1443,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,   743,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   159,    -1,    -1,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,   198,   172,    -1,    -1,    -1,    -1,
    1511,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,
      -1,    -1,   784,    -1,   786,    -1,   193,   194,    -1,    -1,
    1531,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    31,    57,
     812,   813,    -1,    -1,  1555,    -1,  1557,    -1,    -1,    -1,
      -1,    69,    -1,  1564,    -1,    -1,   828,   829,   830,   831,
     832,    -1,    -1,    -1,    -1,    -1,    59,   839,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   853,    -1,    -1,    31,    -1,    -1,    -1,    81,    -1,
      -1,    -1,  1603,  1604,  1605,    -1,    -1,    -1,    -1,  1610,
      -1,   198,    -1,   875,    -1,    -1,    -1,  1618,    -1,    -1,
     103,    -1,    59,    -1,    -1,    -1,   227,   889,   111,   891,
     892,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   914,    -1,    -1,   138,   139,   140,   141,   142,
      -1,   923,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,   950,    -1,
      -1,   953,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     183,   138,   139,   140,   141,   142,   189,    -1,    -1,    -1,
     193,   194,    -1,   975,    -1,    -1,    -1,    -1,    -1,  1720,
      -1,    -1,   159,    -1,    -1,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,    -1,   172,  1737,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1756,   193,   194,   359,    -1,
      -1,  1762,    -1,  1025,  1026,    -1,    -1,   368,    -1,    -1,
      -1,    -1,  1773,    -1,   375,    -1,    -1,    -1,  1779,    -1,
      -1,    -1,  1783,    -1,   385,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   396,    -1,  1059,  1060,  1061,
    1062,    -1,  1064,    -1,  1805,  1067,  1068,  1069,  1070,  1071,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,  1093,  1094,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1845,    -1,    -1,    -1,    -1,  1111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1125,    -1,    -1,    -1,    -1,    -1,   470,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   569,
    1881,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1893,    -1,  1895,  1157,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     600,   512,    -1,  1914,    -1,  1916,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,    -1,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1200,    -1,
      -1,    -1,    27,  1205,    29,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1220,  1221,
      -1,  1223,    -1,    -1,    -1,    -1,    -1,    -1,   569,    -1,
      -1,    -1,    57,    -1,    59,    -1,    -1,    -1,  1240,    -1,
    1242,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,   600,
      -1,    -1,    -1,    -1,  1266,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,  1304,    -1,    -1,  1307,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   159,    50,    51,   162,   163,    -1,
     165,   166,   683,   168,   169,   170,   171,    -1,   173,    -1,
      -1,   176,    -1,    -1,    -1,    70,    -1,    -1,   183,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,   193,   194,
      -1,    -1,    -1,   198,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,   813,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   828,   829,
     830,   831,   743,  1405,    -1,  1407,    -1,  1409,    -1,   839,
    1412,  1413,    -1,    -1,  1416,    -1,  1418,    -1,    -1,  1421,
      -1,    -1,    -1,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,  1433,    -1,    -1,  1436,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,   784,    -1,   786,    -1,    -1,  1450,    -1,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,    -1,
      -1,   812,   813,    -1,    -1,    -1,    -1,    -1,   193,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   828,   829,   830,
     831,   832,    -1,  1495,    -1,    -1,    -1,    -1,   839,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1511,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     950,  1523,  1524,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1533,    -1,    -1,   875,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     891,   892,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   923,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,  1603,  1604,  1605,    -1,    -1,    -1,    -1,  1610,   950,
    1612,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1620,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1060,  1061,  1062,    -1,   975,    -1,    -1,  1067,  1068,  1069,
    1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,  1093,  1094,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1111,    -1,    -1,  1025,  1026,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,  1059,  1060,
    1061,  1062,    -1,  1064,   198,    69,  1067,  1068,  1069,  1070,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,  1093,  1094,    -1,    -1,    -1,    -1,    -1,    -1,
    1762,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1111,  1773,    -1,    -1,    -1,    -1,    -1,  1779,    -1,    -1,
      -1,  1783,    -1,    -1,  1125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1223,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1240,    -1,  1242,    -1,    -1,    -1,  1157,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,  1266,    57,    -1,    -1,
      -1,    -1,    -1,  1845,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,  1855,   198,    -1,    -1,    -1,    -1,  1200,
      -1,    -1,    -1,    -1,  1205,    -1,    -1,    -1,    -1,    -1,
    1872,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1220,
    1221,    -1,  1223,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1240,
      -1,  1242,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1266,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1307,    -1,    69,    10,
      11,    12,    -1,    -1,    -1,  1405,    -1,  1407,    -1,  1409,
      -1,    -1,  1412,    -1,    -1,    -1,  1416,    -1,  1418,    30,
      31,  1421,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,    69,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1405,    -1,  1407,    -1,  1409,    -1,
      -1,  1412,  1413,    -1,    -1,  1416,    57,  1418,    -1,    -1,
    1421,  1511,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,   198,    -1,    -1,
      81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,  1450,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,    -1,    -1,   130,
     131,   132,   133,    -1,  1495,   196,   137,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1511,    -1,    -1,  1603,  1604,  1605,    -1,    -1,   159,    -1,
    1610,    -1,    -1,    -1,   165,   166,    -1,   168,   169,   170,
     171,    -1,   173,    -1,    -1,   176,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   193,    -1,    -1,    -1,   197,    -1,   199,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    10,
      11,    12,  1603,  1604,  1605,    -1,    -1,    -1,    -1,  1610,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1619,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1762,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,  1773,    -1,    -1,    -1,    -1,    -1,  1779,
      -1,    -1,    -1,  1783,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,   198,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,  1845,    -1,    -1,    -1,    -1,
      -1,  1762,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,  1773,    -1,    -1,    -1,    -1,    -1,  1779,    -1,
      -1,    -1,  1783,    -1,    -1,   196,    49,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,  1807,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,   104,    -1,    -1,  1845,   108,   109,   110,   111,   112,
     113,   114,    -1,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   196,   126,   127,   128,   129,   130,   131,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,
     143,   144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,
      -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,   171,    -1,
      -1,   174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,
     183,   184,    -1,   186,    -1,   188,   189,   190,    -1,    -1,
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
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    95,
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
     196,   197,    -1,   199,   200,    -1,   202,   203,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,    94,
      -1,    96,    -1,    98,    -1,   100,   101,    -1,   103,   104,
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
      92,    93,    94,    -1,    96,    -1,    98,    99,   100,    -1,
      -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
      -1,   113,   114,    -1,   116,    -1,   118,   119,   120,   121,
     122,   123,   124,    -1,   126,   127,   128,    -1,   130,   131,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,   151,
      -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,   171,
      -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,    -1,
      -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,    -1,
      -1,   193,   194,    -1,   196,   197,    -1,   199,   200,    -1,
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
      -1,    91,    92,    93,    94,    -1,    96,    97,    98,    -1,
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
      -1,   196,   197,    -1,   199,   200,    -1,   202,   203,     3,
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
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     188,   189,   190,    -1,    -1,   193,   194,    -1,    -1,    -1,
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
      50,    51,    52,    53,    54,    55,    32,    57,    -1,    -1,
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
      13,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    50,    51,    -1,
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
      -1,    13,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    50,    51,
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
      -1,    -1,    13,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    50,
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
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    38,    -1,
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
     195,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,   188,
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
      29,   199,   200,    -1,   202,   203,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,   197,    -1,
     199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
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
     125,   126,    -1,    -1,    -1,   130,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   159,    -1,    -1,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,   171,    -1,   173,    -1,
      -1,   176,     3,     4,    -1,     6,     7,    -1,   183,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,   193,   194,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,
      31,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    57,    -1,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,
      -1,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
     171,    -1,   173,    -1,    -1,   176,    -1,     3,     4,    -1,
       6,     7,   183,   184,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,   193,   194,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    29,    -1,    31,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    59,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,    -1,    -1,    -1,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   159,    -1,    -1,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,   171,    -1,   173,    -1,    -1,
     176,     3,     4,     5,     6,     7,    -1,   183,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,   193,   194,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    57,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,   160,   161,
      -1,    -1,    -1,   165,   166,    -1,   168,   169,   170,   171,
      -1,   173,   174,    -1,   176,    -1,    -1,    -1,    -1,    -1,
      -1,   183,   184,    -1,   186,    -1,   188,   189,    -1,     3,
       4,   193,     6,     7,    11,    12,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    31,    29,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,    -1,    -1,   130,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
      -1,    10,    11,    12,    13,   159,    -1,    -1,    -1,    -1,
      -1,   165,   166,    -1,   168,   169,   170,   171,    27,   173,
      29,    -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,    -1,    -1,    10,    11,    12,    13,
     159,    -1,    -1,    -1,    -1,    -1,   165,   166,    -1,   168,
     169,   170,   171,    27,   173,    29,    -1,   176,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,    -1,    -1,   130,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
      -1,    10,    11,    12,    13,   159,    -1,    -1,    -1,    -1,
      -1,   165,   166,    -1,   168,   169,   170,   171,    27,   173,
      29,    -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     159,    -1,    -1,    -1,    -1,    -1,   165,   166,    -1,   168,
     169,   170,   171,    -1,   173,    -1,    -1,   176,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    31,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,    -1,   103,    -1,    -1,
      -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   159,    -1,    -1,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   183,   143,   144,   145,
     146,   147,    -1,    -1,    -1,    -1,   193,   194,   154,    38,
      -1,   195,    -1,   159,   160,   161,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,    -1,    -1,    -1,   174,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,
      -1,    70,   188,    -1,    -1,    -1,    -1,   193,   194,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,    -1,   143,   144,   145,   146,   147,    -1,
      -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,
     159,   160,   161,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,    -1,    50,    51,   174,    -1,    -1,    -1,    56,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   188,
      -1,    -1,    -1,    70,   193,   194,    -1,    -1,    -1,    -1,
     199,    78,    79,    80,    81,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    31,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,    69,   143,   144,   145,   146,
     147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,    70,    -1,    -1,   174,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,   183,    83,    84,    -1,
      -1,   188,    -1,    -1,    -1,    91,   193,   194,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,    -1,   143,   144,   145,
     146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,
      -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,    70,    -1,    -1,   174,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,   188,    -1,    -1,    -1,    91,   193,   194,    -1,
      -1,   197,    -1,   199,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    70,    -1,    72,   174,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,   188,    -1,    -1,    -1,    91,   193,   194,
      -1,    -1,    -1,    -1,   199,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    70,    -1,    -1,
     174,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,   188,    -1,    -1,    -1,    91,   193,
     194,    -1,    -1,    -1,    -1,   199,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,
     143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,
      -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,    70,    -1,
      -1,   174,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,   188,    -1,    -1,    -1,    91,
     193,   194,    -1,    -1,    -1,    -1,   199,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,
      -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,    -1,
      -1,    -1,   174,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,
      -1,   193,   194,    -1,    -1,    30,    31,   199,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,   136,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   136,
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
      49,    50,    51,    52,    53,    54,    55,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      69,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,
     130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,    -1,   143,   144,   145,   146,   147,    -1,    -1,
      -1,    -1,    -1,    -1,   154,    -1,    -1,   136,    -1,   159,
     160,   161,   162,   163,    -1,   165,   166,    -1,   168,   169,
     170,    -1,    -1,    -1,   174,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,   188,    91,
      -1,    -1,    -1,   193,   194,    -1,    -1,    -1,    -1,   199,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
      -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,    -1,
      -1,    -1,   174,    -1,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,    -1,    -1,    -1,   188,    91,    -1,    -1,
      -1,   193,   194,    -1,    -1,    -1,    -1,   199,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,
     174,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,   193,
     194,    -1,    -1,    -1,    -1,   199,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    32,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    12,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69
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
     453,   455,   468,   470,   472,   122,   123,   124,   137,   159,
     169,   194,   211,   244,   325,   346,   444,   346,   194,   346,
     346,   346,   108,   346,   346,   346,   430,   431,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
      81,    83,    91,   124,   139,   140,   141,   154,   194,   222,
     365,   400,   403,   408,   444,   447,   444,    38,   346,   459,
     460,   346,   124,   130,   194,   222,   257,   400,   401,   402,
     404,   408,   441,   442,   443,   451,   456,   457,   194,   335,
     405,   194,   335,   356,   336,   346,   230,   335,   194,   194,
     194,   335,   196,   346,   211,   196,   346,     3,     4,     6,
       7,    10,    11,    12,    13,    27,    29,    31,    57,    59,
      71,    72,    73,    74,    75,    76,    77,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   130,   131,   132,   133,   137,   138,   142,   159,   163,
     171,   173,   176,   183,   194,   211,   212,   213,   224,   473,
     493,   494,   497,   196,   341,   343,   346,   197,   237,   346,
     111,   112,   162,   214,   215,   216,   217,   221,    83,   199,
     291,   292,   123,   130,   122,   130,    83,   293,   194,   194,
     194,   194,   211,   263,   476,   194,   194,    70,    70,    70,
     336,    83,    90,   155,   156,   157,   465,   466,   162,   197,
     221,   221,   211,   264,   476,   163,   194,   476,   476,    83,
     190,   197,   357,    27,   334,   338,   346,   347,   444,   448,
     226,   197,    90,   406,   465,    90,   465,   465,    32,   162,
     179,   477,   194,     9,   196,    38,   243,   163,   262,   476,
     124,   189,   244,   326,   196,   196,   196,   196,   196,   196,
     196,   196,    10,    11,    12,    30,    31,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      57,    69,   196,    70,    70,   197,   158,   131,   169,   171,
     184,   186,   265,   324,   325,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    67,    68,
     134,   135,   434,    70,   197,   439,   194,   194,    70,   197,
     199,   452,   194,   243,   244,    14,   346,   196,   136,    48,
     211,   429,    90,   334,   347,   158,   444,   136,   201,     9,
     414,   258,   334,   347,   444,   477,   158,   194,   407,   434,
     439,   195,   346,    32,   228,     8,   358,     9,   196,   228,
     229,   336,   337,   346,   211,   277,   232,   196,   196,   196,
     138,   142,   497,   497,   179,   496,   194,   111,   497,    14,
     158,   138,   142,   159,   211,   213,   196,   196,   196,   238,
     115,   176,   196,   214,   216,   214,   216,   221,   197,     9,
     415,   196,   102,   162,   197,   444,     9,   196,   130,   130,
      14,     9,   196,   444,   469,   336,   334,   347,   444,   447,
     448,   195,   179,   255,   137,   444,   458,   459,   346,   366,
     367,   336,   381,   381,   196,    70,   434,   155,   466,    82,
     346,   444,    90,   155,   466,   221,   210,   196,   197,   250,
     260,   390,   392,    91,   194,   199,   359,   360,   362,   403,
     450,   452,   470,    14,   102,   471,   353,   354,   355,   287,
     288,   432,   433,   195,   195,   195,   195,   195,   198,   227,
     228,   245,   252,   259,   432,   346,   200,   202,   203,   211,
     478,   479,   497,    38,   172,   289,   290,   346,   473,   194,
     476,   253,   243,   346,   346,   346,   346,    32,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   404,   346,   346,   454,   454,   346,   461,   462,
     130,   197,   212,   213,   451,   452,   263,   211,   264,   476,
     476,   262,   244,    38,   338,   341,   343,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     163,   197,   211,   435,   436,   437,   438,   451,   454,   346,
     289,   289,   454,   346,   458,   243,   195,   346,   194,   428,
       9,   414,   195,   195,    38,   346,    38,   346,   407,   195,
     195,   195,   451,   289,   197,   211,   435,   436,   451,   195,
     226,   281,   197,   343,   346,   346,    94,    32,   228,   275,
     196,    28,   102,    14,     9,   195,    32,   197,   278,   497,
      31,    91,   224,   490,   491,   492,   194,     9,    50,    51,
      56,    58,    70,   138,   139,   140,   141,   163,   183,   194,
     222,   224,   373,   376,   379,   385,   400,   408,   409,   411,
     211,   495,   226,   194,   236,   197,   196,   197,   196,   102,
     162,   111,   112,   162,   217,   218,   219,   220,   221,   217,
     211,   346,   292,   409,    83,     9,   195,   195,   195,   195,
     195,   195,   195,   196,    50,    51,   486,   488,   489,   132,
     268,   194,     9,   195,   195,   136,   201,     9,   414,     9,
     414,   201,   201,    83,    85,   211,   467,   211,    70,   198,
     198,   207,   209,    32,   133,   267,   178,    54,   163,   178,
     394,   347,   136,     9,   414,   195,   158,   497,   497,    14,
     358,   287,   226,   191,     9,   415,   497,   498,   434,   439,
     434,   198,     9,   414,   180,   444,   346,   195,     9,   415,
      14,   350,   246,   132,   266,   194,   476,   346,    32,   201,
     201,   136,   198,     9,   414,   346,   477,   194,   256,   251,
     261,    14,   471,   254,   243,    72,   444,   346,   477,   201,
     198,   195,   195,   201,   198,   195,    50,    51,    70,    78,
      79,    80,    91,   138,   139,   140,   141,   154,   183,   211,
     374,   377,   380,   417,   419,   420,   424,   427,   211,   444,
     444,   136,   266,   434,   439,   195,   346,   282,    75,    76,
     283,   226,   335,   226,   337,   102,    38,   137,   272,   444,
     409,   211,    32,   228,   276,   196,   279,   196,   279,     9,
     414,    91,   136,   158,     9,   414,   195,   172,   478,   479,
     480,   478,   409,   409,   409,   409,   409,   413,   416,   194,
      70,    70,    70,   158,   194,   409,   158,   197,    10,    11,
      12,    31,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    69,   158,   477,   198,   400,   197,
     240,   216,   216,   211,   217,   217,   221,     9,   415,   198,
     198,    14,   444,   196,   180,     9,   414,   211,   269,   400,
     197,   458,   137,   444,    14,    38,   346,   346,   201,   346,
     198,   207,   497,   269,   197,   393,    14,   195,   346,   359,
     451,   196,   497,   191,   198,    32,   484,   433,    38,    83,
     172,   435,   436,   438,   435,   436,   497,    38,   172,   346,
     409,   287,   194,   400,   267,   351,   247,   346,   346,   346,
     198,   194,   289,   268,    32,   267,   497,    14,   266,   476,
     404,   198,   194,    14,    78,    79,    80,   211,   418,   418,
     420,   422,   423,    52,   194,    70,    70,    70,    90,   155,
     194,     9,   414,   195,   428,    38,   346,   267,   198,    75,
      76,   284,   335,   228,   198,   196,    95,   196,   272,   444,
     194,   136,   271,    14,   226,   279,   105,   106,   107,   279,
     198,   497,   180,   136,   497,   211,   490,     9,   195,   414,
     136,   201,     9,   414,   413,   368,   369,   409,   382,   409,
     410,   382,   130,   212,   359,   361,   363,   195,   130,   212,
     409,   463,   464,   409,   409,   409,    32,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   495,    83,   241,   198,   198,   220,   196,   409,   489,
     102,   103,   485,   487,     9,   297,   195,   194,   338,   343,
     346,   444,   136,   201,   198,   471,   297,   164,   177,   197,
     389,   396,   164,   197,   395,   136,   196,   484,   497,   358,
     498,    83,   172,    14,    83,   477,   444,   346,   195,   287,
     197,   287,   194,   136,   194,   289,   195,   197,   497,   197,
     196,   497,   267,   248,   407,   289,   136,   201,     9,   414,
     419,   422,   370,   371,   420,   383,   420,   421,   383,   155,
     359,   425,   426,   420,   444,   197,   335,    32,    77,   228,
     196,   337,   271,   458,   272,   195,   409,   101,   105,   196,
     346,    32,   196,   280,   198,   180,   497,   136,   172,    32,
     195,   409,   409,   195,   201,     9,   414,   136,   201,     9,
     414,   201,   136,     9,   414,   195,   136,   198,     9,   414,
     409,    32,   195,   226,   196,   196,   211,   497,   497,   485,
     400,     4,   112,   117,   123,   125,   165,   166,   168,   198,
     298,   323,   324,   325,   330,   331,   332,   333,   432,   458,
      38,   346,   198,   197,   198,    54,   346,   346,   346,   358,
      38,    83,   172,    14,    83,   346,   194,   484,   195,   297,
     195,   287,   346,   289,   195,   297,   471,   297,   196,   197,
     194,   195,   420,   420,   195,   201,     9,   414,   136,   201,
       9,   414,   201,   136,   195,     9,   414,   297,    32,   226,
     196,   195,   195,   195,   233,   196,   196,   280,   226,   497,
     497,   136,   409,   409,   409,   409,   359,   409,   409,   409,
     197,   198,   487,   132,   133,   184,   212,   474,   497,   270,
     400,   112,   333,    31,   125,   138,   142,   163,   169,   307,
     308,   309,   310,   400,   167,   315,   316,   128,   194,   211,
     317,   318,   299,   244,   497,     9,   196,     9,   196,   196,
     471,   324,   195,   444,   294,   163,   391,   198,   198,    83,
     172,    14,    83,   346,   289,   117,   348,   484,   198,   484,
     195,   195,   198,   197,   198,   297,   287,   136,   420,   420,
     420,   420,   359,   198,   226,   231,   234,    32,   228,   274,
     226,   195,   409,   136,   136,   136,   226,   400,   400,   476,
      14,   212,     9,   196,   197,   474,   471,   310,   179,   197,
       9,   196,     3,     4,     5,     6,     7,    10,    11,    12,
      13,    27,    28,    29,    57,    71,    72,    73,    74,    75,
      76,    77,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   137,   138,   143,   144,   145,   146,   147,
     159,   160,   161,   171,   173,   174,   176,   183,   184,   186,
     188,   189,   211,   397,   398,     9,   196,   163,   167,   211,
     318,   319,   320,   196,    83,   329,   243,   300,   474,   474,
      14,   244,   198,   295,   296,   474,    14,    83,   346,   195,
     194,   484,   196,   197,   321,   348,   484,   294,   198,   195,
     420,   136,   136,    32,   228,   273,   274,   226,   409,   409,
     409,   198,   196,   196,   409,   400,   303,   497,   311,   312,
     408,   308,    14,    32,    51,   313,   316,     9,    36,   195,
      31,    50,    53,    14,     9,   196,   213,   475,   329,    14,
     497,   243,   196,    14,   346,    38,    83,   388,   197,   226,
     484,   321,   198,   484,   420,   420,   226,    99,   239,   198,
     211,   224,   304,   305,   306,     9,   414,     9,   414,   198,
     409,   398,   398,    59,   314,   319,   319,    31,    50,    53,
     409,    83,   179,   194,   196,   409,   476,   409,    83,     9,
     415,   226,   198,   197,   321,    97,   196,   115,   235,   158,
     102,   497,   180,   408,   170,    14,   486,   301,   194,    38,
      83,   195,   198,   226,   196,   194,   176,   242,   211,   324,
     325,   180,   409,   180,   285,   286,   433,   302,    83,   198,
     400,   240,   173,   211,   196,   195,     9,   415,   119,   120,
     121,   327,   328,   285,    83,   270,   196,   484,   433,   498,
     195,   195,   196,   193,   481,   327,    38,    83,   172,   484,
     197,   482,   483,   497,   196,   197,   322,   498,    83,   172,
      14,    83,   481,   226,     9,   415,    14,   485,   226,    38,
      83,   172,    14,    83,   346,   322,   198,   483,   497,   198,
      83,   172,    14,    83,   346,    14,    83,   346,   346
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

  case 101:

/* Line 1455 of yacc.c  */
#line 891 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 893 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 898 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 899 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 905 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 909 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 910 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 912 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 914 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 920 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(1) - (1)]),
                                           &Parser::useClass);;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useFunction);;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useConst);;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 939 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 941 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 944 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 946 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 947 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 952 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 967 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 970 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 976 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 986 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 990 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 995 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 998 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1009 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1011 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1016 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1019 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1020 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1021 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1022 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1023 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1024 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1025 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1026 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1027 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1028 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1029 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1030 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1031 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1032 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1033 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1037 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1039 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1044 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1046 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1050 "hphp.y"
    { _p->onDeclare((yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                         (yyval) = (yyvsp[(3) - (5)]);
                                         (yyval) = T_DECLARE;;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1059 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1060 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1063 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1064 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1065 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1066 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1070 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
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
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1077 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1078 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1079 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1080 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1081 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1089 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1090 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1099 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1100 "hphp.y"
    { (yyval).reset();;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1104 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1106 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1112 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1113 "hphp.y"
    { (yyval).reset();;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1117 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1118 "hphp.y"
    { (yyval).reset();;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1122 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1128 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1134 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1141 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1147 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1154 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1160 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1168 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1172 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1176 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1180 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1186 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1189 "hphp.y"
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

  case 203:

/* Line 1455 of yacc.c  */
#line 1204 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1207 "hphp.y"
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

  case 205:

/* Line 1455 of yacc.c  */
#line 1221 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1224 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1229 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1232 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1238 "hphp.y"
    { _p->onClassExpressionStart(); ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { _p->onClassExpression((yyval), (yyvsp[(3) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(5) - (8)]), (yyvsp[(7) - (8)])); ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1245 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1248 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1256 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1259 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1267 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1275 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1278 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1279 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1280 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1283 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1284 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1288 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1289 "hphp.y"
    { (yyval).reset();;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1292 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1293 "hphp.y"
    { (yyval).reset();;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1297 "hphp.y"
    { (yyval).reset();;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1300 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1302 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1305 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1307 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1311 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1312 "hphp.y"
    { (yyval).reset();;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1316 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1317 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1321 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1323 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1326 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1328 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1333 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1336 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1338 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (4)]));;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1342 "hphp.y"
    {_p->onDeclareList((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1344 "hphp.y"
    {_p->onDeclareList((yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                           (yyval) = (yyvsp[(1) - (5)]);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1349 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1350 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1351 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1352 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1357 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1359 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1360 "hphp.y"
    { (yyval).reset();;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1363 "hphp.y"
    { (yyval).reset();;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1364 "hphp.y"
    { (yyval).reset();;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1369 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1370 "hphp.y"
    { (yyval).reset();;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1375 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1376 "hphp.y"
    { (yyval).reset();;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1379 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1380 "hphp.y"
    { (yyval).reset();;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1383 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { (yyval).reset();;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1392 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1398 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1404 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1408 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1417 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1422 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1425 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1431 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1435 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1445 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1450 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1455 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1461 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1467 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1475 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1480 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1485 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1489 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1492 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1500 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1503 "hphp.y"
    { (yyval).reset();;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1508 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1511 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1515 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { (yyval).reset();;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { (yyval).reset();;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1593 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1594 "hphp.y"
    { (yyval).reset();;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1619 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (11)]),(yyvsp[(9) - (11)]),(yyvsp[(3) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(7) - (11)]),(yyvsp[(11) - (11)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1627 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1634 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (12)]),(yyvsp[(10) - (12)]),(yyvsp[(4) - (12)]),(yyvsp[(5) - (12)]),(yyvsp[(8) - (12)]),(yyvsp[(12) - (12)]),&(yyvsp[(1) - (12)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1651 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1654 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1655 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1662 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1667 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1670 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1683 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1695 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { (yyval).reset();;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { (yyval).reset();;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval).reset();;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { (yyval).reset();;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { (yyval).reset();;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1867 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { (yyval).reset();;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1888 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { _p->onYieldFrom((yyval),&(yyvsp[(2) - (2)]));;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 1932 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PIPE);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1975 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1983 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1988 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1989 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1991 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1992 "hphp.y"
    { _p->onNullCoalesce((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1994 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1996 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1997 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1998 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2000 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2003 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
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
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2011 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2018 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { (yyval).reset();;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2024 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2030 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[(2) - (12)]),(yyvsp[(5) - (12)]),(yyvsp[(8) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(7) - (12)]),&(yyvsp[(9) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2038 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2044 "hphp.y"
    { _p->finishStatement((yyvsp[(12) - (13)]), (yyvsp[(12) - (13)])); (yyvsp[(12) - (13)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[(1) - (13)]),
                                           (yyvsp[(3) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(12) - (13)]),(yyvsp[(8) - (13)]),&(yyvsp[(10) - (13)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2054 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2062 "hphp.y"
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

  case 530:

/* Line 1455 of yacc.c  */
#line 2072 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2080 "hphp.y"
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

  case 532:

/* Line 1455 of yacc.c  */
#line 2090 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
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

  case 534:

/* Line 1455 of yacc.c  */
#line 2107 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2140 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2141 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2156 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2185 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2190 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2195 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { (yyval).reset();;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2205 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval).reset();;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval).reset();;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
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
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
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
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
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
#line 2270 "hphp.y"
    { _p->onKeyset((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval).reset();;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval).reset();;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval).reset();;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
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

  case 602:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
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

  case 603:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { (yyval).reset();;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { (yyval).reset();;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 2490 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { (yyval).reset();;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { (yyval).reset();;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { (yyval).reset();;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { (yyval).reset();;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2565 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2569 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2574 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2580 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2583 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2596 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2603 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2608 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2610 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2613 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2615 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2624 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2626 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2629 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2634 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2662 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2669 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2673 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2678 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { (yyval).reset();;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2683 "hphp.y"
    { (yyval).reset();;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2684 "hphp.y"
    { (yyval).reset();;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { (yyval).reset();;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2696 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2698 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2705 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2708 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2710 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2715 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2716 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2720 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2723 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2730 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2731 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2732 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2738 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2739 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 2746 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2753 "hphp.y"
    { (yyval).reset();;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2758 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2763 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2767 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2782 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2788 "hphp.y"
    { (yyval).reset();;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2792 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2804 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2806 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2809 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2813 "hphp.y"
    { (yyval).reset();;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2817 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2822 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2823 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2824 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2828 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2830 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2839 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2854 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2858 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2860 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2865 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2867 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2873 "hphp.y"
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
#line 2884 "hphp.y"
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
#line 2899 "hphp.y"
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
#line 2911 "hphp.y"
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
#line 2923 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 2930 "hphp.y"
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

  case 879:

/* Line 1455 of yacc.c  */
#line 2947 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2949 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2951 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2952 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 2967 "hphp.y"
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

  case 888:

/* Line 1455 of yacc.c  */
#line 2976 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2989 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2990 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2991 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2994 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2996 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2998 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3002 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3006 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3007 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3013 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3017 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3024 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3037 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3044 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3050 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3051 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3052 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3056 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3057 "hphp.y"
    { _p->onPipeVariable((yyval));;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3058 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3065 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { (yyval).reset();;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3071 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3072 "hphp.y"
    { (yyval)++;;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3077 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3078 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3079 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3082 "hphp.y"
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
#line 3093 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3094 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3098 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3099 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3102 "hphp.y"
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
#line 3111 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3115 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3118 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3119 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3126 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { (yyval).reset();;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3131 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3133 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3139 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3140 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3141 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3146 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3147 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3151 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3152 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3153 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3154 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3159 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3160 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3165 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3167 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3169 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3170 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3174 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3176 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3177 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3179 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3184 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3186 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3188 "hphp.y"
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
#line 3198 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3200 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3201 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3204 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3205 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3206 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3210 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3211 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3212 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3213 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3214 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3215 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3216 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3217 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3218 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3219 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3220 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3224 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3225 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3230 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3232 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3246 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3251 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3255 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3260 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3267 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 995:

/* Line 1455 of yacc.c  */
#line 3271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 996:

/* Line 1455 of yacc.c  */
#line 3272 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3278 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3282 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3288 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3292 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3299 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3300 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3304 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1004:

/* Line 1455 of yacc.c  */
#line 3307 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1005:

/* Line 1455 of yacc.c  */
#line 3313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1006:

/* Line 1455 of yacc.c  */
#line 3318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3320 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3321 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1016:

/* Line 1455 of yacc.c  */
#line 3342 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3343 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1; ;}
    break;

  case 1020:

/* Line 1455 of yacc.c  */
#line 3352 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1023:

/* Line 1455 of yacc.c  */
#line 3363 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 1024:

/* Line 1455 of yacc.c  */
#line 3365 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 1025:

/* Line 1455 of yacc.c  */
#line 3369 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 1026:

/* Line 1455 of yacc.c  */
#line 3372 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 1027:

/* Line 1455 of yacc.c  */
#line 3376 "hphp.y"
    {;}
    break;

  case 1028:

/* Line 1455 of yacc.c  */
#line 3377 "hphp.y"
    {;}
    break;

  case 1029:

/* Line 1455 of yacc.c  */
#line 3378 "hphp.y"
    {;}
    break;

  case 1030:

/* Line 1455 of yacc.c  */
#line 3384 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 1031:

/* Line 1455 of yacc.c  */
#line 3389 "hphp.y"
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
#line 3400 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 1033:

/* Line 1455 of yacc.c  */
#line 3405 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1034:

/* Line 1455 of yacc.c  */
#line 3406 "hphp.y"
    { ;}
    break;

  case 1035:

/* Line 1455 of yacc.c  */
#line 3411 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 1036:

/* Line 1455 of yacc.c  */
#line 3412 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 1037:

/* Line 1455 of yacc.c  */
#line 3418 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1038:

/* Line 1455 of yacc.c  */
#line 3423 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1039:

/* Line 1455 of yacc.c  */
#line 3428 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1040:

/* Line 1455 of yacc.c  */
#line 3432 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1041:

/* Line 1455 of yacc.c  */
#line 3437 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 1042:

/* Line 1455 of yacc.c  */
#line 3439 "hphp.y"
    { _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)])); (yyval) = (yyvsp[(2) - (5)]);;}
    break;

  case 1043:

/* Line 1455 of yacc.c  */
#line 3445 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1044:

/* Line 1455 of yacc.c  */
#line 3448 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1045:

/* Line 1455 of yacc.c  */
#line 3451 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1046:

/* Line 1455 of yacc.c  */
#line 3452 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1047:

/* Line 1455 of yacc.c  */
#line 3455 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1048:

/* Line 1455 of yacc.c  */
#line 3458 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1049:

/* Line 1455 of yacc.c  */
#line 3461 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1050:

/* Line 1455 of yacc.c  */
#line 3464 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1051:

/* Line 1455 of yacc.c  */
#line 3466 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1052:

/* Line 1455 of yacc.c  */
#line 3472 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1053:

/* Line 1455 of yacc.c  */
#line 3478 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1054:

/* Line 1455 of yacc.c  */
#line 3486 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1055:

/* Line 1455 of yacc.c  */
#line 3487 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14652 "hphp.5.tab.cpp"
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
#line 3490 "hphp.y"

/* !PHP5_ONLY*/
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}

