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
#define YYLAST   18344

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  204
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  295
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1056
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
    2560,  2566,  2571,  2575,  2577,  2581,  2585,  2587,  2589,  2591,
    2593,  2595,  2599,  2603,  2607,  2610,  2611,  2613,  2614,  2616,
    2617,  2623,  2627,  2631,  2633,  2635,  2637,  2639,  2643,  2646,
    2648,  2650,  2652,  2654,  2656,  2660,  2662,  2664,  2666,  2670,
    2672,  2675,  2678,  2683,  2687,  2692,  2694,  2696,  2698,  2702,
    2704,  2707,  2708,  2714,  2718,  2722,  2724,  2728,  2730,  2733,
    2734,  2740,  2744,  2747,  2748,  2752,  2753,  2758,  2761,  2762,
    2766,  2770,  2772,  2773,  2775,  2777,  2779,  2781,  2785,  2787,
    2789,  2791,  2795,  2797,  2799,  2803,  2807,  2810,  2815,  2818,
    2823,  2829,  2835,  2841,  2847,  2849,  2851,  2853,  2855,  2857,
    2859,  2863,  2867,  2872,  2877,  2881,  2883,  2885,  2887,  2889,
    2893,  2895,  2900,  2904,  2908,  2910,  2912,  2914,  2916,  2918,
    2922,  2926,  2931,  2936,  2940,  2942,  2944,  2952,  2962,  2970,
    2977,  2986,  2988,  2993,  2998,  3000,  3002,  3004,  3009,  3012,
    3014,  3015,  3017,  3019,  3021,  3025,  3029,  3033,  3034,  3036,
    3038,  3042,  3046,  3049,  3053,  3060,  3061,  3063,  3068,  3071,
    3072,  3078,  3082,  3086,  3088,  3095,  3100,  3105,  3108,  3111,
    3112,  3118,  3122,  3126,  3128,  3131,  3132,  3138,  3142,  3146,
    3148,  3151,  3154,  3156,  3159,  3161,  3166,  3170,  3174,  3181,
    3185,  3187,  3189,  3191,  3196,  3201,  3206,  3211,  3216,  3221,
    3224,  3227,  3232,  3235,  3238,  3240,  3244,  3248,  3252,  3253,
    3256,  3262,  3269,  3276,  3284,  3286,  3289,  3291,  3294,  3296,
    3301,  3303,  3308,  3312,  3313,  3315,  3319,  3322,  3326,  3328,
    3330,  3331,  3332,  3336,  3338,  3342,  3346,  3349,  3350,  3353,
    3356,  3359,  3362,  3364,  3367,  3372,  3375,  3381,  3385,  3387,
    3389,  3390,  3394,  3399,  3405,  3409,  3411,  3414,  3415,  3420,
    3422,  3426,  3429,  3434,  3440,  3443,  3446,  3448,  3450,  3452,
    3454,  3458,  3461,  3463,  3472,  3479,  3481
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
      -1,   115,    -1,   128,    -1,   126,    -1,   212,    -1,   130,
      -1,   221,   162,    -1,   162,   221,   162,    -1,   215,     9,
     217,    -1,   217,    -1,   215,   416,    -1,   221,    -1,   162,
     221,    -1,   221,   102,   211,    -1,   162,   221,   102,   211,
      -1,   218,     9,   220,    -1,   220,    -1,   218,   416,    -1,
     217,    -1,   111,   217,    -1,   112,   217,    -1,   211,    -1,
     221,   162,   211,    -1,   221,    -1,   159,   162,   221,    -1,
     162,   221,    -1,   222,   477,    -1,   222,   477,    -1,   225,
       9,   473,    14,   409,    -1,   112,   473,    14,   409,    -1,
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
     151,   336,   196,    -1,   126,   194,   469,   195,   196,    -1,
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
      38,    -1,    -1,   111,    -1,    -1,   244,   243,   476,   246,
     194,   287,   195,   484,   321,    -1,    -1,   325,   244,   243,
     476,   247,   194,   287,   195,   484,   321,    -1,    -1,   433,
     324,   244,   243,   476,   248,   194,   287,   195,   484,   321,
      -1,    -1,   169,   211,   250,    32,   497,   471,   197,   294,
     198,    -1,    -1,   433,   169,   211,   251,    32,   497,   471,
     197,   294,   198,    -1,    -1,   265,   262,   253,   266,   267,
     197,   297,   198,    -1,    -1,   433,   265,   262,   254,   266,
     267,   197,   297,   198,    -1,    -1,   131,   263,   255,   268,
     197,   297,   198,    -1,    -1,   433,   131,   263,   256,   268,
     197,   297,   198,    -1,    -1,   130,   258,   407,   266,   267,
     197,   297,   198,    -1,    -1,   171,   264,   260,   267,   197,
     297,   198,    -1,    -1,   433,   171,   264,   261,   267,   197,
     297,   198,    -1,   476,    -1,   163,    -1,   476,    -1,   476,
      -1,   130,    -1,   123,   130,    -1,   123,   122,   130,    -1,
     122,   123,   130,    -1,   122,   130,    -1,   132,   400,    -1,
      -1,   133,   269,    -1,    -1,   132,   269,    -1,    -1,   400,
      -1,   269,     9,   400,    -1,   400,    -1,   270,     9,   400,
      -1,   136,   272,    -1,    -1,   445,    -1,    38,   445,    -1,
     137,   194,   458,   195,    -1,   228,    -1,    32,   226,    97,
     196,    -1,   228,    -1,    32,   226,    99,   196,    -1,   228,
      -1,    32,   226,    95,   196,    -1,   228,    -1,    32,   226,
     101,   196,    -1,   211,    14,   409,    -1,   277,     9,   211,
      14,   409,    -1,   197,   279,   198,    -1,   197,   196,   279,
     198,    -1,    32,   279,   105,   196,    -1,    32,   196,   279,
     105,   196,    -1,   279,   106,   346,   280,   226,    -1,   279,
     107,   280,   226,    -1,    -1,    32,    -1,   196,    -1,   281,
      75,   335,   228,    -1,    -1,   282,    75,   335,    32,   226,
      -1,    -1,    76,   228,    -1,    -1,    76,    32,   226,    -1,
      -1,   286,     9,   434,   327,   498,   172,    83,    -1,   286,
       9,   434,   327,   498,    38,   172,    83,    -1,   286,     9,
     434,   327,   498,   172,    -1,   286,   416,    -1,   434,   327,
     498,   172,    83,    -1,   434,   327,   498,    38,   172,    83,
      -1,   434,   327,   498,   172,    -1,    -1,   434,   327,   498,
      83,    -1,   434,   327,   498,    38,    83,    -1,   434,   327,
     498,    38,    83,    14,   346,    -1,   434,   327,   498,    83,
      14,   346,    -1,   286,     9,   434,   327,   498,    83,    -1,
     286,     9,   434,   327,   498,    38,    83,    -1,   286,     9,
     434,   327,   498,    38,    83,    14,   346,    -1,   286,     9,
     434,   327,   498,    83,    14,   346,    -1,   288,     9,   434,
     498,   172,    83,    -1,   288,     9,   434,   498,    38,   172,
      83,    -1,   288,     9,   434,   498,   172,    -1,   288,   416,
      -1,   434,   498,   172,    83,    -1,   434,   498,    38,   172,
      83,    -1,   434,   498,   172,    -1,    -1,   434,   498,    83,
      -1,   434,   498,    38,    83,    -1,   434,   498,    38,    83,
      14,   346,    -1,   434,   498,    83,    14,   346,    -1,   288,
       9,   434,   498,    83,    -1,   288,     9,   434,   498,    38,
      83,    -1,   288,     9,   434,   498,    38,    83,    14,   346,
      -1,   288,     9,   434,   498,    83,    14,   346,    -1,   290,
     416,    -1,    -1,   346,    -1,    38,   445,    -1,   172,   346,
      -1,   290,     9,   346,    -1,   290,     9,   172,   346,    -1,
     290,     9,    38,   445,    -1,   291,     9,   292,    -1,   292,
      -1,    83,    -1,   199,   445,    -1,   199,   197,   346,   198,
      -1,   293,     9,    83,    -1,   293,     9,    83,    14,   409,
      -1,    83,    -1,    83,    14,   409,    -1,   294,   295,    -1,
      -1,   296,   196,    -1,   474,    14,   409,    -1,   297,   298,
      -1,    -1,    -1,   323,   299,   329,   196,    -1,    -1,   325,
     497,   300,   329,   196,    -1,   330,   196,    -1,   331,   196,
      -1,   332,   196,    -1,    -1,   324,   244,   243,   475,   194,
     301,   285,   195,   484,   481,   322,    -1,    -1,   433,   324,
     244,   243,   476,   194,   302,   285,   195,   484,   481,   322,
      -1,   165,   307,   196,    -1,   166,   315,   196,    -1,   168,
     317,   196,    -1,     4,   132,   400,   196,    -1,     4,   133,
     400,   196,    -1,   117,   270,   196,    -1,   117,   270,   197,
     303,   198,    -1,   303,   304,    -1,   303,   305,    -1,    -1,
     224,   158,   211,   173,   270,   196,    -1,   306,   102,   324,
     211,   196,    -1,   306,   102,   325,   196,    -1,   224,   158,
     211,    -1,   211,    -1,   308,    -1,   307,     9,   308,    -1,
     309,   397,   313,   314,    -1,   163,    -1,    31,   310,    -1,
     310,    -1,   138,    -1,   138,   179,   497,   415,   180,    -1,
     138,   179,   497,     9,   497,   180,    -1,   400,    -1,   125,
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
     474,    14,   409,    -1,   112,   474,    14,   409,    -1,   331,
       9,   474,    -1,   123,   112,   474,    -1,   123,   333,   471,
      -1,   333,   471,    14,   497,    -1,   112,   184,   476,    -1,
     194,   334,   195,    -1,    72,   404,   407,    -1,    72,   257,
      -1,    71,   346,    -1,   389,    -1,   384,    -1,   194,   346,
     195,    -1,   336,     9,   346,    -1,   346,    -1,   336,    -1,
      -1,    27,    -1,    27,   346,    -1,    27,   346,   136,   346,
      -1,   194,   338,   195,    -1,   445,    14,   338,    -1,   137,
     194,   458,   195,    14,   338,    -1,    29,   346,    -1,   445,
      14,   341,    -1,    28,   346,    -1,   445,    14,   343,    -1,
     137,   194,   458,   195,    14,   343,    -1,   347,    -1,   445,
      -1,   334,    -1,   449,    -1,   448,    -1,   137,   194,   458,
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
      33,   346,    -1,   468,    -1,    66,   346,    -1,    65,   346,
      -1,    64,   346,    -1,    63,   346,    -1,    62,   346,    -1,
      61,   346,    -1,    60,   346,    -1,    73,   405,    -1,    59,
     346,    -1,   413,    -1,   365,    -1,   372,    -1,   375,    -1,
     378,    -1,   364,    -1,   200,   406,   200,    -1,    13,   346,
      -1,   386,    -1,   117,   194,   388,   416,   195,    -1,    -1,
      -1,   244,   243,   194,   350,   287,   195,   484,   348,   484,
     197,   226,   198,    -1,    -1,   325,   244,   243,   194,   351,
     287,   195,   484,   348,   484,   197,   226,   198,    -1,    -1,
     189,    83,   353,   358,    -1,    -1,   189,   190,   354,   287,
     191,   484,   358,    -1,    -1,   189,   197,   355,   226,   198,
      -1,    -1,    83,   356,   358,    -1,    -1,   190,   357,   287,
     191,   484,   358,    -1,     8,   346,    -1,     8,   343,    -1,
       8,   197,   226,   198,    -1,    91,    -1,   470,    -1,   360,
       9,   359,   136,   346,    -1,   359,   136,   346,    -1,   361,
       9,   359,   136,   409,    -1,   359,   136,   409,    -1,   360,
     415,    -1,    -1,   361,   415,    -1,    -1,   183,   194,   362,
     195,    -1,   138,   194,   459,   195,    -1,    70,   459,   201,
      -1,   367,   415,    -1,    -1,   367,     9,   346,   136,   346,
      -1,   346,   136,   346,    -1,   367,     9,   346,   136,    38,
     445,    -1,   346,   136,    38,   445,    -1,   369,   415,    -1,
      -1,   369,     9,   409,   136,   409,    -1,   409,   136,   409,
      -1,   371,   415,    -1,    -1,   371,     9,   421,   136,   421,
      -1,   421,   136,   421,    -1,   139,    70,   366,   201,    -1,
     139,    70,   368,   201,    -1,   139,    70,   370,   201,    -1,
     140,    70,   381,   201,    -1,   140,    70,   382,   201,    -1,
     140,    70,   383,   201,    -1,   141,    70,   381,   201,    -1,
     141,    70,   382,   201,    -1,   141,    70,   383,   201,    -1,
     336,   415,    -1,    -1,   410,   415,    -1,    -1,   422,   415,
      -1,    -1,   400,   197,   461,   198,    -1,   400,   197,   463,
     198,    -1,   386,    70,   455,   201,    -1,   387,    70,   455,
     201,    -1,   365,    -1,   372,    -1,   375,    -1,   378,    -1,
     470,    -1,   448,    -1,    91,    -1,   194,   347,   195,    -1,
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
      -1,   400,    -1,   408,    -1,   124,    -1,   453,    -1,   194,
     347,   195,    -1,   401,    -1,   402,   158,   454,    -1,   401,
      -1,   451,    -1,   403,   158,   454,    -1,   400,    -1,   124,
      -1,   456,    -1,   194,   195,    -1,   335,    -1,    -1,    -1,
      90,    -1,   465,    -1,   194,   289,   195,    -1,    -1,    78,
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
      -1,   470,    -1,   408,    -1,   202,   465,   202,    -1,   203,
     465,   203,    -1,   154,   465,   155,    -1,   417,   415,    -1,
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
      -1,   197,   346,   198,    -1,   436,    -1,   454,    -1,   211,
      -1,   197,   346,   198,    -1,   438,    -1,   454,    -1,    70,
     455,   201,    -1,   197,   346,   198,    -1,   446,   440,    -1,
     194,   334,   195,   440,    -1,   457,   440,    -1,   194,   334,
     195,   440,    -1,   194,   334,   195,   435,   437,    -1,   194,
     347,   195,   435,   437,    -1,   194,   334,   195,   435,   436,
      -1,   194,   347,   195,   435,   436,    -1,   452,    -1,   399,
      -1,   450,    -1,   451,    -1,   441,    -1,   443,    -1,   445,
     435,   437,    -1,   403,   158,   454,    -1,   447,   194,   289,
     195,    -1,   448,   194,   289,   195,    -1,   194,   445,   195,
      -1,   399,    -1,   450,    -1,   451,    -1,   441,    -1,   445,
     435,   437,    -1,   444,    -1,   447,   194,   289,   195,    -1,
     194,   445,   195,    -1,   403,   158,   454,    -1,   452,    -1,
     441,    -1,   399,    -1,   365,    -1,   408,    -1,   194,   445,
     195,    -1,   194,   347,   195,    -1,   448,   194,   289,   195,
      -1,   447,   194,   289,   195,    -1,   194,   449,   195,    -1,
     349,    -1,   352,    -1,   445,   435,   439,   477,   194,   289,
     195,    -1,   194,   334,   195,   435,   439,   477,   194,   289,
     195,    -1,   403,   158,   213,   477,   194,   289,   195,    -1,
     403,   158,   454,   194,   289,   195,    -1,   403,   158,   197,
     346,   198,   194,   289,   195,    -1,   453,    -1,   453,    70,
     455,   201,    -1,   453,   197,   346,   198,    -1,   454,    -1,
      83,    -1,    84,    -1,   199,   197,   346,   198,    -1,   199,
     454,    -1,   346,    -1,    -1,   452,    -1,   442,    -1,   443,
      -1,   456,   435,   437,    -1,   402,   158,   452,    -1,   194,
     445,   195,    -1,    -1,   442,    -1,   444,    -1,   456,   435,
     436,    -1,   194,   445,   195,    -1,   458,     9,    -1,   458,
       9,   445,    -1,   458,     9,   137,   194,   458,   195,    -1,
      -1,   445,    -1,   137,   194,   458,   195,    -1,   460,   415,
      -1,    -1,   460,     9,   346,   136,   346,    -1,   460,     9,
     346,    -1,   346,   136,   346,    -1,   346,    -1,   460,     9,
     346,   136,    38,   445,    -1,   460,     9,    38,   445,    -1,
     346,   136,    38,   445,    -1,    38,   445,    -1,   462,   415,
      -1,    -1,   462,     9,   346,   136,   346,    -1,   462,     9,
     346,    -1,   346,   136,   346,    -1,   346,    -1,   464,   415,
      -1,    -1,   464,     9,   409,   136,   409,    -1,   464,     9,
     409,    -1,   409,   136,   409,    -1,   409,    -1,   465,   466,
      -1,   465,    90,    -1,   466,    -1,    90,   466,    -1,    83,
      -1,    83,    70,   467,   201,    -1,    83,   435,   211,    -1,
     156,   346,   198,    -1,   156,    82,    70,   346,   201,   198,
      -1,   157,   445,   198,    -1,   211,    -1,    85,    -1,    83,
      -1,   127,   194,   336,   195,    -1,   128,   194,   445,   195,
      -1,   128,   194,   347,   195,    -1,   128,   194,   449,   195,
      -1,   128,   194,   448,   195,    -1,   128,   194,   334,   195,
      -1,     7,   346,    -1,     6,   346,    -1,     5,   194,   346,
     195,    -1,     4,   346,    -1,     3,   346,    -1,   445,    -1,
     469,     9,   445,    -1,   403,   158,   212,    -1,   403,   158,
     130,    -1,    -1,   102,   497,    -1,   184,   476,    14,   497,
     196,    -1,   433,   184,   476,    14,   497,   196,    -1,   186,
     476,   471,    14,   497,   196,    -1,   433,   186,   476,   471,
      14,   497,   196,    -1,   213,    -1,   497,   213,    -1,   212,
      -1,   497,   212,    -1,   213,    -1,   213,   179,   486,   180,
      -1,   211,    -1,   211,   179,   486,   180,    -1,   179,   479,
     180,    -1,    -1,   497,    -1,   478,     9,   497,    -1,   478,
     415,    -1,   478,     9,   172,    -1,   479,    -1,   172,    -1,
      -1,    -1,   193,   482,   416,    -1,   483,    -1,   482,     9,
     483,    -1,   497,    14,   497,    -1,   497,   485,    -1,    -1,
      32,   497,    -1,   102,   497,    -1,   103,   497,    -1,   488,
     415,    -1,   485,    -1,   487,   485,    -1,   488,     9,   489,
     211,    -1,   489,   211,    -1,   488,     9,   489,   211,   487,
      -1,   489,   211,   487,    -1,    50,    -1,    51,    -1,    -1,
      91,   136,   497,    -1,    31,    91,   136,   497,    -1,   224,
     158,   211,   136,   497,    -1,   491,     9,   490,    -1,   490,
      -1,   491,   415,    -1,    -1,   183,   194,   492,   195,    -1,
     224,    -1,   211,   158,   495,    -1,   211,   477,    -1,   179,
     497,   415,   180,    -1,   179,   497,     9,   497,   180,    -1,
      31,   497,    -1,    59,   497,    -1,   224,    -1,   138,    -1,
     142,    -1,   493,    -1,   494,   158,   495,    -1,   138,   496,
      -1,   163,    -1,   194,   111,   194,   480,   195,    32,   497,
     195,    -1,   194,   497,     9,   478,   415,   195,    -1,   497,
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
    2641,  2643,  2647,  2648,  2652,  2658,  2661,  2665,  2666,  2667,
    2668,  2669,  2670,  2671,  2676,  2678,  2682,  2683,  2686,  2687,
    2691,  2694,  2696,  2698,  2702,  2703,  2704,  2705,  2708,  2712,
    2713,  2714,  2715,  2719,  2721,  2728,  2729,  2730,  2731,  2736,
    2737,  2738,  2739,  2741,  2742,  2744,  2745,  2746,  2750,  2752,
    2756,  2758,  2761,  2764,  2766,  2768,  2771,  2773,  2777,  2779,
    2782,  2785,  2791,  2793,  2796,  2797,  2802,  2805,  2809,  2809,
    2814,  2817,  2818,  2822,  2823,  2827,  2828,  2829,  2833,  2838,
    2843,  2844,  2848,  2853,  2858,  2859,  2863,  2864,  2869,  2871,
    2876,  2887,  2901,  2913,  2928,  2929,  2930,  2931,  2932,  2933,
    2934,  2944,  2953,  2955,  2957,  2961,  2962,  2963,  2964,  2965,
    2981,  2982,  2984,  2986,  2993,  2994,  2995,  2996,  2997,  2998,
    2999,  3000,  3002,  3007,  3011,  3012,  3016,  3019,  3026,  3030,
    3039,  3046,  3054,  3056,  3057,  3061,  3062,  3063,  3065,  3070,
    3071,  3082,  3083,  3084,  3085,  3096,  3099,  3102,  3103,  3104,
    3105,  3116,  3120,  3121,  3122,  3124,  3125,  3126,  3130,  3132,
    3135,  3137,  3138,  3139,  3140,  3143,  3145,  3146,  3150,  3152,
    3155,  3157,  3158,  3159,  3163,  3165,  3168,  3171,  3173,  3175,
    3179,  3180,  3182,  3183,  3189,  3190,  3192,  3202,  3204,  3206,
    3209,  3210,  3211,  3215,  3216,  3217,  3218,  3219,  3220,  3221,
    3222,  3223,  3224,  3225,  3229,  3230,  3234,  3236,  3244,  3246,
    3250,  3254,  3259,  3263,  3271,  3272,  3276,  3277,  3283,  3284,
    3293,  3294,  3302,  3305,  3309,  3312,  3317,  3322,  3324,  3325,
    3326,  3329,  3331,  3337,  3338,  3342,  3343,  3347,  3348,  3352,
    3353,  3356,  3361,  3362,  3366,  3369,  3371,  3375,  3381,  3382,
    3383,  3387,  3391,  3401,  3409,  3411,  3415,  3417,  3422,  3428,
    3431,  3436,  3441,  3443,  3450,  3453,  3456,  3457,  3460,  3463,
    3464,  3469,  3471,  3475,  3481,  3491,  3492
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
     409,   409,   410,   410,   411,   412,   412,   413,   413,   413,
     413,   413,   413,   413,   414,   414,   415,   415,   416,   416,
     417,   417,   417,   417,   418,   418,   418,   418,   418,   419,
     419,   419,   419,   420,   420,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   422,   422,
     423,   423,   424,   424,   424,   424,   425,   425,   426,   426,
     427,   427,   428,   428,   429,   429,   430,   430,   432,   431,
     433,   434,   434,   435,   435,   436,   436,   436,   437,   437,
     438,   438,   439,   439,   440,   440,   441,   441,   442,   442,
     443,   443,   444,   444,   445,   445,   445,   445,   445,   445,
     445,   445,   445,   445,   445,   446,   446,   446,   446,   446,
     446,   446,   446,   446,   447,   447,   447,   447,   447,   447,
     447,   447,   447,   448,   449,   449,   450,   450,   451,   451,
     451,   452,   453,   453,   453,   454,   454,   454,   454,   455,
     455,   456,   456,   456,   456,   456,   456,   457,   457,   457,
     457,   457,   458,   458,   458,   458,   458,   458,   459,   459,
     460,   460,   460,   460,   460,   460,   460,   460,   461,   461,
     462,   462,   462,   462,   463,   463,   464,   464,   464,   464,
     465,   465,   465,   465,   466,   466,   466,   466,   466,   466,
     467,   467,   467,   468,   468,   468,   468,   468,   468,   468,
     468,   468,   468,   468,   469,   469,   470,   470,   471,   471,
     472,   472,   472,   472,   473,   473,   474,   474,   475,   475,
     476,   476,   477,   477,   478,   478,   479,   480,   480,   480,
     480,   481,   481,   482,   482,   483,   483,   484,   484,   485,
     485,   486,   487,   487,   488,   488,   488,   488,   489,   489,
     489,   490,   490,   490,   491,   491,   492,   492,   493,   494,
     495,   495,   496,   496,   497,   497,   497,   497,   497,   497,
     497,   497,   497,   497,   497,   498,   498
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
       5,     4,     3,     1,     3,     3,     1,     1,     1,     1,
       1,     3,     3,     3,     2,     0,     1,     0,     1,     0,
       5,     3,     3,     1,     1,     1,     1,     3,     2,     1,
       1,     1,     1,     1,     3,     1,     1,     1,     3,     1,
       2,     2,     4,     3,     4,     1,     1,     1,     3,     1,
       2,     0,     5,     3,     3,     1,     3,     1,     2,     0,
       5,     3,     2,     0,     3,     0,     4,     2,     0,     3,
       3,     1,     0,     1,     1,     1,     1,     3,     1,     1,
       1,     3,     1,     1,     3,     3,     2,     4,     2,     4,
       5,     5,     5,     5,     1,     1,     1,     1,     1,     1,
       3,     3,     4,     4,     3,     1,     1,     1,     1,     3,
       1,     4,     3,     3,     1,     1,     1,     1,     1,     3,
       3,     4,     4,     3,     1,     1,     7,     9,     7,     6,
       8,     1,     4,     4,     1,     1,     1,     4,     2,     1,
       0,     1,     1,     1,     3,     3,     3,     0,     1,     1,
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
       0,     3,     4,     5,     3,     1,     2,     0,     4,     1,
       3,     2,     4,     5,     2,     2,     1,     1,     1,     1,
       3,     2,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   432,     0,     0,   848,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   939,
       0,   927,   718,     0,   724,   725,   726,    25,   788,   915,
     916,   156,   157,   727,     0,   137,     0,     0,     0,     0,
      26,     0,     0,     0,     0,   191,     0,     0,     0,     0,
       0,     0,   398,   399,   400,   403,   402,   401,     0,     0,
       0,     0,   220,     0,     0,     0,    33,    34,    35,   731,
     733,   734,   728,   729,     0,     0,     0,   735,   730,     0,
     702,    28,    29,    30,    32,    31,     0,   732,     0,     0,
       0,     0,   736,   404,   537,    27,     0,   155,   127,     0,
     719,     0,     0,     4,   117,   119,   787,     0,   701,     0,
       6,   190,     7,     9,     8,    10,     0,     0,   396,   445,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   443,
     904,   905,   519,   515,   516,   517,   518,   426,   522,     0,
     425,   875,   703,   710,     0,   790,   514,   395,   878,   879,
     890,   444,     0,     0,   447,   446,   876,   877,   874,   911,
     914,   504,   789,    11,   403,   402,   401,     0,     0,    32,
       0,   117,   190,     0,   983,   444,   982,     0,   980,   979,
     521,     0,   433,   440,   438,     0,     0,   486,   487,   488,
     489,   513,   511,   510,   509,   508,   507,   506,   505,    25,
     915,   727,   705,    33,    34,    35,     0,     0,  1003,   897,
     703,     0,   704,   467,     0,   465,     0,   943,     0,   797,
     424,   714,   210,     0,  1003,   423,   713,   708,     0,   723,
     704,   922,   923,   929,   921,   715,     0,     0,   717,   512,
       0,     0,     0,     0,   429,     0,   135,   431,     0,     0,
     141,   143,     0,     0,   145,     0,    76,    75,    70,    69,
      61,    62,    53,    73,    84,    85,     0,    56,     0,    68,
      60,    66,    87,    79,    78,    51,    74,    94,    95,    52,
      90,    49,    91,    50,    92,    48,    96,    83,    88,    93,
      80,    81,    55,    82,    86,    47,    77,    63,    97,    71,
      64,    54,    46,    45,    44,    43,    42,    41,    65,    99,
      98,   101,    58,    39,    40,    67,  1047,  1048,    59,  1052,
      38,    57,    89,     0,     0,   117,   100,   994,  1046,     0,
    1049,     0,     0,   147,     0,     0,     0,   181,     0,     0,
       0,     0,     0,     0,   799,     0,   105,   107,   309,     0,
       0,   308,     0,   224,     0,   221,   314,     0,     0,     0,
       0,     0,  1000,   206,   218,   935,   939,   556,   579,   579,
       0,   964,     0,   738,     0,     0,     0,   962,     0,    16,
       0,   121,   198,   212,   219,   607,   549,     0,   988,   529,
     531,   533,   852,   432,   445,     0,     0,   443,   444,   446,
       0,     0,   918,   720,     0,   721,     0,     0,     0,   180,
       0,     0,   123,   300,     0,    24,   189,     0,   217,   202,
     216,   401,   404,   190,   397,   170,   171,   172,   173,   174,
     176,   177,   179,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   927,     0,   169,   920,   920,   949,     0,     0,     0,
       0,     0,     0,     0,     0,   394,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   466,
     464,   853,   854,     0,   920,     0,   866,   300,   300,   920,
       0,   935,     0,   190,     0,     0,   149,     0,   850,   845,
     797,     0,   445,   443,     0,   947,     0,   554,   796,   938,
     723,   445,   443,   444,   123,     0,   300,   422,     0,   868,
     716,     0,   127,   260,     0,   536,     0,   152,     0,     0,
     430,     0,     0,     0,     0,     0,   144,   168,   146,  1047,
    1048,  1044,  1045,     0,  1051,  1037,     0,     0,     0,     0,
      72,    37,    59,    36,   995,   175,   178,   148,   127,     0,
     165,   167,     0,     0,     0,     0,   108,     0,   798,   106,
      18,     0,   102,     0,   310,     0,   150,   223,   222,     0,
       0,   151,   984,     0,     0,   445,   443,   444,   447,   446,
       0,  1030,   230,     0,   936,     0,     0,     0,     0,   797,
     797,     0,     0,   153,     0,     0,   737,   963,   788,     0,
       0,   961,   793,   960,   120,     5,    13,    14,     0,   228,
       0,     0,   542,     0,     0,   797,     0,     0,   711,   706,
     543,     0,     0,     0,     0,   852,   127,     0,   799,   851,
    1056,   421,   435,   500,   884,   903,   132,   126,   128,   129,
     130,   131,   395,     0,   520,   791,   792,   118,   797,     0,
    1004,     0,     0,     0,   799,   301,     0,   525,   192,   226,
       0,   470,   472,   471,   483,     0,     0,   503,   468,   469,
     473,   475,   474,   491,   490,   493,   492,   494,   496,   498,
     497,   495,   485,   484,   477,   478,   476,   479,   480,   482,
     499,   481,   919,     0,     0,   953,     0,   797,   987,     0,
     986,  1003,   881,   208,   200,   214,     0,   988,   204,   190,
       0,   436,   439,   441,   449,   463,   462,   461,   460,   459,
     458,   457,   456,   455,   454,   453,   452,   856,     0,   855,
     858,   880,   862,  1003,   859,     0,     0,     0,     0,     0,
       0,     0,     0,   981,   434,   843,   847,   796,   849,     0,
     707,     0,   942,     0,   941,   226,     0,   707,   926,   925,
     911,   914,     0,     0,   855,   858,   924,   859,   427,   262,
     264,   127,   540,   539,   428,     0,   127,   244,   136,   431,
       0,     0,     0,     0,     0,   256,   256,   142,   797,     0,
       0,     0,  1035,   797,     0,  1010,     0,     0,     0,     0,
       0,   795,     0,    33,    34,    35,     0,     0,   740,   744,
     745,   746,   748,     0,   739,   125,   786,   747,  1003,  1050,
       0,     0,     0,     0,    19,     0,    20,     0,   103,     0,
       0,     0,   114,   799,     0,   112,   107,   104,   109,     0,
     307,   315,   312,     0,     0,   973,   978,   975,   974,   977,
     976,    12,  1028,  1029,     0,   797,     0,     0,     0,   935,
     932,     0,   553,     0,   569,   796,   555,   796,   578,   572,
     575,   972,   971,   970,     0,   966,     0,   967,   969,     0,
       5,     0,     0,     0,   601,   602,   610,   609,     0,   443,
       0,   796,   548,   552,     0,     0,   989,     0,   530,     0,
       0,  1017,   852,   286,  1055,     0,     0,   867,     0,   917,
     796,  1006,  1002,   302,   303,   700,   798,   299,     0,   852,
       0,     0,   228,   527,   194,   502,     0,   586,   587,     0,
     584,   796,   948,     0,     0,   300,   230,     0,   228,     0,
       0,   226,     0,   927,   450,     0,     0,   864,   865,   882,
     883,   912,   913,     0,     0,     0,   831,   804,   805,   806,
     813,     0,    33,    34,    35,     0,     0,   819,   825,   826,
     827,     0,   817,   815,   816,   837,   797,     0,   845,   946,
     945,     0,   228,     0,   869,   722,     0,   266,     0,     0,
     133,     0,     0,     0,     0,     0,     0,     0,   236,   237,
     248,     0,   127,   246,   162,   256,     0,   256,     0,   796,
       0,     0,     0,     0,   796,  1036,  1038,  1009,   797,  1008,
       0,   797,   769,   770,   767,   768,   803,     0,   797,   795,
     562,   581,   581,   551,     0,     0,   955,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1041,   182,     0,   185,   166,
       0,     0,   110,   115,   116,   108,   798,   113,     0,   311,
       0,   985,   154,  1001,  1030,  1021,  1025,   229,   231,   321,
       0,     0,   933,     0,     0,   558,     0,   965,     0,    17,
       0,   988,   227,   321,     0,     0,   707,   545,     0,   712,
     990,     0,  1017,   534,     0,     0,  1056,     0,   291,   289,
     858,   870,  1003,   858,   871,  1005,     0,     0,   304,   124,
       0,   852,   225,     0,   852,     0,   501,   952,   951,     0,
     300,     0,     0,     0,     0,     0,     0,   228,   196,   723,
     857,   300,     0,   809,   810,   811,   812,   820,   821,   835,
       0,   797,     0,   831,   566,   583,   583,     0,   808,   839,
       0,   796,   842,   844,   846,     0,   940,     0,   857,     0,
       0,     0,     0,   263,   541,   138,     0,   431,   236,   238,
     935,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     250,     0,  1042,     0,  1031,     0,  1034,   796,     0,     0,
       0,   742,   796,   794,     0,     0,   797,     0,     0,   783,
     797,     0,     0,   797,     0,   749,   784,   785,   959,     0,
     797,   752,   754,   753,     0,     0,   750,   751,   755,   757,
     756,   772,   771,   774,   773,   775,   777,   779,   778,   776,
     765,   764,   759,   760,   758,   761,   762,   763,   766,  1040,
       0,   127,     0,     0,   111,    21,   313,     0,     0,     0,
    1022,  1027,     0,   395,   937,   935,   437,   442,   448,   560,
       0,     0,    15,     0,   395,   613,     0,     0,   615,   608,
     611,     0,   606,     0,   992,     0,  1018,   538,     0,   292,
       0,     0,   287,     0,   306,   305,  1017,     0,   321,     0,
     852,     0,   300,     0,   909,   321,   988,   321,   991,     0,
       0,     0,   451,     0,     0,   823,   796,   830,   814,     0,
       0,   797,     0,     0,   829,   797,     0,   807,     0,     0,
     797,   818,   836,   944,   321,     0,   127,     0,   259,   245,
       0,     0,     0,   235,   158,   249,     0,     0,   252,     0,
     257,   258,   127,   251,  1043,  1032,     0,  1007,     0,  1054,
     802,   801,   741,   570,   796,   561,     0,   573,   796,   580,
     576,     0,   796,   550,   743,     0,   585,   796,   954,   781,
       0,     0,     0,    22,    23,  1024,  1019,  1020,  1023,   232,
       0,     0,     0,   402,   393,     0,     0,     0,   207,   320,
     322,     0,   392,     0,     0,     0,   988,   395,     0,     0,
     557,   968,   317,   213,   604,     0,     0,   544,   532,     0,
     295,   285,     0,   288,   294,   300,   524,  1017,   395,  1017,
       0,   950,     0,   908,   395,     0,   395,   993,   321,   852,
     906,   834,   833,   822,   571,   796,   565,     0,   574,   796,
     582,   577,     0,   824,   796,   838,   395,   127,   265,   134,
     139,   160,   239,     0,   247,   253,   127,   255,  1033,     0,
       0,     0,   564,   782,   547,     0,   958,   957,   780,   127,
     186,  1026,     0,     0,     0,   996,     0,     0,     0,   233,
       0,   988,     0,   358,   354,   360,   702,    32,     0,   348,
       0,   353,   357,   370,     0,   368,   373,     0,   372,     0,
     371,     0,   190,   324,     0,   326,     0,   327,   328,     0,
       0,   934,   559,     0,   605,   603,   614,   612,   296,     0,
       0,   283,   293,     0,     0,  1017,     0,   203,   524,  1017,
     910,   209,   317,   215,   395,     0,     0,     0,   568,   828,
     841,     0,   211,   261,     0,     0,   127,   242,   159,   254,
    1053,   800,     0,     0,     0,     0,     0,     0,   420,     0,
     997,     0,   338,   342,   417,   418,   352,     0,     0,     0,
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
     190,   199,   316,     0,     0,     0,   284,   298,   907,     0,
       0,   388,   127,   193,  1017,     0,     0,   205,  1017,   832,
       0,     0,   127,   240,   140,   161,     0,   563,   546,   956,
     184,   336,   337,   415,   234,     0,   797,   797,     0,   361,
     349,     0,     0,     0,   367,   369,     0,     0,   374,   381,
     382,   380,     0,     0,   323,   998,     0,     0,     0,   419,
       0,   318,     0,   297,     0,   599,   799,   127,     0,     0,
     195,   201,     0,   567,   840,     0,     0,   163,   339,   117,
       0,   340,   341,     0,   796,     0,   796,   363,   359,   364,
     617,   618,     0,   350,   383,   384,   376,   377,   375,   413,
     410,  1030,   329,   325,   414,     0,   319,   600,   798,     0,
       0,   389,   127,   197,     0,   243,     0,   188,     0,   395,
       0,   355,   362,   366,     0,     0,   852,   331,     0,   597,
     523,   526,     0,   241,     0,     0,   164,   346,     0,   394,
     356,   411,   999,     0,   799,   406,   852,   598,   528,     0,
     187,     0,     0,   345,  1017,   852,   270,   407,   408,   409,
    1056,   405,     0,     0,     0,   344,  1011,   406,     0,  1017,
       0,   343,     0,     0,  1056,     0,   275,   273,  1011,   127,
     799,  1013,     0,   390,   127,   330,     0,   276,     0,     0,
     271,     0,     0,   798,  1012,     0,  1016,     0,     0,   279,
     269,     0,   272,   278,   332,   183,  1014,  1015,   391,   280,
       0,     0,   267,   277,     0,   268,   282,   281
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   113,   910,   635,   181,  1525,   731,
     353,   354,   355,   356,   863,   864,   865,   115,   116,   117,
     118,   119,   410,   667,   668,   549,   255,  1594,   555,  1503,
    1595,  1837,   852,   348,   578,  1797,  1099,  1291,  1856,   427,
     182,   669,   950,  1165,  1351,   123,   638,   967,   670,   689,
     971,   612,   966,   235,   530,   671,   639,   968,   429,   373,
     393,   126,   952,   913,   888,  1117,  1528,  1222,  1028,  1744,
    1598,   808,  1034,   554,   817,  1036,  1392,   800,  1017,  1020,
    1211,  1863,  1864,   657,   658,   683,   684,   360,   361,   367,
    1563,  1722,  1723,  1303,  1439,  1551,  1716,  1846,  1866,  1755,
    1801,  1802,  1803,  1538,  1539,  1540,  1541,  1757,  1758,  1764,
    1813,  1544,  1545,  1549,  1709,  1710,  1711,  1733,  1905,  1440,
    1441,   183,   128,  1880,  1881,  1714,  1443,  1444,  1445,  1446,
     129,   248,   550,   551,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,  1575,   140,   949,  1164,   141,   654,
     655,   656,   252,   402,   545,   644,   645,  1253,   646,  1254,
     142,   143,   618,   619,  1245,  1246,  1360,  1361,   144,   839,
     998,   145,   840,   999,   146,   841,  1000,   621,  1248,  1363,
     147,   842,   148,   149,  1786,   150,   640,  1565,   641,  1134,
     918,  1322,  1319,  1702,  1703,   151,   152,   153,   238,   154,
     239,   249,   414,   537,   155,  1056,  1250,   846,   847,   156,
    1057,   941,   589,  1058,  1003,  1187,  1004,  1189,  1365,  1190,
    1191,  1006,  1369,  1370,  1007,   776,   520,   195,   196,   672,
     660,   503,  1150,  1151,   762,   763,   937,   158,   241,   159,
     160,   185,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   723,   245,   246,   615,   228,   229,   726,   727,  1259,
    1260,   386,   387,   904,   171,   603,   172,   653,   173,   339,
    1724,  1776,   374,   422,   678,   679,  1050,  1893,  1900,  1901,
    1145,  1300,   884,  1301,   885,   886,   822,   823,   824,   340,
     341,   849,   564,  1527,   935
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1597
static const yytype_int16 yypact[] =
{
   -1597,   172, -1597, -1597,  5473, 13714, 13714,   -31, 13714, 13714,
   13714, 11101, 13714, 13714, -1597, 13714, 13714, 13714, 13714, 13714,
   13714, 13714, 13714, 13714, 13714, 13714, 13714, 16956, 16956, 11302,
   13714, 17723,    -4,    46, -1597, -1597, -1597,   147, -1597,   180,
   -1597, -1597, -1597,   153, 13714, -1597,    46,   187,   221,   242,
   -1597,    46, 11503,  5349, 11704, -1597, 14553, 10297,    40, 13714,
    3988,   163, -1597, -1597, -1597,   557,   501,    62,   279,   282,
     357,   368, -1597,  5349,   388,   393,   326,   528,   533, -1597,
   -1597, -1597, -1597, -1597, 13714,   529,  3435, -1597, -1597,  5349,
   -1597, -1597, -1597, -1597,  5349, -1597,  5349, -1597,   457,   443,
    5349,  5349, -1597,   439, -1597, -1597, 11905, -1597, -1597,   411,
     535,   575,   575, -1597,   628,   489,   470,   482, -1597,    87,
   -1597,   641, -1597, -1597, -1597, -1597,  4372,   697, -1597, -1597,
     485,   499,   519,   526,   530,   539,   541,   543, 15769, -1597,
   -1597, -1597, -1597,   325,   632,   671,   685, -1597,   691,   703,
   -1597,    92,   578, -1597,   620,   -17, -1597,  1368,   145, -1597,
   -1597,  1256,    91,   590,   334, -1597,   137,   197,   600,   200,
   -1597, -1597,   727, -1597, -1597, -1597,   653,   613,   662, -1597,
   13714, -1597,   641,   697, 18137,  2490, 18137, 13714, 18137, 18137,
   15077,   636, 17123, 15077, 18137,   785,  5349,   759,   759,   483,
     759,   759,   759,   759,   759,   759,   759,   759,   759, -1597,
   -1597, -1597, -1597, -1597, -1597, -1597,    45, 13714,   657, -1597,
   -1597,   682,   645,    33,   648,    33, 16956, 17171,   640,   834,
   -1597,   653, -1597, 13714,   657, -1597,   688, -1597,   692,   655,
   -1597,   138, -1597, -1597, -1597,    33,    91, 12106, -1597, -1597,
   13714,  8890,   843,    89, 18137,  9895, -1597, 13714, 13714,  5349,
   -1597, -1597, 15817,   656, -1597, 15887, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597, -1597, -1597,  3426, -1597,  3426, -1597,
   -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597, -1597, -1597,    85,    86,   662, -1597,
   -1597, -1597, -1597,   660,  2173,    88, -1597, -1597,   695,   842,
   -1597,   699, 15264, -1597,   664,   673, 15936, -1597,    29, 15984,
    4841,  4841,  5349,   661,   861,   675, -1597,    49, -1597, 16552,
      94, -1597,   742, -1597,   746, -1597,   864,    96, 16956, 13714,
   13714,   686,   706, -1597, -1597, 16653, 11302, 13714, 13714, 13714,
      97,   506,   444, -1597, 13915, 16956,   573, -1597,  5349, -1597,
     497,   489, -1597, -1597, -1597, -1597, 17821,   868,   791, -1597,
   -1597, -1597,    72, 13714,   700,   710, 18137,   712,   971,   713,
    5674, 13714, -1597,   414,   687,   592,   414,   276,   514, -1597,
    5349,  3426,   704,  4726, 14553, -1597, -1597,  1490, -1597, -1597,
   -1597, -1597, -1597,   641, -1597, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, 13714, 13714, 13714, 13714, 12307, 13714, 13714,
   13714, 13714, 13714, 13714, 13714, 13714, 13714, 13714, 13714, 13714,
   13714, 13714, 13714, 13714, 13714, 13714, 13714, 13714, 13714, 13714,
   13714, 17919, 13714, -1597, 13714, 13714, 13714, 14084,  5349,  5349,
    5349,  5349,  5349,  4372,   783,  1004, 10096, 13714, 13714, 13714,
   13714, 13714, 13714, 13714, 13714, 13714, 13714, 13714, 13714, -1597,
   -1597, -1597, -1597,  2979, 13714, 13714, -1597,  4726,  4726, 13714,
   13714, 16653,   715,   641, 12508, 16051, -1597, 13714, -1597,   716,
     902,   757,   721,   723, 14229,    33, 12709, -1597, 12910, -1597,
     655,   728,   729,  1827, -1597,   347,  4726, -1597,  3179, -1597,
   -1597, 16100, -1597, -1597, 10498, -1597, 13714, -1597,   831,  9091,
     920,   738, 13899,   916,   100,    57, -1597, -1597, -1597,   765,
   -1597, -1597, -1597,  3426, -1597,  2249,   744,   944, 16412,  5349,
   -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597,   761,
   -1597, -1597,   766,   762,   768,   771,    81,  4358,  5148, -1597,
   -1597,  5349,  5349, 13714,    33,   163, -1597, -1597, -1597, 16412,
     886, -1597,    33,   116,   117,   775,   776,  2080,   323,   788,
     779,   375,   848,   790,    33,   120,   803, 17227,   798,   991,
     992,   804,   806, -1597,  4028,  5349, -1597, -1597,   934,  3186,
      21, -1597, -1597, -1597,   489, -1597, -1597, -1597,   977,   877,
     833,   351,   854, 13714,   881,  1012,   830,   869, -1597,   143,
   -1597,  3426,  3426,  1014,   843,    72, -1597,   835,  1021, -1597,
    3426,    58, -1597,   472,   154, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597,  1094,  3356, -1597, -1597, -1597, -1597,  1022,   853,
   -1597, 16956, 13714,   839,  1026, 18137,  1023, -1597, -1597,   909,
    3503, 11689, 18275, 15077, 14552, 13714, 18089, 14726, 11281, 12687,
    2750, 12283, 13488, 14080, 14080, 14080, 14080,  2954,  2954,  2954,
    2954,  2954,  1237,  1237,   705,   705,   705,   483,   483,   483,
   -1597,   759, 18137,   841,   844, 17275,   846,  1037,    -9, 13714,
     384,   657,   188, -1597, -1597, -1597,  1033,   791, -1597,   641,
   16754, -1597, -1597, -1597, 15077, 15077, 15077, 15077, 15077, 15077,
   15077, 15077, 15077, 15077, 15077, 15077, 15077, -1597, 13714,   432,
   -1597,   144, -1597,   657,   445,   851,  4569,   858,   860,   855,
    4641,   121,   865, -1597, 18137, 16458, -1597,  5349, -1597,    58,
      31, 16956, 18137, 16956, 17331,   909,    58,    33,   157, -1597,
     143,   891,   870, 13714, -1597,   186, -1597, -1597, -1597,  8689,
     478, -1597, -1597, 18137, 18137,    46, -1597, -1597, -1597, 13714,
     962, 16290, 16412,  5349,  9292,   875,   876, -1597,  1057,   982,
     940,   922, -1597,  1072,   889,  2784,  3426, 16412, 16412, 16412,
   16412, 16412,   893,  1015,  1028,  1030,   899, 16412,   220, -1597,
   -1597, -1597, -1597,     1, -1597, 18231, -1597, -1597,   229, -1597,
    5875,  3108,   907,  5148, -1597,  5148, -1597,  5349,  5349,  5148,
    5148,  5349, -1597,  1087,   912, -1597,   261, -1597, -1597,  4920,
   -1597, 18231,  1097, 16956,   917, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597,   935,  1105,  5349,  3108,   919, 16653,
   16855,  1103, -1597, 13111, -1597, 13714, -1597, 13714, -1597, -1597,
   -1597, -1597, -1597, -1597,   921, -1597, 13714, -1597, -1597,  5071,
   -1597,  3426,  3108,   923, -1597, -1597, -1597, -1597,  1107,   937,
   13714, 17821, -1597, -1597, 14084,   938, -1597,  3426, -1597,   942,
    6076,  1104,    39, -1597, -1597,   148,  2979, -1597,  3179, -1597,
    3426, -1597, -1597,    33, 18137, -1597, 10699, -1597, 16412,    41,
     947,  3108,   877, -1597, -1597, 14726, 13714, -1597, -1597, 13714,
   -1597, 13714, -1597, 11085,   954,  4726,   848,  1106,   877,  3426,
    1139,   909,  5349, 17919,    33, 11487,   966, -1597, -1597,   159,
     967, -1597, -1597,  1148,   734,   734, 16458, -1597, -1597, -1597,
    1111,   975,  1102,  1109,  1110,    48,   987,   249, -1597, -1597,
   -1597,  1024, -1597, -1597, -1597, -1597,  1174,   990,   716,    33,
      33, 13312,   877,  3179, -1597, -1597, 12090,   481,    46,  9895,
   -1597,  6277,   993,  6478,   998, 16290, 16956,  1001,  1051,    33,
   18231,  1183, -1597, -1597, -1597, -1597,   614, -1597,    43,  3426,
    1019,  1064,  3426,  5349,  2249, -1597, -1597, -1597,  1192, -1597,
    1008,  1022,   540,   540,  1135,  1135, 17483,  1006,  1196, 16412,
   16412, 16412, 16412, 17821,  4194, 15409, 16412, 16412, 16412, 16412,
   16173, 16412, 16412, 16412, 16412, 16412, 16412, 16412, 16412, 16412,
   16412, 16412, 16412, 16412, 16412, 16412, 16412, 16412, 16412, 16412,
   16412, 16412, 16412, 16412,  5349, -1597, -1597,  1123, -1597, -1597,
    1013,  1025, -1597, -1597, -1597,   267,  4358, -1597,  1031, -1597,
   16412,    33, -1597, -1597,    68, -1597,   531,  1201, -1597, -1597,
     122,  1032,    33, 10900, 16956, 18137, 17379, -1597,  3058, -1597,
    5272,   791,  1201, -1597,    -6,   224, -1597, 18137,  1083,  1034,
   -1597,  1035,  1104, -1597,  3426,   843,  3426,    50,  1206,  1146,
     192, -1597,   657,   314, -1597, -1597, 16956, 13714, 18137, 18231,
    1039,    41, -1597,  1038,    41,  1045, 14726, 18137, 17435,  1046,
    4726,  1047,  1048,  3426,  1049,  1036,  3426,   877, -1597,   655,
     494,  4726, 13714, -1597, -1597, -1597, -1597, -1597, -1597,  1108,
    1042,  1238,  1158, 16458, 16458, 16458, 16458,  1095, -1597, 17821,
      59, 16458, -1597, -1597, -1597, 16956, 18137,  1055, -1597,    46,
    1221,  1177,  9895, -1597, -1597, -1597,  1059, 13714,  1051,    33,
   16653, 16290,  1061, 16412,  6679,   659,  1062, 13714,    61,   328,
   -1597,  1077, -1597,  3426, -1597,  1124, -1597,  3276,  1230,  1068,
   16412, -1597, 16412, -1597,  1069,  1065,  1258, 17538,  1067, 18231,
    1276,  1085,  1157,  1287,  1112, -1597, -1597, -1597, 17586,  1099,
    1289, 15261,  3789,  4330, 16412, 18185, 12487, 12888, 13088, 13288,
    4672, 14895, 14895, 14895, 14895,  3741,  3741,  3741,  3741,  3741,
    1529,  1529,   540,   540,   540,  1135,  1135,  1135,  1135, -1597,
    1113, -1597,  1114,  1115, -1597, -1597, 18231,  5349,  3426,  3426,
   -1597,   531,  3108,   202, -1597, 16653, -1597, -1597, 15077,    33,
   13513,  1117, -1597,  1116,  1280, -1597,    70, 13714, -1597, -1597,
   -1597, 13714, -1597, 13714, -1597,   843, -1597, -1597,   161,  1295,
    1218, 13714, -1597,  1127,    33, 18137,  1104,  1130, -1597,  1132,
      41, 13714,  4726,  1133, -1597, -1597,   791, -1597, -1597,  1126,
    1136,  1145, -1597,  1147, 16458, -1597, 16458, -1597, -1597,  1149,
    1142,  1294,  1204,  1153, -1597,  1338,  1154, -1597,  1212,  1155,
    1340, -1597, -1597,    33, -1597,  1324, -1597,  1161, -1597, -1597,
    1167,  1168,   123, -1597, -1597, 18231,  1169,  1171, -1597,  5010,
   -1597, -1597, -1597, -1597, -1597, -1597,  3426, -1597,  3426, -1597,
   18231, 17641, -1597, -1597, 16412, -1597, 16412, -1597, 16412, -1597,
   -1597, 16412, 17821, -1597, -1597, 16412, -1597, 16412, -1597, 10679,
   16412,  1172,  6880, -1597, -1597,   531, -1597, -1597, -1597, -1597,
     572, 14727,  3108,  1260, -1597,  1005,  1203,  3340, -1597, -1597,
   -1597,   783, 16097,   102,   103,  1179,   791,  1004,   134, 16956,
   18137, -1597, -1597, -1597,  1205, 13698, 15721, 18137, -1597,    54,
    1362,  1296, 13714, -1597, 18137,  4726,  1261,  1104,  1865,  1104,
    1185, 18137,  1187, -1597,  2011,  1186,  2425, -1597, -1597,    41,
   -1597, -1597,  1248, -1597, -1597, 16458, -1597, 16458, -1597, 16458,
   -1597, -1597, 16458, -1597, 17821, -1597,  2571, -1597,  8689, -1597,
   -1597, -1597, -1597,  9493, -1597, -1597, -1597,  8689, -1597,  1190,
   16412, 17689, 18231, 18231, 18231,  1250, 18231, 17744, 10679, -1597,
   -1597,   531,  3108,  3108,  5349, -1597,  1373, 15554,    75, -1597,
   14727,   791,  3542, -1597,  1209, -1597,   104,  1198,   106, -1597,
   15076, -1597, -1597, -1597,   107, -1597, -1597,   780, -1597,  1202,
   -1597,  1323,   641, -1597, 14902, -1597, 14902, -1597, -1597,  1393,
     783, -1597,    33,  3865, -1597, -1597, -1597, -1597,  1394,  1326,
   13714, -1597, 18137,  1217,  1219,  1104,   516, -1597,  1261,  1104,
   -1597, -1597, -1597, -1597,  2712,  1220, 16458,  1278, -1597, -1597,
   -1597,  1285, -1597,  8689,  9694,  9493, -1597, -1597, -1597,  8689,
   -1597, 18231, 16412, 16412, 16412,  7081,  1226,  1227, -1597, 16412,
   -1597,  3108, -1597, -1597, -1597, -1597, -1597,  3426,  1273,  1005,
   -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597,   603, -1597,  1203, -1597, -1597, -1597, -1597, -1597,
     118,   619, -1597,  1412,   111, 15264,  1323,  1414, -1597,  3426,
     641, -1597, -1597,  1233,  1417, 13714, -1597, 18137, -1597,   126,
    1235, -1597, -1597, -1597,  1104,   516, 14379, -1597,  1104, -1597,
   16458, 16458, -1597, -1597, -1597, -1597,  7282, 18231, 18231, 18231,
   -1597, -1597, -1597, 18231, -1597,  2560,  1426,  1430,  1244, -1597,
   -1597, 16412, 15076, 15076,  1385, -1597,   780,   780,   647, -1597,
   -1597, -1597, 16412,  1369, -1597,  1274,  1262,   113, 16412, -1597,
    5349, -1597, 16412, 18137,  1371, -1597,  1446, -1597,  7483,  1266,
   -1597, -1597,   516, -1597, -1597,  7684,  1263,  1343, -1597,  1363,
    1306, -1597, -1597,  1365,  3426,  1288,  1273, -1597, -1597, 18231,
   -1597, -1597,  1300, -1597,  1435, -1597, -1597, -1597, -1597, 18231,
    1461,   375, -1597, -1597, 18231,  1282, 18231, -1597,   339,  1284,
    7885, -1597, -1597, -1597,  1281, -1597,  1286,  1305,  5349,  1004,
    1302, -1597, -1597, -1597, 16412,  1303,    55, -1597,  1402, -1597,
   -1597, -1597,  8086, -1597,  3108,   907, -1597,  1313,  5349,   743,
   -1597, 18231, -1597,  1298,  1485,   649,    55, -1597, -1597,  1413,
   -1597,  3108,  1301, -1597,  1104,    65, -1597, -1597, -1597, -1597,
    3426, -1597,  1307,  1310,   114, -1597,  1314,   649,   164,  1104,
    1309, -1597,  3426,   547,  3426,    63,  1486,  1425,  1314, -1597,
    1500, -1597,   413, -1597, -1597, -1597,   166,  1497,  1429, 13714,
   -1597,   547,  8287,  3426, -1597,  3426, -1597,  8488,   156,  1499,
    1431, 13714, -1597, 18137, -1597, -1597, -1597, -1597, -1597,  1501,
    1434, 13714, -1597, 18137, 13714, -1597, 18137, 18137
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1597, -1597, -1597,  -565, -1597, -1597, -1597,   165,   -11,   -34,
     401, -1597,  -269,  -516, -1597, -1597,   412,   208,  1410, -1597,
    1742, -1597,   306, -1597,    24, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597, -1597, -1597,  -335, -1597, -1597,  -156,
     169,     5, -1597, -1597, -1597, -1597, -1597, -1597,    26, -1597,
   -1597, -1597, -1597, -1597, -1597,    28, -1597, -1597,  1041,  1044,
    1050,   -92,  -698,  -876,   560,   616,  -342,   313,  -947, -1597,
     -63, -1597, -1597, -1597, -1597,  -731,   146, -1597, -1597, -1597,
   -1597,  -333, -1597,  -600, -1597,  -445, -1597, -1597,   941, -1597,
     -44, -1597, -1597, -1069, -1597, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597,   -78, -1597,    11, -1597, -1597, -1597,
   -1597, -1597,  -159, -1597,   112,  -980, -1597, -1596,  -363, -1597,
    -128,    27,  -125,  -337, -1597,  -160, -1597, -1597, -1597,   125,
     -23,     0,    23,  -740,   -83, -1597, -1597,    20, -1597,   -14,
   -1597, -1597,    -5,   -38,   -19, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597,  -598,  -862, -1597, -1597, -1597, -1597,
   -1597,  1859, -1597, -1597, -1597, -1597, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597, -1597, -1597, -1597,  1182,   500,   367,
   -1597, -1597, -1597, -1597, -1597,   433, -1597, -1597, -1597, -1597,
   -1597, -1597, -1597, -1597,  -960, -1597,  2690,     3, -1597,  1397,
    -405, -1597, -1597,  -473,  3334,  3253, -1597,  -395, -1597, -1597,
     507,   190,  -631, -1597, -1597,   585,   380,  -658, -1597,   381,
   -1597, -1597, -1597, -1597, -1597,   565, -1597, -1597, -1597,   170,
    -894,  -165,  -430,  -428, -1597,   639,  -112, -1597, -1597,     4,
       9,   521, -1597, -1597,   348,   -20, -1597,  -354,    30,  -355,
      78,  -328, -1597, -1597,  -475,  1210, -1597, -1597, -1597, -1597,
   -1597,   681,   364, -1597, -1597, -1597,  -352,  -663, -1597,  1164,
   -1144, -1597,    52,  -186,    -3,   752, -1597,  -320, -1597,  -324,
    -812, -1241,  -236,   167, -1597,   476,   550, -1597, -1597, -1597,
   -1597,   505, -1597,  1735, -1109
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1040
static const yytype_int16 yytable[] =
{
     184,   186,   434,   188,   189,   190,   192,   193,   194,   122,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   337,   405,   227,   230,   512,   933,   120,   484,
     124,   127,   125,   251,   237,   242,   771,  1328,  1146,   254,
     243,   649,   648,   345,   650,   336,   256,   262,   534,   265,
     506,   260,   346,   947,   349,   929,   928,   785,   434,  1138,
    1428,   244,   767,   768,  1314,   483,   720,   253,   407,  1024,
     909,   862,   867,   760,   970,   761,  1163,   344,  1218,   254,
     538,   583,   585,   404,  1611,  1038,   409,  1012,    14,   815,
      14,   792,  1174,  1390,   -72,   -37,   424,   -36,   546,   -72,
     -37,   406,   -36,   595,    14,   600,   546,   380,   795,   813,
     796,  1554,  1556,  -351,    14,  1619,  1704,  1005,   882,   883,
    1773,    14,  1773,  1611,  1454,   873,   546,  1766,   504,   890,
     890,   890,   890,  1329,   539,   521,  1207,  1568,  1197,  1790,
    1371,  -704,   407,   890,   579,   366,  1907,   724,   394,  1227,
    1228,   591,   397,   398,  1767,   501,   502,   404,  1315,  1065,
     409,   504,  -885,   187,  1784,   501,   502,   501,   502,   114,
    -101,  1316,     3,   121,   157,   406,   765,  -898,   430,   523,
     790,   769,   515,   857,   485,  -101,  1147,   412,  -535,  1256,
     247,  1317,   501,   502,   522,   532,  1833,   409,  1066,  1459,
     383,  1252,  1895,  1198,  1918,   580,  1430,  -886,  -928,  1785,
     531,   592,   406,   509,  -889,  -888,   625,  -596,   263,   908,
    -705,   335,  1330,  -594,  -892,  -900,  1569,  -931,   406,  -891,
    -798,  1148,  1107,   395,  -798,  1908,  -290,   347,   372,  1929,
     250,  1230,   541,   858,  1460,   541,   358,  1896,  -796,  1919,
    -274,    14,   254,   552,   816,   505,  -930,  1391,  -893,   392,
    -798,   372,  -872,  -290,   563,   372,   372,  -887,   357,  1468,
     509,  1612,  1613,  1177,  1383,   543,  1474,   690,  1476,   548,
    1428,   -72,   -37,   425,   -36,   547,  -896,  1526,   505,  -885,
     596,   372,   601,   623,   390,   814,   433,   391,  1555,  1557,
    -351,  1350,  1620,  1705,  1225,  1496,  1229,  1774,   574,  1823,
    1891,   874,   875,  1768,  1431,   891,   983,  1304,  1502,  1432,
    1149,    62,    63,    64,   174,  1433,   431,  1434,  1930,  1561,
    1325,   336,   606,  1461,  -886,  -928,  1897,  1368,  1920,  -895,
     510,  -889,  -888,  1103,  1104,  1130,  -712,   605,  -899,  1160,
     609,  -892,   513,  -902,  -931,  -711,  -891,   772,  -706,   381,
     434,   519,   359,   591,   254,   406,   631,  1435,  1436,   857,
    1437,   227,   617,   254,   254,   224,   224,  1848, -1003,   629,
    1002,   257,   965,  -930,  -873,  -893,  1614,  1094,  1320,  -872,
     337,   432,   604,  -593,  -887,  -588,   377,   510,   192,   421,
    1438,   620,   620,   741,  -593,   915,   673,  -117,   421,  1584,
    1717,  -117,  1718,   336,  1120,   258,   114, -1003,   685,   529,
     114,  1321,  1849,   420,   553,   882,   883,  1915,  -117,   420,
     210,    40,   384,   385,  1227,  1228,   259,   127,   691,   692,
     693,   694,   696,   697,   698,   699,   700,   701,   702,   703,
     704,   705,   706,   707,   708,   709,   710,   711,   712,   713,
     714,   715,   716,   717,   718,   719,   730,   721,  1313,   722,
     722,   725,   743,   368,   237,   242,   369,  1380,   675,   688,
     243,   744,   745,   746,   747,   748,   749,   750,   751,   752,
     753,   754,   755,   756,   210,    40,   936,   381,   938,   722,
     766,   244,   685,   685,   722,   770,   742,   573,  1153,   744,
    1154,  -873,   774,   336,   916,  1298,  1299,   508,   879,  -897,
    1171,   782,   399,   784,  1466,   161,  1393,   381,   508,   917,
     802,   685,   394,   736,   737,   430,  1362,  1364,  1364,   803,
     471,   804,  -595,  1372,   484,   964,   109,  1327,   223,   225,
    1515,   370,   472,  1018,  1019,   732,  1209,  1210,   357,   357,
     586,  1337,   371,  -100,  1339,   789,   649,   648,  1179,   650,
     384,   385,   659,   807,   224,   114,   624,   976,  -100,   121,
     483,   764,   375,   972,  1100,   677,  1101,   376,   869,   335,
     862,  1002,   372,  1090,  1091,  1092,   634,   381,   378,   626,
     384,   385,   732,   379,   631,   919,   501,   502,   411,  1093,
     109,  -860,   381,   791,   936,   938,   797,  1761,   381,   382,
     395,  1013,   938,   364,  -863,   413,  -860,   408, -1003,   400,
    -707,   365,  1591,  1298,  1299,  1762,   401,   396,   406,  -863,
     501,   502,   573,   372,   734,   372,   372,   372,   372,   421,
    1769,   420,   534,   739,  1763,  1576,   381,  1578,   381,   420,
     419,  1916,  1095,   631, -1003,   416,  -900, -1003,   759,  1770,
     384,   385,  1771,  -861,  1014,   381,   423,   944,  1816,   426,
     362,   435,   631,  1475,   383,   384,   385,   363,  -861,   573,
     955,   384,   385,   636,   637,   436,  1481,  1817,  1482,   485,
    1818,   408,  -589,   794,  1522,  1523,  1352,   224,   649,   648,
     778,   650,  1731,  1732,   114,   437,   224,   676,   608,  1226,
    1227,  1228,   438,   224,   963,  1343,   439,  1458,   632,   384,
     385,   384,   385,   224,   848,   440,  1353,   441,   408,   442,
    1470,  -590,   954,  1903,  1904,  1382,   627,   525,   384,   385,
     633,   582,   584,   975,   533,  -591,   868,   677,   468,   469,
     470,   474,   471,  1730,  1387,  1227,  1228,  1735,  1877,  1878,
    1879,  1888,   161,   475,   472,   476,   161,   627,   477,   633,
     627,   633,   633,  1559,   507,  1906,  1814,  1815,  1016,   903,
     905,   415,   417,   418,  -894,   866,   866,  -592,  1002,  1002,
    1002,  1002,  1810,  1811,   254,  1022,  1002,   511,    55,   896,
     898,  -705,  1183,  1184,  1185,   209,    62,    63,    64,   174,
     175,   431,  1048,  1051,   388,   659,   127,  1587,   472,  1588,
    1448,  1589,   516,   518,  1590,   922,   421,    50,  1033,  -898,
     524,   527,   508,   528,   649,   648,  -703,   650,   799,   536,
     535,   544,   557, -1039,   565,   372,   568,   569,   587,   224,
     575,   209,    62,    63,    64,   174,   175,   431,  1615,   576,
     588,   590,   597,   213,   214,   215,   598,   127,   599,  1585,
     594,   610,   651,    50,   850,   611,   432,   674,  1125,   602,
    1126,   607,   804,   652,    55,   661,   614,  1472,  -122,    91,
      92,  1128,    93,   179,    95,   662,   630,   663,   665,   687,
     775,   777,   626,   730,   122,  1137,   779,   962,   780,   213,
     214,   215,  1789,   786,   787,   805,  1792,   105,  1739,   546,
     812,   161,   432,   120,   809,   124,   127,   125,   825,  1873,
     997,  1158,  1008,  1706,   563,    91,    92,  1707,    93,   179,
      95,  1166,  1865,   826,  1167,   851,  1168,   127,   854,  1002,
     685,  1002,   930,   853,   114,   855,  1333,   856,   121,   872,
     876,   877,  1865,   105,  1547,   881,   237,   242,  1031,   114,
     887,  1887,   243,   880,   889,   514,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   892,   894,
     895,   897,  1139,   244,   906,   899,  1206,   900,  1040,   911,
     912,   914,  -727,  1045,   764,   114,   797,   920,  1212,   121,
    1573,   921,  1102,   677,  1178,   923,   931,   924,   927,   224,
     932,   940,   614,   942,   945,   946,  1532,   948,   499,   500,
    1306,   951,   957,  1213,   960,   958,   961,   969,   127,  -709,
     127,  1116,   977,   979,  1257,   980,   981,   649,   648,   953,
     650,   866,  1886,   866,  1025,  1015,  1039,   866,   866,  1105,
     161,  1035,  1037,  1041,   114,  1115,  1042,  1898,   121,   157,
    1043,  1044,  1793,  1794,  1046,  1060,   209,  1059,   224,   573,
    1002,   797,  1002,  1063,  1002,   114,  1106,  1002,  1061,   121,
    1062,   759,   659,   794,  1098,   501,   502,  1021,    50,  1307,
    1108,  1110,  1023,  1112,  1114,  1113,  1119,  1123,  1308,   659,
    1133,  1135,  1127,    62,    63,    64,   174,   175,   431,   224,
    1533,   224,  1136,  1142,  1140,   122,  1144,   372,  1173,   649,
     648,  1161,   650,  1534,   213,   214,   215,  1535,  1170,  1186,
    1186,   997,  1335,  1176,   120,  1829,   124,   127,   125,   224,
    1181,  -901,  1182,  1192,   178,   685,   664,    89,  1536,  1193,
      91,    92,  1194,    93,  1537,    95,   685,  1308,   794,  1195,
    1196,  1199,  1200,  1201,   114,  1203,   114,  1221,   114,  1215,
     121,  1002,   121,   432,  1217,  1220,  1202,  1223,   105,  1232,
    1233,  1237,   943,  1238,  1093,  1242,  1290,  1241,  1235,  1375,
    1302,  1292,   254,    62,    63,    64,    65,    66,   431,  1323,
    1331,   224,  1389,  1293,    72,   478,  1305,  1295,   965,  1332,
     573,  1324,  1348,  1876,  1336,  1338,  1378,   224,   224,  1340,
    1342,  1239,  1344,  1355,  1354,  1345,  1347,  1356,  1243,   990,
    1367,   127,  1374,  1376,  1377,  1379,  1384,  1394,  1388,   848,
    1396,   974,  1398,  1399,  1402,   480,  1403,  1404,  1407,  1914,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   432,  1430,  1408,  1410,   465,   466,   467,
     468,   469,   470,  1411,   471,   114,  1412,  1416,  1417,   121,
     157,  1463,  1009,  1485,  1010,  1450,   472,  1414,  1421,  1462,
    1423,  1424,  1455,  1452,   866,  1451,  1456,   434,  1457,  1560,
     161,  1465,  1477,   499,   500,  1467,  1464,  1469,  1473,    14,
    1442,   659,  1029,  1478,   659,   161,  1471,   685,  1224,  1479,
    1487,  1442,  1480,  1484,  1483,  1002,  1002,  1489,  1492,  1494,
    1493,    34,    35,    36,  1488,  1491,  1497,  1499,   997,   997,
     997,   997,  1500,  1501,   211,  1504,   997,  1505,  1564,  1519,
    1543,   161,  1530,   224,   224,  1558,  1570,   114,  1574,  1571,
    1579,  1357,  1580,  1582,  1586,  1600,  1603,  1609,  1617,   114,
     501,   502,  1431,   121,  1111,  1618,  1715,  1432,  1712,    62,
      63,    64,   174,  1433,   431,  1434,  1713,  1719,  1725,  1726,
     614,  1122,  1728,  1729,  1740,  1738,    79,    80,    81,    82,
      83,  1741,  1751,  1752,   221,   221,  1772,   216,  1778,  1781,
     161,  1782,  1787,    87,    88,  1804,  1405,   218,   218,  1806,
    1409,   234,  1808,  1413,  1812,  1435,  1436,    97,  1437,   127,
    1418,   161,  1820,  1821,  1827,  1828,  1822,  1572,  1836,  1835,
     685,   102,  1425,  1832,  1838,  -347,   234,  1839,  1841,   432,
    1843,  1767,   224,  1447,   485,  1844,  1847,  1853,  1453,  1850,
    1854,  1855,  1860,  1862,  1447,  1867,  1871,    62,    63,    64,
      65,    66,   431,  1874,  1875,  1442,  1883,  1885,    72,   478,
    1909,  1442,  1889,  1442,   224,  1890,  1899,  1892,  1910,  1913,
     659,  1921,  1922,  1931,  1932,  1934,  1610,  1935,  1294,   997,
    1870,   997,   733,  1442,   738,   127,  1172,  1597,  1132,  1884,
     735,  1381,  1745,  1882,   127,  1506,   870,   479,  1736,   480,
     161,  1760,   161,  1616,   161,  1765,  1029,  1219,  1924,  1550,
    1894,  1486,   481,   224,   482,  1490,  1777,   432,  1531,  1734,
    1495,   622,  1251,  1366,  1780,  1727,  1244,  1318,   224,   224,
    1188,   209,  1358,  1204,  1359,  1152,  1608,  1049,  1911,  1087,
    1088,  1089,  1090,  1091,  1092,  1845,   616,   114,   686,  1926,
    1297,   121,  1521,    50,  1236,     0,   335,  1422,  1093,  1289,
       0,     0,  1548,     0,     0,     0,     0,     0,     0,     0,
    1552,  1442,     0,     0,     0,     0,     0,     0,  1743,  1597,
     127,     0,     0,   221,     0,     0,   127,     0,     0,   213,
     214,   215,   127,     0,     0,     0,   218,     0,  1447,     0,
       0,     0,     0,     0,  1447,  1309,  1447,     0,     0,   659,
     997,   161,   997,   224,   997,    91,    92,   997,    93,   179,
      95,     0,     0,   114,     0,     0,  1447,   121,   114,     0,
       0,     0,   114,     0,     0,     0,   121,  1334,     0,     0,
       0,  1775,  1498,   105,   687,     0,   234,     0,   234,   372,
       0,     0,   573,     0,     0,   335,     0,     0,  1507,     0,
       0,     0,     0,     0,   336,  1701,     0,     0,     0,     0,
       0,  1858,  1708,     0,     0,     0,     0,     0,     0,   335,
    1783,   335,     0,     0,     0,     0,  1373,     0,   335,  1720,
       0,     0,     0,   161,   434,     0,     0,     0,     0,     0,
       0,   614,  1029,     0,   234,   161,     0,     0,     0,     0,
       0,   997,     0,     0,  1447,     0,   221,     0,   114,   114,
     114,     0,   121,     0,   114,   221,     0,     0,   121,   218,
     114,     0,   221,   127,   121,     0,     0,     0,   218,     0,
       0,     0,   221,     0,     0,   218,     0,     0,     0,     0,
       0,   342,     0,   647,     0,   218,     0,   224,   338,     0,
       0,     0,     0,  1593,     0,     0,   234,     0,     0,     0,
       0,     0,  1599,     0,     0,   127,     0,     0,     0,     0,
       0,     0,   127,     0,     0,  1605,   614,     0,     0,     0,
       0,   234,  1825,     0,   234,     0,     0,     0,     0,     0,
       0,   514,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,     0,     0,     0,   127,     0,     0,
       0,     0,     0,     0,     0,     0,  1859,     0,     0,  1430,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   127,
     573,   234,     0,     0,     0,     0,   219,   219,     0,     0,
       0,     0,     0,     0,   499,   500,     0,     0,     0,     0,
       0,   335,  1746,     0,  1923,   997,   997,     0,   221,     0,
       0,   114,     0,     0,    14,   121,  1933,     0,     0,     0,
    1799,   218,     0,     0,     0,     0,  1936,  1701,  1701,  1937,
       0,  1708,  1708,     0,     0,     0,     0,     0,     0,   127,
       0,     0,     0,   161,   127,   372,  1805,  1807,     0,     0,
       0,     0,     0,   114,     0,     0,     0,   121,     0,     0,
     114,   501,   502,     0,   121,     0,     0,     0,     0,     0,
    1562,     0,     0,   234,     0,   234,     0,  1431,   838,     0,
       0,     0,  1432,     0,    62,    63,    64,   174,  1433,   431,
    1434,     0,     0,     0,     0,   114,     0,     0,     0,   121,
       0,     0,     0,  1857,     0,     0,     0,     0,     0,   838,
       0,   561,     0,   562,     0,  1430,   659,   114,   338,   161,
     338,   121,   788,  1872,   161,     0,     0,     0,   161,     0,
    1435,  1436,     0,  1437,     0,     0,   659,     0,  1788,     0,
       0,     0,     0,     0,     0,   659,     0,     0,  1795,     0,
       0,     0,     0,     0,   432,     0,     0,     0,     0,     0,
      14,   234,   234,  1577,     0,     0,     0,     0,     0,   567,
     234,     0,     0,     0,     0,     0,   338,   114,   221,     0,
       0,   121,   114,     0,     0,   219,   121,     0,     0,     0,
       0,   218,     0,  1830,   514,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,     0,     0,     0,
       0,     0,     0,     0,   161,   161,   161,     0,     0,     0,
     161,     0,     0,  1431,     0,     0,   161,     0,  1432,     0,
      62,    63,    64,   174,  1433,   431,  1434,   221,  1852,     0,
       0,     0,     0,     0,     0,     0,     0,   499,   500,     0,
     218,     0,     0,     0,     0,     0,   680,     0,     0,   342,
       0,     0,     0,   338,     0,     0,   338,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1435,  1436,   221,  1437,
     221,     0,     0,     0,     0,   234,     0,     0,     0,     0,
       0,   218,     0,   218,     0,     0,     0,     0,     0,     0,
     432,     0,     0,     0,   276,  1912,     0,     0,   221,  1581,
    1917,     0,     0,     0,   501,   502,     0,     0,   219,     0,
       0,   218,   838,     0,     0,     0,     0,   219,     0,     0,
       0,     0,   278,     0,   219,   234,   234,   838,   838,   838,
     838,   838,     0,     0,   219,     0,     0,   838,     0,     0,
       0,     0,     0,     0,   209,     0,     0,     0,     0,     0,
       0,   234,     0,     0,     0,     0,     0,   161,     0,     0,
     221,     0,     0,     0,     0,   878,    50,     0,     0,     0,
     819,     0,     0,   218,   566,     0,   221,   221,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   234,   818,   218,
     218,     0,     0,     0,     0,   338,     0,   821,     0,   161,
       0,   559,   213,   214,   215,   560,   161,     0,   647,     0,
       0,   234,   234,     0,     0,     0,     0,     0,     0,     0,
     209,   234,   178,     0,     0,    89,   329,   234,    91,    92,
     820,    93,   179,    95,     0,     0,     0,     0,     0,     0,
     234,   161,    50,     0,     0,     0,   333,     0,   838,     0,
       0,   234,     0,     0,     0,     0,   105,   334,     0,     0,
     219,     0,     0,   161,     0,     0,     0,     0,     0,   234,
       0,     0,     0,   234,     0,     0,   925,   926,   213,   214,
     215,     0,     0,   338,   338,   934,   234,     0,     0,     0,
       0,     0,   338,     0,     0,     0,     0,     0,   178,     0,
       0,    89,     0,     0,    91,    92,     0,    93,   179,    95,
       0,     0,   221,   221,     0,     0,     0,     0,     0,  1430,
       0,     0,     0,   161,     0,   218,   218,     0,   161,     0,
       0,     0,   105,     0,     0,     0,     0,     0,     0,   234,
       0,     0,   234,     0,   234,     0,     0,     0,     0,     0,
     647,     0,     0,     0,     0,     0,     0,     0,     0,   838,
     838,   838,   838,   234,    14,     0,   838,   838,   838,   838,
     838,   838,   838,   838,   838,   838,   838,   838,   838,   838,
     838,   838,   838,   838,   838,   838,   838,   838,   838,   838,
     838,   838,   838,   838,   514,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,     0,     0,     0,
     838,   221,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   218,     0,     0,  1431,     0,     0,
     219,     0,  1432,     0,    62,    63,    64,   174,  1433,   431,
    1434,     0,     0,   221,   234,     0,   234,   499,   500,     0,
     680,   680,     0,     0,     0,     0,   218,   338,   338,     0,
       0,     0,     0,     0,     0,  1430,     0,     0,     0,     0,
       0,     0,     0,   234,     0,     0,   234,     0,     0,     0,
    1435,  1436,     0,  1437,     0,     0,   647,     0,     0,   219,
       0,     0,   221,   234,   234,   234,   234,     0,     0,   234,
       0,   234,     0,     0,   432,   218,     0,   221,   221,     0,
      14,     0,     0,  1583,   501,   502,     0,     0,     0,     0,
     218,   218,     0,   838,     0,     0,     0,     0,     0,     0,
     219,   209,   219,   234,     0,     0,  1131,   234,     0,     0,
     838,     0,   838,   338,     0,     0,     0,     0,     0,     0,
       0,     0,  1141,    50,     0,     0,     0,     0,     0,   338,
     219,     0,     0,     0,   838,  1155,     0,     0,     0,     0,
       0,     0,   338,  1431,     0,     0,     0,     0,  1432,     0,
      62,    63,    64,   174,  1433,   431,  1434,     0,     0,   213,
     214,   215,   221,     0,  1175,     0,     0,     0,   234,   234,
       0,   338,   234,     0,     0,   218,  1430,   220,   220,   178,
       0,   236,    89,     0,     0,    91,    92,     0,    93,   179,
      95,     0,   219,     0,     0,     0,  1435,  1436,     0,  1437,
       0,     0,     0,     0,     0,     0,     0,     0,   219,   219,
       0,     0,     0,   105,     0,     0,     0,     0,  1798,     0,
     432,    14,     0,     0,   234,     0,   234,     0,     0,  1592,
       0,     0,     0,     0,  1231,     0,     0,  1234,     0,     0,
       0,   338,     0,     0,   338,     0,   821,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   234,   471,   234,   647,
       0,     0,     0,     0,   838,   276,   838,     0,   838,   472,
       0,   838,   234,     0,  1431,   838,     0,   838,     0,  1432,
     838,    62,    63,    64,   174,  1433,   431,  1434,     0,     0,
       0,   234,   234,   278,     0,   234,   221,     0,     0,     0,
       0,     0,   234,     0,     0,     0,     0,     0,     0,   218,
       0,     0,     0,     0,     0,   209,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1435,  1436,  1326,
    1437,   934,     0,     0,   219,   219,   338,    50,   338,     0,
       0,   647,     0,     0,     0,   234,     0,   234,     0,   234,
       0,   432,   234,     0,   234,     0,     0,     0,  1346,     0,
    1737,  1349,     0,     0,     0,   338,   220,     0,   338,     0,
     838,     0,   559,   213,   214,   215,   560,     0,     0,     0,
       0,     0,   234,   234,     0,     0,     0,     0,     0,     0,
     234,     0,   234,   178,     0,     0,    89,   329,     0,    91,
      92,     0,    93,   179,    95,     0,  1047,     0,     0,     0,
       0,     0,     0,     0,   234,     0,   234,   333,  1395,     0,
       0,     0,  1155,   234,     0,   338,     0,   105,   334,   338,
       0,     0,     0,   219,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   234, -1040, -1040, -1040,
   -1040, -1040,   463,   464,   465,   466,   467,   468,   469,   470,
       0,   471,   838,   838,   838,   219,     0,     0,     0,   838,
       0,   234,     0,   472,     0,     0,     0,   234,     0,   234,
       0,     0,     0,  1426,  1427,     0,     0,     0,     0,     0,
     338,   338,     0,     0,     0,     0,     0,     0,     0,   220,
       0,     0,     0,     0,     0,     0,     0,     0,   220,     0,
     209,     0,   210,    40,   219,   220,     0,     0,   443,   444,
     445,     0,     0,     0,     0,   220,     0,     0,     0,   219,
     219,     0,    50,     0,     0,     0,   220,     0,   446,   447,
       0,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,     0,   471,     0,     0,   213,   214,
     215,     0,     0,     0,     0,     0,     0,   472,     0,   234,
       0,  1508,     0,  1509,     0,     0,     0,     0,   338,     0,
     338,     0,   757,     0,    91,    92,   234,    93,   179,    95,
     234,   234,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   236,     0,     0,   219,   234,     0,     0,     0,     0,
       0,   838,   105,   338,     0,     0,   758,  1553,   109,     0,
       0,     0,   838,     0,   338,     0,     0,     0,   838,   209,
       0,     0,   838,     0,     0,     0,   443,   444,   445,     0,
       0,   220,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,   234,     0,   446,   447,     0,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,     0,   471,     0,     0,     0,   213,   214,   215,
       0,     0,     0,     0,   838,   472,     0,     0,   843,  1311,
     209,     0,   210,    40,   234,     0,     0,   178,     0,     0,
      89,    90,   338,    91,    92,     0,    93,   179,    95,     0,
       0,   234,    50,     0,     0,     0,     0,     0,     0,   843,
     234,     0,     0,     0,     0,     0,   338,     0,   338,     0,
       0,   105,   234,     0,   234,   338,     0,   276,   219,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   213,   214,
     215,     0,     0,   234,     0,   234,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   278,     0,     0,     0,     0,
       0,     0,   757,     0,    91,    92,     0,    93,   179,    95,
       0,     0,  1756,     0,     0,     0,     0,   209,     0,   338,
       0,   222,   222,     0,     0,   240,   443,   444,   445,     0,
       0,   220,   105,     0,     0,     0,   793,     0,   109,    50,
       0,     0,     0,     0,   907,     0,   446,   447,     0,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,     0,   471,   559,   213,   214,   215,   560,     0,
       0,   209,     0,     0,     0,   472,     0,     0,     0,     0,
     220,     0,     0,     0,     0,   178,     0,     0,    89,   329,
       0,    91,    92,    50,    93,   179,    95,     0,  1397,     0,
       0,     0,     0,     0,  1779,     0,     0,   276,     0,   333,
       0,   338,     0,     0,     0,  1001,     0,     0,  1546,   105,
     334,   220,     0,   220,     0,     0,     0,     0,   338,   213,
     214,   215,     0,     0,     0,   278,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1800,     0,     0,
       0,   220,   843,     0,     0,    91,    92,   209,    93,   179,
      95,     0,     0,     0,     0,     0,   209,   843,   843,   843,
     843,   843,     0,     0,     0,     0,     0,   843,     0,    50,
       0,     0,     0,   105,  1547,     0,     0,     0,    50,  1840,
       0,  1097,     0,     0,     0,     0,   338,     0,     0,     0,
       0,     0,     0,     0,   939,     0,     0,     0,     0,     0,
     222,     0,     0,   220,   559,   213,   214,   215,   560,     0,
       0,     0,     0,     0,   213,   214,   215,  1118,     0,   220,
     220,     0,     0,     0,   209,   178,     0,     0,    89,   329,
       0,    91,    92,     0,    93,   179,    95,   388,     0,     0,
      91,    92,  1118,    93,   179,    95,    50,     0,     0,   333,
       0,   220,     0,     0,     0,   934,     0,     0,     0,   105,
     334,     0,   338,   209,     0,     0,     0,  1902,   105,   934,
       0,     0,   389,     0,   338,     0,   338,     0,   843,     0,
       0,  1162,   213,   214,   215,    50,     0,     0,  1902,     0,
    1927,     0,     0,     0,     0,   338,     0,   338,     0,     0,
       0,     0,     0,   236,     0,     0,     0,  1533,    91,    92,
       0,    93,   179,    95,     0,     0,  1001,     0,     0,     0,
    1534,   213,   214,   215,  1535,     0,     0,     0,     0,     0,
       0,     0,     0,   222,     0,     0,   105,   953,     0,     0,
       0,   178,   222,     0,    89,    90,     0,    91,    92,   222,
      93,  1537,    95,     0,     0,   220,   220,     0,     0,   222,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     240,     0,     0,     0,     0,   105,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   843,
     843,   843,   843,   220,     0,     0,   843,   843,   843,   843,
     843,   843,   843,   843,   843,   843,   843,   843,   843,   843,
     843,   843,   843,   843,   843,   843,   843,   843,   843,   843,
     843,   843,   843,   843, -1040, -1040, -1040, -1040, -1040,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,     0,     0,
     843,  1069,     0,     0,     0,   240,     0,     0,     0,     0,
    1093,     0,     0,     0,   220,     0,     0,     0,     0,     0,
    1070,   845,     0,  1071,  1072,  1073,  1074,  1075,  1076,  1077,
    1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,
    1088,  1089,  1090,  1091,  1092,   222,   220,     0,     0,     0,
       0,     0,   871,     0,     0,     0,     0,     0,  1093,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   266,   267,
       0,   268,   269,     0,     0,   270,   271,   272,   273,     0,
       0,     0,     0,  1001,  1001,  1001,  1001,     0,     0,   220,
       0,  1001,   274,     0,   275,   220,   276,     0,     0,     0,
       0,     0,   844,     0,     0,     0,     0,     0,     0,     0,
     220,   220,     0,   843,     0,     0,     0,     0,     0,     0,
       0,     0,   277,     0,   278,     0,     0,     0,     0,     0,
     843,     0,   843,   844,     0,     0,   279,   280,   281,   282,
     283,   284,   285,     0,     0,     0,   209,     0,     0,     0,
       0,     0,     0,     0,   843,     0,     0,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,    50,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,  1429,   320,     0,   220,   322,   323,   324,     0,
       0,     0,   325,   326,   213,   214,   215,   327,     0,     0,
       0,     0,     0,     0,     0,   222,     0,     0,     0,     0,
       0,     0,     0,     0,   328,     0,     0,    89,   329,     0,
      91,    92,     0,    93,   179,    95,   330,     0,   331,     0,
       0,   332,     0,     0,  1001,     0,  1001,     0,   333,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   105,   334,
       0,     0,     0,  1721,     0,  1030,     0,     0,     0,   209,
       0,     0,     0,     0,   222,     0,     0,     0,     0,     0,
    1052,  1053,  1054,  1055,     0,     0,     0,     0,     0,     0,
    1064,    50,     0,     0,   843,     0,   843,     0,   843,   350,
     351,   843,   220,     0,     0,   843,     0,   843,     0,   209,
     843,   901,     0,   902,     0,   222,     0,   222,     0,     0,
       0,     0,  1529,     0,     0,  1542,     0,   213,   214,   215,
       0,    50,     0,     0,     0,     0,     0,     0,     0,   220,
       0,     0,     0,     0,     0,   222,   844,     0,     0,     0,
     352,     0,     0,    91,    92,     0,    93,   179,    95,     0,
       0,   844,   844,   844,   844,   844,     0,   213,   214,   215,
       0,   844,     0,     0,     0,  1001,     0,  1001,     0,  1001,
       0,   105,  1001,     0,   220,     0,     0,     0,     0,     0,
       0,     0,     0,    91,    92,     0,    93,   179,    95,     0,
     843,  1159,     0,     0,  1067,  1068,  1069,   222,     0,     0,
       0,     0,  1606,  1607,     0,     0,     0,     0,     0,     0,
       0,   105,  1542,   222,   222,  1070,     0,     0,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
       0,     0,     0,     0,     0,   240,     0,     0,     0,     0,
       0,     0,     0,  1093,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1001,     0,     0,     0,
       0,     0,   844,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   843,   843,   843,     0,     0,     0,     0,   843,
       0,  1754,     0,     0,     0,     0,     0,   240,     0,  1542,
       0,     0,     0,  1247,  1249,  1249,     0,     0,     0,  1258,
    1261,  1262,  1263,  1265,  1266,  1267,  1268,  1269,  1270,  1271,
    1272,  1273,  1274,  1275,  1276,  1277,  1278,  1279,  1280,  1281,
    1282,  1283,  1284,  1285,  1286,  1287,  1288,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   222,
     222,  1070,     0,  1296,  1071,  1072,  1073,  1074,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,  1092,     0,     0,     0,  1255,
       0,     0,     0,   844,   844,   844,   844,   240,     0,  1093,
     844,   844,   844,   844,   844,   844,   844,   844,   844,   844,
     844,   844,   844,   844,   844,   844,   844,   844,   844,   844,
     844,   844,   844,   844,   844,   844,   844,   844,     0,     0,
    1001,  1001,     0,     0,     0,     0,     0,     0,     0,   209,
       0,     0,     0,     0,   844,     0,     0,     0,     0,     0,
       0,   843,     0,   209,     0,     0,     0,     0,   222,     0,
       0,    50,   843,     0,     0,     0,     0,     0,   843,   859,
     860,     0,   843,     0,     0,    50,  1385,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     222,     0,     0,  1400,     0,  1401,     0,   213,   214,   215,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   213,   214,   215,     0,     0,     0,  1419,     0,     0,
     861,     0,     0,    91,    92,     0,    93,   179,    95,     0,
       0,     0,     0,   240,   843,   428,     0,    91,    92,   222,
      93,   179,    95,     0,  1869,     0,     0,     0,     0,     0,
       0,   105,     0,     0,   222,   222,     0,   844,     0,     0,
       0,  1529,     0,     0,     0,   105,     0,     0,     0,     0,
       0,     0,     0,     0,   844,     0,   844,     0,     0,   443,
     444,   445,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   844,   446,
     447,     0,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,     0,   471,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   472,   222,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   443,   444,   445,     0,     0,     0,  1511,     0,  1512,
       0,  1513,     0,     0,  1514,     0,     0,     0,  1516,     0,
    1517,   446,   447,  1518,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,     0,   471,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     472,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,     5,
       6,     7,     8,     9,     0,     0,     0,     0,   844,    10,
     844,  1093,   844,     0,     0,   844,   240,     0,     0,   844,
       0,   844,     0,     0,   844,     0,     0,     0,     0,     0,
       0,     0,     0,  1601,   681,     0,     0,   978,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,   222,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   240,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,   982,
       0,     0,     0,     0,   844,    62,    63,    64,   174,   175,
     176,     0,     0,    69,    70,  1747,  1748,  1749,     0,     0,
       0,     0,  1753,   177,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
      85,     0,     0,     0,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,     0,   682,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
     180,     0,   209,     0,     0,   109,   110,     0,   111,   112,
     443,   444,   445,     0,     0,     0,   844,   844,   844,     0,
       0,     0,     0,   844,    50,     0,     0,     0,     0,     0,
     446,   447,  1759,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,     0,   471,     0,     0,
     213,   214,   215,     0,     0,     0,     0,     0,     0,   472,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   352,     0,     0,    91,    92,     0,    93,
     179,    95,     0,     0,  1809,     0,     0,     0,     0,     0,
     443,   444,   445,     0,     0,  1819,     0,     0,     0,     0,
       0,  1824,     0,     0,   105,  1826,     0,     0,     0,     0,
     446,   447,  1390,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,     0,   471,     0,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,   472,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   844,     0,  1861,    11,    12,
      13,     0,     0,     0,     0,     0,   844,     0,     0,     0,
       0,     0,   844,     0,     0,     0,   844,     0,  1109,     0,
      14,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
    1842,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,   844,    52,
      53,    54,    55,    56,    57,    58,     0,    59,    60,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
      71,    72,    73,     0,     0,     0,  1391,     0,    74,    75,
      76,    77,    78,     0,    79,    80,    81,    82,    83,     0,
       0,     0,    84,     0,     0,    85,     0,     0,     0,   209,
      86,    87,    88,    89,    90,     0,    91,    92,     0,    93,
      94,    95,    96,     0,     0,    97,     0,     0,    98,     0,
       0,    50,     0,     0,    99,   100,     0,   101,     0,   102,
     103,   104,     0,     0,   105,   106,     0,   107,   108,  1129,
     109,   110,     0,   111,   112,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,   213,   214,   215,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
     861,     0,     0,    91,    92,     0,    93,   179,    95,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,   105,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,    56,    57,    58,     0,    59,    60,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
     209,    86,    87,    88,    89,    90,     0,    91,    92,     0,
      93,    94,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,    50,     0,     0,    99,   100,     0,   101,     0,
     102,   103,   104,     0,     0,   105,   106,     0,   107,   108,
    1312,   109,   110,     0,   111,   112,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,   213,   214,
     215,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    91,    92,     0,    93,   179,    95,
       0,     0,    14,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,   105,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,    56,    57,    58,     0,    59,
      60,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,     0,    79,    80,    81,    82,
      83,     0,     0,     0,    84,     0,     0,    85,     0,     0,
       0,     0,    86,    87,    88,    89,    90,     0,    91,    92,
       0,    93,    94,    95,    96,     0,     0,    97,     0,     0,
      98,     0,     0,     0,     0,     0,    99,   100,     0,   101,
       0,   102,   103,   104,     0,     0,   105,   106,     0,   107,
     108,     0,   109,   110,     0,   111,   112,     5,     6,     7,
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
     107,   108,   666,   109,   110,     0,   111,   112,     5,     6,
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
       0,   107,   108,  1096,   109,   110,     0,   111,   112,     5,
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
     106,     0,   107,   108,  1143,   109,   110,     0,   111,   112,
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
     105,   106,     0,   107,   108,  1214,   109,   110,     0,   111,
     112,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,  1216,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
       0,    79,    80,    81,    82,    83,     0,     0,     0,    84,
       0,     0,    85,     0,     0,     0,     0,   178,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   179,    95,    96,
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
    1386,     0,    50,    51,     0,     0,     0,    52,    53,    54,
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
     104,     0,     0,   105,   106,     0,   107,   108,  1520,   109,
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
     103,   104,     0,     0,   105,   106,     0,   107,   108,  1750,
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
      48,  1796,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   178,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   179,    95,    96,     0,     0,    97,     0,     0,    98,
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
       0,     0,   178,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   179,    95,    96,     0,     0,    97,     0,     0,
      98,     0,     0,     0,     0,     0,    99,     0,     0,     0,
       0,   102,   103,   104,     0,     0,   105,   106,     0,   107,
     108,  1831,   109,   110,     0,   111,   112,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,  1834,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,     0,    79,    80,    81,
      82,    83,     0,     0,     0,    84,     0,     0,    85,     0,
       0,     0,     0,   178,    87,    88,    89,    90,     0,    91,
      92,     0,    93,   179,    95,    96,     0,     0,    97,     0,
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
       0,   107,   108,  1851,   109,   110,     0,   111,   112,     5,
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
     106,     0,   107,   108,  1868,   109,   110,     0,   111,   112,
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
     105,   106,     0,   107,   108,  1925,   109,   110,     0,   111,
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
       0,   105,   106,     0,   107,   108,  1928,   109,   110,     0,
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
      84,     0,     0,    85,     0,     0,     0,     0,   178,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   179,    95,
      96,     0,     0,    97,     0,     0,    98,     0,     0,     0,
       0,     0,    99,     0,     0,     0,     0,   102,   103,   104,
       0,     0,   105,   106,     0,   107,   108,     0,   109,   110,
       0,   111,   112,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,   542,     0,     0,     0,     0,     0,     0,     0,
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
      13,     0,     0,   806,     0,     0,     0,     0,     0,     0,
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
      12,    13,     0,     0,  1032,     0,     0,     0,     0,     0,
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
      11,    12,    13,     0,     0,  1596,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,    63,    64,   174,   175,    67,     0,    68,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,     0,    79,    80,    81,    82,
      83,     0,     0,     0,    84,     0,     0,    85,     0,     0,
       0,     0,   178,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   179,    95,     0,     0,     0,    97,     0,     0,
      98,     0,     0,     0,     0,     0,    99,     0,     0,     0,
       0,   102,   103,   104,     0,     0,   105,   106,     0,   107,
     108,     0,   109,   110,     0,   111,   112,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,  1742,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,    61,    62,    63,    64,   174,   175,    67,     0,
      68,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,     0,    79,    80,    81,
      82,    83,     0,     0,     0,    84,     0,     0,    85,     0,
       0,     0,     0,   178,    87,    88,    89,    90,     0,    91,
      92,     0,    93,   179,    95,     0,     0,     0,    97,     0,
       0,    98,     0,     0,     0,     0,     0,    99,     0,     0,
       0,     0,   102,   103,   104,     0,     0,   105,   106,     0,
     107,   108,     0,   109,   110,     0,   111,   112,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,    61,    62,    63,    64,   174,   175,    67,
       0,    68,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,     0,    79,    80,
      81,    82,    83,     0,     0,     0,    84,     0,     0,    85,
       0,     0,     0,     0,   178,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   179,    95,     0,     0,     0,    97,
       0,     0,    98,     0,     0,     0,     0,     0,    99,     0,
       0,     0,     0,   102,   103,   104,     0,     0,   105,   106,
       0,   107,   108,     0,   109,   110,     0,   111,   112,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   403,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,   740,     0,     0,     0,     0,     0,
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
     106,     0,     0,     0,     0,   109,   110,     0,   111,   112,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
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
     105,   180,     0,   343,     0,     0,   109,   110,     0,   111,
     112,     5,     6,     7,     8,     9,     0,     0,     0,     0,
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
     174,   175,   176,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   177,    75,    76,    77,    78,
       0,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,    85,     0,     0,     0,     0,   178,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   179,    95,     0,
       0,     0,    97,     0,     0,    98,     0,     0,     0,     0,
       0,    99,     0,     0,     0,     0,   102,   103,   104,     0,
       0,   105,   180,     0,     0,   801,     0,   109,   110,     0,
     111,   112,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  1071,  1072,  1073,  1074,  1075,  1076,  1077,
    1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,
    1088,  1089,  1090,  1091,  1092,     0,     0,  1156,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1093,    15,
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
       0,  1157,     0,    97,     0,     0,    98,     0,     0,     0,
       0,     0,    99,     0,     0,     0,     0,   102,   103,   104,
       0,     0,   105,   180,     0,     0,     0,     0,   109,   110,
       0,   111,   112,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   403,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     104,     0,     0,   105,   106,   443,   444,   445,     0,   109,
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
       0,     0,     0,     0,    50,     0,     0,     0,     0,   191,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   174,   175,   176,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   177,    75,
      76,    77,    78,     0,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,    85,     0,     0,     0,     0,
     178,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     179,    95,     0,     0,     0,    97,     0,     0,    98,     0,
       0,     0,     0,  1169,    99,     0,     0,     0,     0,   102,
     103,   104,     0,     0,   105,   180,     0,     0,     0,     0,
     109,   110,     0,   111,   112,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,     0,   471,     0,
     226,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     472,     0,    15,    16,     0,     0,     0,     0,    17,     0,
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
     102,   103,   104,     0,     0,   105,   180,   443,   444,   445,
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
      98,     0,     0,     0,     0,  1180,    99,     0,     0,     0,
       0,   102,   103,   104,     0,     0,   105,   180,     0,   261,
     444,   445,   109,   110,     0,   111,   112,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,   446,
     447,     0,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,     0,   471,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,   472,     0,
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
     264,     0,     0,   109,   110,     0,   111,   112,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   403,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,   102,   103,   104,     0,     0,   105,   106,
     443,   444,   445,     0,   109,   110,     0,   111,   112,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     446,   447,     0,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,     0,   471,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,   472,
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
      97,     0,     0,    98,     0,     0,     0,     0,  1208,    99,
       0,     0,     0,     0,   102,   103,   104,     0,     0,   105,
     180,   540,     0,     0,     0,   109,   110,     0,   111,   112,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   695,
     471,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   472,     0,     0,     0,     0,    15,    16,     0,
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
       0,    10,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,     0,     0,     0,   740,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1093,     0,    15,    16,
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
       0,     0,    10,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,     0,   471,     0,     0,   781,     0,     0,
       0,     0,     0,     0,     0,     0,   472,     0,     0,    15,
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
       0,     0,     0,    10,  1073,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,     0,     0,     0,     0,   783,     0,
       0,     0,     0,     0,     0,     0,     0,  1093,     0,     0,
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
     104,     0,     0,   105,   180,     0,     0,     0,     0,   109,
     110,     0,   111,   112,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,     0,     0,     0,     0,     0,  1124,
       0,     0,     0,     0,     0,     0,     0,  1093,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
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
     103,   104,     0,     0,   105,   180,     0,     0,     0,     0,
     109,   110,     0,   111,   112,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,     0,     0,     0,     0,     0,     0,
    1205,     0,     0,     0,     0,     0,     0,  1093,     0,     0,
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
     102,   103,   104,     0,     0,   105,   180,     0,     0,     0,
       0,   109,   110,     0,   111,   112,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,     0,   471,     0,     0,     0,     0,
       0,  1449,     0,     0,     0,     0,     0,   472,     0,     0,
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
       0,   102,   103,   104,     0,     0,   105,   180,   443,   444,
     445,     0,   109,   110,     0,   111,   112,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   446,   447,
       0,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,     0,   471,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,   472,     0,     0,
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
       0,    98,     0,     0,     0,     0,  1566,    99,     0,     0,
       0,     0,   102,   103,   104,     0,     0,   105,   180,   443,
     444,   445,     0,   109,   110,     0,   111,   112,     5,     6,
       7,     8,     9,     0,     0,     0,     0,   810,    10,   446,
     447,     0,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,     0,   471,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,   472,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,   628,    39,    40,
       0,   811,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   174,   175,   176,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   177,    75,    76,    77,    78,     0,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,    85,
       0,     0,     0,     0,   178,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   179,    95,     0,   266,   267,    97,
     268,   269,    98,     0,   270,   271,   272,   273,    99,     0,
       0,     0,     0,   102,   103,   104,     0,     0,   105,   180,
       0,   274,     0,   275,   109,   110,     0,   111,   112, -1040,
   -1040, -1040, -1040,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,     0,   471,     0,     0,
       0,   277,     0,     0,     0,     0,     0,     0,     0,   472,
       0,     0,     0,     0,     0,   279,   280,   281,   282,   283,
     284,   285,     0,     0,     0,   209,     0,   210,    40,     0,
       0,     0,     0,     0,     0,     0,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,    50,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,     0,   320,     0,   728,   322,   323,   324,     0,     0,
       0,   325,   570,   213,   214,   215,   571,     0,     0,     0,
       0,     0,   266,   267,     0,   268,   269,     0,     0,   270,
     271,   272,   273,   572,     0,     0,     0,     0,     0,    91,
      92,     0,    93,   179,    95,   330,   274,   331,   275,     0,
     332,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   105,     0,     0,
       0,   729,     0,   109,     0,     0,   277,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     279,   280,   281,   282,   283,   284,   285,     0,     0,     0,
     209,     0,   210,    40,     0,     0,     0,     0,     0,     0,
       0,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,    50,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,     0,   320,     0,   321,
     322,   323,   324,     0,     0,     0,   325,   570,   213,   214,
     215,   571,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   266,   267,     0,   268,   269,     0,   572,   270,
     271,   272,   273,     0,    91,    92,     0,    93,   179,    95,
     330,     0,   331,     0,     0,   332,   274,     0,   275,     0,
     276,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   105,     0,     0,     0,   729,     0,   109,     0,
       0,     0,     0,     0,     0,     0,   277,     0,   278,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     279,   280,   281,   282,   283,   284,   285,     0,     0,     0,
     209,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,    50,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,     0,   320,     0,     0,
     322,   323,   324,     0,     0,     0,   325,   326,   213,   214,
     215,   327,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   328,     0,
       0,    89,   329,     0,    91,    92,     0,    93,   179,    95,
     330,     0,   331,     0,     0,   332,   266,   267,     0,   268,
     269,     0,   333,   270,   271,   272,   273,     0,     0,     0,
       0,     0,   105,   334,     0,     0,     0,  1791,     0,     0,
     274,     0,   275,   447,   276,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,     0,   471,
     277,     0,   278,     0,     0,     0,     0,     0,     0,     0,
       0,   472,     0,     0,   279,   280,   281,   282,   283,   284,
     285,     0,     0,     0,   209,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,    50,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
       0,   320,     0,   321,   322,   323,   324,     0,     0,     0,
     325,   326,   213,   214,   215,   327,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   328,     0,     0,    89,   329,     0,    91,    92,
       0,    93,   179,    95,   330,     0,   331,     0,     0,   332,
     266,   267,     0,   268,   269,     0,   333,   270,   271,   272,
     273,     0,     0,     0,     0,     0,   105,   334,     0,     0,
       0,     0,     0,     0,   274,     0,   275,     0,   276,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,     0,   471,   277,     0,   278,     0,     0,     0,
       0,     0,     0,     0,     0,   472,     0,     0,   279,   280,
     281,   282,   283,   284,   285,     0,     0,     0,   209,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
      50,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,     0,   320,     0,     0,   322,   323,
     324,     0,     0,     0,   325,   326,   213,   214,   215,   327,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   328,     0,     0,    89,
     329,     0,    91,    92,     0,    93,   179,    95,   330,     0,
     331,     0,     0,   332,     0,   266,   267,     0,   268,   269,
     333,  1524,   270,   271,   272,   273,     0,     0,     0,     0,
     105,   334,     0,     0,     0,     0,     0,     0,     0,   274,
       0,   275,     0,   276, -1040, -1040, -1040, -1040,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,     0,     0,     0,     0,     0,     0,     0,     0,   277,
       0,   278,     0,     0,  1093,     0,     0,     0,     0,     0,
       0,     0,     0,   279,   280,   281,   282,   283,   284,   285,
       0,     0,     0,   209,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,    50,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,     0,
     320,     0,     0,   322,   323,   324,     0,     0,     0,   325,
     326,   213,   214,   215,   327,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   328,     0,     0,    89,   329,     0,    91,    92,     0,
      93,   179,    95,   330,     0,   331,     0,     0,   332,  1621,
    1622,  1623,  1624,  1625,     0,   333,  1626,  1627,  1628,  1629,
       0,     0,     0,     0,     0,   105,   334,     0,     0,     0,
       0,     0,     0,  1630,  1631,  1632,     0,   446,   447,     0,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,  1633,   471,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   472,  1634,  1635,  1636,
    1637,  1638,  1639,  1640,     0,     0,     0,   209,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1641,  1642,
    1643,  1644,  1645,  1646,  1647,  1648,  1649,  1650,  1651,    50,
    1652,  1653,  1654,  1655,  1656,  1657,  1658,  1659,  1660,  1661,
    1662,  1663,  1664,  1665,  1666,  1667,  1668,  1669,  1670,  1671,
    1672,  1673,  1674,  1675,  1676,  1677,  1678,  1679,  1680,  1681,
       0,     0,     0,  1682,  1683,   213,   214,   215,     0,  1684,
    1685,  1686,  1687,  1688,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1689,  1690,  1691,     0,     0,
       0,    91,    92,     0,    93,   179,    95,  1692,     0,  1693,
    1694,     0,  1695,     0,     0,     0,     0,     0,     0,  1696,
    1697,     0,  1698,     0,  1699,  1700,     0,   266,   267,   105,
     268,   269,  1068,  1069,   270,   271,   272,   273,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   274,  1070,   275,     0,  1071,  1072,  1073,  1074,  1075,
    1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,     0,     0,
       0,   277,     0,     0,     0,     0,     0,     0,     0,     0,
    1093,     0,     0,     0,     0,   279,   280,   281,   282,   283,
     284,   285,     0,     0,     0,   209,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,    50,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,     0,   320,     0,   321,   322,   323,   324,     0,     0,
       0,   325,   570,   213,   214,   215,   571,     0,     0,     0,
       0,     0,   266,   267,     0,   268,   269,     0,     0,   270,
     271,   272,   273,   572,     0,     0,     0,     0,     0,    91,
      92,     0,    93,   179,    95,   330,   274,   331,   275,     0,
     332,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   105,     0,     0,
       0,     0,     0,     0,     0,     0,   277,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     279,   280,   281,   282,   283,   284,   285,     0,     0,     0,
     209,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,    50,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,     0,   320,     0,  1256,
     322,   323,   324,     0,     0,     0,   325,   570,   213,   214,
     215,   571,     0,     0,     0,     0,     0,   266,   267,     0,
     268,   269,     0,     0,   270,   271,   272,   273,   572,     0,
       0,     0,     0,     0,    91,    92,     0,    93,   179,    95,
     330,   274,   331,   275,     0,   332,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   105,     0,     0,     0,     0,     0,     0,     0,
       0,   277,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   279,   280,   281,   282,   283,
     284,   285,     0,     0,     0,   209,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,    50,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,     0,   320,     0,     0,   322,   323,   324,     0,     0,
       0,   325,   570,   213,   214,   215,   571,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   572,     0,     0,     0,     0,     0,    91,
      92,     0,    93,   179,    95,   330,     0,   331,     0,     0,
     332,   443,   444,   445,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   105,     0,     0,
       0,   446,   447,     0,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,     0,   471,   443,
     444,   445,     0,     0,     0,     0,     0,     0,     0,     0,
     472,     0,     0,     0,     0,     0,     0,     0,     0,   446,
     447,     0,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,     0,   471,   443,   444,   445,
       0,     0,     0,     0,     0,     0,     0,     0,   472,     0,
       0,     0,     0,     0,     0,     0,     0,   446,   447,     0,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,     0,   471,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   472,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   443,   444,   445,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   446,   447,  1567,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,     0,   471,     0,   443,   444,   445,     0,
       0,     0,     0,     0,     0,     0,   472,     0,     0,     0,
       0,     0,     0,     0,     0,   473,   446,   447,     0,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,     0,   471,   443,   444,   445,     0,     0,     0,
       0,     0,     0,     0,     0,   472,     0,     0,     0,     0,
       0,     0,     0,   556,   446,   447,     0,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
       0,   471,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   472,     0,     0,     0,     0,     0,     0,
       0,   443,   444,   445,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   446,   447,   558,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,     0,   471,     0,
     443,   444,   445,     0,     0,     0,     0,     0,     0,     0,
     472,     0,     0,     0,     0,     0,     0,     0,   276,     0,
     446,   447,   577,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   278,   471,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   472,
       0,     0,     0,     0,     0,     0,     0,     0,   209,     0,
     581,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,  1264,     0,     0,  -394,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   174,
     175,   431,     0,   827,   828,     0,     0,     0,     0,   829,
       0,   830,     0,     0,     0,   559,   213,   214,   215,   560,
       0,     0,     0,   831,     0,     0,   773,     0,     0,     0,
       0,    34,    35,    36,   209,     0,   178,     0,     0,    89,
     329,     0,    91,    92,   211,    93,   179,    95,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
     333,     0,     0,     0,     0,     0,   432,     0,     0,     0,
     105,   334,     0,     0,     0,   798,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   832,   833,   834,   835,     0,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,   216,  1026,     0,
       0,     0,   178,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   179,    95,     0,     0,     0,    97,     0,     0,
       0,     0,     0,     0,     0,     0,   836,     0,     0,     0,
      29,   102,     0,     0,     0,     0,   105,   837,    34,    35,
      36,   209,     0,   210,    40,     0,     0,     0,     0,     0,
       0,   211,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   212,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1027,    75,   213,
     214,   215,     0,    79,    80,    81,    82,    83,     0,     0,
       0,     0,     0,     0,   216,     0,     0,     0,     0,   178,
      87,    88,    89,    90,     0,    91,    92,     0,    93,   179,
      95,     0,   827,   828,    97,     0,     0,     0,   829,     0,
     830,     0,     0,     0,     0,     0,     0,     0,   102,     0,
       0,     0,   831,   105,   217,     0,     0,     0,     0,   109,
      34,    35,    36,   209,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   211,     0,     0,     0,     0,   984,   985,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   986,     0,
       0,     0,     0,     0,     0,     0,   987,   988,   989,   209,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   990,
     832,   833,   834,   835,     0,    79,    80,    81,    82,    83,
       0,    50,     0,     0,     0,     0,   216,     0,     0,     0,
       0,   178,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   179,    95,     0,     0,     0,    97,     0,     0,     0,
       0,     0,     0,     0,     0,   836,   991,   992,   993,   994,
     102,     0,     0,     0,     0,   105,   837,     0,     0,     0,
       0,     0,   995,     0,     0,     0,     0,   178,     0,     0,
      89,    90,    29,    91,    92,     0,    93,   179,    95,     0,
      34,    35,    36,   209,     0,   210,    40,     0,     0,     0,
       0,   996,     0,   211,     0,     0,     0,     0,     0,     0,
       0,   105,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,   213,   214,   215,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,   216,     0,     0,     0,
       0,   178,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   179,    95,    29,     0,     0,    97,     0,     0,     0,
       0,    34,    35,    36,   209,     0,   210,    40,     0,     0,
     102,     0,     0,     0,   211,   105,   217,     0,     0,   593,
       0,   109,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   212,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     613,    75,   213,   214,   215,     0,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,   216,     0,     0,
       0,     0,   178,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   179,    95,    29,     0,   973,    97,     0,     0,
       0,     0,    34,    35,    36,   209,     0,   210,    40,     0,
       0,   102,     0,     0,     0,   211,   105,   217,     0,     0,
       0,     0,   109,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,   213,   214,   215,     0,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,   216,     0,
       0,     0,     0,   178,    87,    88,    89,    90,     0,    91,
      92,     0,    93,   179,    95,    29,     0,     0,    97,     0,
       0,     0,     0,    34,    35,    36,   209,     0,   210,    40,
       0,     0,   102,     0,     0,     0,   211,   105,   217,     0,
       0,     0,     0,   109,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   212,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1121,    75,   213,   214,   215,     0,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,   216,
       0,     0,     0,     0,   178,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   179,    95,    29,     0,     0,    97,
       0,     0,     0,     0,    34,    35,    36,   209,     0,   210,
      40,     0,     0,   102,     0,     0,     0,   211,   105,   217,
       0,     0,     0,     0,   109,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     212,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,   213,   214,   215,     0,    79,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
     216,     0,     0,     0,     0,   178,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   179,    95,     0,     0,     0,
      97,     0,     0,   443,   444,   445,     0,     0,     0,     0,
       0,     0,     0,     0,   102,     0,     0,     0,     0,   105,
     217,     0,     0,   446,   447,   109,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,     0,
     471,   443,   444,   445,     0,     0,     0,     0,     0,     0,
       0,     0,   472,     0,     0,     0,     0,     0,     0,     0,
       0,   446,   447,     0,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,     0,   471,     0,
       0,     0,     0,     0,     0,     0,     0,   443,   444,   445,
     472,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   446,   447,   517,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,     0,   471,   443,   444,   445,     0,     0,
       0,     0,     0,     0,     0,     0,   472,     0,     0,     0,
       0,     0,     0,     0,     0,   446,   447,   526,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,     0,   471,     0,     0,     0,     0,     0,     0,     0,
       0,   443,   444,   445,   472,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   446,   447,   893,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,     0,   471,   443,
     444,   445,     0,     0,     0,     0,     0,     0,     0,     0,
     472,     0,     0,     0,     0,     0,     0,     0,     0,   446,
     447,   959,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,     0,   471,     0,     0,     0,
       0,     0,     0,     0,     0,   443,   444,   445,   472,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   446,   447,  1011,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,     0,   471,  1067,  1068,  1069,     0,     0,     0,     0,
       0,     0,     0,     0,   472,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1070,  1310,     0,  1071,  1072,  1073,
    1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1067,  1068,
    1069,     0,  1093,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1070,
       0,  1341,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,     0,     0,  1067,  1068,  1069,     0,
       0,     0,     0,     0,     0,     0,     0,  1093,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1070,     0,  1240,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1067,  1068,  1069,     0,  1093,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1070,     0,  1406,  1071,  1072,  1073,  1074,  1075,
    1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,     0,  1067,
    1068,  1069,     0,     0,     0,     0,     0,     0,     0,     0,
    1093,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1070,     0,  1415,  1071,  1072,  1073,  1074,  1075,  1076,  1077,
    1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,
    1088,  1089,  1090,  1091,  1092,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1067,  1068,  1069,     0,  1093,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1070,     0,  1510,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
       0,    34,    35,    36,   209,     0,   210,    40,     0,     0,
       0,     0,     0,  1093,   211,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1602,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   231,     0,     0,
       0,     0,     0,   232,     0,     0,     0,     0,     0,     0,
       0,     0,   213,   214,   215,     0,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,   216,     0,     0,
    1604,     0,   178,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   179,    95,     0,     0,     0,    97,     0,    34,
      35,    36,   209,     0,   210,    40,     0,     0,     0,     0,
       0,   102,   642,     0,     0,     0,   105,   233,     0,     0,
       0,     0,   109,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     213,   214,   215,     0,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,   216,     0,     0,     0,     0,
     178,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     179,    95,     0,     0,     0,    97,     0,    34,    35,    36,
     209,     0,   210,    40,     0,     0,     0,     0,     0,   102,
     211,     0,     0,     0,   105,   643,     0,     0,     0,     0,
     109,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   231,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   213,   214,
     215,     0,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,   216,     0,     0,     0,     0,   178,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   179,    95,
       0,     0,     0,    97,     0,     0,     0,     0,     0,   443,
     444,   445,     0,     0,     0,     0,     0,   102,     0,     0,
       0,     0,   105,   233,     0,     0,     0,     0,   109,   446,
     447,   956,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,     0,   471,   443,   444,   445,
       0,     0,     0,     0,     0,     0,     0,     0,   472,     0,
       0,     0,     0,     0,     0,     0,     0,   446,   447,     0,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,     0,   471,  1067,  1068,  1069,     0,     0,
       0,     0,     0,     0,     0,     0,   472,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1070,  1420,     0,  1071,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,  1067,  1068,  1069,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1093,     0,     0,     0,     0,     0,
       0,     0,  1070,     0,     0,  1071,  1072,  1073,  1074,  1075,
    1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1092,   445,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1093,     0,     0,     0,     0,   446,   447,     0,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,     0,   471,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   472
};

static const yytype_int16 yycheck[] =
{
       5,     6,   127,     8,     9,    10,    11,    12,    13,     4,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    56,   106,    29,    30,   182,   658,     4,   157,
       4,     4,     4,    33,    31,    31,   511,  1146,   932,    44,
      31,   396,   396,    57,   396,    56,    46,    52,   234,    54,
     162,    51,    57,   684,    59,   655,   654,   530,   183,   921,
    1301,    31,   507,   508,  1133,   157,   471,    44,   106,   809,
     635,   587,   588,   503,   737,   503,   952,    57,  1025,    84,
     245,   350,   351,   106,     9,   816,   106,   785,    49,    32,
      49,   536,   968,    32,     9,     9,     9,     9,     9,    14,
      14,   106,    14,     9,    49,     9,     9,    84,   538,     9,
     538,     9,     9,     9,    49,     9,     9,   775,    50,    51,
       9,    49,     9,     9,    54,     9,     9,     9,    70,     9,
       9,     9,     9,    83,   246,    90,  1012,    83,    90,  1735,
      81,   158,   180,     9,   115,    83,    83,   475,    96,   106,
     107,   102,   100,   101,    36,   134,   135,   180,   164,   158,
     180,    70,    70,   194,    38,   134,   135,   134,   135,     4,
     179,   177,     0,     4,     4,   180,   504,   194,   126,   217,
     535,   509,   187,   102,   157,   194,    38,   109,     8,   130,
     194,   197,   134,   135,   217,   233,  1792,   217,   197,    38,
     155,  1063,    38,   155,    38,   176,     4,    70,    70,    83,
     233,   162,   217,    70,    70,    70,   381,    70,    53,   198,
     158,    56,   172,    70,    70,   194,   172,    70,   233,    70,
     191,    83,   863,   163,   195,   172,   195,   197,    73,    83,
     194,   198,   247,   162,    83,   250,    83,    83,   180,    83,
     195,    49,   257,   258,   197,   197,    70,   196,    70,    94,
     195,    96,    70,   191,   179,   100,   101,    70,    60,  1338,
      70,   196,   197,   971,  1221,   251,  1345,   433,  1347,   255,
    1521,   196,   196,   196,   196,   196,   194,  1431,   197,   197,
     196,   126,   196,   196,    86,   195,   127,    89,   196,   196,
     196,  1177,   196,   196,  1035,  1374,  1037,   196,   342,   196,
     196,   195,   195,   195,   112,   195,   195,   195,   195,   117,
     172,   119,   120,   121,   122,   123,   124,   125,   172,   195,
    1142,   342,   370,   172,   197,   197,   172,  1199,   172,   194,
     197,   197,   197,   859,   860,   910,   158,   370,   194,   949,
     370,   197,   183,   194,   197,   158,   197,   513,   158,    83,
     485,   196,   199,   102,   369,   370,    90,   165,   166,   102,
     168,   376,   377,   378,   379,    27,    28,    38,   158,   384,
     775,   194,   194,   197,    70,   197,  1530,   158,   164,   197,
     424,   189,   369,    70,   197,    70,    70,   197,   403,   179,
     198,   378,   379,   486,    70,    54,   411,   158,   179,  1478,
    1554,   162,  1556,   424,   889,   194,   251,   197,   423,   229,
     255,   197,    83,   162,   259,    50,    51,    14,   179,   162,
      83,    84,   156,   157,   106,   107,   194,   410,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   477,   472,  1131,   474,
     475,   476,   486,   194,   471,   471,   194,  1217,   202,   427,
     471,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,    83,    84,   661,    83,   663,   504,
     505,   471,   507,   508,   509,   510,   486,   342,   938,   514,
     938,   197,   517,   524,   163,   102,   103,   194,   195,   194,
     965,   526,    83,   528,  1336,     4,   198,    83,   194,   178,
     544,   536,   480,   481,   482,   483,  1194,  1195,  1196,   544,
      57,   546,    70,  1201,   672,   731,   199,  1145,    27,    28,
    1412,   194,    69,    75,    76,   477,    75,    76,   350,   351,
     352,  1161,   194,   179,  1164,   535,   921,   921,   973,   921,
     156,   157,   402,   549,   226,   410,    70,   763,   194,   410,
     672,   503,   194,   739,   853,   420,   855,   194,   593,   424,
    1106,   986,   427,    53,    54,    55,   388,    83,    70,   155,
     156,   157,   524,    70,    90,   643,   134,   135,   197,    69,
     199,   179,    83,   535,   779,   780,   538,    14,    83,    90,
     163,   786,   787,   122,   179,    90,   194,   106,   158,   190,
     158,   130,  1494,   102,   103,    32,   197,   194,   643,   194,
     134,   135,   477,   478,   479,   480,   481,   482,   483,   179,
      31,   162,   838,   484,    51,  1467,    83,  1469,    83,   162,
      32,  1902,   848,    90,   194,    90,   194,   197,   503,    50,
     156,   157,    53,   179,   786,    83,   194,   682,    31,    38,
     123,   196,    90,  1346,   155,   156,   157,   130,   194,   524,
     695,   156,   157,   196,   197,   196,  1354,    50,  1356,   672,
      53,   180,    70,   538,   132,   133,  1179,   359,  1063,  1063,
     520,  1063,   196,   197,   549,   196,   368,   203,   370,   105,
     106,   107,   196,   375,   729,  1170,   196,  1325,   155,   156,
     157,   156,   157,   385,   569,   196,  1181,   196,   217,   196,
    1340,    70,   690,   196,   197,  1220,   382,   226,   156,   157,
     386,   350,   351,   758,   233,    70,   591,   592,    53,    54,
      55,    70,    57,  1575,   105,   106,   107,  1579,   119,   120,
     121,  1880,   251,    70,    69,   197,   255,   413,   158,   415,
     416,   417,   418,  1446,   194,  1894,  1766,  1767,   793,   624,
     625,   110,   111,   112,   194,   587,   588,    70,  1193,  1194,
    1195,  1196,  1762,  1763,   809,   805,  1201,   194,   111,   619,
     620,   158,    78,    79,    80,    81,   119,   120,   121,   122,
     123,   124,   825,   826,   162,   655,   799,  1485,    69,  1487,
    1305,  1489,   196,    48,  1492,   645,   179,   103,   814,   194,
     158,   201,   194,     9,  1199,  1199,   158,  1199,   542,   194,
     158,     8,   196,   158,   194,   690,    14,   158,   197,   511,
     196,    81,   119,   120,   121,   122,   123,   124,  1531,   196,
       9,   196,   130,   139,   140,   141,   130,   850,    14,  1479,
     359,   195,    14,   103,   578,   179,   189,   200,   893,   368,
     895,   370,   897,   102,   111,   195,   375,  1342,   194,   165,
     166,   906,   168,   169,   170,   195,   385,   195,   195,   194,
     194,     9,   155,   924,   909,   920,   195,   727,   195,   139,
     140,   141,  1734,   195,   195,    94,  1738,   193,  1586,     9,
      14,   410,   189,   909,   196,   909,   909,   909,   194,   196,
     775,   946,   777,   163,   179,   165,   166,   167,   168,   169,
     170,   956,  1846,     9,   959,   194,   961,   930,   196,  1354,
     965,  1356,   656,   197,   799,   197,  1152,   196,   799,    83,
     195,   195,  1866,   193,   194,   196,   973,   973,   813,   814,
     132,  1875,   973,   195,   194,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,   195,   201,
       9,     9,   924,   973,    70,   201,  1011,   201,   818,    32,
     133,   178,   158,   823,   936,   850,   938,   136,  1018,   850,
    1465,     9,   857,   858,   972,   195,   191,   158,    14,   681,
       9,     9,   511,   180,   195,     9,    31,    14,    67,    68,
    1123,   132,   201,  1019,   198,   201,     9,    14,  1021,   158,
    1023,   886,   201,   195,  1065,   195,   201,  1412,  1412,   194,
    1412,   853,  1874,   855,   102,   195,     9,   859,   860,   861,
     549,   196,   196,    91,   909,   885,   136,  1889,   909,   909,
     158,     9,  1740,  1741,   195,    70,    81,   194,   740,   924,
    1485,  1013,  1487,   194,  1489,   930,     9,  1492,    70,   930,
      70,   936,   932,   938,   197,   134,   135,   801,   103,  1123,
     198,    14,   806,   196,     9,   180,   197,    14,  1123,   949,
     197,    14,   201,   119,   120,   121,   122,   123,   124,   781,
     125,   783,   195,   191,   196,  1130,    32,   972,    32,  1494,
    1494,   194,  1494,   138,   139,   140,   141,   142,   194,   984,
     985,   986,  1157,    14,  1130,  1786,  1130,  1130,  1130,   811,
     194,   194,    14,    52,   159,  1170,   195,   162,   163,   194,
     165,   166,    70,   168,   169,   170,  1181,  1182,  1013,    70,
      70,   194,   158,     9,  1019,   195,  1021,   136,  1023,   196,
    1021,  1586,  1023,   189,   196,   194,  1006,    14,   193,   180,
     136,     9,   681,   195,    69,     9,    83,   201,  1043,  1209,
       9,   198,  1217,   119,   120,   121,   122,   123,   124,   136,
      14,   873,  1227,   198,   130,   131,   194,   196,   194,    83,
    1065,   196,   196,  1864,   195,   197,  1212,   889,   890,   194,
     194,  1051,   195,   201,   136,   197,   197,     9,  1058,    91,
     155,  1224,   197,    32,    77,   196,   195,   180,   196,  1094,
     136,   740,    32,   195,   195,   171,   201,     9,   201,  1900,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,   189,     4,     9,   201,    50,    51,    52,
      53,    54,    55,   136,    57,  1130,     9,   198,     9,  1130,
    1130,    83,   781,     9,   783,  1310,    69,   195,   195,    14,
     196,   196,  1317,   197,  1106,   198,  1321,  1442,  1323,  1447,
     799,   194,   196,    67,    68,   195,  1331,   195,   195,    49,
    1303,  1161,   811,   197,  1164,   814,  1341,  1342,  1032,   194,
     136,  1314,   195,   201,   195,  1740,  1741,     9,   136,     9,
     195,    78,    79,    80,   201,   201,    32,   196,  1193,  1194,
    1195,  1196,   195,   195,    91,   196,  1201,   196,   163,   197,
     167,   850,   112,  1025,  1026,   196,    14,  1212,   117,    83,
     195,  1191,   195,   197,   136,   195,   136,    14,   179,  1224,
     134,   135,   112,  1224,   873,   197,  1552,   117,   196,   119,
     120,   121,   122,   123,   124,   125,    83,    14,    14,    83,
     889,   890,   195,   194,   136,   195,   143,   144,   145,   146,
     147,   136,   196,   196,    27,    28,    14,   154,    14,   196,
     909,    14,   197,   160,   161,     9,  1246,    27,    28,     9,
    1250,    31,   198,  1253,    59,   165,   166,   174,   168,  1422,
    1260,   930,    83,   179,    83,     9,   194,  1462,   115,   196,
    1465,   188,  1297,   197,   158,   102,    56,   102,   180,   189,
     170,    36,  1124,  1303,  1447,    14,   194,   196,   198,   195,
     194,   176,   180,   180,  1314,    83,   173,   119,   120,   121,
     122,   123,   124,   195,     9,  1468,    83,   196,   130,   131,
      14,  1474,   195,  1476,  1156,   195,   197,   193,    83,     9,
    1340,    14,    83,    14,    83,    14,  1527,    83,  1106,  1354,
    1855,  1356,   478,  1496,   483,  1498,   966,  1503,   912,  1871,
     480,  1218,  1595,  1866,  1507,  1389,   595,   169,  1582,   171,
    1019,  1619,  1021,  1532,  1023,  1704,  1025,  1026,  1911,  1437,
    1887,  1361,   184,  1205,   186,  1365,  1716,   189,  1433,  1578,
    1370,   379,  1062,  1196,  1720,  1570,  1059,  1134,  1220,  1221,
     985,    81,  1192,  1008,  1193,   936,  1524,   825,  1898,    50,
      51,    52,    53,    54,    55,  1821,   376,  1422,   424,  1913,
    1114,  1422,  1425,   103,  1044,    -1,  1431,  1291,    69,  1094,
      -1,    -1,  1437,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1441,  1584,    -1,    -1,    -1,    -1,    -1,    -1,  1594,  1595,
    1593,    -1,    -1,   226,    -1,    -1,  1599,    -1,    -1,   139,
     140,   141,  1605,    -1,    -1,    -1,   226,    -1,  1468,    -1,
      -1,    -1,    -1,    -1,  1474,  1124,  1476,    -1,    -1,  1479,
    1485,  1130,  1487,  1305,  1489,   165,   166,  1492,   168,   169,
     170,    -1,    -1,  1498,    -1,    -1,  1496,  1498,  1503,    -1,
      -1,    -1,  1507,    -1,    -1,    -1,  1507,  1156,    -1,    -1,
      -1,  1715,  1376,   193,   194,    -1,   276,    -1,   278,  1524,
      -1,    -1,  1527,    -1,    -1,  1530,    -1,    -1,  1392,    -1,
      -1,    -1,    -1,    -1,  1715,  1540,    -1,    -1,    -1,    -1,
      -1,  1839,  1547,    -1,    -1,    -1,    -1,    -1,    -1,  1554,
    1725,  1556,    -1,    -1,    -1,    -1,  1205,    -1,  1563,  1560,
      -1,    -1,    -1,  1212,  1859,    -1,    -1,    -1,    -1,    -1,
      -1,  1220,  1221,    -1,   334,  1224,    -1,    -1,    -1,    -1,
      -1,  1586,    -1,    -1,  1584,    -1,   359,    -1,  1593,  1594,
    1595,    -1,  1593,    -1,  1599,   368,    -1,    -1,  1599,   359,
    1605,    -1,   375,  1746,  1605,    -1,    -1,    -1,   368,    -1,
      -1,    -1,   385,    -1,    -1,   375,    -1,    -1,    -1,    -1,
      -1,    56,    -1,   396,    -1,   385,    -1,  1449,    56,    -1,
      -1,    -1,    -1,  1497,    -1,    -1,   396,    -1,    -1,    -1,
      -1,    -1,  1506,    -1,    -1,  1788,    -1,    -1,    -1,    -1,
      -1,    -1,  1795,    -1,    -1,  1519,  1305,    -1,    -1,    -1,
      -1,   421,  1780,    -1,   424,    -1,    -1,    -1,    -1,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,  1830,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1839,    -1,    -1,     4,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1852,
    1715,   471,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    68,    -1,    -1,    -1,    -1,
      -1,  1736,  1596,    -1,  1909,  1740,  1741,    -1,   511,    -1,
      -1,  1746,    -1,    -1,    49,  1746,  1921,    -1,    -1,    -1,
    1755,   511,    -1,    -1,    -1,    -1,  1931,  1762,  1763,  1934,
      -1,  1766,  1767,    -1,    -1,    -1,    -1,    -1,    -1,  1912,
      -1,    -1,    -1,  1422,  1917,  1780,  1756,  1757,    -1,    -1,
      -1,    -1,    -1,  1788,    -1,    -1,    -1,  1788,    -1,    -1,
    1795,   134,   135,    -1,  1795,    -1,    -1,    -1,    -1,    -1,
    1449,    -1,    -1,   563,    -1,   565,    -1,   112,   568,    -1,
      -1,    -1,   117,    -1,   119,   120,   121,   122,   123,   124,
     125,    -1,    -1,    -1,    -1,  1830,    -1,    -1,    -1,  1830,
      -1,    -1,    -1,  1838,    -1,    -1,    -1,    -1,    -1,   599,
      -1,   276,    -1,   278,    -1,     4,  1846,  1852,   276,  1498,
     278,  1852,   195,  1858,  1503,    -1,    -1,    -1,  1507,    -1,
     165,   166,    -1,   168,    -1,    -1,  1866,    -1,  1732,    -1,
      -1,    -1,    -1,    -1,    -1,  1875,    -1,    -1,  1742,    -1,
      -1,    -1,    -1,    -1,   189,    -1,    -1,    -1,    -1,    -1,
      49,   651,   652,   198,    -1,    -1,    -1,    -1,    -1,   334,
     660,    -1,    -1,    -1,    -1,    -1,   334,  1912,   681,    -1,
      -1,  1912,  1917,    -1,    -1,   226,  1917,    -1,    -1,    -1,
      -1,   681,    -1,  1787,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1593,  1594,  1595,    -1,    -1,    -1,
    1599,    -1,    -1,   112,    -1,    -1,  1605,    -1,   117,    -1,
     119,   120,   121,   122,   123,   124,   125,   740,  1832,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    68,    -1,
     740,    -1,    -1,    -1,    -1,    -1,   421,    -1,    -1,   424,
      -1,    -1,    -1,   421,    -1,    -1,   424,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   165,   166,   781,   168,
     783,    -1,    -1,    -1,    -1,   775,    -1,    -1,    -1,    -1,
      -1,   781,    -1,   783,    -1,    -1,    -1,    -1,    -1,    -1,
     189,    -1,    -1,    -1,    31,  1899,    -1,    -1,   811,   198,
    1904,    -1,    -1,    -1,   134,   135,    -1,    -1,   359,    -1,
      -1,   811,   812,    -1,    -1,    -1,    -1,   368,    -1,    -1,
      -1,    -1,    59,    -1,   375,   825,   826,   827,   828,   829,
     830,   831,    -1,    -1,   385,    -1,    -1,   837,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,   851,    -1,    -1,    -1,    -1,    -1,  1746,    -1,    -1,
     873,    -1,    -1,    -1,    -1,   195,   103,    -1,    -1,    -1,
      31,    -1,    -1,   873,   111,    -1,   889,   890,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   887,   563,   889,
     890,    -1,    -1,    -1,    -1,   563,    -1,   565,    -1,  1788,
      -1,   138,   139,   140,   141,   142,  1795,    -1,   921,    -1,
      -1,   911,   912,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      81,   921,   159,    -1,    -1,   162,   163,   927,   165,   166,
      91,   168,   169,   170,    -1,    -1,    -1,    -1,    -1,    -1,
     940,  1830,   103,    -1,    -1,    -1,   183,    -1,   948,    -1,
      -1,   951,    -1,    -1,    -1,    -1,   193,   194,    -1,    -1,
     511,    -1,    -1,  1852,    -1,    -1,    -1,    -1,    -1,   969,
      -1,    -1,    -1,   973,    -1,    -1,   651,   652,   139,   140,
     141,    -1,    -1,   651,   652,   660,   986,    -1,    -1,    -1,
      -1,    -1,   660,    -1,    -1,    -1,    -1,    -1,   159,    -1,
      -1,   162,    -1,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,  1025,  1026,    -1,    -1,    -1,    -1,    -1,     4,
      -1,    -1,    -1,  1912,    -1,  1025,  1026,    -1,  1917,    -1,
      -1,    -1,   193,    -1,    -1,    -1,    -1,    -1,    -1,  1039,
      -1,    -1,  1042,    -1,  1044,    -1,    -1,    -1,    -1,    -1,
    1063,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1059,
    1060,  1061,  1062,  1063,    49,    -1,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,  1093,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
    1110,  1124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1124,    -1,    -1,   112,    -1,    -1,
     681,    -1,   117,    -1,   119,   120,   121,   122,   123,   124,
     125,    -1,    -1,  1156,  1144,    -1,  1146,    67,    68,    -1,
     825,   826,    -1,    -1,    -1,    -1,  1156,   825,   826,    -1,
      -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1173,    -1,    -1,  1176,    -1,    -1,    -1,
     165,   166,    -1,   168,    -1,    -1,  1199,    -1,    -1,   740,
      -1,    -1,  1205,  1193,  1194,  1195,  1196,    -1,    -1,  1199,
      -1,  1201,    -1,    -1,   189,  1205,    -1,  1220,  1221,    -1,
      49,    -1,    -1,   198,   134,   135,    -1,    -1,    -1,    -1,
    1220,  1221,    -1,  1223,    -1,    -1,    -1,    -1,    -1,    -1,
     781,    81,   783,  1233,    -1,    -1,   911,  1237,    -1,    -1,
    1240,    -1,  1242,   911,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   927,   103,    -1,    -1,    -1,    -1,    -1,   927,
     811,    -1,    -1,    -1,  1264,   940,    -1,    -1,    -1,    -1,
      -1,    -1,   940,   112,    -1,    -1,    -1,    -1,   117,    -1,
     119,   120,   121,   122,   123,   124,   125,    -1,    -1,   139,
     140,   141,  1305,    -1,   969,    -1,    -1,    -1,  1298,  1299,
      -1,   969,  1302,    -1,    -1,  1305,     4,    27,    28,   159,
      -1,    31,   162,    -1,    -1,   165,   166,    -1,   168,   169,
     170,    -1,   873,    -1,    -1,    -1,   165,   166,    -1,   168,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   889,   890,
      -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,   198,    -1,
     189,    49,    -1,    -1,  1354,    -1,  1356,    -1,    -1,   198,
      -1,    -1,    -1,    -1,  1039,    -1,    -1,  1042,    -1,    -1,
      -1,  1039,    -1,    -1,  1042,    -1,  1044,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,  1396,    57,  1398,  1412,
      -1,    -1,    -1,    -1,  1404,    31,  1406,    -1,  1408,    69,
      -1,  1411,  1412,    -1,   112,  1415,    -1,  1417,    -1,   117,
    1420,   119,   120,   121,   122,   123,   124,   125,    -1,    -1,
      -1,  1431,  1432,    59,    -1,  1435,  1449,    -1,    -1,    -1,
      -1,    -1,  1442,    -1,    -1,    -1,    -1,    -1,    -1,  1449,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   165,   166,  1144,
     168,  1146,    -1,    -1,  1025,  1026,  1144,   103,  1146,    -1,
      -1,  1494,    -1,    -1,    -1,  1485,    -1,  1487,    -1,  1489,
      -1,   189,  1492,    -1,  1494,    -1,    -1,    -1,  1173,    -1,
     198,  1176,    -1,    -1,    -1,  1173,   226,    -1,  1176,    -1,
    1510,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,  1522,  1523,    -1,    -1,    -1,    -1,    -1,    -1,
    1530,    -1,  1532,   159,    -1,    -1,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,    -1,   172,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1554,    -1,  1556,   183,  1233,    -1,
      -1,    -1,  1237,  1563,    -1,  1233,    -1,   193,   194,  1237,
      -1,    -1,    -1,  1124,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1586,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,  1602,  1603,  1604,  1156,    -1,    -1,    -1,  1609,
      -1,  1611,    -1,    69,    -1,    -1,    -1,  1617,    -1,  1619,
      -1,    -1,    -1,  1298,  1299,    -1,    -1,    -1,    -1,    -1,
    1298,  1299,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   359,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   368,    -1,
      81,    -1,    83,    84,  1205,   375,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,   385,    -1,    -1,    -1,  1220,
    1221,    -1,   103,    -1,    -1,    -1,   396,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,  1719,
      -1,  1396,    -1,  1398,    -1,    -1,    -1,    -1,  1396,    -1,
    1398,    -1,   163,    -1,   165,   166,  1736,   168,   169,   170,
    1740,  1741,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   471,    -1,    -1,  1305,  1755,    -1,    -1,    -1,    -1,
      -1,  1761,   193,  1431,    -1,    -1,   197,  1442,   199,    -1,
      -1,    -1,  1772,    -1,  1442,    -1,    -1,    -1,  1778,    81,
      -1,    -1,  1782,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,   511,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,  1804,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,   139,   140,   141,
      -1,    -1,    -1,    -1,  1844,    69,    -1,    -1,   568,   201,
      81,    -1,    83,    84,  1854,    -1,    -1,   159,    -1,    -1,
     162,   163,  1530,   165,   166,    -1,   168,   169,   170,    -1,
      -1,  1871,   103,    -1,    -1,    -1,    -1,    -1,    -1,   599,
    1880,    -1,    -1,    -1,    -1,    -1,  1554,    -1,  1556,    -1,
      -1,   193,  1892,    -1,  1894,  1563,    -1,    31,  1449,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,    -1,    -1,  1913,    -1,  1915,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    59,    -1,    -1,    -1,    -1,
      -1,    -1,   163,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,  1617,    -1,    -1,    -1,    -1,    81,    -1,  1617,
      -1,    27,    28,    -1,    -1,    31,    10,    11,    12,    -1,
      -1,   681,   193,    -1,    -1,    -1,   197,    -1,   199,   103,
      -1,    -1,    -1,    -1,   198,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,   138,   139,   140,   141,   142,    -1,
      -1,    81,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
     740,    -1,    -1,    -1,    -1,   159,    -1,    -1,   162,   163,
      -1,   165,   166,   103,   168,   169,   170,    -1,   172,    -1,
      -1,    -1,    -1,    -1,  1719,    -1,    -1,    31,    -1,   183,
      -1,  1719,    -1,    -1,    -1,   775,    -1,    -1,   128,   193,
     194,   781,    -1,   783,    -1,    -1,    -1,    -1,  1736,   139,
     140,   141,    -1,    -1,    -1,    59,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1755,    -1,    -1,
      -1,   811,   812,    -1,    -1,   165,   166,    81,   168,   169,
     170,    -1,    -1,    -1,    -1,    -1,    81,   827,   828,   829,
     830,   831,    -1,    -1,    -1,    -1,    -1,   837,    -1,   103,
      -1,    -1,    -1,   193,   194,    -1,    -1,    -1,   103,  1804,
      -1,   851,    -1,    -1,    -1,    -1,  1804,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,    -1,    -1,
     226,    -1,    -1,   873,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   887,    -1,   889,
     890,    -1,    -1,    -1,    81,   159,    -1,    -1,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,   162,    -1,    -1,
     165,   166,   912,   168,   169,   170,   103,    -1,    -1,   183,
      -1,   921,    -1,    -1,    -1,  1880,    -1,    -1,    -1,   193,
     194,    -1,  1880,    81,    -1,    -1,    -1,  1892,   193,  1894,
      -1,    -1,   197,    -1,  1892,    -1,  1894,    -1,   948,    -1,
      -1,   951,   139,   140,   141,   103,    -1,    -1,  1913,    -1,
    1915,    -1,    -1,    -1,    -1,  1913,    -1,  1915,    -1,    -1,
      -1,    -1,    -1,   973,    -1,    -1,    -1,   125,   165,   166,
      -1,   168,   169,   170,    -1,    -1,   986,    -1,    -1,    -1,
     138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   359,    -1,    -1,   193,   194,    -1,    -1,
      -1,   159,   368,    -1,   162,   163,    -1,   165,   166,   375,
     168,   169,   170,    -1,    -1,  1025,  1026,    -1,    -1,   385,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     396,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1059,
    1060,  1061,  1062,  1063,    -1,    -1,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,  1093,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
    1110,    12,    -1,    -1,    -1,   471,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,  1124,    -1,    -1,    -1,    -1,    -1,
      31,   568,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,   511,  1156,    -1,    -1,    -1,
      -1,    -1,   599,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,    -1,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,  1193,  1194,  1195,  1196,    -1,    -1,  1199,
      -1,  1201,    27,    -1,    29,  1205,    31,    -1,    -1,    -1,
      -1,    -1,   568,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1220,  1221,    -1,  1223,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    59,    -1,    -1,    -1,    -1,    -1,
    1240,    -1,  1242,   599,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1264,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,  1302,   128,    -1,  1305,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   681,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   159,    -1,    -1,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,   171,    -1,   173,    -1,
      -1,   176,    -1,    -1,  1354,    -1,  1356,    -1,   183,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,   194,
      -1,    -1,    -1,   198,    -1,   812,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,   740,    -1,    -1,    -1,    -1,    -1,
     827,   828,   829,   830,    -1,    -1,    -1,    -1,    -1,    -1,
     837,   103,    -1,    -1,  1404,    -1,  1406,    -1,  1408,   111,
     112,  1411,  1412,    -1,    -1,  1415,    -1,  1417,    -1,    81,
    1420,    83,    -1,    85,    -1,   781,    -1,   783,    -1,    -1,
      -1,    -1,  1432,    -1,    -1,  1435,    -1,   139,   140,   141,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1449,
      -1,    -1,    -1,    -1,    -1,   811,   812,    -1,    -1,    -1,
     162,    -1,    -1,   165,   166,    -1,   168,   169,   170,    -1,
      -1,   827,   828,   829,   830,   831,    -1,   139,   140,   141,
      -1,   837,    -1,    -1,    -1,  1485,    -1,  1487,    -1,  1489,
      -1,   193,  1492,    -1,  1494,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   165,   166,    -1,   168,   169,   170,    -1,
    1510,   948,    -1,    -1,    10,    11,    12,   873,    -1,    -1,
      -1,    -1,  1522,  1523,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   193,  1532,   889,   890,    31,    -1,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,   921,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1586,    -1,    -1,    -1,
      -1,    -1,   948,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1602,  1603,  1604,    -1,    -1,    -1,    -1,  1609,
      -1,  1611,    -1,    -1,    -1,    -1,    -1,   973,    -1,  1619,
      -1,    -1,    -1,  1060,  1061,  1062,    -1,    -1,    -1,  1066,
    1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,  1092,  1093,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1025,
    1026,    31,    -1,  1110,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,   195,
      -1,    -1,    -1,  1059,  1060,  1061,  1062,  1063,    -1,    69,
    1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,
    1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,    -1,    -1,
    1740,  1741,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,  1110,    -1,    -1,    -1,    -1,    -1,
      -1,  1761,    -1,    81,    -1,    -1,    -1,    -1,  1124,    -1,
      -1,   103,  1772,    -1,    -1,    -1,    -1,    -1,  1778,   111,
     112,    -1,  1782,    -1,    -1,   103,  1223,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1156,    -1,    -1,  1240,    -1,  1242,    -1,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,    -1,    -1,    -1,  1264,    -1,    -1,
     162,    -1,    -1,   165,   166,    -1,   168,   169,   170,    -1,
      -1,    -1,    -1,  1199,  1844,   163,    -1,   165,   166,  1205,
     168,   169,   170,    -1,  1854,    -1,    -1,    -1,    -1,    -1,
      -1,   193,    -1,    -1,  1220,  1221,    -1,  1223,    -1,    -1,
      -1,  1871,    -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1240,    -1,  1242,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1264,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,  1305,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,  1404,    -1,  1406,
      -1,  1408,    -1,    -1,  1411,    -1,    -1,    -1,  1415,    -1,
    1417,    30,    31,  1420,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,  1404,    13,
    1406,    69,  1408,    -1,    -1,  1411,  1412,    -1,    -1,  1415,
      -1,  1417,    -1,    -1,  1420,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1510,    38,    -1,    -1,   198,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,  1449,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1494,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,   198,
      -1,    -1,    -1,    -1,  1510,   119,   120,   121,   122,   123,
     124,    -1,    -1,   127,   128,  1602,  1603,  1604,    -1,    -1,
      -1,    -1,  1609,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,   172,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
     194,    -1,    81,    -1,    -1,   199,   200,    -1,   202,   203,
      10,    11,    12,    -1,    -1,    -1,  1602,  1603,  1604,    -1,
      -1,    -1,    -1,  1609,   103,    -1,    -1,    -1,    -1,    -1,
      30,    31,  1618,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
     139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   162,    -1,    -1,   165,   166,    -1,   168,
     169,   170,    -1,    -1,  1761,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,  1772,    -1,    -1,    -1,    -1,
      -1,  1778,    -1,    -1,   193,  1782,    -1,    -1,    -1,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,    69,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1761,    -1,  1844,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,  1772,    -1,    -1,    -1,
      -1,    -1,  1778,    -1,    -1,    -1,  1782,    -1,   198,    -1,
      49,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
    1806,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,    98,
      -1,   100,    -1,    -1,   103,   104,    -1,    -1,  1844,   108,
     109,   110,   111,   112,   113,   114,    -1,   116,   117,   118,
     119,   120,   121,   122,   123,   124,    -1,   126,   127,   128,
     129,   130,   131,    -1,    -1,    -1,   196,    -1,   137,   138,
     139,   140,   141,    -1,   143,   144,   145,   146,   147,    -1,
      -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,    81,
     159,   160,   161,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,    -1,
      -1,   103,    -1,    -1,   183,   184,    -1,   186,    -1,   188,
     189,   190,    -1,    -1,   193,   194,    -1,   196,   197,   198,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     162,    -1,    -1,   165,   166,    -1,   168,   169,   170,    -1,
      -1,    49,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,   193,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,
      98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,   112,   113,   114,    -1,   116,   117,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      81,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,   103,    -1,    -1,   183,   184,    -1,   186,    -1,
     188,   189,   190,    -1,    -1,   193,   194,    -1,   196,   197,
     198,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    49,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,   193,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,
      -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,   114,    -1,   116,
     117,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,    -1,   143,   144,   145,   146,
     147,    -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,
     177,    -1,    -1,    -1,    -1,    -1,   183,   184,    -1,   186,
      -1,   188,   189,   190,    -1,    -1,   193,   194,    -1,   196,
     197,    -1,   199,   200,    -1,   202,   203,     3,     4,     5,
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
      92,    93,    94,    95,    96,    -1,    98,    -1,   100,    -1,
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
     101,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
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
      98,    99,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,
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
      96,    97,    98,    -1,   100,    -1,    -1,   103,   104,    -1,
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
     137,   138,   139,   140,   141,    -1,   143,   144,   145,   146,
     147,    -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,
     177,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,
      -1,   188,   189,   190,    -1,    -1,   193,   194,    -1,   196,
     197,    -1,   199,   200,    -1,   202,   203,     3,     4,     5,
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
      -1,   137,   138,   139,   140,   141,    -1,   143,   144,   145,
     146,   147,    -1,    -1,    -1,   151,    -1,    -1,   154,    -1,
      -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,    -1,    -1,    -1,   174,    -1,
      -1,   177,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,
      -1,    -1,   188,   189,   190,    -1,    -1,   193,   194,    -1,
     196,   197,    -1,   199,   200,    -1,   202,   203,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   137,   138,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,   174,
      -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,    -1,
      -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,   194,
      -1,   196,   197,    -1,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
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
     194,    -1,    -1,    -1,    -1,   199,   200,    -1,   202,   203,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    29,    -1,    -1,    -1,
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
     193,   194,    -1,   196,    -1,    -1,   199,   200,    -1,   202,
     203,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,    -1,
      -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,    -1,
      -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,    -1,
      -1,   193,   194,    -1,    -1,   197,    -1,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    50,
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
      -1,   172,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,
      -1,    -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,
      -1,    -1,   193,   194,    -1,    -1,    -1,    -1,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
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
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,   108,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,    -1,   143,   144,   145,   146,   147,    -1,
      -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,
     159,   160,   161,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,    -1,
      -1,    -1,    -1,   198,   183,    -1,    -1,    -1,    -1,   188,
     189,   190,    -1,    -1,   193,   194,    -1,    -1,    -1,    -1,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
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
     177,    -1,    -1,    -1,    -1,   198,   183,    -1,    -1,    -1,
      -1,   188,   189,   190,    -1,    -1,   193,   194,    -1,   196,
      11,    12,   199,   200,    -1,   202,   203,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    69,    -1,
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
     196,    -1,    -1,   199,   200,    -1,   202,   203,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      10,    11,    12,    -1,   199,   200,    -1,   202,   203,     3,
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
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,   198,   183,
      -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,
     194,   195,    -1,    -1,    -1,   199,   200,    -1,   202,   203,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    32,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    50,    51,    -1,
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
      -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,    -1,
      -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,    -1,
      -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,    -1,
      -1,   193,   194,    -1,    -1,    -1,    -1,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,
      -1,    -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,
      -1,    -1,   193,   194,    -1,    -1,    -1,    -1,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
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
     190,    -1,    -1,   193,   194,    -1,    -1,    -1,    -1,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
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
     189,   190,    -1,    -1,   193,   194,    -1,    -1,    -1,    -1,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
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
       7,    -1,    -1,    -1,    -1,    -1,    13,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
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
      -1,   188,   189,   190,    -1,    -1,   193,   194,    10,    11,
      12,    -1,   199,   200,    -1,   202,   203,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    69,    -1,    -1,
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
      -1,   177,    -1,    -1,    -1,    -1,   198,   183,    -1,    -1,
      -1,    -1,   188,   189,   190,    -1,    -1,   193,   194,    10,
      11,    12,    -1,   199,   200,    -1,   202,   203,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    28,    13,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    69,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,   102,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,     3,     4,   174,
       6,     7,   177,    -1,    10,    11,    12,    13,   183,    -1,
      -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,   194,
      -1,    27,    -1,    29,   199,   200,    -1,   202,   203,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,    -1,   130,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,
      11,    12,    13,   159,    -1,    -1,    -1,    -1,    -1,   165,
     166,    -1,   168,   169,   170,   171,    27,   173,    29,    -1,
     176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,
      -1,   197,    -1,   199,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,    -1,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,   159,    10,
      11,    12,    13,    -1,   165,   166,    -1,   168,   169,   170,
     171,    -1,   173,    -1,    -1,   176,    27,    -1,    29,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   193,    -1,    -1,    -1,   197,    -1,   199,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,    -1,    -1,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,
      -1,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
     171,    -1,   173,    -1,    -1,   176,     3,     4,    -1,     6,
       7,    -1,   183,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,   193,   194,    -1,    -1,    -1,   198,    -1,    -1,
      27,    -1,    29,    31,    31,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      57,    -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,    -1,   130,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   159,    -1,    -1,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,   171,    -1,   173,    -1,    -1,   176,
       3,     4,    -1,     6,     7,    -1,   183,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,   193,   194,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    29,    -1,    31,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    57,    -1,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,    -1,    -1,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,   171,    -1,
     173,    -1,    -1,   176,    -1,     3,     4,    -1,     6,     7,
     183,   184,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
     193,   194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    29,    -1,    31,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    59,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   159,    -1,    -1,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,   173,    -1,    -1,   176,     3,
       4,     5,     6,     7,    -1,   183,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   193,   194,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    57,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   159,   160,   161,    -1,    -1,
      -1,   165,   166,    -1,   168,   169,   170,   171,    -1,   173,
     174,    -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,   183,
     184,    -1,   186,    -1,   188,   189,    -1,     3,     4,   193,
       6,     7,    11,    12,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    31,    29,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,    -1,   130,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,
      11,    12,    13,   159,    -1,    -1,    -1,    -1,    -1,   165,
     166,    -1,   168,   169,   170,   171,    27,   173,    29,    -1,
     176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,    -1,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,   159,    -1,
      -1,    -1,    -1,    -1,   165,   166,    -1,   168,   169,   170,
     171,    27,   173,    29,    -1,   176,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   193,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,    -1,    -1,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,   165,
     166,    -1,   168,   169,   170,   171,    -1,   173,    -1,    -1,
     176,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,
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
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   198,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   196,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   196,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,   196,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      30,    31,   196,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    59,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,
     196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    32,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    -1,    -1,    -1,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    70,    -1,    -1,   195,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    -1,   159,    -1,    -1,   162,
     163,    -1,   165,   166,    91,   168,   169,   170,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
     183,    -1,    -1,    -1,    -1,    -1,   189,    -1,    -1,    -1,
     193,   194,    -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,    -1,   143,   144,   145,   146,
     147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    38,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,
      70,   188,    -1,    -1,    -1,    -1,   193,   194,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,   143,   144,   145,   146,   147,    -1,    -1,
      -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,
     160,   161,   162,   163,    -1,   165,   166,    -1,   168,   169,
     170,    -1,    50,    51,   174,    -1,    -1,    -1,    56,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   188,    -1,
      -1,    -1,    70,   193,   194,    -1,    -1,    -1,    -1,   199,
      78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,   103,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   183,   138,   139,   140,   141,
     188,    -1,    -1,    -1,    -1,   193,   194,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,    -1,    -1,   159,    -1,    -1,
     162,   163,    70,   165,   166,    -1,   168,   169,   170,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      -1,   183,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   193,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    70,    -1,    -1,   174,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
     188,    -1,    -1,    -1,    91,   193,   194,    -1,    -1,   197,
      -1,   199,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,    -1,   143,   144,   145,   146,
     147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,    70,    -1,    72,   174,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,   188,    -1,    -1,    -1,    91,   193,   194,    -1,    -1,
      -1,    -1,   199,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,    -1,   143,   144,   145,
     146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,
      -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,    70,    -1,    -1,   174,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,   188,    -1,    -1,    -1,    91,   193,   194,    -1,
      -1,    -1,    -1,   199,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    70,    -1,    -1,   174,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,   188,    -1,    -1,    -1,    91,   193,   194,
      -1,    -1,    -1,    -1,   199,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,
     174,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,   193,
     194,    -1,    -1,    30,    31,   199,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   136,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,   136,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,   136,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,   136,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,   136,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,   136,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,   136,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   136,
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
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     389,   399,   400,   401,   403,   408,   413,   433,   441,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   468,   470,   472,   122,   123,   124,   137,   159,   169,
     194,   211,   244,   325,   346,   445,   346,   194,   346,   346,
     346,   108,   346,   346,   346,   431,   432,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,    81,
      83,    91,   124,   139,   140,   141,   154,   194,   222,   365,
     400,   403,   408,   445,   448,   445,    38,   346,   459,   460,
     346,   124,   130,   194,   222,   257,   400,   401,   402,   404,
     408,   442,   443,   444,   452,   456,   457,   194,   335,   405,
     194,   335,   356,   336,   346,   230,   335,   194,   194,   194,
     335,   196,   346,   211,   196,   346,     3,     4,     6,     7,
      10,    11,    12,    13,    27,    29,    31,    57,    59,    71,
      72,    73,    74,    75,    76,    77,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     128,   130,   131,   132,   133,   137,   138,   142,   159,   163,
     171,   173,   176,   183,   194,   211,   212,   213,   224,   473,
     493,   494,   497,   196,   341,   343,   346,   197,   237,   346,
     111,   112,   162,   214,   215,   216,   217,   221,    83,   199,
     291,   292,   123,   130,   122,   130,    83,   293,   194,   194,
     194,   194,   211,   263,   476,   194,   194,    70,    70,    70,
     336,    83,    90,   155,   156,   157,   465,   466,   162,   197,
     221,   221,   211,   264,   476,   163,   194,   476,   476,    83,
     190,   197,   357,    27,   334,   338,   346,   347,   445,   449,
     226,   197,   454,    90,   406,   465,    90,   465,   465,    32,
     162,   179,   477,   194,     9,   196,    38,   243,   163,   262,
     476,   124,   189,   244,   326,   196,   196,   196,   196,   196,
     196,   196,   196,    10,    11,    12,    30,    31,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    57,    69,   196,    70,    70,   197,   158,   131,   169,
     171,   184,   186,   265,   324,   325,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    67,
      68,   134,   135,   435,    70,   197,   440,   194,   194,    70,
     197,   194,   243,   244,    14,   346,   196,   136,    48,   211,
     430,    90,   334,   347,   158,   445,   136,   201,     9,   415,
     258,   334,   347,   445,   477,   158,   194,   407,   435,   440,
     195,   346,    32,   228,     8,   358,     9,   196,   228,   229,
     336,   337,   346,   211,   277,   232,   196,   196,   196,   138,
     142,   497,   497,   179,   496,   194,   111,   497,    14,   158,
     138,   142,   159,   211,   213,   196,   196,   196,   238,   115,
     176,   196,   214,   216,   214,   216,   221,   197,     9,   416,
     196,   102,   162,   197,   445,     9,   196,   130,   130,    14,
       9,   196,   445,   469,   336,   334,   347,   445,   448,   449,
     195,   179,   255,   137,   445,   458,   459,   346,   366,   367,
     336,   381,   381,   196,    70,   435,   155,   466,    82,   346,
     445,    90,   155,   466,   221,   210,   196,   197,   250,   260,
     390,   392,    91,   194,   359,   360,   362,   403,   451,   453,
     470,    14,   102,   471,   353,   354,   355,   287,   288,   433,
     434,   195,   195,   195,   195,   195,   198,   227,   228,   245,
     252,   259,   433,   346,   200,   202,   203,   211,   478,   479,
     497,    38,   172,   289,   290,   346,   473,   194,   476,   253,
     243,   346,   346,   346,   346,    32,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     404,   346,   346,   455,   455,   346,   461,   462,   130,   197,
     212,   213,   454,   263,   211,   264,   476,   476,   262,   244,
      38,   338,   341,   343,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   163,   197,   211,
     436,   437,   438,   439,   454,   455,   346,   289,   289,   455,
     346,   458,   243,   195,   346,   194,   429,     9,   415,   195,
     195,    38,   346,    38,   346,   407,   195,   195,   195,   452,
     453,   454,   289,   197,   211,   436,   437,   454,   195,   226,
     281,   197,   343,   346,   346,    94,    32,   228,   275,   196,
      28,   102,    14,     9,   195,    32,   197,   278,   497,    31,
      91,   224,   490,   491,   492,   194,     9,    50,    51,    56,
      58,    70,   138,   139,   140,   141,   183,   194,   222,   373,
     376,   379,   385,   400,   408,   409,   411,   412,   211,   495,
     226,   194,   236,   197,   196,   197,   196,   102,   162,   111,
     112,   162,   217,   218,   219,   220,   221,   217,   211,   346,
     292,   409,    83,     9,   195,   195,   195,   195,   195,   195,
     195,   196,    50,    51,   486,   488,   489,   132,   268,   194,
       9,   195,   195,   136,   201,     9,   415,     9,   415,   201,
     201,    83,    85,   211,   467,   211,    70,   198,   198,   207,
     209,    32,   133,   267,   178,    54,   163,   178,   394,   347,
     136,     9,   415,   195,   158,   497,   497,    14,   358,   287,
     226,   191,     9,   416,   497,   498,   435,   440,   435,   198,
       9,   415,   180,   445,   346,   195,     9,   416,    14,   350,
     246,   132,   266,   194,   476,   346,    32,   201,   201,   136,
     198,     9,   415,   346,   477,   194,   256,   251,   261,    14,
     471,   254,   243,    72,   445,   346,   477,   201,   198,   195,
     195,   201,   198,   195,    50,    51,    70,    78,    79,    80,
      91,   138,   139,   140,   141,   154,   183,   211,   374,   377,
     380,   400,   411,   418,   420,   421,   425,   428,   211,   445,
     445,   136,   266,   435,   440,   195,   346,   282,    75,    76,
     283,   226,   335,   226,   337,   102,    38,   137,   272,   445,
     409,   211,    32,   228,   276,   196,   279,   196,   279,     9,
     415,    91,   136,   158,     9,   415,   195,   172,   478,   479,
     480,   478,   409,   409,   409,   409,   409,   414,   417,   194,
      70,    70,    70,   194,   409,   158,   197,    10,    11,    12,
      31,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    69,   158,   477,   198,   400,   197,   240,
     216,   216,   211,   217,   217,   221,     9,   416,   198,   198,
      14,   445,   196,   180,     9,   415,   211,   269,   400,   197,
     458,   137,   445,    14,    38,   346,   346,   201,   346,   198,
     207,   497,   269,   197,   393,    14,   195,   346,   359,   454,
     196,   497,   191,   198,    32,   484,   434,    38,    83,   172,
     436,   437,   439,   436,   437,   497,    38,   172,   346,   409,
     287,   194,   400,   267,   351,   247,   346,   346,   346,   198,
     194,   289,   268,    32,   267,   497,    14,   266,   476,   404,
     198,   194,    14,    78,    79,    80,   211,   419,   419,   421,
     423,   424,    52,   194,    70,    70,    70,    90,   155,   194,
     158,     9,   415,   195,   429,    38,   346,   267,   198,    75,
      76,   284,   335,   228,   198,   196,    95,   196,   272,   445,
     194,   136,   271,    14,   226,   279,   105,   106,   107,   279,
     198,   497,   180,   136,   497,   211,   490,     9,   195,   415,
     136,   201,     9,   415,   414,   368,   369,   409,   382,   409,
     410,   382,   359,   361,   363,   195,   130,   212,   409,   463,
     464,   409,   409,   409,    32,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   495,
      83,   241,   198,   198,   220,   196,   409,   489,   102,   103,
     485,   487,     9,   297,   195,   194,   338,   343,   346,   445,
     136,   201,   198,   471,   297,   164,   177,   197,   389,   396,
     164,   197,   395,   136,   196,   484,   497,   358,   498,    83,
     172,    14,    83,   477,   445,   346,   195,   287,   197,   287,
     194,   136,   194,   289,   195,   197,   497,   197,   196,   497,
     267,   248,   407,   289,   136,   201,     9,   415,   420,   423,
     370,   371,   421,   383,   421,   422,   383,   155,   359,   426,
     427,    81,   421,   445,   197,   335,    32,    77,   228,   196,
     337,   271,   458,   272,   195,   409,   101,   105,   196,   346,
      32,   196,   280,   198,   180,   497,   136,   172,    32,   195,
     409,   409,   195,   201,     9,   415,   136,   201,     9,   415,
     201,   136,     9,   415,   195,   136,   198,     9,   415,   409,
      32,   195,   226,   196,   196,   211,   497,   497,   485,   400,
       4,   112,   117,   123,   125,   165,   166,   168,   198,   298,
     323,   324,   325,   330,   331,   332,   333,   433,   458,    38,
     346,   198,   197,   198,    54,   346,   346,   346,   358,    38,
      83,   172,    14,    83,   346,   194,   484,   195,   297,   195,
     287,   346,   289,   195,   297,   471,   297,   196,   197,   194,
     195,   421,   421,   195,   201,     9,   415,   136,   201,     9,
     415,   201,   136,   195,     9,   415,   297,    32,   226,   196,
     195,   195,   195,   233,   196,   196,   280,   226,   497,   497,
     136,   409,   409,   409,   409,   359,   409,   409,   409,   197,
     198,   487,   132,   133,   184,   212,   474,   497,   270,   400,
     112,   333,    31,   125,   138,   142,   163,   169,   307,   308,
     309,   310,   400,   167,   315,   316,   128,   194,   211,   317,
     318,   299,   244,   497,     9,   196,     9,   196,   196,   471,
     324,   195,   445,   294,   163,   391,   198,   198,    83,   172,
      14,    83,   346,   289,   117,   348,   484,   198,   484,   195,
     195,   198,   197,   198,   297,   287,   136,   421,   421,   421,
     421,   359,   198,   226,   231,   234,    32,   228,   274,   226,
     195,   409,   136,   136,   136,   226,   400,   400,   476,    14,
     212,     9,   196,   197,   474,   471,   310,   179,   197,     9,
     196,     3,     4,     5,     6,     7,    10,    11,    12,    13,
      27,    28,    29,    57,    71,    72,    73,    74,    75,    76,
      77,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   137,   138,   143,   144,   145,   146,   147,   159,
     160,   161,   171,   173,   174,   176,   183,   184,   186,   188,
     189,   211,   397,   398,     9,   196,   163,   167,   211,   318,
     319,   320,   196,    83,   329,   243,   300,   474,   474,    14,
     244,   198,   295,   296,   474,    14,    83,   346,   195,   194,
     484,   196,   197,   321,   348,   484,   294,   198,   195,   421,
     136,   136,    32,   228,   273,   274,   226,   409,   409,   409,
     198,   196,   196,   409,   400,   303,   497,   311,   312,   408,
     308,    14,    32,    51,   313,   316,     9,    36,   195,    31,
      50,    53,    14,     9,   196,   213,   475,   329,    14,   497,
     243,   196,    14,   346,    38,    83,   388,   197,   226,   484,
     321,   198,   484,   421,   421,   226,    99,   239,   198,   211,
     224,   304,   305,   306,     9,   415,     9,   415,   198,   409,
     398,   398,    59,   314,   319,   319,    31,    50,    53,   409,
      83,   179,   194,   196,   409,   476,   409,    83,     9,   416,
     226,   198,   197,   321,    97,   196,   115,   235,   158,   102,
     497,   180,   408,   170,    14,   486,   301,   194,    38,    83,
     195,   198,   226,   196,   194,   176,   242,   211,   324,   325,
     180,   409,   180,   285,   286,   434,   302,    83,   198,   400,
     240,   173,   211,   196,   195,     9,   416,   119,   120,   121,
     327,   328,   285,    83,   270,   196,   484,   434,   498,   195,
     195,   196,   193,   481,   327,    38,    83,   172,   484,   197,
     482,   483,   497,   196,   197,   322,   498,    83,   172,    14,
      83,   481,   226,     9,   416,    14,   485,   226,    38,    83,
     172,    14,    83,   346,   322,   198,   483,   497,   198,    83,
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
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2660 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2661 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2665 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2669 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2672 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2677 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2678 "hphp.y"
    { (yyval).reset();;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2682 "hphp.y"
    { (yyval).reset();;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2683 "hphp.y"
    { (yyval).reset();;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2686 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    { (yyval).reset();;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2693 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2698 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2702 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2707 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2709 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2715 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2722 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2729 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2730 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2737 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2738 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2740 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2743 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2746 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2751 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2757 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2758 "hphp.y"
    { (yyval).reset();;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2763 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2767 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2772 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2778 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2787 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2793 "hphp.y"
    { (yyval).reset();;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2796 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2797 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2804 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2806 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2809 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2811 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2817 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2818 "hphp.y"
    { (yyval).reset();;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2822 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2823 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2827 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2828 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2829 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2833 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2844 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2848 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2858 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2859 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2863 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2865 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2872 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2878 "hphp.y"
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
#line 2889 "hphp.y"
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
#line 2904 "hphp.y"
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
#line 2916 "hphp.y"
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

  case 874:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2929 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2931 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2932 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2933 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2935 "hphp.y"
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

  case 881:

/* Line 1455 of yacc.c  */
#line 2952 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2954 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2957 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
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

  case 890:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2983 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
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
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2998 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2999 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3001 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3003 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3007 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3011 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3012 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3018 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3038 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3042 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3056 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3057 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { _p->onPipeVariable((yyval));;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3063 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3065 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3070 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3071 "hphp.y"
    { (yyval).reset();;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3082 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3083 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3084 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3087 "hphp.y"
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
#line 3098 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3099 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3103 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3104 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3107 "hphp.y"
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
#line 3116 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3123 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3124 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3125 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3126 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3131 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { (yyval).reset();;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3136 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3138 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3139 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3142 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3144 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3145 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3146 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3151 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3152 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3156 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3157 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3158 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3159 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3164 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3165 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3170 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3172 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3174 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3175 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3179 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3181 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3182 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3184 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3189 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3191 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3193 "hphp.y"
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
#line 3203 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3205 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3206 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3209 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3210 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3215 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3216 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3217 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3218 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3219 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3220 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3221 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3222 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3223 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3224 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3225 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3229 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3230 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3235 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3237 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3251 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3256 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3260 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3265 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 995:

/* Line 1455 of yacc.c  */
#line 3272 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 996:

/* Line 1455 of yacc.c  */
#line 3276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3277 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3283 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3287 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3293 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3297 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3304 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3305 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1004:

/* Line 1455 of yacc.c  */
#line 3309 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1005:

/* Line 1455 of yacc.c  */
#line 3312 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1006:

/* Line 1455 of yacc.c  */
#line 3318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3323 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3324 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3325 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1010:

/* Line 1455 of yacc.c  */
#line 3326 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3347 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1018:

/* Line 1455 of yacc.c  */
#line 3348 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1; ;}
    break;

  case 1021:

/* Line 1455 of yacc.c  */
#line 3357 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1024:

/* Line 1455 of yacc.c  */
#line 3368 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 1025:

/* Line 1455 of yacc.c  */
#line 3370 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 1026:

/* Line 1455 of yacc.c  */
#line 3374 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 1027:

/* Line 1455 of yacc.c  */
#line 3377 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 1028:

/* Line 1455 of yacc.c  */
#line 3381 "hphp.y"
    {;}
    break;

  case 1029:

/* Line 1455 of yacc.c  */
#line 3382 "hphp.y"
    {;}
    break;

  case 1030:

/* Line 1455 of yacc.c  */
#line 3383 "hphp.y"
    {;}
    break;

  case 1031:

/* Line 1455 of yacc.c  */
#line 3389 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 1032:

/* Line 1455 of yacc.c  */
#line 3394 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 1033:

/* Line 1455 of yacc.c  */
#line 3405 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 1034:

/* Line 1455 of yacc.c  */
#line 3410 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1035:

/* Line 1455 of yacc.c  */
#line 3411 "hphp.y"
    { ;}
    break;

  case 1036:

/* Line 1455 of yacc.c  */
#line 3416 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 1037:

/* Line 1455 of yacc.c  */
#line 3417 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 1038:

/* Line 1455 of yacc.c  */
#line 3423 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1039:

/* Line 1455 of yacc.c  */
#line 3428 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1040:

/* Line 1455 of yacc.c  */
#line 3433 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1041:

/* Line 1455 of yacc.c  */
#line 3437 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1042:

/* Line 1455 of yacc.c  */
#line 3442 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 1043:

/* Line 1455 of yacc.c  */
#line 3444 "hphp.y"
    { _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)])); (yyval) = (yyvsp[(2) - (5)]);;}
    break;

  case 1044:

/* Line 1455 of yacc.c  */
#line 3450 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1045:

/* Line 1455 of yacc.c  */
#line 3453 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1046:

/* Line 1455 of yacc.c  */
#line 3456 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1047:

/* Line 1455 of yacc.c  */
#line 3457 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1048:

/* Line 1455 of yacc.c  */
#line 3460 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1049:

/* Line 1455 of yacc.c  */
#line 3463 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1050:

/* Line 1455 of yacc.c  */
#line 3466 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1051:

/* Line 1455 of yacc.c  */
#line 3469 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1052:

/* Line 1455 of yacc.c  */
#line 3471 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1053:

/* Line 1455 of yacc.c  */
#line 3477 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1054:

/* Line 1455 of yacc.c  */
#line 3483 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1055:

/* Line 1455 of yacc.c  */
#line 3491 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1056:

/* Line 1455 of yacc.c  */
#line 3492 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14620 "hphp.7.tab.cpp"
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
#line 3495 "hphp.y"

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

