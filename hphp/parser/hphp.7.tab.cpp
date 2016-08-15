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
#define YYLAST   18408

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  204
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  294
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1054
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1937

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
    2898,  2902,  2906,  2908,  2910,  2912,  2914,  2916,  2920,  2924,
    2929,  2934,  2938,  2940,  2942,  2950,  2960,  2968,  2975,  2984,
    2986,  2991,  2996,  2998,  3000,  3002,  3007,  3010,  3012,  3013,
    3015,  3017,  3019,  3023,  3027,  3031,  3032,  3034,  3036,  3040,
    3044,  3047,  3051,  3058,  3059,  3061,  3066,  3069,  3070,  3076,
    3080,  3084,  3086,  3093,  3098,  3103,  3106,  3109,  3110,  3116,
    3120,  3124,  3126,  3129,  3130,  3136,  3140,  3144,  3146,  3149,
    3152,  3154,  3157,  3159,  3164,  3168,  3172,  3179,  3183,  3185,
    3187,  3189,  3194,  3199,  3204,  3209,  3214,  3219,  3222,  3225,
    3230,  3233,  3236,  3238,  3242,  3246,  3250,  3251,  3254,  3260,
    3267,  3274,  3282,  3284,  3287,  3289,  3292,  3294,  3299,  3301,
    3306,  3310,  3311,  3313,  3317,  3320,  3324,  3326,  3328,  3329,
    3330,  3334,  3336,  3340,  3344,  3347,  3348,  3351,  3354,  3357,
    3360,  3362,  3365,  3370,  3373,  3379,  3383,  3385,  3387,  3388,
    3392,  3397,  3403,  3407,  3409,  3412,  3413,  3418,  3420,  3424,
    3427,  3432,  3438,  3441,  3444,  3446,  3448,  3450,  3452,  3456,
    3459,  3461,  3470,  3477,  3479
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
      -1,   115,    -1,   126,    -1,   212,    -1,   130,    -1,   221,
     162,    -1,   162,   221,   162,    -1,   215,     9,   217,    -1,
     217,    -1,   215,   415,    -1,   221,    -1,   162,   221,    -1,
     221,   102,   211,    -1,   162,   221,   102,   211,    -1,   218,
       9,   220,    -1,   220,    -1,   218,   415,    -1,   217,    -1,
     111,   217,    -1,   112,   217,    -1,   211,    -1,   221,   162,
     211,    -1,   221,    -1,   159,   162,   221,    -1,   162,   221,
      -1,   222,   476,    -1,   222,   476,    -1,   225,     9,   472,
      14,   409,    -1,   112,   472,    14,   409,    -1,   226,   227,
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
     196,    -1,   126,   194,   468,   195,   196,    -1,   196,    -1,
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
      -1,   111,    -1,    -1,   244,   243,   475,   246,   194,   287,
     195,   483,   321,    -1,    -1,   325,   244,   243,   475,   247,
     194,   287,   195,   483,   321,    -1,    -1,   432,   324,   244,
     243,   475,   248,   194,   287,   195,   483,   321,    -1,    -1,
     169,   211,   250,    32,   496,   470,   197,   294,   198,    -1,
      -1,   432,   169,   211,   251,    32,   496,   470,   197,   294,
     198,    -1,    -1,   265,   262,   253,   266,   267,   197,   297,
     198,    -1,    -1,   432,   265,   262,   254,   266,   267,   197,
     297,   198,    -1,    -1,   131,   263,   255,   268,   197,   297,
     198,    -1,    -1,   432,   131,   263,   256,   268,   197,   297,
     198,    -1,    -1,   130,   258,   407,   266,   267,   197,   297,
     198,    -1,    -1,   171,   264,   260,   267,   197,   297,   198,
      -1,    -1,   432,   171,   264,   261,   267,   197,   297,   198,
      -1,   475,    -1,   163,    -1,   475,    -1,   475,    -1,   130,
      -1,   123,   130,    -1,   123,   122,   130,    -1,   122,   123,
     130,    -1,   122,   130,    -1,   132,   400,    -1,    -1,   133,
     269,    -1,    -1,   132,   269,    -1,    -1,   400,    -1,   269,
       9,   400,    -1,   400,    -1,   270,     9,   400,    -1,   136,
     272,    -1,    -1,   444,    -1,    38,   444,    -1,   137,   194,
     457,   195,    -1,   228,    -1,    32,   226,    97,   196,    -1,
     228,    -1,    32,   226,    99,   196,    -1,   228,    -1,    32,
     226,    95,   196,    -1,   228,    -1,    32,   226,   101,   196,
      -1,   211,    14,   409,    -1,   277,     9,   211,    14,   409,
      -1,   197,   279,   198,    -1,   197,   196,   279,   198,    -1,
      32,   279,   105,   196,    -1,    32,   196,   279,   105,   196,
      -1,   279,   106,   346,   280,   226,    -1,   279,   107,   280,
     226,    -1,    -1,    32,    -1,   196,    -1,   281,    75,   335,
     228,    -1,    -1,   282,    75,   335,    32,   226,    -1,    -1,
      76,   228,    -1,    -1,    76,    32,   226,    -1,    -1,   286,
       9,   433,   327,   497,   172,    83,    -1,   286,     9,   433,
     327,   497,    38,   172,    83,    -1,   286,     9,   433,   327,
     497,   172,    -1,   286,   415,    -1,   433,   327,   497,   172,
      83,    -1,   433,   327,   497,    38,   172,    83,    -1,   433,
     327,   497,   172,    -1,    -1,   433,   327,   497,    83,    -1,
     433,   327,   497,    38,    83,    -1,   433,   327,   497,    38,
      83,    14,   346,    -1,   433,   327,   497,    83,    14,   346,
      -1,   286,     9,   433,   327,   497,    83,    -1,   286,     9,
     433,   327,   497,    38,    83,    -1,   286,     9,   433,   327,
     497,    38,    83,    14,   346,    -1,   286,     9,   433,   327,
     497,    83,    14,   346,    -1,   288,     9,   433,   497,   172,
      83,    -1,   288,     9,   433,   497,    38,   172,    83,    -1,
     288,     9,   433,   497,   172,    -1,   288,   415,    -1,   433,
     497,   172,    83,    -1,   433,   497,    38,   172,    83,    -1,
     433,   497,   172,    -1,    -1,   433,   497,    83,    -1,   433,
     497,    38,    83,    -1,   433,   497,    38,    83,    14,   346,
      -1,   433,   497,    83,    14,   346,    -1,   288,     9,   433,
     497,    83,    -1,   288,     9,   433,   497,    38,    83,    -1,
     288,     9,   433,   497,    38,    83,    14,   346,    -1,   288,
       9,   433,   497,    83,    14,   346,    -1,   290,   415,    -1,
      -1,   346,    -1,    38,   444,    -1,   172,   346,    -1,   290,
       9,   346,    -1,   290,     9,   172,   346,    -1,   290,     9,
      38,   444,    -1,   291,     9,   292,    -1,   292,    -1,    83,
      -1,   199,   444,    -1,   199,   197,   346,   198,    -1,   293,
       9,    83,    -1,   293,     9,    83,    14,   409,    -1,    83,
      -1,    83,    14,   409,    -1,   294,   295,    -1,    -1,   296,
     196,    -1,   473,    14,   409,    -1,   297,   298,    -1,    -1,
      -1,   323,   299,   329,   196,    -1,    -1,   325,   496,   300,
     329,   196,    -1,   330,   196,    -1,   331,   196,    -1,   332,
     196,    -1,    -1,   324,   244,   243,   474,   194,   301,   285,
     195,   483,   480,   322,    -1,    -1,   432,   324,   244,   243,
     475,   194,   302,   285,   195,   483,   480,   322,    -1,   165,
     307,   196,    -1,   166,   315,   196,    -1,   168,   317,   196,
      -1,     4,   132,   400,   196,    -1,     4,   133,   400,   196,
      -1,   117,   270,   196,    -1,   117,   270,   197,   303,   198,
      -1,   303,   304,    -1,   303,   305,    -1,    -1,   224,   158,
     211,   173,   270,   196,    -1,   306,   102,   324,   211,   196,
      -1,   306,   102,   325,   196,    -1,   224,   158,   211,    -1,
     211,    -1,   308,    -1,   307,     9,   308,    -1,   309,   397,
     313,   314,    -1,   163,    -1,    31,   310,    -1,   310,    -1,
     138,    -1,   138,   179,   496,   414,   180,    -1,   138,   179,
     496,     9,   496,   180,    -1,   400,    -1,   125,    -1,   169,
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
      83,    -1,    83,    14,   409,    -1,   330,     9,   473,    14,
     409,    -1,   112,   473,    14,   409,    -1,   331,     9,   473,
      -1,   123,   112,   473,    -1,   123,   333,   470,    -1,   333,
     470,    14,   496,    -1,   112,   184,   475,    -1,   194,   334,
     195,    -1,    72,   404,   407,    -1,    72,   257,    -1,    71,
     346,    -1,   389,    -1,   384,    -1,   194,   346,   195,    -1,
     336,     9,   346,    -1,   346,    -1,   336,    -1,    -1,    27,
      -1,    27,   346,    -1,    27,   346,   136,   346,    -1,   194,
     338,   195,    -1,   444,    14,   338,    -1,   137,   194,   457,
     195,    14,   338,    -1,    29,   346,    -1,   444,    14,   341,
      -1,    28,   346,    -1,   444,    14,   343,    -1,   137,   194,
     457,   195,    14,   343,    -1,   347,    -1,   444,    -1,   334,
      -1,   448,    -1,   447,    -1,   137,   194,   457,   195,    14,
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
      -1,   467,    -1,    66,   346,    -1,    65,   346,    -1,    64,
     346,    -1,    63,   346,    -1,    62,   346,    -1,    61,   346,
      -1,    60,   346,    -1,    73,   405,    -1,    59,   346,    -1,
     412,    -1,   365,    -1,   372,    -1,   375,    -1,   378,    -1,
     364,    -1,   200,   406,   200,    -1,    13,   346,    -1,   386,
      -1,   117,   194,   388,   415,   195,    -1,    -1,    -1,   244,
     243,   194,   350,   287,   195,   483,   348,   483,   197,   226,
     198,    -1,    -1,   325,   244,   243,   194,   351,   287,   195,
     483,   348,   483,   197,   226,   198,    -1,    -1,   189,    83,
     353,   358,    -1,    -1,   189,   190,   354,   287,   191,   483,
     358,    -1,    -1,   189,   197,   355,   226,   198,    -1,    -1,
      83,   356,   358,    -1,    -1,   190,   357,   287,   191,   483,
     358,    -1,     8,   346,    -1,     8,   343,    -1,     8,   197,
     226,   198,    -1,    91,    -1,   469,    -1,   360,     9,   359,
     136,   346,    -1,   359,   136,   346,    -1,   361,     9,   359,
     136,   409,    -1,   359,   136,   409,    -1,   360,   414,    -1,
      -1,   361,   414,    -1,    -1,   183,   194,   362,   195,    -1,
     138,   194,   458,   195,    -1,    70,   458,   201,    -1,   367,
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
     400,   197,   460,   198,    -1,   400,   197,   462,   198,    -1,
     386,    70,   454,   201,    -1,   387,    70,   454,   201,    -1,
     365,    -1,   372,    -1,   375,    -1,   378,    -1,   469,    -1,
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
      -1,   401,    -1,   402,   158,   453,    -1,   401,    -1,   450,
      -1,   403,   158,   453,    -1,   400,    -1,   124,    -1,   455,
      -1,   194,   195,    -1,   335,    -1,    -1,    -1,    90,    -1,
     464,    -1,   194,   289,   195,    -1,    -1,    78,    -1,    79,
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
      -1,   222,    -1,    82,    -1,   469,    -1,   408,    -1,   202,
     464,   202,    -1,   203,   464,   203,    -1,   154,   464,   155,
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
     346,   198,    -1,   435,    -1,   453,    -1,   211,    -1,   197,
     346,   198,    -1,   437,    -1,   453,    -1,    70,   454,   201,
      -1,   197,   346,   198,    -1,   445,   439,    -1,   194,   334,
     195,   439,    -1,   456,   439,    -1,   194,   334,   195,   439,
      -1,   194,   334,   195,   434,   436,    -1,   194,   347,   195,
     434,   436,    -1,   194,   334,   195,   434,   435,    -1,   194,
     347,   195,   434,   435,    -1,   451,    -1,   399,    -1,   449,
      -1,   450,    -1,   440,    -1,   442,    -1,   444,   434,   436,
      -1,   403,   158,   453,    -1,   446,   194,   289,   195,    -1,
     447,   194,   289,   195,    -1,   194,   444,   195,    -1,   399,
      -1,   449,    -1,   450,    -1,   440,    -1,   444,   434,   436,
      -1,   443,    -1,   446,   194,   289,   195,    -1,   194,   444,
     195,    -1,   403,   158,   453,    -1,   451,    -1,   440,    -1,
     399,    -1,   365,    -1,   408,    -1,   194,   444,   195,    -1,
     194,   347,   195,    -1,   447,   194,   289,   195,    -1,   446,
     194,   289,   195,    -1,   194,   448,   195,    -1,   349,    -1,
     352,    -1,   444,   434,   438,   476,   194,   289,   195,    -1,
     194,   334,   195,   434,   438,   476,   194,   289,   195,    -1,
     403,   158,   213,   476,   194,   289,   195,    -1,   403,   158,
     453,   194,   289,   195,    -1,   403,   158,   197,   346,   198,
     194,   289,   195,    -1,   452,    -1,   452,    70,   454,   201,
      -1,   452,   197,   346,   198,    -1,   453,    -1,    83,    -1,
      84,    -1,   199,   197,   346,   198,    -1,   199,   453,    -1,
     346,    -1,    -1,   451,    -1,   441,    -1,   442,    -1,   455,
     434,   436,    -1,   402,   158,   451,    -1,   194,   444,   195,
      -1,    -1,   441,    -1,   443,    -1,   455,   434,   435,    -1,
     194,   444,   195,    -1,   457,     9,    -1,   457,     9,   444,
      -1,   457,     9,   137,   194,   457,   195,    -1,    -1,   444,
      -1,   137,   194,   457,   195,    -1,   459,   414,    -1,    -1,
     459,     9,   346,   136,   346,    -1,   459,     9,   346,    -1,
     346,   136,   346,    -1,   346,    -1,   459,     9,   346,   136,
      38,   444,    -1,   459,     9,    38,   444,    -1,   346,   136,
      38,   444,    -1,    38,   444,    -1,   461,   414,    -1,    -1,
     461,     9,   346,   136,   346,    -1,   461,     9,   346,    -1,
     346,   136,   346,    -1,   346,    -1,   463,   414,    -1,    -1,
     463,     9,   409,   136,   409,    -1,   463,     9,   409,    -1,
     409,   136,   409,    -1,   409,    -1,   464,   465,    -1,   464,
      90,    -1,   465,    -1,    90,   465,    -1,    83,    -1,    83,
      70,   466,   201,    -1,    83,   434,   211,    -1,   156,   346,
     198,    -1,   156,    82,    70,   346,   201,   198,    -1,   157,
     444,   198,    -1,   211,    -1,    85,    -1,    83,    -1,   127,
     194,   336,   195,    -1,   128,   194,   444,   195,    -1,   128,
     194,   347,   195,    -1,   128,   194,   448,   195,    -1,   128,
     194,   447,   195,    -1,   128,   194,   334,   195,    -1,     7,
     346,    -1,     6,   346,    -1,     5,   194,   346,   195,    -1,
       4,   346,    -1,     3,   346,    -1,   444,    -1,   468,     9,
     444,    -1,   403,   158,   212,    -1,   403,   158,   130,    -1,
      -1,   102,   496,    -1,   184,   475,    14,   496,   196,    -1,
     432,   184,   475,    14,   496,   196,    -1,   186,   475,   470,
      14,   496,   196,    -1,   432,   186,   475,   470,    14,   496,
     196,    -1,   213,    -1,   496,   213,    -1,   212,    -1,   496,
     212,    -1,   213,    -1,   213,   179,   485,   180,    -1,   211,
      -1,   211,   179,   485,   180,    -1,   179,   478,   180,    -1,
      -1,   496,    -1,   477,     9,   496,    -1,   477,   414,    -1,
     477,     9,   172,    -1,   478,    -1,   172,    -1,    -1,    -1,
     193,   481,   415,    -1,   482,    -1,   481,     9,   482,    -1,
     496,    14,   496,    -1,   496,   484,    -1,    -1,    32,   496,
      -1,   102,   496,    -1,   103,   496,    -1,   487,   414,    -1,
     484,    -1,   486,   484,    -1,   487,     9,   488,   211,    -1,
     488,   211,    -1,   487,     9,   488,   211,   486,    -1,   488,
     211,   486,    -1,    50,    -1,    51,    -1,    -1,    91,   136,
     496,    -1,    31,    91,   136,   496,    -1,   224,   158,   211,
     136,   496,    -1,   490,     9,   489,    -1,   489,    -1,   490,
     414,    -1,    -1,   183,   194,   491,   195,    -1,   224,    -1,
     211,   158,   494,    -1,   211,   476,    -1,   179,   496,   414,
     180,    -1,   179,   496,     9,   496,   180,    -1,    31,   496,
      -1,    59,   496,    -1,   224,    -1,   138,    -1,   142,    -1,
     492,    -1,   493,   158,   494,    -1,   138,   495,    -1,   163,
      -1,   194,   111,   194,   479,   195,    32,   496,   195,    -1,
     194,   496,     9,   477,   414,   195,    -1,   496,    -1,    -1
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
    2813,  2817,  2818,  2822,  2823,  2824,  2828,  2833,  2838,  2839,
    2843,  2848,  2853,  2854,  2858,  2859,  2864,  2866,  2871,  2882,
    2896,  2908,  2923,  2924,  2925,  2926,  2927,  2928,  2929,  2939,
    2948,  2950,  2952,  2956,  2957,  2958,  2959,  2960,  2976,  2977,
    2979,  2981,  2988,  2989,  2990,  2991,  2992,  2993,  2994,  2995,
    2997,  3002,  3006,  3007,  3011,  3014,  3021,  3025,  3034,  3041,
    3049,  3051,  3052,  3056,  3057,  3058,  3060,  3065,  3066,  3077,
    3078,  3079,  3080,  3091,  3094,  3097,  3098,  3099,  3100,  3111,
    3115,  3116,  3117,  3119,  3120,  3121,  3125,  3127,  3130,  3132,
    3133,  3134,  3135,  3138,  3140,  3141,  3145,  3147,  3150,  3152,
    3153,  3154,  3158,  3160,  3163,  3166,  3168,  3170,  3174,  3175,
    3177,  3178,  3184,  3185,  3187,  3197,  3199,  3201,  3204,  3205,
    3206,  3210,  3211,  3212,  3213,  3214,  3215,  3216,  3217,  3218,
    3219,  3220,  3224,  3225,  3229,  3231,  3239,  3241,  3245,  3249,
    3254,  3258,  3266,  3267,  3271,  3272,  3278,  3279,  3288,  3289,
    3297,  3300,  3304,  3307,  3312,  3317,  3319,  3320,  3321,  3324,
    3326,  3332,  3333,  3337,  3338,  3342,  3343,  3347,  3348,  3351,
    3356,  3357,  3361,  3364,  3366,  3370,  3376,  3377,  3378,  3382,
    3386,  3396,  3404,  3406,  3410,  3412,  3417,  3423,  3426,  3431,
    3436,  3438,  3445,  3448,  3451,  3452,  3455,  3458,  3459,  3464,
    3466,  3470,  3476,  3486,  3487
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
     445,   445,   446,   446,   446,   446,   446,   446,   446,   446,
     446,   447,   448,   448,   449,   449,   450,   450,   450,   451,
     452,   452,   452,   453,   453,   453,   453,   454,   454,   455,
     455,   455,   455,   455,   455,   456,   456,   456,   456,   456,
     457,   457,   457,   457,   457,   457,   458,   458,   459,   459,
     459,   459,   459,   459,   459,   459,   460,   460,   461,   461,
     461,   461,   462,   462,   463,   463,   463,   463,   464,   464,
     464,   464,   465,   465,   465,   465,   465,   465,   466,   466,
     466,   467,   467,   467,   467,   467,   467,   467,   467,   467,
     467,   467,   468,   468,   469,   469,   470,   470,   471,   471,
     471,   471,   472,   472,   473,   473,   474,   474,   475,   475,
     476,   476,   477,   477,   478,   479,   479,   479,   479,   480,
     480,   481,   481,   482,   482,   483,   483,   484,   484,   485,
     486,   486,   487,   487,   487,   487,   488,   488,   488,   489,
     489,   489,   490,   490,   491,   491,   492,   493,   494,   494,
     495,   495,   496,   496,   496,   496,   496,   496,   496,   496,
     496,   496,   496,   497,   497
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
       3,     3,     1,     1,     1,     1,     1,     3,     3,     4,
       4,     3,     1,     1,     7,     9,     7,     6,     8,     1,
       4,     4,     1,     1,     1,     4,     2,     1,     0,     1,
       1,     1,     3,     3,     3,     0,     1,     1,     3,     3,
       2,     3,     6,     0,     1,     4,     2,     0,     5,     3,
       3,     1,     6,     4,     4,     2,     2,     0,     5,     3,
       3,     1,     2,     0,     5,     3,     3,     1,     2,     2,
       1,     2,     1,     4,     3,     3,     6,     3,     1,     1,
       1,     4,     4,     4,     4,     4,     4,     2,     2,     4,
       2,     2,     1,     3,     3,     3,     0,     2,     5,     6,
       6,     7,     1,     2,     1,     2,     1,     4,     1,     4,
       3,     0,     1,     3,     2,     3,     1,     1,     0,     0,
       3,     1,     3,     3,     2,     0,     2,     2,     2,     2,
       1,     2,     4,     2,     5,     3,     1,     1,     0,     3,
       4,     5,     3,     1,     2,     0,     4,     1,     3,     2,
       4,     5,     2,     2,     1,     1,     1,     1,     3,     2,
       1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   431,     0,     0,   846,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   937,
       0,   925,   717,     0,   723,   724,   725,    25,   788,   913,
     914,   155,   156,   726,     0,   136,     0,     0,     0,     0,
      26,     0,     0,     0,     0,   190,     0,     0,     0,     0,
       0,     0,   397,   398,   399,   402,   401,   400,     0,     0,
       0,     0,   219,     0,     0,     0,    33,    34,    35,   730,
     732,   733,   727,   728,     0,     0,     0,   734,   729,     0,
     701,    28,    29,    30,    32,    31,     0,   731,     0,     0,
       0,     0,   735,   403,   536,    27,     0,   154,   126,     0,
     718,     0,     0,     4,   116,   118,   787,     0,   700,     0,
       6,   189,     7,     9,     8,    10,     0,     0,   395,   444,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   442,
     902,   903,   518,   514,   515,   516,   517,   425,   521,     0,
     424,   873,   702,   709,     0,   790,   513,   394,   876,   877,
     888,   443,     0,     0,   446,   445,   874,   875,   872,   909,
     912,   503,   789,    11,   402,   401,   400,     0,     0,    32,
       0,   116,   189,     0,   981,   443,   980,     0,   978,   977,
     520,     0,   432,   439,   437,     0,     0,   485,   486,   487,
     488,   512,   510,   509,   508,   507,   506,   505,   504,    25,
     913,   726,   704,    33,    34,    35,     0,     0,  1001,   895,
     702,     0,   703,   466,     0,   464,     0,   941,     0,   797,
     423,   713,   209,     0,  1001,   422,   712,   707,     0,   722,
     703,   920,   921,   927,   919,   714,     0,     0,   716,   511,
       0,     0,     0,     0,   428,     0,   134,   430,     0,     0,
     140,   142,     0,     0,   144,     0,    76,    75,    70,    69,
      61,    62,    53,    73,    84,    85,     0,    56,     0,    68,
      60,    66,    87,    79,    78,    51,    74,    94,    95,    52,
      90,    49,    91,    50,    92,    48,    96,    83,    88,    93,
      80,    81,    55,    82,    86,    47,    77,    63,    97,    71,
      64,    54,    46,    45,    44,    43,    42,    41,    65,    98,
     100,    58,    39,    40,    67,  1045,  1046,    59,  1050,    38,
      57,    89,     0,     0,   116,    99,   992,  1044,     0,  1047,
       0,     0,   146,     0,     0,     0,   180,     0,     0,     0,
       0,     0,     0,   799,     0,   104,   106,   308,     0,     0,
     307,     0,   223,     0,   220,   313,     0,     0,     0,     0,
       0,   998,   205,   217,   933,   937,   555,   578,   578,     0,
     962,     0,   737,     0,     0,     0,   960,     0,    16,     0,
     120,   197,   211,   218,   606,   548,     0,   986,   528,   530,
     532,   850,   431,   444,     0,     0,   442,   443,   445,     0,
       0,   916,   719,     0,   720,     0,     0,     0,   179,     0,
       0,   122,   299,     0,    24,   188,     0,   216,   201,   215,
     400,   403,   189,   396,   169,   170,   171,   172,   173,   175,
     176,   178,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     925,     0,   168,   918,   918,   947,     0,     0,     0,     0,
       0,     0,     0,     0,   393,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   465,   463,
     851,   852,     0,   918,     0,   864,   299,   299,   918,     0,
     933,     0,   189,     0,     0,   148,     0,   848,   843,   797,
       0,   444,   442,     0,   945,     0,   553,   796,   936,   722,
     444,   442,   443,   122,     0,   299,   421,     0,   866,   715,
       0,   126,   259,     0,   535,     0,   151,     0,     0,   429,
       0,     0,     0,     0,     0,   143,   167,   145,  1045,  1046,
    1042,  1043,     0,  1049,  1035,     0,     0,     0,     0,    72,
      37,    59,    36,   993,   174,   177,   147,   126,     0,   164,
     166,     0,     0,     0,     0,   107,     0,   798,   105,    18,
       0,   101,     0,   309,     0,   149,   222,   221,     0,     0,
     150,   982,     0,     0,   444,   442,   443,   446,   445,     0,
    1028,   229,     0,   934,     0,     0,     0,     0,   797,   797,
       0,     0,   152,     0,     0,   736,   961,   788,     0,     0,
     959,   793,   958,   119,     5,    13,    14,     0,   227,     0,
       0,   541,     0,     0,   797,     0,     0,   710,   705,   542,
       0,     0,     0,     0,   850,   126,     0,   799,   849,  1054,
     420,   434,   499,   882,   901,   131,   125,   127,   128,   129,
     130,   394,     0,   519,   791,   792,   117,   797,     0,  1002,
       0,     0,     0,   799,   300,     0,   524,   191,   225,     0,
     469,   471,   470,   482,     0,     0,   502,   467,   468,   472,
     474,   473,   490,   489,   492,   491,   493,   495,   497,   496,
     494,   484,   483,   476,   477,   475,   478,   479,   481,   498,
     480,   917,     0,     0,   951,     0,   797,   985,     0,   984,
    1001,   879,   207,   199,   213,     0,   986,   203,   189,     0,
     435,   438,   440,   448,   462,   461,   460,   459,   458,   457,
     456,   455,   454,   453,   452,   451,   854,     0,   853,   856,
     878,   860,  1001,   857,     0,     0,     0,     0,     0,     0,
       0,     0,   979,   433,   841,   845,   796,   847,     0,   706,
       0,   940,     0,   939,   225,     0,   706,   924,   923,   909,
     912,     0,     0,   853,   856,   922,   857,   426,   261,   263,
     126,   539,   538,   427,     0,   126,   243,   135,   430,     0,
       0,     0,     0,     0,   255,   255,   141,   797,     0,     0,
       0,  1033,   797,     0,  1008,     0,     0,     0,     0,     0,
     795,     0,    33,    34,    35,   701,     0,     0,   739,   700,
     743,   744,   745,   747,     0,   738,   124,   746,  1001,  1048,
       0,     0,     0,     0,    19,     0,    20,     0,   102,     0,
       0,     0,   113,   799,     0,   111,   106,   103,   108,     0,
     306,   314,   311,     0,     0,   971,   976,   973,   972,   975,
     974,    12,  1026,  1027,     0,   797,     0,     0,     0,   933,
     930,     0,   552,     0,   568,   796,   554,   796,   577,   571,
     574,   970,   969,   968,     0,   964,     0,   965,   967,     0,
       5,     0,     0,     0,   600,   601,   609,   608,     0,   442,
       0,   796,   547,   551,     0,     0,   987,     0,   529,     0,
       0,  1015,   850,   285,  1053,     0,     0,   865,     0,   915,
     796,  1004,  1000,   301,   302,   699,   798,   298,     0,   850,
       0,     0,   227,   526,   193,   501,     0,   585,   586,     0,
     583,   796,   946,     0,     0,   299,   229,     0,   227,     0,
       0,   225,     0,   925,   449,     0,     0,   862,   863,   880,
     881,   910,   911,     0,     0,     0,   829,   804,   805,   806,
     813,     0,    33,    34,    35,     0,     0,   817,   823,   824,
     825,   815,   816,   835,   797,     0,   843,   944,   943,     0,
     227,     0,   867,   721,     0,   265,     0,     0,   132,     0,
       0,     0,     0,     0,     0,     0,   235,   236,   247,     0,
     126,   245,   161,   255,     0,   255,     0,   796,     0,     0,
       0,     0,   796,  1034,  1036,  1007,   797,  1006,     0,   797,
     768,   769,   766,   767,   803,     0,   797,   795,   561,   580,
     580,     0,   550,     0,     0,   953,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1039,   181,     0,   184,   165,     0,
       0,   109,   114,   115,   107,   798,   112,     0,   310,     0,
     983,   153,   999,  1028,  1019,  1023,   228,   230,   320,     0,
       0,   931,     0,     0,   557,     0,   963,     0,    17,     0,
     986,   226,   320,     0,     0,   706,   544,     0,   711,   988,
       0,  1015,   533,     0,     0,  1054,     0,   290,   288,   856,
     868,  1001,   856,   869,  1003,     0,     0,   303,   123,     0,
     850,   224,     0,   850,     0,   500,   950,   949,     0,   299,
       0,     0,     0,     0,     0,     0,   227,   195,   722,   855,
     299,     0,   809,   810,   811,   812,   818,   819,   833,     0,
     797,     0,   829,   565,   582,   582,     0,   808,   837,   796,
     840,   842,   844,     0,   938,     0,   855,     0,     0,     0,
       0,   262,   540,   137,     0,   430,   235,   237,   933,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   249,     0,
    1040,     0,  1029,     0,  1032,   796,     0,     0,     0,   741,
     796,   794,     0,     0,   797,     0,     0,   782,   797,     0,
     785,   784,     0,   797,     0,   748,   786,   783,   957,     0,
     797,   751,   753,   752,     0,     0,   749,   750,   754,   756,
     755,   771,   770,   773,   772,   774,   776,   778,   777,   775,
     764,   763,   758,   759,   757,   760,   761,   762,   765,  1038,
       0,   126,     0,     0,   110,    21,   312,     0,     0,     0,
    1020,  1025,     0,   394,   935,   933,   436,   441,   447,   559,
       0,     0,    15,     0,   394,   612,     0,     0,   614,   607,
     610,     0,   605,     0,   990,     0,  1016,   537,     0,   291,
       0,     0,   286,     0,   305,   304,  1015,     0,   320,     0,
     850,     0,   299,     0,   907,   320,   986,   320,   989,     0,
       0,     0,   450,     0,     0,   821,   796,   828,   814,     0,
       0,   797,     0,     0,   827,   797,     0,   807,     0,     0,
     797,   834,   942,   320,     0,   126,     0,   258,   244,     0,
       0,     0,   234,   157,   248,     0,     0,   251,     0,   256,
     257,   126,   250,  1041,  1030,     0,  1005,     0,  1052,   802,
     801,   740,   569,   796,   560,     0,   572,   796,   579,   575,
       0,   796,   549,   742,     0,   584,   796,   952,   780,     0,
       0,     0,    22,    23,  1022,  1017,  1018,  1021,   231,     0,
       0,     0,   401,   392,     0,     0,     0,   206,   319,   321,
       0,   391,     0,     0,     0,   986,   394,     0,     0,   556,
     966,   316,   212,   603,     0,     0,   543,   531,     0,   294,
     284,     0,   287,   293,   299,   523,  1015,   394,  1015,     0,
     948,     0,   906,   394,     0,   394,   991,   320,   850,   904,
     832,   831,   820,   570,   796,   564,     0,   573,   796,   581,
     576,     0,   822,   796,   836,   394,   126,   264,   133,   138,
     159,   238,     0,   246,   252,   126,   254,  1031,     0,     0,
       0,   563,   781,   546,     0,   956,   955,   779,   126,   185,
    1024,     0,     0,     0,   994,     0,     0,     0,   232,     0,
     986,     0,   357,   353,   359,   701,    32,     0,   347,     0,
     352,   356,   369,     0,   367,   372,     0,   371,     0,   370,
       0,   189,   323,     0,   325,     0,   326,   327,     0,     0,
     932,   558,     0,   604,   602,   613,   611,   295,     0,     0,
     282,   292,     0,     0,  1015,     0,   202,   523,  1015,   908,
     208,   316,   214,   394,     0,     0,     0,   567,   826,   839,
       0,   210,   260,     0,     0,   126,   241,   158,   253,  1051,
     800,     0,     0,     0,     0,     0,     0,   419,     0,   995,
       0,   337,   341,   416,   417,   351,     0,     0,     0,   332,
     665,   664,   661,   663,   662,   682,   684,   683,   653,   623,
     625,   624,   643,   659,   658,   619,   630,   631,   633,   632,
     652,   636,   634,   635,   637,   638,   639,   640,   641,   642,
     644,   645,   646,   647,   648,   649,   651,   650,   620,   621,
     622,   626,   627,   629,   667,   668,   677,   676,   675,   674,
     673,   672,   660,   679,   669,   670,   671,   654,   655,   656,
     657,   680,   681,   685,   687,   686,   688,   689,   666,   691,
     690,   693,   695,   694,   628,   698,   696,   697,   692,   678,
     618,   364,   615,     0,   333,   385,   386,   384,   377,     0,
     378,   334,   411,     0,     0,     0,     0,   415,     0,   189,
     198,   315,     0,     0,     0,   283,   297,   905,     0,     0,
     387,   126,   192,  1015,     0,     0,   204,  1015,   830,     0,
       0,   126,   239,   139,   160,     0,   562,   545,   954,   183,
     335,   336,   414,   233,     0,   797,   797,     0,   360,   348,
       0,     0,     0,   366,   368,     0,     0,   373,   380,   381,
     379,     0,     0,   322,   996,     0,     0,     0,   418,     0,
     317,     0,   296,     0,   598,   799,   126,     0,     0,   194,
     200,     0,   566,   838,     0,     0,   162,   338,   116,     0,
     339,   340,     0,   796,     0,   796,   362,   358,   363,   616,
     617,     0,   349,   382,   383,   375,   376,   374,   412,   409,
    1028,   328,   324,   413,     0,   318,   599,   798,     0,     0,
     388,   126,   196,     0,   242,     0,   187,     0,   394,     0,
     354,   361,   365,     0,     0,   850,   330,     0,   596,   522,
     525,     0,   240,     0,     0,   163,   345,     0,   393,   355,
     410,   997,     0,   799,   405,   850,   597,   527,     0,   186,
       0,     0,   344,  1015,   850,   269,   406,   407,   408,  1054,
     404,     0,     0,     0,   343,  1009,   405,     0,  1015,     0,
     342,     0,     0,  1054,     0,   274,   272,  1009,   126,   799,
    1011,     0,   389,   126,   329,     0,   275,     0,     0,   270,
       0,     0,   798,  1010,     0,  1014,     0,     0,   278,   268,
       0,   271,   277,   331,   182,  1012,  1013,   390,   279,     0,
       0,   266,   276,     0,   267,   281,   280
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   113,   910,   634,   181,  1524,   730,
     352,   353,   354,   355,   863,   864,   865,   115,   116,   117,
     118,   119,   409,   666,   667,   548,   255,  1593,   554,  1502,
    1594,  1836,   852,   347,   577,  1796,  1098,  1291,  1855,   426,
     182,   668,   950,  1164,  1351,   123,   637,   967,   669,   688,
     971,   611,   966,   235,   529,   670,   638,   968,   428,   372,
     392,   126,   952,   913,   888,  1116,  1527,  1220,  1026,  1743,
    1597,   807,  1032,   553,   816,  1034,  1391,   799,  1015,  1018,
    1209,  1862,  1863,   656,   657,   682,   683,   359,   360,   366,
    1562,  1721,  1722,  1303,  1438,  1550,  1715,  1845,  1865,  1754,
    1800,  1801,  1802,  1537,  1538,  1539,  1540,  1756,  1757,  1763,
    1812,  1543,  1544,  1548,  1708,  1709,  1710,  1732,  1904,  1439,
    1440,   183,   128,  1879,  1880,  1713,  1442,  1443,  1444,  1445,
     129,   248,   549,   550,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,  1574,   140,   949,  1163,   141,   653,
     654,   655,   252,   401,   544,   643,   644,  1253,   645,  1254,
     142,   143,   617,   618,  1243,  1244,  1360,  1361,   144,   840,
     998,   145,   841,   999,   146,   842,  1000,   620,  1246,  1363,
     147,   843,   148,   149,  1785,   150,   639,  1564,   640,  1133,
     918,  1322,  1319,  1701,  1702,   151,   152,   153,   238,   154,
     239,   249,   413,   536,   155,  1054,  1248,   847,   156,  1055,
     941,   588,  1056,  1001,  1186,  1002,  1188,  1365,  1189,  1190,
    1004,  1369,  1370,  1005,   775,   519,   195,   196,   671,   659,
     502,  1149,  1150,   761,   762,   937,   158,   241,   159,   160,
     185,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     722,   245,   246,   614,   228,   229,   725,   726,  1259,  1260,
     385,   386,   904,   171,   602,   172,   652,   173,   338,  1723,
    1775,   373,   421,   677,   678,  1048,  1892,  1899,  1900,  1144,
    1300,   884,  1301,   885,   886,   821,   822,   823,   339,   340,
     849,   563,  1526,   935
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1573
static const yytype_int16 yypact[] =
{
   -1573,   197, -1573, -1573,  5827, 14068, 14068,   -20, 14068, 14068,
   14068, 11455, 14068, 14068, -1573, 14068, 14068, 14068, 14068, 14068,
   14068, 14068, 14068, 14068, 14068, 14068, 14068, 16976, 16976, 11656,
   14068, 17743,   -14,    33, -1573, -1573, -1573,   136, -1573,   229,
   -1573, -1573, -1573,   170, 14068, -1573,    33,   156,   187,   200,
   -1573,    33, 11857,  4650, 12058, -1573, 14936, 10651,   213, 14068,
   10386,    98, -1573, -1573, -1573,    54,    61,    76,   228,   239,
     241,   352, -1573,  4650,   370,   386,   249,   361,   520, -1573,
   -1573, -1573, -1573, -1573, 14068,   575,   804, -1573, -1573,  4650,
   -1573, -1573, -1573, -1573,  4650, -1573,  4650, -1573,   436,   414,
    4650,  4650, -1573,   317, -1573, -1573, 12259, -1573, -1573,   453,
     548,   616,   616, -1573,   588,   468,    -1,   442, -1573,    96,
   -1573,   602, -1573, -1573, -1573, -1573,  3748,   639, -1573, -1573,
     473,   492,   495,   506,   513,   518,   525,   529,  5364, -1573,
   -1573, -1573, -1573,    79,   573,   579,   586, -1573,   663,   679,
   -1573,   168,   546, -1573,   593,     6, -1573,  1095,   185, -1573,
   -1573,  2390,   146,   560,    91, -1573,   147,    83,   577,    85,
   -1573, -1573,   705, -1573, -1573, -1573,   623,   605,   634, -1573,
   14068, -1573,   602,   639, 18157,  2514, 18157, 14068, 18157, 18157,
   15285,   609, 17143, 15285, 18157,   761,  4650,   756,   756,   335,
     756,   756,   756,   756,   756,   756,   756,   756,   756, -1573,
   -1573, -1573, -1573, -1573, -1573, -1573,    78, 14068,   658, -1573,
   -1573,   684,   656,   371,   671,   371, 16976, 17191,   662,   858,
   -1573,   623, -1573, 14068,   658, -1573,   712, -1573,   725,   675,
   -1573,   148, -1573, -1573, -1573,   371,   146, 12460, -1573, -1573,
   14068,  9244,   879,   100, 18157, 10249, -1573, 14068, 14068,  4650,
   -1573, -1573, 11439,   697, -1573, 11841, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573, -1573, -1573,  3822, -1573,  3822, -1573,
   -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573, -1573,    99,    90,   634, -1573, -1573,
   -1573, -1573,   695,  2927,    97, -1573, -1573,   737,   889, -1573,
     746, 15472, -1573,   710,   714, 12444, -1573,    50, 14052,  3803,
    3803,  4650,   715,   899,   718, -1573,   237, -1573, 16572,   103,
   -1573,   779, -1573,   781, -1573,   904,   105, 16976, 14068, 14068,
     726,   741, -1573, -1573, 16673, 11656, 14068, 14068, 14068,   106,
      80,   472, -1573, 14269, 16976,   600, -1573,  4650, -1573,   485,
     468, -1573, -1573, -1573, -1573, 17841,   915,   828, -1573, -1573,
   -1573,    77, 14068,   736,   743, 18157,   751,  1061,   752,  6028,
   14068, -1573,   489,   732,   629,   489,    73,   458, -1573,  4650,
    3822,   754,  5080, 14936, -1573, -1573,  2976, -1573, -1573, -1573,
   -1573, -1573,   602, -1573, -1573, -1573, -1573, -1573, -1573, -1573,
   -1573, -1573, 14068, 14068, 14068, 14068, 12661, 14068, 14068, 14068,
   14068, 14068, 14068, 14068, 14068, 14068, 14068, 14068, 14068, 14068,
   14068, 14068, 14068, 14068, 14068, 14068, 14068, 14068, 14068, 14068,
   17939, 14068, -1573, 14068, 14068, 14068,  4867,  4650,  4650,  4650,
    4650,  4650,  3748,   838,   693, 10450, 14068, 14068, 14068, 14068,
   14068, 14068, 14068, 14068, 14068, 14068, 14068, 14068, -1573, -1573,
   -1573, -1573,  2351, 14068, 14068, -1573,  5080,  5080, 14068, 14068,
   16673,   759,   602, 12862, 16074, -1573, 14068, -1573,   763,   953,
     809,   772,   776, 14438,   371, 13063, -1573, 13264, -1573,   675,
     785,   786,  2248, -1573,   138,  5080, -1573,  2852, -1573, -1573,
   16122, -1573, -1573, 10852, -1573, 14068, -1573,   882,  9445,   974,
     788, 14253,   971,   110,    68, -1573, -1573, -1573,   807, -1573,
   -1573, -1573,  3822, -1573,   576,   793,   980, 16433,  4650, -1573,
   -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573,   797, -1573,
   -1573,   796,   799,   801,   806,   424, 10587,  5703, -1573, -1573,
    4650,  4650, 14068,   371,    98, -1573, -1573, -1573, 16433,   922,
   -1573,   371,   130,   132,   805,   811,  2328,   174,   812,   813,
     477,   885,   817,   371,   133,   824, 17247,   821,  1014,  1016,
     825,   826, -1573,  1366,  4650, -1573, -1573,   960,  2874,    38,
   -1573, -1573, -1573,   468, -1573, -1573, -1573,  1000,   900,   856,
     380,   877, 14068,   906,  1034,   849,   888, -1573,   154, -1573,
    3822,  3822,  1035,   879,    77, -1573,   859,  1042, -1573,  3822,
      60, -1573,   459,   186, -1573, -1573, -1573, -1573, -1573, -1573,
   -1573,   917,  3143, -1573, -1573, -1573, -1573,  1044,   874, -1573,
   16976, 14068,   863,  1051, 18157,  1048, -1573, -1573,   931,  3506,
   12043, 18295, 15285, 14761, 14068, 18109, 14935, 12841,  3795,  2636,
    3247, 13440, 13641, 13641, 13641, 13641,  3080,  3080,  3080,  3080,
    3080,   822,   822,   713,   713,   713,   335,   335,   335, -1573,
     756, 18157,   864,   869, 17295,   873,  1063,   353, 14068,   481,
     658,   180, -1573, -1573, -1573,  1075,   828, -1573,   602, 16774,
   -1573, -1573, -1573, 15285, 15285, 15285, 15285, 15285, 15285, 15285,
   15285, 15285, 15285, 15285, 15285, 15285, -1573, 14068,   483, -1573,
     155, -1573,   658,   532,   890,  3324,   895,   898,   893,  3444,
     134,   901, -1573, 18157, 16479, -1573,  4650, -1573,    60,    52,
   16976, 18157, 16976, 17351,   931,    60,   371,   161, -1573,   154,
     938,   902, 14068, -1573,   178, -1573, -1573, -1573,  9043,   521,
   -1573, -1573, 18157, 18157,    33, -1573, -1573, -1573, 14068,   997,
   16311, 16433,  4650,  9646,   905,   909, -1573,  1091,  1017,   973,
     954, -1573,  1102,   918,  4365,  3822, 16433, 16433, 16433, 16433,
   16433,   920,  1045,  1053,  1054,   968,   933, 16433,   475,   978,
   -1573, -1573, -1573, -1573,   940, -1573, 18251, -1573,   245, -1573,
    6229,  5502,   946,  5703, -1573,  5703, -1573,  4650,  4650,  5703,
    5703,  4650, -1573,  1138,   952, -1573,   447, -1573, -1573,  3493,
   -1573, 18251,  1139, 16976,   962, -1573, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573,   982,  1154,  4650,  5502,   972, 16673,
   16875,  1151, -1573, 13465, -1573, 14068, -1573, 14068, -1573, -1573,
   -1573, -1573, -1573, -1573,   967, -1573, 14068, -1573, -1573,  5425,
   -1573,  3822,  5502,   975, -1573, -1573, -1573, -1573,  1156,   976,
   14068, 17841, -1573, -1573,  4867,   977, -1573,  3822, -1573,   983,
    6430,  1146,    72, -1573, -1573,   102,  2351, -1573,  2852, -1573,
    3822, -1573, -1573,   371, 18157, -1573, 11053, -1573, 16433,    67,
     985,  5502,   900, -1573, -1573, 14935, 14068, -1573, -1573, 14068,
   -1573, 14068, -1573,  3586,   986,  5080,   885,  1149,   900,  3822,
    1168,   931,  4650, 17939,   371,  3712,   989, -1573, -1573,   190,
     990, -1573, -1573,  1171,  1162,  1162, 16479, -1573, -1573, -1573,
    1140,   996,  1121,  1123,  1124,   111,  1003, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573,  1189,  1010,   763,   371,   371, 13666,
     900,  2852, -1573, -1573,  3761,   652,    33, 10249, -1573,  6631,
    1011,  6832,  1013, 16311, 16976,  1012,  1076,   371, 18251,  1199,
   -1573, -1573, -1573, -1573,   446, -1573,   323,  3822,  1041,  1087,
    3822,  4650,   576, -1573, -1573, -1573,  1220, -1573,  1033,  1044,
     632,   632,  1161,  1161, 17503,  1030,  1223, 16433, 16433, 16433,
   16433, 15617, 17841, 16170, 15762, 16433, 16433, 16433, 16433, 16194,
   16433, 16433, 16433, 16433, 16433, 16433, 16433, 16433, 16433, 16433,
   16433, 16433, 16433, 16433, 16433, 16433, 16433, 16433, 16433, 16433,
   16433, 16433, 16433,  4650, -1573, -1573,  1153, -1573, -1573,  1047,
    1050, -1573, -1573, -1573,   448, 10587, -1573,  1055, -1573, 16433,
     371, -1573, -1573,    74, -1573,   686,  1237, -1573, -1573,   135,
    1058,   371, 11254, 16976, 18157, 17399, -1573,  2826, -1573,  5626,
     828,  1237, -1573,    12,    35, -1573, 18157,  1113,  1059, -1573,
    1062,  1146, -1573,  3822,   879,  3822,   248,  1240,  1172,   189,
   -1573,   658,   194, -1573, -1573, 16976, 14068, 18157, 18251,  1064,
      67, -1573,  1070,    67,  1066, 14935, 18157, 17455,  1067,  5080,
    1073,  1072,  3822,  1085,  1074,  3822,   900, -1573,   675,   543,
    5080, 14068, -1573, -1573, -1573, -1573, -1573, -1573,  1137,  1082,
    1277,  1196, 16479, 16479, 16479, 16479,  1133, -1573, 17841, 16479,
   -1573, -1573, -1573, 16976, 18157,  1107, -1573,    33,  1267,  1229,
   10249, -1573, -1573, -1573,  1111, 14068,  1076,   371, 16673, 16311,
    1115, 16433,  7033,   673,  1112, 14068,    75,   372, -1573,  1131,
   -1573,  3822, -1573,  1178, -1573,  5197,  1285,  1129, 16433, -1573,
   16433, -1573,  1130,  1118,  1326, 17558,  1141, 18251,  1334,  1145,
   -1573, -1573,  1211,  1339,  1155, -1573, -1573, -1573, 17606,  1158,
    1342, 15469, 18339, 11033, 16433, 18205,  4814, 13041, 13241, 12637,
   13842, 14434, 14434, 14434, 14434,  3613,  3613,  3613,  3613,  3613,
    1080,  1080,   632,   632,   632,  1161,  1161,  1161,  1161, -1573,
    1157, -1573,  1167,  1169, -1573, -1573, 18251,  4650,  3822,  3822,
   -1573,   686,  5502,  1173, -1573, 16673, -1573, -1573, 15285,   371,
   13867,  1166, -1573,  1170,  1425, -1573,   347, 14068, -1573, -1573,
   -1573, 14068, -1573, 14068, -1573,   879, -1573, -1573,   109,  1340,
    1270, 14068, -1573,  1175,   371, 18157,  1146,  1177, -1573,  1180,
      67, 14068,  5080,  1182, -1573, -1573,   828, -1573, -1573,  1183,
    1176,  1186, -1573,  1188, 16479, -1573, 16479, -1573, -1573,  1195,
    1190,  1384,  1258,  1197, -1573,  1387,  1198, -1573,  1261,  1205,
    1393, -1573,   371, -1573,  1371, -1573,  1209, -1573, -1573,  1218,
    1221,   137, -1573, -1573, 18251,  1222,  1226, -1573,  5277, -1573,
   -1573, -1573, -1573, -1573, -1573,  3822, -1573,  3822, -1573, 18251,
   17661, -1573, -1573, 16433, -1573, 16433, -1573, 16433, -1573, -1573,
   16433, 17841, -1573, -1573, 16433, -1573, 16433, -1573, 11636, 16433,
    1227,  7234, -1573, -1573,   686, -1573, -1573, -1573, -1573,   669,
    4221,  5502,  1305, -1573,  1219,  1252,  2866, -1573, -1573, -1573,
     838,  2637,   108,   113,  1231,   828,   693,   139, 16976, 18157,
   -1573, -1573, -1573,  1262,  3991,  4554, 18157, -1573,   389,  1414,
    1347, 14068, -1573, 18157,  5080,  1314,  1146,  1462,  1146,  1238,
   18157,  1239, -1573,  1904,  1235,  2014, -1573, -1573,    67, -1573,
   -1573,  1299, -1573, -1573, 16479, -1573, 16479, -1573, 16479, -1573,
   -1573, 16479, -1573, 17841, -1573,  2029, -1573,  9043, -1573, -1573,
   -1573, -1573,  9847, -1573, -1573, -1573,  9043, -1573,  1241, 16433,
   17709, 18251, 18251, 18251,  1301, 18251, 17764, 11636, -1573, -1573,
     686,  5502,  5502,  4650, -1573,  1426, 15907,    94, -1573,  4221,
     828,  4077, -1573,  1260, -1573,   118,  1244,   119, -1573, 15284,
   -1573, -1573, -1573,   124, -1573, -1573,  2625, -1573,  1246, -1573,
    1360,   602, -1573, 15110, -1573, 15110, -1573, -1573,  1431,   838,
   -1573,   371, 14588, -1573, -1573, -1573, -1573,  1434,  1370, 14068,
   -1573, 18157,  1259,  1263,  1146,   622, -1573,  1314,  1146, -1573,
   -1573, -1573, -1573,  2116,  1265, 16479,  1319, -1573, -1573, -1573,
    1322, -1573,  9043, 10048,  9847, -1573, -1573, -1573,  9043, -1573,
   18251, 16433, 16433, 16433,  7435,  1266,  1268, -1573, 16433, -1573,
    5502, -1573, -1573, -1573, -1573, -1573,  3822,  1521,  1219, -1573,
   -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573,
   -1573,   612, -1573,  1252, -1573, -1573, -1573, -1573, -1573,    93,
     345, -1573,  1449,   125, 15472,  1360,  1451, -1573,  3822,   602,
   -1573, -1573,  1274,  1459, 14068, -1573, 18157, -1573,   122,  1278,
   -1573, -1573, -1573,  1146,   622, 14762, -1573,  1146, -1573, 16479,
   16479, -1573, -1573, -1573, -1573,  7636, 18251, 18251, 18251, -1573,
   -1573, -1573, 18251, -1573,   757,  1467,  1468,  1282, -1573, -1573,
   16433, 15284, 15284,  1422, -1573,  2625,  2625,   620, -1573, -1573,
   -1573, 16433,  1399, -1573,  1306,  1292,   126, 16433, -1573,  4650,
   -1573, 16433, 18157,  1404, -1573,  1479, -1573,  7837,  1293, -1573,
   -1573,   622, -1573, -1573,  8038,  1295,  1374, -1573,  1392,  1337,
   -1573, -1573,  1394,  3822,  1317,  1521, -1573, -1573, 18251, -1573,
   -1573,  1328, -1573,  1464, -1573, -1573, -1573, -1573, 18251,  1488,
     477, -1573, -1573, 18251,  1315, 18251, -1573,   152,  1313,  8239,
   -1573, -1573, -1573,  1316, -1573,  1320,  1341,  4650,   693,  1330,
   -1573, -1573, -1573, 16433,  1335,    82, -1573,  1435, -1573, -1573,
   -1573,  8440, -1573,  5502,   946, -1573,  1346,  4650,   482, -1573,
   18251, -1573,  1325,  1513,   672,    82, -1573, -1573,  1440, -1573,
    5502,  1329, -1573,  1146,    89, -1573, -1573, -1573, -1573,  3822,
   -1573,  1332,  1338,   127, -1573,  1336,   672,   128,  1146,  1343,
   -1573,  3822,   633,  3822,   411,  1524,  1456,  1336, -1573,  1532,
   -1573,   460, -1573, -1573, -1573,   164,  1529,  1469, 14068, -1573,
     633,  8641,  3822, -1573,  3822, -1573,  8842,   462,  1537,  1470,
   14068, -1573, 18157, -1573, -1573, -1573, -1573, -1573,  1540,  1472,
   14068, -1573, 18157, 14068, -1573, 18157, 18157
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1573, -1573, -1573,  -543, -1573, -1573, -1573,   216,     0,   -34,
     498, -1573,  -264,  -505, -1573, -1573,   451,    -2,  1549, -1573,
    2174, -1573,  -497, -1573,    28, -1573, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573, -1573, -1573,  -297, -1573, -1573,  -156,
     294,    27, -1573, -1573, -1573, -1573, -1573, -1573,    30, -1573,
   -1573, -1573, -1573, -1573, -1573,    31, -1573, -1573,  1079,  1081,
    1083,  -106,  -647,  -858,   599,   654,  -303,   357,  -925, -1573,
     -26, -1573, -1573, -1573, -1573,  -722,   181, -1573, -1573, -1573,
   -1573,  -290, -1573,  -592, -1573,  -440, -1573, -1573,   984, -1573,
       7, -1573, -1573, -1012, -1573, -1573, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573,   -24, -1573,    64, -1573, -1573, -1573,
   -1573, -1573,  -111, -1573,   160,  -910, -1573, -1572,  -313, -1573,
    -155,   257,  -118,  -288, -1573,  -113, -1573, -1573, -1573,   171,
     -10,     4,    48,  -718,   -70, -1573, -1573,    32, -1573,   -12,
   -1573, -1573,    -5,   -29,    29, -1573, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573,  -605,  -852, -1573, -1573, -1573, -1573,
   -1573,  1094, -1573, -1573, -1573, -1573, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573, -1573, -1573, -1573,  1230,   547,   415,
   -1573, -1573, -1573, -1573, -1573,   487, -1573, -1573, -1573, -1573,
   -1573, -1573, -1573, -1573,  -900, -1573,  3046,    39, -1573,  1444,
    -395, -1573, -1573,  -465,  3689,  3608, -1573, -1573, -1573,   559,
     802,  -624, -1573, -1573,   640,   435,  -620, -1573,   437, -1573,
   -1573, -1573, -1573, -1573,   625, -1573, -1573, -1573,    26,  -886,
    -167,  -419,  -414, -1573,   696,  -101, -1573, -1573,    42,    45,
     594, -1573, -1573,    44,    -9, -1573,  -354,    43,  -355,    58,
    -305, -1573, -1573,  -453,  1264, -1573, -1573, -1573, -1573, -1573,
     711,   419, -1573, -1573, -1573,  -352,  -698, -1573,  1210, -1011,
   -1573,   -73,  -174,    56,   810, -1573,  -262, -1573,  -276, -1078,
   -1233,  -182,   217, -1573,   527,   601, -1573, -1573, -1573, -1573,
     549, -1573,   -27, -1103
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1038
static const yytype_int16 yytable[] =
{
     184,   186,   483,   188,   189,   190,   192,   193,   194,   433,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   336,   393,   227,   230,   511,   396,   397,   341,
     157,   122,   120,   933,   124,   125,   404,   251,   970,   254,
     648,   647,  1328,   649,   798,   344,  1145,   262,   928,   265,
     256,   482,   345,   429,   348,   260,   335,   770,   356,   947,
     533,   505,   929,  1325,   784,   433,   766,   767,  1427,  1137,
     237,   224,   224,   242,   244,   719,   243,   406,   537,   254,
     850,   862,   867,   759,   389,   582,   584,   390,   760,   343,
    1022,   909,   253,  1036,  1162,   791,   403,   408,  1216,   -37,
     814,   405,  1765,  1610,   -37,   423,   -36,  1389,   -72,   545,
    1173,   -36,   594,   -72,   599,   545,    14,  1553,   794,   812,
    1314,    14,  1555,   795,   882,   883,    14,  -350,  1618,  1766,
     503,    14,   379,  1703,  1772,  1772,  1610,  1010,    14,   873,
    1146,   545,   890,   890,   890,   538,   890,  1458,   890,  -587,
     623,   406,  1205,  -885,  1003,   508,   380, -1001,   930,   365,
    1783,  -592,  1789,   630,  -703,   578,  1894,   411,   520,   723,
     403,   408,   500,   501,   187,   405,  1315,   361,   420,   789,
     247,   357,   514,   363,   362,  1147,   500,   501,   522,  1316,
    1847,   364,  1459, -1001,   500,   501, -1001,     3,   764,  1320,
    -896,  1196,  1917,   768,   531,  1784,  -595,   521,   408,  1317,
    1252,  1895,   405,   624,   500,   501,   503,  -884,  -926,  1832,
     114,   210,    40,   530,   508,  -887,   579,   250,   405,   383,
     384,  -929,  1321,   382,  -704,  1848,   908,  -534,  -883,  1106,
    -593,  -710,   540,  -705,  -592,   540,  -898,  1918,  -928,   560,
    -891,   561,   254,   551,  -796,  -886,  -890,   504,  1465,  -870,
    -889,   127,  -289,  -798,  -871,   815,  1197,  -798,  -289,   263,
     224,  1390,   334,  -895,  1148,   674,   689,  -273,   562,   542,
    -885,  1460,   509,   547,  -798,   507,   -37,  1427,  1767,   371,
    1611,  1612,   424,   -36,  1382,   -72,   546,   358,   121,   595,
    1896,   600,   622,  1019,  1554,   813,   566,   573,  1021,  1556,
     391,  1223,   371,  1227,  -350,  1619,   371,   371,  1350,   376,
    1704,  1773,  1822,  1890,  1176,   874,  1467,   875,   891,   983,
    1304,  1329,  1501,  1473,  1560,  1475,  1919,   109,  -711,   590,
     605,   335,   371,   504,  -884,  -926,  1368,   356,   356,   585,
     257,   509,  -887,   687,  1102,  1103,   771,  1159,  -929,   604,
     608,  1495,  -894,   254,   405,  -883,   433,  1129,   507,   879,
     227,   616,   254,   254,   965,  -928,  1768,  -891,   628,  -893,
    -897,   258,  -886,  -890,  -900,   633,  -870,  -889,  1575,   336,
    1577,  -871,   470,   679,   259,  1769,   341,   192,  1770,   591,
     398,  1453,   224,  1093,   471,   672,   393,   735,   736,   429,
     346,   224,   518,   607,   484,   740,   603,   684,   224,  1525,
    1330,   432,   367,   335,   420,   619,   619,   658,   224,  1225,
    1226,   377,  1313,   368,   915,   369,  1119,   690,   691,   692,
     693,   695,   696,   697,   698,   699,   700,   701,   702,   703,
     704,   705,   706,   707,   708,   709,   710,   711,   712,   713,
     714,   715,   716,   717,   718,  1583,   720,   114,   721,   721,
     724,   114,  1567,   742,  1914,   552,   729,   512,  1225,  1226,
     743,   744,   745,   746,   747,   748,   749,   750,   751,   752,
     753,   754,   755,   936,  1906,   938,  1729,  1379,   721,   765,
    1734,   684,   684,   721,   769,   500,   501,   399,   743,   237,
     394,   773,   242,   244,   400,   243,   483,   741,  1613,  1152,
     781,  1228,   783,   335,  1153,  1170,   857,   882,   883,  -594,
     684,   801,  -100,  1222,   731,   817,   210,    40,   802,  1327,
     803,   380,  1716,   916,  1717,  1928,   370,  -100,   630,   590,
     857,  1224,  1225,  1226,   224,   380,   964,   572,   917,  1514,
     763,  1568,  1298,  1299,   374,   482,   648,   647,  1337,   649,
    1392,  1339,   380,  1362,  1364,  1364,   806,   788,  1178,  1371,
     375,   731,   972,  1907,   866,   866,   858,   869,   976,  1099,
     378,  1100,   790,   500,   501,   796,  1016,  1017,   161,   394,
     862,    62,    63,    64,   174,   175,   430,   818,   395,   419,
     419,   936,   938,   919,   383,   384,   954,  -706,  1011,   938,
     418,   223,   225,   925,   926,   114,  1760,   625,   383,   384,
     419,   380,   934, -1001,  1929,   676,   422,   405,   412,   334,
     425,  1590,   371,  -588,  1761,   383,   384,   419,  1474,  -589,
     410,  1815,   109,  -898,   420,  1788,  -590,   209,   380,  1791,
     -99,   675,  -858,  1762,   533,   381,   127,   819,  1915,   434,
    1816,   431, -1001,  1817,  1094,   -99,   944,  -858,  1872,    50,
     658,   635,   636,   380,  1012,  1089,  1090,  1091,   435,   955,
     630,   436,   572,   371,   733,   371,   371,   371,   371,   380,
     407,  1092,   437,   121,   383,   384,   415,   648,   647,   438,
     649,  -861,   380,  1352,   439,   213,   214,   215,   758,   630,
    1457,   440,  -859,   963,   224,   441,  -861,  1207,  1208,  1343,
     382,   383,   384,   473,  1480,   178,  1481,  -859,    89,   572,
    1353,    91,    92,   475,    93,   179,    95,  1558,  1469,   474,
      55,   476,   975,   793,   506,   631,   383,   384,    62,    63,
      64,   174,   175,   430,   114,  1381,   467,   468,   469,   105,
     470,  -892,   383,   384,   407,  -591,  1887,   738,  1386,  1225,
    1226,  -704,   471,   224,   848,   383,   384,  1014,  1298,  1299,
    1905,  1876,  1877,  1878,  1421,  1885,   387,   679,   679,   510,
     626,  1521,  1522,   254,   632,   515,   868,   676,  1020,   517,
    1897,   407,    62,    63,    64,   174,   175,   430,  1730,  1731,
     524,   414,   416,   417,   224,   471,   224,   532,   431,  1902,
    1903,   626,  1614,   632,   626,   632,   632,   420,   209,   903,
     905,  1031,   523,   648,   647,   161,   649,   581,   583,   161,
    -896,   866,  1447,   866,   224,  1813,  1814,   866,   866,  1104,
      50,  1809,  1810,   526,  1586,   507,  1587,   527,  1588,   535,
    -702,  1589,   464,   465,   466,   467,   468,   469,  1497,   470,
    1046,  1049,   431,   534,  1130,   209,  1584,   543,  1124,   564,
    1125,   471,   803,   556,  1506, -1037,   213,   214,   215,  1177,
    1140,  1127,  1471,   567,   568,   371,   574,    50,   587,   596,
     575,   597,   586,  1154,   589,  1136,   178,   224,   598,    89,
     610,   609,    91,    92,   729,    93,   179,    95,   484,   650,
     651,   660,   673,   224,   224,   157,   122,   120,   661,   124,
     125,  1157,  1174,   213,   214,   215,   662,   664,  -121,    55,
     105,  1165,   593,   686,  1166,  1797,  1167,   774,   658,  1864,
     684,   601,   776,   606,   625,  1738,   387,   778,   613,    91,
      92,   779,    93,   179,    95,   658,   804,  1333,   629,  1864,
     785,   786,  1138,   545,   808,   811,   562,   824,  1886,   825,
     997,   851,  1006,   853,   763,   854,   796,   105,   855,  1592,
     876,   388,   856,   161,  1204,   872,   877,   880,  1598,   881,
    1229,   889,   237,  1232,   114,   242,   244,   887,   243,   892,
    1210,  1604,   894,   895,  1572,   897,   899,   900,  1029,   114,
     906,   528,   911,   912,   914,  -726,    62,    63,    64,    65,
      66,   430,   920,   921,   923,  1211,   924,    72,   477,   927,
     931,   932,  1306,   940,   942,   127,   648,   647,   945,   649,
     946,  1251,   948,   951,  1257,   957,   114,   224,   224,   796,
     958,   960,   961,  1101,   676,   513,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   479,   969,
     979,   977,   121,   980,   981,   953,  -708,  1013,  1745,  1023,
    1037,  1033,  1115,   866,   613,  1035,   431,   127,  1039,  1040,
    1307,  1042,  1041,  1044,  1057,  1058,  1326,  1308,   934,  1792,
    1793,   219,   219,  1059,  1060,   114,  1061,  1062,   498,   499,
    1086,  1087,  1088,  1089,  1090,  1091,  1064,  1065,   648,   647,
     572,   649,   161,  1097,   121,  1346,   114,  1105,  1349,  1092,
    1107,  1335,   758,  1109,   793,   157,   122,   120,  1111,   124,
     125,  1828,  1112,  1113,   684,  1122,   127,   224,  1126,  1118,
    1134,  1135,  1132,  1139,  1141,   684,  1308,  1429,  1143,  1160,
    1169,  1172,  1175,  1180,  -899,  1181,   658,   127,   371,   658,
    1192,  1193,  1191,  1194,  1195,   500,   501,  1198,  1199,   224,
    1185,  1185,   997,   121,  1394,  1201,  1218,  1213,  1154,  1215,
     254,  1374,  1219,  1221,    62,    63,    64,    65,    66,   430,
    1388,  1230,    14,  1231,   121,    72,   477,   793,  1236,  1235,
    1092,  1239,  1240,   114,  1787,   114,  1290,   114,  1377,  1875,
    1182,  1183,  1184,   209,  1794,  1292,  1302,   224,  1293,  1323,
    1531,  1295,  1305,   965,  1331,  1332,   663,  1233,  1324,  1336,
    1340,  1342,   224,   224,   478,    50,   479,  1338,  1344,  1345,
    1348,  1425,  1426,  1354,   943,  1913,   127,   572,   127,   480,
     572,   481,  1347,  1355,   431,  1430,  1356,   990,  1367,  1829,
    1431,  1559,    62,    63,    64,   174,  1432,   430,  1433,  1375,
     209,   213,   214,   215,  1373,  1449,  1376,  1378,  1387,   848,
    1383,  1393,  1454,   121,  1395,   121,  1455,  1397,  1456,  1402,
     219,   777,    50,   433,  1398,  1401,  1463,    91,    92,  1446,
      93,   179,    95,   974,  1851,  1403,  1470,   684,  1434,  1435,
    1446,  1436,  1406,  1407,  1532,   114,  1409,  1410,  1411,   224,
    1413,  1416,  1420,  1462,  1461,   105,  1415,  1533,   213,   214,
     215,  1534,   431,  1422,  1450,  1423,   658,  1451,  1507,  1464,
    1508,  1437,  1466,  1477,  1007,  1468,  1008,  1472,   178,  1476,
    1478,    89,  1535,  1479,    91,    92,   127,    93,  1536,    95,
    1482,  1483,   161,  1484,  1486,  1714,  1488,  1491,  1487,  1490,
    1492,  1911,  1493,  1496,  1027,  1498,  1916,   161,   997,   997,
     997,   997,   105,  1499,  1552,   997,  1500,  1529,  1503,  1542,
     896,   898,  1504,   121,  1518,  1563,   114,  1557,  1569,  1429,
    1570,  1573,  1581,  1578,  1579,  1585,  1599,  1602,   114,  1616,
    1608,  1617,  1711,  1712,   161,  1718,   922,   209,  1724,   901,
    1607,   902,   219,  1725,  1727,  1739,  1571,  1728,  1740,   684,
    1737,   219,  1750,  1771,  1751,  1777,  1429,  1110,   219,    50,
    1780,   221,   221,  1781,    14,  1786,  1803,  1805,   219,   127,
    1807,  1811,  1819,   613,  1121,  1820,  1821,  1826,  1827,  1835,
    1831,  1834,   224,  1446,  -346,  1837,  1838,  1840,  1842,  1446,
    1766,  1446,  1843,   161,   658,   213,   214,   215,  1849,  1846,
    1859,    14,  1852,  1424,  1853,  1861,   121,  1854,  1866,  1870,
    1873,  1446,  1874,  1882,   161,  1884,  1609,  1888,   962,  1891,
    1596,    91,    92,  1889,    93,   179,    95,  1430,  1908,  1909,
    1898,  1912,  1431,  1920,    62,    63,    64,   174,  1432,   430,
    1433,  1930,  1921,  1931,  1933,  1934,  1294,  1869,   732,   105,
    1441,   737,   734,  1779,  1726,  1171,  1131,  1883,  1744,  1505,
     997,  1441,   997,  1380,  1430,  1881,   218,   218,   870,  1431,
     234,    62,    63,    64,   174,  1432,   430,  1433,  1735,  1755,
    1434,  1435,  1764,  1436,  1759,  1615,  1549,  1923,  1893,    34,
      35,    36,  1776,  1530,   219,   234,  1733,  1249,   621,  1446,
    1366,   161,   211,   161,   431,   161,  1242,  1027,  1217,  1038,
    1318,  1742,  1596,  1452,  1043,  1187,  1358,  1434,  1435,  1359,
    1436,  1202,  1151,   685,  1047,  1910,  1925,   114,  1844,   615,
    1297,  1520,  1289,  1234,     0,     0,   334,     0,     0,     0,
       0,   431,  1547,     0,     0,     0,     0,     0,     0,     0,
    1576,     0,     0,     0,    79,    80,    81,    82,    83,     0,
     221,     0,     0,     0,     0,   216,     0,     0,   127,     0,
    1774,    87,    88,  1857,     0,     0,     0,  1114,     0,     0,
       0,  1778,     0,     0,     0,    97,     0,     0,     0,     0,
     997,     0,   997,   484,   997,     0,  1824,   997,     0,   102,
       0,     0,     0,   114,   335,   121,     0,  1309,   114,  1782,
       0,     0,   114,   161,  1441,     0,     0,     0,     0,     0,
    1441,     0,  1441,     0,  1551,     0,     0,     0,     0,   371,
     433,     0,   572,     0,     0,   334,     0,     0,     0,  1334,
       0,     0,  1441,     0,   127,  1700,     0,     0,     0,     0,
       0,     0,  1707,   127,     0,     0,     0,     0,     0,   334,
       0,   334,     0,     0,   219,   218,  1839,     0,   334,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   121,     0,     0,     0,     0,     0,  1372,     0,     0,
     121,   997,   221,     0,   161,     0,  1200,     0,   114,   114,
     114,   221,   613,  1027,   114,     0,   161,     0,   221,     0,
     114,     0,     0,     0,     0,   234,     0,   234,   221,     0,
       0,     0,     0,   219,     0,     0,     0,     0,     0,   646,
    1441,     0,     0,     0,     0,     0,     0,     0,     0,   127,
       0,  1237,   934,  1719,     0,   127,     0,     0,  1241,     0,
       0,   127,     0,     0,  1901,     0,   934,     0,     0,     0,
       0,   658,     0,     0,   219,     0,   219,     0,     0,     0,
       0,     0,   234,     0,     0,  1901,   121,  1926,     0,     0,
       0,   658,   121,     0,     0,     0,     0,     0,   121,   613,
     658,     0,     0,  1922,   219,     0,     0,   218,  1429,     0,
       0,     0,     0,     0,     0,  1932,   218,     0,     0,     0,
       0,     0,     0,   218,     0,  1935,     0,     0,  1936,     0,
     572,     0,     0,   218,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   234,     0,     0,     0,     0,     0,
       0,   334,     0,    14,   221,   997,   997,     0,     0,     0,
       0,   114,     0,     0,     0,     0,     0,   219,     0,   234,
    1798,     0,   234,     0,     0,     0,     0,  1700,  1700,     0,
       0,  1707,  1707,   219,   219,     0,     0,     0,     0,     0,
       0,     0,  1357,     0,     0,   371,     0,     0,     0,     0,
       0,     0,   127,   114,     0,     0,     0,     0,     0,     0,
     114,     0,     0,     0,     0,   161,  1430,     0,  1429,   234,
       0,  1431,     0,    62,    63,    64,   174,  1432,   430,  1433,
       0,     0,     0,  1429,     0,     0,     0,     0,     0,   121,
       0,     0,  1561,     0,   127,   114,  1404,     0,     0,     0,
    1408,   127,     0,  1856,     0,  1412,     0,     0,     0,   218,
       0,     0,  1417,    14,     0,     0,     0,   114,     0,  1434,
    1435,     0,  1436,  1871,     0,     0,     0,     0,    14,     0,
       0,   121,     0,     0,     0,     0,   127,     0,   121,     0,
       0,   161,     0,   431,     0,  1858,   161,     0,     0,     0,
     161,     0,  1580,     0,     0,     0,     0,     0,   127,     0,
       0,   234,     0,   234,     0,     0,   838,   219,   219,     0,
    1429,     0,     0,   121,   221,     0,  1430,   114,     0,     0,
       0,  1431,   114,    62,    63,    64,   174,  1432,   430,  1433,
       0,  1430,     0,     0,     0,   121,  1431,   838,    62,    63,
      64,   174,  1432,   430,  1433,     0,     0,     0,     0,     0,
       0,     0,     0,  1485,     0,    14,     0,  1489,   127,     0,
       0,     0,  1494,   127,     0,     0,     0,     0,     0,  1434,
    1435,     0,  1436,   221,     0,     0,   161,   161,   161,     0,
       0,     0,   161,     0,  1434,  1435,     0,  1436,   161,   234,
     234,     0,     0,   431,     0,   121,     0,     0,   234,     0,
     121,     0,  1582,     0,     0,     0,     0,   219,   431,     0,
       0,     0,     0,     0,   221,     0,   221,  1591,  1430,   218,
     337,     0,     0,  1431,     0,    62,    63,    64,   174,  1432,
     430,  1433,     0,     0,     0,     0,     0,     0,     0,   219,
       0,     0,     0,     0,   221,     0,     0,     0,     0,     0,
       0,     0,   513,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,     0,     0,     0,     0,     0,
       0,  1434,  1435,     0,  1436,     0,     0,     0,   218,     0,
       0,     0,     0,     0,     0,     0,     0,   219,     0,     0,
       0,     0,     0,     0,     0,   431,     0,     0,     0,     0,
       0,     0,   219,   219,  1736,   498,   499,   221,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   218,
       0,   218,     0,   221,   221,     0,     0,     0,     0,   161,
       0,     0,   513,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,     0,     0,     0,     0,   218,
     838,     0,     0,     0,     0,   646,     0,     0,     0,     0,
       0,     0,     0,   234,   234,   838,   838,   838,   838,   838,
       0,   161,   500,   501,     0,     0,   838,     0,   161,     0,
       0,     0,     0,     0,     0,   498,   499,     0,     0,   219,
     234,     0,     0,     0,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,     0,     0,     0,
       0,     0,   218,   161,     0,     0,     0,     0,     0,     0,
       0,     0,   209,     0,   210,    40,   234,     0,   218,   218,
       0,     0,     0,   787,     0,   161,     0,     0,     0,     0,
     337,     0,   337,     0,    50,     0,     0,   498,   499,     0,
     234,   234,   500,   501,     0,     0,     0,   221,   221,     0,
     234,     0,     0,     0,     0,     0,   234,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   234,
     213,   214,   215,     0,     0,     0,     0,   838,     0,     0,
     234,     0,     0,     0,     0,   161,   646,   337,     0,     0,
     161,     0,     0,     0,   756,     0,    91,    92,   234,    93,
     179,    95,   234,   878,   500,   501,     0,     0,   513,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,     0,   219,     0,   105,     0,     0,     0,   757,     0,
     109,     0,     0,     0,     0,     0,     0,  1804,  1806,     0,
       0,     0,     0,     0,     0,     0,     0,   221,     0,     0,
       0,     0,   218,   218,     0,     0,     0,     0,     0,     0,
       0,   498,   499,     0,     0,     0,   234,     0,     0,   234,
       0,   234,     0,     0,   337,     0,     0,   337,     0,   221,
       0,     0,     0,     0,     0,     0,   838,   838,   838,   838,
       0,   234,     0,     0,   838,   838,   838,   838,   838,   838,
     838,   838,   838,   838,   838,   838,   838,   838,   838,   838,
     838,   838,   838,   838,   838,   838,   838,   838,   838,   838,
     838,   838,   646,     0,     0,     0,     0,   221,   500,   501,
       0,     0,     0,     0,     0,     0,     0,     0,   838,     0,
       0,     0,   221,   221,     0,     0,     0,     0,   276,     0,
       0,     0,   218,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   234,   470,   234,     0,   278,     0,     0,     0,
       0,     0,     0,     0,   218,   471,   209,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   209,     0,
       0,   234,     0,     0,   234,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,   337,     0,   820,     0,
      50,   839,     0,     0,     0,     0,     0,   234,  -393,   221,
       0,     0,   218,     0,     0,     0,    62,    63,    64,   174,
     175,   430,     0,     0,   213,   214,   215,   218,   218,     0,
     838,     0,   839,     0,     0,   558,   213,   214,   215,   559,
     234,     0,     0,     0,   234,     0,     0,   838,  1705,   838,
      91,    92,  1706,    93,   179,    95,   178,     0,     0,    89,
     328,     0,    91,    92,     0,    93,   179,    95,     0,     0,
       0,     0,     0,   838,     0,     0,     0,     0,   105,  1546,
     332,     0,     0,     0,   337,   337,   431,     0,     0,     0,
     105,   333,     0,   337,     0,     0,   442,   443,   444,     0,
       0,     0,     0,     0,     0,     0,     0,   234,   234,     0,
       0,   234,     0,     0,   218,   646,   445,   446,     0,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,     0,   470,   442,   443,   444,     0,     0,     0,
       0,     0,   221,     0,     0,   471,     0,     0,     0,     0,
       0,     0,     0,     0,   445,   446,     0,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
       0,   470,     0,   209,     0,   210,    40,   646,     0,     0,
       0,     0,     0,   471,   234,     0,   234,   209,     0,     0,
       0,     0,   838,     0,   838,    50,   838,     0,   276,   838,
     234,     0,     0,   838,     0,   838,     0,     0,   838,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   234,
     234,     0,     0,   234,     0,   839,   278,     0,     0,     0,
     234,   213,   214,   215,  1545,     0,     0,   218,   337,   337,
     839,   839,   839,   839,   839,   213,   214,   215,   209,     0,
       0,   839,     0,     0,     0,   756,     0,    91,    92,     0,
      93,   179,    95,     0,     0,     0,     0,  1311,     0,     0,
      50,    91,    92,     0,    93,   179,    95,     0,   565,     0,
       0,     0,   234,     0,     0,   105,     0,     0,     0,   792,
       0,   109,     0,     0,     0,     0,     0,   209,   838,   105,
    1546,     0,     0,     0,     0,   558,   213,   214,   215,   559,
     234,   234,   907,   220,   220,     0,     0,   236,   234,    50,
     234,     0,     0,     0,     0,   337,   178,     0,     0,    89,
     328,     0,    91,    92,     0,    93,   179,    95,     0,     0,
       0,   337,   234,     0,   234,     0,     0,     0,     0,     0,
     332,   234,     0,     0,   337,   213,   214,   215,     0,     0,
     105,   333,   839, -1038, -1038, -1038, -1038, -1038,   462,   463,
     464,   465,   466,   467,   468,   469,     0,   470,     0,     0,
       0,    91,    92,   337,    93,   179,    95,     0,     0,   471,
     838,   838,   838,   442,   443,   444,     0,   838,     0,   234,
       0,     0,     0,     0,     0,   234,     0,   234,     0,   105,
     686,     0,     0,   445,   446,     0,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,     0,
     470,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   337,   471,     0,   337,     0,   820,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   839,   839,   839,   839,     0,     0,     0,     0,   839,
     839,   839,   839,   839,   839,   839,   839,   839,   839,   839,
     839,   839,   839,   839,   839,   839,   839,   839,   839,   839,
     839,   839,   839,   839,   839,   839,   839,   234,     0,     0,
       0,     0,   220,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   839,   234,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   234,   470,     0,     0,     0,     0,   838,
       0,     0,     0,     0,     0,     0,   471,   337,     0,   337,
     838,     0,     0,     0,     0,     0,   838,     0,     0,     0,
     838,     0,     0,     0,   442,   443,   444,     0,     0,     0,
       0,   939,     0,     0,     0,     0,   337,     0,     0,   337,
       0,     0,   234,     0,   445,   446,     0,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
       0,   470,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   838,   471,     0,   839,     0,     0,     0,     0,
       0,     0,   234,     0,   220,   337,     0,     0,     0,   337,
       0,     0,   839,   220,   839,     0,     0,     0,     0,   234,
     220,     0,     0,     0,     0,     0,     0,     0,   234,     0,
     220,     0,     0,     0,     0,     0,     0,     0,   839,     0,
     234,   220,   234,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   442,   443,   444,     0,     0,     0,
       0,   234,     0,   234,     0,     0,     0,     0,     0,     0,
       0,     0,   337,   337,   445,   446,     0,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
       0,   470,     0,   442,   443,   444,     0,     0,     0,     0,
       0,     0,     0,   471,     0,     0,   236,     0,     0,     0,
       0,     0,   978,   445,   446,     0,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,     0,
     470,     0,     0,     0,     0,     0,   220,     0,     0,     0,
       0,     0,   471,     0,     0,     0,     0,     0,     0,   337,
       0,   337,     0,     0,     0,     0,     0,   839,     0,   839,
       0,   839,     0,     0,   839,     0,     0,   209,   839,     0,
     839,     0,     0,   839,     0,     0,   442,   443,   444,     0,
       0,     0,     0,     0,   337,     0,     0,     0,     0,    50,
       0,     0,     0,   844,     0,   337,   445,   446,     0,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   982,   470,   844,   213,   214,   215,     0,     0,
       0,     0,     0,     0,     0,   471, -1038, -1038, -1038, -1038,
   -1038,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,     0,
       0,    91,    92,     0,    93,   179,    95,     0,     0,     0,
       0,     0,  1092,   839,     0,     0,     0,     0,     0,     0,
       0,  1108,     0,     0,     0,     0,     0,     0,     0,   105,
     953,     0,     0,   337,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   222,   222,     0,     0,
     240,     0,   442,   443,   444,     0,   220,   337,     0,   337,
       0,     0,     0,     0,     0,     0,   337,     0,     0,     0,
       0,     0,   445,   446,     0,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,     0,   470,
       0,   442,   443,   444,     0,   839,   839,   839,     0,     0,
       0,   471,   839,     0,  1168,   220,     0,     0,     0,     0,
     337,   445,   446,     0,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,     0,   470,     0,
       0,     0,     0,     0,     0,     0,   220,     0,   220,   209,
     471,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,    50,   470,   276,     0,     0,   220,   844,     0,     0,
       0,     0,     0,     0,   471,     0,     0,     0,     0,     0,
       0,     0,   844,   844,   844,   844,   844,     0,     0,     0,
       0,   278,     0,   844,   209,     0,     0,   213,   214,   215,
       0,     0,   337,     0,     0,     0,     0,  1096,     0,     0,
       0,     0,     0,   209,     0,     0,    50,     0,     0,   337,
    1179,   427,     0,    91,    92,   222,    93,   179,    95,   220,
       0,     0,     0,     0,     0,    50,     0,     0,  1799,     0,
       0,     0,     0,  1117,   839,   220,   220,     0,     0,     0,
       0,   105,   213,   214,   215,   839,     0,     0,     0,     0,
       0,   839,     0,     0,     0,   839,     0,     0,  1117,  1206,
     558,   213,   214,   215,   559,   351,     0,   220,    91,    92,
       0,    93,   179,    95,     0,     0,     0,   337,     0,     0,
       0,   178,     0,     0,    89,   328,     0,    91,    92,     0,
      93,   179,    95,     0,   844,     0,   105,  1161,     0,     0,
       0,   442,   443,   444,     0,   332,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   105,   333,   839,     0,   236,
       0,   445,   446,     0,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   222,   470,     0,
       0,     0,     0,   337,     0,     0,   222,     0,     0,     0,
     471,     0,     0,   222,     0,   337,     0,   337,     0,   220,
     220,     0,     0,   222,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   240,     0,   337,     0,   337,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   844,   844,   844,   844,     0,   220,     0,
       0,   844,   844,   844,   844,   844,   844,   844,   844,   844,
     844,   844,   844,   844,   844,   844,   844,   844,   844,   844,
     844,   844,   844,   844,   844,   844,   844,   844,   844,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   844,     0,     0,   209,   240,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   220,
       0,     0,     0,     0,     0,   846,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,  1565,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   222,
       0,   220,  1532,     0,     0,     0,   871,     0,     0,     0,
       0,     0,     0,     0,     0,  1533,   213,   214,   215,  1534,
       0,     0,     0,     0,   266,   267,     0,   268,   269,     0,
       0,   270,   271,   272,   273,     0,   178,     0,     0,    89,
      90,     0,    91,    92,   220,    93,  1536,    95,   274,   220,
     275,     0,   276,     0,     0,     0,   845,     0,     0,     0,
       0,     0,     0,     0,   220,   220,     0,   844,     0,     0,
     105,     0,     0,     0,     0,     0,     0,     0,   277,     0,
     278,     0,     0,     0,   844,     0,   844,   845,     0,     0,
       0,     0,   279,   280,   281,   282,   283,   284,   285,     0,
       0,     0,   209,     0,     0,     0,     0,     0,     0,     0,
     844,     0,     0,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,    50,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,  1428,     0,
       0,   220,   321,   322,   323,     0,     0,     0,   324,   325,
     213,   214,   215,   326,     0,     0,     0,     0,     0,   222,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     327,     0,     0,    89,   328,     0,    91,    92,     0,    93,
     179,    95,   329,     0,   330,     0,   276,   331,     0,     0,
       0,     0,     0,     0,   332,  1523,     0,     0,     0,     0,
       0,     0,     0,     0,   105,   333,     0,     0,     0,  1028,
       0,     0,     0,     0,   278,     0,     0,     0,   222,     0,
       0,     0,     0,     0,  1050,  1051,  1052,  1053,     0,     0,
       0,     0,     0,     0,     0,  1063,   209,     0,     0,   844,
       0,   844,     0,   844,     0,     0,   844,   220,     0,     0,
     844,     0,   844,     0,     0,   844,     0,     0,    50,   222,
       0,   222,     0,     0,     0,     0,     0,  1528,     0,     0,
    1541,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   220,     0,     0,     0,     0,   222,
     845,     0,     0,   558,   213,   214,   215,   559,     0,     0,
       0,     0,     0,     0,     0,   845,   845,   845,   845,   845,
       0,     0,     0,     0,   178,     0,   845,    89,   328,     0,
      91,    92,     0,    93,   179,    95,     0,  1045,     0,   220,
       0,     0,     0,     0,     0,     0,     0,     0,   332,     0,
       0,     0,     0,     0,     0,   844,  1158,     0,   105,   333,
       0,     0,   222,     0,   442,   443,   444,  1605,  1606,     0,
       0,     0,     0,     0,     0,     0,     0,  1541,   222,   222,
       0,     0,     0,     0,   445,   446,     0,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     240,   470,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   471,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   845,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   844,   844,   844,
       0,     0,     0,     0,   844,     0,  1753,     0,     0,     0,
       0,     0,   240,     0,  1541,     0,  1245,  1247,  1247,     0,
       0,     0,     0,  1258,  1261,  1262,  1263,  1265,  1266,  1267,
    1268,  1269,  1270,  1271,  1272,  1273,  1274,  1275,  1276,  1277,
    1278,  1279,  1280,  1281,  1282,  1283,  1284,  1285,  1286,  1287,
    1288,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   222,   222,     0,     0,     0,  1296,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   209,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   845,   845,   845,   845,
       0,   240,  1566,    50,   845,   845,   845,   845,   845,   845,
     845,   845,   845,   845,   845,   845,   845,   845,   845,   845,
     845,   845,   845,   845,   845,   845,   845,   845,   845,   845,
     845,   845,     0,     0,     0,     0,     0,     0,     0,   213,
     214,   215,     0,     0,     0,     0,     0,     0,   845,     0,
       0,     0,     0,     0,     0,     0,   844,     0,     0,     0,
       0,     0,   222,     0,     0,    91,    92,   844,    93,   179,
      95,     0,     0,   844,     0,     0,     0,   844,     0,  1384,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   105,   222,     0,  1399,     0,  1400,  1071,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
     266,   267,  1418,   268,   269,     0,     0,   270,   271,   272,
     273,     0,     0,  1092,     0,     0,     0,   240,     0,   844,
       0,     0,   222,     0,   274,     0,   275,     0,     0,  1868,
       0,     0,     0,     0,     0,     0,     0,   222,   222,     0,
     845,     0,     0,     0,     0,     0,  1528,     0,     0,     0,
       0,     0,     0,     0,   277,     0,     0,   845,     0,   845,
       0,     0,     0,     0,     0,     0,     0,     0,   279,   280,
     281,   282,   283,   284,   285,     0,     0,     0,   209,     0,
     210,    40,     0,   845,     0,     0,     0,     0,     0,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
      50,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   222,     0,     0,   727,   321,   322,
     323,     0,     0,     0,   324,   569,   213,   214,   215,   570,
       0,  1510,     0,  1511,     0,  1512,     0,     0,  1513,     0,
       0,     0,  1515,     0,  1516,     0,   571,  1517,     0,     0,
       0,     0,    91,    92,     0,    93,   179,    95,   329,     0,
     330,     0,     0,   331,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     105,     0,     0,     0,   728,     0,   109,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     5,     6,     7,     8,     9,     0,     0,
       0,     0,   845,    10,   845,     0,   845,     0,     0,   845,
     240,     0,     0,   845,     0,   845,     0,     0,   845,     0,
       0,     0,     0,     0,     0,     0,     0,  1600,   680,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,   222,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   240,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,   845,    62,
      63,    64,   174,   175,   176,     0,     0,    69,    70,  1746,
    1747,  1748,     0,     0,     0,     0,  1752,   177,    75,    76,
      77,    78,     0,    79,    80,    81,    82,    83,   276,     0,
       0,     0,     0,     0,    85,     0,     0,     0,     0,   178,
      87,    88,    89,    90,     0,    91,    92,     0,    93,   179,
      95,     0,   681,     0,    97,     0,   278,    98,     0,     0,
       0,     0,     0,    99,     0,     0,     0,     0,   102,   103,
     104,     0,     0,   105,   180,     0,     0,     0,   209,   109,
     110,     0,   111,   112,     0,     0,     0,   442,   443,   444,
     845,   845,   845,     0,     0,     0,     0,   845,     0,     0,
      50,     0,     0,     0,     0,     0,  1758,   445,   446,  1389,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,     0,   470,   558,   213,   214,   215,   559,
       0,     0,     0,     0,     0,     0,   471,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   178,     0,     0,    89,
     328,     0,    91,    92,     0,    93,   179,    95,  1808,  1396,
       0,     0,     0,     0,   442,   443,   444,     0,     0,  1818,
     332,     0,     0,     0,     0,  1823,     0,     0,     0,  1825,
     105,   333,     0,     0,   445,   446,     0,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
       0,   470,     0,     0,     0,     0,     0,     0,     5,     6,
       7,     8,     9,   471,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   845,
       0,  1860,    11,    12,    13,     0,     0,     0,     0,     0,
     845,     0,     0,     0,     0,     0,   845,     0,     0,     0,
     845,     0,     0,  1390,    14,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,  1841,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,   845,    52,    53,    54,    55,    56,    57,    58,
       0,    59,    60,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,    71,    72,    73,     0,     0,     0,
     472,     0,    74,    75,    76,    77,    78,     0,    79,    80,
      81,    82,    83,     0,     0,     0,    84,     0,     0,    85,
       0,     0,     0,   209,    86,    87,    88,    89,    90,     0,
      91,    92,     0,    93,    94,    95,    96,     0,     0,    97,
       0,     0,    98,     0,     0,    50,     0,     0,    99,   100,
       0,   101,     0,   102,   103,   104,     0,     0,   105,   106,
       0,   107,   108,  1128,   109,   110,     0,   111,   112,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,   213,   214,   215,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,   178,     0,     0,    89,    90,     0,    91,    92,     0,
      93,   179,    95,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,   105,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,    56,    57,
      58,     0,    59,    60,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,   209,    86,    87,    88,    89,    90,
       0,    91,    92,     0,    93,    94,    95,    96,     0,     0,
      97,     0,     0,    98,     0,     0,    50,     0,     0,    99,
     100,     0,   101,     0,   102,   103,   104,     0,     0,   105,
     106,     0,   107,   108,  1312,   109,   110,     0,   111,   112,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,   213,   214,   215,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,   861,     0,     0,    91,    92,
       0,    93,   179,    95,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,   105,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,    56,
      57,    58,     0,    59,    60,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,     0,
      79,    80,    81,    82,    83,     0,     0,     0,    84,     0,
       0,    85,     0,     0,     0,     0,    86,    87,    88,    89,
      90,     0,    91,    92,     0,    93,    94,    95,    96,     0,
       0,    97,     0,     0,    98,     0,     0,     0,     0,     0,
      99,   100,     0,   101,     0,   102,   103,   104,     0,     0,
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
       0,     0,    85,     0,     0,     0,     0,   178,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   179,    95,    96,
       0,     0,    97,     0,     0,    98,     0,     0,     0,     0,
       0,    99,     0,     0,     0,     0,   102,   103,   104,     0,
       0,   105,   106,     0,   107,   108,   665,   109,   110,     0,
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
       0,     0,   105,   106,     0,   107,   108,  1095,   109,   110,
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
     104,     0,     0,   105,   106,     0,   107,   108,  1142,   109,
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
     103,   104,     0,     0,   105,   106,     0,   107,   108,  1212,
     109,   110,     0,   111,   112,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,  1214,    47,     0,
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
       0,    48,     0,    49,  1385,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,     0,    79,    80,    81,    82,
      83,     0,     0,     0,    84,     0,     0,    85,     0,     0,
       0,     0,   178,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   179,    95,    96,     0,     0,    97,     0,     0,
      98,     0,     0,     0,     0,     0,    99,     0,     0,     0,
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
     107,   108,  1519,   109,   110,     0,   111,   112,     5,     6,
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
       0,   107,   108,  1749,   109,   110,     0,   111,   112,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,  1795,    49,     0,     0,    50,
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
     105,   106,     0,   107,   108,  1830,   109,   110,     0,   111,
     112,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,  1833,    48,     0,    49,     0,
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
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,     0,    79,    80,    81,    82,    83,     0,     0,     0,
      84,     0,     0,    85,     0,     0,     0,     0,   178,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   179,    95,
      96,     0,     0,    97,     0,     0,    98,     0,     0,     0,
       0,     0,    99,     0,     0,     0,     0,   102,   103,   104,
       0,     0,   105,   106,     0,   107,   108,  1850,   109,   110,
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
     104,     0,     0,   105,   106,     0,   107,   108,  1867,   109,
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
     103,   104,     0,     0,   105,   106,     0,   107,   108,  1924,
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
    1927,   109,   110,     0,   111,   112,     5,     6,     7,     8,
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
     108,     0,   109,   110,     0,   111,   112,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,   541,     0,     0,     0,
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
       0,     0,    11,    12,    13,     0,     0,   805,     0,     0,
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
       0,     0,     0,    11,    12,    13,     0,     0,  1030,     0,
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
     106,     0,   107,   108,     0,   109,   110,     0,   111,   112,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,  1595,
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
    1741,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,    10,     0,     0,     0,   209,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   402,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,   739,    50,
       0,     0,     0,     0,     0,     0,     0,   349,   350,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,   213,   214,   215,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,   351,     0,
       0,    91,    92,    50,    93,   179,    95,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   174,   175,   176,     0,     0,    69,    70,   105,
       0,     0,     0,     0,     0,     0,     0,   177,    75,    76,
      77,    78,     0,    79,    80,    81,    82,    83,     0,     0,
       0,     0,     0,     0,    85,     0,     0,     0,     0,   178,
      87,    88,    89,    90,     0,    91,    92,     0,    93,   179,
      95,     0,     0,     0,    97,     0,     0,    98,     0,     0,
       0,     0,     0,    99,     0,     0,     0,     0,   102,   103,
     104,     0,     0,   105,   106,     0,     0,     0,     0,   109,
     110,     0,   111,   112,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,   209,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,   859,   860,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,   213,   214,   215,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,   861,
       0,     0,    91,    92,    50,    93,   179,    95,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   174,   175,   176,     0,     0,    69,    70,
     105,     0,     0,     0,     0,     0,     0,     0,   177,    75,
      76,    77,    78,     0,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,    85,     0,     0,     0,     0,
     178,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     179,    95,     0,     0,     0,    97,     0,     0,    98,     0,
       0,     0,     0,     0,    99,     0,     0,     0,     0,   102,
     103,   104,     0,     0,   105,   180,     0,   342,     0,     0,
     109,   110,     0,   111,   112,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     102,   103,   104,     0,     0,   105,   180,     0,     0,   800,
       0,   109,   110,     0,   111,   112,     5,     6,     7,     8,
       9,     0,     0,     0,  1069,     0,    10,  1070,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,     0,
       0,  1155,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1092,    15,    16,     0,     0,     0,     0,    17,
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
       0,    93,   179,    95,     0,  1156,     0,    97,     0,     0,
      98,     0,     0,     0,     0,     0,    99,     0,     0,     0,
       0,   102,   103,   104,     0,     0,   105,   180,     0,     0,
       0,     0,   109,   110,     0,   111,   112,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   402,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,   102,   103,   104,     0,     0,   105,   106,   442,
     443,   444,     0,   109,   110,     0,   111,   112,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   445,
     446,     0,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,     0,   470,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,   471,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,   191,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   174,   175,   176,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   177,    75,    76,    77,    78,     0,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,    85,
       0,     0,     0,     0,   178,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   179,    95,     0,     0,     0,    97,
       0,     0,    98,     0,     0,   555,     0,     0,    99,     0,
       0,     0,     0,   102,   103,   104,     0,     0,   105,   180,
       0,     0,     0,     0,   109,   110,     0,   111,   112,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,     0,     0,   226,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1092,    15,    16,     0,     0,
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
     180,   442,   443,   444,     0,   109,   110,     0,   111,   112,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   445,   446,     0,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,     0,   470,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
     471,     0,     0,    17,     0,    18,    19,    20,    21,    22,
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
       0,    97,     0,     0,    98,     0,     0,   557,     0,     0,
      99,     0,     0,     0,     0,   102,   103,   104,     0,     0,
     105,   180,     0,   261,   443,   444,   109,   110,     0,   111,
     112,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,   445,   446,     0,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,     0,
     470,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,   471,     0,    17,     0,    18,    19,    20,    21,
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
       0,   105,   180,     0,   264,     0,     0,   109,   110,     0,
     111,   112,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   402,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
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
       0,     0,   105,   106,   442,   443,   444,     0,   109,   110,
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
      63,    64,   174,   175,   176,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   177,    75,    76,
      77,    78,     0,    79,    80,    81,    82,    83,     0,     0,
       0,     0,     0,     0,    85,     0,     0,     0,     0,   178,
      87,    88,    89,    90,     0,    91,    92,     0,    93,   179,
      95,     0,     0,     0,    97,     0,     0,    98,     0,     0,
     576,     0,     0,    99,     0,     0,     0,     0,   102,   103,
     104,     0,     0,   105,   180,   539,     0,     0,     0,   109,
     110,     0,   111,   112,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,   694,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1092,     0,     0,     0,
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
       0,     0,     0,     0,     0,    10,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,     0,   470,     0,
     739,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     471,     0,    15,    16,     0,     0,     0,     0,    17,     0,
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
       9,     0,     0,     0,     0,     0,    10,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,     0,     0,     0,
       0,   780,     0,     0,     0,     0,     0,     0,     0,     0,
    1092,     0,     0,    15,    16,     0,     0,     0,     0,    17,
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
       0,   102,   103,   104,     0,     0,   105,   180,     0,     0,
       0,     0,   109,   110,     0,   111,   112,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,     0,     0,     0,
       0,     0,   782,     0,     0,     0,     0,     0,     0,     0,
    1092,     0,     0,     0,    15,    16,     0,     0,     0,     0,
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
       7,     8,     9,     0,     0,     0,     0,     0,    10,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,     0,   470,     0,     0,
       0,     0,     0,  1123,     0,     0,     0,     0,     0,   471,
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
       0,     0,     0,     0,   109,   110,     0,   111,   112,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
   -1038, -1038, -1038, -1038,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,     0,   470,     0,
       0,     0,     0,     0,  1203,     0,     0,     0,     0,     0,
     471,     0,     0,     0,     0,     0,    15,    16,     0,     0,
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
      10,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,     0,     0,
       0,     0,     0,     0,     0,  1448,     0,     0,     0,     0,
       0,  1092,     0,     0,     0,     0,     0,    15,    16,     0,
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
     105,   180,   442,   443,   444,     0,   109,   110,     0,   111,
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
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     174,   175,   176,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   177,    75,    76,    77,    78,
       0,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,    85,     0,     0,     0,     0,   178,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   179,    95,     0,
       0,     0,    97,     0,     0,    98,     0,     0,   580,     0,
       0,    99,     0,     0,     0,     0,   102,   103,   104,     0,
       0,   105,   180,   442,   443,   444,     0,   109,   110,     0,
     111,   112,     5,     6,     7,     8,     9,     0,     0,     0,
       0,   809,    10,   445,   446,     0,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,     0,
     470,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,   471,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,   627,    39,    40,     0,   810,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   174,   175,   176,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   177,    75,    76,    77,
      78,     0,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,    85,     0,     0,     0,     0,   178,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   179,    95,
       0,   266,   267,    97,   268,   269,    98,     0,   270,   271,
     272,   273,    99,     0,     0,     0,     0,   102,   103,   104,
       0,     0,   105,   180,     0,   274,     0,   275,   109,   110,
       0,   111,   112, -1038, -1038, -1038, -1038,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
       0,     0,     0,     0,     0,   277,     0,     0,     0,     0,
       0,     0,     0,  1092,     0,     0,     0,     0,     0,   279,
     280,   281,   282,   283,   284,   285,     0,     0,     0,   209,
       0,   210,    40,     0,     0,     0,     0,     0,     0,     0,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,    50,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,     0,     0,     0,   320,   321,
     322,   323,     0,     0,     0,   324,   569,   213,   214,   215,
     570,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   266,   267,     0,   268,   269,     0,   571,   270,   271,
     272,   273,     0,    91,    92,     0,    93,   179,    95,   329,
       0,   330,     0,     0,   331,   274,     0,   275,     0,   276,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   105,     0,     0,     0,   728,     0,   109,     0,     0,
       0,     0,     0,     0,     0,   277,     0,   278,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   279,
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
       0,   330,     0,     0,   331,   266,   267,     0,   268,   269,
       0,   332,   270,   271,   272,   273,     0,     0,     0,     0,
       0,   105,   333,     0,     0,     0,  1720,     0,     0,   274,
       0,   275,   446,   276,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,     0,   470,   277,
       0,   278,     0,     0,     0,     0,     0,     0,     0,     0,
     471,     0,     0,   279,   280,   281,   282,   283,   284,   285,
       0,     0,     0,   209,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,    50,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,     0,
       0,     0,     0,   321,   322,   323,     0,     0,     0,   324,
     325,   213,   214,   215,   326,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   327,     0,     0,    89,   328,     0,    91,    92,     0,
      93,   179,    95,   329,     0,   330,     0,     0,   331,   266,
     267,     0,   268,   269,     0,   332,   270,   271,   272,   273,
       0,     0,     0,     0,     0,   105,   333,     0,     0,     0,
    1790,     0,     0,   274,     0,   275,     0,   276,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,     0,   470,   277,     0,   278,     0,     0,     0,     0,
       0,     0,     0,     0,   471,     0,     0,   279,   280,   281,
     282,   283,   284,   285,     0,     0,     0,   209,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,    50,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,     0,     0,     0,   320,   321,   322,   323,
       0,     0,     0,   324,   325,   213,   214,   215,   326,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   327,     0,     0,    89,   328,
       0,    91,    92,     0,    93,   179,    95,   329,     0,   330,
       0,     0,   331,   266,   267,     0,   268,   269,     0,   332,
     270,   271,   272,   273,     0,     0,     0,     0,     0,   105,
     333,     0,     0,     0,     0,     0,     0,   274,     0,   275,
       0,   276,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   277,     0,   278,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   279,   280,   281,   282,   283,   284,   285,     0,     0,
       0,   209,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,    50,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,     0,     0,     0,
       0,   321,   322,   323,     0,     0,     0,   324,   325,   213,
     214,   215,   326,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   327,
       0,     0,    89,   328,     0,    91,    92,     0,    93,   179,
      95,   329,     0,   330,     0,     0,   331,  1620,  1621,  1622,
    1623,  1624,     0,   332,  1625,  1626,  1627,  1628,     0,     0,
       0,     0,     0,   105,   333,     0,     0,     0,     0,     0,
       0,  1629,  1630,  1631,     0,   445,   446,     0,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,  1632,   470,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   471,  1633,  1634,  1635,  1636,  1637,
    1638,  1639,     0,     0,     0,   209,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1640,  1641,  1642,  1643,
    1644,  1645,  1646,  1647,  1648,  1649,  1650,    50,  1651,  1652,
    1653,  1654,  1655,  1656,  1657,  1658,  1659,  1660,  1661,  1662,
    1663,  1664,  1665,  1666,  1667,  1668,  1669,  1670,  1671,  1672,
    1673,  1674,  1675,  1676,  1677,  1678,  1679,  1680,     0,     0,
       0,  1681,  1682,   213,   214,   215,     0,  1683,  1684,  1685,
    1686,  1687,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1688,  1689,  1690,     0,     0,     0,    91,
      92,     0,    93,   179,    95,  1691,     0,  1692,  1693,     0,
    1694,     0,     0,     0,     0,     0,     0,  1695,  1696,     0,
    1697,     0,  1698,  1699,     0,   266,   267,   105,   268,   269,
    1067,  1068,   270,   271,   272,   273,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   274,
    1069,   275,     0,  1070,  1071,  1072,  1073,  1074,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,     0,     0,     0,     0,   277,
       0,     0,     0,     0,     0,     0,     0,     0,  1092,     0,
       0,     0,     0,   279,   280,   281,   282,   283,   284,   285,
       0,     0,     0,   209,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,    50,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,     0,
       0,     0,   320,   321,   322,   323,     0,     0,     0,   324,
     569,   213,   214,   215,   570,     0,     0,     0,     0,     0,
     266,   267,     0,   268,   269,     0,     0,   270,   271,   272,
     273,   571,     0,     0,     0,     0,     0,    91,    92,     0,
      93,   179,    95,   329,   274,   330,   275,     0,   331,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   105,     0,     0,     0,     0,
       0,     0,     0,     0,   277,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   279,   280,
     281,   282,   283,   284,   285,     0,     0,     0,   209,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
      50,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,     0,     0,     0,  1250,   321,   322,
     323,     0,     0,     0,   324,   569,   213,   214,   215,   570,
       0,     0,     0,     0,     0,   266,   267,     0,   268,   269,
       0,     0,   270,   271,   272,   273,   571,     0,     0,     0,
       0,     0,    91,    92,     0,    93,   179,    95,   329,   274,
     330,   275,     0,   331,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     105,     0,     0,     0,     0,     0,     0,     0,     0,   277,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   279,   280,   281,   282,   283,   284,   285,
       0,     0,     0,   209,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,    50,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,     0,
       0,     0,  1256,   321,   322,   323,     0,     0,     0,   324,
     569,   213,   214,   215,   570,     0,     0,     0,     0,     0,
     266,   267,     0,   268,   269,     0,     0,   270,   271,   272,
     273,   571,     0,     0,     0,     0,     0,    91,    92,     0,
      93,   179,    95,   329,   274,   330,   275,     0,   331,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   105,     0,     0,     0,     0,
       0,     0,     0,     0,   277,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   279,   280,
     281,   282,   283,   284,   285,     0,     0,     0,   209,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
      50,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,     0,     0,     0,     0,   321,   322,
     323,     0,     0,     0,   324,   569,   213,   214,   215,   570,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   571,     0,     0,     0,
       0,     0,    91,    92,     0,    93,   179,    95,   329,     0,
     330,     0,     0,   331,   442,   443,   444,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     105,     0,     0,     0,   445,   446,     0,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
       0,   470,   442,   443,   444,     0,     0,     0,     0,     0,
       0,     0,     0,   471,     0,     0,     0,     0,     0,     0,
       0,     0,   445,   446,     0,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,     0,   470,
    1066,  1067,  1068,     0,     0,     0,     0,     0,     0,     0,
       0,   471,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1069,     0,     0,  1070,  1071,  1072,  1073,  1074,  1075,
    1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1264,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1092,
       0,     0,     0,     0,   826,   827,     0,     0,     0,     0,
     828,     0,   829,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   830,     0,     0,     0,     0,   772,
       0,     0,    34,    35,    36,   209,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   211,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   797,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   831,   832,   833,   834,     0,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,   216,  1024,
       0,     0,     0,   178,    87,    88,    89,   835,     0,    91,
      92,     0,    93,   179,    95,  1255,     0,     0,    97,     0,
       0,     0,     0,     0,     0,     0,     0,   836,     0,     0,
       0,    29,   102,     0,     0,     0,     0,   105,   837,    34,
      35,    36,   209,     0,   210,    40,     0,     0,     0,     0,
       0,     0,   211,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1025,    75,
     213,   214,   215,     0,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,   216,     0,     0,     0,     0,
     178,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     179,    95,     0,   826,   827,    97,     0,     0,     0,   828,
       0,   829,     0,     0,     0,     0,     0,     0,     0,   102,
       0,     0,     0,   830,   105,   217,     0,     0,     0,     0,
     109,    34,    35,    36,   209,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   211,     0,     0,     0,     0,   984,
     985,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   986,
       0,     0,     0,     0,     0,     0,     0,   987,   988,   989,
     209,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     990,   831,   832,   833,   834,     0,    79,    80,    81,    82,
      83,     0,    50,     0,     0,     0,     0,   216,     0,     0,
       0,     0,   178,    87,    88,    89,   835,     0,    91,    92,
       0,    93,   179,    95,     0,     0,     0,    97,     0,     0,
       0,     0,     0,     0,     0,     0,   836,   991,   992,   993,
     994,   102,     0,     0,     0,     0,   105,   837,     0,     0,
       0,     0,     0,   995,     0,     0,     0,     0,     0,     0,
       0,     0,    29,     0,    91,    92,     0,    93,   179,    95,
      34,    35,    36,   209,     0,   210,    40,     0,     0,     0,
       0,     0,   996,   211,     0,     0,     0,     0,     0,     0,
       0,     0,   105,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,   213,   214,   215,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,   216,     0,     0,     0,
       0,   178,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   179,    95,    29,     0,     0,    97,     0,     0,     0,
       0,    34,    35,    36,   209,     0,   210,    40,     0,     0,
     102,     0,     0,     0,   211,   105,   217,     0,     0,   592,
       0,   109,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   212,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     612,    75,   213,   214,   215,     0,    79,    80,    81,    82,
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
       0,     0,  1120,    75,   213,   214,   215,     0,    79,    80,
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
      97,     0,     0,   442,   443,   444,     0,     0,     0,     0,
       0,     0,     0,     0,   102,     0,     0,     0,     0,   105,
     217,     0,     0,   445,   446,   109,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,     0,
     470,   442,   443,   444,     0,     0,     0,     0,     0,     0,
       0,     0,   471,     0,     0,     0,     0,     0,     0,     0,
       0,   445,   446,     0,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,     0,   470,     0,
       0,     0,     0,     0,     0,     0,     0,   442,   443,   444,
     471,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   445,   446,   516,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,     0,   470,   442,   443,   444,     0,     0,
       0,     0,     0,     0,     0,     0,   471,     0,     0,     0,
       0,     0,     0,     0,     0,   445,   446,   525,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,     0,   470,     0,     0,     0,     0,     0,     0,     0,
       0,   442,   443,   444,   471,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   445,   446,   893,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,     0,   470,   442,
     443,   444,     0,     0,     0,     0,     0,     0,     0,     0,
     471,     0,     0,     0,     0,     0,     0,     0,     0,   445,
     446,   959,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,     0,   470,     0,     0,     0,
       0,     0,     0,     0,     0,   442,   443,   444,   471,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   445,   446,  1009,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,     0,   470,  1066,  1067,  1068,     0,     0,     0,     0,
       0,     0,     0,     0,   471,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1069,  1310,     0,  1070,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1066,  1067,
    1068,     0,  1092,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1069,
       0,  1341,  1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,
    1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,
    1088,  1089,  1090,  1091,     0,     0,  1066,  1067,  1068,     0,
       0,     0,     0,     0,     0,     0,     0,  1092,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1069,     0,  1238,
    1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1066,  1067,  1068,     0,  1092,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1069,     0,  1405,  1070,  1071,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,     0,     0,  1066,
    1067,  1068,     0,     0,     0,     0,     0,     0,     0,     0,
    1092,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1069,     0,  1414,  1070,  1071,  1072,  1073,  1074,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1066,  1067,  1068,     0,  1092,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1069,     0,  1509,  1070,  1071,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
       0,    34,    35,    36,   209,     0,   210,    40,     0,     0,
       0,     0,     0,  1092,   211,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1601,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   231,     0,     0,
       0,     0,     0,   232,     0,     0,     0,     0,     0,     0,
       0,     0,   213,   214,   215,     0,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,   216,     0,     0,
    1603,     0,   178,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   179,    95,     0,     0,     0,    97,     0,    34,
      35,    36,   209,     0,   210,    40,     0,     0,     0,     0,
       0,   102,   641,     0,     0,     0,   105,   233,     0,     0,
       0,     0,   109,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     213,   214,   215,     0,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,   216,     0,     0,     0,     0,
     178,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     179,    95,     0,     0,     0,    97,     0,    34,    35,    36,
     209,     0,   210,    40,     0,     0,     0,     0,     0,   102,
     211,     0,     0,     0,   105,   642,     0,     0,     0,     0,
     109,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   231,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   213,   214,
     215,     0,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,   216,     0,     0,     0,     0,   178,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   179,    95,
       0,     0,     0,    97,     0,     0,     0,     0,     0,   442,
     443,   444,     0,     0,     0,     0,     0,   102,     0,     0,
       0,     0,   105,   233,     0,     0,     0,     0,   109,   445,
     446,   956,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,     0,   470,   442,   443,   444,
       0,     0,     0,     0,     0,     0,     0,     0,   471,     0,
       0,     0,     0,     0,     0,     0,     0,   445,   446,     0,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,     0,   470,  1066,  1067,  1068,     0,     0,
       0,     0,     0,     0,     0,     0,   471,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1069,  1419,     0,  1070,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1066,  1067,  1068,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1092,     0,     0,     0,     0,     0,
       0,     0,  1069,     0,     0,  1070,  1071,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,   444,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1092,     0,     0,     0,     0,   445,   446,     0,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,  1068,   470,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   471,     0,     0,     0,     0,     0,
    1069,     0,     0,  1070,  1071,  1072,  1073,  1074,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1092
};

static const yytype_int16 yycheck[] =
{
       5,     6,   157,     8,     9,    10,    11,    12,    13,   127,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    56,    96,    29,    30,   182,   100,   101,    56,
       4,     4,     4,   657,     4,     4,   106,    33,   736,    44,
     395,   395,  1145,   395,   541,    57,   932,    52,   653,    54,
      46,   157,    57,   126,    59,    51,    56,   510,    60,   683,
     234,   162,   654,  1141,   529,   183,   506,   507,  1301,   921,
      31,    27,    28,    31,    31,   470,    31,   106,   245,    84,
     577,   586,   587,   502,    86,   349,   350,    89,   502,    57,
     808,   634,    44,   815,   952,   535,   106,   106,  1023,     9,
      32,   106,     9,     9,    14,     9,     9,    32,     9,     9,
     968,    14,     9,    14,     9,     9,    49,     9,   537,     9,
    1132,    49,     9,   537,    50,    51,    49,     9,     9,    36,
      70,    49,    84,     9,     9,     9,     9,   784,    49,     9,
      38,     9,     9,     9,     9,   246,     9,    38,     9,    70,
      70,   180,  1010,    70,   774,    70,    83,   158,   655,    83,
      38,    70,  1734,    90,   158,   115,    38,   109,    90,   474,
     180,   180,   134,   135,   194,   180,   164,   123,   179,   534,
     194,    83,   187,   122,   130,    83,   134,   135,   217,   177,
      38,   130,    83,   194,   134,   135,   197,     0,   503,   164,
     194,    90,    38,   508,   233,    83,    70,   217,   217,   197,
    1062,    83,   217,   380,   134,   135,    70,    70,    70,  1791,
       4,    83,    84,   233,    70,    70,   176,   194,   233,   156,
     157,    70,   197,   155,   158,    83,   198,     8,    70,   863,
      70,   158,   247,   158,    70,   250,   194,    83,    70,   276,
      70,   278,   257,   258,   180,    70,    70,   197,  1336,    70,
      70,     4,   195,   191,    70,   197,   155,   195,   191,    53,
     226,   196,    56,   194,   172,   202,   432,   195,   179,   251,
     197,   172,   197,   255,   195,   194,   196,  1520,   195,    73,
     196,   197,   196,   196,  1219,   196,   196,   199,     4,   196,
     172,   196,   196,   800,   196,   195,   333,   341,   805,   196,
      94,  1033,    96,  1035,   196,   196,   100,   101,  1176,    70,
     196,   196,   196,   196,   971,   195,  1338,   195,   195,   195,
     195,    83,   195,  1345,   195,  1347,   172,   199,   158,   102,
     369,   341,   126,   197,   197,   197,  1198,   349,   350,   351,
     194,   197,   197,   426,   859,   860,   512,   949,   197,   369,
     369,  1373,   194,   368,   369,   197,   484,   910,   194,   195,
     375,   376,   377,   378,   194,   197,    31,   197,   383,   194,
     194,   194,   197,   197,   194,   387,   197,   197,  1466,   423,
    1468,   197,    57,   420,   194,    50,   423,   402,    53,   162,
      83,    54,   358,   158,    69,   410,   479,   480,   481,   482,
     197,   367,   196,   369,   157,   485,   368,   422,   374,  1430,
     172,   127,   194,   423,   179,   377,   378,   401,   384,   106,
     107,    70,  1130,   194,    54,   194,   889,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,  1477,   471,   251,   473,   474,
     475,   255,    83,   485,    14,   259,   476,   183,   106,   107,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   660,    83,   662,  1574,  1215,   503,   504,
    1578,   506,   507,   508,   509,   134,   135,   190,   513,   470,
     163,   516,   470,   470,   197,   470,   671,   485,  1529,   938,
     525,   198,   527,   523,   938,   965,   102,    50,    51,    70,
     535,   543,   179,  1030,   476,   562,    83,    84,   543,  1144,
     545,    83,  1553,   163,  1555,    83,   194,   194,    90,   102,
     102,   105,   106,   107,   510,    83,   730,   341,   178,  1411,
     502,   172,   102,   103,   194,   671,   921,   921,  1160,   921,
     198,  1163,    83,  1193,  1194,  1195,   548,   534,   973,  1199,
     194,   523,   738,   172,   586,   587,   162,   592,   762,   853,
      70,   855,   534,   134,   135,   537,    75,    76,     4,   163,
    1105,   119,   120,   121,   122,   123,   124,    31,   194,   162,
     162,   778,   779,   642,   156,   157,   689,   158,   785,   786,
      32,    27,    28,   650,   651,   409,    14,   155,   156,   157,
     162,    83,   659,   158,   172,   419,   194,   642,    90,   423,
      38,  1493,   426,    70,    32,   156,   157,   162,  1346,    70,
     197,    31,   199,   194,   179,  1733,    70,    81,    83,  1737,
     179,   203,   179,    51,   838,    90,   409,    91,  1901,   196,
      50,   189,   197,    53,   848,   194,   681,   194,   196,   103,
     654,   196,   197,    83,   785,    53,    54,    55,   196,   694,
      90,   196,   476,   477,   478,   479,   480,   481,   482,    83,
     106,    69,   196,   409,   156,   157,    90,  1062,  1062,   196,
    1062,   179,    83,  1178,   196,   139,   140,   141,   502,    90,
    1325,   196,   179,   728,   680,   196,   194,    75,    76,  1169,
     155,   156,   157,    70,  1354,   159,  1356,   194,   162,   523,
    1180,   165,   166,   197,   168,   169,   170,  1445,  1340,    70,
     111,   158,   757,   537,   194,   155,   156,   157,   119,   120,
     121,   122,   123,   124,   548,  1218,    53,    54,    55,   193,
      57,   194,   156,   157,   180,    70,  1879,   483,   105,   106,
     107,   158,    69,   739,   568,   156,   157,   792,   102,   103,
    1893,   119,   120,   121,  1291,  1873,   162,   824,   825,   194,
     381,   132,   133,   808,   385,   196,   590,   591,   804,    48,
    1888,   217,   119,   120,   121,   122,   123,   124,   196,   197,
     226,   110,   111,   112,   780,    69,   782,   233,   189,   196,
     197,   412,  1530,   414,   415,   416,   417,   179,    81,   623,
     624,   813,   158,  1198,  1198,   251,  1198,   349,   350,   255,
     194,   853,  1305,   855,   810,  1765,  1766,   859,   860,   861,
     103,  1761,  1762,   201,  1484,   194,  1486,     9,  1488,   194,
     158,  1491,    50,    51,    52,    53,    54,    55,  1375,    57,
     824,   825,   189,   158,   911,    81,  1478,     8,   893,   194,
     895,    69,   897,   196,  1391,   158,   139,   140,   141,   972,
     927,   906,  1342,    14,   158,   689,   196,   103,     9,   130,
     196,   130,   197,   940,   196,   920,   159,   873,    14,   162,
     179,   195,   165,   166,   924,   168,   169,   170,   671,    14,
     102,   195,   200,   889,   890,   909,   909,   909,   195,   909,
     909,   946,   969,   139,   140,   141,   195,   195,   194,   111,
     193,   956,   358,   194,   959,   198,   961,   194,   932,  1845,
     965,   367,     9,   369,   155,  1585,   162,   195,   374,   165,
     166,   195,   168,   169,   170,   949,    94,  1151,   384,  1865,
     195,   195,   924,     9,   196,    14,   179,   194,  1874,     9,
     774,   194,   776,   197,   936,   196,   938,   193,   197,  1496,
     195,   197,   196,   409,  1009,    83,   195,   195,  1505,   196,
    1037,   194,   973,  1040,   798,   973,   973,   132,   973,   195,
    1016,  1518,   201,     9,  1464,     9,   201,   201,   812,   813,
      70,   229,    32,   133,   178,   158,   119,   120,   121,   122,
     123,   124,   136,     9,   195,  1017,   158,   130,   131,    14,
     191,     9,  1122,     9,   180,   798,  1411,  1411,   195,  1411,
       9,  1061,    14,   132,  1064,   201,   850,  1023,  1024,  1011,
     201,   198,     9,   857,   858,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,   171,    14,
     195,   201,   798,   195,   201,   194,   158,   195,  1595,   102,
       9,   196,   886,  1105,   510,   196,   189,   850,    91,   136,
    1122,     9,   158,   195,   194,    70,  1143,  1122,  1145,  1739,
    1740,    27,    28,    70,    70,   909,   158,   194,    67,    68,
      50,    51,    52,    53,    54,    55,   158,   197,  1493,  1493,
     924,  1493,   548,   197,   850,  1172,   930,     9,  1175,    69,
     198,  1156,   936,    14,   938,  1129,  1129,  1129,   196,  1129,
    1129,  1785,   180,     9,  1169,    14,   909,  1123,   201,   197,
      14,   195,   197,   196,   191,  1180,  1181,     4,    32,   194,
     194,    32,    14,   194,   194,    14,  1160,   930,   972,  1163,
     194,    70,    52,    70,    70,   134,   135,   194,     9,  1155,
     984,   985,   986,   909,  1231,   195,   194,   196,  1235,   196,
    1215,  1207,   136,    14,   119,   120,   121,   122,   123,   124,
    1225,   180,    49,   136,   930,   130,   131,  1011,   195,     9,
      69,   201,     9,  1017,  1731,  1019,    83,  1021,  1210,  1863,
      78,    79,    80,    81,  1741,   198,     9,  1203,   198,   136,
      31,   196,   194,   194,    14,    83,   195,  1041,   196,   195,
     194,   194,  1218,  1219,   169,   103,   171,   197,   195,   197,
     196,  1298,  1299,   136,   680,  1899,  1019,  1061,  1021,   184,
    1064,   186,   197,   201,   189,   112,     9,    91,   155,  1786,
     117,  1446,   119,   120,   121,   122,   123,   124,   125,    32,
      81,   139,   140,   141,   197,  1310,    77,   196,   196,  1093,
     195,   180,  1317,  1019,   136,  1021,  1321,    32,  1323,   201,
     226,   519,   103,  1441,   195,   195,  1331,   165,   166,  1303,
     168,   169,   170,   739,  1831,     9,  1341,  1342,   165,   166,
    1314,   168,   201,     9,   125,  1129,   201,   136,     9,  1305,
     195,     9,   195,    83,    14,   193,   198,   138,   139,   140,
     141,   142,   189,   196,   198,   196,  1340,   197,  1395,   194,
    1397,   198,   195,   197,   780,   195,   782,   195,   159,   196,
     194,   162,   163,   195,   165,   166,  1129,   168,   169,   170,
     195,   201,   798,     9,   136,  1551,     9,   136,   201,   201,
     195,  1898,     9,    32,   810,   196,  1903,   813,  1192,  1193,
    1194,  1195,   193,   195,  1441,  1199,   195,   112,   196,   167,
     618,   619,   196,  1129,   197,   163,  1210,   196,    14,     4,
      83,   117,   197,   195,   195,   136,   195,   136,  1222,   179,
      14,   197,   196,    83,   850,    14,   644,    81,    14,    83,
    1523,    85,   358,    83,   195,   136,  1461,   194,   136,  1464,
     195,   367,   196,    14,   196,    14,     4,   873,   374,   103,
     196,    27,    28,    14,    49,   197,     9,     9,   384,  1222,
     198,    59,    83,   889,   890,   179,   194,    83,     9,   115,
     197,   196,  1448,  1467,   102,   158,   102,   180,   170,  1473,
      36,  1475,    14,   909,  1478,   139,   140,   141,   195,   194,
     180,    49,   196,  1297,   194,   180,  1222,   176,    83,   173,
     195,  1495,     9,    83,   930,   196,  1526,   195,   726,   193,
    1502,   165,   166,   195,   168,   169,   170,   112,    14,    83,
     197,     9,   117,    14,   119,   120,   121,   122,   123,   124,
     125,    14,    83,    83,    14,    83,  1105,  1854,   477,   193,
    1303,   482,   479,  1719,  1569,   966,   912,  1870,  1594,  1388,
    1354,  1314,  1356,  1216,   112,  1865,    27,    28,   594,   117,
      31,   119,   120,   121,   122,   123,   124,   125,  1581,  1616,
     165,   166,  1703,   168,  1618,  1531,  1436,  1910,  1886,    78,
      79,    80,  1715,  1432,   510,    56,  1577,  1060,   378,  1583,
    1195,  1017,    91,  1019,   189,  1021,  1057,  1023,  1024,   817,
    1133,  1593,  1594,   198,   822,   985,  1191,   165,   166,  1192,
     168,  1006,   936,   423,   824,  1897,  1912,  1421,  1820,   375,
    1113,  1424,  1093,  1042,    -1,    -1,  1430,    -1,    -1,    -1,
      -1,   189,  1436,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     198,    -1,    -1,    -1,   143,   144,   145,   146,   147,    -1,
     226,    -1,    -1,    -1,    -1,   154,    -1,    -1,  1421,    -1,
    1714,   160,   161,  1838,    -1,    -1,    -1,   885,    -1,    -1,
      -1,  1718,    -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,
    1484,    -1,  1486,  1446,  1488,    -1,  1779,  1491,    -1,   188,
      -1,    -1,    -1,  1497,  1714,  1421,    -1,  1123,  1502,  1724,
      -1,    -1,  1506,  1129,  1467,    -1,    -1,    -1,    -1,    -1,
    1473,    -1,  1475,    -1,  1440,    -1,    -1,    -1,    -1,  1523,
    1858,    -1,  1526,    -1,    -1,  1529,    -1,    -1,    -1,  1155,
      -1,    -1,  1495,    -1,  1497,  1539,    -1,    -1,    -1,    -1,
      -1,    -1,  1546,  1506,    -1,    -1,    -1,    -1,    -1,  1553,
      -1,  1555,    -1,    -1,   680,   226,  1803,    -1,  1562,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1497,    -1,    -1,    -1,    -1,    -1,  1203,    -1,    -1,
    1506,  1585,   358,    -1,  1210,    -1,  1004,    -1,  1592,  1593,
    1594,   367,  1218,  1219,  1598,    -1,  1222,    -1,   374,    -1,
    1604,    -1,    -1,    -1,    -1,   276,    -1,   278,   384,    -1,
      -1,    -1,    -1,   739,    -1,    -1,    -1,    -1,    -1,   395,
    1583,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1592,
      -1,  1049,  1879,  1559,    -1,  1598,    -1,    -1,  1056,    -1,
      -1,  1604,    -1,    -1,  1891,    -1,  1893,    -1,    -1,    -1,
      -1,  1845,    -1,    -1,   780,    -1,   782,    -1,    -1,    -1,
      -1,    -1,   333,    -1,    -1,  1912,  1592,  1914,    -1,    -1,
      -1,  1865,  1598,    -1,    -1,    -1,    -1,    -1,  1604,  1305,
    1874,    -1,    -1,  1908,   810,    -1,    -1,   358,     4,    -1,
      -1,    -1,    -1,    -1,    -1,  1920,   367,    -1,    -1,    -1,
      -1,    -1,    -1,   374,    -1,  1930,    -1,    -1,  1933,    -1,
    1714,    -1,    -1,   384,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   395,    -1,    -1,    -1,    -1,    -1,
      -1,  1735,    -1,    49,   510,  1739,  1740,    -1,    -1,    -1,
      -1,  1745,    -1,    -1,    -1,    -1,    -1,   873,    -1,   420,
    1754,    -1,   423,    -1,    -1,    -1,    -1,  1761,  1762,    -1,
      -1,  1765,  1766,   889,   890,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1190,    -1,    -1,  1779,    -1,    -1,    -1,    -1,
      -1,    -1,  1745,  1787,    -1,    -1,    -1,    -1,    -1,    -1,
    1794,    -1,    -1,    -1,    -1,  1421,   112,    -1,     4,   470,
      -1,   117,    -1,   119,   120,   121,   122,   123,   124,   125,
      -1,    -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,  1745,
      -1,    -1,  1448,    -1,  1787,  1829,  1244,    -1,    -1,    -1,
    1248,  1794,    -1,  1837,    -1,  1253,    -1,    -1,    -1,   510,
      -1,    -1,  1260,    49,    -1,    -1,    -1,  1851,    -1,   165,
     166,    -1,   168,  1857,    -1,    -1,    -1,    -1,    49,    -1,
      -1,  1787,    -1,    -1,    -1,    -1,  1829,    -1,  1794,    -1,
      -1,  1497,    -1,   189,    -1,  1838,  1502,    -1,    -1,    -1,
    1506,    -1,   198,    -1,    -1,    -1,    -1,    -1,  1851,    -1,
      -1,   562,    -1,   564,    -1,    -1,   567,  1023,  1024,    -1,
       4,    -1,    -1,  1829,   680,    -1,   112,  1911,    -1,    -1,
      -1,   117,  1916,   119,   120,   121,   122,   123,   124,   125,
      -1,   112,    -1,    -1,    -1,  1851,   117,   598,   119,   120,
     121,   122,   123,   124,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1361,    -1,    49,    -1,  1365,  1911,    -1,
      -1,    -1,  1370,  1916,    -1,    -1,    -1,    -1,    -1,   165,
     166,    -1,   168,   739,    -1,    -1,  1592,  1593,  1594,    -1,
      -1,    -1,  1598,    -1,   165,   166,    -1,   168,  1604,   650,
     651,    -1,    -1,   189,    -1,  1911,    -1,    -1,   659,    -1,
    1916,    -1,   198,    -1,    -1,    -1,    -1,  1123,   189,    -1,
      -1,    -1,    -1,    -1,   780,    -1,   782,   198,   112,   680,
      56,    -1,    -1,   117,    -1,   119,   120,   121,   122,   123,
     124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1155,
      -1,    -1,    -1,    -1,   810,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,
      -1,   165,   166,    -1,   168,    -1,    -1,    -1,   739,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1203,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   189,    -1,    -1,    -1,    -1,
      -1,    -1,  1218,  1219,   198,    67,    68,   873,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   780,
      -1,   782,    -1,   889,   890,    -1,    -1,    -1,    -1,  1745,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    -1,   810,
     811,    -1,    -1,    -1,    -1,   921,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   824,   825,   826,   827,   828,   829,   830,
      -1,  1787,   134,   135,    -1,    -1,   837,    -1,  1794,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    68,    -1,    -1,  1305,
     851,    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      -1,    -1,   873,  1829,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    83,    84,   887,    -1,   889,   890,
      -1,    -1,    -1,   195,    -1,  1851,    -1,    -1,    -1,    -1,
     276,    -1,   278,    -1,   103,    -1,    -1,    67,    68,    -1,
     911,   912,   134,   135,    -1,    -1,    -1,  1023,  1024,    -1,
     921,    -1,    -1,    -1,    -1,    -1,   927,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   940,
     139,   140,   141,    -1,    -1,    -1,    -1,   948,    -1,    -1,
     951,    -1,    -1,    -1,    -1,  1911,  1062,   333,    -1,    -1,
    1916,    -1,    -1,    -1,   163,    -1,   165,   166,   969,   168,
     169,   170,   973,   195,   134,   135,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,  1448,    -1,   193,    -1,    -1,    -1,   197,    -1,
     199,    -1,    -1,    -1,    -1,    -1,    -1,  1755,  1756,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1123,    -1,    -1,
      -1,    -1,  1023,  1024,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    68,    -1,    -1,    -1,  1037,    -1,    -1,  1040,
      -1,  1042,    -1,    -1,   420,    -1,    -1,   423,    -1,  1155,
      -1,    -1,    -1,    -1,    -1,    -1,  1057,  1058,  1059,  1060,
      -1,  1062,    -1,    -1,  1065,  1066,  1067,  1068,  1069,  1070,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,  1198,    -1,    -1,    -1,    -1,  1203,   134,   135,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1109,    -1,
      -1,    -1,  1218,  1219,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,  1123,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,  1143,    57,  1145,    -1,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1155,    69,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,  1172,    -1,    -1,  1175,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   562,    -1,   564,    -1,
     103,   567,    -1,    -1,    -1,    -1,    -1,  1198,   111,  1305,
      -1,    -1,  1203,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,   139,   140,   141,  1218,  1219,    -1,
    1221,    -1,   598,    -1,    -1,   138,   139,   140,   141,   142,
    1231,    -1,    -1,    -1,  1235,    -1,    -1,  1238,   163,  1240,
     165,   166,   167,   168,   169,   170,   159,    -1,    -1,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,
      -1,    -1,    -1,  1264,    -1,    -1,    -1,    -1,   193,   194,
     183,    -1,    -1,    -1,   650,   651,   189,    -1,    -1,    -1,
     193,   194,    -1,   659,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1298,  1299,    -1,
      -1,  1302,    -1,    -1,  1305,  1411,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,  1448,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    81,    -1,    83,    84,  1493,    -1,    -1,
      -1,    -1,    -1,    69,  1395,    -1,  1397,    81,    -1,    -1,
      -1,    -1,  1403,    -1,  1405,   103,  1407,    -1,    31,  1410,
    1411,    -1,    -1,  1414,    -1,  1416,    -1,    -1,  1419,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1430,
    1431,    -1,    -1,  1434,    -1,   811,    59,    -1,    -1,    -1,
    1441,   139,   140,   141,   128,    -1,    -1,  1448,   824,   825,
     826,   827,   828,   829,   830,   139,   140,   141,    81,    -1,
      -1,   837,    -1,    -1,    -1,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,    -1,   201,    -1,    -1,
     103,   165,   166,    -1,   168,   169,   170,    -1,   111,    -1,
      -1,    -1,  1493,    -1,    -1,   193,    -1,    -1,    -1,   197,
      -1,   199,    -1,    -1,    -1,    -1,    -1,    81,  1509,   193,
     194,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
    1521,  1522,   198,    27,    28,    -1,    -1,    31,  1529,   103,
    1531,    -1,    -1,    -1,    -1,   911,   159,    -1,    -1,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,
      -1,   927,  1553,    -1,  1555,    -1,    -1,    -1,    -1,    -1,
     183,  1562,    -1,    -1,   940,   139,   140,   141,    -1,    -1,
     193,   194,   948,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,   165,   166,   969,   168,   169,   170,    -1,    -1,    69,
    1601,  1602,  1603,    10,    11,    12,    -1,  1608,    -1,  1610,
      -1,    -1,    -1,    -1,    -1,  1616,    -1,  1618,    -1,   193,
     194,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1037,    69,    -1,  1040,    -1,  1042,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1057,  1058,  1059,  1060,    -1,    -1,    -1,    -1,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,
    1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1092,  1718,    -1,    -1,
      -1,    -1,   226,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1109,  1735,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,  1754,    57,    -1,    -1,    -1,    -1,  1760,
      -1,    -1,    -1,    -1,    -1,    -1,    69,  1143,    -1,  1145,
    1771,    -1,    -1,    -1,    -1,    -1,  1777,    -1,    -1,    -1,
    1781,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,   198,    -1,    -1,    -1,    -1,  1172,    -1,    -1,  1175,
      -1,    -1,  1803,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1843,    69,    -1,  1221,    -1,    -1,    -1,    -1,
      -1,    -1,  1853,    -1,   358,  1231,    -1,    -1,    -1,  1235,
      -1,    -1,  1238,   367,  1240,    -1,    -1,    -1,    -1,  1870,
     374,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1879,    -1,
     384,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1264,    -1,
    1891,   395,  1893,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,  1912,    -1,  1914,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1298,  1299,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,   470,    -1,    -1,    -1,
      -1,    -1,   198,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,   510,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,  1395,
      -1,  1397,    -1,    -1,    -1,    -1,    -1,  1403,    -1,  1405,
      -1,  1407,    -1,    -1,  1410,    -1,    -1,    81,  1414,    -1,
    1416,    -1,    -1,  1419,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,  1430,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,   567,    -1,  1441,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   198,    57,   598,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,
      -1,    -1,    69,  1509,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,
     194,    -1,    -1,  1529,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      31,    -1,    10,    11,    12,    -1,   680,  1553,    -1,  1555,
      -1,    -1,    -1,    -1,    -1,    -1,  1562,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    10,    11,    12,    -1,  1601,  1602,  1603,    -1,    -1,
      -1,    69,  1608,    -1,   198,   739,    -1,    -1,    -1,    -1,
    1616,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   780,    -1,   782,    81,
      69,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,   103,    57,    31,    -1,    -1,   810,   811,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   826,   827,   828,   829,   830,    -1,    -1,    -1,
      -1,    59,    -1,   837,    81,    -1,    -1,   139,   140,   141,
      -1,    -1,  1718,    -1,    -1,    -1,    -1,   851,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,   103,    -1,    -1,  1735,
     198,   163,    -1,   165,   166,   226,   168,   169,   170,   873,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,  1754,    -1,
      -1,    -1,    -1,   887,  1760,   889,   890,    -1,    -1,    -1,
      -1,   193,   139,   140,   141,  1771,    -1,    -1,    -1,    -1,
      -1,  1777,    -1,    -1,    -1,  1781,    -1,    -1,   912,   198,
     138,   139,   140,   141,   142,   162,    -1,   921,   165,   166,
      -1,   168,   169,   170,    -1,    -1,    -1,  1803,    -1,    -1,
      -1,   159,    -1,    -1,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,   948,    -1,   193,   951,    -1,    -1,
      -1,    10,    11,    12,    -1,   183,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   193,   194,  1843,    -1,   973,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,   358,    57,    -1,
      -1,    -1,    -1,  1879,    -1,    -1,   367,    -1,    -1,    -1,
      69,    -1,    -1,   374,    -1,  1891,    -1,  1893,    -1,  1023,
    1024,    -1,    -1,   384,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   395,    -1,  1912,    -1,  1914,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1057,  1058,  1059,  1060,    -1,  1062,    -1,
      -1,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,
    1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1109,    -1,    -1,    81,   470,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1123,
      -1,    -1,    -1,    -1,    -1,   567,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   510,
      -1,  1155,   125,    -1,    -1,    -1,   598,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
      -1,    10,    11,    12,    13,    -1,   159,    -1,    -1,   162,
     163,    -1,   165,   166,  1198,   168,   169,   170,    27,  1203,
      29,    -1,    31,    -1,    -1,    -1,   567,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1218,  1219,    -1,  1221,    -1,    -1,
     193,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      59,    -1,    -1,    -1,  1238,    -1,  1240,   598,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1264,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,  1302,    -1,
      -1,  1305,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,   680,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     159,    -1,    -1,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,   171,    -1,   173,    -1,    31,   176,    -1,    -1,
      -1,    -1,    -1,    -1,   183,   184,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   193,   194,    -1,    -1,    -1,   811,
      -1,    -1,    -1,    -1,    59,    -1,    -1,    -1,   739,    -1,
      -1,    -1,    -1,    -1,   826,   827,   828,   829,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   837,    81,    -1,    -1,  1403,
      -1,  1405,    -1,  1407,    -1,    -1,  1410,  1411,    -1,    -1,
    1414,    -1,  1416,    -1,    -1,  1419,    -1,    -1,   103,   780,
      -1,   782,    -1,    -1,    -1,    -1,    -1,  1431,    -1,    -1,
    1434,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1448,    -1,    -1,    -1,    -1,   810,
     811,    -1,    -1,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   826,   827,   828,   829,   830,
      -1,    -1,    -1,    -1,   159,    -1,   837,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,   172,    -1,  1493,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,    -1,
      -1,    -1,    -1,    -1,    -1,  1509,   948,    -1,   193,   194,
      -1,    -1,   873,    -1,    10,    11,    12,  1521,  1522,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1531,   889,   890,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
     921,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   948,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1601,  1602,  1603,
      -1,    -1,    -1,    -1,  1608,    -1,  1610,    -1,    -1,    -1,
      -1,    -1,   973,    -1,  1618,    -1,  1058,  1059,  1060,    -1,
      -1,    -1,    -1,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1023,  1024,    -1,    -1,    -1,  1109,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1057,  1058,  1059,  1060,
      -1,  1062,   198,   103,  1065,  1066,  1067,  1068,  1069,  1070,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,  1109,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1760,    -1,    -1,    -1,
      -1,    -1,  1123,    -1,    -1,   165,   166,  1771,   168,   169,
     170,    -1,    -1,  1777,    -1,    -1,    -1,  1781,    -1,  1221,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   193,  1155,    -1,  1238,    -1,  1240,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
       3,     4,  1264,     6,     7,    -1,    -1,    10,    11,    12,
      13,    -1,    -1,    69,    -1,    -1,    -1,  1198,    -1,  1843,
      -1,    -1,  1203,    -1,    27,    -1,    29,    -1,    -1,  1853,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1218,  1219,    -1,
    1221,    -1,    -1,    -1,    -1,    -1,  1870,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    -1,  1238,    -1,  1240,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      83,    84,    -1,  1264,    -1,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,  1305,    -1,    -1,   130,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
      -1,  1403,    -1,  1405,    -1,  1407,    -1,    -1,  1410,    -1,
      -1,    -1,  1414,    -1,  1416,    -1,   159,  1419,    -1,    -1,
      -1,    -1,   165,   166,    -1,   168,   169,   170,   171,    -1,
     173,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     193,    -1,    -1,    -1,   197,    -1,   199,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,  1403,    13,  1405,    -1,  1407,    -1,    -1,  1410,
    1411,    -1,    -1,  1414,    -1,  1416,    -1,    -1,  1419,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1509,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,  1448,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1493,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,  1509,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,  1601,
    1602,  1603,    -1,    -1,    -1,    -1,  1608,   137,   138,   139,
     140,   141,    -1,   143,   144,   145,   146,   147,    31,    -1,
      -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,
     160,   161,   162,   163,    -1,   165,   166,    -1,   168,   169,
     170,    -1,   172,    -1,   174,    -1,    59,   177,    -1,    -1,
      -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,   188,   189,
     190,    -1,    -1,   193,   194,    -1,    -1,    -1,    81,   199,
     200,    -1,   202,   203,    -1,    -1,    -1,    10,    11,    12,
    1601,  1602,  1603,    -1,    -1,    -1,    -1,  1608,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,  1617,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,  1760,   172,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,  1771,
     183,    -1,    -1,    -1,    -1,  1777,    -1,    -1,    -1,  1781,
     193,   194,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,     7,    69,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1760,
      -1,  1843,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
    1771,    -1,    -1,    -1,    -1,    -1,  1777,    -1,    -1,    -1,
    1781,    -1,    -1,   196,    49,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,  1805,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,    94,
      -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,  1843,   108,   109,   110,   111,   112,   113,   114,
      -1,   116,   117,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,   129,   130,   131,    -1,    -1,    -1,
     196,    -1,   137,   138,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,   154,
      -1,    -1,    -1,    81,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,   171,    -1,    -1,   174,
      -1,    -1,   177,    -1,    -1,   103,    -1,    -1,   183,   184,
      -1,   186,    -1,   188,   189,   190,    -1,    -1,   193,   194,
      -1,   196,   197,   198,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,   159,    -1,    -1,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    49,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,   193,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,
      94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
     114,    -1,   116,   117,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,   129,   130,   131,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,
     154,    -1,    -1,    -1,    81,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,   171,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,   103,    -1,    -1,   183,
     184,    -1,   186,    -1,   188,   189,   190,    -1,    -1,   193,
     194,    -1,   196,   197,   198,   199,   200,    -1,   202,   203,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   162,    -1,    -1,   165,   166,
      -1,   168,   169,   170,    -1,    -1,    49,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,   193,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,   112,
     113,   114,    -1,   116,   117,   118,   119,   120,   121,   122,
     123,   124,    -1,   126,   127,   128,   129,   130,   131,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,
     143,   144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,
      -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,   171,    -1,
      -1,   174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,
     183,   184,    -1,   186,    -1,   188,   189,   190,    -1,    -1,
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
      -1,    -1,    -1,    91,    92,    93,    94,    95,    96,    -1,
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
      -1,    98,    -1,   100,   101,    -1,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,    -1,   113,   114,    -1,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,    -1,   143,   144,   145,   146,
     147,    -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,
     177,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,
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
      92,    93,    94,    -1,    96,    97,    98,    -1,   100,    -1,
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
      -1,    -1,   137,   138,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,   174,
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
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
     151,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,    -1,
      -1,    -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,
      -1,    -1,   193,   194,    -1,   196,   197,    -1,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,   112,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,   139,   140,   141,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,
      -1,   165,   166,   103,   168,   169,   170,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,   193,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,   143,   144,   145,   146,   147,    -1,    -1,
      -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,
     160,   161,   162,   163,    -1,   165,   166,    -1,   168,   169,
     170,    -1,    -1,    -1,   174,    -1,    -1,   177,    -1,    -1,
      -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,   188,   189,
     190,    -1,    -1,   193,   194,    -1,    -1,    -1,    -1,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,   112,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,   139,   140,   141,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,   162,
      -1,    -1,   165,   166,   103,   168,   169,   170,    -1,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,   127,   128,
     193,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,    -1,   143,   144,   145,   146,   147,    -1,
      -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,
     159,   160,   161,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,    -1,
      -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,   188,
     189,   190,    -1,    -1,   193,   194,    -1,   196,    -1,    -1,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     188,   189,   190,    -1,    -1,   193,   194,    -1,    -1,   197,
      -1,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    31,    -1,    13,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    50,    51,    -1,    -1,    -1,    -1,    56,
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
      -1,   168,   169,   170,    -1,   172,    -1,   174,    -1,    -1,
     177,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,
      -1,   188,   189,   190,    -1,    -1,   193,   194,    -1,    -1,
      -1,    -1,   199,   200,    -1,   202,   203,     3,     4,     5,
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
      -1,    -1,    -1,   108,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,   174,
      -1,    -1,   177,    -1,    -1,   196,    -1,    -1,   183,    -1,
      -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,   194,
      -1,    -1,    -1,    -1,   199,   200,    -1,   202,   203,     3,
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
     194,    10,    11,    12,    -1,   199,   200,    -1,   202,   203,
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
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,
     143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,
      -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,
      -1,   174,    -1,    -1,   177,    -1,    -1,   196,    -1,    -1,
     183,    -1,    -1,    -1,    -1,   188,   189,   190,    -1,    -1,
     193,   194,    -1,   196,    11,    12,   199,   200,    -1,   202,
     203,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    69,    -1,    56,    -1,    58,    59,    60,    61,
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
      -1,   193,   194,    -1,   196,    -1,    -1,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     190,    -1,    -1,   193,   194,   195,    -1,    -1,    -1,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
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
     188,   189,   190,    -1,    -1,   193,   194,    -1,    -1,    -1,
      -1,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
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
      -1,   188,   189,   190,    -1,    -1,   193,   194,    -1,    -1,
      -1,    -1,   199,   200,    -1,   202,   203,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
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
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    69,
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
      -1,    -1,    -1,    -1,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
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
      13,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
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
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,    -1,
      -1,    -1,   174,    -1,    -1,   177,    -1,    -1,   196,    -1,
      -1,   183,    -1,    -1,    -1,    -1,   188,   189,   190,    -1,
      -1,   193,   194,    10,    11,    12,    -1,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    28,    13,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    69,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,   102,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
      -1,     3,     4,   174,     6,     7,   177,    -1,    10,    11,
      12,    13,   183,    -1,    -1,    -1,    -1,   188,   189,   190,
      -1,    -1,   193,   194,    -1,    27,    -1,    29,   199,   200,
      -1,   202,   203,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,    -1,    -1,   130,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,   159,    10,    11,
      12,    13,    -1,   165,   166,    -1,   168,   169,   170,   171,
      -1,   173,    -1,    -1,   176,    27,    -1,    29,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   193,    -1,    -1,    -1,   197,    -1,   199,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
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
      -1,   173,    -1,    -1,   176,     3,     4,    -1,     6,     7,
      -1,   183,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   193,   194,    -1,    -1,    -1,   198,    -1,    -1,    27,
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
      -1,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   159,    -1,    -1,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,   173,    -1,    -1,   176,     3,
       4,    -1,     6,     7,    -1,   183,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   193,   194,    -1,    -1,    -1,
     198,    -1,    -1,    27,    -1,    29,    -1,    31,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    57,    -1,    59,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,    -1,    -1,   130,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,   171,    -1,   173,
      -1,    -1,   176,     3,     4,    -1,     6,     7,    -1,   183,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   193,
     194,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,
      -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    59,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,    -1,    -1,
      -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,
      -1,    -1,   162,   163,    -1,   165,   166,    -1,   168,   169,
     170,   171,    -1,   173,    -1,    -1,   176,     3,     4,     5,
       6,     7,    -1,   183,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,   193,   194,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    57,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,    -1,   143,   144,   145,
     146,   147,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   159,   160,   161,    -1,    -1,    -1,   165,
     166,    -1,   168,   169,   170,   171,    -1,   173,   174,    -1,
     176,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,    -1,
     186,    -1,   188,   189,    -1,     3,     4,   193,     6,     7,
      11,    12,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      31,    29,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
      -1,    -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,     6,     7,    -1,    -1,    10,    11,    12,
      13,   159,    -1,    -1,    -1,    -1,    -1,   165,   166,    -1,
     168,   169,   170,   171,    27,   173,    29,    -1,   176,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,    -1,    -1,   130,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,    -1,    10,    11,    12,    13,   159,    -1,    -1,    -1,
      -1,    -1,   165,   166,    -1,   168,   169,   170,   171,    27,
     173,    29,    -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     193,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
      -1,    -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,     6,     7,    -1,    -1,    10,    11,    12,
      13,   159,    -1,    -1,    -1,    -1,    -1,   165,   166,    -1,
     168,   169,   170,   171,    27,   173,    29,    -1,   176,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,    -1,    -1,    -1,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,
      -1,    -1,   165,   166,    -1,   168,   169,   170,   171,    -1,
     173,    -1,    -1,   176,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     193,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,   195,
      -1,    -1,    78,    79,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,    -1,   143,   144,   145,
     146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    38,
      -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,   195,    -1,    -1,   174,    -1,
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
     199,    78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      91,   138,   139,   140,   141,    -1,   143,   144,   145,   146,
     147,    -1,   103,    -1,    -1,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,   138,   139,   140,
     141,   188,    -1,    -1,    -1,    -1,   193,   194,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    -1,   165,   166,    -1,   168,   169,   170,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      -1,    -1,   183,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   193,    -1,    -1,   103,    -1,    -1,    -1,    -1,
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
     130,   131,   132,   133,   137,   138,   142,   159,   163,   171,
     173,   176,   183,   194,   211,   212,   213,   224,   472,   492,
     493,   496,   196,   341,   343,   346,   197,   237,   346,   111,
     112,   162,   214,   215,   216,   217,   221,    83,   199,   291,
     292,   123,   130,   122,   130,    83,   293,   194,   194,   194,
     194,   211,   263,   475,   194,   194,    70,    70,    70,   336,
      83,    90,   155,   156,   157,   464,   465,   162,   197,   221,
     221,   211,   264,   475,   163,   194,   475,   475,    83,   190,
     197,   357,    27,   334,   338,   346,   347,   444,   448,   226,
     197,   453,    90,   406,   464,    90,   464,   464,    32,   162,
     179,   476,   194,     9,   196,    38,   243,   163,   262,   475,
     124,   189,   244,   326,   196,   196,   196,   196,   196,   196,
     196,   196,    10,    11,    12,    30,    31,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      57,    69,   196,    70,    70,   197,   158,   131,   169,   171,
     184,   186,   265,   324,   325,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    67,    68,
     134,   135,   434,    70,   197,   439,   194,   194,    70,   197,
     194,   243,   244,    14,   346,   196,   136,    48,   211,   429,
      90,   334,   347,   158,   444,   136,   201,     9,   414,   258,
     334,   347,   444,   476,   158,   194,   407,   434,   439,   195,
     346,    32,   228,     8,   358,     9,   196,   228,   229,   336,
     337,   346,   211,   277,   232,   196,   196,   196,   138,   142,
     496,   496,   179,   495,   194,   111,   496,    14,   158,   138,
     142,   159,   211,   213,   196,   196,   196,   238,   115,   176,
     196,   214,   216,   214,   216,   221,   197,     9,   415,   196,
     102,   162,   197,   444,     9,   196,   130,   130,    14,     9,
     196,   444,   468,   336,   334,   347,   444,   447,   448,   195,
     179,   255,   137,   444,   457,   458,   346,   366,   367,   336,
     381,   381,   196,    70,   434,   155,   465,    82,   346,   444,
      90,   155,   465,   221,   210,   196,   197,   250,   260,   390,
     392,    91,   194,   359,   360,   362,   403,   450,   452,   469,
      14,   102,   470,   353,   354,   355,   287,   288,   432,   433,
     195,   195,   195,   195,   195,   198,   227,   228,   245,   252,
     259,   432,   346,   200,   202,   203,   211,   477,   478,   496,
      38,   172,   289,   290,   346,   472,   194,   475,   253,   243,
     346,   346,   346,   346,    32,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   404,
     346,   346,   454,   454,   346,   460,   461,   130,   197,   212,
     213,   453,   263,   211,   264,   475,   475,   262,   244,    38,
     338,   341,   343,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   163,   197,   211,   435,
     436,   437,   438,   453,   454,   346,   289,   289,   454,   346,
     457,   243,   195,   346,   194,   428,     9,   414,   195,   195,
      38,   346,    38,   346,   407,   195,   195,   195,   451,   452,
     453,   289,   197,   211,   435,   436,   453,   195,   226,   281,
     197,   343,   346,   346,    94,    32,   228,   275,   196,    28,
     102,    14,     9,   195,    32,   197,   278,   496,    31,    91,
     224,   489,   490,   491,   194,     9,    50,    51,    56,    58,
      70,   138,   139,   140,   141,   163,   183,   194,   222,   224,
     373,   376,   379,   385,   400,   408,   409,   411,   211,   494,
     226,   194,   236,   197,   196,   197,   196,   102,   162,   111,
     112,   162,   217,   218,   219,   220,   221,   217,   211,   346,
     292,   409,    83,     9,   195,   195,   195,   195,   195,   195,
     195,   196,    50,    51,   485,   487,   488,   132,   268,   194,
       9,   195,   195,   136,   201,     9,   414,     9,   414,   201,
     201,    83,    85,   211,   466,   211,    70,   198,   198,   207,
     209,    32,   133,   267,   178,    54,   163,   178,   394,   347,
     136,     9,   414,   195,   158,   496,   496,    14,   358,   287,
     226,   191,     9,   415,   496,   497,   434,   439,   434,   198,
       9,   414,   180,   444,   346,   195,     9,   415,    14,   350,
     246,   132,   266,   194,   475,   346,    32,   201,   201,   136,
     198,     9,   414,   346,   476,   194,   256,   251,   261,    14,
     470,   254,   243,    72,   444,   346,   476,   201,   198,   195,
     195,   201,   198,   195,    50,    51,    70,    78,    79,    80,
      91,   138,   139,   140,   141,   154,   183,   211,   374,   377,
     380,   417,   419,   420,   424,   427,   211,   444,   444,   136,
     266,   434,   439,   195,   346,   282,    75,    76,   283,   226,
     335,   226,   337,   102,    38,   137,   272,   444,   409,   211,
      32,   228,   276,   196,   279,   196,   279,     9,   414,    91,
     136,   158,     9,   414,   195,   172,   477,   478,   479,   477,
     409,   409,   409,   409,   409,   413,   416,   194,    70,    70,
      70,   158,   194,   409,   158,   197,    10,    11,    12,    31,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    69,   158,   476,   198,   400,   197,   240,   216,
     216,   211,   217,   217,   221,     9,   415,   198,   198,    14,
     444,   196,   180,     9,   414,   211,   269,   400,   197,   457,
     137,   444,    14,    38,   346,   346,   201,   346,   198,   207,
     496,   269,   197,   393,    14,   195,   346,   359,   453,   196,
     496,   191,   198,    32,   483,   433,    38,    83,   172,   435,
     436,   438,   435,   436,   496,    38,   172,   346,   409,   287,
     194,   400,   267,   351,   247,   346,   346,   346,   198,   194,
     289,   268,    32,   267,   496,    14,   266,   475,   404,   198,
     194,    14,    78,    79,    80,   211,   418,   418,   420,   422,
     423,    52,   194,    70,    70,    70,    90,   155,   194,     9,
     414,   195,   428,    38,   346,   267,   198,    75,    76,   284,
     335,   228,   198,   196,    95,   196,   272,   444,   194,   136,
     271,    14,   226,   279,   105,   106,   107,   279,   198,   496,
     180,   136,   496,   211,   489,     9,   195,   414,   136,   201,
       9,   414,   413,   368,   369,   409,   382,   409,   410,   382,
     130,   212,   359,   361,   363,   195,   130,   212,   409,   462,
     463,   409,   409,   409,    32,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   494,
      83,   241,   198,   198,   220,   196,   409,   488,   102,   103,
     484,   486,     9,   297,   195,   194,   338,   343,   346,   444,
     136,   201,   198,   470,   297,   164,   177,   197,   389,   396,
     164,   197,   395,   136,   196,   483,   496,   358,   497,    83,
     172,    14,    83,   476,   444,   346,   195,   287,   197,   287,
     194,   136,   194,   289,   195,   197,   496,   197,   196,   496,
     267,   248,   407,   289,   136,   201,     9,   414,   419,   422,
     370,   371,   420,   383,   420,   421,   383,   155,   359,   425,
     426,   420,   444,   197,   335,    32,    77,   228,   196,   337,
     271,   457,   272,   195,   409,   101,   105,   196,   346,    32,
     196,   280,   198,   180,   496,   136,   172,    32,   195,   409,
     409,   195,   201,     9,   414,   136,   201,     9,   414,   201,
     136,     9,   414,   195,   136,   198,     9,   414,   409,    32,
     195,   226,   196,   196,   211,   496,   496,   484,   400,     4,
     112,   117,   123,   125,   165,   166,   168,   198,   298,   323,
     324,   325,   330,   331,   332,   333,   432,   457,    38,   346,
     198,   197,   198,    54,   346,   346,   346,   358,    38,    83,
     172,    14,    83,   346,   194,   483,   195,   297,   195,   287,
     346,   289,   195,   297,   470,   297,   196,   197,   194,   195,
     420,   420,   195,   201,     9,   414,   136,   201,     9,   414,
     201,   136,   195,     9,   414,   297,    32,   226,   196,   195,
     195,   195,   233,   196,   196,   280,   226,   496,   496,   136,
     409,   409,   409,   409,   359,   409,   409,   409,   197,   198,
     486,   132,   133,   184,   212,   473,   496,   270,   400,   112,
     333,    31,   125,   138,   142,   163,   169,   307,   308,   309,
     310,   400,   167,   315,   316,   128,   194,   211,   317,   318,
     299,   244,   496,     9,   196,     9,   196,   196,   470,   324,
     195,   444,   294,   163,   391,   198,   198,    83,   172,    14,
      83,   346,   289,   117,   348,   483,   198,   483,   195,   195,
     198,   197,   198,   297,   287,   136,   420,   420,   420,   420,
     359,   198,   226,   231,   234,    32,   228,   274,   226,   195,
     409,   136,   136,   136,   226,   400,   400,   475,    14,   212,
       9,   196,   197,   473,   470,   310,   179,   197,     9,   196,
       3,     4,     5,     6,     7,    10,    11,    12,    13,    27,
      28,    29,    57,    71,    72,    73,    74,    75,    76,    77,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   137,   138,   143,   144,   145,   146,   147,   159,   160,
     161,   171,   173,   174,   176,   183,   184,   186,   188,   189,
     211,   397,   398,     9,   196,   163,   167,   211,   318,   319,
     320,   196,    83,   329,   243,   300,   473,   473,    14,   244,
     198,   295,   296,   473,    14,    83,   346,   195,   194,   483,
     196,   197,   321,   348,   483,   294,   198,   195,   420,   136,
     136,    32,   228,   273,   274,   226,   409,   409,   409,   198,
     196,   196,   409,   400,   303,   496,   311,   312,   408,   308,
      14,    32,    51,   313,   316,     9,    36,   195,    31,    50,
      53,    14,     9,   196,   213,   474,   329,    14,   496,   243,
     196,    14,   346,    38,    83,   388,   197,   226,   483,   321,
     198,   483,   420,   420,   226,    99,   239,   198,   211,   224,
     304,   305,   306,     9,   414,     9,   414,   198,   409,   398,
     398,    59,   314,   319,   319,    31,    50,    53,   409,    83,
     179,   194,   196,   409,   475,   409,    83,     9,   415,   226,
     198,   197,   321,    97,   196,   115,   235,   158,   102,   496,
     180,   408,   170,    14,   485,   301,   194,    38,    83,   195,
     198,   226,   196,   194,   176,   242,   211,   324,   325,   180,
     409,   180,   285,   286,   433,   302,    83,   198,   400,   240,
     173,   211,   196,   195,     9,   415,   119,   120,   121,   327,
     328,   285,    83,   270,   196,   483,   433,   497,   195,   195,
     196,   193,   480,   327,    38,    83,   172,   483,   197,   481,
     482,   496,   196,   197,   322,   497,    83,   172,    14,    83,
     480,   226,     9,   415,    14,   484,   226,    38,    83,   172,
      14,    83,   346,   322,   198,   482,   496,   198,    83,   172,
      14,    83,   346,    14,    83,   346,   346
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
#line 2833 "hphp.y"
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
#line 2848 "hphp.y"
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
#line 2983 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2994 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2996 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2998 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3002 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3006 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3007 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3013 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3017 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3024 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3037 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 3060 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3065 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { (yyval).reset();;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3077 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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

  case 923:

/* Line 1455 of yacc.c  */
#line 3093 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3094 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3098 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3099 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 928:

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

  case 929:

/* Line 1455 of yacc.c  */
#line 3111 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3115 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3118 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3119 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3126 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { (yyval).reset();;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3131 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3133 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3139 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3140 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3141 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3146 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3147 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3151 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3152 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3153 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3154 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3159 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3160 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3165 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3167 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3169 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3170 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3174 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3176 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3177 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3179 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3184 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3186 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 964:

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

  case 965:

/* Line 1455 of yacc.c  */
#line 3198 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3200 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3201 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3204 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3205 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3206 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3210 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3211 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3212 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
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
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3217 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3218 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3219 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3220 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3224 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3225 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3230 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3232 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3246 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3251 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3255 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3260 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3267 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
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
#line 3278 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3282 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3288 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3292 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3299 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3300 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3304 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3307 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1004:

/* Line 1455 of yacc.c  */
#line 3313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1005:

/* Line 1455 of yacc.c  */
#line 3318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1006:

/* Line 1455 of yacc.c  */
#line 3319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3320 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3321 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1015:

/* Line 1455 of yacc.c  */
#line 3342 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1016:

/* Line 1455 of yacc.c  */
#line 3343 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1; ;}
    break;

  case 1019:

/* Line 1455 of yacc.c  */
#line 3352 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1022:

/* Line 1455 of yacc.c  */
#line 3363 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 1023:

/* Line 1455 of yacc.c  */
#line 3365 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 1024:

/* Line 1455 of yacc.c  */
#line 3369 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 1025:

/* Line 1455 of yacc.c  */
#line 3372 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 1026:

/* Line 1455 of yacc.c  */
#line 3376 "hphp.y"
    {;}
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
#line 3384 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 1030:

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

  case 1031:

/* Line 1455 of yacc.c  */
#line 3400 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 1032:

/* Line 1455 of yacc.c  */
#line 3405 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1033:

/* Line 1455 of yacc.c  */
#line 3406 "hphp.y"
    { ;}
    break;

  case 1034:

/* Line 1455 of yacc.c  */
#line 3411 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 1035:

/* Line 1455 of yacc.c  */
#line 3412 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 1036:

/* Line 1455 of yacc.c  */
#line 3418 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1037:

/* Line 1455 of yacc.c  */
#line 3423 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1038:

/* Line 1455 of yacc.c  */
#line 3428 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1039:

/* Line 1455 of yacc.c  */
#line 3432 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1040:

/* Line 1455 of yacc.c  */
#line 3437 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 1041:

/* Line 1455 of yacc.c  */
#line 3439 "hphp.y"
    { _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)])); (yyval) = (yyvsp[(2) - (5)]);;}
    break;

  case 1042:

/* Line 1455 of yacc.c  */
#line 3445 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1043:

/* Line 1455 of yacc.c  */
#line 3448 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1044:

/* Line 1455 of yacc.c  */
#line 3451 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1045:

/* Line 1455 of yacc.c  */
#line 3452 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1046:

/* Line 1455 of yacc.c  */
#line 3455 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1047:

/* Line 1455 of yacc.c  */
#line 3458 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1048:

/* Line 1455 of yacc.c  */
#line 3461 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1049:

/* Line 1455 of yacc.c  */
#line 3464 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1050:

/* Line 1455 of yacc.c  */
#line 3466 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1051:

/* Line 1455 of yacc.c  */
#line 3472 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1052:

/* Line 1455 of yacc.c  */
#line 3478 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1053:

/* Line 1455 of yacc.c  */
#line 3486 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1054:

/* Line 1455 of yacc.c  */
#line 3487 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14623 "hphp.7.tab.cpp"
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
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

